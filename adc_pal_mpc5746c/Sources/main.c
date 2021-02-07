/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * THIS SOFTWARE IS PROVIDED BY NXP "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL NXP OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
/* ###################################################################
 **     Filename    : main.c
 **     Project     : adc_pal_mpc5746c
 **     Processor   : MPC5746C_256
 **     Version     : Driver 01.00
 **     Compiler    : GNU C Compiler
 **     Date/Time   : 2017-03-14, 14:08, # CodeGen: 1
 **     Abstract    :
 **         Main module.
 **         This module contains user's application code.
 **     Settings    :
 **     Contents    :
 **         No public methods
 **
 ** ###################################################################*/
/*!
 ** @file main.c
 ** @version 01.00
 ** @brief
 **         Main module.
 **         This module contains user's application code.
 */
/*!
 **  @addtogroup main_module main module documentation
 **  @{
 */
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "pin_mux.h"
#include "osif.h"

volatile int exit_code = 0;
/* User includes (#include below this line is not maintained by Processor Expert) */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


#define NUM_CONV_GROUP_ITERATIONS       10UL
#define DELAY_BETWEEN_SW_TRIG_GROUPS    1500UL /* [milliseconds] */


/* This example is setup to work with motherboard */
#define LED_PORT PTG
#define LED 2        	        /* pin PG[2] - LED1 (DS2) on Motherboard */
#define ADC_CHAN_NUM      (9u)	/* ADC1_P[0] corresponding to PB[4] */
#define ADC_CHAN_INSTANCE (1u)	/* ADC1_P[0] corresponding to PB[4] */

#define PIT_CHANNEL		  (3u)

#define welcomeStr "\r\n  This is an example for ADC PAL: it prints the conversion results from groups of conversions.\
		\r\n    The first result in each group is measured on ADC1 Input 9 - connected to potentiometer.\
		\r\n    Other results in the group are not relevant, because are measured on unconnected ADC channels - added to group only for demonstration purpose. \r\n"
#define part1Str  "\r\n  *** Part 1: software triggered conversion group \r\n"
#define part2Str  "\r\n  *** Part 2: hardware triggered conversion group \r\n"
#define headerStr  "\r\n  ADC potentiometer result: "
#define exitStr    "\r\n\r\n  ADC PAL example execution finished successfully.\r\n"

/* Flag used to store if an ADC PAL conversion group has finished executing */
volatile bool groupConvDone = false;
/* Flag used to store the offset of the most recent result in the result buffer */
volatile uint32_t resultLastOffset = 0;

extern uint32_t FmcBspGetSysClk(void);


void adc_pal1_callback00(const adc_callback_info_t * const callbackInfo, void * userData)
{
	(void) userData;

	groupConvDone = true;
	resultLastOffset = callbackInfo->resultBufferTail;
}

void adc_pal1_callback02(const adc_callback_info_t * const callbackInfo, void * userData)
{
	(void) userData;

	groupConvDone = true;
	resultLastOffset = callbackInfo->resultBufferTail;
}


/*
* Description: Send a string to user UART console
*/
static void print(const char *sourceStr);

/*
* Description: Convert a float to null terminated char array
*/
static void floatToStr (const float *srcValue, char *destStr, uint8_t maxLen);


/*!
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
 */

void EOL_start()
{
typedef void(*JumpToPtr)(void);
	JumpToPtr pJumpTo;
	if(*(uint32_t*)0x01000000==0xA55AA55A)
	{
		pJumpTo = (JumpToPtr)0x01000400;
		pJumpTo();
	}
}
int main(void)
{
	/* Write your local variable definition here */
	status_t status;
	uint8_t selectedGroupIndex;
	uint16_t resultStartOffset;
	uint16_t adcMax;
	float resVolts;
	uint8_t idx;
	volatile uint32_t sysClk = 0;

	EOL_start();

	/* Buffer used to store processed data for serial communication */
	char msg[255] =
	{ 0, };

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
#ifdef PEX_RTOS_INIT
	PEX_RTOS_INIT();                   /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
#endif
	/*** End of Processor Expert internal initialization.                    ***/

	/* Initialize and configure clocks
	 *  -   see clock manager component for details
	 */

	sysClk=FmcBspGetSysClk();
	CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
			g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
	CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);
	sysClk=FmcBspGetSysClk();

	/* Initialize pins
	 *  -   See PinSettings component for more info
	 */

  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;) {
    if(exit_code != 0) {
      break;
    }
  }
  return exit_code;
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
 ** @}
 */


/*FUNCTION**********************************************************************
*
* Description   : Send a string to user UART console
*
* param sourceStr: pointer to the input array of characters
* return:          None
* END**************************************************************************/
static void print(const char *sourceStr)
{
	/* Send data via UART */
	LINFLEXD_UART_DRV_SendDataBlocking(INST_LINFLEXD_UART1,(uint8_t *) sourceStr, strlen(sourceStr), 1000U);
}


/*FUNCTION**********************************************************************
*
* Description   : Convert a float to null terminated char array
*
* param srcValue:  pointer to the source float value
* param destStr:   pointer to the destination string
* param maxLen:    maximum length of the string
* return:          None
* END**************************************************************************/
static void floatToStr (const float *srcValue, char *destStr, uint8_t maxLen)
{
	uint8_t i, lessThanOne = 0;
	float tempVal = (*srcValue);
	uint8_t currentVal;

	if (tempVal < 0)
	{
		tempVal *= -1;
		*destStr = '-';
		destStr++;
	}
	for (i = 0; i < maxLen; i++)
	{
		currentVal = (uint8_t) (tempVal);
		*destStr = currentVal + 48;
		destStr++;
		tempVal -= currentVal;
		if ((tempVal < 1) && (lessThanOne == 0))
		{
			*destStr = '.';
			destStr++;
			lessThanOne = 1;
		}
		tempVal *= 10;
	}
	*destStr = 0;
}


/*
 ** ###################################################################
 **
 **     This file was created by Processor Expert 10.1 [05.21]
 **     for the NXP C55 series of microcontrollers.
 **
 ** ###################################################################
 */
