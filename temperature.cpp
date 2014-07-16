//
//  temperatures.c
//  Mbed JEEP
//
//  Created by fmonpelat on 7/8/14.
//  Copyright (c) 2014 ___FMONPELAT___. All rights reserved.
//
#include "mbed.h"
#include "TextLCD.h"
#include "OneWire/DS18B20.h"
#include "OneWire/OneWireDefs.h"
#include "temperature.h"



bool tempMode(DS18B20 *device,TextLCD_I2C *lcd,float max_temp){

        float temp;

        while (!(*device).initialize());    // keep calling until it works
        (*device).setResolution(twelveBit);

            temp=(*device).readTemperature();
            //lcd.cls();
            (*lcd).setAddress(0,2);
            (*lcd).printf("H2O Temp: %.1f",temp);
            (*lcd).putc(223);
            wait(0.2);
            if(temp>max_temp){
                return true;
            }
            return false;
}


bool getTemp(DS18B20 *device,float maxThreshold,float *temp){


        while (!(*device).initialize());    // keep calling until it works
        (*device).setResolution(twelveBit);

            *temp=(*device).readTemperature();

            if(*temp>maxThreshold){
                return true;
            }
            return false;
}
