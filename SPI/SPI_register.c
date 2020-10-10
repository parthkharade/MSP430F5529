//#include <SPI.h>
#include <housekeeping.h>
#include <msp430.h>

short checkDataHalfAtReceive = 0;
short checkDataHalfAtSend = 0;
short counterParameter = 0; //will choose power, volt, temp, curent 
short counterSubsystem = 0; //will choose EPS, OBC, Payload, TTC, ADCS 
uint16_t receivedData = 0x00;

int main(void){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    initGPIO();
    initSPI();
    __enable_interrupt();//setting GIE to enable global interrupts
    return 0;
}

extern void initGPIO(){
    P3DIR |= (BIT4| ~(BIT3 | BIT2));//Pin 3.4 is SOMI(output), Pin 3.3 is SIMO(Input)
                                   // pin 3.2 is STE. EPS enabled at low state.
    P3SEL |= (BIT2 | BIT3 | BIT4);

    P2DIR &= ~(BIT7);//Pin 2.7 is configured as CLK, supplied by OBC
    P2SEL |= BIT7;

    /*
        P1.2 is set as input in with pull resistor. OBC will control it and tell us if data sent is useful or not
        1 = useful data
        0 = useless data
    */
    P1DIR &= ~BIT2;
    P1REN |= BIT2;
    P1OUT |= BIT2;

    /*
        P1.0 is set as output pin.
        1 = data sent by OBC had a bit flip
        0 = data sent by OBC did not have a bit flip
    */
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;//default condition, set high if error detected

}

extern void initSPI(){
    UCA0CTL1 |= UCSWRST; //setting UCSWRST to change bits in UCA0CTL0
    /*
        UC7BIT: 8 bits of data will be transmitted
        UCMST: EPS is operating in slave mode
        UCCKPL: Inactive state of clock is low
        UCCKPH: Data is changed on the first clock edge and captured on the following edge
        UCMODEx: Slave is enabled when STE is low
        UCSYNC: Synchronous mode on
        UCMSB: Data is being transmitted MSB first
    */
    UCA0CTL0 &= ~(UC7BIT | UCMST | UCCKPL | UCCKPH | UCMODE_2);
    UCA0CTL0 |= (UCSYNC | UCMSB);
    //SPI configured
    UCA0IE   |= (BIT0 | BIT1); //enable RXIE and TXIE interrupts
    UCA0CTL1 &= ~UCSWRST; //resetting UCSWRST to prevent changing of bits in UCA0CTL0
    clearInterrupts();
}

inline extern void clearInterrupts(){ 
     UCA0IFG &= ~(BIT0 | BIT1);//clearing TXIFG and RXIFG
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
__interrupt void exchangeData(void){
    switch(_even_in_range(UCA0IV, 0x04)){
        case 00: //No interrupt pending
            break;

        case 02:
            {uint16_t dataInBuffer;
            if((UCA0IFG & UCRXIFG) && (P1IN & BIT2)){ //if we have received the data, P1.2 is HIGH and bit flip has not ocurred
                if (checkDataHalfAtReceive == 0){
                    dataInBuffer = UCA0RXBUF;
                    dataInBuffer = dataInBuffer << 8;
                    receivedData |= dataInBuffer;
                    checkDataHalfAtReceive = 1;
                }
                if (checkDataHalfAtReceive == 1){
                    dataInBuffer = UCA0RXBUF;
                    receivedData |= dataInBuffer;
                    checkDataHalfAtReceive = 0;
                }
                //also need to add code for turning subsystems on and off. Maybe integrate with OCPC?
            }
            break;}

        case 04: //Transmit Buffer empty
            {uint8_t dataToSend = 0x000;
            if(!(P1IN & BIT2))//check if P1.2 is low
              {if (checkDataHalfAtSend == 0){
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
                    counterSubsystem++;//changing the column while keeping the row constsant
            }
            else
                counterParameter++;

            UCA0TXBUF = dataToSend;
              }break;}

        default:
            break;
    }
}

