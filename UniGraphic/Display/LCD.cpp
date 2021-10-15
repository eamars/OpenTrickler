/* mbed UniGraphic library - universal LCD driver class
 * Copyright (c) 2015 Giuliano Dianda
 * Released under the MIT License: http://mbed.org/license/mit
 *
 * Derived work of:
 *
 * mbed library for the mbed Lab Board  128*32 pixel LCD
 * use C12832 controller
 * Copyright (c) 2012 Peter Drescher - DC2PD
 * Released under the MIT License: http://mbed.org/license/mit
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "platform.h"
#include "LCD.h"


//#include "mbed_debug.h"

#define SWAP(a, b)  { a ^= b; b ^= a; a ^= b; }

#if DEVICE_PORTINOUT
LCD::LCD(proto_t displayproto, PortName port, PinName CS, PinName reset, PinName DC, PinName WR, PinName RD, const int lcdsize_x, const int lcdsize_y, const int ic_x_segs, const int ic_y_coms, const char *name)
    : GraphicsDisplay(name), screensize_X(lcdsize_x), screensize_Y(lcdsize_y), _LCDPAGES(lcdsize_y>>3), _IC_X_SEGS(ic_x_segs), _IC_Y_COMS(ic_y_coms), _IC_PAGES(ic_y_coms>>3)
{
    useNOP=false;
    //buffer = (unsigned char*) malloc (screensize_X*_LCDPAGES);
    //posix_memalign(void **memptr, size_t alignment, size_t size);
    posix_memalign((void **)&buffer, 16,screensize_X*_LCDPAGES);
    buffer16 = (unsigned short*)buffer;
    draw_mode = NORMAL;
    set_orientation(1);
    foreground(White);
    background(Black);
    set_auto_up(true);
    tftID=0;
  //  cls();
  //  locate(0,0);
}
#endif  

LCD::LCD(proto_t displayproto, PinName* buspins, PinName CS, PinName reset, PinName DC, PinName WR, PinName RD, const int lcdsize_x, const int lcdsize_y, const int ic_x_segs, const int ic_y_coms, const char *name)
    : GraphicsDisplay(name), screensize_X(lcdsize_x), screensize_Y(lcdsize_y), _LCDPAGES(lcdsize_y>>3), _IC_X_SEGS(ic_x_segs), _IC_Y_COMS(ic_y_coms), _IC_PAGES(ic_y_coms>>3)
{
    useNOP=false;
    //buffer = (unsigned char*) malloc (screensize_X*_LCDPAGES);
    //posix_memalign(void **memptr, size_t alignment, size_t size);
    posix_memalign((void **)&buffer, 16,screensize_X*_LCDPAGES);
    buffer16 = (unsigned short*)buffer;
    draw_mode = NORMAL;
    set_orientation(1);
    foreground(White);
    background(Black);
    set_auto_up(true);
    tftID=0;
  //  cls();
  //  locate(0,0);
}
LCD::LCD(proto_t displayproto, int Hz, PinName mosi, PinName miso, PinName sclk, PinName CS, PinName reset, PinName DC, const int lcdsize_x, const int lcdsize_y, const int ic_x_segs, const int ic_y_coms, const char *name)
    : GraphicsDisplay(name), screensize_X(lcdsize_x), screensize_Y(lcdsize_y), _LCDPAGES(lcdsize_y>>3), _IC_X_SEGS(ic_x_segs), _IC_Y_COMS(ic_y_coms), _IC_PAGES(ic_y_coms>>3)
{
    if(displayproto==SPI_8)
    {
        proto = new SPI8(Hz, mosi, miso, sclk, CS, reset, DC);
        useNOP=false;
    }
    else if(displayproto==SPI_16)
    {
        proto = new SPI16(Hz, mosi, miso, sclk, CS, reset, DC);
        useNOP=true;
    }
    //buffer = (unsigned char*) malloc (screensize_X*_LCDPAGES);
    //posix_memalign(void **memptr, size_t alignment, size_t size);
    posix_memalign((void **)&buffer, 16,screensize_X*_LCDPAGES);
    buffer16 = (unsigned short*)buffer;
    draw_mode = NORMAL;
  //  cls();
    set_orientation(1);
    foreground(White);
    background(Black);
    set_auto_up(true);
    tftID=0;
  //  locate(0,0);
}
LCD::LCD(proto_t displayproto, int Hz, int address, PinName sda, PinName scl, PinName reset, const int lcdsize_x, const int lcdsize_y, const int ic_x_segs, const int ic_y_coms, const char* name)
    : GraphicsDisplay(name), screensize_X(lcdsize_x), screensize_Y(lcdsize_y), _LCDPAGES(lcdsize_y>>3), _IC_X_SEGS(ic_x_segs), _IC_Y_COMS(ic_y_coms), _IC_PAGES(ic_y_coms>>3) 
{

    //buffer = (unsigned char*) malloc (screensize_X*_LCDPAGES);
    //posix_memalign(void **memptr, size_t alignment, size_t size);
    posix_memalign((void **)&buffer, 16,screensize_X*_LCDPAGES);
    buffer16 = (unsigned short*)buffer;
    
    draw_mode = NORMAL;
  //  cls();
    set_orientation(1);
    foreground(White);
    background(Black);
    set_auto_up(true);
    tftID=0;
}
        
LCD::~LCD()
{
    free(buffer);
}

void LCD::wr_cmd8(unsigned char cmd)
    {
        if(useNOP) proto->wr_cmd16(0xE300|cmd); // E3 is NOP cmd for LCD
        else proto->wr_cmd8(cmd);
    }
void LCD::wr_data8(unsigned char data)
    {
        proto->wr_data8(data);
    }
void LCD::wr_cmd16(unsigned short cmd)
    {
        proto->wr_cmd16(cmd);
    }
void LCD::wr_gram(unsigned short data, unsigned int count)
    {
        proto->wr_gram(data, count);
    }
void LCD::wr_grambuf(unsigned short* data, unsigned int lenght)
    {
        proto->wr_grambuf(data, lenght);
    }
void LCD::hw_reset()
    {
        proto->hw_reset();
    }
void LCD::BusEnable(bool enable)
    {
        proto->BusEnable(enable);
    }



// monochrome LCD driver ICs does not have ram rotate in hw (swap raw<->columns) like TFT displays
// for portrait views, XY swap will be done in sw in pixel() function
void LCD::set_orientation(int o)
{
    orientation = o;
    switch (o) {
        case (0):// portrait view -90°
            mirrorXY(Y);
            col_offset = 0;
            page_offset = _IC_PAGES-_LCDPAGES;
            set_width(screensize_Y);
            set_height(screensize_X);
        //    portrait = true;
            break;
        case (1): // default, landscape view 0°
            mirrorXY(NONE);
            col_offset = 0;
            page_offset = 0;
            set_width(screensize_X);
            set_height(screensize_Y);
       //     portrait = false;
            break;
        case (2):// portrait view +90°
            mirrorXY(X);
            col_offset = _IC_X_SEGS-screensize_X; // some displays have less pixels than IC ram
            page_offset = 0;
            set_width(screensize_Y);
            set_height(screensize_X);
       //     portrait = true;
            break;
        case (3):// landscape view +180°
            mirrorXY(XY);
            col_offset = _IC_X_SEGS-screensize_X;
            page_offset = _IC_PAGES-_LCDPAGES;
            set_width(screensize_X);
            set_height(screensize_Y);
       //     portrait = false;
            break;
    }
}
void LCD::mirrorXY(mirror_t mode)
{
    switch (mode)
    {
        case(NONE):
         //   wr_cmd8(0xA0);
            wr_cmd16(0xA0C8); // this is in real Y mirror command, but seems most displays have COMs wired inverted, so assume this is the default no-y-mirror
            break;
        case(X):
        //    wr_cmd8(0xA1);
            wr_cmd16(0xA1C8);
            break;
        case(Y):
        //    wr_cmd8(0xA0);
            wr_cmd16(0xA0C0);
            break;
        case(XY):
        //    wr_cmd8(0xA1);
            wr_cmd16(0xA1C0);
            break;
    }
}
void LCD::invert(unsigned char o)
{
    if(o == 0) wr_cmd8(0xA6);
    else wr_cmd8(0xA7);
}

void LCD::set_contrast(int o)
{
    contrast = o;
 //   wr_cmd8(0x81);      //  set volume
    wr_cmd16(0x8100|(o&0x3F));
}

int LCD::get_contrast(void)
{
    return(contrast);
}
void LCD::window(int x, int y, int w, int h) {
    // current pixel location
    cur_x = x;
    cur_y = y;
    // window settings
    win_x1 = x;
    win_x2 = x + w - 1;
    win_y1 = y;
    win_y2 = y + h - 1;
}
void LCD::window_pushpixel(unsigned short color) {
    pixel(cur_x, cur_y, color);
    cur_x++;
    if(cur_x > win_x2) {
        cur_x = win_x1;
        cur_y++;
        if(cur_y > win_y2) {
            cur_y = win_y1;
        }
    }
}
void LCD::window_pushpixel(unsigned short color, unsigned int count) {
    while(count)
    {
        pixel(cur_x, cur_y, color);
        cur_x++;
        if(cur_x > win_x2)
        {
            cur_x = win_x1;
            cur_y++;
            if(cur_y > win_y2)
            {
                cur_y = win_y1;
            }
        }
        count--;
    }
}
void LCD::window_pushpixelbuf(unsigned short* color, unsigned int lenght) {
    while(lenght)
    {
        pixel(cur_x, cur_y, *color++);
        cur_x++;
        if(cur_x > win_x2)
        {
            cur_x = win_x1;
            cur_y++;
            if(cur_y > win_y2)
            {
                cur_y = win_y1;
            }
        }
        lenght--;
    }
}

void LCD::pixel(int x, int y, unsigned short color)
{
    /*
    if(!(orientation&1)) SWAP(x,y);
    // first check parameter
    if((x >= screensize_X) || (y >= screensize_Y)) return;

   
    if (color) {buffer[(x>>3)+(y*_IC_X_SEGS>>4)]&=  ~(1 << (7-(x&7)));}
    else       {buffer[(x>>3)+(y*_IC_X_SEGS>>4)]|=   (1 << (7-(x&7)));}
    
    //buffer[0]=0xFF;
    //buffer[16]=0xAA;
    //buffer[1023]=0xFF;
*/
}

unsigned short LCD::pixelread(int x, int y)
{
    /*
    if(!(orientation&1)) SWAP(x,y);
    // first check parameter
    if((x >= screensize_X) || (y >= screensize_Y)) return 0;
    
    
    if((buffer[(x>>3)+(y*_IC_X_SEGS>>4)] & (1 << (7-(x&7))))==0) return 0xFFFF ;  // pixel not set, White
    else return 0; // pixel set, Black
    */
    return 0;
}
void LCD::copy_to_lcd(void)
{
    /*
    unsigned short i=0;
    unsigned short setcolcmd = 0x0010 | ((col_offset&0xF)<<8) | (col_offset>>4);
    for(int page=0; page<_LCDPAGES; page++)
    {
      //  wr_cmd8(col_offset&0xF);              // set column low nibble
      //  wr_cmd8(0x10|(col_offset>>4));      // set column hi  nibble
        wr_cmd16(setcolcmd);
        wr_cmd8(0xB0|(page+page_offset));      // set page
        wr_grambuf(buffer16+i, screensize_X>>1);   // send whole page pixels
        i+=screensize_X>>1;
    }
    */
}
void LCD::cls(void)
{
    /*
    unsigned short tmp = _background^0xFFFF;
    memset(buffer,tmp,screensize_X*_LCDPAGES);  // clear display buffer
    unsigned short setcolcmd = 0x0010 | ((col_offset&0xF)<<8) | (col_offset>>4);
    for(int page=0; page<_LCDPAGES; page++)
    {
     //   wr_cmd8((unsigned char)col_offset&0xF);              // set column low nibble
     //   wr_cmd8(0x10|(col_offset>>4));      // set column hi  nibble
        wr_cmd16(setcolcmd);
        wr_cmd8(0xB0|(page+page_offset));      // set page
        wr_gram(tmp, screensize_X>>1);   // send whole page pixels = background
    }
    */
}
int LCD::sizeX()
{
    return screensize_X;
}
int LCD::sizeY()
{
    return screensize_Y;
}