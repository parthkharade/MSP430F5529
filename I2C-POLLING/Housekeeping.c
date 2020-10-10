#include <Housekeeping.h>
#include "stdio.h"

#include<msp430.h>
#include"stdint.h"

void main(void)
{


        WDTCTL = WDTPW | WDTHOLD;       // stop watchdog timer
        __enable_interrupt();           //enabling global interrupts
        initI2C();                      //initialize the i2c module
        printf("Hello  World\n");

         while(1)
          {
               gethousekeeping();       //Reading all housekeeping data
               __delay_cycles(2500000);
          }

}


extern void writeLM75A(uint8_t REGADD,uint8_t *data)
{
        writedata(LM75A_EPS,REGADD,data);
        writedata(LM75A_OBC,REGADD,data);
        writedata(LM75A_TTC,REGADD,data);
        writedata(LM75A_ADCS,REGADD,data);
        writedata(LM75A_PL,REGADD,data);
}



extern void writeINA219(uint8_t REGADD,uint8_t *data)
{

        writedata(INA219_EPS,REGADD,data);
        writedata(INA219_OBC,REGADD,data);
        writedata(INA219_TTC,REGADD,data);
        writedata(INA219_ADCS,REGADD,data);
        writedata(INA219_PL,REGADD,data);

}


extern void gethousekeeping()
{
        unsigned int i = 0;



        // Voltages from different subsystems
        VOLT_EPS = readdata(INA219_EPS,INA219_SHUNTV);
        VOLT_OBC = readdata(INA219_OBC,INA219_SHUNTV);
        VOLT_TTC = readdata(INA219_TTC,INA219_SHUNTV);
        VOLT_ADCS= readdata(INA219_ADCS,INA219_SHUNTV);
        VOLT_PL  = readdata(INA219_PL,INA219_SHUNTV);




        //reading temperature
        TEMP_EPS = readdata(LM75A_EPS,LM75A_TEMP);
        TEMP_OBC = readdata(LM75A_OBC,LM75A_TEMP);
        TEMP_TTC = readdata(LM75A_TTC,LM75A_TEMP);
        TEMP_ADCS= readdata(LM75A_ADCS,LM75A_TEMP);
        TEMP_PL  = readdata(LM75A_PL,LM75A_TEMP);



        // power from different subsystems
        POW_EPS = readdata(INA219_EPS,INA219_POWER);
        POW_OBC = readdata(INA219_OBC,INA219_POWER);
        POW_TTC = readdata(INA219_TTC,INA219_POWER);
        POW_ADCS= readdata(INA219_ADCS,INA219_POWER);
        POW_PL  = readdata(INA219_PL,INA219_POWER);



        //current from different subsystems
        CURRENT_EPS = readdata(INA219_EPS,INA219_CURRENT);
        CURRENT_OBC = readdata(INA219_OBC,INA219_CURRENT);
        CURRENT_TTC = readdata(INA219_TTC,INA219_CURRENT);
        CURRENT_ADCS= readdata(INA219_ADCS,INA219_CURRENT);
        CURRENT_PL  = readdata(INA219_PL,INA219_CURRENT);



        for(i =0;i<5;i++){
            printf("\nVOLTAGE:\n %u\n ",housekeeping[i][0]);
            printf("\nCURRENTS:\n %u\n ",housekeeping[i][1]);
            printf("\nPOWER:\n %u\n ",housekeeping[i][2]);
            printf("\nTEMPERATURES: \n%u\n",housekeeping[i][3]);
}
}

extern void readPanelVoltage()
{
    VOLT_PANEL=readdata(INA219_PANEL,INA219_SHUNTV);
    printf("\nPanel Voltage: \n%u\n",panel[0]);
}


extern void readPanelPower()
{
    POW_PANEL=readdata(INA219_PANEL,INA219_POWER);
    printf("\nPanel Power : \n%u\n",panel[1]);
}


