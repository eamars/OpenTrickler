/* mbed TextDisplay Library Base Class
 * Copyright (c) 2007-2009 sford
 * Released under the MIT License: http://mbed.org/license/mit
 *
 * A common base class for Text displays
 * To port a new display, derive from this class and implement
 * the constructor (setup the display), character (put a character
 * at a location), rows and columns (number of rows/cols) functions.
 * Everything else (locate, printf, putc, cls) will come for free
 *
 * The model is the display will wrap at the right and bottom, so you can
 * keep writing and will always get valid characters. The location is 
 * maintained internally to the class to make this easy
 */

#ifndef MBED_TEXTDISPLAY_H
#define MBED_TEXTDISPLAY_H

#include "mbed.h"

/** A common base class for Text displays
*/
class TextDisplay : public Stream {
public:

    // functions needing implementation in derived implementation class
    // ----------------------------------------------------------------
    /** Create a TextDisplay interface
    * @param name The name used in the path to access the strean through the filesystem
    */
    TextDisplay(const char *name = NULL);

    /** output a character at the given position
    *
    * @param column column where charater must be written
    * @param  row where character must be written
    * @param c the character to be written to the TextDisplay
    * @note this method may be overridden in a derived class.
    */
    virtual void character(int column, int row, int c) = 0;

    /** return number of rows on TextDisplay
    * @result number of rows
    * @note this method must be supported in the derived class.
    */
    virtual int rows() = 0;

    /** return number of columns on TextDisplay
    * @result number of columns
    * @note this method must be supported in the derived class.
    */
    virtual int columns() = 0;
    
    // functions that come for free, but can be overwritten
    // ----------------------------------------------------
    /** redirect output from a stream (stoud, sterr) to  display
    * @param stream stream that shall be redirected to the TextDisplay
    * @note this method may be overridden in a derived class.
    * @returns true if the claim succeeded.
    */
    virtual bool claim (FILE *stream);

    /** clear the entire screen
    * @note this method may be overridden in a derived class.
    */
    virtual void cls();

    /** locate the cursor at a character position.
    * Based on the currently active font, locate the cursor on screen.
    * @note this method may be overridden in a derived class.
    * @param column is the horizontal offset from the left side.
    * @param row is the vertical offset from the top.
    */
    virtual void locate(int column, int row);

    /** set the foreground color
    * @note this method may be overridden in a derived class.
    * @param color is color to use for foreground drawing.
    */
    virtual void foreground(uint16_t colour);

    /** set the background color
    * @note this method may be overridden in a derived class.
    * @param color is color to use for background drawing.
    */
    virtual void background(uint16_t colour);

    // putc (from Stream)
    // printf (from Stream)
    
protected:

    virtual int _putc(int value);
    virtual int _getc();

    // character location
    int _column;
    int _row;

    // colours
    volatile uint16_t _foreground;
    volatile uint16_t _background;
    char *_path;
};

#endif
