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

#include "GPS_VTG.h"
#include <math.h>

GPS_VTG::GPS_VTG() 
{
    _velocity_knots = 0;
    _velocity_kph = 0;
    _track_true = 0;    
    _track_mag = 0;    
}

GPS_VTG *
GPS_VTG::vtg(GPS_VTG *n)
{
    if (n == NULL) n = new GPS_VTG;
    
    n->_velocity_knots = _velocity_knots;
    n->_velocity_kph   = _velocity_kph;
    n->_track_true     = _track_true;
    n->_track_mag      = _track_mag;
    
    return n;    
}

void 
GPS_VTG::nmea_vtg(char *s)
{
    char *token;
    int  token_counter = 0;
    char *vel_knots = (char *)NULL;
    char *vel_kph   = (char *)NULL;
    char *trk_t  = (char *)NULL;
    char *trk_m  = (char *)NULL;
    
    token = strtok(s, ",");
    while (token) {
        switch (token_counter) {
            case 5:  vel_knots = token; break;
            case 7:  vel_kph   = token; break;
            case 1:  trk_t     = token; break;
            case 3:  trk_m     = token; break;            
        }
        token = strtok((char *)NULL, ",");
        token_counter++;
    }
    
    if (trk_t)     { _track_true     = atof(trk_t);     }
    if (trk_m)     { _track_mag      = atof(trk_m);     }    
    if (vel_knots) { _velocity_knots = atof(vel_knots); }
    if (vel_kph)   { _velocity_kph   = atof(vel_kph);   }    
}

