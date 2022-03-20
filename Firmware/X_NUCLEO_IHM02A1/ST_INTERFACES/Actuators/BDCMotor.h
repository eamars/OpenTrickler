/**
 ******************************************************************************
 * @file    BDCMotor.h
 * @author  IPC Rennes
 * @version V1.0.0
 * @date    April 6th, 2016
 * @brief   This file contains the abstract class describing the interface of a
 *          Brush DC motor component.
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

#ifndef __BDCMOTOR_CLASS_H
#define __BDCMOTOR_CLASS_H


/* Includes ------------------------------------------------------------------*/

#include <Component.h>


/* Classes  ------------------------------------------------------------------*/

/**
 * An abstract class for BDCMotor components.
 */
class BDCMotor : public Component {
public:

    /**
     * @brief Rotation modes.
     */
    typedef enum {
        BWD = 0, /* Backward. */
        FWD = 1  /* Forward. */
    } direction_t;

    /**
     * @brief  Disabling the specified bridge.
     * @param  bridgeId from 0 for bridge A to 1 for bridge B.
     * @retval None.
     */
    virtual void disable_bridge(unsigned int) = 0;    
     
    /**
     * @brief  Enabling the specified bridge.
     * @param  bridgeId from 0 for bridge A to 1 for bridge B
     * @retval None.
     */
    virtual void enable_bridge(unsigned int) = 0;    
     
    /**
     * @brief  Getting the PWM frequency of the specified bridge;
     * @param  bridgeId from 0 for bridge A to 1 for bridge B.
     * @retval The frequency in Hz of the specified bridge input PWM.
     */
    virtual unsigned int get_bridge_input_pwm_freq(unsigned int) = 0;    
    
    /**
     * @brief  Getting the bridge status.
     * @param  bridgeId from 0 for bridge A to 1 for bridge B.
     * @retval The status.
     */
    virtual unsigned int get_bridge_status(unsigned int) = 0;    
    
    /**
     * @brief  Getting the device State. 
     * @param  motorId from 0 to (MAX_NUMBER_OF_BRUSH_DC_MOTORS - 1). 
     * @retval The device state
     */
    virtual unsigned int get_device_state(unsigned int) = 0;

    /**
     * @brief  Getting the current speed in % of the specified motor.
     * @param  motorId from 0 to (MAX_NUMBER_OF_BRUSH_DC_MOTORS - 1). 
     * @retval The current speed in %.
     */
    virtual unsigned int get_speed(unsigned int) = 0;

    /**
     * @brief  Stopping the motor and disabling the power bridge immediately.
     * @param  motorId from 0 to (MAX_NUMBER_OF_BRUSH_DC_MOTORS - 1). 
     * @retval None.
     */
    virtual void hard_hiz(unsigned int) = 0;
    
    /**
     * @brief  Stopping the motor immediately.
     * @param  motorId from 0 to (MAX_NUMBER_OF_BRUSH_DC_MOTORS - 1). 
     * @retval None.
     */
    virtual void hard_stop(unsigned int) = 0;

    /**
     * @brief  Running the motor.
     * @param  motorId from 0 to (MAX_NUMBER_OF_BRUSH_DC_MOTORS - 1).
     * @param  direction The direction of rotation.
     * @retval None.
     */
    virtual void run(unsigned int, direction_t) = 0;
    
    /**
     * @brief  Setting the PWM frequency of the specified bridge.
     * @param  bridgeId from 0 for bridge A to 1 for bridge B.
     * @param  frequency of the PWM in Hz
     * @retval None.
     */
    virtual void set_bridge_input_pwm_freq(unsigned int, unsigned int) = 0;    
        
    /**
     * @brief  Setting the dual bridge configuration mode.
     * @param  configuration. The bridge configuration.
     * @retval None.
     */
    virtual void set_dual_full_bridge_config(unsigned int) = 0;

    /**
     * @brief  Setting the speed in %.
     * @param  motorId from 0 to (MAX_NUMBER_OF_BRUSH_DC_MOTORS - 1).
     * @param  speed The new speed in %.
     * @retval "true" in case of success, "false" otherwise.
     */
    virtual bool set_speed(unsigned int, unsigned int) = 0;

    /**
     * @brief Destructor.
     */
    virtual ~BDCMotor() {};
};

#endif /* __BDCMOTOR_CLASS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/ 
