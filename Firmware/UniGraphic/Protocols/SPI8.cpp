 /* mbed UniGraphic library - SPI8 protocol class
 * Copyright (c) 2015 Giuliano Dianda
 * Released under the MIT License: http://mbed.org/license/mit
 *
 * Derived work of:
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
 
#include "SPI8.h"


SPI8::SPI8(int Hz, PinName mosi, PinName miso, PinName sclk, PinName CS, PinName reset, PinName DC)
    : _CS(CS), _spi(mosi, miso, sclk), _reset(reset), _DC(DC)
{
    _reset = 1;
    _DC=1;
    _CS=1;
    _spi.format(8,0);                  // 8 bit spi mode 0
    _spi.frequency(Hz);
    hw_reset();    
}

void SPI8::wr_cmd8(unsigned char cmd)
{     
    _DC.write(0); // 0=cmd
    _spi.write(cmd);      // write 8bit
    _DC.write(1); // 1=data next
}
void SPI8::wr_data8(unsigned char data)
{
    _spi.write(data);    // write 8bit
}
void SPI8::wr_cmd16(unsigned short cmd)
{     
    _DC.write(0); // 0=cmd
    _spi.write(cmd>>8);      // write 8bit
    _spi.write(cmd&0xFF);      // write 8bit
    _DC.write(1); // 1=data next
}
void SPI8::wr_data16(unsigned short data)
{
    _spi.write(data>>8);    // write 8bit
    _spi.write(data&0xFF);    // write 8bit
}
void SPI8::wr_gram(unsigned short data)
{
    _spi.write(data>>8);    // write 8bit
    _spi.write(data&0xFF);    // write 8bit
}
void SPI8::wr_gram(unsigned short data, unsigned int count)
{
    if((data>>8)==(data&0xFF))
    {
        count<<=1;
        while(count)
        {
            _spi.write(data);    // write 8bit
            count--;
        }
    }
    else
    {
        while(count)
        {
            _spi.write(data>>8);    // write 8bit
            _spi.write(data&0xFF);    // write 8bit
            count--;
        }
    }
}
void SPI8::wr_grambuf(unsigned short* data, unsigned int lenght)
{
    while(lenght)
    {
        uint8_t high = (*data)>>8;
        uint8_t low = (*data)&0xFF;
        _spi.write(low);    // write 8bit
        _spi.write(high);    // write 8bit
        
        data++;
        lenght--;
    }
}
unsigned short SPI8::rd_gram(bool convert)
{
    unsigned int r=0;
    _spi.write(0); // whole first byte is dummy
    r |= _spi.write(0);
    r <<= 8;
    r |= _spi.write(0);
    if(convert)
    {
        r <<= 8;
        r |= _spi.write(0);
        // gram is 18bit/pixel, if you set 16bit/pixel (cmd 3A), during writing the 16bits are expanded to 18bit
        // during reading, you read the raw 18bit gram
        r = RGB24to16((r&0xFF0000)>>16, (r&0xFF00)>>8, r&0xFF);// 18bit pixel padded to 24bits, rrrrrr00_gggggg00_bbbbbb00, converted to 16bit
    } 
    _CS = 1; // force CS HIG to interupt the "read state"
    _CS = 0;
    return (unsigned short)r;
}
unsigned int SPI8::rd_reg_data32(unsigned char reg)
{
    wr_cmd8(reg);
    unsigned int r=0;
   
    r |= _spi.write(0); // we get only 7bit valid, first bit was the dummy cycle
    r <<= 8;
    r |= _spi.write(0);
    r <<= 8;
    r |= _spi.write(0);
    r <<= 8;
    r |= _spi.write(0);
    r <<= 1; // 32bits are aligned, now collecting bit_0
    r |= (_spi.write(0) >> 7);
    // we clocked 7 more bit so ILI waiting for 8th, we need to reset spi bus
    _CS = 1; // force CS HIG to interupt the cmd
    _CS = 0;
    return r;
}
unsigned int SPI8::rd_extcreg_data32(unsigned char reg, unsigned char SPIreadenablecmd)
{
    unsigned int r=0;
    for(int regparam=1; regparam<4; regparam++) // when reading EXTC regs, first parameter is always dummy, so start with 1
    {
        wr_cmd8(SPIreadenablecmd);  // spi-in enable cmd, 0xD9 (ili9341) or 0xFB (ili9488) or don't know
        wr_data8(0xF0|regparam);    // in low nibble specify which reg parameter we want
        wr_cmd8(reg);               // now send cmd (select register we want to read)
        r <<= 8;
        r |= _spi.write(0);
        // r = _spi.write(0) >> 8; for 16bit
    }
    _CS = 1; // force CS HIG to interupt the cmd
    _CS = 0;
    return r;
}
// ILI932x specific
void SPI8::dummyread()
{
    _spi.write(0);    // dummy read
}
// ILI932x specific
void SPI8::reg_select(unsigned char reg, bool forread)
{
    _CS = 1;    //fixme: really needed?
    _CS = 0;    //fixme: really needed?
    _spi.write(0x70);
    _spi.write(0);    // write MSB
    _spi.write(reg);    // write LSB
    _CS = 1;    //fixme: really needed?
    _CS = 0;    //fixme: really needed?
    if(forread) _spi.write(0x73);
    else _spi.write(0x72);
}
// ILI932x specific
void SPI8::reg_write(unsigned char reg, unsigned short data)
{
    _CS = 1;    //fixme: really needed?
    _CS = 0;    //fixme: really needed?
    _spi.write(0x70);
    _spi.write(0);    // write MSB
    _spi.write(reg);    // write LSB
    _CS = 1;    //fixme: really needed?
    _CS = 0;    //fixme: really needed?
    _spi.write(0x72);
    _spi.write(data>>8);
    _spi.write(data&0xFF);
}
// ILI932x specific
unsigned short SPI8::reg_read(unsigned char reg)
{
    unsigned short r=0;
    _CS = 1;    //fixme: really needed?
    _CS = 0;    //fixme: really needed?
    _spi.write(0x70);
    _spi.write(0);    // write MSB
    _spi.write(reg);    // write LSB
    _CS = 1;    //fixme: really needed?
    _CS = 0;    //fixme: really needed?
    _spi.write(0x73);
    _spi.write(0);    // dummy read
    r = _spi.write(0);    // read 8bit
    r <<= 8;
    r |= _spi.write(0);    // read 8bit
    return r;
}
void SPI8::hw_reset()
{
    thread_sleep_for(15);
    _DC = 1;
    _CS = 1;
    _reset = 0;                        // display reset
    thread_sleep_for(2);
    _reset = 1;                       // end reset
    thread_sleep_for(100);
}
void SPI8::BusEnable(bool enable)
{
    _CS = enable ? 0:1;
}