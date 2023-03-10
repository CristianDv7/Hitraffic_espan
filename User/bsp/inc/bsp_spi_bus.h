/*
*********************************************************************************************************
*
*	Nombre del módulo: controlador de bus SPI
*	Nombre del archivo: bsp_spi_bus.h
*	Version : V1.0
*	Nombre del archivo: bsp_spi_bus.h
*
*********************************************************************************************************
*/

#ifndef _BSP_SPI_BUS_H
#define _BSP_SPI_BUS_H
#include <stdint.h>



/*
	[El reloj SPI más rápido se divide por 2, no se admite ninguna división de frecuencia]
	Si es SPI1, el reloj SCK = 42M cuando la frecuencia se divide por 2, y el reloj SCK = 21M cuando la frecuencia se divide por 4
	Si es SPI3, reloj SCK = 21M cuando se divide por 2
*/
#define SPI_SPEED_42M		SPI_BaudRatePrescaler_2
#define SPI_SPEED_21M		SPI_BaudRatePrescaler_4
#define SPI_SPEED_5_2M	SPI_BaudRatePrescaler_8
#define SPI_SPEED_2_6M	SPI_BaudRatePrescaler_16
#define SPI_SPEED_1_3M	SPI_BaudRatePrescaler_32
#define SPI_SPEED_0_6M	SPI_BaudRatePrescaler_64

void bsp_InitSpiBus(void);
uint8_t Spi_SendByte(uint8_t _ucValue);


#endif
