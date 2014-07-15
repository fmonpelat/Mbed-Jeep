#ifdef COMPILE_EXAMPLE1_CODE_MODGPS

#include "mbed.h"
#include "GPS.h"

Serial pc(USBTX, USBRX);
DigitalOut led1(LED1);

// SET THIS.
// Create an instance of the GPS object. You will need to
// set p25 to whichever Serial RX pin you have connected
// your GPS module to.
GPS gps(NC, GPSRX);

char rmc[GPS_BUFFER_LEN];
char gga[GPS_BUFFER_LEN];
char vtg[GPS_BUFFER_LEN];
char ukn[GPS_BUFFER_LEN];

// 0.1 second flash of LED2
DigitalOut led2(LED2);
Timeout t2;
void t2out(void) { led2 = 0; }
void blip2(void) { led2 = 1; t2.attach(&t2out, 0.1); }

// 0.1 second flash of LED3
DigitalOut led3(LED3);
Timeout t3;
void t3out(void) { led3 = 0; }
void blip3(void) { led3 = 1; t3.attach(&t3out, 0.1); }

// 0.1 second flash of LED4
DigitalOut led4(LED4);

Timeout t4;
void t4out(void) { led4 = 0; }
void blip4(void) { led4 = 1; t4.attach(&t4out, 0.1); }

int main() {
    GPS_Time q1;
    
    // SET THIS.
    // Ensure you set the baud rate to match your serial
    // communications to your PC/Max/Linux host so you
    // can read the messages.
    pc.baud(PCBAUD);
    
    // Tell MODGPS "we want copies of the NMEA sentences". When a callback
    // is made our buffers will contain a copy of the last received sentence
    // before it was processed/destroyed.    
    gps.setRmc(rmc);
    gps.setGga(gga);
    gps.setVtg(vtg);
    gps.setUkn(ukn);
    
    // SET THIS.
    // Most GPS modules use 9600,8,n,1 so that's what
    // we default to here. Ensure your GPS module matches
    // this, otherwise set it to match.
    gps.baud(GPSBUAD);
    gps.format(8, GPS::None, 1);
    
    // OPTIONAL
    // If you GPS has a 1 pulse per second output you can
    // connect it to an Mbed pin. Here you specify what pin
    // and on what "edge" teh signal is active. If your GPS
    // module has a rising edge at the one second point then
    // use GPS::ppsRise
    #ifdef PPSPIN
    gps.ppsAttach(PPSPIN, GPS::ppsFall);
    #endif

    // Sample of a callback to a function when the 1PPS activates.
    // For this example, we flash LED2 for 0.1 second.
    gps.attach_pps(&blip2);
    
    // Sample of a callback to a function when a NMEA GGA message is recieved.
    // For this example, we flash LED2 for 0.1 second.
    gps.attach_gga(&blip3);
    
    // Sample of a callback to a function when a NMEA RMC message is recieved.
    // For this example, we flash LED2 for 0.1 second.
    gps.attach_rmc(&blip4);

    while(1) {
        // Every 3 seconds, flip LED1 and print the basic GPS info.
        wait(3);
        led1 = 1;
        
        // Demonstrate how to find out the GPS location co-ords.
        pc.printf("Method 1. Lat = %.4f ", gps.latitude());
        pc.printf("Lon = %.4f ", gps.longitude());
        pc.printf("Alt = %.4f ", gps.altitude());
        
        // Gran a snapshot of the current time.
        gps.timeNow(&q1);
        pc.printf("%02d:%02d:%02d %02d/%02d/%04d\r\n", 
            q1.hour, q1.minute, q1.second, q1.day, q1.month, q1.year);
            
        // Alternative method that does the same thing.        
        pc.printf("Method 2. Lat = %.4f ", gps.latitude());
        pc.printf("Lon = %.4f ", gps.longitude());
        pc.printf("Alt = %.4f ", gps.altitude());
        
        GPS_Time *q2 = gps.timeNow();
        pc.printf("%02d:%02d:%02d %02d/%02d/%04d\r\n\n", 
            q2->hour, q2->minute, q2->second, q2->day, q2->month, q2->year);
        delete(q2);        
        led1 = 0;
    }
}

#endif
