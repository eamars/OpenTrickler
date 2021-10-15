/**
 ******************************************************************************
 * @file       L6470_def.h
 * @date       01/10/2014 12:00:00
 * @brief      This file contains definitions, exported variables and function
 *             prototypes related to the L6470.
 ******************************************************************************
 *
 * COPYRIGHT(c) 2014 STMicroelectronics
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


/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __L6470_H
#define __L6470_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/

#include "../Common/microstepping_motor_def.h"


/* Definitions ---------------------------------------------------------------*/

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Components
 * @{
 */  
  
/** @addtogroup L6470
 * @{
 */
  
/** @defgroup L6470_Exported_Defines L6470_Exported_Defines
 * @{
 */  

/**
  * @defgroup   L6470_Exported_Constants
  * @brief      L6470 Exported Constants.
  * @{
  */

/**
  * @defgroup   L6470_Register_Max_Values
  * @brief      Maximum values for L6470 registers.
  * @{
  */

#define L6470_MAX_POSITION          (0x1FFFFF)          //!< Max position
#define L6470_MIN_POSITION          (-(0x200000))       //!< Min position
#define L6470_POSITION_RANGE        ((uint32_t)(L6470_MAX_POSITION - L6470_MIN_POSITION))   //!< Position range
#define L6470_MAX_SPEED             (0xFFFFF)           //!< max value of SPEED
#define L6470_MAX_ACC               (0xFFF)             //!< max value of ACC
#define L6470_MAX_DEC               (0xFFF)             //!< max value of DEC
#define L6470_MAX_MAX_SPEED         (0x3FF)             //!< max value of MAX_SPEED
#define L6470_MAX_MIN_SPEED         (0xFFF)             //!< max value of MIN_SPEED
#define L6470_MAX_FS_SPD            (0x3FF)             //!< max value of FS_SPD
#define L6470_MAX_INT_SPEED         (0x3FFF)            //!< max value of INT_SPEED
#define L6470_MAX_ST_SLP            (0xFF)              //!< max value of ST_SLP
#define L6470_MAX_FN_SLP_ACC        (0xFF)              //!< max value of FN_SLP_ACC
#define L6470_MAX_FN_SLP_DEC        (0xFF)              //!< max value of FN_SLP_DEC
#define L6470_MAX_OCD_TH            (0xF)               //!< max value of OCD_TH
#define L6470_MAX_STALL_TH          (0x7F)              //!< max value of STALL_TH

/**
  * @}
  */ /* End of L6470_Register_Max_Values */

#define L6470REGIDSIZE          25                  //!< Max number of identifiers of L6470 Registers
#define L6470APPCMDIDSIZE       19                  //!< Max number of identifiers of L6470 Application Commands
#define L6470DIRIDSIZE           2                  //!< Max number of identifiers of L6470 directions
#define L6470ACTIDSIZE           2                  //!< Max number of identifiers of actions to perform about ABS_POS register
#define L6470MAXSPICMDBYTESIZE   4                  //!< Max number of byte to send via SPI to perform an application command
#define L6470DAISYCHAINSIZE      2                  //!< Max number of identifiers of L6470 in daisy chain configuration

#define L6470_MAX_SPEED_VALUE   ((float)15610)      //!< max value for the speed in step/s
#define L6470_MAX_ACC_VALUE     ((float)59590)      //!< max value for the acceleration in step/s^2
#define L6470_MAX_DEC_VALUE     ((float)59590)      //!< max value for the acceleration in step/s^2
#define L6470_MAX_DEC_VALUE     ((float)59590)      //!< max value for the acceleration in step/s^2

#define OCD_TH_STEP             ((float)375)      //!< Minimum step for OCD_TH register in mAmpere
#define STALL_TH_STEP           ((float)31.25)    //!< Minimum step for STALL_TH register in mAmpere

#define L6470_ACC_CONV          ((float)0.068719)   //!< Conversion factor for acceleration value from step/s^2 to the right value
#define L6470_DEC_CONV          ((float)0.068719)   //!< Conversion factor for deceleration value from step/s^2 to the right value
#define L6470_MAXSPEED_CONV     ((float)0.065536)   //!< Conversion factor for max speed value from step/s to the right value
#define L6470_MINSPEED_CONV     ((float)4.194304)   //!< Conversion factor for min speed value from step/s to the right value
#define L6470_SPEED_CONV        ((float)67.108864)  //!< Conversion factor for speed value from step/s to the right value


/* Types ---------------------------------------------------------------------*/

/**
  * @addtogroup BSP
  * @{
  */

/**
  * @addtogroup Components
  * @{
  */

/**
  * @defgroup L6470
  * @brief    Tools to manage the L6470 Stepper Motor Driver.
  * @{
  */

/**
  * @defgroup   L6470_Exported_Types
  * @brief      L6470 Exported Types.
  * @{
  */

/**
  * @brief  The structure to store some features of the L6470 Registers.
  */
typedef struct {
  uint8_t Address;                  //!< Register Address
  uint8_t Name[12];                 //!< Register Name
  uint8_t LengthBit;                //!< Register Length in bits
  uint8_t LengthByte;               //!< Register Length in bytes
  uint32_t ResetValue;              //!< Register Reset Value
} sL6470_Register_t;

/**
  * @brief  The structure to store some features of the L6470 Application Commands.
  */
typedef struct {
  uint8_t Mnemonic[12];             //!< AppCmd Mnemonic
  uint8_t BinaryCode;               //!< AppCmd Binary Code
  uint8_t NrOfParameters;           //!< AppCmd number of needed parameters
} sL6470_ApplicationCommand_t;

/**
  * @brief  The structure to store some features about the L6470 Motor Direction.
  */
typedef struct {
  uint8_t Mnemonic[8];              //!< L6470 Direction Mnemonic
  uint8_t BinaryCode;               //!< L6470 Direction Binary Code
} sL6470_Direction_t;

/**
  * @brief  The structure to store some features about the action taken with the L6470 ABS_POS register.
  */
typedef struct {
  uint8_t Mnemonic[4];              //!< ACT Mnemonic
  uint8_t BinaryCode;               //!< ACT Binary Code
} sL6470_ACT_t;

/**
  * @brief  The structure used to store the identifier of the L6470 application
  *         command and its the needed parameters.
  * @note   The data stored into this structure will be used to fill the matrix
  *         used by SPI to send the command to the L6470.
  */
typedef struct {
  eL6470_AppCmdId_t L6470_AppCmdId; //!< The identifier of the actual L6470 Application Command
  uint32_t p1;                      //!< The 1st parameter if needed
  uint32_t p2;                      //!< The 2nd parameter if needed
  uint32_t p3;                      //!< The 3rd parameter if needed
} sL6470_AppCmdPkg_t;

/**
  * @}
  */ /* End of L6470_Exported_Types */

/** 
 * @brief  L6470 driver initialization structure definition.
 */
/* ACTION --------------------------------------------------------------------*
 * Declare here the component's initialization structure, if any, one         *
 * variable per line without initialization.                                  *
 *                                                                            *
 * Example:                                                                   *
 *   typedef struct                                                           *
 *   {                                                                        *
 *     int frequency;                                                         *
 *     int update_mode;                                                       *
 *   } COMPONENT_Init_t;                                                      *
 *----------------------------------------------------------------------------*/
typedef struct
{
  float     motorvoltage;           //!< motor supply voltage in V
  float     fullstepsperrevolution; //!< min number of steps per revolution for the motor
  float     phasecurrent;           //!< max motor phase voltage in A
  float     phasevoltage;           //!< max motor phase voltage in V
  float     speed;                  //!< motor initial speed [step/s]
  float     acc;                    //!< motor acceleration [step/s^2] (comment for infinite acceleration mode)
  float     dec;                    //!< motor deceleration [step/s^2] (comment for infinite deceleration mode)
  float     maxspeed;               //!< motor maximum speed [step/s]
  float     minspeed;               //!< motor minimum speed [step/s]
  float     fsspd;                  //!< motor full-step speed threshold [step/s]
  float     kvalhold;               //!< holding kval [V]
  float     kvalrun;                //!< constant speed kval [V]
  float     kvalacc;                //!< acceleration starting kval [V]
  float     kvaldec;                //!< deceleration starting kval [V]
  float     intspeed;               //!< intersect speed for bemf compensation curve slope changing [step/s]
  float     stslp;                  //!< start slope [s/step]
  float     fnslpacc;               //!< acceleration final slope [s/step]
  float     fnslpdec;               //!< deceleration final slope [s/step]
  uint8_t   kterm;                  //!< thermal compensation factor (range [0, 15])
  float     ocdth;                  //!< ocd threshold [ma] (range [375 ma, 6000 ma])
  float     stallth;                //!< stall threshold [ma] (range [31.25 ma, 4000 ma])
  uint8_t   step_sel;               //!< step mode selection
  uint8_t   alarmen;                //!< alarm conditions enable
  uint16_t  config;                 //!< ic configuration
} L6470_init_t;

/** 
 * @brief  L6470 driver data structure definition.
 */ 
/* ACTION --------------------------------------------------------------------*
 * Declare here the structure of component's data, if any, one variable per   *
 * line without initialization.                                               *
 *                                                                            *
 * Example:                                                                   *
 *   typedef struct                                                           *
 *   {                                                                        *
 *       int T0_out;                                                          *
 *       int T1_out;                                                          *
 *       float T0_degC;                                                       *
 *       float T1_degC;                                                       *
 *   } COMPONENT_Data_t;                                                      *
 *----------------------------------------------------------------------------*/
typedef struct
{
  uint8_t L6470_Id;                                                                  //!< The L6470 identifier inside the daisy chain
  sL6470_Register_t *L6470_Register; //[L6470REGIDSIZE];                             //!< Array whose elements are a structure in which store information about the L6470 Registers (the address, the names, the length in bits, the reset value)
  sL6470_ApplicationCommand_t *L6470_ApplicationCommand; //[L6470APPCMDIDSIZE];      //!< Array whose elements are a structure in which store information about the L6470 Application Commands (the mnemonic name, the number of needed parameters, the related funtion to call)
  sL6470_Direction_t *L6470_Direction; //[L6470DIRIDSIZE];                           //!< The mnemonic names for the L6470 direction
  sL6470_ACT_t *L6470_ACT; //[L6470ACTIDSIZE];                                       //!< Action taken about ABS_POS register
  sL6470_AppCmdPkg_t L6470_AppCmdPkg[L6470DAISYCHAINSIZE];                           //!< To store the identifier of the actual L6470 application command and its the needed parameters
  uint8_t L6470_DaisyChainSpiTxStruct[L6470MAXSPICMDBYTESIZE][L6470DAISYCHAINSIZE];  //!< To store the matrix that contains the command data that are going to be sent by SPI to the L6470 daisy chain
  uint8_t L6470_DaisyChainSpiRxStruct[L6470MAXSPICMDBYTESIZE][L6470DAISYCHAINSIZE];  //!< To store the matrix that contains the received data by SPI from the L6470 daisy chain
  eFlagStatus_t L6470_DaisyChain_HalfPrepared; /* = ZERO_F; */                       //!< Boolean variable used when more than one L6470 into the daisy chain is going to be addressed for commanding
  sL6470_StatusRegister_t L6470_StatusRegister;                                      //!< To store the received L6470_StatusRegister
  sL6470_StatusRegister_t *pL6470_StatusRegister; /* = &L6470_StatusRegister; */     //!< Pointer to the L6470_StatusRegister variable
} L6470_Data_t;


/* Functions -----------------------------------------------------------------*/

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Components
 * @{
 */

/** @addtogroup L6470
 * @{
 */

/** @defgroup L6470_Imported_Functions L6470_Imported_Functions
 * @{
 */

/* ACTION --------------------------------------------------------------------*
 * Declare here extern platform-dependent APIs you might need (e.g.: I/O and  *
 * interrupt related functions), and implement them in a glue-logic file on   *
 * the target environment, for example within the "x_nucleo_board.c" file.    *
 * E.g.:                                                                      *
 *   extern status_t COMPONENT_IO_Init (void *handle);                        *
 *   extern status_t COMPONENT_IO_Read (handle, buf, regadd, bytes);          *
 *   extern status_t COMPONENT_IO_Write(handle, buf, regadd, bytes);          *
 *   extern void     COMPONENT_IO_ITConfig(void);                             *
 *----------------------------------------------------------------------------*/
extern void L6470_DISABLE(void);
extern void L6470_ENABLE(void);
extern void L6470_nCS_LOW(void);
extern void L6470_nCS_HIGH(void);
extern void L6470_SPI_Communication(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size, uint32_t Timeout);

#ifdef __cplusplus
}
#endif

#endif /* __L6470_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
