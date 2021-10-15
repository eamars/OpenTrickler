/**
 ******************************************************************************
 * @file    DbgMCU.h
 * @author  AST / EST
 * @version V0.0.1
 * @date    30-March-2015
 * @brief   Header file for enabling debugging in sleep modes for STM32 MCUs
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

/* Define to prevent from recursive inclusion --------------------------------*/
#ifndef __DBG_MCU_H
#define __DBG_MCU_H

/* Includes ------------------------------------------------------------------*/

/* Classes -------------------------------------------------------------------*/
/** Helper class DbgMCU providing a default constructor which enables debugging
 *  on STM32 MCUs while using sleep modes.
 */
class DbgMCU
{
 public:
	/** Create a DbgMCU dummy object */
        DbgMCU(void) {
		/* the following code is NOT portable */
                volatile uint32_t *dbgmcu_creg = (uint32_t*)0xE0042004;
                uint32_t tmp = *dbgmcu_creg;
		
		tmp &= ~(0xE7);
		tmp |= 0x27; // Set asynchronous communication via DBGMCU_CR (for ITM/printf)
		// tmp |= 0xE7; // Set 4-pin tracing via DBGMCU_CR (for ETM)
                *dbgmcu_creg = tmp;
	}
};

#endif /* __DBG_MCU_H */
