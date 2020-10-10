/* This code is written for "Burn Wire" Antenna Deployment mechanism. The door is held by 4 Nylon wires and presses down a switch.
  The wires are wrapped around resitors which disipate enough heat to melt the wires and open the door.
  As the door opens, the switch is released, which produces an interrupt to signal the end of the mechanism.
  Pins Used:
  - Output Pins: 6.1, 6.2, 6.3, 6.4 (connected to the wire resistors)
  - Input Pins: 1.1, 1.2, 1.3, 1.4, 1.5 (Provide interrupts to signal the end of the process)
  - Pin 4.7 (LED): Switched on when all the wires have melted

Output pins are connected to P-MOSFETS, so low output = current in resitors

Timer: Using TimerA
A timer runs in the background to prevent the resitors from burning out completely, in case we don't receive all the interrupt signals on the pins.
If we have not received any interrupt from a pin when the timer times out, output on the respective pin is turned low for some time.
The output is again turned high, when the timer times to the compare value.

     0 -  1000: Output High (No current)
     1000 - 3000: Output Low (High Current)

Code Logic:
4 flags are used to track the status of 4 Nylon wires controlling the door. After an interrupt is received on the pin the respective flag is updated.
Once all the interrupts are received the timer is disabled and LED is switched ON.

*/

#include <msp430f5529.h>
#include "stdio.h"
#include "stdbool.h"  //to use boolean declarations
#include "stdint.h"   //to use declarations like uint8_t and uint32_t

volatile bool g1,g2,g3,g4;
uint8_t count =4;

/*
 * main.c
 */
void gpio_init();
void timer_init();
int main(void)
{
    g1= false;
    g2= false;
    g3= false;
    g4= false;

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
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
    P4DIR =   BIT7;                     // Setting the LED as the output pin
    P6DIR =  (BIT1+BIT2+BIT3+BIT4);     // setting 6.1-6.4 as output pins for controlling current
    P1DIR = ~(BIT1+BIT2+BIT3+BIT4+BIT5);// Setting 1.1-1.5 as input pins
    P1REN =  (BIT1+BIT2+BIT3+BIT4+BIT5);// Enabaling pull up/down resistor
    P1OUT = ~(BIT1+BIT2+BIT3+BIT4+BIT5);// Setting as input using pull down resistor

    //configuring gpio interrupts
    P1IE = (BIT1+BIT2+BIT3+BIT4+BIT5);
    P1IES = (BIT1+BIT2+BIT3+BIT4+BIT5);
    P1IFG = 0x0000;

    //setting initial values
    P4OUT = 0x0000;
    P6OUT - 0x0000;

}
void timer_init()
{
    TA1CTL = 0x0000;// removing any previous setting

    TA1CCTL1 = 0x0000;// removing any previous setting
  /*
   Setting for TA1CTL register:
    - TASSEL__ACLK: selects ACLK as clock source
    - TACLR: clears timer counter
    - TIMER_A_TAIE_INTERRUPT_DISABLE: disables TAIFG interrupt
    - TIMER_A_CLOCKSOURCE_DIVIDER_32: /32  clockdivider
  */
    TA1EX0 = (0x17 & 0x0007);//  TIMER_A_CLOCKSOURCE_DIVIDER_32 = 0x17

    TA1CTL |= (TASSEL__ACLK + TACLR + 0x000 + ((0x17 >> 3)<<8)); //TIMER_A_TAIE_INTERRUPT_DISABLE = 0x00

    TA1CCR0 = 0xBB8; // Timer Period = 3000
  /*
   Setting for Compare mode:
    - ~CAP: Compare mode set
    - TIMER_A_OUTPUTMODE_OUTBITVALUE: 0 output on compare
    - TIMER_A_CAPTURECOMPARE_REGISTER_1: CCR1 selected
    - CCIE: enabling Capture Compare interrupt
    */

    TA1CCTL1 &= ~(CAP);

    TA1CCTL1 |=(OUTMOD_0 + 0x04 + CCIE); //TIMER_A_CAPTURECOMPARE_REGISTER_1 = 0x04

    TA1CCR1 = 0x3E8;// Compare value = 1000

    TA1CTL &= ~(TAIFG);// clearing CCR0IFG

    TA1CCTL1 &= ~(CCIFG);//clearing CCR1IFG

    TA1CTL |= MC_1;// starts timer in up mode

}
void reset()
{

    count =4;
    g1= false;
    g2= false;
    g3= false;
    g4= false;
    P4OUT = 0x0000;
    P1IE = (BIT1+BIT2+BIT3+BIT4+BIT5);
    P1IFG = 0x0000;// clearing all  GPIO interrupt flags
    TA1CTL &= ~(TAIFG);// clearing CCR0IFG
    TA1CCTL1 &= ~(CCIFG);// clearing CCR1IFG

    TA1CTL |= MC_1;// starts timer in up mode

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
            P1IFG &= ~(BIT1);
            break;


        case P1IV_P1IFG2:
            P1IE &= ~(BIT2);
            count --;
            g1 = true;
            P6OUT |= BIT1;
            if(count == 0)
            {
                TA1CTL |= MC_0;
                P6OUT |= (BIT1 + BIT2 + BIT3 + BIT4);
                P4OUT |= BIT7;
            }
            P1IFG &= ~(BIT2);
            break;


        case P1IV_P1IFG3:
            P1IE &= ~(BIT3);
            count --;
            g2 = true;
            P6OUT |= BIT2;
            if(count == 0)
            {
                TA1CTL |= MC_0;
                P6OUT |= (BIT1 + BIT2 + BIT3 + BIT4);
                P4OUT |= BIT7;
            }
            P1IFG &= ~(BIT3);
            break;


        case P1IV_P1IFG4:
            P1IE &= ~(BIT4);
            count --;
            g3 = true;
            P6OUT |= BIT3;
            if(count == 0)
            {
                TA1CTL |= MC_0;
                P6OUT |= (BIT1 + BIT2 + BIT3 + BIT4);
                P4OUT |= BIT7;
            }
            P1IFG &= ~(BIT4);
            break;


        case P1IV_P1IFG5:
            P1IE &= ~(BIT5);
            count --;
            g4 = true;
            P6OUT |= BIT4;
            if(count == 0)
            {
                TA1CTL |= MC_0;
                P6OUT |= (BIT1 + BIT2 + BIT3 + BIT4);
                P4OUT |= BIT7;
            }
            P1IFG &= ~(BIT5);
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
        P6OUT |= BIT1;
    }
    if(g2 == false)
    {
        P6OUT |= BIT2;
    }
    if(g3 == false)
    {
        P6OUT |= BIT3;
    }
    if(g4 == false)
    {

        P6OUT |= BIT4;
    }
  TA1CTL &= ~(TAIFG);
}

#pragma vector = 48
__interrupt void CCR1ISR()
{
    switch(TA1IV)
    {

         case 0x00: break; // None
         case 0x02:
             if(g1 == false)
             P6OUT &= ~(BIT1);
             if(g2 == false)
             P6OUT &= ~(BIT2);
             if(g3 == false)
             P6OUT &= ~(BIT3);
             if(g4 == false)
             P6OUT &= ~(BIT4);
              break; // CCR1 IFG
          case 0x04: break; // CCR2 IFG

          case 0x06: break; // CCR3 IFG
          case 0x08: break; // CCR4 IFG
          case 0x0A: break; // CCR5 IFG
          case 0x0C: break; // CCR6 IFG
          case 0x0E: break; // TA0IFG

          default: _never_executed();

    }
    TA1CCTL1 &= ~(CCIFG);

}

