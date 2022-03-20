/**
 ******************************************************************************
 * @file    XNucleoIHM02A1.cpp
 * @author  AST / Software Platforms and Cloud
 * @version V1.0
 * @date    November 3rd, 2015
 * @brief   Implementation file for the X_NUCLEO_IHM02A1 expansion board.
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


/* Generated with STM32CubeTOO -----------------------------------------------*/


/* Includes ------------------------------------------------------------------*/

/* ACTION 1 ------------------------------------------------------------------*
 * Include here platform specific header files.                               *
 *----------------------------------------------------------------------------*/
#include "mbed.h"
#include "DevSPI.h"
/* ACTION 2 ------------------------------------------------------------------*
 * Include here expansion board specific header files.                        *
 *----------------------------------------------------------------------------*/
#include "XNucleoIHM02A1.h"


/* Variables -----------------------------------------------------------------*/

/* Number of expansion boards. */
uint8_t XNucleoIHM02A1::number_of_boards = 0;


/* Methods -------------------------------------------------------------------*/

/**
 * @brief Constructor.
 * @param init_0        pointer to the initialization structure of the first motor.
 * @param init_1        pointer to the initialization structure of the second motor.
 * @param flag_irq      pin name of the FLAG pin of the component.
 * @param busy_irq      pin name of the BUSY pin of the component.
 * @param standby_reset pin name of the STBY\RST pin of the component.
 * @param ssel          pin name of the SSEL pin of the SPI device to be used for communication.
 * @param spi           SPI device to be used for communication.
 */
XNucleoIHM02A1::XNucleoIHM02A1(L6470_init_t *init_0, L6470_init_t *init_1, PinName flag_irq, PinName busy_irq, PinName standby_reset, PinName ssel, DevSPI *spi) : dev_spi(spi)
{
    /* Checking stackability. */
    if (!(number_of_boards < EXPBRD_MOUNTED_NR_MAX)) {
        error("Instantiation of the X_NUCLEO_IHM02A1 expansion board failed: it can be stacked up to %d times.\r\n", EXPBRD_MOUNTED_NR_MAX);
    }
    instance_id = number_of_boards++;

    /* SPI communication. */
    if (dev_spi == NULL) {
        dev_spi = new DevSPI(X_NUCLEO_IHM02A1_PIN_SPI_MOSI, X_NUCLEO_IHM02A1_PIN_SPI_MISO, X_NUCLEO_IHM02A1_PIN_SPI_SCLK);
    }

    /* Instantiating the components. */
    /* ACTION 3 --------------------------------------------------------------*
     * Instantiate here the expansion board's components.                     *
     *                                                                        *
     * Example:                                                               *
     *   component_1 = new COMPONENT_1(ssel, *dev_spi);                       *
     *   component_2 = new COMPONENT_2(ssel, *dev_spi);                       *
     *------------------------------------------------------------------------*/
    components[0] = l6470_0 = new L6470(flag_irq, busy_irq, standby_reset, ssel, *dev_spi);
    components[1] = l6470_1 = new L6470(flag_irq, busy_irq, standby_reset, ssel, *dev_spi);

    /* Initializing the components. */
    init_components[0] = init_0;
    init_components[1] = init_1;
    if (!init()) {
        error("Initialization of the X_NUCLEO_IHM02A1 expansion board failed.\r\n");
    }
}

/**
 * @brief Constructor.
 * @param init_0        pointer to the initialization structure of the first motor.
 * @param init_1        pointer to the initialization structure of the second motor.
 * @param flag_irq      pin name of the FLAG pin of the component.
 * @param busy_irq      pin name of the BUSY pin of the component.
 * @param standby_reset pin name of the STBY\RST pin of the component.
 * @param ssel          pin name of the SSEL pin of the SPI device to be used for communication.
 * @param mosi          pin name of the MOSI pin of the SPI device to be used for communication.
 * @param miso          pin name of the MISO pin of the SPI device to be used for communication.
 * @param sclk          pin name of the SCLK pin of the SPI device to be used for communication.
 */
XNucleoIHM02A1::XNucleoIHM02A1(L6470_init_t *init_0, L6470_init_t *init_1, PinName flag_irq, PinName busy_irq, PinName standby_reset, PinName ssel, PinName mosi, PinName miso, PinName sclk)
{
    /* Checking stackability. */
    if (!(number_of_boards < EXPBRD_MOUNTED_NR_MAX)) {
        error("Instantiation of the X_NUCLEO_IHM02A1 expansion board failed: it can be stacked up to %d times.\r\n", EXPBRD_MOUNTED_NR_MAX);
    }
    instance_id = number_of_boards++;

    /* SPI communication. */
    dev_spi = new DevSPI(mosi, miso, sclk);

    /* Instantiating the components. */
    /* ACTION 3 --------------------------------------------------------------*
     * Instantiate here the expansion board's components.                     *
     *                                                                        *
     * Example:                                                               *
     *   component_1 = new COMPONENT_1(ssel, *dev_spi);                       *
     *   component_2 = new COMPONENT_2(ssel, *dev_spi);                       *
     *------------------------------------------------------------------------*/
    components[0] = l6470_0 = new L6470(flag_irq, busy_irq, standby_reset, ssel, *dev_spi);
    components[1] = l6470_1 = new L6470(flag_irq, busy_irq, standby_reset, ssel, *dev_spi);

    /* Initializing the components. */
    init_components[0] = init_0;
    init_components[1] = init_1;
    if (!init()) {
        error("Initialization of the X_NUCLEO_IHM02A1 expansion board failed.\r\n");
    }
}

/**
 * @brief Initializing the X_NUCLEO_IHM02A1 board.
 * @retval true if initialization is successful, false otherwise.
 */
bool XNucleoIHM02A1::init(void)
{
    /* Initializing the components. */
    /* ACTION 4 --------------------------------------------------------------*
     * Initialize here the expansion board's components.                      *
     *                                                                        *
     * Example:                                                               *
     *   return (init_COMPONENT_1() && init_COMPONENT_2());                   *
     *------------------------------------------------------------------------*/
    return (init_L6470_0() && init_L6470_1());
}

/* ACTION 5 ------------------------------------------------------------------*
 * Implement here an initialization method for each expansion board's         *
 * component.                                                                 *
 *                                                                            *
 * Example:                                                                   *
 *   bool ExpansionBoard::init_COMPONENT_1(void)                              *
 *   {                                                                        *
 *     // Verifying identity.                                                 *
 *     uint8_t id = 0;                                                        *
 *     int ret = component_1->read_id(&id);                                   *
 *     if ((ret != COMPONENT_OK) || (id != I_AM_COMPONENT_1))                 *
 *     {                                                                      *
 *       delete component_1;                                                  *
 *       component_1 = NULL;                                                  *
 *       return true;                                                         *
 *     }                                                                      *
 *                                                                            *
 *     // Configuration.                                                      *
 *     COMPONENT_init_t init;                                                 *
 *     init.property_1 = COMPONENT_1_PROPERY_1_INIT;                          *
 *     init.property_N = COMPONENT_1_PROPERY_N_INIT;                          *
 *                                                                            *
 *     // Initialization.                                                     *
 *     if (component_1->init(&init) != COMPONENT_OK)                          *
 *       return false;                                                        *
 *                                                                            *
 *     return true;                                                           *
 *   }                                                                        *
 *----------------------------------------------------------------------------*/
/**
 * @brief  Initialize the L6470 component.
 * @retval true if initialization is successful, false otherwise.
 */
bool XNucleoIHM02A1::init_L6470_0(void)
{
    /* Initialization. */
    if (l6470_0->init((void *) init_components[0]) != COMPONENT_OK) {
        return false;
    }

    return true;
}

/**
 * @brief  Initialize the L6470 component.
 * @retval true if initialization is successful, false otherwise.
 */
bool XNucleoIHM02A1::init_L6470_1(void)
{
    /* Initialization. */
    if (l6470_1->init((void *) init_components[1]) != COMPONENT_OK) {
        return false;
    }

    return true;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/ 
