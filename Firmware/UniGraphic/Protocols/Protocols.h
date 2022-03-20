 /* mbed UniGraphic library - Abstract protocol class
 * Copyright (c) 2015 Giuliano Dianda
 * Released under the MIT License: http://mbed.org/license/mit
 */
 
/** @file Protocols.h
*/
#ifndef Protocols_H
#define Protocols_H

#include "mbed.h"

#define RGB24to16(r,g,b)  (((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3)) //5 red | 6 green | 5 blue
#define BGR2RGB(color) (((color&0x1F)<<11) | (color&0x7E0) | ((color&0xF800)>>11))

#define FLIP_NONE   0
#define FLIP_X      1
#define FLIP_Y      2

//#define USE_CS

/** Protocol types
*/
#include "platform.h"

#if DEVICE_PORTINOUT
enum proto_t {
    PAR_8   /**< Parallel 8bit, port pins 0 to 7 */
    ,PAR_16 /**< Parallel 16bit, port pins 0 to 15 */
    ,BUS_8   /**< Parallel 8bit, scattered pins */
    ,BUS_16   /**< Parallel 16bit, scattered pins */
    ,SPI_8  /**< SPI 8bit */
    ,SPI_16 /**< SPI 16bit */
    ,I2C_   /**< I2C */
};
#else 
enum proto_t {
    BUS_8   /**< Parallel 8bit, scattered pins */
    ,BUS_16   /**< Parallel 16bit, scattered pins */
    ,SPI_8  /**< SPI 8bit */
    ,SPI_16 /**< SPI 16bit */
    ,I2C_   /**< I2C */
};
#endif


/** Abstract interface class for spi and parallel protocols
*/
class Protocols
{
 public:
    
    /** Send 8bit command to display controller 
    *
    * @param cmd: byte to send  
    *
    */   
    virtual void wr_cmd8(unsigned char cmd) = 0;
    
    /** Send 8bit data to display controller 
    *
    * @param data: byte to send   
    *
    */   
    virtual void wr_data8(unsigned char data) = 0;
    
    /** Send 2x8bit command to display controller 
    *
    * @param cmd: halfword to send  
    *
    */   
    virtual void wr_cmd16(unsigned short cmd) = 0;
    
    /** Send 2x8bit data to display controller 
    *
    * @param data: halfword to send   
    *
    */   
    virtual void wr_data16(unsigned short data) = 0;
    
    /** Send 16bit pixeldata to display controller 
    *
    * @param data: halfword to send   
    *
    */   
    virtual void wr_gram(unsigned short data) = 0;
    
    /** Send same 16bit pixeldata to display controller multiple times
    *
    * @param data: halfword to send
    * @param count: how many
    *
    */   
    virtual void wr_gram(unsigned short data, unsigned int count) = 0;
    
    /** Send array of pixeldata shorts to display controller
    *
    * @param data: unsigned short pixeldata array
    * @param lenght: lenght (in shorts)
    *
    */   
    virtual void wr_grambuf(unsigned short* data, unsigned int lenght) = 0;
    
    /** Read 16bit pixeldata from display controller (with dummy cycle)
    *
    * @param convert true/false. Convert 18bit to 16bit, some controllers returns 18bit
    * @returns 16bit color
    */ 
    virtual unsigned short rd_gram(bool convert) = 0;
    
    /** Read 4x8bit register data (with dummy cycle)
    * @param reg the register to read
    * @returns data as uint
    * 
    */ 
    virtual unsigned int rd_reg_data32(unsigned char reg) = 0;
    
    /** Read 3x8bit ExtendedCommands register data
    * @param reg the register to read
    * @param SPIreadenablecmd vendor/device specific cmd to read EXTC registers
    * @returns data as uint
    * @note EXTC regs (0xB0 to 0xFF) are read/write registers but needs special cmd to be read in SPI mode
    */ 
    virtual unsigned int rd_extcreg_data32(unsigned char reg, unsigned char SPIreadenablecmd) = 0;
    
    /** ILI932x specific, does a dummy read cycle, number of bits is protocol dependent
    * for PAR protocols: a signle RD bit toggle
    * for SPI8: 8clocks
    * for SPI16: 16 clocks
    */   
    virtual void dummyread () = 0;
    
    /** ILI932x specific, select register for a successive write or read
    *
    * @param reg register to be selected
    * @param forread false = a write next (default), true = a read next
    * @note forread only used by SPI protocols
    */   
    virtual void reg_select(unsigned char reg, bool forread =false) = 0;
    
    /** ILI932x specific, write register with data
    *
    * @param reg register to write
    * @param data 16bit data
    */   
    virtual void reg_write(unsigned char reg, unsigned short data) = 0;
    
    /** ILI932x specific, read register
    *
    * @param reg register to be read
    * @returns 16bit register value
    */ 
    virtual unsigned short reg_read(unsigned char reg) = 0;
    
    /** HW reset sequence (without display init commands)   
    */
    virtual void hw_reset() = 0;
    
    /** Set ChipSelect high or low
    * @param enable 0/1   
    */
    virtual void BusEnable(bool enable) = 0;

};
#endif