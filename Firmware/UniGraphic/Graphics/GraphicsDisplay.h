/* mbed GraphicsDisplay Display Library Base Class
 * Copyright (c) 2007-2009 sford
 * Released under the MIT License: http://mbed.org/license/mit
 *
 * A library for providing a common base class for Graphics displays
 * To port a new display, derive from this class and implement
 * the constructor (setup the display), pixel (put a pixel
 * at a location), width and height functions. Everything else
 * (locate, printf, putc, cls, window, putp, fill, blit, blitbit) 
 * will come for free. You can also provide a specialised implementation
 * of window and putp to speed up the results
 */

#ifndef MBED_GRAPHICSDISPLAY_H
#define MBED_GRAPHICSDISPLAY_H

#include "TextDisplay.h"
#include "Terminal6x8.h"



/* some RGB color definitions                                                 */
#define Black           0x0000      /*   0,   0,   0 */
#define Navy            0x000F      /*   0,   0, 128 */
#define DarkGreen       0x03E0      /*   0, 128,   0 */
#define DarkCyan        0x03EF      /*   0, 128, 128 */
#define Maroon          0x7800      /* 128,   0,   0 */
#define Purple          0x780F      /* 128,   0, 128 */
#define Olive           0x7BE0      /* 128, 128,   0 */
#define LightGrey       0xC618      /* 192, 192, 192 */
#define DarkGrey        0x7BEF      /* 128, 128, 128 */
#define Blue            0x001F      /*   0,   0, 255 */
#define Green           0x07E0      /*   0, 255,   0 */
#define Cyan            0x07FF      /*   0, 255, 255 */
#define Red             0xF800      /* 255,   0,   0 */
#define Magenta         0xF81F      /* 255,   0, 255 */
#define Yellow          0xFFE0      /* 255, 255,   0 */
#define White           0xFFFF      /* 255, 255, 255 */
#define Orange          0xFD20      /* 255, 165,   0 */
#define GreenYellow     0xAFE5      /* 173, 255,  47 */

/** Bitmap
 */
struct Bitmap_s{
    int xSize;
    int ySize;
    int Byte_in_Line;
    char* data;
    };

/** A common base class for Graphics displays
*/
class GraphicsDisplay : public TextDisplay {

public:         
          
    /** Create a GraphicsDisplay interface
    * @param name The name used by the parent class to access the interface
    */
    GraphicsDisplay(const char* name);
     
////// functions needing implementation in derived implementation class ///////////////////////////////////////
////// ---------------------------------------------------------------- ///////////////////////////////////////

    /** Draw a pixel in the specified color.
    * @note this method must be supported in the derived class.
    * @param x is the horizontal offset to this pixel.
    * @param y is the vertical offset to this pixel.
    * @param color defines the color for the pixel.
    */
    virtual void pixel(int x, int y, unsigned short color) = 0;
    
    /** Set the window, which controls where items are written to the screen.
    * When something hits the window width, it wraps back to the left side
    * and down a row. If the initial write is outside the window, it will
    * be captured into the window when it crosses a boundary.
    * @param x is the left edge in pixels.
    * @param y is the top edge in pixels.
    * @param w is the window width in pixels.
    * @param h is the window height in pixels.
    * @note this method must be overridden in a derived class.
    */
    virtual void window(int x, int y, int w, int h) = 0;

    /** Push a single pixel into the window and increment position.
    * You may first call window() then push pixels in loop.
    * @param color is the pixel color.
    * @note this method must be overridden in a derived class.
    */
    virtual void window_pushpixel(unsigned short color) = 0;
    
    /** Push some pixels of the same color into the window and increment position.
    * You must first call window() then push pixels.
    * @param color is the pixel color.
    * @param count: how many
    */
    virtual void window_pushpixel(unsigned short color, unsigned int count) = 0;
    
    /** Push array of pixel colors into the window and increment position.
    * You must first call window() then push pixels.
    * @param color is the pixel color.
    */
    virtual void window_pushpixelbuf(unsigned short* color, unsigned int lenght) = 0;
  
    /** If framebuffer is used, it needs to be sent to LCD from time to time
    @note this method must be overridden in a derived class.
    @note real function for LCD, dummy for TFT
    */
    virtual void copy_to_lcd() = 0; 

/////// functions that come for free, but can be overwritten///////////////////////////////////////////////////
/////// ----------------------------------------------------///////////////////////////////////////////////////

    /** Set window to max possible size
    * May be overridden in a derived class.
    */
    virtual void WindowMax(void);

    /** clear the entire screen
    * Basically it sets windomax then fill with background color
    * May be overridden in a derived class.
    */
    virtual void cls();

    /** draw a circle
   *
   * @param x0,y0 center
   * @param r radius
   * @param color 16 bit color                                                                 *
   *
   */    
  virtual void circle(int x, int y, int r, unsigned short color); 
  
  /** draw a filled circle
   *
   * @param x0,y0 center
   * @param r radius
   * @param color 16 bit color                                                                 *
   */    
  virtual void fillcircle(int x, int y, int r, unsigned short color); 
 
    
  /** draw a 1 pixel line
   *
   * @param x0,y0 start point
   * @param x1,y1 stop point
   * @param color 16 bit color
   *
   */    
  virtual void line(int x0, int y0, int x1, int y1, unsigned short color);
    
    /** draw a horizontal line
   *
   * @param x0 horizontal start
   * @param x1 horizontal stop
   * @param y vertical position
   * @param color 16 bit color                                               
   *
   */
  void hline(int x0, int x1, int y, unsigned short color);
    
  /** draw a vertical line
   *
   * @param x horizontal position
   * @param y0 vertical start 
   * @param y1 vertical stop
   * @param color 16 bit color
   */
  void vline(int y0, int y1, int x, unsigned short color);
  
  /** draw a rect
   *
   * @param x0,y0 top left corner
   * @param x1,y1 down right corner
   * @param color 16 bit color
   *                                                   *
   */    
  virtual void rect(int x0, int y0, int x1, int y1, unsigned short color);
    
  /** draw a filled rect
   *
   * @param x0,y0 top left corner
   * @param x1,y1 down right corner
   * @param color 16 bit color
   *
   */    
  virtual void fillrect(int x0, int y0, int x1, int y1, unsigned short color);
  
    /** setup cursor position for text
   *
   * @param x x-position (top left)
   * @param y y-position 
   */   
  virtual void locate(int x, int y);
    
    /** put a char on the screen
   *
   * @param value char to print
   * @returns printed char
   *
   */
  virtual int _putc(int value);
    
  /** draw a character on given position out of the active font to the TFT
   *
   * @param x x-position of char (top left) 
   * @param y y-position
   * @param c char to print
   *
   */    
  virtual void character(int x, int y, int c);
    
  /** paint a bitmap on the TFT 
   *
   * @param x,y : upper left corner 
   * @param w width of bitmap
   * @param h high of bitmap
   * @param *bitmap pointer to the bitmap data
   *
   *   bitmap format: 16 bit R5 G6 B5
   * 
   *   use Gimp to create / load , save as BMP, option 16 bit R5 G6 B5            
   *   use winhex to load this file and mark data stating at offset 0x46 to end
   *   use edit -> copy block -> C Source to export C array
   *   paste this array into your program
   * 
   *   define the array as static const unsigned char to put it into flash memory
   *   cast the pointer to (unsigned char *) :
   *   tft.Bitmap(10,40,309,50,(unsigned char *)scala);
   */    
  void Bitmap(int x, int y, int w, int h,unsigned char *bitmap);
    
    /** paint monochrome bitmap to screen
      *
      * @param bm Bitmap in flash
      * @param x  x start
      * @param y  y start 
      *
      */
    void Bitmap_BW(Bitmap_s bm, int x, int y);
    
   /** paint a 16 bit BMP from filesytem on the TFT (slow) 
   *
   * @param x,y : position of upper left corner 
   * @param *Name_BMP name of the BMP file with drive: "/local/test.bmp"
   *
   * @returns 1 if bmp file was found and painted
   * @returns  0 if bmp file was found not found
   * @returns -1 if file is no bmp
   * @returns -2 if bmp file is no 16 bit bmp
   * @returns -3 if bmp file is to big for screen 
   * @returns -4 if buffer malloc go wrong
   *
   *   bitmap format: 16 bit R5 G6 B5
   * 
   *   use Gimp to create / load , save as BMP, option 16 bit R5 G6 B5
   *   copy to internal file system or SD card           
   */      
  int BMP_16(int x, int y, const char *Name_BMP);  
    
    
    
  /** select the font to use
   *
   * @param f pointer to font array 
   * @param firstascii first ascii code present in font array, default 32 (space)
   * @param lastascii last ascii code present in font array, default 127 (DEL)
   * @param proportional enable/disable variable font width (default enabled)
   *                                                                              
   *   font array can created with GLCD Font Creator from http://www.mikroe.com
   *   you have to add 4 parameter at the beginning of the font array to use: 
   *   - the number of byte / char (not used in this revision, set to whatever)
   *   - the vertial size in pixel
   *   - the horizontal size in pixel
   *   - the number of byte per vertical line (not used in this revision, set to whatever)
   *   you also have to change the array to cont unsigned char[] and __align(2)
   *
   */  
  void set_font(unsigned char* f, unsigned char firstascii=32, unsigned char lastascii=127, bool proportional = true);
  
  /** Zoom fount
   *
   * @param x_mul horizontal size multiplier
   * @param y_mul vertical size multiplier
   */  
  void set_font_zoom(unsigned char x_mul, unsigned char y_mul);

    /** Get the number of columns based on the currently active font.
    * @returns number of columns.
    * @note this method may be overridden in a derived class.
    */
    virtual int columns();

    /** Get the number of rows based on the currently active font.
    * @returns number of rows.
    * @note this method may be overridden in a derived class.
    */
    virtual int rows();
    
    /** get the current oriented screen width in pixels
    * @returns screen width in pixels.
    */
    int width();

    /** get the current oriented screen height in pixels
    * @returns screen height in pixels.
    */
    int height();
    
    /** set the current oriented screen width in pixels
    * @param width screen width in pixels.
    */
    void set_width(int width);

    /** set the current oriented screen height in pixels
    * @param height screen height in pixels.
    */
    void set_height(int height);
    
    /** setup auto update of screen 
      *
      * @param up 1 = on , 0 = off
      * if switched off the program has to call copy_to_lcd() 
      * to update screen from framebuffer
      */
    void set_auto_up(bool up);
 
    /** get status of the auto update function
      *
      *  @returns if auto update is on
      */
    bool get_auto_up(void); 
    
    
    
private:

    unsigned char* font;
    // display width and height related to current orientation
    int oriented_width;
    int oriented_height;
    
    // text char location
    int char_x;
    int char_y;
    
    int  fontoffset;// bytes / char (short)
    int  fonthor;   // hor size of font (char)
    int  fontvert;  // ver size of font (char)
    int fontbpl;   // bytes per line (char)
    int fontzoomver; // size multiplier
    int fontzoomhor; // size multiplier
    unsigned char firstch;  // first ascii code present in font array (usually 32)
    unsigned char lastch;   // last ascii code present in font array (usually 127)
    bool auto_up;  // autoupdate flag for LCD
    bool fontprop;
    

};

#endif
