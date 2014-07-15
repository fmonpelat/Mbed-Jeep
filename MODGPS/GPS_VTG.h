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

#ifndef GPS_VTG_H
#define GPS_VTG_H

#include "mbed.h"

/** GPS_Time definition.
 */
class GPS_VTG {
public:

    //! The velocity (in knots)
    double _velocity_knots;
    //! The velocity (in kph)
    double _velocity_kph;
    //! The track (in decimal degrees true)
    double _track_true;    
    //! The track (in decimal degrees magnetic)
    double _track_mag;    
    
    GPS_VTG();
    GPS_VTG * vtg(GPS_VTG *n);
    void nmea_vtg(char *s); 
    
    double velocity_knots(void) { return _velocity_knots; }
    double velocity_kph(void)   { return _velocity_kph; }
    double track_true(void)     { return _track_true;   } 
    double track_mag(void)      { return _track_mag;    }
     
};

#endif

