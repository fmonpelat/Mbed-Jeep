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

#ifndef GPS_TIME_H
#define GPS_TIME_H

#include "mbed.h"

/** GPS_Time definition.
 */
class GPS_Time {
public:

    //! The year
    int  year;      
    //! The month
    int  month;     
    //! The day
    int  day;       
    //! The hour
    int  hour;      
    //! The minute
    int  minute;    
    //! The second
    int  second;    
    //! Tenths of a second
    int  tenths;    
    //! Hundredths of a second
    int  hundreths; 
    //! Time status.
    char status;    
    //! The velocity (in knots)
    double velocity;
    //! The track (in decimal degrees true)
    double track;    
    //! The magnetic variation direction
    char magvar_dir;
    //! The magnetic variation value
    double magvar;
    
    GPS_Time();
    void fractionalReset(void) { tenths = hundreths = 0; }
    void operator++();
    void operator++(int);
    GPS_Time * timeNow(GPS_Time *n);
    GPS_Time * timeNow(void) { return timeNow(NULL); }
    void nmea_rmc(char *s);
    double velocity_knots(void) { return velocity; }
    double velocity_kph(void) { return (velocity * 1.852); }
    double velocity_mps(void) { return velocity_kph() / 3600.0; }
    double velocity_mph(void) { return velocity_kph() / 0.621371192; }
    double track_over_ground(void) { return track; }
    double magnetic_variation(void) { return magvar_dir == 'W' ? (magvar * -1.0) : (magvar); }
    double julian_day_number(GPS_Time *t);
    double julian_date(GPS_Time *t);
    double julian_day_number(void) { return julian_day_number(this); }
    double julian_date(void) { return julian_date(this); }   
    double siderealDegrees(double jd, double longitude);
    double siderealDegrees(GPS_Time *t, double longitude);
    double siderealHA(double jd, double longitude);
    double siderealHA(GPS_Time *t, double longitude);
    time_t to_C_tm(bool set = false);
};

#endif

