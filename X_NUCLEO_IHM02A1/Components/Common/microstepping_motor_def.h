/**
 ******************************************************************************
 * @file    microstepping_motor_def.h
 * @author  IPD SYSTEM LAB & TECH MKTG
 * @version V0.0.1
 * @date    04-June-2015
 * @brief   This file contains all the functions prototypes for the microstepping
 *          motor driver with motion engine.   
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
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

#ifndef __MICROSTEPPINGMOTOR_H
#define __MICROSTEPPINGMOTOR_H

#ifdef __cplusplus
 extern "C" {
#endif 


/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "component_def.h"
   

/* Types ---------------------------------------------------------------------*/

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */ 

/** @defgroup MicrosteppingMotorDriver
  * @{
  */
    
/** @defgroup StepperMotorExportedTypes
  * @{
  */

/**
  * @brief The L6470 Registers Identifiers.
  */
typedef enum {
  L6470_ABS_POS_ID = 0,         //!< Current position
  L6470_EL_POS_ID,              //!< Electrical position
  L6470_MARK_ID,                //!< Mark position
  L6470_SPEED_ID,               //!< Current speed
  L6470_ACC_ID,                 //!< Acceleration
  L6470_DEC_ID,                 //!< Deceleration
  L6470_MAX_SPEED_ID,           //!< Maximum speed
  L6470_MIN_SPEED_ID,           //!< Minimum speed
  L6470_FS_SPD_ID,              //!< Full-step speed
  L6470_KVAL_HOLD_ID,           //!< Holding KVAL
  L6470_KVAL_RUN_ID,            //!< Constant speed KVAL
  L6470_KVAL_ACC_ID,            //!< Acceleration starting KVAL
  L6470_KVAL_DEC_ID,            //!< Deceleration starting KVAL
  L6470_INT_SPEED_ID,           //!< Intersect speed
  L6470_ST_SLP_ID,              //!< Start slope
  L6470_FN_SLP_ACC_ID,          //!< Acceleration final slope
  L6470_FN_SLP_DEC_ID,          //!< Deceleration final slope
  L6470_K_THERM_ID,             //!< Thermal compensation factor
  L6470_ADC_OUT_ID,             //!< ADC output, (the reset value is according to startup conditions)
  L6470_OCD_TH_ID,              //!< OCD threshold
  L6470_STALL_TH_ID,            //!< STALL threshold
  L6470_STEP_MODE_ID,           //!< Step mode
  L6470_ALARM_EN_ID,            //!< Alarm enable
  L6470_CONFIG_ID,              //!< IC configuration
  L6470_STATUS_ID               //!< Status, (the reset value is according to startup conditions)  
} eL6470_RegId_t;

/**
  * @brief The L6470 Application Commands Identifiers.
  */
typedef enum {
  L6470_NOP_ID = 0,             //!< Nothing
  L6470_SETPARAM_ID,            //!< Writes VALUE in PARAM register
  L6470_GETPARAM_ID,            //!< Returns the stored value in PARAM register
  L6470_RUN_ID,                 //!< Sets the target speed and the motor direction
  L6470_STEPCLOCK_ID,           //!< Puts the device into Step-clock mode and imposes DIR direction
  L6470_MOVE_ID,                //!< Makes N_STEP (micro)steps in DIR direction (Not performable when motor is running)
  L6470_GOTO_ID,                //!< Brings motor into ABS_POS position (minimum path)
  L6470_GOTODIR_ID,             //!< Brings motor into ABS_POS position forcing DIR direction
  L6470_GOUNTIL_ID,             //!< Performs a motion in DIR direction with speed SPD until SW is closed, the ACT action is executed then a SoftStop takes place
  L6470_RELEASESW_ID,           //!< Performs a motion in DIR direction at minimum speed until the SW is released (open), the ACT action is executed then a HardStop takes place
  L6470_GOHOME_ID,              //!< Brings the motor into HOME position
  L6470_GOMARK_ID,              //!< Brings the motor into MARK position
  L6470_RESETPOS_ID,            //!< Resets the ABS_POS register (set HOME position)
  L6470_RESETDEVICE_ID,         //!< Device is reset to power-up conditions
  L6470_SOFTSTOP_ID,            //!< Stops motor with a deceleration phase
  L6470_HARDSTOP_ID,            //!< Stops motor immediately
  L6470_SOFTHIZ_ID,             //!< Puts the bridges into high impedance status after a deceleration phase
  L6470_HARDHIZ_ID,             //!< Puts the bridges into high impedance status immediately
  L6470_GETSTATUS_ID            //!< Returns the STATUS register value
} eL6470_AppCmdId_t;

/**
  * @brief The L6470 Status Register Flag identifiers.
  */
typedef enum {
  HiZ_ID = 0,                       //!< HiZ flag identifier inside the L6470 Status Register
  BUSY_ID,                          //!< BUSY flag identifier inside the L6470 Status Register
  SW_F_ID,                          //!< SW_F flag identifier inside the L6470 Status Register
  SW_EVN_ID,                        //!< SW_EVN flag identifier inside the L6470 Status Register
  DIR_ID,                           //!< DIR flag identifier inside the L6470 Status Register
  MOT_STATUS_ID,                    //!< MOT_STATUS flag identifier inside the L6470 Status Register
  NOTPERF_CMD_ID,                   //!< NOTPERF_CMD flag identifier inside the L6470 Status Register
  WRONG_CMD_ID,                     //!< WRONG_CMD flag identifier inside the L6470 Status Register
  UVLO_ID,                          //!< UVLO flag identifier inside the L6470 Status Register
  TH_WRN_ID,                        //!< TH_WRN flag identifier inside the L6470 Status Register
  TH_SD_ID,                         //!< TH_SD flag identifier inside the L6470 Status Register
  OCD_ID,                           //!< OCD flag identifier inside the L6470 Status Register
  STEP_LOSS_A_ID,                   //!< STEP_LOSS_A flag identifier inside the L6470 Status Register
  STEP_LOSS_B_ID,                   //!< STEP_LOSS_B flag identifier inside the L6470 Status Register
  SCK_MOD_ID                        //!< SCK_MOD flag identifier inside the L6470 Status Register
} eL6470_StatusRegisterFlagId_t;

/**
  * @brief The L6470 Direction identifiers.
  */
typedef enum {
  L6470_DIR_REV_ID = 0,             //!< Reverse direction
  L6470_DIR_FWD_ID                  //!< Forward direction
} eL6470_DirId_t;

/**
  * @brief The L6470 Action identifiers about ABS_POS register.
  */
typedef enum {
  L6470_ACT_RST_ID = 0,             //!< ABS_POS register is reset
  L6470_ACT_CPY_ID                  //!< ABS_POS register value is copied into the MARK register
} eL6470_ActId_t;

/**
  * @brief The L6470 Status Register Flag states.
  */
typedef enum {
  ZERO_F = 0,                       //!< The flag is '0'
  ONE_F = !ZERO_F                   //!< The flag is '1'
} eFlagStatus_t;

/**
  * @brief The L6470 Motor Directions.
  */
typedef enum {
  REVERSE_F = 0,                    //!< Reverse motor direction
  FORWARD_F = !REVERSE_F            //!< Forward motor direction
} eMotorDirection_t;

/**
  * @brief The L6470 Motor Status.
  */
typedef enum {
  STOPPED_F = 0,                    //!< Stopped
  ACCELERATION_F = 1,               //!< Acceleration
  DECELERATION_F = 2,               //!< Deceleration
  CONSTANTSPEED_F = 3               //!< Constant speed
} eMotorStatus_t;

/**
  * @brief  The possible stepping modes for L6470.
  */
typedef enum
{
  FULL_STEP       = 0x00,   //!< Full-step
  HALF_STEP       = 0x01,   //!< Half-step
  MICROSTEP_1_4   = 0x02,   //!< 1/4 microstep
  MICROSTEP_1_8   = 0x03,   //!< 1/8 microstep
  MICROSTEP_1_16  = 0x04,   //!< 1/16 microstep
  MICROSTEP_1_32  = 0x05,   //!< 1/32 microstep
  MICROSTEP_1_64  = 0x06,   //!< 1/64 microstep
  MICROSTEP_1_128 = 0x07    //!< 1/128 microstep
} eMotorStepMode_t;

/**
  * @brief  The identifiers for the possible L6470 alarm conditions.
  */
typedef enum
{
  L6470_OVERCURRENT                       = 0x01, //!< Overcurrent
  L6470_THERMAL_SHUTDOWN                  = 0x02, //!< Thermal shutdown
  L6470_THERMAL_WARNING                   = 0x04, //!< Thermal warning
  L6470_UNDERVOLTAGE                      = 0x08, //!< Undervoltage
  L6470_STALL_DETECTION_A                 = 0x10, //!< Stall detection (Bridge A)
  L6470_STALL_DETECTION_B                 = 0x20, //!< Stall detection (Bridge B)
  L6470_SWITCH_TURN_ON_EVENT              = 0x40, //!< Switch turn-on event
  L6470_WRONG_OR_NON_PERFORMABLE_COMMAND  = 0x80  //!< Wrong or non-performable command
} eL6470_AlarmCondition_t;

/**
  * @brief The L6470 STEP_MODE Register (see L6470 DataSheet for more details).
  */
typedef struct {
  uint8_t STEP_SEL: 3;              //!< Step mode
  uint8_t WRT: 1;                   //!< When the register is written, this bit should be set to 0.
  uint8_t SYNC_SEL: 3;              //!< Synchronization selection
  uint8_t SYNC_EN: 1;               //!< Synchronization enable
} sL6470_StepModeRegister_t;

/**
  * @brief The L6470 ALARM_EN Register (see L6470 DataSheet for more details).
  */
typedef struct {
  uint8_t OCD_EN: 1;                //!< Overcurrent
  uint8_t TH_SD_EN: 1;              //!< Thermal shutdown
  uint8_t TH_WRN_EN: 1;             //!< Thermal warning
  uint8_t UVLO_EN: 1;               //!< Undervoltage
  uint8_t STEP_LOSS_A_EN: 1;        //!< Stall detection (Bridge A)
  uint8_t STEP_LOSS_B_EN: 1;        //!< Stall detection (Bridge B)
  uint8_t SW_EVN_EN: 1;             //!< Switch turn-on event
  uint8_t WRONG_NOTPERF_CMD_EN: 1;  //!< Wrong or non-performable command
} sL6470_AlarmEnRegister_t;

/**
  * @brief The L6470 CONFIG Register (see L6470 DataSheet for more details).
  */
typedef struct {
  uint8_t OSC_SEL: 3;               //!< Oscillator Selection
  uint8_t EXT_CLK: 1;               //!< External Clock
  uint8_t SW_MODE: 1;               //!< Switch mode
  uint8_t EN_VSCOMP: 1;             //!< Motor supply voltage compensation
  uint8_t RESERVED: 1;              //!< RESERVED
  uint8_t OC_SD: 1;                 //!< Overcurrent event
  uint8_t POW_SR: 2;                //!< Output slew rate
  uint8_t F_PWM_DEC: 3;             //!< Multiplication factor
  uint8_t F_PWM_INT: 3;             //!< Integer division factor
} sL6470_ConfigRegister_t;

/**
  * @brief The L6470 STATUS Register (see L6470 DataSheet for more details).
  */
typedef struct {
  uint8_t HiZ: 1;                   //!< The bridges are in high impedance state (the flag is active high)
  uint8_t BUSY: 1;                  //!< BUSY pin status (the flag is active low)
  uint8_t SW_F: 1;                  //!< SW input status (the flag is low for open and high for closed)
  uint8_t SW_EVN: 1;                //!< Switch turn-on event (the flag is active high)
  uint8_t DIR: 1;                   //!< The current motor direction (1 as forward, 0 as reverse)
  uint8_t MOT_STATUS: 2;            //!< The current motor status (0 as stopped, 1 as acceleration, 2 as deceleration, 3 as constant speed)
  uint8_t NOTPERF_CMD: 1;           //!< The command received by SPI cannot be performed (the flag is active high)
  uint8_t WRONG_CMD: 1;             //!< The command received by SPI does not exist at all (the flag is active high)
  uint8_t UVLO: 1;                  //!< Undervoltage lockout or reset events (the flag is active low)
  uint8_t TH_WRN: 1;                //!< Thermal warning event (the flag is active low)
  uint8_t TH_SD: 1;                 //!< Thermal shutdown event (the flag is active low)
  uint8_t OCD: 1;                   //!< Overcurrent detection event (the flag is active low)
  uint8_t STEP_LOSS_A: 1;           //!< Stall detection on bridge A (the flag is active low)
  uint8_t STEP_LOSS_B: 1;           //!< Stall detection on bridge B (the flag is active low)
  uint8_t SCK_MOD: 1;               //!< Step-clock mode (the flag is active high)
} sL6470_StatusRegister_t;

/**
  * @brief Stepper Motor Registers
  */
typedef struct
{
  uint32_t  ABS_POS;                //!< CurrentPosition Register
  uint16_t  EL_POS;                 //!< ElectricalPosition Register
  uint32_t  MARK;                   //!< MarkPosition Register
  uint32_t  SPEED;                  //!< CurrentSpeed Register
  uint16_t  ACC;                    //!< Acceleration Register
  uint16_t  DEC;                    //!< Deceleration Register
  uint16_t  MAX_SPEED;              //!< MaximumSpeed Register
  uint16_t  MIN_SPEED;              //!< MinimumSpeed Register
  uint16_t  FS_SPD;                 //!< FullStepSpeed Register
  uint8_t   KVAL_HOLD;              //!< HoldingKval Register
  uint8_t   KVAL_RUN;               //!< ConstantSpeedKval Register
  uint8_t   KVAL_ACC;               //!< AccelerationStartingKval Register
  uint8_t   KVAL_DEC;               //!< DecelerationStartingKval Register
  uint16_t  INT_SPEED;              //!< IntersectSpeed Register
  uint8_t   ST_SLP;                 //!< StartSlope Register
  uint8_t   FN_SLP_ACC;             //!< AccelerationFinalSlope Register
  uint8_t   FN_SLP_DEC;             //!< DecelerationFinalSlope Register
  uint8_t   K_THERM;                //!< ThermalCompensationFactor Register
  uint8_t   ADC_OUT;                //!< AdcOutput Register
  uint8_t   OCD_TH;                 //!< OcdThreshold Register
  uint8_t   STALL_TH;               //!< StallThreshold Register
  uint8_t   STEP_MODE;              //!< StepMode Register
  uint8_t   ALARM_EN;               //!< AlarmEnable Register
  uint16_t  CONFIG;                 //!< Config Register
  uint16_t  STATUS;                 //!< Status Register
} StepperMotorRegister_t;

/** 
 * @brief  MICROSTEPPING_MOTOR driver virtual table structure definition.
 */
typedef struct
{
  /* ACTION ----------------------------------------------------------------*
   * Declare here the component's generic functions.                        *
   * Tag this group of functions with the " Generic " C-style comment.      *
   * A component's interface has to define at least the two generic         *
   * functions provided here below within the "Example" section, as the     *
   * first and second functions of its Virtual Table. They have to be       *
   * specified exactly in the given way.                                    *
   *                                                                        *
   * Example:                                                               *
   *   status_t (*Init)   (void *handle, void *init);                       *
   *   status_t (*ReadID) (void *handle, uint8_t *id);                      *
   *------------------------------------------------------------------------*/
  /* Generic */
  status_t (*Init)(void *handle, void *init);
  status_t (*ReadID)(void *handle, uint8_t *id);
  
  /* ACTION ----------------------------------------------------------------*
   * Declare here the component's specific functions.                       *
   * Tag this group of functions with the " Specific " C-style comment.     *
   * Do not specify any function if not required.                           *
   *                                                                        *
   * Example:                                                               *
   *   status_t (*GetValue) (void *handle, float *f);                       *
   *------------------------------------------------------------------------*/
  /* Specific */
  void     (*SetParam)(void *handle, eL6470_RegId_t L6470_RegId, uint32_t Value);
  uint32_t (*GetParam)(void *handle, eL6470_RegId_t L6470_RegId);
  void     (*Run)(void *handle, eL6470_DirId_t L6470_DirId, uint32_t Speed);
  void     (*StepClock)(void *handle, eL6470_DirId_t L6470_DirId);
  void     (*Move)(void *handle, eL6470_DirId_t L6470_DirId, uint32_t N_Step);
  void     (*GoTo)(void *handle, uint32_t AbsPos);
  void     (*GoToDir)(void *handle, eL6470_DirId_t L6470_DirId, uint32_t AbsPos);
  void     (*GoUntil)(void *handle, eL6470_ActId_t L6470_ActId, eL6470_DirId_t L6470_DirId, uint32_t Speed);
  void     (*ReleaseSW)(void *handle, eL6470_ActId_t L6470_ActId, eL6470_DirId_t L6470_DirId);
  void     (*GoHome)(void *handle);
  void     (*GoMark)(void *handle);
  void     (*ResetPos)(void *handle);
  void     (*ResetDevice)(void *handle);
  void     (*SoftStop)(void *handle);
  void     (*HardStop)(void *handle);
  void     (*SoftHiZ)(void *handle);
  void     (*HardHiZ)(void *handle);
  uint16_t (*GetStatus)(void *handle);
  void     (*PrepareSetParam)(void *handle, eL6470_RegId_t L6470_RegId, uint32_t Value);
  void     (*PrepareGetParam)(void *handle, eL6470_RegId_t L6470_RegId);
  void     (*PrepareRun)(void *handle, eL6470_DirId_t L6470_DirId, uint32_t Speed);
  void     (*PrepareStepClock)(void *handle, eL6470_DirId_t L6470_DirId);
  void     (*PrepareMove)(void *handle, eL6470_DirId_t L6470_DirId, uint32_t N_Step);
  void     (*PrepareGoTo)(void *handle, uint32_t AbsPos);
  void     (*PrepareGoToDir)(void *handle, eL6470_DirId_t L6470_DirId, uint32_t AbsPos);
  void     (*PrepareGoUntil)(void *handle, eL6470_ActId_t L6470_ActId, eL6470_DirId_t L6470_DirId, uint32_t Speed);
  void     (*PrepareReleaseSW)(void *handle, eL6470_ActId_t L6470_ActId, eL6470_DirId_t L6470_DirId);
  void     (*PrepareGoHome)(void *handle);
  void     (*PrepareGoMark)(void *handle);
  void     (*PrepareResetPos)(void *handle);
  void     (*PrepareResetDevice)(void *handle);
  void     (*PrepareSoftStop)(void *handle);
  void     (*PrepareHardStop)(void *handle);
  void     (*PrepareSoftHiZ)(void *handle);
  void     (*PrepareHardHiZ)(void *handle);
  void     (*PrepareGetStatus)(void *handle);
  uint8_t  (*CheckStatusRegisterFlag)(void *handle, uint8_t L6470_StatusRegisterFlagId);
  uint8_t* (*PerformPreparedApplicationCommand)(void *handle);
  uint8_t* (*GetRegisterName)(void *handle, uint8_t id);
  int32_t  (*AbsPos_2_Position)(void *handle, uint32_t AbsPos);
  uint32_t (*Position_2_AbsPos)(void *handle, int32_t Position);
  float    (*Speed_2_Step_s)(void *handle, uint32_t Speed);
  uint32_t (*Step_s_2_Speed)(void *handle, float Step_s);
  float    (*Acc_2_Step_s2)(void *handle, uint16_t Acc);
  uint16_t (*Step_s2_2_Acc)(void *handle, float Step_s2);
  float    (*Dec_2_Step_s2)(void *handle, uint16_t Dec);
  uint16_t (*Step_s2_2_Dec)(void *handle, float Step_s2);
  float    (*MaxSpeed_2_Step_s)(void *handle, uint16_t MaxSpeed);
  uint16_t (*Step_s_2_MaxSpeed)(void *handle, float Step_s);
  float    (*MinSpeed_2_Step_s)(void *handle, uint16_t MinSpeed);
  uint16_t (*Step_s_2_MinSpeed)(void *handle, float Step_s);
  float    (*FsSpd_2_Step_s)(void *handle, uint16_t FsSpd);
  uint16_t (*Step_s_2_FsSpd)(void *handle, float Step_s);
  float    (*IntSpeed_2_Step_s)(void *handle, uint16_t IntSpeed);
  uint16_t (*Step_s_2_IntSpeed)(void *handle, float Step_s);
  float    (*StSlp_2_s_Step)(void *handle, uint8_t StSlp);
  uint8_t  (*s_Step_2_StSlp)(void *handle, float s_Step);
  float    (*FnSlpAcc_2_s_Step)(void *handle, uint8_t FnSlpAcc);
  uint8_t  (*s_Step_2_FnSlpAcc)(void *handle, float s_Step);
  float    (*FnSlpDec_2_s_Step)(void *handle, uint8_t FnSlpDec);
  uint8_t  (*s_Step_2_FnSlpDec)(void *handle, float s_Step);
  float    (*OcdTh_2_mA)(void *handle, uint8_t OcdTh);
  uint8_t  (*mA_2_OcdTh)(void *handle, float mA);
  float    (*StallTh_2_mA)(void *handle, uint8_t StallTh);
  uint8_t  (*mA_2_StallTh)(void *handle, float mA);
  uint32_t (*ExtractReturnedData)(void *handle, uint8_t* pL6470_DaisyChainSpiRxStruct, uint8_t LengthByte);
} MICROSTEPPING_MOTOR_VTable_t;

/**
  * @brief Stepper Motor Board Driver Structure
  */
typedef struct
{
  void     (*SetParam)(uint8_t, uint8_t, eL6470_RegId_t, uint32_t);
  uint32_t (*GetParam)(uint8_t, uint8_t, eL6470_RegId_t);
  void     (*Run)(uint8_t, uint8_t, eL6470_DirId_t, uint32_t);
  void     (*StepClock)(uint8_t, uint8_t, eL6470_DirId_t);
  void     (*Move)(uint8_t, uint8_t, eL6470_DirId_t, uint32_t);
  void     (*GoTo)(uint8_t, uint8_t L6470_Id, uint32_t AbsPos);
  void     (*GoToDir)(uint8_t, uint8_t, eL6470_DirId_t, uint32_t);
  void     (*GoUntil)(uint8_t, uint8_t, eL6470_ActId_t, eL6470_DirId_t, uint32_t);
  void     (*ReleaseSW)(uint8_t, uint8_t, eL6470_ActId_t, eL6470_DirId_t);
  void     (*GoHome)(uint8_t, uint8_t);
  void     (*GoMark)(uint8_t, uint8_t);
  void     (*ResetPos)(uint8_t, uint8_t);
  void     (*ResetDevice)(uint8_t, uint8_t);
  void     (*SoftStop)(uint8_t, uint8_t);
  void     (*HardStop)(uint8_t, uint8_t);
  void     (*SoftHiZ)(uint8_t, uint8_t);
  void     (*HardHiZ)(uint8_t, uint8_t);
  uint16_t (*GetStatus)(uint8_t, uint8_t);
  uint8_t  (*CheckStatusRegisterFlag)(uint8_t, uint8_t, uint8_t);
  uint8_t* (*PerformPreparedApplicationCommand)(uint8_t);
} MICROSTEPPING_MOTOR_EB_VTable_t;

/**
  * @brief  Stepper Motor Handle Structure
  */
typedef struct __StepperMotorDriver_HandleTypeDef
{
  uint8_t DaisyChainPosition;
  void (*Config)(void*);
  MICROSTEPPING_MOTOR_VTable_t *Command;
} StepperMotorDriverHandle_t;

/**
  * @brief  Stepper Motor Handle Structure  
  */ 
typedef struct __StepperMotorBoard_HandleTypeDef
{
  uint8_t StackedPosition;
  void (*Config)(void*);
  MICROSTEPPING_MOTOR_EB_VTable_t *Command;
  StepperMotorDriverHandle_t *StepperMotorDriverHandle[2];
  uint8_t (*Select)(uint8_t);
} StepperMotorBoardHandle_t;

/**
  * @}
  */    /* StepperMotorExportedTypes */

/**
  * @}
  */    /* MicrosteppingMotorDriver */
  
/**
  * @}
  */    /* Components */

/**
  * @}
  */    /* BSP */

#ifdef __cplusplus
}
#endif

#endif /* __MICROSTEPPINGMOTOR_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
