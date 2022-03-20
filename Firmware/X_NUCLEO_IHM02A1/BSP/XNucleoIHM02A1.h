/**
 ******************************************************************************
 * @file    XNucleoIHM02A1.h
 * @author  AST / Software Platforms and Cloud
 * @version V1.0
 * @date    November 3rd, 2015
 * @brief   Class header file for the X-NUCLEO-IHM02A1 expansion board.
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


/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __X_NUCLEO_IHM02A1_CLASS_H
#define __X_NUCLEO_IHM02A1_CLASS_H


/* Includes ------------------------------------------------------------------*/

/* ACTION 1 ------------------------------------------------------------------*
 * Include here platform specific header files.                               *
 *----------------------------------------------------------------------------*/
#include "mbed.h"
#include "DevSPI.h"
/* ACTION 2 ------------------------------------------------------------------*
 * Include here expansion board configuration's header files.                 *
 *----------------------------------------------------------------------------*/
#include "x_nucleo_ihm02a1_config.h"
/* ACTION 3 ------------------------------------------------------------------*
 * Include here expansion board's components' header files.                   *
 *                                                                            *
 * Example:                                                                   *
 *   #include "COMPONENT_1.h"                                                 *
 *   #include "COMPONENT_2.h"                                                 *
 *----------------------------------------------------------------------------*/
#include "L6470.h"


/* Classes -------------------------------------------------------------------*/

/** Class representing a X-NUCLEO-IHM02A1 board.
 */
class XNucleoIHM02A1
{
public:

    /*** Constructor, Destructor, and Initialization Methods ***/

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
    XNucleoIHM02A1(L6470_init_t *init_0, L6470_init_t *init_1, PinName flag_irq, PinName busy_irq, PinName standby_reset, PinName ssel, DevSPI *spi);

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
    XNucleoIHM02A1(L6470_init_t *init_0, L6470_init_t *init_1, PinName flag_irq, PinName busy_irq, PinName standby_reset, PinName ssel, PinName mosi, PinName miso, PinName sclk);

    /**
     * @brief Destructor.
     */
    ~XNucleoIHM02A1(void) {}

    /**
     * @brief Initializing the X-NUCLEO-IHM02A1 board.
     * @retval true if initialization is successful, false otherwise.
     */
    bool init(void);


    /*** Other Public Expansion Board Related Methods ***/

    /**
     * @brief  Getting the array of components.
     * @param  None.
     * @retval The array of components.
     */
    L6470 **get_components(void)
    {
        return components;
    }

    /**
      * @brief  Performing the actions set on the motors with calls to a number of
      *         "Prepare<Action>()" methods, one for each motor of the daisy-chain.
      * @param  None.
      * @retval A pointer to the results returned by the components, i.e. an
      *         integer value for each of them.
      */
    virtual uint32_t* perform_prepared_actions(void)
    {
        /* Performing pre-actions, if needed. */
        for (int m = 0; m < L6470DAISYCHAINSIZE; m++) {
            /*
               "GetPosition()" is needed by "PrepareSetMark()" at the time when the
               prepared actions get performed.
            */
            if (components[m]->get_prepared_action() == L6470::PREPARED_SET_MARK) {
                components[m]->prepare_set_mark((uint32_t) components[m]->get_position());
            }
        }
        
        /* Performing the prepared actions and getting back raw data. */
        uint8_t *raw_data = components[0]->perform_prepared_actions();

        /* Processing raw data. */
        for (int m = 0; m < L6470DAISYCHAINSIZE; m++) {
            results[m] = components[m]->get_result(raw_data);
        }

        /* Returning results. */
        return results;
    }


    /*** Public Expansion Board Related Attributes ***/

    /* ACTION 4 --------------------------------------------------------------*
     * Declare here a public attribute for each expansion board's component.  *
     * You will have to call these attributes' public methods within your     *
     * main program.                                                          *
     *                                                                        *
     *   Example:                                                             *
     *     COMPONENT_1 *component_1;                                          *
     *     COMPONENT_2 *component_2;                                          *
     *------------------------------------------------------------------------*/
    L6470 *l6470_0;
    L6470 *l6470_1;


protected:

    /*** Protected Expansion Board Related Initialization Methods ***/

    /* ACTION 5 --------------------------------------------------------------*
     * Declare here a protected initialization method for each expansion      *
     * board's component.                                                     *
     *                                                                        *
     * Example:                                                               *
     *   bool init_COMPONENT_1(void);                                         *
     *   bool init_COMPONENT_2(void);                                         *
     *------------------------------------------------------------------------*/
    bool init_L6470_0(void);
    bool init_L6470_1(void);


    /*** Component's Instance Variables ***/

    /* IO Device. */
    DevSPI *dev_spi;

    /* Components. */
    L6470 *components[L6470DAISYCHAINSIZE];
    
    /* Components' initialization. */
    L6470_init_t *init_components[L6470DAISYCHAINSIZE];

    /* Results of prepared actions. */
    uint32_t results[L6470DAISYCHAINSIZE];

    /* ACTION 6 --------------------------------------------------------------*
     * Declare here the component's static and non-static data, one variable  *
     * per line.                                                              *
     *                                                                        *
     * Example:                                                               *
     *   int instance_id;                                                     *
     *   static int number_of_instances;                                      *
     *------------------------------------------------------------------------*/
    /* Data. */
    uint8_t instance_id;

    /* Static data. */
    static uint8_t number_of_boards;
};

#endif /* __X_NUCLEO_IHM02A1_CLASS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
