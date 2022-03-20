/* mbed UniGraphic library - Graphics class
 * Copyright (c) 2015 Giuliano Dianda
 * Released under the MIT License: http://mbed.org/license/mit
 *
 * Derived work of:
 *
 * mbed GraphicsDisplay Display Library Base Class
 * Copyright (c) 2007-2009 sford
 * Released under the MIT License: http://mbed.org/license/mit
 *
 * mbed library for 240*320 pixel display TFT based on ILI9341 LCD Controller
 * Copyright (c) 2013 Peter Drescher - DC2PD
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
 
 
#include "GraphicsDisplay.h"
#define SWAP(a, b)  { a ^= b; b ^= a; a ^= b; }
GraphicsDisplay::GraphicsDisplay(const char *name):TextDisplay(name) {
    set_font((unsigned char*)Terminal6x8,32,127,true);
 //   foreground(0xFFFF);
 //   background(0x0000);
    char_x = 0;
    char_y = 0;
    oriented_width=0;
    oriented_height=0;
    fontzoomver=1;
    fontzoomhor=1;
    auto_up = true;
}
    
void GraphicsDisplay::WindowMax (void)
{
    window (0, 0, oriented_width,  oriented_height);
}
void GraphicsDisplay::set_width(int width)
{
    oriented_width = width;
}
void GraphicsDisplay::set_height(int height)
{
    oriented_height = height;
}
int GraphicsDisplay::width()
{
    return oriented_width;
}
int GraphicsDisplay::height()
{
    return oriented_height;
}
void GraphicsDisplay::circle(int x0, int y0, int r, unsigned short color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        pixel(x0-x, y0+y,color);
        pixel(x0+x, y0+y,color);
        pixel(x0+x, y0-y,color);
        pixel(x0-x, y0-y,color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
    if(auto_up) copy_to_lcd();
}
void GraphicsDisplay::fillcircle(int x0, int y0, int r, unsigned short color)
{
    bool old_auto_up=auto_up;
    if(auto_up) auto_up=false;
    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        vline(x0-x, y0-y, y0+y, color);
        vline(x0+x, y0-y, y0+y, color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
    if(old_auto_up)
    {
        auto_up=true;
        copy_to_lcd();
    }
}
void GraphicsDisplay::hline(int x0, int x1, int y, unsigned short color)
{
    int len = x1 - x0 + 1;
    window(x0,y,len,1);
 //   for (int j=0; j<len; j++) window_pushpixel(color);
    window_pushpixel(color, len);
    if(auto_up) copy_to_lcd();
    return;
}
void GraphicsDisplay::vline(int x, int y0, int y1, unsigned short color)
{
    int len = y1 - y0 + 1;
    window(x,y0,1,len);
  //  for (int y=0; y<len; y++) window_pushpixel(color);
    window_pushpixel(color, len);
    if(auto_up) copy_to_lcd();
    return;
}
void GraphicsDisplay::line(int x0, int y0, int x1, int y1, unsigned short color)
{
    //WindowMax();
    int   dx = 0, dy = 0;
    int   dx_sym = 0, dy_sym = 0;
    int   dx_x2 = 0, dy_x2 = 0;
    int   di = 0;
 
    dx = x1-x0;
    dy = y1-y0;
 
    if (dx == 0) {        /* vertical line */
        if (y1 < y0) SWAP(y0,y1);
        vline(x0,y0,y1,color);
        return;
    }
 
    if (dx > 0) {
        dx_sym = 1;
    } else {
        dx_sym = -1;
    }
    if (dy == 0) {        /* horizontal line */
        if (x1 < x0) SWAP(x1,x0);
        hline(x0,x1,y0,color);
        return;
    }
 
    if (dy > 0) {
        dy_sym = 1;
    } else {
        dy_sym = -1;
    }
 
    dx = dx_sym*dx;
    dy = dy_sym*dy;
 
    dx_x2 = dx*2;
    dy_x2 = dy*2;
 
    if (dx >= dy) {
        di = dy_x2 - dx;
        while (x0 != x1) {
 
            pixel(x0, y0, color);
            x0 += dx_sym;
            if (di<0) {
                di += dy_x2;
            } else {
                di += dy_x2 - dx_x2;
                y0 += dy_sym;
            }
        }
        pixel(x0, y0, color);
    } else {
        di = dx_x2 - dy;
        while (y0 != y1) {
            pixel(x0, y0, color);
            y0 += dy_sym;
            if (di < 0) {
                di += dx_x2;
            } else {
                di += dx_x2 - dy_x2;
                x0 += dx_sym;
            }
        }
        pixel(x0, y0, color);
    }
    if(auto_up) copy_to_lcd();
    return;
}
void GraphicsDisplay::rect(int x0, int y0, int x1, int y1, unsigned short color)
{
    bool old_auto_up=auto_up;
    if(auto_up) auto_up=0;
    if (x1 > x0) hline(x0,x1,y0,color);
    else  hline(x1,x0,y0,color);
 
    if (y1 > y0) vline(x0,y0,y1,color);
    else vline(x0,y1,y0,color);
 
    if (x1 > x0) hline(x0,x1,y1,color);
    else  hline(x1,x0,y1,color);
 
    if (y1 > y0) vline(x1,y0,y1,color);
    else vline(x1,y1,y0,color);
    if(old_auto_up)
    {
        auto_up=true;
        copy_to_lcd();
    }
    return;
}
void GraphicsDisplay::fillrect(int x0, int y0, int x1, int y1, unsigned short color)
{
    if(x0 > x1) SWAP(x0,x1);
    if(y0 > y1) SWAP(y0,y1);
     
    int h = y1 - y0 + 1;
    int w = x1 - x0 + 1;
    unsigned int pixels = h * w;
    window(x0,y0,w,h);
 //   for (unsigned int p=0; p<pixels; p++) window_pushpixel(color);
    window_pushpixel(color, pixels);
    if(auto_up) copy_to_lcd();
    return;
}
void GraphicsDisplay::locate(int x, int y)
{
    char_x = x;
    char_y = y;
}
int GraphicsDisplay::columns()
{
    return oriented_width / fonthor;
}
int GraphicsDisplay::rows()
{
    return oriented_height / fontvert;
}
void GraphicsDisplay::set_font(unsigned char* f, unsigned char firstascii, unsigned char lastascii, bool proportional)
{
    font = f;
    // read font parameter from start of array
    //fontoffset = font[0];                    // bytes / char
    fonthor = font[1];                       // get hor size of font
    fontvert = font[2];                      // get vert size of font
    //fontbpl = font[3];                       // bytes per line
    fontbpl = (fontvert+7)>>3; //bytes per line, rounded up to multiple of 8
    fontoffset = (fonthor*fontbpl)+1;
    firstch = firstascii;   // first ascii code present in font array (usually 32)
    lastch = lastascii;     // last ascii code present in font array (usually 127)
    fontprop=proportional;
    set_font_zoom(1,1);
}
void GraphicsDisplay::set_font_zoom(unsigned char x_mul, unsigned char y_mul)
{
    fontzoomhor=((x_mul==0) ? 1:x_mul);
    fontzoomver=((y_mul==0) ? 1:y_mul);
}
int GraphicsDisplay::_putc(int value)
{
    if (value == '\n') {    // new line
        char_x = 0;
        char_y = char_y + fontvert*fontzoomver;
        if (char_y >= oriented_height - fontvert*fontzoomver) {
            char_y = 0;
        }
    } else {
        character(char_x, char_y, value);
        if(auto_up) copy_to_lcd();
    }
    return value;
}
void GraphicsDisplay::character(int x, int y, int c)
{
    char_x=x;
    char_y=y;
    int j,i,b;
    unsigned char* zeichen;
    unsigned char z,w,v;
 
 /*   // read font parameter from start of array
    offset = font[0];                    // bytes / char
    hor = font[1];                       // get hor size of font
    vert = font[2];                      // get vert size of font
    bpl = font[3];                       // bytes per line
*/
    if (char_x + fonthor*fontzoomhor > oriented_width) {
        char_x = 0;
        char_y = char_y + fontvert*fontzoomver;
        if (char_y > oriented_height - fontvert*fontzoomver) {
            char_y = 0;
        }
    }
    window(char_x, char_y,fonthor*fontzoomhor,fontvert*fontzoomver); // char box
    if ((c < firstch) || (c > lastch)) {   // test char range - if not exist fill with blank
         for (i = 0; i < fonthor*fontvert*fontzoomver;i++){
          window_pushpixel(_background, fontzoomhor); //(color, howmany) 
          }
    }
    else{
        zeichen = &font[((c-firstch) * fontoffset) + 4]; // start of char bitmap
        w = zeichen[0];                          // width of actual char
        // construct the char into the buffer
        for (j=0; j<fontvert; j++) {  //  vert line
            for (v=0; v<fontzoomver; v++) { // repeat horiz line for vertical zooming
              for (i=0; i<fonthor; i++) {   //  horz line
                z =  zeichen[(fontbpl * i) + ((j & 0xF8) >> 3)+1];
                b = 1 << (j & 0x07);
                if (( z & b ) == 0x00) {
                //   pixel(char_x+i,char_y+j,0);
                    window_pushpixel(_background, fontzoomhor); //(color, howmany)
                } else {
                //    pixel(char_x+i,char_y+j,1);
                    window_pushpixel(_foreground, fontzoomhor);
                }
              }
            } //for each zoomed vert
         }
    }
    if(fontprop)
    {
        if((w+1)<fonthor) char_x += (w*fontzoomhor)+1; // put at least 1 blank after variable-width characters, except characters that occupy whole fonthor space like "_"
        else char_x += fonthor*fontzoomhor;
    }
    else char_x += fonthor*fontzoomhor; // fixed width
}
void GraphicsDisplay::Bitmap_BW(Bitmap_s bm, int x, int y)
{
    int h,v,b;
 //   int cropX;
    char d;
    if(x<0) x=0;
    if(y<0) y=0;
    int cropX = (x+bm.xSize)-oriented_width;
    if(cropX<0) cropX=0; 
    window(x, y, bm.xSize-cropX, bm.ySize);
    for(v=0; v < bm.ySize; v++) {   // lines
        if((v + y) >= oriented_height) break; // no need to crop Y
        for(h=0; h < bm.xSize; h++) { // pixel
            if((h + x) >= oriented_width) break;
            d = bm.data[bm.Byte_in_Line * v + ((h & 0xF8) >> 3)];
            b = 0x80 >> (h & 0x07);
            if((d & b) == 0) {
                window_pushpixel(_background);
            } else {
                window_pushpixel(_foreground);
            }
        }
    }
    if(auto_up) copy_to_lcd();
}
void GraphicsDisplay::Bitmap(int x, int y, int w, int h,unsigned char *bitmap)
{
    int  j;
    unsigned char  padd;
    unsigned short *bitmap_ptr = (unsigned short *)bitmap;    
 
    padd = w%2; // the lines are padded to multiple of 4 bytes in a bitmap
    if(x<0) x=0;
    else if(x>=oriented_width) return;
    if(y<0) y=0;
    else if(y>=oriented_height) return;
    int cropX = (x+w)-oriented_width;
    if(cropX<0) cropX=0;
    int cropY = (y+h)-oriented_height;
    if(cropY<0) cropY=0;
    window(x, y, w-cropX, h-cropY);
    bitmap_ptr += ((h - 1)* (w + padd)); // begin of last line in array (first line of image)(standard bmp scan direction is left->right bottom->top)
    for (j = 0; j < h-cropY; j++) {         //Lines
        window_pushpixelbuf(bitmap_ptr, w-cropX);
        bitmap_ptr -= w+padd;
    }
    if(auto_up) copy_to_lcd();
}
 
// local filesystem is not implemented in kinetis board , but you can add a SD card
// fixme this whole functions needs testing and speedup
int GraphicsDisplay::BMP_16(int x, int y, const char *Name_BMP)
{
 
#define OffsetPixelWidth    18
#define OffsetPixelHeigh    22
#define OffsetFileSize      34
#define OffsetPixData       10
#define OffsetBPP           28
 
    char filename[50];
    unsigned char BMP_Header[54];
    unsigned short BPP_t;
    unsigned int PixelWidth,PixelHeigh,start_data;
    unsigned int    i,off;
    int padd,j;
    unsigned short *line;
 
    // get the filename
    i=0;
    while (*Name_BMP!='\0') {
        filename[i++]=*Name_BMP++;
    }
    filename[i] = 0;  
    
    FILE *Image = fopen((const char *)&filename[0], "rb");  // open the bmp file
    if (!Image) {
        return(0);      // error file not found !
    }
 
    fread(&BMP_Header[0],1,54,Image);      // get the BMP Header
 
    if (BMP_Header[0] != 0x42 || BMP_Header[1] != 0x4D) {  // check magic byte
        fclose(Image);
        return(-1);     // error no BMP file
    }
 
    BPP_t = BMP_Header[OffsetBPP] + (BMP_Header[OffsetBPP + 1] << 8);
    if (BPP_t != 0x0010) {
        fclose(Image);
        return(-2);     // error no 16 bit BMP
    }
 
    PixelHeigh = BMP_Header[OffsetPixelHeigh] + (BMP_Header[OffsetPixelHeigh + 1] << 8) + (BMP_Header[OffsetPixelHeigh + 2] << 16) + (BMP_Header[OffsetPixelHeigh + 3] << 24);
    PixelWidth = BMP_Header[OffsetPixelWidth] + (BMP_Header[OffsetPixelWidth + 1] << 8) + (BMP_Header[OffsetPixelWidth + 2] << 16) + (BMP_Header[OffsetPixelWidth + 3] << 24);
    if (PixelHeigh > oriented_height + y || PixelWidth > oriented_width + x) {
        fclose(Image);
        return(-3);      // to big
    }
 
    start_data = BMP_Header[OffsetPixData] + (BMP_Header[OffsetPixData + 1] << 8) + (BMP_Header[OffsetPixData + 2] << 16) + (BMP_Header[OffsetPixData + 3] << 24);
 
    line = (unsigned short *) malloc (2 * PixelWidth); // we need a buffer for a line
    if (line == NULL) {
        return(-4);         // error no memory
    }
 
    // the bmp lines are padded to multiple of 4 bytes
    padd = -1;
    do {
        padd ++;
    } while ((PixelWidth * 2 + padd)%4 != 0);
 
    window(x, y,PixelWidth ,PixelHeigh);
//    wr_cmd(0x2C);  // send pixel
    for (j = PixelHeigh - 1; j >= 0; j--) {               //Lines bottom up
        off = j * (PixelWidth  * 2 + padd) + start_data;   // start of line
        fseek(Image, off ,SEEK_SET);
        fread(line,1,PixelWidth * 2,Image);       // read a line - slow 
  /*      for (i = 0; i < PixelWidth; i++)
        {        // copy pixel data to TFT
     //       wr_16(line[i]);                  // one 16 bit pixel   
            window_pushpixel(line[i]);
            
        } */
        window_pushpixelbuf(line, PixelWidth);
     }
    free (line);
    fclose(Image);
    if(auto_up) copy_to_lcd();
    return(1);
}
 
void GraphicsDisplay::cls (void)
{
    unsigned int pixels = ( oriented_width * oriented_height);
    WindowMax();
    for (unsigned int i = 0; i < pixels; i++)
    {
        window_pushpixel(_background);
    }
}
void GraphicsDisplay::set_auto_up(bool up)
{
    if(up) auto_up = true;
    else auto_up = false;
}
bool GraphicsDisplay::get_auto_up(void)
{
    return (auto_up);
}
