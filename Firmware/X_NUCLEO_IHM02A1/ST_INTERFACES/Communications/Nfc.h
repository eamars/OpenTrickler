/**
 ******************************************************************************
 * @file    Nfc.h
 * @author  ST Central Labs
 * @version V1.0.0
 * @date    13-April-2015
 * @brief   This file contains the abstract class describing the interface of a
 *          nfc component.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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


/* Generated with Stm32CubeTOO -----------------------------------------------*/

/* Define to prevent from recursive inclusion --------------------------------*/

#ifndef __NFC_CLASS_H
#define __NFC_CLASS_H


/* Includes ------------------------------------------------------------------*/

#include <Component.h>

typedef enum {
    NFC_SUCCESS = 0,
} NFC_t;
/* Error codes are in component driver */


/* Classes  ------------------------------------------------------------------*/

/**
 * An abstract class for Nfc components. 
 */
class Nfc : public Component {
public:

    /**
     * Read data from the tag.
     * @param offset Read offset.
     * @param nb_bytes_to_read Number of bytes to read.
     * @param[out] p_buffer_read Buffer to store the read data into.
     * @return NFC_SUCCESS if no errors 
     */
    virtual int read_binary(uint16_t offset, uint8_t nb_bytes_to_read, uint8_t *p_buffer_read) = 0;

    /**
     * Write data to the tag.
     * @param offset Write offset.
     * @param nb_bytes_to_write Number of bytes to write.
     * @param p_buffer_write Buffer to write.
     * @return NFC_SUCCESS if no errors
     */
    virtual int update_binary(uint16_t offset, uint8_t nb_bytes_to_write, uint8_t *p_buffer_write) = 0;

    /**
     * @brief Destructor.
     */
    virtual ~Nfc() {};
};

#endif /* __NFC_CLASS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/ 
