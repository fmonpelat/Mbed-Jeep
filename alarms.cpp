/*
 * alarms.cpp
 *
 *  Created on: Jul 14, 2014
 *      Author: fmonpelat
 */

#include "mbed.h"
#include "TextLCD.h"
#include "OneWire/DS18B20.h"
#include "OneWire/OneWireDefs.h"
#include "common.h"


void masterAlarm(TextLCD_I2C &lcd,int holdBack ,bool *flag){

	if(*flag==false) lcd.cls();
    lcd.setAddress(4,1);
    lcd.printf("MASTER ALARM.");
    Buzz.beep(1000,0.5);
    *flag=true;


}

