#include <msp430.h> 
#include "gpio.c"
#include "timer_a.c"
#include "stdio.h"


volatile bool g1,g2,g3,g4;
uint8_t count =4;

Timer_A_initUpModeParam param = {0};
Timer_A_initCompareModeParam cparam = {0};





/*
 * main.c
 */
int main(void)
{
	g1= false;
	g2= false;
	g3= false;
	g4= false;

	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    __enable_interrupt();
    gpio_init();
    timer_init();

    while(1)
    {
    	__no_operation();
    }

}

void gpio_init()
{
	//setting pin directions
	GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN7);
	GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4);
	GPIO_setAsInputPin(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5);

	//configuring gpio interrupts
	GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5);
	GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5, GPIO_LOW_TO_HIGH_TRANSITION);
	GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5);

	//setting initial values
	GPIO_setOutputLowOnPin(GPIO_PORT_P4,GPIO_PIN7);
	GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4);

}
void timer_init()
{
	param.clockSource = TIMER_A_CLOCKSOURCE_ACLK;
	param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_32;
	param.timerPeriod = 3000;
	param.timerClear = TIMER_A_DO_CLEAR;
	param.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
	param.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE;
	param.startTimer = false;

	cparam.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_ENABLE;
	cparam.compareOutputMode = TIMER_A_OUTPUTMODE_OUTBITVALUE;
	cparam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
	cparam.compareValue = 1000;


	Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1|TIMER_A_CAPTURECOMPARE_REGISTER_0);
	Timer_A_initUpMode(TIMER_A1_BASE, &param);
	Timer_A_initCompareMode(TIMER_A1_BASE, &cparam);
	Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);

}
void reset()
{

	count =4;
	g1= false;
	g2= false;
	g3= false;
	g4= false;
	GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN7);
	GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5);
	GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5);
	Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_1|TIMER_A_CAPTURECOMPARE_REGISTER_0);
	Timer_A_initUpMode(TIMER_A1_BASE, &param);
	Timer_A_initCompareMode(TIMER_A1_BASE, &cparam);
	Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
}
#pragma vector = 47
__interrupt void stop(void)
{
	switch(P1IV)
	{
		case P1IV_P1IFG0:
			break;


		case P1IV_P1IFG1:
			reset();
			timer_init();
			GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN1);
			break;


		case P1IV_P1IFG2:
			GPIO_disableInterrupt(GPIO_PORT_P1, GPIO_PIN2);
			count --;
			g1 = true;
			GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN1);
			if(count == 0)
			{
				Timer_A_stop(TIMER_A1_BASE);
				GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4);
				GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN7);
			}
			GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN2);
			break;


		case P1IV_P1IFG3:
			GPIO_disableInterrupt(GPIO_PORT_P1, GPIO_PIN3);
			count --;
			g2 = true;
			GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN2);
			if(count == 0)
			{
				Timer_A_stop(TIMER_A1_BASE);
				GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4);
				GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN7);
			}
			GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN3);
			break;


		case P1IV_P1IFG4:
			GPIO_disableInterrupt(GPIO_PORT_P1, GPIO_PIN4);
			count --;
			g3 = true;
			GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN3);
			if(count == 0)
			{
				Timer_A_stop(TIMER_A1_BASE);
				GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4);
				GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN7);
			}
			GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN4);
			break;


		case P1IV_P1IFG5:
			GPIO_disableInterrupt(GPIO_PORT_P1, GPIO_PIN5);
			count --;
			g4 = true;
			GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN4);
			if(count == 0)
			{
				Timer_A_stop(TIMER_A1_BASE);
				GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4);
				GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN7);
			}
			GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN5);
			break;


		case P1IV_P1IFG6:
			break;


		case P1IV_P1IFG7:
			break;
		default : printf("faulty interrupt");
	}

}

#pragma vector = 49
__interrupt void CCR0ISR()
{
	if(g1 == false)
	{
		GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN1);
	}
	if(g2 == false)
	{
		GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN2);
	}
	if(g3 == false)
	{
		GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN3);
	}
	if(g4 == false)
	{
		GPIO_setOutputHighOnPin(GPIO_PORT_P6,GPIO_PIN4);
	}
	Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_0);
}

#pragma vector = 48
__interrupt void CCR1ISR()
{
    switch(TA1IV)
    {

         case 0x00: break; // None
         case 0x02:
        	 if(g1 == false)
        	 GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN1);
        	 if(g2 == false)
             GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN2);
             if(g3 == false)
             GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN3);
             if(g4 == false)
             GPIO_setOutputLowOnPin(GPIO_PORT_P6,GPIO_PIN4);
              break; // CCR1 IFG
          case 0x04: break; // CCR2 IFG

          case 0x06: break; // CCR3 IFG
          case 0x08: break; // CCR4 IFG
          case 0x0A: break; // CCR5 IFG
          case 0x0C: break; // CCR6 IFG
          case 0x0E: break; // TA0IFG

          default: _never_executed();

    }
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE,TIMER_A_CAPTURECOMPARE_REGISTER_1);

}
