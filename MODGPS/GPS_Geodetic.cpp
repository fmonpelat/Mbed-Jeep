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

#include "GPS_Geodetic.h"

void 
GPS_Geodetic::nmea_gga(char *s) {
    char *token;
    int  token_counter = 0;
    char *latitude  = (char *)NULL;
    char *longitude = (char *)NULL;
    char *lat_dir   = (char *)NULL;
    char *lon_dir   = (char *)NULL;
    char *qual      = (char *)NULL;
    char *altitude  = (char *)NULL;
    char *sats      = (char *)NULL;
            
    token = strtok(s, ",");
    while (token) {
        switch (token_counter) {
                case 2:  latitude  = token; break;
                case 4:  longitude = token; break;
                case 3:  lat_dir   = token; break;    
                case 5:  lon_dir   = token; break;    
                case 6:  qual      = token; break;
                case 7:  sats      = token; break;
                case 9:  altitude  = token; break;
        }
        token = strtok((char *)NULL, ",");
        token_counter++;
    }

    // If the fix quality is valid set our location information. 
    if (latitude && longitude && altitude && sats) {         
        lat = convert_lat_coord(latitude,  lat_dir[0]);
        lon = convert_lon_coord(longitude, lon_dir[0]);
        alt = convert_height(altitude);        
        num_of_gps_sats = atoi(sats);
        gps_satellite_quality = atoi(qual);
    }
    else {
        gps_satellite_quality = 0;
    }    
}

double 
GPS_Geodetic::convert_lat_coord(char *s, char north_south) 
{
    int deg, min, sec;
    double fsec, val;
    
    deg  = ( (s[0] - '0') * 10) + s[1] - '0';
    min  = ( (s[2] - '0') * 10) + s[3] - '0';
    sec  = ( ((s[5] - '0') * 1000) + ((s[6] - '0') * 100) + ((s[7] - '0') * 10) + (s[8] - '0'));
    fsec = (double)((double)sec /10000.0);
    val  = (double)deg + ((double)((double)min/60.0)) + (fsec/60.0);
    if (north_south == 'S') { val *= -1.0; }
    lat = val;
    return val;
}

double 
GPS_Geodetic::convert_lon_coord(char *s, char east_west) 
{
    int deg, min, sec;
    double fsec, val;
    
    deg  = ( (s[0] - '0') * 100) + ((s[1] - '0') * 10) + (s[2] - '0');
    min  = ( (s[3] - '0') * 10) + s[4] - '0';
    sec  = ( ((s[6] - '0') * 1000) + ((s[7] - '0') * 100) + ((s[8] - '0') * 10) + (s[9] - '0'));
    fsec = (double)((double)sec /10000.0);
    val  = (double)deg + ((double)((double)min/60.0)) + (fsec/60.0);
    if (east_west == 'W') { val *= -1.0; }
    lon = val;
    return val;

}

double 
GPS_Geodetic::convert_height(char *s) 
{
    double val = (double)(atof(s) / 1000.0);
    alt = val;
    return val;
}


