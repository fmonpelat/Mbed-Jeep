#ifdef COMPILE_EXAMPLE2_CODE_MODGPS

#include "mbed.h"
#include "GPS.h"

Serial pc(USBTX, USBRX);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

Timeout t2, t3, t4;

void t2out(void) { led2 = 0; }
void t3out(void) { led3 = 0; }
void t4out(void) { led4 = 0; }

void blip2(void) { led2 = 1; t2.attach(&t2out, 0.1); }
void blip3(void) { led3 = 1; t3.attach(&t3out, 0.1); }
void blip4(void) { led4 = 1; t4.attach(&t4out, 0.1); }

int main() {
    GPS *gps = new GPS(NC, p25);
    GPS_Time q1;
    GPS_Geodetic *geo;
    
    pc.baud(115200);
    
    gps->baud(9600);
    gps->format(8, Serial::None, 1);
    
    gps->ppsAttach(p29, GPS::ppsFall);

    gps->attach_pps(&blip2);
    gps->attach_gga(&blip3);
    gps->attach_rmc(&blip4);

    while(1) {
        // Every 3 seconds, flip LED1 and print the basic GPS info.
        wait(3);
        flip1();
        pc.printf("Method 1. Lat = %.4f ", gps->latitude());
        pc.printf("Lon = %.4f ", gps->longitude());
        pc.printf("Alt = %.4f ", gps->altitude());
        gps->timeNow(&q1);
        pc.printf("%02d:%02d:%02d %02d/%02d/%04d\r\n", 
            q1.hour, q1.minute, q1.second, q1.day, q1.month, q1.year);
            
        // Alternative method that does the same thing.
        geo = gps->geodetic();        
        pc.printf("Method 2. Lat = %.4f ", geo->lat);
        pc.printf("Lon = %.4f ", geo->lon);
        pc.printf("Alt = %.4f ", geo->alt);
        delete(geo);
        
        GPS_Time *q2 = gps->timeNow();
        pc.printf("%02d:%02d:%02d %02d/%02d/%04d\r\n\n", 
            q2->hour, q2->minute, q2->second, q2->day, q2->month, q2->year);
        delete(q2);        
    }
}

#endif
