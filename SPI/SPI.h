#include <msp430f5529.h> 
#define EPSADDRESS USCI_A0_BASE

void initGPIO();
void initSPI();
void clearInterrupts();
void clear_flags();
