/*
*********************************************************************************************************
*
* Nombre del módulo: interrupción de puerto serie + módulo de controlador FIFO
* Nombre del archivo: bsp_uart_fifo.h
* Version : V1.0 
* Descripción: archivo de cabecera
*
*********************************************************************************************************
*/

#ifndef _BSP_USART_FIFO_H_
#define _BSP_USART_FIFO_H_
#include <stdint.h>
#include "bsp.h"

/*	
	Asignación de puerto serie: [Puerto serie 1] 
	RS232 --- Imprimir puerto de depuración, computadora host
		PA9/USART1_TX
		PA10/USART1_RX

	Puerto serie 2】Módulo GPS --- Posicionamiento por satélite, temporización
		PA2/USART2_TX
		PA3/USART2_RX 

	【Puerto serie 3】Módulo WiFi
		PB10/USART3_TX
		PB11/USART3_RX

	[Puerto serie 4] Comunicación RS485
		PC10/UART4_TX
		PC11/UART4_RX
		PD0/BOOT1/RS485_TX_EN
		
	[Puerto serie 5] Entrada de detector RS232
		PC12/UART5_TX
		PD2/UART5_RX
*/

#define	UART1_FIFO_EN	1
#define	UART2_FIFO_EN	1
#define	UART3_FIFO_EN	1
#define	UART4_FIFO_EN	1
#define	UART5_FIFO_EN	1

/* Los envíos del chip RS485 habilitan GPIO, PB2 */
#define RCC_RS485_DIR 	 RCC_APB2Periph_GPIOD
#define RS485_DIR_PORT   GPIOD
#define RS485_DIR_PIN	 GPIO_Pin_0

//Recepción de alto nivel, transmisión de bajo nivel
#define RS485_RX_EN()	RS485_DIR_PORT->BSRR = RS485_DIR_PIN
#define RS485_TX_EN()	RS485_DIR_PORT->BRR = RS485_DIR_PIN


/* Estructura del dispositivo serie */
typedef struct
{
	USART_TypeDef *uart;		/* Punto de dispositivo serial interno STM32 */
	uint8_t *pTxBuf;			/* enviar búfer */
	uint8_t *pRxBuf;			/* búfer de recepción */
	uint16_t usTxBufSize;		/* tamaño del búfer de envío */
	uint16_t usRxBufSize;		/* tamaño del búfer de recepción */
	__IO uint16_t usTxWrite;	/* enviar puntero de escritura de búfer */
	__IO uint16_t usTxRead;		/* enviar puntero de lectura de búfer */
	__IO uint16_t usTxCount;	/* El número de datos que esperan ser enviados */

	__IO uint16_t usRxWrite;	/* recibir el puntero de escritura del búfer */
	__IO uint16_t usRxRead;		/* Recibe el puntero de lectura del búfer */
	__IO uint16_t usRxCount;	/* El número de datos nuevos que no han sido leídos */

	void (*SendBefor)(void); 	/* Puntero de función de devolución de llamada antes de comenzar a enviar (utilizado principalmente para cambiar RS485 al modo de envío) */
	void (*SendOver)(void); 	/* El puntero de la función de devolución de llamada después del envío (utilizado principalmente para RS485 para cambiar el modo de envío al modo de recepción) */
	void (*ReciveNew)(uint16_t _byte);	/* El puntero de la función de devolución de llamada de los datos recibidos por el puerto serie */
}UART_T;

/* Definir la tasa de baudios del puerto serie y el tamaño del búfer FIFO, dividido en búfer de envío y búfer de recepción, compatible con dúplex completo */
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
