/*
*********************************************************************************************************
*
*	模块名称 : Conductor de autobús SPI
*	文件名称 : bsp_spi_bus.h
*	版    本 : V1.0
*	说    明 :El controlador subyacente del bus SPI. Proporciona configuración de SPI, envío y recepción de datos y compatibilidad con SPI para compartir varios dispositivos.
*	修改记录 :
*		版本号  日期        作者    说明
*   v1.0    2014-10-24      wcx     
*
*********************************************************************************************************
*/

#include "bsp.h"


#define SPI_peripheral		SPI1
#define SPI_BAUD			SPI_BaudRatePrescaler_8


/*
	asignación de puerto SPI1
	PA5	SPI1_SCK
	PA6	SPI1_MISO
	PA7	SPI1_MOSI
*/

uint8_t g_spi_busy = 0;		/* Indicador de uso compartido de bus SPI */

static void bsp_InitSpi1GPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Habilitar reloj GPIO */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	/* PA5-SCK PA6-MISO PA7-MOSI */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  /* Velocidad máxima del puerto IO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		/* establecer salida */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitSPIBus
*	功能说明: 片选CS，Configure parámetros como el modo de trabajo y la velocidad del hardware SPI interno del STM32 para acceder al Flash serial de la interfaz SPI. Solo incluye la configuración de líneas de puertos SCK, MOSI y MISO. No incluye chip select CS, ni incluye INT, BUSY, etc. específicos para chips periféricos
*	形    参: Ninguno
*	返 回 值: Ninguno
*********************************************************************************************************
*/
void bsp_InitSpiBus(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	
	bsp_InitSpi1GPIO();
	
	/* Enciende el reloj SPI */
	//ENABLE_SPI_RCC();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	/* Configurar parámetros de hardware SPI */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* dirección de datos: dúplex completo de 2 hilos */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		/* Modo de trabajo SPI de STM32: modo host */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* longitud de datos: 8 bytes */
	/* SPI_CPOL y SPI_CPHA se usan en combinación para determinar la relación de fase entre el reloj y los puntos de muestreo de datos,
	  Configuración: Bus inactivo es de alto nivel, el segundo borde (datos de muestreo de borde ascendente)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;			/* Datos de muestra del flanco ascendente del reloj */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		/* Datos de muestra en el segundo borde del reloj */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			/* Método de control de selección de chips: control de software */

	/* Establecer el factor del preescalador de velocidad en baudios */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BAUD;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	/* Orden de transmisión de bits de datos: orden superior primero */
	SPI_InitStructure.SPI_CRCPolynomial = 7;			/* Registro polinomial CRC, 7 despues de resetear */
	SPI_Init(SPI_peripheral, &SPI_InitStructure);

	SPI_Cmd(SPI_peripheral, DISABLE);			/* Deshabilitar SPI primero */
	SPI_Cmd(SPI_peripheral, ENABLE);			/* Habilitar SPI */
}
/*
*********************************************************************************************************
*	函 数 名: Spi_SendByte
*	功能说明: Envíe un byte al dispositivo mientras muestrea los datos devueltos por el dispositivo en la línea MISO
*	parámetro formal: _ucByte: valor del byte enviado
*	valor de retorno: Datos devueltos por el dispositivo de muestreo de línea MISO
*********************************************************************************************************
*/
uint8_t Spi_SendByte(uint8_t _ucValue)
{
	/* Esperar a que se envíen los últimos datos */
	while(SPI_I2S_GetFlagStatus(SPI_peripheral, SPI_I2S_FLAG_TXE) == RESET);

	/* Enviar 1 byte a través del hardware SPI */
	SPI_I2S_SendData(SPI_peripheral, _ucValue);

	/* Esperar a que se complete la tarea de recibir un byte */
	while(SPI_I2S_GetFlagStatus(SPI_peripheral, SPI_I2S_FLAG_RXNE) == RESET);

	/* Devuelve los datos leídos desde el bus SPI */
	return SPI_I2S_ReceiveData(SPI_peripheral);
}
