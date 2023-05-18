#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include "stm32f4xx_hal.h"
#include "common_porting.h"
#include "main.h"

extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;

uint8_t GTXBuffer[512], GRXBuffer[2048];

volatile uint8_t int1_flag = 0;
volatile uint8_t int2_flag = 0;

//void Enable_MCU_INT1_Pin(void)
//{
//	GPIO_InitTypeDef GPIO_InitStruct = {0};
//
//	__HAL_RCC_GPIOA_CLK_ENABLE();
//
//	/*Configure GPIO pin : INT1_Pin */
//	GPIO_InitStruct.Pin = INT1_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	HAL_GPIO_Init(INT1_GPIO_Port, &GPIO_InitStruct);
//
//	/* EXTI interrupt init*/
//	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
//	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
//}
//
//void Disable_MCU_INT1_Pin(void)
//{
//	GPIO_InitTypeDef GPIO_InitStruct = {0};
//
//	__HAL_RCC_GPIOA_CLK_ENABLE();
//
//	/*Configure GPIO pin : INT1_Pin */
//	GPIO_InitStruct.Pin = INT1_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	HAL_GPIO_Init(INT1_GPIO_Port, &GPIO_InitStruct);
//
//	/* EXTI interrupt init*/
//	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
//	HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
//}
//
//void Enable_MCU_INT2_Pin(void)
//{
//	GPIO_InitTypeDef GPIO_InitStruct = {0};
//
//	__HAL_RCC_GPIOA_CLK_ENABLE();
//
//	/*Configure GPIO pin : INT2_Pin */
//	GPIO_InitStruct.Pin = INT2_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	HAL_GPIO_Init(INT2_GPIO_Port, &GPIO_InitStruct);
//
//	/* EXTI interrupt init*/
//	HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
//  	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
//}
//
//void Disable_MCU_INT2_Pin(void)
//{
//	GPIO_InitTypeDef GPIO_InitStruct = {0};
//
//	__HAL_RCC_GPIOA_CLK_ENABLE();
//
//	/*Configure GPIO pin : INT2_Pin */
//	GPIO_InitStruct.Pin = INT2_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	HAL_GPIO_Init(INT2_GPIO_Port, &GPIO_InitStruct);
//
//	/* EXTI interrupt init*/
//	HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
//  	HAL_NVIC_DisableIRQ(EXTI3_IRQn);
//
//}

#if defined(FIFO_WM_INT)
extern volatile uint16_t bma456_fifo_ready;
#endif

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_10) //INT1
	{
		printf("INT1 Triggered\r\n");
		int1_flag = 1;
#if defined(FIFO_WM_INT)
		bma456_fifo_ready = 1;
		#endif
	} else if (GPIO_Pin == GPIO_PIN_3) //INT2
	{
		//printf("INT2 Triggered\r\n");
		int2_flag = 1;
#if defined(FIFO_WM_INT)
		#if defined(USE_BMA456)
		//bma456_fifo_ready = 1;
		#endif
	#endif
	} else if (GPIO_Pin == GPIO_PIN_5) //DRDY_BMM150
	{
		printf("DRDY_BMM150 Triggered\r\n");
	}
}

void DelayUs(uint32_t Delay) {
	uint32_t i;

	while (Delay--) {
		for (i = 0; i < 84; i++) {
			;
		}
	}
}

void bmp3_delay_us(uint32_t period, void *intf_ptr) {
	uint32_t i;

	while (period--) {
		for (i = 0; i < 84; i++) {
			;
		}
	}
}

//void UART_Printf(uint8_t *buff, uint16_t size) {
//	//HAL_UART_Transmit_DMA(&huart2, buff, size);
//	HAL_UART_Transmit(&UART_HANDLE, buff, size, BUS_TIMEOUT);
//}

char chBuffer[256];
//#if 1
//void printf(char *format, ...) {
//#if defined(DEBUG_EN)
//	va_list ap;
//	//char timestamp[16];
//	va_start(ap, format);
//	vsnprintf(chBuffer, sizeof(chBuffer), format, ap);
//	//sprintf(timestamp, "[%d]", xTaskGetTickCount()); //xTaskGetTickCountFromISR()
//	//Printf((uint8_t *)timestamp, strlen(timestamp));
//	UART_Printf((uint8_t*) chBuffer, strlen(chBuffer));
//	va_end(ap);
//#endif
//}

//#else
//void printf(char *format, ...)
//{
//#if defined(DEBUG_EN)
//    va_list ap;
//    //char timestamp[16];
//    va_start(ap, format);
//    vsnprintf(GTXBuffer, sizeof(GTXBuffer), format, ap);
//    //sprintf(timestamp, "[%d]", xTaskGetTickCount()); //xTaskGetTickCountFromISR()
//    //Printf((uint8_t *)timestamp, strlen(timestamp));
//    UART_Printf((uint8_t *)GTXBuffer,strlen(GTXBuffer));
//    va_end(ap);
//#endif
//}
//#endif

#if defined(USE_BOSCH_SENSOR_API)
/*******************************************************************************
 * Function Name  : I2C_Read
 * Description    : Write data into slave device.
 * Input          : I2C1 or I2C2, slave_address7, subaddress, Write Number
 * Output         : None
 * Return         : number of bytes transmitted
 *******************************************************************************/
int8_t SensorAPI_I2Cx_Read(uint8_t subaddress, uint8_t *pBuffer,
		uint16_t ReadNumbr, void *intf_ptr) {
	uint8_t dev_addr = *(uint8_t*) intf_ptr;
	uint16_t DevAddress = dev_addr << 1;

	// send register address
	HAL_I2C_Master_Transmit(&I2C_HANDLE, DevAddress, &subaddress, 1,
			BUS_TIMEOUT);
	HAL_I2C_Master_Receive(&I2C_HANDLE, DevAddress, pBuffer, ReadNumbr,
			BUS_TIMEOUT);
	return 0;
}

int8_t SensorAPI_I2Cx_Write(uint8_t subaddress, uint8_t *pBuffer,
		uint16_t WriteNumbr, void *intf_ptr) {
	uint8_t dev_addr = *(uint8_t*) intf_ptr;
	uint16_t DevAddress = dev_addr << 1;

	GTXBuffer[0] = subaddress;
	memcpy(&GTXBuffer[1], pBuffer, WriteNumbr);

	// send register address
	HAL_I2C_Master_Transmit(&I2C_HANDLE, DevAddress, GTXBuffer, WriteNumbr + 1,
			BUS_TIMEOUT);
	return 0;
}

/*******************************************************************************
 * Function Name  : SPI_Read
 * Description    : Write data into slave device.
 * Input          : I2C1 or I2C2, slave_address7, subaddress, Write Number
 * Output         : None
 * Return         : number of bytes transmitted
 *******************************************************************************/
int8_t SensorAPI_SPIx_Read(uint8_t subaddress, uint8_t *pBuffer,
		uint16_t ReadNumbr, void *intf_ptr) {
	GTXBuffer[0] = subaddress | 0x80;

	HAL_GPIO_WritePin(BARO_SPI_CS_GPIO_Port, BARO_SPI_CS_Pin, GPIO_PIN_RESET); // NSS low

	//HAL_SPI_TransmitReceive(&hspi2, pTxData, pRxData, ReadNumbr+1, BUS_TIMEOUT); // timeout 1000msec;
	HAL_SPI_TransmitReceive(&SPI_HANDLE, GTXBuffer, GRXBuffer, ReadNumbr + 1,
			BUS_TIMEOUT); // timeout 1000msec;
	while (SPI_HANDLE.State == HAL_SPI_STATE_BUSY)
		;  // wait for xmission complete

	HAL_GPIO_WritePin(BARO_SPI_CS_GPIO_Port, BARO_SPI_CS_Pin, GPIO_PIN_SET); // NSS high
	memcpy(pBuffer, GRXBuffer + 1, ReadNumbr);

	return 0;
}

int8_t SensorAPI_SPIx_Write(uint8_t subaddress, uint8_t *pBuffer,
		uint16_t WriteNumbr, void *intf_ptr) {
	GTXBuffer[0] = subaddress & 0x7F;
	memcpy(&GTXBuffer[1], pBuffer, WriteNumbr);

	HAL_GPIO_WritePin(BARO_SPI_CS_GPIO_Port, BARO_SPI_CS_Pin, GPIO_PIN_RESET); // NSS low

	//HAL_SPI_TransmitReceive(&hspi2, pTxData, pRxData, WriteNumbr+1, BUS_TIMEOUT); // send register address + write data
	HAL_SPI_Transmit(&SPI_HANDLE, GTXBuffer, WriteNumbr + 1, BUS_TIMEOUT); // send register address + write data
	while (SPI_HANDLE.State == HAL_SPI_STATE_BUSY)
		;  // wait for xmission complete

	HAL_GPIO_WritePin(BARO_SPI_CS_GPIO_Port, BARO_SPI_CS_Pin, GPIO_PIN_SET); // NSS high

	return 0;
}

#endif

