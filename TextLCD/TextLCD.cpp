/* mbed TextLCD Library, for a 4-bit LCD based on HD44780
 * Copyright (c) 2007-2010, sford, http://mbed.org
 *               2013, v01: WH, Added LCD types, fixed LCD address issues, added Cursor and UDCs 
 *               2013, v02: WH, Added I2C and SPI bus interfaces  
 *               2013, v03: WH, Added support for LCD40x4 which uses 2 controllers 
 *               2013, v04: WH, Added support for Display On/Off, improved 4bit bootprocess
 *               2013, v05: WH, Added support for 8x2B, added some UDCs   
 *               2013, v06: WH, Added support for devices that use internal DC/DC converters 
 *               2013, v07: WH, Added support for backlight and include portdefinitions for LCD2004 Module from DFROBOT 
 *               2014, v08: WH, Refactored in Base and Derived Classes to deal with mbed lib change regarding 'NC' defined pins 
 *               2014, v09: WH/EO, Added Class for Native SPI controllers such as ST7032 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "TextLCD.h"
#include "mbed.h"

/** Create a TextLCD_Base interface
  *
  * @param type  Sets the panel size/addressing mode (default = LCD16x2)
  * @param ctrl  LCD controller (default = HD44780)           
  */
TextLCD_Base::TextLCD_Base(LCDType type, LCDCtrl ctrl) : _type(type), _ctrl(ctrl) {
}


/**  Init the LCD Controller(s)
  *  Clear display 
  */
void TextLCD_Base::_init() {
  
  // Select and configure second LCD controller when needed
  if(_type==LCD40x4) {
    _ctrl_idx=_LCDCtrl_1; // Select 2nd controller
    
    _initCtrl();                   // Init 2nd controller
    
    // Secondary LCD controller Clearscreen
    _writeCommand(0x01);       // cls, and set cursor to 0    
    wait_ms(10);     // The CLS command takes 1.64 ms.
                     // Since we are not using the Busy flag, Lets be safe and take 10 ms
    
  }
    
  // Select and configure primary LCD controller
  _ctrl_idx=_LCDCtrl_0; // Select primary controller  

  _initCtrl();                   // Init primary controller
  
  // Primary LCD controller Clearscreen
  _writeCommand(0x01);       // cls, and set cursor to 0

  wait_ms(10);     // The CLS command takes 1.64 ms.
                   // Since we are not using the Busy flag, Lets be safe and take 10 ms
    
} 

/**  Init the LCD controller
  *  4-bit mode, number of lines, fonttype, no cursor etc
  *  
  */
void TextLCD_Base::_initCtrl() {

    this->_setRS(false);      // command mode
    
    wait_ms(20);        // Wait 20ms to ensure powered up

    // send "Display Settings" 3 times (Only top nibble of 0x30 as we've got 4-bit bus)    
    for (int i=0; i<3; i++) {
        _writeNibble(0x3);
        wait_ms(15);     // This command takes 1.64ms, so wait for it 
    }
    _writeNibble(0x2);   // 4-bit mode
    wait_us(40);         // most instructions take 40us

    // Display is now in 4-bit mode
   
    // Device specific initialisations for DC/DC converter to generate VLCD or VLED
    switch (_ctrl) {
      case ST7036:
          // ST7036 controller: Initialise Voltage booster for VLCD. VDD=5V
          // Note: supports 1,2 or 3 lines
          _writeByte( 0x29 );    // 4-bit Databus, 2 Lines, Select Instruction table 1
          wait_ms(30);           // > 26,3ms 
          _writeByte( 0x14 );    // Bias: 1/5, 2-Lines LCD 
          wait_ms(30);           // > 26,3ms
          _writeByte( 0x55 );    // Icon off, Booster on, Set Contrast C5, C4
          wait_ms(30);           // > 26,3ms
          _writeByte( 0x6d );    // Voltagefollower On, Ampl ratio Rab2, Rab1, Rab0
          wait_ms(200);          // > 200ms!
          _writeByte( 0x78 );    // Set Contrast C3, C2, C1, C0
          wait_ms(30);           // > 26,3ms
          _writeByte( 0x28 );    // Return to Instruction table 0
          wait_ms(50);      
          break;
          
      case ST7032:
          _writeByte( 0x1c );    //Internal OSC frequency adjustment 183HZ, bias will be 1/4 
          wait_us(30);
          _writeByte( 0x73 );    //Contrast control  low byte
          wait_us(30);  
          _writeByte( 0x57 );    //booster circuit is turn on. /ICON display off. /Contrast control   high byte
          wait_us(30);
          _writeByte( 0x6c );    //Follower control
          wait_us(50);   
          _writeByte( 0x0c );    //DISPLAY ON
          wait_us(30);
          break;
          
      case WS0010:         
          // WS0010 OLED controller: Initialise DC/DC Voltage converter for LEDs
          // Note: supports 1 or 2 lines (and 16x100 graphics)
          //       supports 4 fonts (English/Japanese (default), Western European-I, English/Russian, Western European-II)
                           // Cursor/Disp shift set 0001 SC RL  0 0
                           //
                           // Mode en Power set     0001 GC PWR 1 1                           
                           //  GC  = 0 (Graph Mode=1, Char Mode=0)             
                           //  PWR =   (DC/DC On/Off)
    
          //_writeCommand(0x13);   // DC/DC off              
          _writeCommand(0x17);   // DC/DC on
          
          wait_ms(10);
          break;
        
        default:
          // Devices that do not use DC/DC Voltage converters but external VLCD
          break;                  
    }
    
    // Initialise Display configuration
    switch (_type) {
        case LCD8x1:
        case LCD8x2B:        
            //8x1 is a regular 1 line display
            //8x2B is a special case of 16x1
            _writeCommand(0x20); // Function set 001 DL N F - -
                                 //  DL=0 (4 bits bus)             
                                 //   N=0 (1 line)
                                 //   F=0 (5x7 dots font)
            break;                                
            
        case LCD24x4:
            // Special mode for KS0078
            _writeCommand(0x2A); // Function set 001 DL N RE DH REV
                                 //   DL=0  (4 bits bus)             
                                 //    N=1  (Dont care for KS0078)
                                 //   RE=0  (Extended Regs, special mode for KS0078)
                                 //   DH=1  (Disp shift, special mode for KS0078)                                
                                 //   REV=0 (Reverse, special mode for KS0078)

            _writeCommand(0x2E); // Function set 001 DL N RE DH REV
                                 //   DL=0  (4 bits bus)             
                                 //    N=1  (Dont care for KS0078)
                                 //   RE=1  (Ena Extended Regs, special mode for KS0078)
                                 //   DH=1  (Disp shift, special mode for KS0078)                                
                                 //   REV=0 (Reverse, special mode for KS0078)

            _writeCommand(0x09); // Ext Function set 0000 1 FW BW NW
                                 //   FW=0  (5-dot font, special mode for KS0078)
                                 //   BW=0  (Cur BW invert disable, special mode for KS0078)
                                 //   NW=1  (4 Line, special mode for KS0078)                                

            _writeCommand(0x2A); // Function set 001 DL N RE DH REV
                                 //   DL=0  (4 bits bus)             
                                 //    N=1  (Dont care for KS0078)
                                 //   RE=0  (Dis. Extended Regs, special mode for KS0078)
                                 //   DH=1  (Disp shift, special mode for KS0078)                                
                                 //   REV=0 (Reverse, special mode for KS0078)
            break;
                                            
// All other LCD types are initialised as 2 Line displays (including LCD40x4)
        default:
            _writeCommand(0x28); // Function set 001 DL N F - -
                                 //  DL=0 (4 bits bus) 
                                 //   N=1 (2 lines)
                                 //   F=0 (5x7 dots font, only option for 2 line display)
                                 //    -  (Don't care)                                
            
            break;
    }

    _writeCommand(0x06); // Entry Mode 0000 01 CD S 
                         //   Cursor Direction and Display Shift
                         //   CD=1 (Cur incr)
                         //   S=0  (No display shift)                        

//    _writeCommand(0x0C); // Display Ctrl 0000 1 D C B
//                         //   Display On, Cursor Off, Blink Off   
    setCursor(CurOff_BlkOff);     
    setMode(DispOn);     
}


/** Clear the screen, Cursor home. 
  */
void TextLCD_Base::cls() {

  // Select and configure second LCD controller when needed
  if(_type==LCD40x4) {
    _ctrl_idx=_LCDCtrl_1; // Select 2nd controller

    // Second LCD controller Cursor always Off
    _setCursorAndDisplayMode(_currentMode, CurOff_BlkOff);

    // Second LCD controller Clearscreen
    _writeCommand(0x01); // cls, and set cursor to 0    

    wait_ms(10);     // The CLS command takes 1.64 ms.
                     // Since we are not using the Busy flag, Lets be safe and take 10 ms

  
    _ctrl_idx=_LCDCtrl_0; // Select primary controller
  }
  
  // Primary LCD controller Clearscreen
  _writeCommand(0x01); // cls, and set cursor to 0

  wait_ms(10);     // The CLS command takes 1.64 ms.
                   // Since we are not using the Busy flag, Lets be safe and take 10 ms

  // Restore cursormode on primary LCD controller when needed
  if(_type==LCD40x4) {
    _setCursorAndDisplayMode(_currentMode,_currentCursor);     
  }
                   
  _row=0;          // Reset Cursor location
  _column=0;
}

/** Move cursor to selected row and column
  */
void TextLCD_Base::locate(int column, int row) {
    
   // setAddress() does all the heavy lifting:
   //   check column and row sanity, 
   //   switch controllers for LCD40x4 if needed
   //   switch cursor for LCD40x4 if needed
   //   set the new memory address to show cursor at correct location
   setAddress(column, row);
       
}
    

/** Write a single character (Stream implementation)
  */
int TextLCD_Base::_putc(int value) {
  int addr;
    
    if (value == '\n') {
      //No character to write
      
      //Update Cursor      
      _column = 0;
      _row++;
      if (_row >= rows()) {
        _row = 0;
      }      
    }
    else {
      //Character to write      
      _writeData(value); 
              
      //Update Cursor
      _column++;
      if (_column >= columns()) {
        _column = 0;
        _row++;
        if (_row >= rows()) {
          _row = 0;
        }
      }          
    } //else

    //Set next memoryaddress, make sure cursor blinks at next location
    addr = getAddress(_column, _row);
    _writeCommand(0x80 | addr);
            
    return value;
}


// get a single character (Stream implementation)
int TextLCD_Base::_getc() {
    return -1;
}


// Write a nibble using the 4-bit interface
void TextLCD_Base::_writeNibble(int value) {

// Enable is Low
    this->_setEnable(true);        
    this->_setData(value & 0x0F);   // Low nibble
    wait_us(1); // Data setup time        
    this->_setEnable(false);    
    wait_us(1); // Datahold time

// Enable is Low

}


// Write a byte using the 4-bit interface
void TextLCD_Base::_writeByte(int value) {

// Enable is Low
    this->_setEnable(true);          
    this->_setData(value >> 4);   // High nibble
    wait_us(1); // Data setup time    
    this->_setEnable(false);   
    wait_us(1); // Data hold time
    
    this->_setEnable(true);        
    this->_setData(value >> 0);   // Low nibble
    wait_us(1); // Data setup time        
    this->_setEnable(false);    
    wait_us(1); // Datahold time

// Enable is Low

}

// Write a command byte to the LCD controller
void TextLCD_Base::_writeCommand(int command) {

    this->_setRS(false);        
    wait_us(1);  // Data setup time for RS       
    
    this->_writeByte(command);   
    wait_us(40); // most instructions take 40us            
}

// Write a data byte to the LCD controller
void TextLCD_Base::_writeData(int data) {

    this->_setRS(true);            
    wait_us(1);  // Data setup time for RS 
        
    this->_writeByte(data);
    wait_us(40); // data writes take 40us                
}


#if (0)
// This is the original _address() method.
// It is confusing since it returns the memoryaddress or-ed with the set memorycommand 0x80.
// Left it in here for compatibility with older code. New applications should use getAddress() instead.
// 
int TextLCD_Base::_address(int column, int row) {
    switch (_type) {
        case LCD20x4:
            switch (row) {
                case 0:
                    return 0x80 + column;
                case 1:
                    return 0xc0 + column;
                case 2:
                    return 0x94 + column;
                case 3:
                    return 0xd4 + column;
            }
        case LCD16x2B:
            return 0x80 + (row * 40) + column;
        case LCD16x2:
        case LCD20x2:
        default:
            return 0x80 + (row * 0x40) + column;
    }
}
#endif


// This replaces the original _address() method.
// Left it in here for compatibility with older code. New applications should use getAddress() instead.
int TextLCD_Base::_address(int column, int row) {
  return 0x80 | getAddress(column, row);
}

// This is new method to return the memory address based on row, column and displaytype.
//
int TextLCD_Base::getAddress(int column, int row) {

    switch (_type) {
        case LCD8x1:
            return 0x00 + column;                        

        case LCD8x2B:
            // LCD8x2B is a special layout of LCD16x1
            if (row==0) 
              return 0x00 + column;                        
            else   
              return 0x08 + column;                        


        case LCD16x1:
            // LCD16x1 is a special layout of LCD8x2
            if (column<8) 
              return 0x00 + column;                        
            else   
              return 0x40 + (column - 8);                        

        case LCD12x4:
            switch (row) {
                case 0:
                    return 0x00 + column;
                case 1:
                    return 0x40 + column;
                case 2:
                    return 0x0C + column;
                case 3:
                    return 0x4C + column;
            }

        case LCD16x4:
            switch (row) {
                case 0:
                    return 0x00 + column;
                case 1:
                    return 0x40 + column;
                case 2:
                    return 0x10 + column;
                case 3:
                    return 0x50 + column;
            }

        case LCD20x4:
            switch (row) {
                case 0:
                    return 0x00 + column;
                case 1:
                    return 0x40 + column;
                case 2:
                    return 0x14 + column;
                case 3:
                    return 0x54 + column;
            }

// Special mode for KS0078
        case LCD24x4:
            switch (row) {
                case 0:
                    return 0x00 + column;
                case 1:
                    return 0x20 + column;
                case 2:
                    return 0x40 + column;
                case 3:
                    return 0x60 + column;
            }

// Not sure about this one, seems wrong.
        case LCD16x2B:      
            return 0x00 + (row * 40) + column;
      
        case LCD8x2:               
        case LCD12x2:                
        case LCD16x2:
        case LCD20x2:
        case LCD24x2:        
        case LCD40x2:                
            return 0x00 + (row * 0x40) + column;

        case LCD40x4:                
          // LCD40x4 is a special case since it has 2 controllers
          // Each controller is configured as 40x2
          if (row<2) { 
            // Test to see if we need to switch between controllers  
            if (_ctrl_idx != _LCDCtrl_0) {

              // Second LCD controller Cursor Off
              _setCursorAndDisplayMode(_currentMode, CurOff_BlkOff);    

              // Select primary controller
              _ctrl_idx = _LCDCtrl_0;

              // Restore cursormode on primary LCD controller
              _setCursorAndDisplayMode(_currentMode, _currentCursor);    
            }           
            
            return 0x00 + (row * 0x40) + column;          
          }
          else {

            // Test to see if we need to switch between controllers  
            if (_ctrl_idx != _LCDCtrl_1) {
              // Primary LCD controller Cursor Off
              _setCursorAndDisplayMode(_currentMode, CurOff_BlkOff);    

              // Select secondary controller
              _ctrl_idx = _LCDCtrl_1;

              // Restore cursormode on secondary LCD controller
              _setCursorAndDisplayMode(_currentMode, _currentCursor);    
            }           
                                   
            return 0x00 + ((row-2) * 0x40) + column;          
          } 
            
// Should never get here.
        default:            
            return 0x00;        
    }
}


// Set row, column and update memoryaddress.
//
void TextLCD_Base::setAddress(int column, int row) {
   
// Sanity Check column
    if (column < 0) {
      _column = 0;
    }
    else if (column >= columns()) {
      _column = columns() - 1;
    } else _column = column;
    
// Sanity Check row
    if (row < 0) {
      _row = 0;
    }
    else if (row >= rows()) {
      _row = rows() - 1;
    } else _row = row;
    
    
// Compute the memory address
// For LCD40x4:  switch controllers if needed
//               switch cursor if needed
    int addr = getAddress(_column, _row);
    
    _writeCommand(0x80 | addr);
}

int TextLCD_Base::columns() {
    switch (_type) {
        case LCD8x1:
        case LCD8x2:
        case LCD8x2B:                
            return 8;
        
        case LCD12x2:        
        case LCD12x4:        
            return 12;        

        case LCD16x1:        
        case LCD16x2:
        case LCD16x2B:
        case LCD16x4:        
            return 16;
            
        case LCD20x2:
        case LCD20x4:
            return 20;

        case LCD24x2:
        case LCD24x4:        
            return 24;        

        case LCD40x2:
        case LCD40x4:
            return 40;        
        
// Should never get here.
        default:
            return 0;
    }
}

int TextLCD_Base::rows() {
    switch (_type) {
        case LCD8x1: 
        case LCD16x1:         
            return 1;           

        case LCD8x2:  
        case LCD8x2B:                        
        case LCD12x2:                      
        case LCD16x2:
        case LCD16x2B:
        case LCD20x2:
        case LCD24x2:        
        case LCD40x2:                
            return 2;
                    
        case LCD12x4:        
        case LCD16x4:
        case LCD20x4:
        case LCD24x4:        
        case LCD40x4:
            return 4;

// Should never get here.      
        default:
            return 0;        
    }
}


// Set the Cursor Mode (Cursor Off & Blink Off, Cursor On & Blink Off, Cursor Off & Blink On, Cursor On & Blink On
void TextLCD_Base::setCursor(LCDCursor cursorMode) { 

  // Save new cursor mode, needed when 2 controllers are in use or when display is switched off/on
  _currentCursor = cursorMode;
    
  // Configure only current LCD controller
  _setCursorAndDisplayMode(_currentMode, _currentCursor);
    
}

// Set the Displaymode (On/Off)
void TextLCD_Base::setMode(LCDMode displayMode) { 

  // Save new displayMode, needed when 2 controllers are in use or when cursor is changed
  _currentMode = displayMode;
    
  // Select and configure second LCD controller when needed
  if(_type==LCD40x4) {
    if (_ctrl_idx==_LCDCtrl_0) {      
      // Configure primary LCD controller
      _setCursorAndDisplayMode(_currentMode, _currentCursor);

      // Select 2nd controller
      _ctrl_idx=_LCDCtrl_1;
  
      // Configure secondary LCD controller    
      _setCursorAndDisplayMode(_currentMode, CurOff_BlkOff);

      // Restore current controller
      _ctrl_idx=_LCDCtrl_0;       
    }
    else {
      // Select primary controller
      _ctrl_idx=_LCDCtrl_0;
    
      // Configure primary LCD controller
      _setCursorAndDisplayMode(_currentMode, CurOff_BlkOff);
       
      // Restore current controller
      _ctrl_idx=_LCDCtrl_1;

      // Configure secondary LCD controller    
      _setCursorAndDisplayMode(_currentMode, _currentCursor);

    }
  }
  else {
    // Configure primary LCD controller
    _setCursorAndDisplayMode(_currentMode, _currentCursor);
  }   
    
}


// Set the Displaymode (On/Off) and Cursortype for current controller
void TextLCD_Base::_setCursorAndDisplayMode(LCDMode displayMode, LCDCursor cursorType) { 
    
    // Configure current LCD controller       
    _writeCommand(0x08 | displayMode | cursorType);
}

// Set the Backlight mode (Off/On)
void TextLCD_Base::setBacklight(LCDBacklight backlightMode) {

    if (backlightMode == LightOn) {
      this->_setBL(true);
    }
    else {
      this->_setBL(false);    
    }
} 


void TextLCD_Base::setUDC(unsigned char c, char *udc_data) {
  
  // Select and configure second LCD controller when needed
  if(_type==LCD40x4) {
    _LCDCtrl_Idx current_ctrl_idx = _ctrl_idx; // Temp save current controller
   
    // Select primary controller     
    _ctrl_idx=_LCDCtrl_0;
    
    // Configure primary LCD controller
    _setUDC(c, udc_data);

    // Select 2nd controller
    _ctrl_idx=_LCDCtrl_1;
  
    // Configure secondary LCD controller    
    _setUDC(c, udc_data);

    // Restore current controller
    _ctrl_idx=current_ctrl_idx;       
  }
  else {
    // Configure primary LCD controller
    _setUDC(c, udc_data); 
  }
    
}

void TextLCD_Base::_setUDC(unsigned char c, char *udc_data) {
  
  // Select CG RAM for current LCD controller
  _writeCommand(0x40 + ((c & 0x07) << 3)); //Set CG-RAM address,
                                           //8 sequential locations needed per UDC
  // Store UDC pattern 
  for (int i=0; i<8; i++) {
    _writeData(*udc_data++);
  }
   
  //Select DD RAM again for current LCD controller
  int addr = getAddress(_column, _row);
  _writeCommand(0x80 | addr);
  
}

//--------- End TextLCD_Base -----------



//--------- Start TextLCD Bus -----------

/* Create a TextLCD interface for using regular mbed pins
 *
 * @param rs     Instruction/data control line
 * @param e      Enable line (clock)
 * @param d4-d7  Data lines for using as a 4-bit interface
 * @param type   Sets the panel size/addressing mode (default = LCD16x2)
 * @param bl     Backlight control line (optional, default = NC)  
 * @param e2     Enable2 line (clock for second controller, LCD40x4 only) 
 * @param ctrl   LCD controller (default = HD44780)   
 */ 
TextLCD::TextLCD(PinName rs, PinName e,
                 PinName d4, PinName d5, PinName d6, PinName d7,
                 LCDType type, PinName bl, PinName e2, LCDCtrl ctrl) :
                 TextLCD_Base(type, ctrl), 
                 _rs(rs), _e(e), _d(d4, d5, d6, d7) {

  // The hardware Backlight pin is optional. Test and make sure whether it exists or not to prevent illegal access.
  if (bl != NC) {
    _bl = new DigitalOut(bl);   //Construct new pin 
    _bl->write(0);              //Deactivate    
  }
  else {
    // No Hardware Backlight pin       
    _bl = NULL;                 //Construct dummy pin     
  }  

  // The hardware Enable2 pin is only needed for LCD40x4. Test and make sure whether it exists or not to prevent illegal access.
  if (e2 != NC) {
    _e2 = new DigitalOut(e2);   //Construct new pin 
    _e2->write(0);              //Deactivate    
  }
  else {
    // No Hardware Enable pin       
    _e2 = NULL;                 //Construct dummy pin     
  }  
                                                                           
  _init();

}

/** Set E pin (or E2 pin)
  * Used for mbed pins, I2C bus expander or SPI shiftregister
  * Default PinName value for E2 is NC, must be used as pointer to avoid issues with mbed lib and DigitalOut pins
  *   @param  value true or false
  *   @return none 
  */
void TextLCD::_setEnable(bool value) {

  if(_ctrl_idx==_LCDCtrl_0) {
    if (value) {
      _e  = 1;    // Set E bit 
    }  
    else { 
      _e  = 0;    // Reset E bit  
    }  
  }    
  else { 
    if (value) {
      if (_e2 != NULL) {_e2->write(1);}  //Set E2 bit
    }  
    else { 
      if (_e2 != NULL) {_e2->write(0);}  //Reset E2 bit     
    }  
  }    

}    

// Set RS pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD::_setRS(bool value) {

  if (value) {
    _rs  = 1;    // Set RS bit 
  }  
  else  {
    _rs  = 0;    // Reset RS bit 
  }  

}    

/** Set BL pin
  * Used for mbed pins, I2C bus expander or SPI shiftregister
  * Default PinName value is NC, must be used as pointer to avoid issues with mbed lib and DigitalOut pins
  *   @param  value true or false
  *   @return none  
  */
void TextLCD::_setBL(bool value) {

  if (value) {
    if (_bl != NULL) {_bl->write(1);}  //Set BL bit
  }  
  else { 
    if (_bl != NULL) {_bl->write(0);}  //Reset BL bit  
  }  

}    

// Place the 4bit data on the databus
// Used for mbed pins, I2C bus expander or SPI shifregister
void TextLCD::_setData(int value) {
  _d = value & 0x0F;   // Write Databits 
}    

/** Destruct a TextLCD interface for using regular mbed pins
  *
  * @param  none
  * @return none
  */ 
TextLCD::~TextLCD() {
   if (_bl != NULL) {delete _bl;}  // BL pin
   if (_e2 != NULL) {delete _e2;}  // E2 pin
}

    
//----------- End TextLCD ---------------


//--------- Start TextLCD_I2C -----------

/** Create a TextLCD interface using an I2C PC8574 or PCF8574A portexpander
  *
  * @param i2c             I2C Bus
  * @param deviceAddress   I2C slave address (PCF8574 or PCF8574A, default = 0x40)
  * @param type            Sets the panel size/addressing mode (default = LCD16x2)
  * @param ctrl            LCD controller (default = HD44780)    
  */
TextLCD_I2C::TextLCD_I2C(I2C *i2c, char deviceAddress, LCDType type, LCDCtrl ctrl) :
                         TextLCD_Base(type, ctrl), 
                         _i2c(i2c){
                              
  _slaveAddress = deviceAddress & 0xFE;
  
  // Init the portexpander bus
  _lcd_bus = D_LCD_BUS_DEF;
  
  // write the new data to the portexpander
  _i2c->write(_slaveAddress, &_lcd_bus, 1);    

  _init();
    
}

// Set E pin (or E2 pin)
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_I2C::_setEnable(bool value) {

  if(_ctrl_idx==_LCDCtrl_0) {
    if (value)
      _lcd_bus |= D_LCD_E;     // Set E bit 
    else                     
      _lcd_bus &= ~D_LCD_E;    // Reset E bit                     
  }
  else {
    if (value)
      _lcd_bus |= D_LCD_E2;    // Set E2 bit 
    else                     
      _lcd_bus &= ~D_LCD_E2;   // Reset E2bit                     
    }    

  // write the new data to the I2C portexpander
  _i2c->write(_slaveAddress, &_lcd_bus, 1);    

}    

// Set RS pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_I2C::_setRS(bool value) {

  if (value)
    _lcd_bus |= D_LCD_RS;    // Set RS bit 
  else                     
    _lcd_bus &= ~D_LCD_RS;   // Reset RS bit                     

  // write the new data to the I2C portexpander
  _i2c->write(_slaveAddress, &_lcd_bus, 1);    
                  
}    

// Set BL pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_I2C::_setBL(bool value) {

  if (value)
    _lcd_bus |= D_LCD_BL;    // Set BL bit 
  else                     
    _lcd_bus &= ~D_LCD_BL;   // Reset BL bit                     

  // write the new data to the I2C portexpander
  _i2c->write(_slaveAddress, &_lcd_bus, 1);    
                 
}    



// Place the 4bit data on the databus
// Used for mbed pins, I2C bus expander or SPI shifregister
void TextLCD_I2C::_setData(int value) {
  int data;

  // Set bit by bit to support any mapping of expander portpins to LCD pins
  
  data = value & 0x0F;
  if (data & 0x01)
    _lcd_bus |= D_LCD_D4;   // Set Databit 
  else                     
    _lcd_bus &= ~D_LCD_D4;  // Reset Databit                     

  if (data & 0x02)
    _lcd_bus |= D_LCD_D5;   // Set Databit 
  else                     
    _lcd_bus &= ~D_LCD_D5;  // Reset Databit                     

  if (data & 0x04)
    _lcd_bus |= D_LCD_D6;   // Set Databit 
  else                     
    _lcd_bus &= ~D_LCD_D6;  // Reset Databit                     

  if (data & 0x08)
    _lcd_bus |= D_LCD_D7;   // Set Databit 
  else                     
    _lcd_bus &= ~D_LCD_D7;  // Reset Databit                     
                    
  // write the new data to the I2C portexpander
  _i2c->write(_slaveAddress, &_lcd_bus, 1);  
                 
}    

//---------- End TextLCD_I2C ------------



//--------- Start TextLCD_SPI -----------

 /** Create a TextLCD interface using an SPI 74595 portexpander
   *
   * @param spi             SPI Bus
   * @param cs              chip select pin (active low)
   * @param type            Sets the panel size/addressing mode (default = LCD16x2)
   * @param ctrl            LCD controller (default = HD44780)      
   */
TextLCD_SPI::TextLCD_SPI(SPI *spi, PinName cs, LCDType type, LCDCtrl ctrl) :
                         TextLCD_Base(type, ctrl), 
                         _spi(spi),        
                         _cs(cs) {      
        
  // Setup the spi for 8 bit data, low steady state clock,
  // rising edge capture, with a 500KHz or 1MHz clock rate  
  _spi->format(8,0);
  _spi->frequency(500000);    
  //_spi.frequency(1000000);    


  // Init the portexpander bus
  _lcd_bus = D_LCD_BUS_DEF;
  
  // write the new data to the portexpander
  _setCS(false);  
  _spi->write(_lcd_bus);   
  _setCS(true);  
  
  _init();
    
}

// Set E pin (or E2 pin)
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_SPI::_setEnable(bool value) {

  if(_ctrl_idx==_LCDCtrl_0) {
    if (value)
      _lcd_bus |= D_LCD_E;     // Set E bit 
    else                     
      _lcd_bus &= ~D_LCD_E;    // Reset E bit                     
  }
  else {
    if (value)
      _lcd_bus |= D_LCD_E2;    // Set E2 bit 
    else                     
      _lcd_bus &= ~D_LCD_E2;   // Reset E2 bit                     
  }
                  
  // write the new data to the SPI portexpander
  _setCS(false);  
  _spi->write(_lcd_bus);   
  _setCS(true);  
  
}    

// Set RS pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_SPI::_setRS(bool value) {

  if (value) {
    _lcd_bus |= D_LCD_RS;    // Set RS bit 
  }  
  else {                    
    _lcd_bus &= ~D_LCD_RS;   // Reset RS bit                     
  }
     
  // write the new data to the SPI portexpander
  _setCS(false);  
  _spi->write(_lcd_bus);   
  _setCS(true);     

}    

// Set BL pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_SPI::_setBL(bool value) {

  if (value) {
    _lcd_bus |= D_LCD_BL;    // Set BL bit 
  }  
  else {
    _lcd_bus &= ~D_LCD_BL;   // Reset BL bit                     
  }
      
  // write the new data to the SPI portexpander
  _setCS(false);  
  _spi->write(_lcd_bus);   
  _setCS(true);  
     
}    



// Place the 4bit data on the databus
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_SPI::_setData(int value) {
  int data;

  // Set bit by bit to support any mapping of expander portpins to LCD pins
    
  data = value & 0x0F;
  if (data & 0x01)
    _lcd_bus |= D_LCD_D4;   // Set Databit 
  else                     
    _lcd_bus &= ~D_LCD_D4;  // Reset Databit                     

  if (data & 0x02)
    _lcd_bus |= D_LCD_D5;   // Set Databit 
  else                     
    _lcd_bus &= ~D_LCD_D5;  // Reset Databit                     

  if (data & 0x04)
    _lcd_bus |= D_LCD_D6;   // Set Databit 
  else                     
    _lcd_bus &= ~D_LCD_D6;  // Reset Databit                     

  if (data & 0x08)
    _lcd_bus |= D_LCD_D7;   // Set Databit 
  else                     
    _lcd_bus &= ~D_LCD_D7;  // Reset Databit                     
                    
  // write the new data to the SPI portexpander
  _setCS(false);  
  _spi->write(_lcd_bus);   
  _setCS(true);  
        
}    


// Set CS line.
// Only used for SPI bus
void TextLCD_SPI::_setCS(bool value) {

  if (value) {   
    _cs  = 1;    // Set CS pin 
  }  
  else {
    _cs  = 0;    // Reset CS pin 
  }
}

//---------- End TextLCD_SPI ------------


//--------- Start TextLCD_SPI_N ---------

 /** Create a TextLCD interface using a controller with a native SPI interface
   *
   * @param spi             SPI Bus
   * @param cs              chip select pin (active low)
   * @param rs              Instruction/data control line
   * @param type            Sets the panel size/addressing mode (default = LCD16x2)
   * @param bl              Backlight control line (optional, default = NC)  
   * @param ctrl            LCD controller (default = ST7032) 
   */       
TextLCD_SPI_N::TextLCD_SPI_N(SPI *spi, PinName cs, PinName rs, LCDType type, PinName bl, LCDCtrl ctrl) :
                             TextLCD_Base(type, ctrl), 
                             _spi(spi),        
                             _cs(cs),
                             _rs(rs) {      
        
  // Setup the spi for 8 bit data, low steady state clock,
  // rising edge capture, with a 500KHz or 1MHz clock rate  
  _spi->format(8,0);
  _spi->frequency(1000000);    
  
  // The hardware Backlight pin is optional. Test and make sure whether it exists or not to prevent illegal access.
  if (bl != NC) {
    _bl = new DigitalOut(bl);   //Construct new pin 
    _bl->write(0);              //Deactivate    
  }
  else {
    // No Hardware Backlight pin       
    _bl = NULL;                 //Construct dummy pin     
  }  
  
  _writeByte( 0x39 );  //FUNCTION SET 8 bit,N=1 2-line display mode,5*7dot IS=1
  wait_us(30);
  _init();
}

TextLCD_SPI_N::~TextLCD_SPI_N() {
   if (_bl != NULL) {delete _bl;}  // BL pin
}

// Not used in this mode
void TextLCD_SPI_N::_setEnable(bool value) {
}    

// Set RS pin
// Used for mbed pins, I2C bus expander or SPI shiftregister
void TextLCD_SPI_N::_setRS(bool value) {
    _rs = value;
}    

// Set BL pin
void TextLCD_SPI_N::_setBL(bool value) {
    if (_bl)
        _bl->write(value);   
}    

// Write a byte using SPI
void TextLCD_SPI_N::_writeByte(int value) {
    _cs = 0;
    wait_us(1);
    _spi->write(value);
    wait_us(1);
    _cs = 1;
}
    
// Not used in this mode
void TextLCD_SPI_N::_setData(int value) {
}    


//-------- End TextLCD_SPI_N ------------





