/**
 ******************************************************************************
 * @file    component_def.h
 * @author  AST
 * @version V1.0.0
 * @date    1 April 2015
 * @brief   Generic header file containing a generic component's definitions
 *          and I/O functions.
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


/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __COMPONENT_H
#define __COMPONENT_H


/* Types ---------------------------------------------------------------------*/

/**
 * @brief  Component's Context structure definition.
 */
typedef struct
{  
    /* Identity. */
    uint8_t who_am_i;

    /* ACTION ----------------------------------------------------------------*/
    /* There should be only a unique identifier for each component, which     */
    /* should be the "who_am_i" parameter, hence this parameter is optional.  */
    /* -----------------------------------------------------------------------*/
    /* Type. */
    uint8_t type;

    /* Configuration. */
    uint8_t address;

    /* Pointer to the Data. */
    void *p_data;

    /* Pointer to the Virtual Table. */
    void *p_vt;

    /* ACTION ----------------------------------------------------------------*/
    /* There should be only a unique virtual table for each component, which  */
    /* should be the "p_vt" parameter, hence this parameter is optional.      */
    /* -----------------------------------------------------------------------*/
    /* Pointer to the Extended Virtual Table. */
    void *p_ext_vt;
} handle_t;

/**
 * @brief  Component's Status enumerator definition.
 */
typedef enum
{
    COMPONENT_OK = 0,
    COMPONENT_ERROR,
    COMPONENT_TIMEOUT,
    COMPONENT_NOT_IMPLEMENTED
} status_t;

#endif /* __COMPONENT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
