#include <msp430f5529.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
/**
 * main.c
 */

#define LM75A  0x48
#define INA219 0x40



#define NO_SENSORS          4u
#define HEADER_SIZE         1u
#define BLOCK_SIZE          ((NO_SENSORS)*2+(HEADER_SIZE))
#define NUMBER_OF_BLOCKS    100u
#define ARRAY_SIZE          ((BLOCK_SIZE)*(NUMBER_OF_BLOCKS))
#define INVALID             0x49 // 'I'
#define VALID               0x56 // 'V'



volatile uint8_t block=0;
volatile uint8_t rdata[ARRAY_SIZE];
volatile uint8_t count =0;
volatile uint8_t sensor =0;
uint8_t Address[NO_SENSORS] =  {INA219,LM75A,INA219,LM75A};
void initI2C();
void inittimer();
void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    __enable_interrupt();
    initI2C();
    inittimer();

}
void inittimer()
{
    TA0CTL  &=  0x00;
    TA0CTL  |=  TASSEL__ACLK|ID__1|MC__STOP|TAIE;
    TA0CTL  |=  MC__CONTINOUS;
}
void initI2C()
{
//*************Initializing the GPIO Pins****************

        P3SEL    |= (BIT0|BIT1);//Setting 3.0 and 3.1 as peripheral module functions
//*************Initializing the I2C Module***************
        UCB0CTL1 |= UCSWRST;                    // Software Reset Enable
        UCB0CTL0 |= UCSYNC + UCMODE_3 + UCMST;  // Synchronous Mode , I2C Mode , Master Mode
        UCB0CTL1 |= UCSSEL__SMCLK;              // Use SMCLk, Set as Receiver.
        UCB0BRW   = 0x00A0;                     // Setting the clock pre-scalar = Input Clock Frequncy/I2C Rate = 100Hz

        UCB0CTL1 &= ~(UCSWRST);                 // Releasing Software Reset
        UCB0IE   |= UCNACKIE|UCRXIE|UCTXIE;
        UCB0I2CSA|= Address[0];
}
#pragma vector =TIMER0_A1_VECTOR
__interrupt void TimerHandler()
{
    switch(__even_in_range(TA0IV, 0x0E))
    {
    case TA0IV_TAIFG:

        if(block < NUMBER_OF_BLOCKS)
        {
        rdata[(block*(BLOCK_SIZE))] = INVALID;
        UCB0I2CSA = Address[sensor];
        UCB0CTL1 |= UCTXSTT;
        }
        break;

    }
}

#pragma vector = USCI_B0_VECTOR
__interrupt void I2CHandler()
{
    switch (__even_in_range(UCB0IV,0x0C)) {
        case USCI_NONE :

            break;
        case USCI_I2C_UCALIFG:

            break;
        case USCI_I2C_UCNACKIFG:

                rdata[(block*(BLOCK_SIZE))+HEADER_SIZE+sensor*2] = 0xFF;
                rdata[(block*(BLOCK_SIZE))+HEADER_SIZE+sensor*2+1] = 0xFF;
                sensor ++;
                if(sensor<NO_SENSORS)
                {

                    count = 0;
                    UCB0I2CSA = Address[sensor];
                    UCB0CTL1 |= UCTXSTT;
                }
                else
                {
                    rdata[(block*(BLOCK_SIZE))] = VALID;
                    block ++;
                    sensor =0;
                    count =0;
                }
            break;
        case USCI_I2C_UCSTTIFG:

            break;
        case USCI_I2C_UCSTPIFG:

            break;
        case USCI_I2C_UCRXIFG:
                rdata[(block*(BLOCK_SIZE))+HEADER_SIZE+sensor*2+count] = UCB0RXBUF;
                if(count == 0)
                {
                    if(sensor<(NO_SENSORS-1))
                    {
                        UCB0I2CSA = Address[sensor+1];
                        UCB0CTL1 |= UCTXSTT;
                    }
                    else
                    {
                        UCB0CTL1 |= UCTXSTP;
                    }
                    count ++;
                }
                else
                {
                    if(sensor < (NO_SENSORS-1))
                    {
                        sensor ++;
                    }
                    else
                    {
                        rdata[(block*(BLOCK_SIZE))] = VALID;
                        sensor =0;
                        block++;
                    }
                    count = 0;
                }

            break;
        case USCI_I2C_UCTXIFG:

            break;
        default:

            break;
    }
}
