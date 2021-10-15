/**
 ******************************************************************************
 * @file    DevSPI.h
 * @author  AST / Software Platforms and Cloud / EST
 * @version V1.2.1
 * @date    19-February-2016
 * @brief   Header file for a special SPI class DevSPI which provides some
 *          helper functions for on-board communication.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Define to prevent from recursive inclusion --------------------------------*/
#ifndef __DEV_SPI_H
#define __DEV_SPI_H

/* Includes ------------------------------------------------------------------*/
#include "mbed.h"

/* Macros --------------------------------------------------------------------*/
#if (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) /* GCC */ || \
    (defined(G_BYTE_ORDER) && (G_BYTE_ORDER == G_BIG_ENDIAN)) /* IAR */ || \
    (defined(__BIG_ENDIAN)) /* ARM */
#define __DEV_SPI_BIG_ENDIAN
#endif

/* Classes -------------------------------------------------------------------*/
/** Helper class DevSPI providing functions for synchronous SPI communication
 *  common for a series of SPI devices.
 */
class DevSPI : public SPI
{
 public:
    /*
     * Create a DevSPI interface.
     * @param mosi pin name of the MOSI pin of the SPI device to be used for communication.
     * @param miso pin name of the MISO pin of the SPI device to be used for communication.
     * @param sclk pin name of the SCLK pin of the SPI device to be used for communication.
     */
    DevSPI(PinName mosi, PinName miso, PinName sclk) : SPI(mosi, miso, sclk)
    {
        /* Set default configuration. */
        setup(8, 3, 1E6);
    }

    /*
     * Setup the spi.
     * Typically:
     *  + 8 bit data;
     *  + high steady state clock;
     *  + second edge capture;
     *  + 1MHz clock rate.
     *
     * @param bits         Number of bits per SPI frame (4 - 16)
     * @param mode         Clock polarity and phase mode (0 - 3)
     * @param frequency_hz SCLK frequency in hz (default = 1MHz)
     *
     * @code
     * mode | POL PHA
     * -----+--------
     *   0  |  0   0
     *   1  |  0   1
     *   2  |  1   0
     *   3  |  1   1
     * @endcode
     */
    void setup(int bits, int mode = 0, int frequency_hz = 1E6)
    {
        /* Set given configuration. */
        format(bits, mode);
        frequency(frequency_hz);
    }

    /**
     * @brief      Writes a buffer to the SPI peripheral device in 8-bit data mode 
     *             using synchronous SPI communication.
     * @param[in]  pBuffer pointer to the buffer of data to send.
     * @param[in]  ssel GPIO of the SSEL pin of the SPI device to be used for communication.
     * @param[in]  NumBytesToWrite number of bytes to write.
     * @retval     0 if ok.
     * @retval     -1 if data format error.
     * @note       When using the SPI in Interrupt-mode, remember to disable interrupts
     *             before calling this function and to enable them again after.
     */
    int spi_write(uint8_t* pBuffer, DigitalOut &ssel, uint16_t NumBytesToWrite)
    {
	/* Check data format */
	if(_bits != 8) return -1;

        /* Select the chip. */
        ssel = 0;
        
        /* Write data. */
	for (int i = 0; i < NumBytesToWrite; i++) {
            write(pBuffer[i]);
	}

        /* Unselect the chip. */
        ssel = 1;

        return 0;
    }

    /**
     * @brief      Reads a buffer from the SPI peripheral device in 8-bit data mode 
     *             using synchronous SPI communication.
     * @param[out] pBuffer pointer to the buffer to read data into.
     * @param[in]  ssel GPIO of the SSEL pin of the SPI device to be used for communication.
     * @param[in]  NumBytesToRead number of bytes to read.
     * @retval     0 if ok.
     * @retval     -1 if data format error.
     * @note       When using the SPI in Interrupt-mode, remember to disable interrupts
     *             before calling this function and to enable them again after.
     */
    int spi_read(uint8_t* pBuffer, DigitalOut &ssel, uint16_t NumBytesToRead)
    {
	/* Check data format */
	if(_bits != 8) return -1;

        /* Select the chip. */
        ssel = 0;
        
        /* Read data. */
	for (int i = 0; i < NumBytesToRead; i++) {
	    pBuffer[i] = write(0);
	}

        /* Unselect the chip. */
        ssel = 1;

        return 0;
    }

    /**
     * @brief      Reads and write a buffer from/to the SPI peripheral device at the same time 
     *             in 8-bit data mode using synchronous SPI communication.
     * @param[out] pBufferToRead pointer to the buffer to read data into.
     * @param[in]  pBufferToWrite pointer to the buffer of data to send.
     * @param[in]  ssel GPIO of the SSEL pin of the SPI device to be used for communication.
     * @param[in]  NumBytes number of bytes to read and write.
     * @retval     0 if ok.
     * @retval     -1 if data format error.
     * @note       When using the SPI in Interrupt-mode, remember to disable interrupts
     *             before calling this function and to enable them again after.
     */
    int spi_read_write(uint8_t* pBufferToRead, uint8_t* pBufferToWrite, DigitalOut &ssel, uint16_t NumBytes)
    {
	/* Check data format */
	if(_bits != 8) return -1;

        /* Select the chip. */
        ssel = 0;
        
        /* Read and write data at the same time. */
	for (int i = 0; i < NumBytes; i++) {
            pBufferToRead[i] = write(pBufferToWrite[i]);
	}

        /* Unselect the chip. */
        ssel = 1;

        return 0;
    }

    /**
     * @brief      Writes a buffer to the SPI peripheral device in 16-bit data mode 
     *             using synchronous SPI communication.
     * @param[in]  pBuffer pointer to the buffer of data to send.
     * @param[in]  ssel GPIO of the SSEL pin of the SPI device to be used for communication.
     * @param[in]  NumValuesToWrite number of 16-bit values to write.
     * @retval     0 if ok.
     * @retval     -1 if data format error.
     * @note       When using the SPI in Interrupt-mode, remember to disable interrupts
     *             before calling this function and to enable them again after.
     * @note       In order to guarantee this method to work correctly you have to
     *             pass buffers which are correctly aligned.
     */
    int spi_write(uint16_t* pBuffer, DigitalOut &ssel, uint16_t NumValuesToWrite)
    {
	/* Check data format */
	if(_bits != 16) return -1;

        /* Select the chip. */
        ssel = 0;
        
        /* Write data. */
	for (int i = 0; i < NumValuesToWrite; i++) {
	    write(htons(pBuffer[i]));
	}

        /* Unselect the chip. */
        ssel = 1;

        return 0;
    }

    /**
     * @brief      Reads a buffer from the SPI peripheral device in 16-bit data mode 
     *             using synchronous SPI communication.
     * @param[out] pBuffer pointer to the buffer to read data into.
     * @param[in]  ssel GPIO of the SSEL pin of the SPI device to be used for communication.
     * @param[in]  NumValuesToRead number of 16-bit values to read.
     * @retval     0 if ok.
     * @retval     -1 if data format error.
     * @note       When using the SPI in Interrupt-mode, remember to disable interrupts
     *             before calling this function and to enable them again after.
     * @note       In order to guarantee this method to work correctly you have to
     *             pass buffers which are correctly aligned.
     */
    int spi_read(uint16_t* pBuffer, DigitalOut &ssel, uint16_t NumValuesToRead)
    {
	/* Check data format */
	if(_bits != 16) return -1;

        /* Select the chip. */
        ssel = 0;
        
        /* Read data. */
	for (int i = 0; i < NumValuesToRead; i++) {
	    pBuffer[i] = ntohs((uint16_t)write(0));
	}

        /* Unselect the chip. */
        ssel = 1;

        return 0;
    }

    /**
     * @brief      Reads and write a buffer from/to the SPI peripheral device at the same time 
     *             in 16-bit data mode using synchronous SPI communication.
     * @param[out] pBufferToRead pointer to the buffer to read data into.
     * @param[in]  pBufferToWrite pointer to the buffer of data to send.
     * @param[in]  ssel GPIO of the SSEL pin of the SPI device to be used for communication.
     * @param[in]  NumValues number of 16-bit values to read and write.
     * @retval     0 if ok.
     * @retval     -1 if data format error.
     * @note       When using the SPI in Interrupt-mode, remember to disable interrupts
     *             before calling this function and to enable them again after.
     * @note       In order to guarantee this method to work correctly you have to
     *             pass buffers which are correctly aligned.
     */
    int spi_read_write(uint16_t* pBufferToRead, uint16_t* pBufferToWrite, DigitalOut &ssel, uint16_t NumValues)
    {
	/* Check data format */
	if(_bits != 16) return -1;

	/* Select the chip. */
        ssel = 0;
        
        /* Read and write data at the same time. */
	for (int i = 0; i < NumValues; i++) {
	    pBufferToRead[i] = ntohs((uint16_t)write(htons(pBufferToWrite[i])));
	}

        /* Unselect the chip. */
        ssel = 1;

        return 0;
    }

protected:
    inline uint16_t htons(uint16_t x) {
#ifndef __DEV_SPI_BIG_ENDIAN
	return (((x)<<8)|((x)>>8));
#else  // __DEV_SPI_BIG_ENDIAN
	return (x);
#endif // __DEV_SPI_BIG_ENDIAN
    }

    inline uint16_t ntohs(uint16_t x) {
	return htons(x);
    }
};

#endif /* __DEV_SPI_H */
