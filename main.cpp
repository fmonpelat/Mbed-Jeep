//
//  main.cpp
//  Mbed JEEP
//
//  Created by fmonpelat on 7/8/14.
//  Copyright (c) 2014 ___FMONPELAT___. All rights reserved.
//

#include "mbed.h"
#include "TextLCD.h"
#include "OneWire/DS18B20.h"
#include "OneWire/OneWireDefs.h"
#include "MODGPS/GPS.h"
#include "keypad/Keypad.h"
#include "beep/beep.h"
#include "temperature.h"
#include "alarms.h"
#include "common.h"
#include "menu.h"

#define PCBAUD 9600
#define GPSRX p14
#define GPSBAUD 9600
#define THERMOMETER DS18B20
#define JEEP_INTRO 5
#define GPS_FIX 2

DigitalOut myled(LED1);

//buzzer for alarm
Beep Buzz(p21);

// PC DEBUG
Serial PC(USBTX, USBRX);

//GPS DEF
GPS gps(NC, p14);

// I2C Communication LCD - 20x4
I2C i2c_lcd(p28,p27); // SDA, SCL
TextLCD_I2C lcd(&i2c_lcd, 0x4E, TextLCD::LCD20x4);   // I2C bus, PCF8574 Slaveaddress, LCD Type

// Temperature Controller Initialization.
//device( crcOn, useAddress, parasitic, mbed pin );
THERMOMETER WaterTemp(true, true, false, p25);

// KEYPAD DEFS
char Keytable[] = {
    'A', 'B', 'C', 'D',   // c0
    '3', '6', '9', '#',   // c1
    '2', '5', '8', '0',   // c2
    '1', '4', '7', '*',   // c3
  // r0   r1   r2   r3
 };
uint32_t Index= -1;

// keypad initialization
//             r0   r1   r2   r3   c3   c2   c1   c0
Keypad keypad( p6 , p7,  p8,  p9,  p5,  NC,  NC,  NC,30);



//######## Prototypes ############
void printIntro(void);
void ScreenLoadinggps(void);
uint32_t commandAfterInput(uint32_t index);
void init(void);

int main() {
    
    bool error=false;
    bool masterflag=false;
    int i=3;
    uint32_t row,col;
    bool gpsFixflag=true;
    bool firstExecflag=true;
    bool keypadFlagA=false;
    bool keypadFlagB=false;
    bool keypadFlagC=false;
    bool keypadFlagD=false;
    bool firstPressedA=true;
    bool firstPressedB=true;
    bool firstPressedC=true;
    bool firstPressedD=true;

    // GPS VARIABLES
    uint32_t quality;
    GPS_Geodetic GpsData;
    GPS_Time GpsTime;
    GPS_VTG GpsVector;
    double localHour;
    
    keypad.attach(&commandAfterInput);
    keypad.start();
    
    lcd.setUDC(0, (char *) udc_bar_6);
    for(row=0;row<4;row++)
    {
    	for(col=0;col<20;col++)
    	{
    	lcd.putc(0);
    	}

    }
    // init displays the logo.
    init();

    // we erase screen just in case.
    lcd.cls();

    while(true)
    {

    	switch(Index)
    	{

    		//-------------------    GPS DATA -------------------------------
    		case 0:
    				if(firstPressedA) lcd.cls();
    				lcd.setAddress(0,0);

					gps.geodetic(&GpsData);
					gps.timeNow(&GpsTime);
					gps.vtg(&GpsVector);
					lcd.setAddress(0,0);
					// test gps quality
					if(!gps.getGPSquality())
					{
						i++;
						PC.printf("fix failed: %d\n",i);
					}
					else
					{
						i=0;
						if(gpsFixflag==false){
							lcd.cls();
							lcd.setAddress(0,0);
							PC.printf("gpsFixflag is false / cls() / fix=OK\n");
						}
						gpsFixflag=true;
					}
					if( i<3 )
					{
						lcd.setAddress(0,0);
						localHour=GpsTime.hour-3;
						lcd.printf("%02d:%02d:%02d %02d/%02d/%04d\n", GpsTime.hour, GpsTime.minute, GpsTime.second, GpsTime.day, GpsTime.month, GpsTime.year);
						lcd.setAddress(0,1);
						lcd.printf("Sat:%d",gps.numOfSats());
						lcd.setAddress(0,2);
						lcd.printf("Sp:%.1fkn Cp:%.2f", GpsVector._velocity_kph ,GpsVector._track_mag);
					}
					else
					{
						PC.printf("GPS fix failed waiting for new fix\n");
						if(firstExecflag==true || gpsFixflag==false) lcd.cls();
						ScreenLoadinggps();
						while( (quality=gps.getGPSquality()) ){
							PC.printf("GPS fix 2sec wait loop / quality=%d\n",quality);
							wait(GPS_FIX);
						}
						PC.printf("GPS fix 2sec wait\n");
						gpsFixflag=false;
						firstExecflag=false;
					}
					firstPressedA=false;
					keypadFlagA=true;
			    	keypadFlagB=false;
			    	keypadFlagC=false;
			    	keypadFlagD=false;
					break;

			//---------------------------   NAVEGATION --------------------------
			case 1:
					if(firstPressedB) lcd.cls();
					lcd.setAddress(0,0);
					lcd.printf("navegation menu");
					keypadFlagA=false;
					keypadFlagB=true;
			    	keypadFlagC=false;
			    	keypadFlagD=false;
			    	firstPressedB=false;
					break;

			//--------------------------- TEMPERATURE STATS -----------------------
			case 2:
					if(firstPressedC) lcd.cls();
					lcd.setAddress(0,0);
					lcd.printf("temperature menu");
					keypadFlagA=false;
					keypadFlagB=false;
			    	keypadFlagC=true;
			    	keypadFlagD=false;
			    	firstPressedC=false;
					break;
			//--------------------------- DRIVING ---------------------------------
			case 3:
					if(firstPressedD) lcd.cls();
					lcd.setAddress(0,0);
					lcd.printf("driving menu");
					keypadFlagA=false;
					keypadFlagB=false;
			    	keypadFlagC=false;
			    	keypadFlagD=true;
			    	firstPressedD=false;
					break;

			default:
				// main menu options
				lcd.setAddress(0,1);
				lcd.printf("Press to start ...");
				break;
			}
    	keypadFlagA=false;
    	keypadFlagB=false;
    	keypadFlagC=false;
    	keypadFlagD=false;
    }

/*
            if ( error ){
            	masterAlarm(lcd,1 ,&masterflag);
            	lcd.setAddress(2,0);
            	lcd.printf("Water Temp HI");
                // function that holds up the loop until user presses a button.
            	error=tempMode(&WaterTemp,&lcd,30);
            }

*/



}

// FUNCTIONS...

// INLINE MODE FUNCTIONS

void printIntro(void){
    
    lcd.setUDC(0, (char *) udc_7);
    
    lcd.setAddress(0,0);
    lcd.printf("  (_)___ ___ ____ \n");
    
    lcd.printf("  | / -_) -_)  _ ");
    lcd.putc(0);
    lcd.printf("\n");
    
    lcd.printf(" _/ ");
    lcd.putc(0);
    lcd.printf("___");
    lcd.putc(0);
    lcd.printf("___| .__/\n");
    
    lcd.printf("|__/        |_|   \n");
    
}    
  

    
void ScreenLoadinggps(void){

        lcd.setAddress(0,1);
        lcd.printf("Loading Gps data   ");
        lcd.setAddress(0,2);
        lcd.printf("     - No Fix -  ");
        lcd.setAddress(0,1);
        lcd.printf("Loading Gps data.  ");
        wait_ms(1000);
        lcd.setAddress(0,1);
        lcd.printf("Loading Gps data.. ");
        wait_ms(1000);
        lcd.setAddress(0,1);
        lcd.printf("Loading Gps data...");
        wait_ms(1000);    
}

uint32_t commandAfterInput(uint32_t index)
{

    Index=index;
    //PC.printf("#############################\n");
    //PC.printf("Index:%d => Key:%c\n", Index, Keytable[Index]);
    return 0;
}

void init(void){

	// starting lcd backlight on initialization
	lcd.setBacklight(TextLCD::LightOn);

    //JEEP INTRO
    printIntro();
    wait(JEEP_INTRO);

}

