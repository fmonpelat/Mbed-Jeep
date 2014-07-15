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

#include "GPS.h"

GPS::GPS(PinName tx, PinName rx, const char *name) : Serial(tx, rx, name) 
{
    _nmeaOnUart0 = false;
    
    _lastByte = 0;
    
    _gga = (char *)NULL;
    
    _rmc = (char *)NULL;
    
    _vtg = (char *)NULL;

    _base = LPC_UART1;
//    switch(_uidx) {
//        case 1:   _base = LPC_UART1; break;
//        case 2:   _base = LPC_UART2; break;
//        case 3:   _base = LPC_UART3; break;
//        default : _base = NULL;      break;
//    }
    
    _pps = NULL;
    _ppsInUse = false;
    
    if (_base != NULL) attach(this, &GPS::rx_irq);
    
    _second100 = new Ticker;
    _second100->attach_us(this, &GPS::ticktock, GPS_TICKTOCK);
}

void 
GPS::ppsAttach(PinName irq, ppsEdgeType type) 
{
    if (_pps != NULL) delete(_pps);
    _pps = new InterruptIn(irq);
    if (type == ppsRise) _pps->rise(this, &GPS::pps_irq);
    else _pps->fall(this, &GPS::pps_irq);
    _ppsInUse = true;     
}
    
void 
GPS::ppsUnattach(void) 
{
    if (_pps != NULL) delete(_pps);
    _ppsInUse = false;
}
    
double 
GPS::latitude(void)  
{
    double a, b;
    do { a = thePlace.lat; b = thePlace.lat; }  while (a != b);
    return a; 
}

double 
GPS::longitude(void) 
{ 
    double a, b;
    do { a = thePlace.lon; b = thePlace.lon; } while (a != b);
    return a; 
}

double 
GPS::altitude(void)  
{ 
    double a, b;
    do { a = thePlace.alt; b = thePlace.alt; } while (a != b);
    return a; 
}

GPS_Geodetic *
GPS::geodetic(GPS_Geodetic *q)
{
    GPS_Geodetic a;
    
    if (q == NULL) q = new GPS_Geodetic;
    
    do {
        memcpy(&a, &thePlace, sizeof(GPS_Geodetic));
        memcpy(q,  &thePlace, sizeof(GPS_Geodetic));
    }
    while (memcmp(&a, q, sizeof(GPS_Geodetic)) != 0);
    
    return q;
}

GPS_VTG *
GPS::vtg(GPS_VTG *q)
{
    GPS_VTG a;
    
    if (q == NULL) q = new GPS_VTG;
    
    do {
        memcpy(&a, &theVTG, sizeof(GPS_VTG));
        memcpy(q,  &theVTG, sizeof(GPS_VTG));
    }
    while (memcmp(&a, q, sizeof(GPS_VTG)) != 0);
    
    return q;
}

void
GPS::ticktock(void)
{
    int i;
    
    // Increment the time structure by 1/100th of a second.
    ++theTime; 
    
    // Test the serial queue.
    if (process_required) {
        char *s = buffer[active_buffer == 0 ? 1 : 0];
        if (!strncmp(s, "$GPRMC", 6)) {
            if (_rmc) {
                for(i = 0; s[i] != '\n'; i++) {
                    _rmc[i] = s[i];
                }
                _rmc[i++] = '\n'; _rmc[i] = '\0';
            }
            theTime.nmea_rmc(s);
            cb_rmc.call();
            if (!_ppsInUse) theTime.fractionalReset();
        }
        else if (!strncmp(s, "$GPGGA", 6)) {
            if (_gga) {
                for(int i = 0; s[i] != '\n'; i++) {
                    _gga[i] = s[i];
                }
                _gga[i++] = '\n'; _gga[i] = '\0';
            }            
            thePlace.nmea_gga(s);            
            cb_gga.call();
        }
        else if (!strncmp(s, "$GPVTG", 6)) {
            if (_vtg) {
                for(int i = 0; s[i] != '\n'; i++) {
                    _vtg[i] = s[i];
                }
                _vtg[i++] = '\n'; _vtg[i] = '\0';
            }
            theVTG.nmea_vtg(s);            
            cb_vtg.call();
        }
        else {
            if (_ukn) {
                for(int i = 0; s[i] != '\n'; i++) {
                    _ukn[i] = s[i];
                }
                _ukn[i++] = '\n'; _ukn[i] = '\0';
                cb_ukn.call();
            }
        }
        process_required = false;
    }
    
    // If we have a valid GPS time then, once per minute, set the RTC.
    if (theTime.status == 'A' && theTime.second == 0 && theTime.tenths == 0 && theTime.hundreths == 0) {
        // set_time() is defined in rtc_time.h
        // http://mbed.org/projects/libraries/svn/mbed/trunk/rtc_time.h
        set_time(theTime.to_C_tm());        
    }
    
}

void 
GPS::pps_irq(void)
{
    theTime.fractionalReset();
    theTime++; // Increment the time/date by one second. 
    cb_pps.call();
}

void 
GPS::rx_irq(void)
{
    uint32_t iir __attribute__((unused));
    char c;
    
    if (_base) {
        iir = (uint32_t)*((char *)_base + GPS_IIR); 
        while((int)(*((char *)_base + GPS_LSR) & 0x1)) {
            c = (char)(*((char *)_base + GPS_RBR) & 0xFF);             
            
            // strtok workaround. 
            // Found that ,, together (which some NMEA sentences
            // contain for a null/empty field) confuses strtok()
            // function. Solution:- Push a "zero" into the string 
            // for missing/empty fields.
            if (c == ',' && _lastByte == ',') {
                buffer[active_buffer][rx_buffer_in] = '0';
                if (++rx_buffer_in >= GPS_BUFFER_LEN) rx_buffer_in = 0;
            }
            
            // Debugging/dumping data. 
            if (_nmeaOnUart0) LPC_UART0->RBR = c; 
            
            // Put the byte into the string.
            buffer[active_buffer][rx_buffer_in] = c;
            if (++rx_buffer_in >= GPS_BUFFER_LEN) rx_buffer_in = 0;
            
            // Save for next time an irq occurs. See strtok() above.
            _lastByte = c;
            
            // If end of NMEA sentence flag for processing.
            if (c == '\n') {
                active_buffer = active_buffer == 0 ? 1 : 0;
                process_required = true;
                rx_buffer_in = 0;                
            }            
        }
    }
}      
