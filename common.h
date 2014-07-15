/*
 * common.h
 *
 *  Created on: Jul 14, 2014
 *      Author: fmonpelat
 */

#ifndef COMMON_H_
#define COMMON_H_

#include "MODGPS/GPS.h"
#include "beep/beep.h"

extern Beep Buzz;
extern Serial PC;
extern GPS gps;
extern I2C i2c_lcd;
extern TextLCD_I2C lcd;
extern DS18B20 WaterTemp;



#endif /* COMMON_H_ */
