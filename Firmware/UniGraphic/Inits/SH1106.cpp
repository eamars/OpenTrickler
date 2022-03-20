/* mbed UniGraphic library - Device specific class
 * SH1106 by Karl Zweimüller, based on
 * SSD1306 by Copyright (c) 2015 Peter Drescher
 * Released under the MIT License: http://mbed.org/license/mit
 */

#include "Protocols.h"
#include "SH1106.h"

//////////////////////////////////////////////////////////////////////////////////
// display settings ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


#define IC_X_SEGS    132 // 132 SEG
#define IC_Y_COMS    64  // 64  COM

#define SH1106_SETCONTRAST 0x81
#define SH1106_DISPLAYALLON_RESUME 0xA4
#define SH1106_DISPLAYALLON 0xA5
#define SH1106_NORMALDISPLAY 0xA6
#define SH1106_INVERTDISPLAY 0xA7
#define SH1106_DISPLAYOFF 0xAE
#define SH1106_DISPLAYON 0xAF

#define SH1106_SETDISPLAYOFFSET 0xD3
#define SH1106_SETCOMPINS 0xDA

#define SH1106_SETVCOMDETECT 0xDB

#define SH1106_SETDISPLAYCLOCKDIV 0xD5
#define SH1106_SETPRECHARGE 0xD9

#define SH1106_SETMULTIPLEX 0xA8

#define SH1106_SETLOWCOLUMN 0x00
#define SH1106_SETHIGHCOLUMN 0x10

#define SH1106_SETSTARTLINE 0x40

#define SH1106_MEMORYMODE 0x20
#define SH1106_COLUMNADDR 0x21
#define SH1106_PAGEADDR   0x22

#define SH1106_COMSCANINC 0xC0
#define SH1106_COMSCANDEC 0xC8

#define SH1106_SEGREMAP 0xA0

#define SH1106_CHARGEPUMP 0x8D

#define SH1106_EXTERNALVCC 0x1
#define SH1106_SWITCHCAPVCC 0x2


SH1106::SH1106(proto_t displayproto, PortName port, PinName CS, PinName reset, PinName DC, PinName WR, PinName RD, const char *name , unsigned int LCDSIZE_X, unsigned  int LCDSIZE_Y)
    : LCD(displayproto, port, CS, reset, DC, WR, RD, LCDSIZE_X, LCDSIZE_Y, IC_X_SEGS, IC_Y_COMS, name)
{
    hw_reset();
    BusEnable(true);
    init();
    cls();
    set_orientation(1);
    locate(0,0);
    copy_to_lcd();
}
SH1106::SH1106(proto_t displayproto, int Hz, PinName mosi, PinName miso, PinName sclk, PinName CS, PinName reset, PinName DC, const char *name, unsigned int LCDSIZE_X, unsigned  int LCDSIZE_Y)
    : LCD(displayproto, Hz, mosi, miso, sclk, CS, reset, DC, LCDSIZE_X, LCDSIZE_Y, IC_X_SEGS, IC_Y_COMS, name)
{
    hw_reset();
    BusEnable(true);
    init();
    cls();
    set_orientation(1);
    locate(0,0);
    copy_to_lcd();
}

SH1106::SH1106(proto_t displayproto, int Hz, int address, PinName sda, PinName scl, PinName reset, const char* name , unsigned int LCDSIZE_X, unsigned  int LCDSIZE_Y)
    : LCD(displayproto, Hz, address, sda, scl,reset, LCDSIZE_X, LCDSIZE_Y, IC_X_SEGS, IC_Y_COMS, name)
{
    hw_reset();
    init();
    cls();
    set_orientation(1);
    locate(0,0);
    copy_to_lcd();
}


// reset and init the lcd controller
void SH1106::init()
{
    // Init sequence for 128x64 OLED module
    wr_cmd8(SH1106_DISPLAYOFF);                    // 0xAE
    wr_cmd8(SH1106_SETDISPLAYCLOCKDIV);            // 0xD5
    wr_cmd8(0x80);                                  // the suggested ratio 0x80
    wr_cmd8(SH1106_SETMULTIPLEX);                  // 0xA8
    wr_cmd8(0x3F);
    wr_cmd8(SH1106_SETDISPLAYOFFSET);              // 0xD3
    wr_cmd8(0x00);                                   // no offset
	
    wr_cmd8(SH1106_SETSTARTLINE | 0x0);            // line #0 0x40
    wr_cmd8(SH1106_CHARGEPUMP);                    // 0x8D
    wr_cmd8(0x10);

    wr_cmd8(SH1106_MEMORYMODE);                    // 0x20
    wr_cmd8(0x00);                                  // 0x0 act like ks0108
    wr_cmd8(SH1106_SEGREMAP | 0x1);
    wr_cmd8(SH1106_COMSCANDEC);
    wr_cmd8(SH1106_SETCOMPINS);                    // 0xDA
    wr_cmd8(0x12);
    wr_cmd8(SH1106_SETCONTRAST);                   // 0x81
    wr_cmd8(0x9F);
    wr_cmd8(SH1106_SETPRECHARGE);                  // 0xd9
    wr_cmd8(0x22); 
    wr_cmd8(SH1106_SETVCOMDETECT);                 // 0xDB
    wr_cmd8(0x40);
    wr_cmd8(SH1106_DISPLAYALLON_RESUME);           // 0xA4
    wr_cmd8(SH1106_NORMALDISPLAY);                 // 0xA6
    wr_cmd8(SH1106_DISPLAYON);
}

////////////////////////////////////////////////////////////////////
// functions that overrides the standard ones implemented in LCD.cpp
////////////////////////////////////////////////////////////////////

void SH1106::copy_to_lcd(void)
{

    uint16_t i = 0;
    for(uint8_t page=0; page<8 /*_LCDPAGES */; page++) {
        wr_cmd8(0xB0 | page);        // set page
        wr_cmd8(0x00 | 2);             // set column low nibble My Display starts at column 2 (up to column 130 of 132)
        wr_cmd8(0x10 | 0);             // set column hi  nibble
        wr_grambuf(buffer16+i, screensize_X>>1);   // send whole page pixels as shorts(16bit) not bytes!
        i += screensize_X >> 1;
    }
}

void SH1106::cls(void)
{
    unsigned short tmp = _background^0xFFFF;
    //memset(buffer,tmp,/*screensize_X*_LCDPAGES*/ 128*64/8);  // clear display buffer
    memset(buffer,0x00,screensize_X*(screensize_Y>>3));  // clear display buffer
    if (get_auto_up()) {
        copy_to_lcd();
    }
}

void SH1106::mirrorXY(mirror_t mode)
{
    switch (mode) {
        case(NONE):
            wr_cmd16(0xA0C0);
            break;
        case(X):
            wr_cmd16(0xA1C0);
            break;
        case(Y):
            wr_cmd16(0xA0C8);
            break;
        case(XY):
            wr_cmd16(0xA1C8);
            break;
    }
}

void SH1106::set_contrast(int o)
{
    contrast = o;

    wr_cmd16(0x8100|(o&0xFF));
}

void SH1106::pixel(int x, int y, unsigned short color)
{
    // first check parameter
    if((x >= screensize_X) || (y >= screensize_Y) || (x<0) || (y<0)) return;

    //Buffer[(Xpoint + (Ypoint / 8) * sOLED_DIS.OLED_Dis_Column)] |= 1 << (Ypoint % 8);
    //Buffer[(Xpoint + (Ypoint / 8) * sOLED_DIS.OLED_Dis_Column)] |= 0 << (Ypoint % 8);
    if (color) {
        buffer[x+((y>>3)*screensize_X)] |=  1 << (y % 8);
    } else       {
        buffer[x+((y>>3)*screensize_X)] |=  0 << (y % 8);
    }
}


unsigned short SH1106::pixelread(int x, int y)
{
    // first check parameter
    if((x >= screensize_X) || (y >= screensize_Y) || (x<0) || (y<0)) return 0;

    //Buffer[(Xpoint + (Ypoint / 8) * sOLED_DIS.OLED_Dis_Column)] |= 1 << (Ypoint % 8);
    //Buffer[(Xpoint + (Ypoint / 8) * sOLED_DIS.OLED_Dis_Column)] |= 0 << (Ypoint % 8);
    if (buffer[x+((y>>3)*screensize_X)] &  1 << (y % 8) == 0)
        return Black ;  // pixel not set, Black
    else return White; // pixel set, White
}

