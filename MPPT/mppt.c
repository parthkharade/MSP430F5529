/*
 * Code for mppt implementation using P&O algorithm.
The housekeeping functions store a local copy of the PVI values which are fetched by functions in the mppt source code.
I am doing this tight now for the sake for simplicity. All the housekeeping data can be made global later on if required.

Things to do:
1) Write functions for
	a) GPIO and Timer initialization. // RTOS uses TIEMR0_A0 so this module is to be kept free to support future inclusions
	                                     of RTOS if any. Use TIMER_A1 for sample and TIMER_A2 for PWM.
	b) PWM Generation.
	c) Timer for sampling interval.
	d) Update duty cycle.
	e) Get sensor data.
	f) Perturb and Observe.

3) 	P_old V_old I_old.
	P_new V_new I_new.
	after updating duty cycle old <----- new (part of update duty cycle function).

4)	Initialize duty cycle to 50% initially

5) ISRs in the code
	a) Timer interrupt ISR.

//////////  PIN USAGE \\\\\\\\\\
	_________________
	|	MSP430F5529	|
	|			  	|
	|			3.0 |<-----> SDA(not to be initialized here)
	|			3.1	|------> SCL(not to be initialized here)
	|             	|
	|			2.4	|------> TIMER A2 CCR1 output for PWM
	|				|
	|_______________|

*/

#include <msp430.h>
#include <mppt.h>
#include<Housekeeping.h>
volatile bool update = false;
volatile uint16_t duty 	= 20;
int resultV = 0;
int resultP = 0;
int count = 0;s

typedef struct Old
	{
		volatile float P ;
		volatile float Volt ;
		//volatile float I = 0.0f;
		// Reading I is not really required if we are directly reading V and P. Just keeping it for the sake of it.
	} Old;

typedef struct New
	{
		volatile float P ;
		volatile float Volt ;
		//volatile float I = 0.0f;
		// Reading I is not really required if we are directly reading V and P. Just keeping it for the sake of it.
	} New;

struct New New1;
struct Old Old1;

void main(void)
{


    WDTCTL = WDTPW | WDTHOLD;		 	// stop internal watch-dog timer.

    init_GPIO();
    init_Timer();    //Note when writing the complete code all peripheral initializations can be grouped together.
                     //Then this can be called from a main init function such as GPIO_init, I2C_init ,Timer_init etc etc.

    while(1)
    {
    	if(update == true)
    	{
    		update_duty(); //this function further calls get_data() and P&O. Then after evaluating the values updates the duty cycle.
    	}
    }


}

#pragma vector = TIMER1_A0_VECTOR   //The CCR0 vector for timer A1..... TIMER1_A = TIMER A1 and the following 0 denotes whether
                                    //it is a CCR0 interrupt or the other grouped interrupt(denoted by TIMER1_A1_VECTOR)
__interrupt void update_isr(void)
{
	update = true;
}


void init_GPIO()
{
	P2DIR |= BIT4;
	P2OUT |= BIT4;
}
void init_Timer()
{

//*************Sampling Timer**************
    TA1CTL |= (TASSEL__ACLK + MC__UP + ID__1);     //Initialize Timer A1 as sampling timer
    TA1CCR0 = (uint16_t)UPDATE_FREQ;
    TA1CCTL0 |= CCIE;

//****************PWM Timer******************
    TA2CTL |= (TASSEL__SMCLK + MC__UP + ID__1);     //Timer A2 for PWM, SMCLK, UP Mode, Divider 1
    TA2CCTL1 |= OUTMOD_3;                           //Set Reset Mode
    TA2CCR0 = PERIOD;
    TA2CCR1 = duty;                                 //CCR 1 value set.

}
void update_duty()
{
	PandO();
    TA2CCR1 = duty;
	update = false;
}
void PandO()
{

	New1.P = getPanelPower();
	New1.Volt = getPanelVoltage();

	if(New1.P > Old1.P)
	{
		if(New1.Volt > Old1.Volt)
		{
			duty--;			//increase panel voltage
		}
		else
		{
			duty++;			//decrease panel voltage
		}
		Old1.P = New1.P;
 		Old1.Volt =	New1.Volt;
	}
	else if(New1.P < Old1.P)
	{
		if(New1.Volt > Old1.Volt)
		{
			duty++;			//decrease panel voltage
		}
		else
		{
			duty--;			//increase panel voltage
		}
		Old1.P = New1.P;
 		Old1.Volt =	New1.Volt;
	}
    else
	{
       Old1.P = New1.P;
       Old1.Volt = New1.Volt;
	}


}

float getPanelPower()
{
	volatile float P = 0.0f;
	//volatile uint16_t PVal = 0; // Value in INA219 Power register
	READINFO PVal[3];
	PVal[0] = readdata(INA219_PANEL1,INA219_POWER);           //Include Housekeeping.h in this Project
	PVal[1] = raeddata(INA219_PANEL2,INA219_POWER);
	PVal[2] = readdata(INA219_PANEL3,INA219_POWER);
	for(int i=0; i<3 ;i++)
	{
	    if(PVal[i].readstatus)
	    {
	        count++;
	        resultP = resultP + PVal[i].result;
	    }
	    else
	    {
	        printf("NACK in sensor %d\n",i+1);
	    }
	}
	P = (resultP/count) /* *something */;   //(consult INA219 Datasheet for this, Value depends on shunt Resistor)
	count = 0;
	return P;
}


float getPanelVoltage()
{
    /*
     * Have made an array of structs where eaxh struct is of type READINFO defined in housekeeping.h.
     */
	volatile float Volt = 0.0f;
	//volatile uint16_t VVal = 0;                       // Value in INA219 Voltage register
	READINFO VVal[3];
    VVal[0] = readdata(INA219_PANEL1,INA219_POWER);
    VVal[1] = raeddata(INA219_PANEL2,INA219_POWER);
    VVal[2] = readdata(INA219_PANEL3,INA219_POWER);
    for(int i=0; i<3 ;i++)
    {
        if(VVal[i].readstatus)
        {
            count++;
            resultV = resultV + VVal[i].result;
        }
        else
        {
            printf("NACK in sensor %d\n",i+1);
        }
    }
	Volt = (resultV/count)  /* *something*/ ; //(consult INA219 Datasheet for this, Value depends on Shunt Resistor)
	count = 0;
	return Volt;
}
