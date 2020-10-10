/*
 * mppt.h
 *
 *  Created on: 14-Jun-2020
 *      Author: ASUS
 */

#ifndef MPPT_H_
#define MPPT_H_

#define UPDATE_FREQ 1600u // this value has been set for ACLK with f= 32KHz
#define PERIOD 40u

void init_GPIO();
void init_Timer();
void update_duty();
void PandO();
float getPanelPower();
float getPanelVoltage();
#endif /* MPPT_H_  */
