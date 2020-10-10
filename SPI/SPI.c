/* 
    3 GPIO pins are being used:
    1. Signaling housekeeping update (Pin )
    2. Signaling error in received data (P1.1) 
    3. To check if OBC is sending useful data (P1.2)
    The update pin will be configured in the housekeeping code.
    The data received from the OBC will be a 8 bit value where one bit is kept as a parity bit, and other bits 
    will denote  subsystems. 
*/

#include <housekeeping.h>
#include <SPI.h>
#include <driverlib.h>

int checkDataHalfAtReceive = 0;
int checkDataHalfAtSend = 0;
int counterParameter = 0;//will choose power, volt, temp, curent
int counterSubsystem = 0;//will choose EPS, OBC, Payload, TTC, ADCS
uint8_t receivedData = 0x000;//data from the OBC 

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    initGPIO();
    initSPI();
    __enable_interrupt();//setting GIE to enable global interrupts
    return 0;
}

extern void initGPIO()
{
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P3, GPIO_PIN4);//SOMI is from where we would send data
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN3);//SIMO is where we would receive data
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN2);//STE controlled by OBC
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2, GPIO_PIN7);//CLK given by OBC

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN2);//Used to signal if OBC is sending useful data or not
    //GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN2, GPIO_HIGH_TO_LOW_TRANSITION);

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN1);//Used to indicate validity of received data after checking parity bit
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN1);//default condition, set high if error detected

}

extern void initSPI(){
    bool initSlaveSuccess = STATUS_SUCCESS;
    while(!initSlaveSuccess)
    {
        initSlaveSuccess = USCI_A_SPI_initSlave(USCI_A1_BASE, USCI_A_SPI_MSB_FIRST,USCI_A_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT,USCI_A_SPI_CLOCKPOLARITY_INACTIVITY_LOW);
    }
    USCI_A_SPI_disableInterrupt(USCI_A0_BASE, USCI_A_SPI_RECEIVE_INTERRUPT);
    USCI_A_SPI_enableInterrupt(USCI_A0_BASE, USCI_A_SPI_TRANSMIT_INTERRUPT);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN2);
    clearInterrupts();
}

extern void clearInterrupts()
{
    USCI_A_SPI_clearInterrupt(USCI_A0_BASE, USCI_A_SPI_TRANSMIT_INTERRUPT);
    USCI_A_SPI_clearInterrupt(USCI_A0_BASE, USCI_A_SPI_RECEIVE_INTERRUPT);
    //GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN2);
}

extern void isBitFlipped(uint8_t dataToCheck){//would be run from the main code by the scheduler
    uint8_t noOfSetBits = 0;
    short bits;
    for(bits = 1; bits <= 8; bits++)
    {
        if(dataToCheck & BIT0)//would depend on how many bits is data from OBC
        {
            noOfSetBits++;
        }
        dataToCheck >>= 1;//would depend on how many bits is data from OBC
    }

    if(noOfSetBits % 2)//assuming odd parity
        P1OUT |= BIT0; //P1.0 HIGH

    else
        P1OUT &= ~BIT0;//P1.0 LOW
}

extern void changeModeOfOperation(uint8_t modeData){
    short bits;
    modeData >>= 1; //to get rid of parity bit
    for(bits = 1; bits <= 7; bits++){ //7 because the last bit would be the parity bit. Also this number would probably change
        short subsystemState = modeData & BIT8;
        if(subsystemState)
            //turn subsystem on, pass on bits in the function call
            //possibly add a success/failure array here
            modeData >>= 1;
        else
        modeData >>= 1;
    }
}

#pragma vector = USCI_A0_VECTOR
__interrupt void exchangeData(void)
{
    switch(_even_in_range(UCA0IV, 0x04)){
        case 00: //No interrupt pending
            break;

        case 02:
            {uint16_t dataInBuffer; 
             if((UCA0IFG & UCRXIFG) && (!(P1IN & BIT2)))//if we have received the data and if pin 1.2 is low
            {
                dataInBuffer = USCI_A_SPI_receiveData(USCI_A0_BASE);
                
                else
                {
                    if(P1OUT & BIT0)
                    {
                        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                    }

                    if(receivedData & BIT6)
                    {
                         //code for turning on/off subsystem-1
                    }

                    if(receivedData & BIT7)
                    {
                        //code for turning on/off subsystem-2
                    }
                }
            }
            break;

        case 04: //Transmit Buffer empty
         {
            uint8_t dataToSend = 0x00;
            if (checkDataHalfAtSend == 0){
                dataToSend |= housekeeping[counterSubsystem][counterParameter] >> 8;
                checkDataHalfAtSend = 1;
            }
            else{
                dataToSend |= housekeeping[counterSubsystem][counterParameter];
                checkDataHalfAtSend = 0;
            }

            if(counterParameter == 3){//transmitting all power data first, then all current data and so on
                counterParameter = 0;
                if(counterSubsystem == 4)
                    counterSubsystem = 0;
                else
                    counterSubsystem++;//changing the column while keeping the row constant
            }
            else
                counterParameter++;

            USCI_A_SPI_transmitData(USCI_A0_BASE,dataToSend);
            break;
        }

        default:
            break;
    }
}


