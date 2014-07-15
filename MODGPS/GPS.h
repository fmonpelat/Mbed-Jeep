/*
    Copyright (c) 2010 Andy Kirkham
 
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
 
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
 
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#ifndef GPS_H
#define GPS_H

#include "mbed.h"
#include "GPS_VTG.h"
#include "GPS_Time.h"
#include "GPS_Geodetic.h"

#define GPS_RBR  0x00
#define GPS_THR  0x00
#define GPS_DLL  0x00
#define GPS_IER  0x04
#define GPS_DML  0x04
#define GPS_IIR  0x08
#define GPS_FCR  0x08
#define GPS_LCR  0x0C
#define GPS_LSR  0x14
#define GPS_SCR  0x1C
#define GPS_ACR  0x20
#define GPS_ICR  0x24
#define GPS_FDR  0x28
#define GPS_TER  0x30

#define GPS_BUFFER_LEN  128
#define GPS_TICKTOCK    10000

/** @defgroup API The MODGPS API */

/** GPS module
 * @author Andy Kirkham
 * @see http://mbed.org/cookbook/MODGPS
 * @see example1.cpp
 * @see example2.cpp
 * @see API 
 *
 * @image html /media/uploads/AjK/gps_interfaces.png "Wiring up the GPS module"
 *
 * Example:
 * @code
 * #include "mbed.h"
 * #include "GPS.h"
 *
 * DigitalOut led1(LED1);
 * Serial pc(USBTX, USBRX);
 * GPS gps(NC, p10); 
 *
 * int main() {
 *     GPS_Time t;
 *
 *     // Wait for the GPS NMEA data to become valid.
 *     while (!gps.isTimeValid()) {
 *       led1 = !led1;
 *       wait(1);
 *     }
 *
 *     gps.timeNow(&t);
 *
 *     pc.printf("The time/date is %02d:%02d:%02d %02d/%02d/%04d\r\n",
 *        t.hour, t.minute, t.second, t.day, t.month, t.year);
 *
 *     // Wait until at least four satellites produce a position fix and a valid quality.
 *     while (gps.numOfSats() < 4 && gps.getGPSquality != 0) {
 *       led1 = !led1;
 *       wait(1);
 *     }
 *
 *     pc.printf("Lat = %.4f Lon = %.4f Alt = %.1fkm\r\n", 
 *         gps.latitude(), gps.longitude, gps.altitude());
 *
 *     // Make the LED go steady to indicate we have finished.
 *     led1 = 1;
 * 
 *     while(1) {}
 * }
 * @endcode
 */

class GPS : Serial {
public:
    
    //! The PPS edge type to interrupt on.
    enum ppsEdgeType { 
        ppsRise = 0,    /*!< Use the rising edge (default). */
        ppsFall         /*!< Use the falling edge. */
    };
    
    //! A copy of the Serial parity enum
    enum Parity {
        None = 0
        , Odd
        , Even
        , Forced1   
        , Forced0
    };
    
    //! GPS constructor.
    /**
     * The GPS constructor is used to initialise the GPS object.
     *
     * @param tx Usually unused and set to NC
     * @param rx The RX pin the GPS is connected to, p10, p14( OR p25), p27.
     * @param name An option name for RPC usage.
     */
    GPS(PinName tx, PinName rx, const char *name = NULL);

    //! Is the time reported by the GPS valid.
    /**
     * Method used to check the validity of the time the GPS module is reporting.
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     if (gps.isTimeValid()) {
     *         // Time is valid :)
     *     }
     *     else {
     *         // Doh, time is not valid :(
     *     )
     *     
     * @endcode
     *
     * @ingroup API
     * @return bool true if valid, false otherwise
     */
    bool isTimeValid(void) { return theTime.status == 'V' ? false : true; }
    
    //! Is the positional fix reported by the GPS valid.
    /**
     * Method used to check the validity of the positional data. This method
     * returns the GGA field, 0 is "bad, 1 is "ok", etc. See the NMEA GGA 
     * description for more details.
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     if (gps.getGPSquality() == 0) {
     *         // The location fix is no good/not accurate :(
     *     }
     *     else {
     *         // All good, can use last fix data.
     *     )
     *     
     * @endcode
     *
     * @ingroup API
     * @return int 0 on no fix, 1... (see NMEA GGA for more details).
     */
    int getGPSquality(void) { return thePlace.getGPSquality(); }
    
    //! How many satellites were used in the last fix.
    /**
     * Method returns the number of GPS satellites used on the last fix.
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     int sats = gps.numOfSats();
     *     
     * @endcode
     *
     * @ingroup API
     * @return int The number of satellites.
     */
    int numOfSats(void) { return thePlace.numOfSats(); }
    
    //! What was the last reported latitude (in degrees)
    /**
     * Method returns a double in degrees, positive being North, negative being South.
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     double latitude = gps.latitude();
     *     
     * @endcode
     *
     * @ingroup API
     * @return double Degrees
     */
    double latitude(void);
    
    //! What was the last reported longitude (in degrees)
    /**
     * Method returns a double in degrees, positive being East, negative being West.
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     double logitude = gps.logitude();
     *     
     * @endcode
     *
     * @ingroup API
     * @return double Degrees
     */
    double longitude(void);
    
    //! What was the last reported altitude (in kilometers)
    /**
     * Method returns a double in kilometers.
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     double altitude = gps.altitude();
     *     
     * @endcode
     *
     * @ingroup API
     * @return double Kilometers
     */
    double altitude(void);
    
    //! What was the last reported altitude/height (in kilometers)
    /**
     * @see altitude()
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     double height = gps.height();
     *     
     * @endcode
     *
     * Note, this is identical to altitude()
     * @see altitude()
     *
     * @ingroup API
     * @return double Kilometers
     */
    double height(void) { return altitude(); }
    
    //! Get all vector parameters together.
    /**
     * Pass a pointer to a GPS_VTG object and the current
     * GPS data will be copied into it.
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     // Then get the data...
     *     GPS_VTG p;
     *     gps.vtg(&p);
     *     printf("Speed (knots)  = %.4f", p.velocity_knots);
     *     printf("Speed (kps)    = %.4f", p.velocity_kps);
     *     printf("Track (true)  = %.4f", p.track_true);
     *     printf("Track (mag)    = %.4f", p.track_mag);
     *
     * @endcode
     *
     * @ingroup API
     * @param g A GSP_VTG pointer to an existing GPS_VTG object.
     * @return GPS_VTG * The pointer passed in.
     */
    GPS_VTG *vtg(GPS_VTG *g);
    
    //! Get all vector parameters together.
    /**
     * Get all the vector data at once. For example:-
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     // Then get the data...
     *     GPS_VTG *p = gps.vtg();
     *     printf("Speed (knots)  = %.4f", p->velocity_knots);
     *     printf("Speed (kps)    = %.4f", p->velocity_kps);
     *     printf("Track (true)  = %.4f", p->track_true);
     *     printf("Track (mag)    = %.4f", p->track_mag);     
     *     delete(p); // then remember to delete the object to prevent memory leaks.
     *
     * @endcode
     *
     * @ingroup API
     * @return GPS_Geodetic * A pointer to the data.
     */
    GPS_VTG *vtg(void) { return vtg(NULL); }
    
    //! Get all three geodetic parameters together.
    /**
     * Pass a pointer to a GPS_Geodetic object and the current
     * GPS data will be copied into it.
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     // Then get the data...
     *     GPS_Geodetic p;
     *     gps.geodetic(&p);
     *     printf("Latitude  = %.4f", p.lat);
     *     printf("Longitude = %.4f", p.lon);
     *     printf("Altitude  = %.4f", p.alt);
     *
     * @endcode
     *
     * @ingroup API
     * @param g A GSP_Geodetic pointer to an existing GPS_Geodetic object.
     * @return GPS_Geodetic * The pointer passed in.
     */
    GPS_Geodetic *geodetic(GPS_Geodetic *g);
    
    //! Get all three geodetic parameters together.
    /**
     * Get all the geodetic data at once. For example:-
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     // Then get the data...
     *     GPS_Geodetic *p = gps.geodetic();
     *     printf("Latitude = %.4f", p->lat);
     *     delete(p); // then remember to delete the object to prevent memory leaks.
     *
     * @endcode
     *
     * @ingroup API
     * @return GPS_Geodetic * A pointer to the data.
     */
    GPS_Geodetic *geodetic(void) { return geodetic(NULL); }
    
    //! Take a snap shot of the current time.
    /**
     * Pass a pointer to a GPS_Time object to get a copy of the current
     * time and date as reported by the GPS.
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     // Then get the data...
     *     GPS_Time t;
     *     gps.timeNow(&t);
     *     printf("Year = %d", t.year);
     *
     * @endcode
     *
     * @ingroup API
     * @param n A GPS_Time * pointer to an existing GPS_Time object.
     * @return GPS_Time * The pointer passed in.
     */
    GPS_Time * timeNow(GPS_Time *n) { return theTime.timeNow(n); }
    
    //! Take a snap shot of the current time.
    /**
     * Pass a pointer to a GPS_Time object to get a copy of the current
     * time and date as reported by the GPS.
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     // Then get the data...
     *     GPS_Time *t = gps.timeNow();
     *     printf("Year = %d", t->year);
     *     delete(t); // Avoid memory leaks.
     *
     * @endcode
     *
     * @ingroup API
     * @return GPS_Time * The pointer passed in.
     */
    GPS_Time * timeNow(void) { GPS_Time *n = new GPS_Time; return theTime.timeNow(n); }
    
    //! Return the curent day.
    /**
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     // Then get the Julain Day Number.
     *     double julianDayNumber = gps.julianDayNumber();
     *
     * @endcode
     *
     * @ingroup API
     * @return double The Julian Date as a double.
     */
    double julianDayNumber(void) { return theTime.julian_day_number(); } 
    
    //! Return the curent date/time as a Julian date
    /**
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     // Then get the Julian Date.
     *     double julianDate = gps.julianDate();
     *
     * @endcode
     *
     * @ingroup API
     * @return double The Julian Date as a double.
     */
    double julianDate(void) { return theTime.julian_date(); }

    //! Get the current sidereal degree angle.
    /**
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *     double sidereal = gps.siderealDegrees();
     *
     * @endcode
     *
     * @ingroup API
     * @return double Sidereal degree angle..
     */
    double siderealDegrees(void) { return theTime.siderealDegrees(&theTime, longitude()); }
    
    //! Get the current sidereal hour angle.
    /**
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *     double sidereal = gps.siderealHA();
     *
     * @endcode
     *
     * @ingroup API
     * @return double Sidereal degree angle..
     */
    double siderealHA(void) { return theTime.siderealHA(&theTime, longitude()); }
    
    //! Optionally, connect a 1PPS single to an Mbed pin.
    /**
     * Optional: If the GPS unit has a 1PPS output, use this to
     * connect that to our internal ISR. Using the 1PPS increases
     * the GPS_Time time accuracy from +/-0.25s to +/-0.001s
     *
     * @code
     *     // Assuming we have a GPS object previously created...
     *     GPS gps(NC, p9); 
     *
     *     gps.ppsAttach(p29); // default to GPS::ppsRise, rising edge. 
     *
     *     // Or...
     *     gps.ppsAttach(p29, GPS::ppsRise); // The default.
     *
     *     // Or...
     *     gps.ppsAttach(p29, GPS::ppsFall); // If a falling edge.
     *
     * @endcode
     *
     * <b>Note</b>, before using this function you should attach an actual
     * callback function using attach_pps()
     *
     * @see attach_pps()
     *
     * @ingroup API
     * @param irq A PinName to attach
     * @param type The type of edge, MAX7456::ppsRise OR MAX7456::ppsFall
     */
    void ppsAttach(PinName irq, ppsEdgeType type = ppsRise);
    
    //! Remove any 1PPS signal previously attached.
    void ppsUnattach(void);
    
    //! GPS serial receive interrupt handler.
    void rx_irq(void);    
    
    //! GPS pps interrupt handler.
    void pps_irq(void);
    
    //! A pointer to the UART peripheral base address being used.
    void *_base;
    
    //! The RX serial buffer.
    char buffer[2][GPS_BUFFER_LEN];
    
    //! The current "active" buffer, i.e. the buffer the ISR is writing to.
    int  active_buffer;
    
    //! The active buffer "in" pointer.
    int  rx_buffer_in;
    
    //! Boolean flag set when the "passive" buffer is full and needs processing.
    bool process_required;
    
    //! 10ms Ticker callback.
    void ticktock(void);
    
    //! Attach a user object/method callback function to the PPS signal
    /**
     * Attach a user callback object/method to call when the 1PPS signal activates. 
     *
     * @code
     *     class FOO {
     *     public:
     *         void myCallback(void);
     *     };
     *
     *     GPS gps(NC, p9); 
     *     Foo foo;
     *
     *     gps.attach_pps(foo, &FOO::myCallback);
     * 
     * @endcode
     *
     * @ingroup API
     * @param tptr pointer to the object to call the member function on
     * @param mptr pointer to the member function to be called
     */
    template<typename T>
    void attach_pps(T* tptr, void (T::*mptr)(void)) { cb_pps.attach(tptr, mptr); }
    
    //! Attach a user callback function to the PPS signal
    /**
     * Attach a user callback function pointer to call when the 1PPS signal activates. 
     *
     * @code
     *     void myCallback(void) { ... }
     *
     *     GPS gps(NC, p9); 
     *     Foo foo;
     *
     *     gps.attach_pps(&myCallback);
     * 
     * @endcode
     *
     * @ingroup API
     * @param fptr Callback function pointer
     */
    void attach_pps(void (*fptr)(void)) { cb_pps.attach(fptr); } 
    
    //! A callback object for the 1PPS user API.
    FunctionPointer cb_pps;
    
    //! Attach a user callback function to the NMEA RMC message processed signal.
    /**
     * Attach a user callback object/method to call when an NMEA RMC packet has been processed. 
     *
     * @code
     *     class FOO {
     *     public:
     *         void myCallback(void);
     *     };
     *
     *     GPS gps(NC, p9); 
     *     Foo foo;
     *
     *     gps.attach_rmc(foo, &FOO::myCallback);
     * 
     * @endcode
     *
     * @ingroup API 
     * @param tptr pointer to the object to call the member function on
     * @param mptr pointer to the member function to be called
     */
    template<typename T>
    void attach_rmc(T* tptr, void (T::*mptr)(void)) { cb_rmc.attach(tptr, mptr); }
    
    //! Attach a user callback function to the NMEA RMC message processed signal.
    /**
     * Attach a user callback function pointer to call when an NMEA RMC packet has been processed. 
     *
     * @code
     *     void myCallback(void) { ... }
     *
     *     GPS gps(NC, p9); 
     *     Foo foo;
     *
     *     gps.attach_rmc(&myCallback);
     * 
     * @endcode
     *
     * @ingroup API 
     * @param fptr Callback function pointer.
     */
    void attach_rmc(void (*fptr)(void)) { cb_rmc.attach(fptr); } 
    
    //! A callback object for the NMEA RMS message processed signal user API.
    FunctionPointer cb_rmc;
    
    //! Attach a user callback function to the NMEA GGA message processed signal.
    /**
     * Attach a user callback object/method to call when an NMEA GGA packet has been processed. 
     *
     * @code
     *     class FOO {
     *     public:
     *         void myCallback(void);
     *     };
     *
     *     GPS gps(NC, p9); 
     *     Foo foo;
     *
     *     gps.attach_gga(foo, &FOO::myCallback);
     * 
     * @endcode
     *
     * @ingroup API 
     * @param tptr pointer to the object to call the member function on
     * @param mptr pointer to the member function to be called
     */
    template<typename T>
    void attach_gga(T* tptr, void (T::*mptr)(void)) { cb_gga.attach(tptr, mptr); }
    
    //! Attach a user callback function to the NMEA GGA message processed signal.
    /**
     * Attach a user callback function pointer to call when an NMEA GGA packet has been processed. 
     *
     * @code
     *     void myCallback(void) { ... }
     *
     *     GPS gps(NC, p9); 
     *     Foo foo;
     *
     *     gps.attach_gga(&myCallback);
     * 
     * @endcode
     *
     * @ingroup API 
     * @param fptr Callback function pointer.
     */
    void attach_gga(void (*fptr)(void)) { cb_gga.attach(fptr); } 
    
    //! A callback object for the NMEA GGA message processed signal user API.
    FunctionPointer cb_gga;


    //! Attach a user callback function to the NMEA VTG message processed signal.
    /**
     * Attach a user callback object/method to call when an NMEA VTG packet has been processed. 
     *
     * @code
     *     class FOO {
     *     public:
     *         void myCallback(void);
     *     };
     *
     *     GPS gps(NC, p9); 
     *     Foo foo;
     *
     *     gps.attach_vtg(foo, &FOO::myCallback);
     * 
     * @endcode
     *
     * @ingroup API 
     * @param tptr pointer to the object to call the member function on
     * @param mptr pointer to the member function to be called
     */
    template<typename T>
    void attach_vtg(T* tptr, void (T::*mptr)(void)) { cb_vtg.attach(tptr, mptr); }
    
    //! Attach a user callback function to the NMEA VTG message processed signal.
    /**
     * Attach a user callback function pointer to call when an NMEA VTG packet has been processed. 
     *
     * @code
     *     void myCallback(void) { ... }
     *
     *     GPS gps(NC, p9); 
     *     Foo foo;
     *
     *     gps.attach_vtg(&myCallback);
     * 
     * @endcode
     *
     * @ingroup API 
     * @param fptr Callback function pointer.
     */
    void attach_vtg(void (*fptr)(void)) { cb_vtg.attach(fptr); } 
    
    //! A callback object for the NMEA RMS message processed signal user API.
    FunctionPointer cb_vtg;
    
    //! Attach a user callback function to the unknown NMEA message.
    /**
     * Attach a user callback object/method to call when an unknown NMEA packet. 
     *
     * @code
     *     class FOO {
     *     public:
     *         void myCallback(void);
     *     };
     *
     *     GPS gps(NC, p9); 
     *     Foo foo;
     *
     *     gps.attach_ukn(foo, &FOO::myCallback);
     * 
     * @endcode
     *
     * @ingroup API 
     * @param tptr pointer to the object to call the member function on
     * @param mptr pointer to the member function to be called
     */
    template<typename T>
    void attach_ukn(T* tptr, void (T::*mptr)(void)) { cb_ukn.attach(tptr, mptr); }
    
    //! Attach a user callback function to the unknown NMEA message.
    /**
     * Attach a user callback function pointer to call when an unknown NMEA. 
     *
     * @code
     *     void myCallback(void) { ... }
     *
     *     GPS gps(NC, p9); 
     *     Foo foo;
     *
     *     gps.attach_ukn(&myCallback);
     * 
     * @endcode
     *
     * @ingroup API 
     * @param fptr Callback function pointer.
     */
    void attach_ukn(void (*fptr)(void)) { cb_ukn.attach(fptr); } 
    
    //! A callback object for the NMEA RMS message processed signal user API.
    FunctionPointer cb_ukn;
    
    /**
     * Set's the GGA string memory pointer.
     * @param s char pointer ti string.
     * @return char s passed in.
     */
    char * setGga(char *s) { _gga = s; return s; }
    
    /**
     * Set's the RMC string memory pointer.
     * @param s char pointer ti string.
     * @return char s passed in.
     */
    char * setRmc(char *s) { _rmc = s; return s; };
    
    /**
     * Set's the VTG string memory pointer.
     * @param s char pointer ti string.
     * @return char s passed in.
     */
    char * setVtg(char *s) { _vtg = s; return s; };
    
    /**
     * Set's the UKN string memory pointer.
     * @param s char pointer ti string.
     * @return char s passed in.
     */
    char * setUkn(char *s) { _ukn = s; return s; };
    
    //! Set the baud rate the GPS module is using.
    /** 
     * Set the baud rate of the serial port
     * 
     * @see http://mbed.org/projects/libraries/api/mbed/trunk/Serial#Serial.baud
     *
     * @ingroup API 
     * @param baudrate The baudrate to set.
     */
    void baud(int baudrate) { Serial::baud(baudrate); }
    
   //! Set the serial port format the GPS module is using. 
   /**
    * Set the transmission format used by the Serial port
    *
    * @see http://mbed.org/projects/libraries/api/mbed/trunk/Serial#Serial.format
    *
    * @ingroup API 
    * @param bits - The number of bits in a word (5-8; default = 8)
    * @param parity - The parity used (GPS::None, GPS::Odd, GPS::Even, GPS::Forced1, GPS::Forced0; default = GPS::None)
    * @param stop_bits - The number of stop bits (1 or 2; default = 1)
    */
    void format(int bits, Parity parity, int stop_bits) { Serial::format(bits, (Serial::Parity)parity, stop_bits); }
    
   //! Send incoming GPS bytes to Uart0
   /**
    * Send incoming GPS bytes to Uart0
    *
    * This can be useful for printing out the bytes from the GPS onto
    * a the common debug port Uart0. Note, Uart0 should have been setup
    * and initialised before switching this on. Also, realistically,
    * you should ensure Uart0 has a higher baud rate than that being
    * used by the GPS. Sending of bytes to Uart0 is "raw" and should
    * only be used to initially gather data and should NOT be used as
    * part of the application design. If you need to forward on the 
    * data you should come up with a proper strategy.
    *
    * @ingroup API 
    * @param b - True to send to Uart0, false otherwise
    */
    void NmeaOnUart0(bool b) { _nmeaOnUart0 = b; }
        
protected:

    //! Flag set true when a GPS PPS has been attached to a pin.
    bool         _ppsInUse;
    
    //! An InterruptIn object to "trigger" on the PPS edge.
    InterruptIn *_pps;
    
    //! A Ticker object called every 10ms.
    Ticker      *_second100;
    
    //! A GPS_Time object used to hold the last parsed time/date data.
    GPS_Time     theTime;
    
    //! A GPS_Geodetic object used to hold the last parsed positional data.
    GPS_Geodetic thePlace;
    
    //! A GPS_VTG object used to hold vector data.
    GPS_VTG      theVTG; 
    
    //! Used to record the previous byte received.
    char _lastByte;
    
    char *_gga;
    char *_rmc;
    char *_vtg;
    char *_ukn;
    
    //! Used for debugging.
    bool _nmeaOnUart0;      
};

#endif

