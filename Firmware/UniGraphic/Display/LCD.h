
#ifndef MBED_LCD_H
#define MBED_LCD_H

#include "SPI8.h"
#include "SPI16.h"
#include "Protocols.h"

#include "GraphicsDisplay.h"

// undefine the KL43Z and KL46Z LCD macro
#ifdef LCD
#undef LCD
#endif

/** Draw mode
  * NORMAl
  * XOR set pixel by xor the screen
  */
enum {NORMAL,XOR};

/** Mirror mode */
enum mirror_t {X,Y,XY,NONE};


/** A common base class for monochrome Display
*/
class LCD : public GraphicsDisplay
{

public:         
          
    /** Create a monochrome LCD Parallel Port interface
    * @param name The name used by the parent class to access the interface
    */
    LCD(proto_t displayproto,PortName port, PinName CS, PinName reset, PinName DC, PinName WR, PinName RD, const int lcdsize_x, const int lcdsize_y, const int ic_x_segs, const int ic_y_coms, const char* name);
    
    /** Create a monochrome LCD Parallel Bus interface
    * @param name The name used by the parent class to access the interface
    */
    LCD(proto_t displayproto,PinName* buspins, PinName CS, PinName reset, PinName DC, PinName WR, PinName RD, const int lcdsize_x, const int lcdsize_y, const int ic_x_segs, const int ic_y_coms, const char* name);
    
    /** Create a monochrome LCD SPI interface
    * @param name The name used by the parent class to access the interface
    */
    LCD(proto_t displayproto, int Hz, PinName mosi, PinName miso, PinName sclk, PinName CS, PinName reset, PinName DC, const int lcdsize_x, const int lcdsize_y, const int ic_x_segs, const int ic_y_coms, const char* name);
    
    /** Create a monochrome LCD I2C interface
    * @param name The name used by the parent class to access the interface
    */
    LCD(proto_t displayproto, int Hz, int address,PinName sda, PinName scl, PinName reset, const int lcdsize_x, const int lcdsize_y, const int ic_x_segs, const int ic_y_coms, const char* name);
    
    
    /** Destructor
    * will free framebuffer
    */
    virtual ~LCD();
    

    
/////// functions that come for free, but can be overwritten///////////////////////////////////////////////////
/////// ----------------------------------------------------///////////////////////////////////////////////////

    /** Draw a pixel in the specified color.
    * @param x is the horizontal offset to this pixel.
    * @param y is the vertical offset to this pixel.
    * @param color defines the color for the pixel.
    */
    virtual void pixel(int x, int y, unsigned short color);

    /** Set the window, which controls where items are written to the screen.
    * When something hits the window width, it wraps back to the left side
    * and down a row. If the initial write is outside the window, it will
    * be captured into the window when it crosses a boundary.
    * @param x is the left edge in pixels.
    * @param y is the top edge in pixels.
    * @param w is the window width in pixels.
    * @param h is the window height in pixels.
    */
    virtual void window(int x, int y, int w, int h);
    
    /** Read pixel color at location
    * @param x is the horizontal offset to this pixel.
    * @param y is the vertical offset to this pixel.
    * @returns 16bit color, 0000=Black(pixel set), FFFF=White(pixel clear).
    */
    virtual unsigned short pixelread(int x, int y);

    /** Push a single pixel into the window and increment position.
    * You must first call window() then push pixels in loop.
    * @param color is the pixel color.
    */
    virtual void window_pushpixel(unsigned short color);
    
    /** Push some pixels of the same color into the window and increment position.
    * You must first call window() then push pixels.
    * @param color is the pixel color.
    * @param count: how many
    */
    virtual void window_pushpixel(unsigned short color, unsigned int count);
    
    /** Push array of pixel colors into the window and increment position.
    * You must first call window() then push pixels.
    * @param color is the pixel color.
    */
    virtual void window_pushpixelbuf(unsigned short* color, unsigned int lenght);
 
    /** Framebuffer is used, it needs to be sent to LCD from time to time
    */
    virtual void copy_to_lcd();
    
    /** set the contrast of the screen
      *
      * @param o contrast 0-63
      * @note may be overrided in case of not standard command
      */
    virtual void set_contrast(int o);

    /** read the contrast level
      *
      */
    int get_contrast(void);

    /** display inverted colors
      *
      * @param o = 0 normal, 1 invert
      */
    void invert(unsigned char o);

    /** clear the entire screen
    * The inherited one sets windomax then fill with background color
    * We override it to speedup
    */
    virtual void cls();
    
    /** Set the orientation of the screen
    *  x,y: 0,0 is always top left 
    *
    * @param o direction to use the screen (0-3)
    * 0 = -90°
    * 1 = default 0°
    * 2 = +90°
    * 3 = +180°
    *
    */  
    void set_orientation(int o);
    
    /** Set ChipSelect high or low
    * @param enable 0/1   
    */
    virtual void BusEnable(bool enable);
    
    /** get display X size in pixels (native, orientation independent)
    * @returns X size in pixels
    */
    int sizeX();

    /** get display Y size in pixels (native, orientation independent)
    * @returns Y size in pixels
    */
    int sizeY();
    
////////////////////////////////////////////////////////////////////////////////    
    // not implemented yet
//////////////////////////////////////////////////////////////////
  //  virtual unsigned short pixelread(int x, int y){return 0;};
    virtual void window4read(int x, int y, int w, int h){};
    void setscrollarea (int startY, int areasize){};
    void scroll (int lines){};
    void scrollreset(){};
    void FastWindow(bool enable){};
    
    unsigned int tftID;
    
    
    
    
protected:

    /** set mirror mode
      * @note may be overridden by specific display init class in case of not standard cmds or inverted wiring 
      * @param mode NONE, X, Y, XY 
      */
    virtual void mirrorXY(mirror_t mode);

////// functions needed by parent class ///////////////////////////////////////
////// -------------------------------- ///////////////////////////////////////

    /** Send 8bit command to display controller 
    *
    * @param cmd: byte to send  
    * @note if protocol is SPI16, it will insert NOP cmd before, so if cmd is a 2byte cmd, the second cmd will be broken. Use wr_cmd16 for 2bytes cmds
    */   
    void wr_cmd8(unsigned char cmd);
    
    /** Send 8bit data to display controller 
    *
    * @param data: byte to send   
    *
    */   
    void wr_data8(unsigned char data);
    
    /** Send 16bit command to display controller 
    *
    * @param cmd: halfword to send  
    *
    */   
    void wr_cmd16(unsigned short cmd);
    
    /** Send same 16bit pixeldata to display controller multiple times
    *
    * @param data: halfword to send
    * @param count: how many
    *
    */   
    virtual void wr_gram(unsigned short data, unsigned int count);
    
    /** Send array of pixeldata shorts to display controller
    *
    * @param data: unsigned short pixeldata array
    * @param lenght: lenght (in shorts)
    *
    */   
    virtual void wr_grambuf(unsigned short* data, unsigned int lenght);
    
    /** HW reset sequence (without display init commands)   
    */
    void hw_reset();
  
    int draw_mode;
    int contrast;
    
//private:
protected:

    Protocols* proto;
    const int screensize_X;
    const int screensize_Y;
    const int _LCDPAGES;
    const int _IC_X_SEGS;
    const int _IC_Y_COMS;
    const int _IC_PAGES;
    
    int page_offset;
    int col_offset;
    // pixel location
    int cur_x;
    int cur_y;
    // window location
    int win_x1;
    int win_x2;
    int win_y1;
    int win_y2;
    int orientation;
    bool useNOP;
  
    
    uint8_t *buffer;
    uint16_t *buffer16;   
};

#endif