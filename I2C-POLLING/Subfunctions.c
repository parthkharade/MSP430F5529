#include <Housekeeping.h>
//Base Level I2C Functions
// Note:  having a seperate file for this is not needed. This file can be merged with the housekeeping.c file if required.

bool fail = false;

extern void initI2C()
{
//*************Initializing the GPIO Pins****************

	    P3SEL 	 |= (BIT0|BIT1);//Setting 3.0 and 3.1 as peripheral module functions
//*************Initializing the I2C Module***************
	    UCB0CTL1 |= UCSWRST; 					// Software Reset Enable
	    UCB0CTL0 |= UCSYNC + UCMODE_3 + UCMST;	// Synchronous Mode , I2C Mode , Master Mode
	    UCB0CTL1 |= UCSSEL__SMCLK;				// Use SMCLk, Set as Receiver.
	    UCB0BRW	  = 0x00A0;						// Setting the clock pre-scalar = Input Clock Frequncy/I2C Rate = 100Hz

	    UCB0CTL1 &= ~(UCSWRST);					// Releasing Software Reset
	    UCB0IE	 |= UCNACKIE;
}


extern bool writedata(uint8_t SA,uint8_t REGADD,uint8_t *data)
{

 //1. The USCI module is configured as an I2C master by selecting the I2C mode with UCMODEx = 11 and
 //UCSYNC = 1 and setting the UCMST bit
 //2. Write desired slave address to UCBxI2CSA selecting size bt UCSLA10, UCTR for transmitting mode, UCTXSTT for gen of start con.
 //3. first data to be transmitted can be written into UCBxTXBUF. As soon as the slave acknowledges the address, the
 //UCTXSTT bit is cleared.

//*************Setting Transmit Mode********************

    UCB0CTL1 |= UCTR;                               			//Transmit Mode

//*************Setting the slave and register addresses*******
    UCB0I2CSA = SA;                                           	//Slave Address
    UCB0CTL1 |= UCTXSTT ;

//*******************Data Transmission************************

    //*******************Bit 1************************
    while(!((UCB0IFG & UCTXIFG)|(fail)));						// Wait for TX buffer to get empty or for NACK to be set
     if(fail)
      {

      	 return false;												//return read value
      }

    UCB0TXBUF = REGADD;											//Sending the Register Address

    	//*******************Bit 2************************
    while(!((UCB0IFG & UCTXIFG)|(fail)));						// Wait for TX buffer to get empty or for NACK to be set
     if(fail)
      {

      	 return false;												//return read value
      }
    UCB0TXBUF = *(data);										//MSB

    	//*******************Bit 3************************
    while(!((UCB0IFG & UCTXIFG)|(fail)));						// Wait for TX buffer to get empty or for NACK to be set
     if(fail)
      {

      	 return false;												//return read value
      }
    UCB0TXBUF = *(data + 1);                                  	//LSB

    	//*******************Stop************************
    UCB0CTL1 |= UCTXSTP;										// Send Stop
    return true;

}


extern READINFO readdata(uint8_t SA,uint8_t REGADD)
{
	READINFO read;

	volatile int16_t result1 = 0;
	volatile int16_t result2 = 0;

    UCB0CTL1 |= UCTR;                                         	//Transmit Mode

    UCB0I2CSA = SA;                                           	//Slave Address Set

//*******************Data Transmission : Set Pointer Address to be read from ************************
    UCB0CTL1 |= UCTXSTT;                                      	//Start Condition(High to Low on SDA when SCL is high)

    while(!((UCB0IFG & UCTXIFG)|(fail)));						// Wait for TX buffer to get empty or for NACK to be set
   if(fail)
    {
    	 read.result = 0;
    	 read.readstatus = false;
    	 return read;												//return read value
    }
    UCB0TXBUF = REGADD;                                       	//Register Address


    while(!((UCB0IFG & UCTXIFG)|(fail)));						// Wait for TX buffer to get empty or for NACK to be set
     if(fail)
     {
      	 read.result = 0;
      	 read.readstatus = false;
      	 return read;												//return read value
     }
//*******************Data Reception   : Read from set pointer address ************************
    UCB0CTL1 &= ~(UCTR);                                      	//Sets Receive mode

    UCB0CTL1 |= UCTXSTT;                                      	//Again start Condition, Repeated Start condition with read condition now

    while(!((UCB0IFG & UCRXIFG)|(fail)));							//Wait for RX buffer to get filled or NACK.
    if(fail)
      {
       	 read.result = 0;
       	 read.readstatus = false;
       	 return read;												//return read value
      }
    result1 = UCB0RXBUF;                                      	//Reading MSB


    while(!((UCB0IFG & UCRXIFG)|(fail)));	                             	//Waiting for RX buffer to get filled
    if(fail)
     {
       	 read.result = 0;
       	 read.readstatus = false;
       	 return read;												//return read value
      }

    UCB0CTL1 |= UCTXSTP;										// Send Stop
    result2 = UCB0RXBUF;                                      	//Reading LSB



    read.result = (result2<<8)|(result1);
    read.readstatus = true;
    return read;												//return read value


}


#pragma vector = USCI_B0_VECTOR
 __interrupt void NACK()
 {

	 switch(__even_in_range(UCB0IV,0x0C))
	 {
	 case 0x04:
	 fail = true;
	 break;
	 default:
		 printf("Default\n");
		 break;
	 }
 }
