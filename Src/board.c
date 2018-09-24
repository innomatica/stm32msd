#include <stdarg.h>
#include <stdio.h>
#include "board.h"
//#include "stm32l4xx_ll_tim.h"
#include "stm32l4xx_hal_tim.h"

extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart2;
/** Formatted string output to USB_CDC
 *
 *	\param printf() like parameters
#include "usbd_cdc.h"
extern USBD_HandleTypeDef hUsbDeviceFS;
void USB_Printf(const char* format,...)
{
	char buffer[256];
	int length;
	va_list args;
	va_start(args, format);

	length = vsprintf(buffer, format, args);
	if(length)
		USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*)buffer, length);

	va_end(args);
}
 */


/** System clock 48MHz
 *	TIM6 Prescaler: 0
 *
 */
#define CNTR08000		(5999)
#define CNTR16000		(2999)
#define CNTR22100		(2171)
#define CNTR24000		(1999)
#define CNTR48000		(999)

void AudioOut_Init(uint32_t freq, uint32_t vol)
{
	// frequency
	if(freq == 8000U)
	{
		__HAL_TIM_SetAutoreload(&htim6, CNTR08000);
	}
	else if(freq == 16000U)
	{
		__HAL_TIM_SetAutoreload(&htim6, CNTR16000);
	}
	if(freq == 22100U)
	{
		__HAL_TIM_SetAutoreload(&htim6, CNTR22100);
	}
	else if(freq == 24000U)
	{
		__HAL_TIM_SetAutoreload(&htim6, CNTR24000);
	}
	else if(freq == 48000U)
	{
		__HAL_TIM_SetAutoreload(&htim6, CNTR48000);
	}

	HAL_TIM_Base_Start_IT(&htim6);
	UART_Printf("\r\nAudioOut_Init");

	// volume
}

void AudioOut_DeInit(void)
{
	HAL_TIM_Base_Stop_IT(&htim6);
	DbgPrintf("\r\nAudioOut_DeInit");
}

void AudioOut_Start(uint8_t *buff, uint32_t size)
{
	__HAL_TIM_DISABLE_IT(&htim6, TIM_IT_UPDATE);
	//UART_Printf("Audio_Start:%d,%d,%d\n\r",pointer,full_size,half_size);
	__HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);
}
void AudioOut_Play(uint8_t *buff, uint32_t size)
{
	__HAL_TIM_DISABLE_IT(&htim6, TIM_IT_UPDATE);
	//UART_Printf("Audio_Play:%d,%d,%d\n\r",pointer,full_size,half_size);
	__HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);
}
void AudioOut_Volume(uint8_t vol)
{
	UNUSED(vol);
}
void AudioOut_Mute(uint8_t cmd)
{
	UNUSED(cmd);
}
void AudioOut_Pause(void)
{
	HAL_TIM_Base_Stop_IT(&htim6);
}


/** Formatted string ouput to UART
 *
 *	\param printf() like parameters
 */
void UART_Printf(const char* format,...)
{
	char buffer[256];
	int length;
	va_list args;
	va_start(args, format);

	length = vsprintf(buffer, format, args);
	if(length)
		HAL_UART_Transmit(&huart2, (uint8_t*)buffer, length, 1000);

	va_end(args);
}


/** Microsecond delay
 *
 * \warning This is subject to error since the delay relies on execution of
 *		nop().
 */
void USecDelay(unsigned usec)
{
	while(usec-- > 0)
	{
		// approximately 1 usec delay in 32MHz clock
		asm(
		    "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
		    "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
		    "nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\t"
		);
	}
}

uint8_t PushButton_Read(void)
{
	return BTN_READ;
}

void SerialComm_Init(void)
{
	// clear USART interrupt
	HAL_NVIC_ClearPendingIRQ(USART1_IRQn);
	// enable RX interrupt
	USART1_ENABLE_RX_IT;
}

void SerialComm_SendByteArray(uint8_t *buffer, int size)
{
	// note that HAL_UART_TransmitIT no longer works
	HAL_UART_Transmit(&huart2, buffer, size, 1000);
}

#if UNIT_TEST

#endif
