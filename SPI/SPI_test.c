#include <msp430.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
volatile uint8_t receivedData;
bool print = false;
int main(void){
    WDTCTL = WDTPW | WDTHOLD;
    __enable_interrupt();
   // stop watchdog timer
    initGPIO();
    initSPI();

   //setting GIE to enable global interrupts
    while(1)
    {
        if(print == true)
        {
            print = false;
            printf("Data : %u\n", receivedData);
        }
    }
}
void initGPIO(){
    P3DIR |= (BIT4);
    P3DIR &= ~(BIT3 | BIT2);
    //Pin 3.4 is SOMI(output), Pin 3.3 is SIMO(Input)
    //Pin 3.2 is STE. EPS enabled at low state.
    P3SEL |= (BIT2 | BIT3 | BIT4);
    P2DIR &= ~(BIT7);
    P2SEL |= BIT7;
    //Pin 2.7 is configured as CLK, supplied by OBC
}
void initSPI(){
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
    UCA0CTL0 &= ~(UC7BIT | UCMST | UCCKPL);
    UCA0CTL0 |= (UCSYNC | UCMSB | UCMODE_2 | UCCKPH);
//    clear_flags();

    //enable RXIE and TXIE interrupts
    //SPI configured
    UCA0CTL1 &= ~UCSWRST; //SPI enabled
    UCA0IE   |= (BIT0+BIT1);
    UCA0TXBUF = 0b10101010;

    }
void clear_flags(){
     UCA0IFG &= ~(BIT0 | BIT1);//clearing TXIFG and RXIFG
}
#pragma vector = USCI_A0_VECTOR
__interrupt void exchangeData(void){
    switch(_even_in_range(UCA0IV, 0x04)){
        case 0x00: //No interrupt pending
            break;
        case 0x02:
            receivedData = UCA0RXBUF;
            print = true;

            break;
        case 0x04: //Transmit Buffer empty
            UCA0TXBUF = 0xA6;
            break;

        default:
            break;
    }
}
