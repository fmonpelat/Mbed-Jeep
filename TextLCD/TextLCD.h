/* mbed TextLCD Library, for a 4-bit LCD based on HD44780
 * Copyright (c) 2007-2010, sford, http://mbed.org
 *               2013, v01: WH, Added LCD types, fixed LCD address issues, added Cursor and UDCs 
 *               2013, v02: WH, Added I2C and SPI bus interfaces
 *               2013, v03: WH, Added support for LCD40x4 which uses 2 controllers   
 *               2013, v04: WH, Added support for Display On/Off, improved 4bit bootprocess  
 *               2013, v05: WH, Added support for 8x2B, added some UDCs  
 *               2013, v06: WH, Added support for devices that use internal DC/DC converters 
 *               2013, v07: WH, Added support for backlight and include portdefinitions for LCD2004 Module from DFROBOT
 *               2014, v08: WH, Refactored in Base and Derived Classes to deal with mbed lib change regarding 'NC' defined DigitalOut pins
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

#ifndef MBED_TEXTLCD_H
#define MBED_TEXTLCD_H

#include "mbed.h"


/** A TextLCD interface for driving 4-bit HD44780-based LCDs
 *
 * Currently supports 8x1, 8x2, 12x4, 16x1, 16x2, 16x4, 20x2, 20x4, 24x2, 24x4, 40x2 and 40x4 panels
 * Interface options include direct mbed pins, I2C portexpander (PCF8474, PCF8574A) or SPI bus shiftregister (74595)
 * Supports some controllers that provide internal DC/DC converters for VLCD or VLED. 
 *
 * @code
 * #include "mbed.h"
 * #include "TextLCD.h"
 * 
 * // I2C Communication
 * I2C i2c_lcd(p28,p27); // SDA, SCL
 *
 * // SPI Communication
 * SPI spi_lcd(p5, NC, p7); // MOSI, MISO, SCLK
 *
 * //TextLCD lcd(p15, p16, p17, p18, p19, p20);                          // RS, E, D4-D7, LCDType=LCD16x2, BL=NC, E2=NC, LCDTCtrl=HD44780
 * //TextLCD_SPI lcd(&spi_lcd, p8, TextLCD::LCD40x4);                    // SPI bus, CS pin, LCD Type  
 * TextLCD_I2C lcd(&i2c_lcd, 0x42, TextLCD::LCD20x4);                  // I2C bus, PCF8574 Slaveaddress, LCD Type
 * //TextLCD_I2C lcd(&i2c_lcd, 0x42, TextLCD::LCD16x2, TextLCD::WS0010); // I2C bus, PCF8574 Slaveaddress, LCD Type, Device Type
 * //TextLCD_SPI_N lcd(&spi_lcd, p8, p9);                                  // SPI bus, CS pin, RS pin, LCDType=LCD16x2, BL=NC, LCDTCtrl=ST7032   
 * 
 * int main() {
 *   lcd.printf("Hello World!\n");
 * }
 * @endcode
 */

//Pin Defines for I2C PCF8574 and SPI 74595 Bus interfaces
//LCD and serial portexpanders should be wired accordingly 
//
// JEEP: nos salteamos esta parte ya que el hardware que tenemos es otro.
#if (0)
//Definitions for hardware used by WH 
//Note: LCD RW pin must be connected to GND
//      E2 is used for LCD40x4 (second controller)
//      BL may be used for future expansion to control backlight
#define D_LCD_PIN_D4   0
#define D_LCD_PIN_D5   1
#define D_LCD_PIN_D6   2
#define D_LCD_PIN_D7   3
#define D_LCD_PIN_RS   4
#define D_LCD_PIN_E    5
#define D_LCD_PIN_E2   6
#define D_LCD_PIN_BL   7

#define D_LCD_PIN_RW   D_LCD_PIN_E2

#else

//Definitions for LCD2004 Module from DFROBOT, See http://arduino-info.wikispaces.com/LCD-Blue-I2C
//This hardware is different from earlier/different Arduino I2C LCD displays
//Note: LCD RW pin must be kept LOW
//      E2 is not available on default Arduino hardware and does not support LCD40x4 (second controller)
//      BL is used to control backlight
#define D_LCD_PIN_RS   0
#define D_LCD_PIN_RW   1
#define D_LCD_PIN_E    2
#define D_LCD_PIN_BL   3
#define D_LCD_PIN_D4   4
#define D_LCD_PIN_D5   5
#define D_LCD_PIN_D6   6
#define D_LCD_PIN_D7   7

#define D_LCD_PIN_E2   D_LCD_PIN_RW
#endif

//Bitpattern Defines for I2C PCF8574 and SPI 74595 Bus
//
#define D_LCD_D4       (1<<D_LCD_PIN_D4)
#define D_LCD_D5       (1<<D_LCD_PIN_D5)
#define D_LCD_D6       (1<<D_LCD_PIN_D6)
#define D_LCD_D7       (1<<D_LCD_PIN_D7)
#define D_LCD_RS       (1<<D_LCD_PIN_RS)
#define D_LCD_E        (1<<D_LCD_PIN_E)
#define D_LCD_E2       (1<<D_LCD_PIN_E2)
#define D_LCD_BL       (1<<D_LCD_PIN_BL)
//#define D_LCD_RW       (1<<D_LCD_PIN_RW)


#define D_LCD_BUS_MSK  (D_LCD_D4 | D_LCD_D5 | D_LCD_D6 | D_LCD_D7)
#define D_LCD_BUS_DEF  0x00


/** Some sample User Defined Chars 5x7 dots */
const char udc_ae[] = {0x00, 0x00, 0x1B, 0x05, 0x1F, 0x14, 0x1F, 0x00};  //æ
const char udc_0e[] = {0x00, 0x00, 0x0E, 0x13, 0x15, 0x19, 0x0E, 0x00};  //ø
const char udc_ao[] = {0x0E, 0x0A, 0x0E, 0x01, 0x0F, 0x11, 0x0F, 0x00};  //å
const char udc_AE[] = {0x0F, 0x14, 0x14, 0x1F, 0x14, 0x14, 0x17, 0x00};  //Æ
const char udc_0E[] = {0x0E, 0x13, 0x15, 0x15, 0x15, 0x19, 0x0E, 0x00};  //Ø
const char udc_Ao[] = {0x0E, 0x0A, 0x0E, 0x11, 0x1F, 0x11, 0x11, 0x00};  //Å
const char udc_PO[] = {0x04, 0x0A, 0x0A, 0x1F, 0x1B, 0x1B, 0x1F, 0x00};  //Padlock Open
const char udc_PC[] = {0x1C, 0x10, 0x08, 0x1F, 0x1B, 0x1B, 0x1F, 0x00};  //Padlock Closed

const char udc_0[]  = {0x18, 0x14, 0x12, 0x11, 0x12, 0x14, 0x18, 0x00};  // |>
const char udc_1[]  = {0x03, 0x05, 0x09, 0x11, 0x09, 0x05, 0x03, 0x00};  // <|
const char udc_2[]  = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00};  // |
const char udc_3[]  = {0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00};  // ||
const char udc_4[]  = {0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x00};  // |||
const char udc_5[]  = {0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00};  // =
const char udc_6[]  = {0x15, 0x0a, 0x15, 0x0a, 0x15, 0x0a, 0x15, 0x00};  // checkerboard
const char udc_7[]  = {0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00, 0x00};  // \

const char udc_degr[]   = {0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00};  // Degree symbol

const char udc_TM_T[]   = {0x1F, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00};  // Trademark T
const char udc_TM_M[]   = {0x11, 0x1B, 0x15, 0x11, 0x00, 0x00, 0x00, 0x00};  // Trademark M

//const char udc_Bat_Hi[] = {0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00};  // Battery Full
//const char udc_Bat_Ha[] = {0x0E, 0x11, 0x13, 0x17, 0x1F, 0x1F, 0x1F, 0x00};  // Battery Half
//const char udc_Bat_Lo[] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F, 0x00};  // Battery Low
const char udc_Bat_Hi[] = {0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00};  // Battery Full
const char udc_Bat_Ha[] = {0x0E, 0x11, 0x11, 0x1F, 0x1F, 0x1F, 0x1F, 0x00};  // Battery Half
const char udc_Bat_Lo[] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x1F, 0x1F, 0x00};  // Battery Low
const char udc_AC[]     = {0x0A, 0x0A, 0x1F, 0x11, 0x0E, 0x04, 0x04, 0x00};  // AC Power

//const char udc_smiley[] = {0x00, 0x0A, 0x00, 0x04, 0x11, 0x0E, 0x00, 0x00};  // Smiley
//const char udc_droopy[] = {0x00, 0x0A, 0x00, 0x04, 0x00, 0x0E, 0x11, 0x00};  // Droopey
//const char udc_note[]   = {0x01, 0x03, 0x05, 0x09, 0x0B, 0x1B, 0x18, 0x00};  // Note

//const char udc_bar_1[]  = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00};  // Bar 1
//const char udc_bar_2[]  = {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00};  // Bar 11
//const char udc_bar_3[]  = {0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x00};  // Bar 111
//const char udc_bar_4[]  = {0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x00};  // Bar 1111
//const char udc_bar_5[]  = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00};  // Bar 11111
const char udc_bar_6[]  = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};

/** A TextLCD interface for driving 4-bit HD44780-based LCDs
 *
 * @brief Currently supports 8x1, 8x2, 12x2, 12x4, 16x1, 16x2, 16x4, 20x2, 20x4, 24x2, 24x4, 40x2 and 40x4 panels
 *        Interface options include direct mbed pins, I2C portexpander (PCF8474, PCF8574A) or SPI bus shiftregister (74595) 
 *
 */
class TextLCD_Base : public Stream {
public:

    /** LCD panel format */
    enum LCDType {
        LCD8x1,     /**<  8x1 LCD panel */    
        LCD8x2,     /**<  8x2 LCD panel */          
        LCD8x2B,    /**<  8x2 LCD panel (actually 16x1) */                  
        LCD12x2,    /**< 12x2 LCD panel */                          
        LCD12x4,    /**< 12x4 LCD panel */                  
        LCD16x1,    /**< 16x1 LCD panel (actually 8x2) */          
        LCD16x2,    /**< 16x2 LCD panel (default) */
        LCD16x2B,   /**< 16x2 LCD panel alternate addressing */
        LCD16x4,    /**< 16x4 LCD panel */        
        LCD20x2,    /**< 20x2 LCD panel */
        LCD20x4,    /**< 20x4 LCD panel */
        LCD24x2,    /**< 24x2 LCD panel */        
        LCD24x4,    /**< 24x4 LCD panel, special mode KS0078 */                
        LCD40x2,    /**< 40x2 LCD panel */                
        LCD40x4     /**< 40x4 LCD panel, Two controller version */                        
    };

    /** LCD Controller Device */
    enum LCDCtrl {
        HD44780,    /**<  HD44780 (default)      */    
        WS0010,     /**<  WS0010 OLED Controller */    
        ST7036,      /**<  ST7036                 */   
        ST7032      /**<  ST7032                 */   
    };


    /** LCD Cursor control */
    enum LCDCursor {
        CurOff_BlkOff = 0x00,  /**<  Cursor Off, Blinking Char Off */    
        CurOn_BlkOff  = 0x02,  /**<  Cursor On, Blinking Char Off */    
        CurOff_BlkOn  = 0x01,  /**<  Cursor Off, Blinking Char On */    
        CurOn_BlkOn   = 0x03   /**<  Cursor On, Blinking Char On */    
    };


    /** LCD Display control */
    enum LCDMode {
        DispOff = 0x00,  /**<  Display Off */    
        DispOn  = 0x04   /**<  Display On */            
    };

   /** LCD Backlight control */
    enum LCDBacklight {
        LightOff,        /**<  Backlight Off */    
        LightOn          /**<  Backlight On */            
    };


#if DOXYGEN_ONLY
    /** Write a character to the LCD
     *
     * @param c The character to write to the display
     */
    int putc(int c);

    /** Write a formated string to the LCD
     *
     * @param format A printf-style format string, followed by the
     *               variables to use in formating the string.
     */
    int printf(const char* format, ...);
#endif

    /** Locate to a screen column and row
     *
     * @param column  The horizontal position from the left, indexed from 0
     * @param row     The vertical position from the top, indexed from 0
     */
    void locate(int column, int row);


    /** Return the memoryaddress of screen column and row location
     *
     * @param column  The horizontal position from the left, indexed from 0
     * @param row     The vertical position from the top, indexed from 0
     * @param return  The memoryaddress of screen column and row location
     */
    int  getAddress(int column, int row);    
    
    
    /** Set the memoryaddress of screen column and row location
     *
     * @param column  The horizontal position from the left, indexed from 0
     * @param row     The vertical position from the top, indexed from 0
     */
    void setAddress(int column, int row);        


    /** Clear the screen and locate to 0,0
     */
    void cls();

    /** Return the number of rows
     *
     * @param return  The number of rows
     */
    int rows();
    
    /** Return the number of columns
     *
     * @param return  The number of columns
     */  
    int columns();  

    /** Set the Cursormode
     *
     * @param cursorMode  The Cursor mode (CurOff_BlkOff, CurOn_BlkOff, CurOff_BlkOn, CurOn_BlkOn)
     */
    void setCursor(LCDCursor cursorMode);     
    

    /** Set the Displaymode
     *
     * @param displayMode The Display mode (DispOff, DispOn)
     */
    void setMode(LCDMode displayMode);     

    /** Set the Backlight mode
     *
     *  @param backlightMode The Backlight mode (LightOff, LightOn)
     */
    void setBacklight(LCDBacklight backlightMode); 


    /** Set User Defined Characters
     *
     * @param unsigned char c   The Index of the UDC (0..7)
     * @param char *udc_data    The bitpatterns for the UDC (8 bytes of 5 significant bits)     
     */
    void setUDC(unsigned char c, char *udc_data);


protected:

   /** LCD controller select, mainly used for LCD40x4
     */
    enum _LCDCtrl_Idx {
        _LCDCtrl_0,  /*<  Primary */    
        _LCDCtrl_1,  /*<  Secondary */            
    };

    /** Create a TextLCD_Base interface
      * @brief Base class, can not be instantiated
      *
      * @param type  Sets the panel size/addressing mode (default = LCD16x2)
      * @param ctrl  LCD controller (default = HD44780)           
      */
    TextLCD_Base(LCDType type = LCD16x2, LCDCtrl ctrl = HD44780);

    
    // Stream implementation functions
    virtual int _putc(int value);
    virtual int _getc();

/** Low level methods for LCD controller
  */
    void _init();    
    void _initCtrl();    
    int  _address(int column, int row);
    void _setCursor(LCDCursor show);
    void _setUDC(unsigned char c, char *udc_data);   
    void _setCursorAndDisplayMode(LCDMode displayMode, LCDCursor cursorType);       
    
/** Low level write operations to LCD controller
  */
    void _writeNibble(int value);
    virtual void _writeByte(int value);
    void _writeCommand(int command);
    void _writeData(int data);

/** Pure Virtual Low level writes to LCD Bus (serial or parallel)
  */
    virtual void _setEnable(bool value) = 0;
    virtual void _setRS(bool value) = 0;  
    virtual void _setBL(bool value) = 0;
    virtual void _setData(int value) = 0;

    
//Display type
    LCDType _type;

//Display mode
    LCDMode _currentMode;

//Controller type 
    LCDCtrl _ctrl;    

//Controller select, mainly used for LCD40x4 
    _LCDCtrl_Idx _ctrl_idx;    

// Cursor
    int _column;
    int _row;
    LCDCursor _currentCursor;    
};

//--------- End TextLCD_Base -----------



//--------- Start TextLCD Bus -----------

/** Create a TextLCD interface for using regular mbed pins
  *
  */
class TextLCD : public TextLCD_Base {
public:    
    /** Create a TextLCD interface for using regular mbed pins
     *
     * @param rs    Instruction/data control line
     * @param e     Enable line (clock)
     * @param d4-d7 Data lines for using as a 4-bit interface
     * @param type  Sets the panel size/addressing mode (default = LCD16x2)
     * @param bl    Backlight control line (optional, default = NC)      
     * @param e2    Enable2 line (clock for second controller, LCD40x4 only)  
     * @param ctrl  LCD controller (default = HD44780)           
     */
    TextLCD(PinName rs, PinName e, PinName d4, PinName d5, PinName d6, PinName d7, LCDType type = LCD16x2, PinName bl = NC, PinName e2 = NC, LCDCtrl ctrl = HD44780);


   /** Destruct a TextLCD interface for using regular mbed pins
     *
     * @param  none
     * @return none
     */ 
    virtual ~TextLCD();

private:    
//Low level writes to LCD Bus (serial or parallel)
    virtual void _setEnable(bool value);
    virtual void _setRS(bool value);  
    virtual void _setBL(bool value);
    virtual void _setData(int value);

/** Regular mbed pins bus
  */
    DigitalOut _rs, _e;
    BusOut _d;
    
/** Optional Hardware pins for the Backlight and LCD40x4 device
  * Default PinName value is NC, must be used as pointer to avoid issues with mbed lib and DigitalOut pins
  */
    DigitalOut *_bl, *_e2;       
};

    
//----------- End TextLCD ---------------


//--------- Start TextLCD_I2C -----------


/** Create a TextLCD interface using an I2C PC8574 or PCF8574A portexpander
  *
  */
class TextLCD_I2C : public TextLCD_Base {    
public:
    /** Create a TextLCD interface using an I2C PC8574 or PCF8574A portexpander
     *
     * @param i2c             I2C Bus
     * @param deviceAddress   I2C slave address (PCF8574 or PCF8574A, default = 0x40)
     * @param type            Sets the panel size/addressing mode (default = LCD16x2)
     * @param ctrl            LCD controller (default = HD44780)                
     */
    TextLCD_I2C(I2C *i2c, char deviceAddress = 0x40, LCDType type = LCD16x2, LCDCtrl ctrl = HD44780);

private:
//Low level writes to LCD Bus (serial or parallel)
    virtual void _setEnable(bool value);
    virtual void _setRS(bool value);  
    virtual void _setBL(bool value);
    virtual void _setData(int value);   
  
//I2C bus
    I2C *_i2c;
    char _slaveAddress;
    
// Internal bus mirror value for serial bus only
    char _lcd_bus;   
    
};


//---------- End TextLCD_I2C ------------



//--------- Start TextLCD_SPI -----------


/** Create a TextLCD interface using an SPI 74595 portexpander
  *
  */
class TextLCD_SPI : public TextLCD_Base {    
public:
    /** Create a TextLCD interface using an SPI 74595 portexpander
     *
     * @param spi             SPI Bus
     * @param cs              chip select pin (active low)
     * @param type            Sets the panel size/addressing mode (default = LCD16x2)
     * @param ctrl            LCD controller (default = HD44780)                     
     */
    TextLCD_SPI(SPI *spi, PinName cs, LCDType type = LCD16x2, LCDCtrl ctrl = HD44780);


private:
//Low level writes to LCD Bus (serial or parallel)
    virtual void _setEnable(bool value);
    virtual void _setRS(bool value);  
    virtual void _setBL(bool value);
    virtual void _setData(int value);
    virtual void _setCS(bool value);
    
//Low level writes to LCD serial bus only
    void _writeBus();      
   
// SPI bus        
    SPI *_spi;
    DigitalOut _cs;    
    
// Internal bus mirror value for serial bus only
    char _lcd_bus;   

};

//---------- End TextLCD_SPI ------------


//--------- Start TextLCD_NativeSPI -----------


/** Create a TextLCD interface using a controller with native SPI interface
  *
  */
class TextLCD_SPI_N : public TextLCD_Base {    
public:
    /** Create a TextLCD interface using a controller with native SPI interface
     *
     * @param spi             SPI Bus
     * @param cs              chip select pin (active low)
     * @param rs              Instruction/data control line
     * @param type            Sets the panel size/addressing mode (default = LCD16x2)
     * @param bl              Backlight control line (optional, default = NC)  
     * @param ctrl            LCD controller (default = ST7032)                     
     */
    TextLCD_SPI_N(SPI *spi, PinName cs, PinName rs, LCDType type = LCD16x2, PinName bl = NC, LCDCtrl ctrl = ST7032);
    virtual ~TextLCD_SPI_N(void);

private:
    virtual void _setEnable(bool value);
    virtual void _setRS(bool value);  
    virtual void _setBL(bool value);
    virtual void _setData(int value);
    virtual void _writeByte(int value);
   
// SPI bus        
    SPI *_spi;
    DigitalOut _cs;    
    DigitalOut _rs;
    DigitalOut *_bl;

};

//---------- End TextLCD_SPI_N ------------

#endif
