/*
 * functions.h
 *
 *  Created on: 17-Mar-2020
 *      Author: ASUS
 */
#include "stdint.h"
#ifndef INCLUDE_GUARD

#define INCLUDE_GUARD
uint16_t housekeeping[5][4];
uint16_t panel[2];

//Voltages
#define VOLT_EPS  housekeeping[0][0]
#define VOLT_OBC  housekeeping[1][0]
#define VOLT_TTC  housekeeping[2][0]
#define VOLT_ADCS housekeeping[3][0]
#define VOLT_PL   housekeeping[4][0]


//Currents
#define CURRENT_EPS  housekeeping[0][1]
#define CURRENT_OBC  housekeeping[1][1]
#define CURRENT_TTC  housekeeping[2][1]
#define CURRENT_ADCS housekeeping[3][1]
#define CURRENT_PL   housekeeping[4][1]

//Power
#define POW_EPS  housekeeping[0][2]
#define POW_OBC  housekeeping[1][2]
#define POW_TTC  housekeeping[2][2]
#define POW_ADCS housekeeping[3][2]
#define POW_PL   housekeeping[4][2]

//Temperature
#define TEMP_EPS  housekeeping[0][3]
#define TEMP_OBC  housekeeping[1][3]
#define TEMP_TTC  housekeeping[2][3]
#define TEMP_ADCS housekeeping[3][3]
#define TEMP_PL   housekeeping[4][3]

//Panel voltage and power
#define VOLT_PANEL panel[0]
#define POW_PANEL panel[1]

//SENSOR ADDRESSES
#define INA219_OBC           0x40//pin A1-GND, pin A0-GND
#define INA219_EPS           0x41//pin A1-GND, pin A0-Vs+
#define INA219_TTC           0x42//pin A1-GND, pin A0-SDA
#define INA219_ADCS          0x43//pin A1-GND, pin A0-SCL
#define INA219_PL            0x44//pin A1-Vs+, pin A0-GND
#define INA219_PANEL         0x45//pin A1-Vs+, pin A0-Vs+
#define LM75A_OBC            0x68
#define LM75A_EPS            0x70
#define LM75A_TTC            0x78
#define LM75A_ADCS           0x80
#define LM75A_PL             0x88

//INA219 INTERNAL REGISTER ADDRESSES
#define INA219_CONFIG    0x00
#define INA219_SHUNTV    0x01
#define INA219_BUSV      0x02
#define INA219_POWER     0x03
#define INA219_CURRENT   0x04
#define INA219_CALIB     0x05


//LM75A INTERNAL REGISTER ADDRESSES
#define LM75A_TEMP          0x00
#define LM75A_CONFIG        0x01
#define LM75A_THYST         0x02
#define LM75A_TOS           0x03
#define LM75A_PRODID        0x07





extern void initI2C();
extern void writedata(uint8_t ,uint8_t ,uint8_t *);
extern uint16_t readdata(uint8_t ,uint8_t);
extern void writeINA219(uint8_t ,uint8_t *);
extern void gethousekeeping();
extern void writeLM75A(uint8_t ,uint8_t *);
extern void readPanelVoltage();
extern void readPanelPower();
#endif /* FUNCTIONS_H_ */
