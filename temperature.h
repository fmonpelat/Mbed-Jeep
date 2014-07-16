/*
 * temperature.h
 *
 *  Created on: Jul 14, 2014
 *      Author: fmonpelat
 */



#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_


//Prototypes

bool tempMode(DS18B20 *,TextLCD_I2C *,float);//deprecated
bool getTemp(DS18B20 *,float ,float *);




#endif /* TEMPERATURE_H_ */
