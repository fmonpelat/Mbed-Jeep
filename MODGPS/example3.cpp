#ifdef COMPILE_EXAMPLE3_CODE_MODGPS

// See forum post http://mbed.org/forum/mbed/topic/2151/

#include "mbed.h"
#include "GPS.h"

Serial pc(USBTX, USBRX);
DigitalOut led1(LED1);

// SET THIS.
// Create an instance of the GPS object. You will need to
// set p25 to whichever Serial RX pin you have connected
// your GPS module to.
GPS gps(NC, GPSRX);

// 0.05 second flash of LED2
DigitalOut led2(LED2);
Timeout t2;
void t2out(void) { led2 = 0; }
void blip2(void) { led2 = 1; t2.attach(&t2out, 0.05); }

// 0.05 second flash of LED3
DigitalOut led3(LED3);
Timeout t3;
void t3out(void) { led3 = 0; }
void blip3(void) { led3 = 1; t3.attach(&t3out, 0.05); }

// 0.05 second flash of LED4
DigitalOut led4(LED4);

Timeout t4;
void t4out(void) { led4 = 0; }
void blip4(void) { led4 = 1; t4.attach(&t4out, 0.05); }

int main() {
    GPS_Time q1;
    GPS_VTG  v1;
    
    // SET THIS.
    // Ensure you set the baud rate to match your serial
    // communications to your PC/Max/Linux host so you
    // can read the messages.
    pc.baud(PCBAUD);
    
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

    // Sample of a callback to a function when a NMEA GGA message is recieved.
    // For this example, we flash LED2 for 0.05 second.
    gps.attach_gga(&blip2);
    
    // Sample of a callback to a function when a NMEA RMC message is recieved.
    // For this example, we flash LED3 for 0.05 second.
    gps.attach_rmc(&blip3);

    // Sample of a callback to a function when a NMEA VTG message is recieved.
    // For this example, we flash LED4 for 0.05 second.
    gps.attach_vtg(&blip4);
        
    while(1) {
        // Every 3 seconds, flip LED1 and print the basic GPS info.
        wait(3);
        led1 = 1;
        
        // Demonstrate how to find out the GPS location co-ords.
        pc.printf("Method 1. Lat = %.4f ", gps.latitude());
        pc.printf("Lon = %.4f ", gps.longitude());
        pc.printf("Alt = %.4f ", gps.altitude());
        
        // Grab a snapshot of the current time.
        gps.timeNow(&q1);
        pc.printf("%c %02d:%02d:%02d %02d/%02d/%04d\r\n", 
            q1.status, q1.hour, q1.minute, q1.second, q1.day, q1.month, q1.year);
        
        gps.vtg(&v1);
        pc.printf("Method 1. Vector data, Speed (knots):%lf, Speed(kph):%lf, Track(true):%lf, Track(mag)%lf\r\n", 
            v1.velocity_knots(), v1.velocity_kph(), v1.track_true(), v1.track_mag());
            
        // Alternative method that does the same thing.        
        pc.printf("Method 2. Lat = %.4f ", gps.latitude());
        pc.printf("Lon = %.4f ", gps.longitude());
        pc.printf("Alt = %.4f ", gps.altitude());
        
        GPS_Time *q2 = gps.timeNow();
        pc.printf("%c %02d:%02d:%02d %02d/%02d/%04d\r\n", 
            q2->status, q2->hour, q2->minute, q2->second, q2->day, q2->month, q2->year);
        delete(q2); 
        
        GPS_VTG *v2 = gps.vtg();
        pc.printf("Method 2. Vector data, Speed (knots):%lf, Speed(kph):%lf, Track(true):%lf, Track(mag):%lf\r\n\n", 
            v2->velocity_knots(), v2->velocity_kph(), v2->track_true(), v2->track_mag());
        delete(v2); 
         
        led1 = 0;
    }
}

#endif
