/*
*********************************************************************************************************
* Nombre del módulo: interrupción de puerto serie + módulo de controlador FIFO
* Nombre del archivo: bsp_uart_fifo.c
* Version : V1.0 
* Descripción: use la interrupción del puerto serie + el modo FIFO para realizar
acceso simultáneo a múltiples puertos seriales
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
* Nombre de la función: bsp_InitUart
* Descripción de la función: Inicializar el hardware del puerto serie y asignar valores iniciales a las variables globales
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_InitUart(void)
{
	UartVarInit();		/* Las variables globales deben inicializarse antes de configurar el hardware */
	InitHardUart();		/* Configurar los parámetros de hardware del puerto serie (tasa de baudios, etc.) */
	RS485_InitTXE();	/* Configurar el hardware de habilitación de envío del chip RS485, configurado como salida push-pull */
	ConfigUartNVIC();	/* Configurar la interrupción del puerto serie*/
}

/*
*********************************************************************************************************
* Nombre de la función: UartSendBuf
* Function description: Fill data into UART send buffer, and start sending interrupt. After the interrupt processing function is sent, 
la interrupción de envío se apaga automáticamente 
* Formal parameters: none 
* Return value: None
*********************************************************************************************************
*/
void UartSendBuf(UART_T *uart_t, uint8_t *_ucaBuf, uint16_t _usLen) 
{
	uint16_t i;
    
	if (uart_t->SendBefor != 0)
	{
		uart_t->SendBefor();		/* Si se trata de comunicación RS485, puede configurar RS485 para enviar el modo en esta función */
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

		/* Llena el búfer de envío con nuevos datos */
		uart_t->pTxBuf[uart_t->usTxWrite] = _ucaBuf[i];

		DISABLE_INT();
		if(++uart_t->usTxWrite >= uart_t->usTxBufSize)
		{
			uart_t->usTxWrite = 0;
		}
		uart_t->usTxCount++;
		ENABLE_INT();
	}

	USART_ITConfig(uart_t->uart, USART_IT_TXE, ENABLE); //Habilitar interrupción de envío
}

void UartSendChar(UART_T *uart_t, uint8_t _ucByte)
{
	UartSendBuf(uart_t, &_ucByte, 1);
}

/*
*********************************************************************************************************
* Nombre de la función: UartGetChar
* Descripción de la función: Leer 1 byte de datos del búfer de recepción del puerto serie (para la llamada del programa principal)
* Parámetro formal: _pUart: dispositivo serie 
* _pByte: puntero para almacenar datos leídos
* Valor de retorno: 0 significa que no hay datos, 1 significa leer datos
*********************************************************************************************************
*/
uint8_t UartGetChar(UART_T *uart_t, uint8_t *_pByte)
{
	uint16_t usCount;

	/* usRxWrite La variable se reescribe en la función de interrupción.
	Cuando el programa principal lee la variable, debe proteger el área crítica. */
	DISABLE_INT();
	usCount = uart_t->usRxCount;
	ENABLE_INT();

	/* Devuelve 0 si los índices de lectura y escritura son iguales */
	if (usCount == 0)	/* no mas datos */
	{
		return 0;
	}
	else
	{
		*_pByte = uart_t->pRxBuf[uart_t->usRxRead];		/*Tomar 1 dato del puerto serie que recibe FIFO*/
		/* Reescribe el índice de lectura FIFO */
		DISABLE_INT();
		if (++uart_t->usRxRead >= uart_t->usRxBufSize)// zona de búfer leer hasta el final,
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
* Nombre de la función: bsp_SetUart1Baud
* Descripción de la función: modificar la tasa de baudios UART1
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_SetUart1Baud(uint32_t _baud)
{
	USART_InitTypeDef USART_InitStructure;

	/* Paso 2: Configurar los parámetros de hardware del puerto serie */
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
* Nombre de la función: bsp_SetUart2Baud
* Descripción de la función: modificar la tasa de baudios UART2
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
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
* Nombre de la función: RS485_InitTXE
* Descripción de la función: Configure la línea de puerto de habilitación de transmisión RS485 TXE 
* Formal parameters: None 
* Return value: None
*********************************************************************************************************
*/
void RS485_InitTXE(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_RS485_DIR, ENABLE);	/* Enciende el reloj GPIO */

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	/* Modo de salida push-pull */
	GPIO_InitStructure.GPIO_Pin = RS485_DIR_PIN;
	GPIO_Init(RS485_DIR_PORT, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
* Nombre de la función: bsp_Set485Baud
* Descripción de la función: modificar la tasa de baudios UART4
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_Set485Baud(uint32_t _baud)
{
	USART_InitTypeDef USART_InitStructure;

	/* Paso 2: Configurar los parámetros de hardware del puerto serie */
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
	RS485_TX_EN();	/* Cambiar el chip transceptor RS485 al modo de envío */
}

void RS485_SendOver(void)
{
	RS485_RX_EN();	/* Cambiar el chip transceptor RS485 al modo de recepción */
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
* Nombre de la función: UartVarInit
* Descripción de la función: Inicializar variables relacionadas con el puerto serie
* 	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
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
* Nombre de la función: InitHardUart
* Descripción de la función: Configure los parámetros de hardware del puerto serie (tasa de baudios,
bits de datos, bits de parada, bits de inicio, bits de paridad, habilitar interrupción) adecuado para
Placa de desarrollo STM32-F4
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

	/* Paso 1: Enciende los relojes para las partes GPIO y USART */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* Paso 2: Configurar el GPIO de USART Tx como modo de multiplexación push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Paso 3: Configurar el GPIO de USART Rx como modo de entrada flotante Ya que después de la
La CPU se restablece, el GPIO pasa por defecto al modo de entrada flotante, por lo que el siguiente paso
no es necesario Sin embargo, sigo recomendando agregarlo para facilitar la lectura y para
evitar que otros lugares modifiquen los parámetros de configuración de esta línea
*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* Paso 4: Configurar los parámetros de hardware del puerto serie */
	USART_InitStructure.USART_BaudRate = UART1_BAUD;	/* Tasa de baudios */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	/* Habilitar interrupción de recepción */
	/*
USART_ITConfig(USART1, USART_IT_TXE, HABILITAR);
NOTA: No active el envío de interrupciones aquí.
la habilitación de interrupción de envío está habilitada en SendUart()
función
*/
	USART_Cmd(USART1, ENABLE);		/* habilitar puerto serie*/

	/* Pequeño defecto de la CPU: el puerto serie está configurado
bueno, si lo envía directamente, el primer byte no puede ser
enviado La siguiente sentencia resuelve el problema de que
el primer byte no se puede enviar correctamente */
	
	USART_ClearFlag(USART1, USART_FLAG_TC);     /* Borra el indicador de transmisión completa, el indicador de transmisión completa */
#endif

#if UART2_FIFO_EN == 1		/* Puerto serie 2 TX = PA2, RX = PA3 */
	/* Paso 1: Encienda los relojes para piezas GPIO y USART */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* Paso 2: Configurar el GPIO de USART Tx como modo de multiplexación push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Paso 3: Configurar el GPIO de USART Rx como modo de entrada flotante Ya que después de la
La CPU se restablece, el GPIO pasa por defecto al modo de entrada flotante, por lo que el siguiente paso
no es necesario Sin embargo, sigo recomendando agregarlo para facilitar la lectura y para
evitar que otros lugares modifiquen los parámetros de configuración de esta línea
*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* El paso 3 ya se ha realizado, por lo que este paso se puede omitir GPIO_InitStructure.GPIO_Speed ​​??= GPIO_Speed_50MHz; */
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Paso 4: Configure los parámetros de hardware del puerto serie*/
	USART_InitStructure.USART_BaudRate = UART2_BAUD;	/* tasa de baudios */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;		/* Seleccionar solo el modo de recepción*/
	USART_Init(USART2, &USART_InitStructure);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	/* Habilitar interrupción de recepción */
	/*
		USART_ITConfig(USART1, USART_IT_TXE, HABILITAR); NOTA: No active las interrupciones de envío aquí
		La habilitación de interrupción de envío está habilitada en la función SendUart()
	*/
	USART_Cmd(USART2, ENABLE);		/* habilitar puerto serie */

	/* Pequeño defecto de la CPU: el puerto serie está configurado
bueno, si lo envía directamente, el primer byte no puede ser
enviado La siguiente sentencia resuelve el problema de que
el primer byte no se puede enviar correctamente */
	USART_ClearFlag(USART2, USART_FLAG_TC);    /* Borrar indicador de finalización de envío, indicador de transmisión completa */
#endif

#if UART3_FIFO_EN == 1			/* Serial puerto 3 TX = PB10 RX = PB11 */
	/* Paso 1: Enciende los relojes GPIO y UART */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* Paso 2: Configurar el GPIO de USART Tx como modo de multiplexación push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Paso 3: Configurar el GPIO de USART Rx como modo de entrada flotante Ya que después de la
La CPU se restablece, el GPIO pasa por defecto al modo de entrada flotante, por lo que el siguiente paso
no es necesario Sin embargo, sigo recomendando agregarlo para facilitar la lectura y para
evitar que otros lugares modifiquen los parámetros de configuración de esta línea
*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/* El paso 3 ya se ha realizado, por lo que este paso se puede omitir 
	GPIO_InitStructure.GPIO_Speed ????= GPIO_Speed_50MHz; */
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Paso 4: Configure los parámetros de hardware del puerto serie*/
	USART_InitStructure.USART_BaudRate = UART3_BAUD;	/* tasa de baudiios */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);	/* Habilitar interrupción de recepción */
	/*
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		NOTE: Do not turn on send interrupts here 
		The send interrupt enable is enabled in the SendUart() function
	*/
	USART_Cmd(USART3, ENABLE);		/* habilitar puerto serie*/

	/* Pequeño defecto de la CPU: el puerto serie está configurado
bueno, si lo envía directamente, el primer byte no puede ser
enviado La siguiente sentencia resuelve el problema de que
el primer byte no se puede enviar correctamente */
	USART_ClearFlag(USART3, USART_FLAG_TC);    /* Borrar indicador de finalización de envío, indicador de transmisión completa */
#endif

#if UART4_FIFO_EN == 1			/*Serial port 4 TX = PC10   RX = PC11 */
	/* Configure PD0 como una salida push-pull para cambiar el estado del transceptor del chip RS485 */
	{
		RCC_APB2PeriphClockCmd(RCC_RS485_DIR, ENABLE);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Pin = RS485_DIR_PIN;
		GPIO_Init(RS485_DIR_PORT, &GPIO_InitStructure);
	}
	/* Paso 1: Enciende los relojes GPIO y UART */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	/* Paso 2: Configure el GPIO de USART Tx como modo de multiplexación push-pull*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Paso 3: Configurar el GPIO de USART Rx como modo de entrada flotante Ya que después de la
La CPU se restablece, el GPIO pasa por defecto al modo de entrada flotante, por lo que el siguiente paso
no es necesario Sin embargo, sigo recomendando agregarlo para facilitar la lectura y para
evitar que otros lugares modifiquen los parámetros de configuración de esta línea
*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Paso 4: Configurar los parámetros de hardware del puerto serie */
	USART_InitStructure.USART_BaudRate = UART4_BAUD;	/* tasa baudios */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART4, &USART_InitStructure);

	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);	/* Habilitar interrupción de recepción */
		/*
	USART_ITConfig(USART1, USART_IT_TXE, HABILITAR); NOTA: No active las interrupciones de envío aquí
		La habilitación de interrupción de envío está habilitada en la función SendUart()
	*/
	USART_Cmd(UART4, ENABLE);		/* habilitar puerto serie*/

	/* Pequeño defecto de la CPU: el puerto serie está configurado
bueno, si lo envía directamente, el primer byte no puede ser
enviado La siguiente sentencia resuelve el problema de que
el primer byte no se puede enviar correctamente */
	USART_ClearFlag(UART4, USART_FLAG_TC);     /* Borrar indicador de finalización de envío, indicador de transmisión completa */

#endif

#if UART5_FIFO_EN == 1			/* Serial port 5 TX = PC12 RX = PD2 */
	/* Paso 1: Enciende el reloj GPIO y UART */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

	/* Paso 2: Configurar el GPIO de USART Tx como modo de multiplexación push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Paso 3: Configurar el GPIO de USART Rx como modo de entrada flotante Ya que después de la
La CPU se restablece, el GPIO pasa por defecto al modo de entrada flotante, por lo que el siguiente paso
no es necesario Sin embargo, sigo recomendando agregarlo para facilitar la lectura y para
evitar que otros lugares modifiquen los parámetros de configuración de esta línea
*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);


	/* Paso 4: Configure los parámetros de hardware del puerto serie*/
	USART_InitStructure.USART_BaudRate = UART5_BAUD;	/* Tasa de baudios*/
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(UART5, &USART_InitStructure);

	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);	/* Habilitar interrupción de recepción */
		/*
	USART_ITConfig(USART1, USART_IT_TXE, HABILITAR); NOTA: No active las interrupciones de envío aquí
		La habilitación de interrupción de envío está habilitada en la función SendUart()
	*/
	USART_Cmd(UART5, ENABLE);		/* habilitar puerto serie */

	/* Pequeño defecto de la CPU: el puerto serie está configurado
bueno, si lo envía directamente, el primer byte no puede ser
enviado La siguiente sentencia resuelve el problema de que
el primer byte no se puede enviar correctamente */
	USART_ClearFlag(UART5, USART_FLAG_TC);    /* Borra el indicador de finalización de envío, el indicador de transmisión completa */
#endif
}

/*
*********************************************************************************************************
* Nombre de la función: ConfigUartNVIC
* Descripción de la función: configurar la interrupción de hardware del puerto serie.
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
*********************************************************************************************************
*/
static void ConfigUartNVIC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Configurar los bits de prioridad de prioridad de NVIC */
	/*NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0); --- Configure el grupo de prioridad de interrupción en bsp_Init() en bsp.c */
#if UART1_FIFO_EN == 1
	/* Habilita la interrupción del puerto serie 1 */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if UART2_FIFO_EN == 1
	/* Habilita la interrupción del puerto serie 2 */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if UART3_FIFO_EN == 1
	/* Habilita la interrupción del puerto serie 3 */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if UART4_FIFO_EN == 1
	/* Habilita la interrupción del puerto serie 4 */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if UART5_FIFO_EN == 1
	/* Habilita la interrupción del puerto serie 5 */
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
}

/*
*********************************************************************************************************
* Nombre de la función: UartIRQ
* Descripción de la función: llamado por el programa de servicio de interrupción, función de procesamiento de interrupción de puerto serie general 
* Parámetro formal: _pUart: dispositivo serie
* Valor devuelto: Ninguno
*********************************************************************************************************
*/
static void UartIRQ(UART_T *_pUart)
{
	/* Manejo de interrupciones de recepción */
	if(USART_GetITStatus(_pUart->uart, USART_IT_RXNE) != RESET)
	{
		/* Leer datos del puerto serial recibir registro de datos y almacenarlos
		en el FIFO de recepción */
		
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

		/* Función de devolución de llamada para notificar a la aplicación que se han recibido
		nuevos datos, generalmente enviando un mensaje o configurando una bandera */
        if (_pUart->ReciveNew)
        {
            _pUart->ReciveNew(ch);
        }
	}

	/* Maneja la interrupción de envío de búfer vacío */
	if(USART_GetITStatus(_pUart->uart, USART_IT_TXE) != RESET)
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* Cuando se han obtenido los datos en el búfer de envío, el búfer de envío 
			la interrupción vacía está prohibida (nota: el último dato aún no ha sido enviado*/
			USART_ITConfig(_pUart->uart, USART_IT_TXE, DISABLE);

			/* Habilitar interrupción completa de transmisión de datos*/
			USART_ITConfig(_pUart->uart, USART_IT_TC, ENABLE);
		}
		else
		{
			/* Tome 1 byte del FIFO de envío y escríbalo en el puerto serie
			envío de registro de datos */
			USART_SendData(_pUart->uart, _pUart->pTxBuf[_pUart->usTxRead]);
			if (++_pUart->usTxRead >= _pUart->usTxBufSize)
			{
				_pUart->usTxRead = 0;
			}
			_pUart->usTxCount--;
		}
	}
	/* Interrumpe cuando se han enviado todos los bits de datos */
	else if (USART_GetITStatus(_pUart->uart, USART_IT_TC) != RESET)
	{
		//if (_pUart->usTxRead == _pUart->usTxWrite)
		if (_pUart->usTxCount == 0)
		{
			/* If all the data in the sending FIFO has been sent, the data sending 
			complete interrupt is prohibited *//* Si todos los datos en el envío FIFO han sido enviados, la interrupción completa del envío de datos está prohibida */
			USART_ITConfig(_pUart->uart, USART_IT_TC, DISABLE);

			/* Función de devolución de llamada, generalmente utilizada para manejar la comunicación RS485, establece el
Chip RS485 para modo recepción, para evitar apropiarse del bus */
			if (_pUart->SendOver)
			{
				_pUart->SendOver();
			}
		}
		else
		{
			/* En circunstancias normales, esta rama no se ingresará */
/* Si los datos en el FIFO de envío no han sido completados, tomar
1 datos del FIFO de envío y escribirlos en el registro de datos de envío */
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
	/* Manejo de interrupciones de recepción */
	if(USART_GetITStatus(Uart2Gps.uart, USART_IT_RXNE) != RESET)
	{
		/* Leer datos del puerto serial recibir registro de datos y
almacenarlo en el FIFO de recepción */
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
        
        //Uart2Gps es para GPS, y si el GPS obtiene un código de fin de paquete, lo analizamos; 
        if(ch == '\n')//0x0a
		{
			if(Uart2Gps.ReciveNew)
			{
				Uart2Gps.ReciveNew(Uart2Gps.usRxCount);
			}
		}
	}

	/* Maneja la interrupción de envío de búfer vacío */
	if(USART_GetITStatus(Uart2Gps.uart, USART_IT_TXE) != RESET)
	{
		if (Uart2Gps.usTxCount == 0)
		{
			/* Cuando se han obtenido los datos en el búfer de envío, deshabilitar el envío
interrupción de búfer vacío (nota: los últimos datos aún no se han enviado)*/
			USART_ITConfig(Uart2Gps.uart, USART_IT_TXE, DISABLE);

			/* Habilitar interrupción completa de transmisión de datos */
			USART_ITConfig(Uart2Gps.uart, USART_IT_TC, ENABLE);
		}
		else
		{
			/* Tome 1 byte del FIFO de envío y escríbalo en el registro de datos de envío del puerto serie */
			USART_SendData(Uart2Gps.uart, Uart2Gps.pTxBuf[Uart2Gps.usTxRead]);
			if (++Uart2Gps.usTxRead >= Uart2Gps.usTxBufSize)
			{
				Uart2Gps.usTxRead = 0;
			}
			Uart2Gps.usTxCount--;
		}
	}
	/* Interrumpe cuando se envían todos los bits de datos */
	else if (USART_GetITStatus(Uart2Gps.uart, USART_IT_TC) != RESET)
	{
		if (Uart2Gps.usTxCount == 0)
		{
			/* Si se han enviado todos los datos en el FIFO de envío, deshabilite la interrupción cuando se envían los datos */
			USART_ITConfig(Uart2Gps.uart, USART_IT_TC, DISABLE);

			/* Función de devolución de llamada, generalmente utilizada para manejar la comunicación RS485, 860 configure el chip RS485 en modo de recepción para evitar adelantarse al bus */
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
            //printf("Coloca el GPS en el área abierta\n");
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
* Nombre de la función: USART1_IRQHandler USART2_IRQHandler USART3_IRQHandler UART4_IRQHandler UART5_IRQHandler 
* Descripción de la función: rutina de servicio de interrupción USART
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
*********************************************************************************************************
*/
#if UART1_FIFO_EN == 1
void USART1_IRQHandler(void)
{
	UartIRQ(&g_tUart1);
}
#endif

//Interrupción de recepción de GPS
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
* Nombre de la función: fputc
* Descripción de la función: redefine la función putc para que la función printf
can be used to print output from serial port 1 
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
*********************************************************************************************************
*/
int fputc(int ch, FILE *f)
{
    UartSendChar(&g_tUart5, ch);
	return ch;
}

/*
*********************************************************************************************************
* Nombre de la función: fgetc
* Function description: Redefine the getc function so that you can use the getchar 
función para ingresar datos desde el puerto serie 1
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
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
