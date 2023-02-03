/*
*********************************************************************************************************
* Module name: serial port interrupt + FIFO driver module 
* File name: bsp_uart_fifo.c 
* Version : V1.0 
* Description: Use serial port interrupt + FIFO mode to realize 
simultaneous access to multiple serial ports
*********************************************************************************************************
*/

#include "bsp.h"
/*Define cada estructura de variables puerto serial */

/* Define each serial port structure variable */ //RS232
#if UART1_FIFO_EN == 1
  UART_T g_tUart1;
	static uint8_t g_TxBuf1[UART1_TX_BUF_SIZE];		/* Envio de buffer */
	static uint8_t g_RxBuf1[UART1_RX_BUF_SIZE];		/* Recepcion de buffer */
#endif

#if UART2_FIFO_EN == 1    // GPS
  UART_T Uart2Gps;
	static uint8_t g_TxBuf2[UART2_TX_BUF_SIZE];		/* ���ͻ����� */
	static uint8_t g_RxBuf2[UART2_RX_BUF_SIZE];		/* ���ջ����� */
#endif

#if UART3_FIFO_EN == 1 	// WIFI
  UART_T g_tUart3;
	static uint8_t g_TxBuf3[UART3_TX_BUF_SIZE];		/* ���ͻ����� */
	static uint8_t g_RxBuf3[UART3_RX_BUF_SIZE];		/* ���ջ����� */
#endif

#if UART4_FIFO_EN == 1  //RS485
  UART_T g_tUart4;
	static uint8_t g_TxBuf4[UART4_TX_BUF_SIZE];		/* ���ͻ����� */
	static uint8_t g_RxBuf4[UART4_RX_BUF_SIZE];		/* ���ջ����� */
#endif

#if UART5_FIFO_EN == 1	// BORNERA DE SALIDA
  UART_T g_tUart5;
	static uint8_t g_TxBuf5[UART5_TX_BUF_SIZE];		/* ���ͻ����� */
	static uint8_t g_RxBuf5[UART5_RX_BUF_SIZE];		/* ���ջ����� */
#endif


static void UartVarInit(void);
static void InitHardUart(void);

void UartSendBuf(UART_T *uart_t, uint8_t *_ucaBuf, uint16_t _usLen);
uint8_t UartGetChar(UART_T *_pUart, uint8_t *_pByte);
static void UartIRQ(UART_T *_pUart);
static void ConfigUartNVIC(void);

void RS485_InitTXE(void);

/*
*********************************************************************************************************
* Function name: bsp_InitUart 
* Function description: Initialize the serial port hardware and assign initial values ??to global variables. 
* Formal parameters: none 
* Return value: None
*********************************************************************************************************
*/
void bsp_InitUart(void)
{
	UartVarInit();		/* Global variables must be initialized before configuring the hardware */
	InitHardUart();		/* Configure the hardware parameters of the serial port (baud rate, etc.) */
	RS485_InitTXE();	/* Configure the sending enable hardware of the RS485 chip, configured as a push-pull output */
	ConfigUartNVIC();	/* Configure serial port interrupt*/
}

/*
*********************************************************************************************************
* Function name: UartSendBuf 
* Function description: Fill data into UART send buffer, and start sending interrupt. After the interrupt processing function is sent, 
the sending interrupt is automatically turned off 
* Formal parameters: none 
* Return value: None
*********************************************************************************************************
*/
void UartSendBuf(UART_T *uart_t, uint8_t *_ucaBuf, uint16_t _usLen) 
{
	uint16_t i;
    
	if (uart_t->SendBefor != 0)
	{
		uart_t->SendBefor();		/* If it is RS485 communication, you can set RS485 to send mode in this function */
	}
    
	for (i = 0; i < _usLen; i++)
	{
		while(1)
		{
			__IO uint16_t usCount;

			DISABLE_INT();
			usCount = uart_t->usTxCount;
			ENABLE_INT();

			if (usCount < uart_t->usTxBufSize)
			{
				break;
			}
		}

		/* Fill the send buffer with new data */
		uart_t->pTxBuf[uart_t->usTxWrite] = _ucaBuf[i];

		DISABLE_INT();
		if(++uart_t->usTxWrite >= uart_t->usTxBufSize)
		{
			uart_t->usTxWrite = 0;
		}
		uart_t->usTxCount++;
		ENABLE_INT();
	}

	USART_ITConfig(uart_t->uart, USART_IT_TXE, ENABLE); //Enable send interrupt
}

void UartSendChar(UART_T *uart_t, uint8_t _ucByte)
{
	UartSendBuf(uart_t, &_ucByte, 1);
}

/*
*********************************************************************************************************
* Function name: UartGetChar 
* Function description: Read 1 byte of data from the serial port receiving buffer (for the main program call) 
* Formal parameter: _pUart : serial device 
* _pByte: pointer to store read data 
* Return value: 0 means no data, 1 means read data
*********************************************************************************************************
*/
uint8_t UartGetChar(UART_T *uart_t, uint8_t *_pByte)
{
	uint16_t usCount;

	/* usRxWrite The variable is rewritten in the interrupt function. 
	When the main program reads the variable, it must protect the critical area. */
	DISABLE_INT();
	usCount = uart_t->usRxCount;
	ENABLE_INT();

	/* Returns 0 if the read and write indices are the same */
	if (usCount == 0)	/* no more data */
	{
		return 0;
	}
	else
	{
		*_pByte = uart_t->pRxBuf[uart_t->usRxRead];		/* Take 1 data from the serial port receiving FIFO */
		/* Rewrite FIFO read index */
		DISABLE_INT();
		if (++uart_t->usRxRead >= uart_t->usRxBufSize)//buffer zone read to end,
		{
			uart_t->usRxRead = 0;
		}
		uart_t->usRxCount--;
		ENABLE_INT();
		return 1;
	}
}

uint16_t UartGetCount(UART_T *uart_t)
{
	uint16_t usCount;
	DISABLE_INT();
	usCount = uart_t->usRxCount;
	ENABLE_INT();
	return usCount;
}

void UartClearTxFifo(UART_T *uart_t)
{
	uart_t->usTxWrite = 0;
	uart_t->usTxRead = 0;
	uart_t->usTxCount = 0;
}

void UartClearRxFifo(UART_T *uart_t)
{
	uart_t->usRxWrite = 0;
	uart_t->usRxRead = 0;
	uart_t->usRxCount = 0;
}

/*
*********************************************************************************************************
* Function name: bsp_SetUart1Baud 
* Function description: Modify UART1 baud rate 
* Formal parameters: none 
* Return value: None
*********************************************************************************************************
*/
void bsp_SetUart1Baud(uint32_t _baud)
{
	USART_InitTypeDef USART_InitStructure;

	/* Step 2: Configure serial port hardware parameters */
	USART_InitStructure.USART_BaudRate = _baud;	/* tasa de baudios */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
}

/*
*********************************************************************************************************
* Function name: bsp_SetUart2Baud 
* Function description: Modify UART2 baud rate 
* Formal parameters: None 
* Return value: None
*********************************************************************************************************
*/
void bsp_SetUart2Baud(uint32_t _baud)
{
	USART_InitTypeDef USART_InitStructure;

	/* Step 2: Configure serial port hardware parameters*/
	USART_InitStructure.USART_BaudRate = _baud;	/* tasa de baudios */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
}

/*
*********************************************************************************************************
* Function name: RS485_InitTXE 
* Function description: Configure the RS485 transmission enable port line TXE 
* Formal parameters: None 
* Return value: None
*********************************************************************************************************
*/
void RS485_InitTXE(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_RS485_DIR, ENABLE);	/* Turn on the GPIO clock */

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	/* Push-pull output mode */
	GPIO_InitStructure.GPIO_Pin = RS485_DIR_PIN;
	GPIO_Init(RS485_DIR_PORT, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
* Function name: bsp_Set485Baud 
* Function description: modify UART4 baud rate 
* Formal parameters: None 
* Return value: None
*********************************************************************************************************
*/
void bsp_Set485Baud(uint32_t _baud)
{
	USART_InitTypeDef USART_InitStructure;

	/* Step 2: Configure serial port hardware parameters */
	USART_InitStructure.USART_BaudRate = _baud;	/* tasa de baudios */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);
}

void RS485_SendBefor(void)
{
	RS485_TX_EN();	/* Switch the RS485 transceiver chip to send mode */
}

void RS485_SendOver(void)
{
	RS485_RX_EN();	/* Switch the RS485 transceiver chip to receive mode */
}

void RS485_SendBuf(uint8_t *_ucaBuf, uint16_t _usLen)
{
    UartSendBuf(&g_tUart4, _ucaBuf, _usLen);
}

void RS485_SendStr(char *_pBuf)
{
	RS485_SendBuf((uint8_t *)_pBuf, strlen(_pBuf));
}

/*
*********************************************************************************************************
* Function name: UartVarInit 
* Function description: Initialize variables related to the serial port 
* Formal parameters: None 
* Return value: None
*********************************************************************************************************
*/

/*INICIALIZACION DE VARIABLES DE PUERTOS UART*/
static void UartVarInit(void)
{
#if UART1_FIFO_EN == 1
	g_tUart1.uart = USART1;						/* STM32 puerto */
	g_tUart1.pTxBuf = g_TxBuf1;					/* Puntero de buffer de envio */
	g_tUart1.pRxBuf = g_RxBuf1;					/* Puntero de buffer de recepcion*/
	g_tUart1.usTxBufSize = UART1_TX_BUF_SIZE;	/* Tamano del buffer de transmision */
	g_tUart1.usRxBufSize = UART1_RX_BUF_SIZE;	/* Tamano del buffer de recepcion*/
	g_tUart1.usTxWrite = 0;						/* Envio de indice de escritura FIFO */
	g_tUart1.usTxRead = 0;						/* Envio de indice de lectura FIFO */
	g_tUart1.usRxWrite = 0;						/* Recepcion de indice de escritura FIFO */
	g_tUart1.usRxRead = 0;						/* Recepcion de indice de lectura FIFO */
	g_tUart1.usRxCount = 0;						/* Numero de nuevos datos recibidos */
	g_tUart1.usTxCount = 0;						/* Numero de datos enviados */
	g_tUart1.SendBefor = 0;						/* Llamado de funcion antes de enviar datos */
	g_tUart1.SendOver = 0;						/* Llamado de funcion luego de enviar datos */
	g_tUart1.ReciveNew = 0;						/* Llamado de funcion luego de recibir nuevos datos */
#endif

#if UART2_FIFO_EN == 1
	Uart2Gps.uart = USART2;						/* STM32 �����豸 */
	Uart2Gps.pTxBuf = g_TxBuf2;					/* ���ͻ�����ָ�� */
	Uart2Gps.pRxBuf = g_RxBuf2;					/* ���ջ�����ָ�� */
	Uart2Gps.usTxBufSize = UART2_TX_BUF_SIZE;	/* ���ͻ�������С */
	Uart2Gps.usRxBufSize = UART2_RX_BUF_SIZE;	/* ���ջ�������С */
	Uart2Gps.usTxWrite = 0;						/* ����FIFOд���� */
	Uart2Gps.usTxRead = 0;						/* ����FIFO������ */
	Uart2Gps.usRxWrite = 0;						/* ����FIFOд���� */
	Uart2Gps.usRxRead = 0;						/* ����FIFO������ */
	Uart2Gps.usRxCount = 0;						/* ���յ��������ݸ��� */
	Uart2Gps.usTxCount = 0;						/* �����͵����ݸ��� */
	Uart2Gps.SendBefor = 0;						/* ��������ǰ�Ļص����� */
	Uart2Gps.SendOver = 0;						/* ������Ϻ�Ļص����� */
	Uart2Gps.ReciveNew = Gps_ReciveNew;			/* ���յ������ݺ�Ļص����� */
#endif

#if UART3_FIFO_EN == 1
	g_tUart3.uart = USART3;						/* STM32 �����豸 */
	g_tUart3.pTxBuf = g_TxBuf3;					/* ���ͻ�����ָ�� */
	g_tUart3.pRxBuf = g_RxBuf3;					/* ���ջ�����ָ�� */
	g_tUart3.usTxBufSize = UART3_TX_BUF_SIZE;	/* ���ͻ�������С */
	g_tUart3.usRxBufSize = UART3_RX_BUF_SIZE;	/* ���ջ�������С */
	g_tUart3.usTxWrite = 0;						/* ����FIFOд���� */
	g_tUart3.usTxRead = 0;						/* ����FIFO������ */
	g_tUart3.usRxWrite = 0;						/* ����FIFOд���� */
	g_tUart3.usRxRead = 0;						/* ����FIFO������ */
	g_tUart3.usRxCount = 0;						/* ���յ��������ݸ��� */
	g_tUart3.usTxCount = 0;						/* �����͵����ݸ��� */
	g_tUart3.SendBefor = 0;						/* ��������ǰ�Ļص����� */
	g_tUart3.SendOver = 0;						/* ������Ϻ�Ļص����� */
	g_tUart3.ReciveNew = 0;						/* ���յ������ݺ�Ļص����� */
#endif

#if UART4_FIFO_EN == 1
	g_tUart4.uart = UART4;						/* STM32 �����豸 */
	g_tUart4.pTxBuf = g_TxBuf4;					/* ���ͻ�����ָ�� */
	g_tUart4.pRxBuf = g_RxBuf4;					/* ���ջ�����ָ�� */
	g_tUart4.usTxBufSize = UART4_TX_BUF_SIZE;	/* ���ͻ�������С */
	g_tUart4.usRxBufSize = UART4_RX_BUF_SIZE;	/* ���ջ�������С */
	g_tUart4.usTxWrite = 0;						/* ����FIFOд���� */
	g_tUart4.usTxRead = 0;						/* ����FIFO������ */
	g_tUart4.usRxWrite = 0;						/* ����FIFOд���� */
	g_tUart4.usRxRead = 0;						/* ����FIFO������ */
	g_tUart4.usRxCount = 0;						/* ���յ��������ݸ��� */
	g_tUart4.usTxCount = 0;						/* �����͵����ݸ��� */
	g_tUart4.SendBefor = RS485_SendBefor;	/* ��������ǰ�Ļص����� */
	g_tUart4.SendOver = RS485_SendOver;		/* ������Ϻ�Ļص����� */
	g_tUart4.ReciveNew = 0;						/* RS485_ReciveNew ���յ������ݺ�Ļص����� */
#endif

#if UART5_FIFO_EN == 1
	g_tUart5.uart = UART5;						/* STM32 �����豸 */
	g_tUart5.pTxBuf = g_TxBuf5;					/* ���ͻ�����ָ�� */
	g_tUart5.pRxBuf = g_RxBuf5;					/* ���ջ�����ָ�� */
	g_tUart5.usTxBufSize = UART5_TX_BUF_SIZE;	/* ���ͻ�������С */
	g_tUart5.usRxBufSize = UART5_RX_BUF_SIZE;	/* ���ջ�������С */
	g_tUart5.usTxWrite = 0;						/* ����FIFOд���� */
	g_tUart5.usTxRead = 0;						/* ����FIFO������ */
	g_tUart5.usRxWrite = 0;						/* ����FIFOд���� */
	g_tUart5.usRxRead = 0;						/* ����FIFO������ */
	g_tUart5.usRxCount = 0;						/* ���յ��������ݸ��� */
	g_tUart5.usTxCount = 0;						/* �����͵����ݸ��� */
	g_tUart5.SendBefor = 0;						/* ��������ǰ�Ļص����� */
	g_tUart5.SendOver = 0;						/* ������Ϻ�Ļص����� */
	g_tUart5.ReciveNew = 0;						/* ���յ������ݺ�Ļص����� */
#endif
}

/*
*********************************************************************************************************
* Function name: InitHardUart 
* Function description: Configure the hardware parameters of the serial port (baud rate, 
data bits, stop bits, start bits, parity bits, interrupt enable) suitable for 
STM32-F4 development board 
* Formal parameters: none 
* Return value: None
*********************************************************************************************************
*/
/*DEFINICION DE BAUD RATE, DATA BITS, STOP BITS, ETC*/
static void InitHardUart(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

#if UART1_FIFO_EN == 1		/* Serial 1 TX = PA9   RX = PA10 or TX = PB6   RX = PB7*/

	/* Step 1: Turn on clocks for GPIO and USART parts */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* Step 2: Configure the GPIO of USART Tx as push-pull multiplexing mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Step 3: Configure the GPIO of USART Rx as floating input mode Since after the
	CPU is reset, the GPIO defaults to the floating input mode, so the following step 
	is not necessary However, I still recommend adding it for easy reading, and to 
	prevent other places from modifying the setting parameters of this line
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* Step 4: Configure serial port hardware parameters */
	USART_InitStructure.USART_BaudRate = UART1_BAUD;	/* Tasa de baudios */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	/* Enable receive interrupt */
	/*
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		NOTE: Do not turn on send interrupts here The 
		send interrupt enable is enabled in the SendUart() 
		function
	*/
	USART_Cmd(USART1, ENABLE);		/* enable serial port*/

	/* Small defect of the CPU: the serial port is configured 
	well, if you send it directly, the first byte cannot be 
	sent out The following statement solves the problem that 
	the first byte cannot be sent out correctly */
	
	USART_ClearFlag(USART1, USART_FLAG_TC);     /* Clear the transmission complete flag, Transmission Complete flag */
#endif

#if UART2_FIFO_EN == 1		/* Serial port 2 TX = PA2, RX = PA3 */
	/* Step 1: Turn on Clocks for GPIO and USART Parts */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* Step 2: Configure the GPIO of USART Tx as push-pull multiplexing mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Step 3: Configure GPIO of USART Rx as floating input mode Since after the CPU is reset, the GPIO defaults 
	to the floating input mode, so the following step is not necessary However, I still recommend adding it for 
	easy reading, and to prevent other places from modifying the setting parameters of this line
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/*  Step 3 has already been done, so this step can be skipped GPIO_InitStructure.GPIO_Speed ??= GPIO_Speed_50MHz;
	*/
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Step 4: Configure serial port hardware parameters*/
	USART_InitStructure.USART_BaudRate = UART2_BAUD;	/* tasa de baudios */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;		/* Only select receive mode*/
	USART_Init(USART2, &USART_InitStructure);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	/* Enable receive interrupt */
	/*
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE); NOTE: Do not turn on send interrupts here 
		The send interrupt enable is enabled in the SendUart() function
	*/
	USART_Cmd(USART2, ENABLE);		/* enable serial port */

	/* Small defect of the CPU: the serial port is configured well, if you send it directly, 
	the first byte cannot be sent out The following statement solves the problem that the 
	first byte cannot be sent out correctly */
	USART_ClearFlag(USART2, USART_FLAG_TC);     /* Clear send completion flag, Transmission Complete flag */
#endif

#if UART3_FIFO_EN == 1			/* Serial port 3 TX = PB10 RX = PB11 */
	/* Step 1: Turn on GPIO and UART clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* Step 2: Configure the GPIO of USART Tx as push-pull multiplexing mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Step 3: Configure GPIO of USART Rx as floating input mode Since after the CPU is reset, the GPIO 
	defaults to the floating input mode, so the following step is not necessary However, I still 
	recommend adding it for easy reading, and to prevent other places from modifying the setting 
	parameters of this line
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*  Step 3 has already been done, so this step can be skipped
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	*/
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Step 4: Configure serial port hardware parameters */
	USART_InitStructure.USART_BaudRate = UART3_BAUD;	/* tasa de baudiios */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);	/* Enable receive interrupt */
	/*
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		NOTE: Do not turn on send interrupts here 
		The send interrupt enable is enabled in the SendUart() function
	*/
	USART_Cmd(USART3, ENABLE);		/* enable serial port*/

	/* Small defect of the CPU: the serial port is configured well, 
	if you send it directly, the first byte cannot be sent out The 
	following statement solves the problem that the first byte cannot 
	be sent out correctly */
	USART_ClearFlag(USART3, USART_FLAG_TC);     /* Clear send completion flag, Transmission Complete flag */
#endif

#if UART4_FIFO_EN == 1			/*Serial port 4 TX = PC10   RX = PC11 */
	/* Configure PD0 as a push-pull output for switching the transceiver status of the RS485 chip */
	{
		RCC_APB2PeriphClockCmd(RCC_RS485_DIR, ENABLE);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Pin = RS485_DIR_PIN;
		GPIO_Init(RS485_DIR_PORT, &GPIO_InitStructure);
	}
	/* Step 1: Turn on GPIO and UART clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	/* Step 2: Configure the GPIO of USART Tx as push-pull multiplexing mode*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Step 3: Configure GPIO of USART Rx as floating input mode Since after the CPU 
	is reset, the GPIO defaults to the floating input mode, so the following step is 
	not necessary However, I still recommend adding it for easy reading, and to 
	prevent other places from modifying the setting parameters of this line
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Step 4: Configure serial port hardware parameters */
	USART_InitStructure.USART_BaudRate = UART4_BAUD;	/* tasa baudios */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);

	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);	/* Enable receive interrupt */
	/*
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		NOTE: Do not turn on send interrupts here The 
		send interrupt enable is enabled in the SendUart() function
	*/
	USART_Cmd(UART4, ENABLE);		/* enable serial port*/

	/* Small defect of the CPU: the serial port is configured well, 
	if you send it directly, the first byte cannot be sent out The 
	following statement solves the problem that the first byte cannot 
	be sent out correctly */
	USART_ClearFlag(UART4, USART_FLAG_TC);     /* Clear send completion flag, Transmission Complete flag */
#endif

#if UART5_FIFO_EN == 1			/* Serial port 5 TX = PC12 RX = PD2 */
	/* Step 1: Turn on GPIO and UART clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

	/* Step 2: Configure the GPIO of USART Tx as push-pull multiplexing mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Step 3: Configure GPIO of USART Rx as floating input mode Since 
	after the CPU is reset, the GPIO defaults to the floating input mode,
	so the following step is not necessary However, I still recommend 
	adding it for easy reading, and to prevent other places from modifying 
	the setting parameters of this line
	*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);


	/* Step 4: Configure serial port hardware parameters*/
	USART_InitStructure.USART_BaudRate = UART5_BAUD;	/* Baud rate*/
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART5, &USART_InitStructure);

	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);	/* Enable receive interrupt */
	/*
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE); NOTE: Do not turn on send 
		interrupts here The send interrupt enable is enabled in the SendUart() 
		function
	*/
	USART_Cmd(UART5, ENABLE);		/* enable serial port */

	/* Small defect of the CPU: the serial port is configured well, if you send it 
	directly, the first byte cannot be sent out The following statement solves the 
	problem that the first byte cannot be sent out correctly */
	USART_ClearFlag(UART5, USART_FLAG_TC);     /* Clear the sending completion flag, Transmission Complete flag */
#endif
}

/*
*********************************************************************************************************
*	Function name: ConfigUartNVIC 
* Function description: configure serial port hardware interrupt. 
* Formal parameters: none 
* Return value: None
*********************************************************************************************************
*/
static void ConfigUartNVIC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configure the NVIC Preemption Priority Bits */
	/*	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  --- Configure the interrupt priority group in bsp_Init() in bsp.c */

#if UART1_FIFO_EN == 1
	/* Enable serial port 1 interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if UART2_FIFO_EN == 1
	/* Enable serial port 2 interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if UART3_FIFO_EN == 1
	/* Enable serial port 3 interrupt t */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if UART4_FIFO_EN == 1
	/* Enable serial port 4 interrupt t */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if UART5_FIFO_EN == 1
	/* Enable serial port 5 interrupt t */
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
}

/*
*********************************************************************************************************
* Function name: UartIRQ 
* Function description: Called by interrupt service program, general serial port interrupt processing function 
* Formal parameter: _pUart : serial device 
* Return value: None
*********************************************************************************************************
*/
static void UartIRQ(UART_T *_pUart)
{
	/* Handling Receive Interrupts  */
	if(USART_GetITStatus(_pUart->uart, USART_IT_RXNE) != RESET)
	{
		/* Read data from the serial port receive data register and store it 
		in the receive FIFO */
		
		uint8_t ch;
		
		ch = USART_ReceiveData(_pUart->uart);
		_pUart->pRxBuf[_pUart->usRxWrite] = ch;
		
		if (++_pUart->usRxWrite >= _pUart->usRxBufSize)
		{
			_pUart->usRxWrite = 0;
		}
		if (_pUart->usRxCount < _pUart->usRxBufSize)
		{
			_pUart->usRxCount++;
		}

		/* Callback function to notify the application that new data has been received,
		usually by sending a message or setting a flag */
        if (_pUart->ReciveNew)
        {
            _pUart->ReciveNew(ch);
        }
	}

	/* Handle send buffer empty interrupt */
	if(USART_GetITStatus(_pUart->uart, USART_IT_TXE) != RESET)
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* When the data in the sending buffer has been fetched, the sending buffer 
			empty interrupt is prohibited (note: the last data has not been sent yet*/
			USART_ITConfig(_pUart->uart, USART_IT_TXE, DISABLE);

			/* Enable data transmission complete interrupt*/
			USART_ITConfig(_pUart->uart, USART_IT_TC, ENABLE);
		}
		else
		{
			/* Take 1 byte from the sending FIFO and write it into the serial port 
			sending data register */
			USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}
	}
	/* Interrupt when all data bits have been sent */
	else if (USART_GetITStatus(_pUart->uart, USART_IT_TC) != RESET)
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* If all the data in the sending FIFO has been sent, the data sending 
			complete interrupt is prohibited */
			USART_ITConfig(_pUart->uart, USART_IT_TC, DISABLE);

			/* Callback function, generally used to handle RS485 communication, set the 
			RS485 chip to receive mode, to avoid preempting the bus */
			if (_pUart->SendOver)
			{
				_pUart->SendOver();
			}
		}
		else
		{
			/* Under normal circumstances, this branch will not be entered */
			/* If the data in the sending FIFO has not been completed, take 
			1 data from the sending FIFO and write it into the sending data register */
			USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}
	}
}

static void GpsIRQ(void)
{
	/* Handling Receive Interrupts  */
	if(USART_GetITStatus(Uart2Gps.uart, USART_IT_RXNE) != RESET)
	{
		/* Read data from the serial port receive data register and 
		store it in the receive FIFO */
		uint8_t ch;
		
		ch = USART_ReceiveData(Uart2Gps.uart);
        if(ch == '$')
        {
            Uart2Gps.usRxWrite = 0;
            Uart2Gps.usRxCount = 0;
        }
		Uart2Gps.pRxBuf[Uart2Gps.usRxWrite] = ch;
        Uart2Gps.usRxWrite++;
        
		if (Uart2Gps.usRxCount < Uart2Gps.usRxBufSize)
		{
			Uart2Gps.usRxCount++;
		}
        
        //Uart2Gps is for GPS, and if GPS get a packet end code then we analysis it; 
        if(ch == '\n')//0x0a
		{
			if(Uart2Gps.ReciveNew)
			{
				Uart2Gps.ReciveNew(Uart2Gps.usRxCount);
			}
		}
	}

	/* Handle send buffer empty interrupt */
	if(USART_GetITStatus(Uart2Gps.uart, USART_IT_TXE) != RESET)
	{
		if (Uart2Gps.usTxCount == 0)
		{
			/* When the data in the sending buffer has been fetched, disable the sending 
			buffer empty interrupt (note: the last data has not been sent yet)*/
			USART_ITConfig(Uart2Gps.uart, USART_IT_TXE, DISABLE);

			/* Enable data transmission complete interrupt */
			USART_ITConfig(Uart2Gps.uart, USART_IT_TC, ENABLE);
		}
		else
		{
			/* Take 1 byte from the sending FIFO and write it into the serial port sending data register */
			USART_SendData(Uart2Gps.uart, Uart2Gps.pTxBuf[Uart2Gps.usTxRead]);
			if (++Uart2Gps.usTxRead >= Uart2Gps.usTxBufSize)
			{
				Uart2Gps.usTxRead = 0;
			}
			Uart2Gps.usTxCount--;
		}
	}
	/* Interrupt when all data bits are sent */
	else if (USART_GetITStatus(Uart2Gps.uart, USART_IT_TC) != RESET)
	{
		if (Uart2Gps.usTxCount == 0)
		{
			/* If all the data in the sending FIFO has been sent, disable the interrupt when the data is sent */
			USART_ITConfig(Uart2Gps.uart, USART_IT_TC, DISABLE);

			/* Callback function, generally used to handle RS485 communication, 
			set the RS485 chip to receive mode to avoid preempting the bus */
			if (Uart2Gps.SendOver)
			{
				Uart2Gps.SendOver();
			}
		}
		else
		{
			/* Under normal circumstances, this branch will not be entered */ 
			/* If the data in the sending FIFO is not complete, take 1 data from 
			the sending FIFO and write it into the sending data register */
			USART_SendData(Uart2Gps.uart, Uart2Gps.pTxBuf[Uart2Gps.usTxRead]);
			if (++Uart2Gps.usTxRead >= Uart2Gps.usTxBufSize)
			{
				Uart2Gps.usTxRead = 0;
			}
			Uart2Gps.usTxCount--;
		}
	}
}

void Gps_ReciveNew(uint16_t RxCount)
{
    if(strncmp((char*)(&Uart2Gps.pRxBuf[3]), "GGA", 3) == 0)
    {
        if(strstr((char*)Uart2Gps.pRxBuf, ",,,,,"))
        {
            //printf("Place the GPS to open area\n");
            return;
        }
        else 
        {
            //float fLat,fLng;
            char tmp[10];
            //$GNGGA,073741.000,2243.0486,N,11348.3295,E,1,09,1.6,18.8,M,0.0,M,,*44
            sscanf((char*)Uart2Gps.pRxBuf,"%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]", tmp, OP.Gps.time_str, OP.Gps.Latitude, OP.Gps.NS, OP.Gps.Longitude, OP.Gps.EW);
            /* 
            sscanf(OP.Gps.Latitude+2,"%f", &fLat);
            fLat /= 60;
            fLat += (OP.Gps.Latitude[0] - '0')*10 + (OP.Gps.Latitude[1] - '0');
            
            sscanf(OP.Gps.Longitude+3,"%f", &fLng);
            fLng /= 60;
            fLng += (OP.Gps.Longitude[0] - '0')*100 + (OP.Gps.Longitude[1] - '0')*10 + (OP.Gps.Longitude[2] - '0');
            printf("Lng,Lat:%.06f,%.06f\n", fLng, fLat);
            */
//            printf("Time : %s\n", OP.Gps.time_str);
//            printf("ns   : %s\n", OP.Gps.NS);
//            printf("ew   : %s\n", OP.Gps.EW);
//            printf("Lat  : %s\n", OP.Gps.Latitude);
//            printf("Lng  : %s\n", OP.Gps.Longitude);
        }
    }
    else if(strncmp((char*)(&Uart2Gps.pRxBuf[3]), "ZDA", 3) == 0)
    {
        if(strstr((char*)Uart2Gps.pRxBuf, ",,,,,"))
        {
            BasicInfo.GpsClockFlag = 0;
            //printf("Place the GPS to open area\n");
            return;
        }
        else 
        {
            int n;
            char tmp[10];
            //$GNZDA,073741.000,22,11,2018,00,00*45
            if(sscanf((char*)Uart2Gps.pRxBuf,"%[^,],%[^,],%[^,],%[^,],%[^,]", tmp,
               OP.Gps.time_str, OP.Gps.day_str, OP.Gps.month_str, OP.Gps.year_str)==5)
            {
                //2022/01/07 035457.000
                sscanf(OP.Gps.year_str+2,"%2d", &n);    OP.Gps.utc.year = n;
                sscanf(OP.Gps.month_str,"%2d", &n);     OP.Gps.utc.month = n;
                sscanf(OP.Gps.day_str,"%2d", &n);       OP.Gps.utc.day = n;
                sscanf(OP.Gps.time_str,"%2d", &n);      OP.Gps.utc.hour = n;
                sscanf(&OP.Gps.time_str[2],"%2d", &n);  OP.Gps.utc.minute = n;
                sscanf(&OP.Gps.time_str[4],"%2d", &n);  OP.Gps.utc.second = n;
                
                utc_to_local(&OP.Gps.local, &OP.Gps.utc, BasicInfo.TimeZone);
                #if DEBUG > 9
                printf_fifo_hex(&OP.Gps.local.second, 7);
                #endif
                if(OP.Gps.utc.second >= 2 && Rtc.second >= 2)
                {
                    if(OP.Seconds > OP.gps_seconds)
                    {
                        if(OP.Seconds - OP.gps_seconds > 2)OP.sync_with_gps_flag = 1;
                    }
                    else
                    {
                         if(OP.gps_seconds - OP.Seconds > 2)OP.sync_with_gps_flag = 1;
                    }
                }
                BasicInfo.GpsClockFlag = 1;
            }
        }
    }
}

void printf_fifo_dec(uint8_t* tx, uint8_t len)
{
    while(len--)
    {
        printf("%02d ",*tx++);
    }
    printf("\n");
}

void printf_fifo_hex(uint8_t* tx, uint8_t len)
{
    while(len--)
    {
        printf("%02x ",*tx++);
    }
    printf("\n");
}
/*
*********************************************************************************************************
* Function name: USART1_IRQHandler USART2_IRQHandler USART3_IRQHandler UART4_IRQHandler UART5_IRQHandler 
* Function description: USART interrupt service routine 
* Formal parameters: none 
* Return value: None
*********************************************************************************************************
*/
#if UART1_FIFO_EN == 1
void USART1_IRQHandler(void)
{
	UartIRQ(&g_tUart1);
}
#endif

//GPS reception interrupt
#if UART2_FIFO_EN == 1
void USART2_IRQHandler(void)
{
    GpsIRQ();
}
#endif

#if UART3_FIFO_EN == 1
void USART3_IRQHandler(void)
{
	UartIRQ(&g_tUart3);
}
#endif

#if UART4_FIFO_EN == 1
void UART4_IRQHandler(void)
{
	UartIRQ(&g_tUart4);
}
#endif

#if UART5_FIFO_EN == 1
void UART5_IRQHandler(void)
	
{
	UartIRQ(&g_tUart5);
	/*UartIRQ(&g_tUart5);
	BEEP_ON();
	bsp_DelayMS(1000);
	BEEP_OFF();
	bsp_DelayMS(1000);
	BEEP_ON();
	bsp_DelayMS(1000);
	BEEP_OFF();
	*/
}
#endif

/*
*********************************************************************************************************
* Function name: fputc 
* Function description: Redefine the putc function so that the printf function 
can be used to print output from serial port 1 
* Formal parameters: None 
* Return value: None
*********************************************************************************************************
*/
int fputc(int ch, FILE *f)
{
    UartSendChar(&g_tUart5, ch);
	return ch;
}

/*
*********************************************************************************************************
* Function name: fgetc 
* Function description: Redefine the getc function so that you can use the getchar 
function to input data from serial port 1 
* Formal parameters: none 
* Return value: None
*********************************************************************************************************
*/
int fgetc(FILE *f)
{
	uint8_t ucData;
	while(UartGetChar(&g_tUart5, &ucData) == 0);
	return ucData;
}



/*FUNCIÓN PARA RECEPCIÓN DE DATOS*/

void serial_ex(uint16_t RxSerial)
{
	
	
	BEEP_ON();
	bsp_DelayMS(1000);
	BEEP_OFF();
	bsp_DelayMS(1000);
	BEEP_ON();
	bsp_DelayMS(1000);
	BEEP_OFF();
}
