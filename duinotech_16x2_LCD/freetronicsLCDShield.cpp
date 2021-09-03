/* mbed freetronicsLCDShield Library, written by Koen J.F. Kempeneers
 * koen.kempeneers@damiaaninstituut.be
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

#include "mbed.h"
#include "mbed_wait_api.h"
#include "freetronicsLCDShield.h"

#define PULSE_E     wait_ns(1);    \
                    _e = 0;             \
                    wait_ns(1);    \
                    _e = 1;


freetronicsLCDShield::freetronicsLCDShield (PinName rs, PinName e, PinName d0, PinName d1, PinName d2, PinName d3, PinName bl, PinName a0) 
            : _rs(rs), _e(e), _d(d0, d1, d2, d3), _bl(bl), _a0(a0) {
    // Class constructor
    // Init the display, wait 15ms to insure the power is up
    _e = true;
    _rs = false;
    ThisThread::sleep_for(15);
    
    for (int i = 0; i < 3; i++){
        writeByte(0x3);
        wait_us(1640);  // this command takes 1.64ms, so wait for it
    }

    writeByte(0x2);     // 4-bit mode
    writeCommand(0x28); // Function set 001 BW N F - -
    writeCommand(0x0C); // Display on/off controll 0000 1 D C B (D(isplay) On/Off C(ursor) On/Off B(link) On/Off 
    writeCommand(0x6);  // Cursor Direction and Display Shift : 0000 01 CD S (CD 0-left, 1-right S(hift) 0-no, 1-yes
    cls();
    
    // Set the PWM period to 20ms
    _bl.period_ms(1);
    _bl.write(0.0);
}

void freetronicsLCDShield::setCursorPosition (int line, int col) {
    // Set the new cursorposition
    writeCommand(0x80 + (line * 0x40) + col);
}

void freetronicsLCDShield::setBackLight (bool blStatus) {
    // Switch the backlight on (true) or off (false)
    _bl = (blStatus) ? 1.0 : 0.0;
}

void freetronicsLCDShield::setBackLight (float blIntensity) {
    // Switch the backlight on (true) or off (false)
    _bl = blIntensity;
}

void freetronicsLCDShield::setCursor (bool cStatus, bool blink) {
    int tmp = 0;
    
    if (blink) tmp = 0x01;
    if (cStatus) tmp |= 0x02; 
    writeCommand(0x0C + tmp);
}

void freetronicsLCDShield::shift(bool direction) {
    if(direction == LEFT) shiftLeft();
    else shiftRight();
}

void freetronicsLCDShield::shiftLeft(void) {
    writeCommand(0x18 + 0x04);
}

void freetronicsLCDShield::shiftRight(void) {
    writeCommand(0x18);
}

void freetronicsLCDShield::cls(void) {
    // Clear the display and place the cursor at 0, 0
    writeCommand(0x01);
    wait_us(1640);
}

void freetronicsLCDShield::home(void) {
    // Undo shift operations and place cursor at 0,0
    writeCommand(0x02);
    wait_us(1640);
}

void freetronicsLCDShield::writeCGRAM (char address, const char *ptr, char nbytes) {
    // Write the address only once, it is autoincremented 
    writeCommand(0x40 | address);

    // Write the data
    for(int i = 0; i < nbytes; i++) {
        writeData(*ptr++);
    }
}

// Low level output functions
void freetronicsLCDShield::writeByte (int byte) {
    // Split the byte in high and low nibble, write high nibble first
    _d = byte >> 4;
    PULSE_E;
    _d = byte >> 0;
    PULSE_E;
    // Most instructions take 40us
    wait_us(40); 
}

void freetronicsLCDShield::writeData (int data) {
    _rs = true;
    writeByte(data);
} 

void freetronicsLCDShield::writeCommand (int command) {
    _rs = false;
    writeByte(command);
} 

float freetronicsLCDShield::readButtonAnalog(void) {
    return(_a0.read());
}

freetronicsLCDShield::ButtonType_t freetronicsLCDShield::readButton(void) {
    float val;
    
    // read analog value
    val = readButtonAnalog();
    
    // determine which button is pressed
    // analog reading for each button are centered at 0, 144, 329, 504, 741, 1000
    
    // right
    if (val < 0.0740f)
    {
        return freetronicsLCDShield::BTN_RIGHT;
    }
    
    // up
    else if (val < 0.2625f)
    {
        return freetronicsLCDShield::BTN_UP;
    }
    
    // down
    else if (val < 0.4855f)
    {
        return freetronicsLCDShield::BTN_DOWN;
    }
    
    // left
    else if (val < 0.6815f)
    {
        return freetronicsLCDShield::BTN_LEFT;
    }
    
    // none
    else if (val < 0.8465f)
    {
        return freetronicsLCDShield::BTN_NONE;
    }
    
    // select
    else
    {
        return freetronicsLCDShield::BTN_SELECT;
    }
}

// Virtual functions for stream
int freetronicsLCDShield::_putc(int value) {   
    writeData(value);
    return value;
}

int freetronicsLCDShield::_getc() {
    return -1;
}