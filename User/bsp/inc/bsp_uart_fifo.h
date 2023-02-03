/*
*********************************************************************************************************
*
* Module name: serial port interrupt + FIFO driver module 
* File name: bsp_uart_fifo.h 
* Version : V1.0 
* Description: Header file
*
*********************************************************************************************************
*/

#ifndef _BSP_USART_FIFO_H_
#define _BSP_USART_FIFO_H_
#include <stdint.h>
#include "bsp.h"

/*	
	串口分配：
	【串口1】 RS232	--- 打印调试口，上位机
		PA9/USART1_TX
		PA10/USART1_RX

	【串口2】GPS 模块  --- 卫星定位，授时
		PA2/USART2_TX
		PA3/USART2_RX 

	【串口3】WiFi 模块
		PB10/USART3_TX
		PB11/USART3_RX

	【串口4】 RS485 通信
		PC10/UART4_TX
		PC11/UART4_RX
		PD0/BOOT1/RS485_TX_EN
		
	【串口5】 RS232 检测器输入
		PC12/UART5_TX
		PD2/UART5_RX
*/

#define	UART1_FIFO_EN	1
#define	UART2_FIFO_EN	1
#define	UART3_FIFO_EN	1
#define	UART4_FIFO_EN	1
#define	UART5_FIFO_EN	1

/* RS485 chip sends enable GPIO, PB2 */
#define RCC_RS485_DIR 	 RCC_APB2Periph_GPIOD
#define RS485_DIR_PORT   GPIOD
#define RS485_DIR_PIN	 GPIO_Pin_0

//High level receive, low level transmit
#define RS485_RX_EN()	RS485_DIR_PORT->BSRR = RS485_DIR_PIN
#define RS485_TX_EN()	RS485_DIR_PORT->BRR = RS485_DIR_PIN


/* Serial device structure */
typedef struct
{
	USART_TypeDef *uart;		/* STM32 internal serial device pointe */
	uint8_t *pTxBuf;			/* send buffer */
	uint8_t *pRxBuf;			/* receive buffer */
	uint16_t usTxBufSize;		/* send buffer size */
	uint16_t usRxBufSize;		/* receive buffer size */
	__IO uint16_t usTxWrite;	/* send buffer write pointer */
	__IO uint16_t usTxRead;		/* send buffer read pointer */
	__IO uint16_t usTxCount;	/* The number of data waiting to be sent */

	__IO uint16_t usRxWrite;	/* receive buffer write pointer */
	__IO uint16_t usRxRead;		/* Receive buffer read pointer */
	__IO uint16_t usRxCount;	/* The number of new data that has not been read */

	void (*SendBefor)(void); 	/* Callback function pointer before starting to send (mainly used for switching RS485 to send mode) */
	void (*SendOver)(void); 	/* The callback function pointer after sending (mainly used for RS485 to switch the sending mode to receiving mode) */
	void (*ReciveNew)(uint16_t _byte);	/* The callback function pointer of the data received by the serial port */
}UART_T;

/* Define serial port baud rate and FIFO buffer size, divided into send buffer and receive buffer, support full duplex */
#if UART1_FIFO_EN == 1
	#define UART1_BAUD			115200
	#define UART1_TX_BUF_SIZE	1*1024
	#define UART1_RX_BUF_SIZE	1*1024
	extern UART_T g_tUart1;
#endif

#if UART2_FIFO_EN == 1
	#define UART2_BAUD			9600
	#define UART2_TX_BUF_SIZE	128
	#define UART2_RX_BUF_SIZE	1024
    extern UART_T Uart2Gps;
#endif

#if UART3_FIFO_EN == 1
	#define UART3_BAUD			115200
	#define UART3_TX_BUF_SIZE	1024
	#define UART3_RX_BUF_SIZE	1024
    extern UART_T g_tUart3;
#endif

#if UART4_FIFO_EN == 1
	#define UART4_BAUD			9600
	#define UART4_TX_BUF_SIZE	1024
	#define UART4_RX_BUF_SIZE	1024
    extern UART_T g_tUart4;
#endif

#if UART5_FIFO_EN == 1
	#define UART5_BAUD			115200
	#define UART5_TX_BUF_SIZE	1024
	#define UART5_RX_BUF_SIZE	1024
    extern UART_T g_tUart5;
#endif


void bsp_InitUart(void);

void UartSendBuf(UART_T *uart_t, uint8_t *_ucaBuf, uint16_t _usLen);
void UartSendChar(UART_T *uart_t, uint8_t _ucByte);

uint16_t UartGetCount(UART_T *uart_t);

void UartClearTxFifo(UART_T *uart_t);
void UartClearRxFifo(UART_T *uart_t);

void RS485_SendBuf(uint8_t *_ucaBuf, uint16_t _usLen);
void RS485_SendStr(char *_pBuf);

void bsp_Set485Baud(uint32_t _baud);

void bsp_SetUart1Baud(uint32_t _baud);
void bsp_SetUart2Baud(uint32_t _baud);

void Gps_ReciveNew(uint16_t RxCount);
void serial_ex(uint16_t RxSerial);				/*FUNCION AGRAGADA*/

void printf_fifo_dec(uint8_t* tx, uint8_t len);
void printf_fifo_hex(uint8_t* tx, uint8_t len);


uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte);


#endif
