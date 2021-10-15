/*
||
|| @file Hotboards_Keypad.h
|| @version 3.2
|| @ported by Diego (Hotboards)
|| This Keypad fork library allow to work with keyboard that
|| has physical pull-ups on columns (rather than rows)
||
|| Keypad library originaly develop by:
|| @author Mark Stanley, Alexander Brevig
|| @contact mstanley@technologist.com, alexanderbrevig@gmail.com
||
|| @description
|| | This library provides a simple interface for using matrix
|| | keypads. It supports multiple keypresses while maintaining
|| | backwards compatibility with the old single key library.
|| | It also supports user selectable pins and definable keymaps.
|| #
||
|| @license
|| | This library is free software; you can redistribute it and/or
|| | modify it under the terms of the GNU Lesser General Public
|| | License as published by the Free Software Foundation; version
|| | 2.1 of the License.
|| |
|| | This library is distributed in the hope that it will be useful,
|| | but WITHOUT ANY WARRANTY; without even the implied warranty of
|| | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
|| | Lesser General Public License for more details.
|| |
|| | You should have received a copy of the GNU Lesser General Public
|| | License along with this library; if not, write to the Free Software
|| | Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
|| #
||
*/

#ifndef KEYPAD_H
#define KEYPAD_H

#include "Key.h"


#define OPEN 0
#define CLOSED 1

typedef char KeypadEvent;
typedef unsigned int uint;
typedef unsigned long ulong;

// Made changes according to this post http://arduino.cc/forum/index.php?topic=58337.0
// by Nick Gammon. Thanks for the input Nick. It actually saved 78 bytes for me. :)
typedef struct {
    uint8_t rows;
    uint8_t columns;
} KeypadSize;

#define LIST_MAX 10     // Max number of keys on the active list.
#define MAPSIZE 10      // MAPSIZE is the number of rows (times 16 columns)
#define makeKeymap(x) ((char*)x)


/** Hotboards_keypad class.
 *  Used to control general purpose leds
 *
 * Example:
 * @code
 * #include "mbed.h"
 * #include "Hotboards_keypad.h"
 *
 * char keys[ 4 ][ 4 ] =
 * {
 *    {'1','2','3','A'},
 *    {'4','5','6','B'},
 *    {'7','8','9','C'},
 *    {'*','0','#','D'}
 * };
 * DigitalInOut rowPins[ 4 ] = {PC_0, PC_1, PC_2, PC_3}; 
 * DigitalInOut colPins[ 4 ] = {PB_2, PB_1, PB_15, PB_14};

 * Keypad kpd( makeKeymap( keys ), rowPins, colPins, 4, 4 );

 * int main( void ) 
 * {
 *    while(1){
 *       char key = keypad.getKey( );
 *       if( key ){
 *           // do something with key
 *       }
 *    }
 * }
 * @endcode
 */
class Keypad : public Key {
  public:

    /** Allows custom keymap, pin configuration, and keypad sizes
      * @param userKeymap pointer to bidimentional array with key definitions
      * @param row pointer to array with pins conected to rows
      * @param col pointer to array with pins conected to columns
      * @param numRows number of rows in use
      * @param numCols number of columns in use
      * 
      * Example:
      * @code
      *   char keys[ 4 ][ 4 ] =
      *   {
      *     {'1','2','3','A'},
      *     {'4','5','6','B'},
      *     {'7','8','9','C'},
      *     {'*','0','#','D'}
      *   };
      *   DigitalInOut rowPins[ 4 ] = {PC_0, PC_1, PC_2, PC_3}; 
      *   DigitalInOut colPins[ 4 ] = {PB_2, PB_1, PB_15, PB_14};
      *   Keypad kpd( makeKeymap( keys ), rowPins, colPins, 4, 4 );
      * @endcode
      */
    Keypad(char *userKeymap, DigitalInOut *row, DigitalInOut *col, uint8_t numRows, uint8_t numCols);

    /** Returns a single key only. Retained for backwards compatibility.
      * @return key pressed (user defined key)
      * 
      * Example:
      * @code
      *   char key = keypad.getKey( );
      *   if( key ){
      *     // do something with key
      *   }
      * @endcode
      */
    char getKey(void);
    
    /** Populate the key list (check public key array).
      * @return true if user pressed any key(s)
      * 
      * Example:
      * @code
      * if( kpd.getKeys( ) ){
      *   // you need to poll the array kpd.key[]
      *   for( int i=0 ; i<LIST_MAX ; i++ ){
      *     if( kpd.key[ i ].stateChanged ){ 
      *       chat = kpd.key[ i ].kchar;
      *     }
      * }
      * @endcode
      */
    bool getKeys(void);
    
    /** Get the state of the first key on the list of active keys
      * @return key[0].kstate
      * 
      * Example:
      * @code
      *   if(kpd.getState() == PRESSED ){
      *     // do something
      *   }
      * @endcode
      */
    KeyState getState(void);
    
    /** Let the user define a keymap - assume the same row/column 
      * count as defined in constructor
      * @param userKeymap pointer to user keymap
      * 
      * Example:
      * @code
      *   // lets assume user wnats to change the keymap
      *   kpd.begin( makeKeymap( NewKeys )),
      * @endcode
      */
    void begin(char *userKeymap);
    
    /** Return a true if the selected key is pressed. Is neccesary 
      * to call getKeys function first
      * @return key pressed (user defined key)
      * 
      * Example:
      * @code
      *  if( kpd.getKeys( ) ){
      *    if( kpd.isPressed( '2' ) ){
      *      // key '2' have been press
      *    }
      *  } 
      * @endcode
      */
    bool isPressed(char keyChar);
    
    /** Set a new debounce time (1ms is the minimum value)
      * @param debounce time in milliseconds
      * 
      * Example:
      * @code
      *  // change default 10ms debounce time to 20ms
      *  kpd.setDebounceTime( 20 );
      * @endcode
      */
    void setDebounceTime(uint);
    
    /** Set a new time to considered a key is in hold state
      * @param hold hold time in milliseconds
      * 
      * Example:
      * @code
      *  // change default 500ms hold time to 250ms
      *  kpd.setHoldTime( 250 );
      * @endcode
      */
    void setHoldTime(uint);
    
    /** Set a callback function to be called everytime an key change its status 
      * @param listener function to be called
      * 
      * Example:
      * @code
      *   kpd.addEventListener( keypadEvent );
      * @endcode
      */
    void addEventListener(void (*listener)(char));
    
    /** Search by character for a key in the list of active keys.
      * @return Returns -1 if not found or the index into the list of active keys.
      * 
      * Example:
      * @code
      *   // kpd.getKeys function needs to be called first
      *   int index = findInList( '7' );
      *   if( kpd.key[ index ].stateChanged ){
      *     // the key change, so do something =)
      *   }
      * @endcode
      */
    int findInList(char keyChar);
    
    /** Search by code for a key in the list of active keys.
      * @return Returns -1 if not found or the index into the list of active keys.
      * 
      * Example:
      * @code
      *   // kpd.getKeys function needs to be called first
      *   int index = findInList( 10 );
      *   if( kpd.key[ index ].stateChanged ){
      *     // the key change, so do something =)
      *   }
      * @endcode
      */
    int findInList(int keyCode);
    
    /** lock everything while waiting for a keypress.
      * @return key pressed 
      * 
      * Example:
      * @code
      *   char key = kpd.waitForKey();
      * @endcode
      */
    char waitForKey(void);
    
    /** Return stateChanged element of the first key from the active list
      * @return key[0].stateChanged
      * 
      * Example:
      * @code
      *   if( kpd.keyStateChanged() ){
      *     // do something
      *   }
      * @endcode
      */
    bool keyStateChanged(void);
    
    /** The number of keys on the key list, key[LIST_MAX]
      * @return number of keys
      * 
      * Example:
      * @code
      *   int keyNum = numKeys(); 
      * @endcode
      */
    uint8_t numKeys(void);
    
    uint bitMap[MAPSIZE];   // 10 row x 16 column array of bits. Except Due which has 32 columns.
    Key key[LIST_MAX];
    unsigned long holdTimer;

private:
    unsigned long startTime;
    char *keymap;
    DigitalInOut *rowPins;
    DigitalInOut *columnPins;
    KeypadSize sizeKpd;
    uint debounceTime;
    uint holdTime;
    bool single_key;
    Timer debounce;

    void scanKeys();
    bool updateList();
    void nextKeyState(uint8_t n, bool button);
    void transitionTo(uint8_t n, KeyState nextState);
    void (*keypadEventListener)(char);
};

#endif

/*
|| @changelog
|| | 3.1 2013-01-15 - Mark Stanley     : Fixed missing RELEASED & IDLE status when using a single key.
|| | 3.0 2012-07-12 - Mark Stanley     : Made library multi-keypress by default. (Backwards compatible)
|| | 3.0 2012-07-12 - Mark Stanley     : Modified pin functions to support Keypad_I2C
|| | 3.0 2012-07-12 - Stanley & Young  : Removed static variables. Fix for multiple keypad objects.
|| | 3.0 2012-07-12 - Mark Stanley     : Fixed bug that caused shorted pins when pressing multiple keys.
|| | 2.0 2011-12-29 - Mark Stanley     : Added waitForKey().
|| | 2.0 2011-12-23 - Mark Stanley     : Added the public function keyStateChanged().
|| | 2.0 2011-12-23 - Mark Stanley     : Added the private function scanKeys().
|| | 2.0 2011-12-23 - Mark Stanley     : Moved the Finite State Machine into the function getKeyState().
|| | 2.0 2011-12-23 - Mark Stanley     : Removed the member variable lastUdate. Not needed after rewrite.
|| | 1.8 2011-11-21 - Mark Stanley     : Added test to determine which header file to compile,
|| |                                          WProgram.h or Arduino.h.
|| | 1.8 2009-07-08 - Alexander Brevig : No longer uses arrays
|| | 1.7 2009-06-18 - Alexander Brevig : This library is a Finite State Machine every time a state changes
|| |                                          the keypadEventListener will trigger, if set
|| | 1.7 2009-06-18 - Alexander Brevig : Added setDebounceTime setHoldTime specifies the amount of
|| |                                          microseconds before a HOLD state triggers
|| | 1.7 2009-06-18 - Alexander Brevig : Added transitionTo
|| | 1.6 2009-06-15 - Alexander Brevig : Added getState() and state variable
|| | 1.5 2009-05-19 - Alexander Brevig : Added setHoldTime()
|| | 1.4 2009-05-15 - Alexander Brevig : Added addEventListener
|| | 1.3 2009-05-12 - Alexander Brevig : Added lastUdate, in order to do simple debouncing
|| | 1.2 2009-05-09 - Alexander Brevig : Changed getKey()
|| | 1.1 2009-04-28 - Alexander Brevig : Modified API, and made variables private
|| | 1.0 2007-XX-XX - Mark Stanley : Initial Release
|| #
*/