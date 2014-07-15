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

#include "GPS_Time.h"

GPS_Time::GPS_Time() 
{
    year = 2000;
    month = 1;
    day = 1;
    hour = 0;
    minute = 0;
    second = 0;
    tenths = 0;
    hundreths = 0;
    status = 'V';
    velocity = 0;
    track = 0;    
    magvar_dir = 'W';
    magvar = 0;
}

time_t
GPS_Time::to_C_tm(bool set) 
{
    GPS_Time t;
    tm       ct;
    time_t   q;
    
    timeNow(&t);
    ct.tm_sec       = t.second;
    ct.tm_min       = t.minute;
    ct.tm_hour      = t.hour;
    ct.tm_mday      = t.day;
    ct.tm_mon       = t.month - 1;
    ct.tm_year      = t.year - 1900;
    ct.tm_isdst     = 0; // GPS has no understanding of DST.    
    
    q = mktime(&ct);
    if (set) {
        set_time(q);
    }
    return q;
}

GPS_Time *
GPS_Time::timeNow(GPS_Time *n)
{
    if (n == NULL) n = new GPS_Time;
    
    do {
        memcpy(n, this, sizeof(GPS_Time));
    }
    while (memcmp(n, this, sizeof(GPS_Time)));
    return n;    
}

void
GPS_Time::operator++()
{
    hundreths++;
    if (hundreths == 10) {
        hundreths = 0;
        tenths++;
        if (tenths == 10) {
            tenths = hundreths = 0;
        }
    }
}

void
GPS_Time::operator++(int)
{
    const int days[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    bool dateInc = false;
    
    tenths = hundreths = 0;
    second++;
    
    if (second == 60) {
        second = 0;
        minute++;
        if (minute == 60) {
            minute = 0;
            hour++;
            if (hour == 24) {
                hour = 0;
                dateInc = true;
            }
        }
    }
    
    if (dateInc) {
        /* Handle February leap year. */    
        int leap_year = ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) ? 1 : 0;
        int days_this_month = days[month - 1];
        if (month == 2 && leap_year) days_this_month++;        
        day++;
        if (day > days_this_month) {
            day = 1;
            month++;
            if (month == 13) {
                year++;
            }
        }
    }
}

// $GPRMC,112709.735,A,5611.5340,N,00302.0306,W,000.0,307.0,150411,,,A*70
void 
GPS_Time::nmea_rmc(char *s)
{
    char *token;
    int  token_counter = 0;
    char *time   = (char *)NULL;
    char *date   = (char *)NULL;
    char *stat   = (char *)NULL;
    char *vel    = (char *)NULL;
    char *trk    = (char *)NULL;
    char *magv   = (char *)NULL;
    char *magd   = (char *)NULL;

    token = strtok(s, ",");
    while (token) {
        switch (token_counter) {
            case 9:  date   = token; break;
            case 1:  time   = token; break;
            case 2:  stat   = token; break;
            case 7:  vel    = token; break;
            case 8:  trk    = token; break;
            case 10: magv   = token; break;
            case 11: magd   = token; break;
        }
        token = strtok((char *)NULL, ",");
        token_counter++;
    }
    
    if (stat && date && time) {
        hour       = (char)((time[0] - '0') * 10) + (time[1] - '0');
        minute     = (char)((time[2] - '0') * 10) + (time[3] - '0');
        second     = (char)((time[4] - '0') * 10) + (time[5] - '0');
        day        = (char)((date[0] - '0') * 10) + (date[1] - '0');
        month      = (char)((date[2] - '0') * 10) + (date[3] - '0');
        year       =  (int)((date[4] - '0') * 10) + (date[5] - '0') + 2000;
        status     = stat[0];
        velocity   = atof(vel);
        track      = atof(trk);
        magvar     = atof(magv);
        magvar_dir = magd[0];
    }    
}

double 
GPS_Time::julian_day_number(GPS_Time *t) {
    double wikipedia_jdn = (double)(1461 * ((int)t->year + 4800 + ((int)t->month - 14) / 12)) / 4 + (367 * ((int)t->month - 2 - 12 * (((int)t->month - 14) / 12))) / 12 - (3 * (((int)t->year + 4900 + ((int)t->month - 14) / 12 ) / 100)) / 4 + (int)t->day - 32075;    
    return wikipedia_jdn;
}

double 
GPS_Time::julian_date(GPS_Time *t) {
    double hour, minute, second, jd;
    hour   = (double)t->hour;
    minute = (double)t->minute;
    second = (double)t->second + ((double)t->tenths / 10.) + ((double)t->hundreths / 100.);
                                 
    jd = julian_day_number(t) -  0.5 +
         ((hour - 12.) / 24.) +
         (minute / 1440.) + 
         (second / 86400.);        
        
    return jd;
}

double 
GPS_Time::siderealDegrees(double jd, double longitude) {
    double sidereal, gmst, lmst;
    double T  = jd - 2451545.0;
    double T1 = T / 36525.0;
    double T2 = T1 * T1;
    double T3 = T2 * T1;
     
    /* Calculate gmst angle. */
    sidereal = 280.46061837 + (360.98564736629 * T) + (0.000387933 * T2) - (T3 / 38710000.0);
     
    /* Convert to degrees. */
    sidereal = fmod(sidereal, 360.0);
    if (sidereal < 0.0) sidereal += 360.0;
 
    gmst = sidereal;
    lmst = gmst + longitude;
    return lmst;
}

double 
GPS_Time::siderealDegrees(GPS_Time *t, double longitude) {
    if (t == NULL) t = new GPS_Time;
    return siderealDegrees(julian_date(t), longitude);
}

double 
GPS_Time::siderealHA(double jd, double longitude) {
    double lmst = siderealDegrees(jd, longitude);
    return lmst / 360.0 * 24.0;
}

double 
GPS_Time::siderealHA(GPS_Time *t, double longitude) {
    double lmst = siderealDegrees(t, longitude);
    return lmst / 360.0 * 24.0;
}

