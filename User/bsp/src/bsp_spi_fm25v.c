/*
*********************************************************************************************************
*	Nombre del módulo : Módulo de lectura y escritura FLASH serial de interfaz SPI
*	Nombre del archivo: bsp_spi_fm25v.c
*	libro de versiones : V1.0
*	ilustrar: 
*
*********************************************************************************************************
*/

#include "bsp.h"


/* definir serie Flash ID */
const char Fm25v_ID[9] = {0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0xC2,0x24,0x00};


/* Fm25v10-G Línea de selección de chip Establecer bajo para seleccionar, establecer alto para no seleccionar */
#define Fm25v_CS_PORT       GPIOC
#define Fm25v_CS_PIN        GPIO_Pin_4
#define Fm25v_CS_LOW()      Fm25v_CS_PORT->BRR  = Fm25v_CS_PIN
#define Fm25v_CS_HIGH()     Fm25v_CS_PORT->BSRR = Fm25v_CS_PIN

/* Fm25v10-G Línea de selección de chip Establecer bajo para seleccionar, establecer alto para no seleccionar */
#define W25Q_CS_PORT        GPIOC
#define W25Q_CS_PIN	        GPIO_Pin_5
#define W25Q_CS_LOW()       W25Q_CS_PORT->BRR  = W25Q_CS_PIN
#define W25Q_CS_HIGH()      W25Q_CS_PORT->BSRR = W25Q_CS_PIN

/* Fm25v10-G Línea de selección de chip Establecer bajo para seleccionar, establecer alto para no seleccionar */
#define W5500_CS_PORT       GPIOA
#define W5500_CS_PIN        GPIO_Pin_8
#define W5500_CS_LOW()      W5500_CS_PORT->BRR  = W5500_CS_PIN
#define W5500_CS_HIGH()     W5500_CS_PORT->BSRR = W5500_CS_PIN


#define Fm25v_WREN	0x06	/* Set write enable latch */
#define Fm25v_WRDI	0x04	/* Reset write enable latch */
#define Fm25v_RDSR	0x05	/* Read Status Register */
#define Fm25v_WRSR	0x01	/* Write Status Register */
#define Fm25v_READ	0x03	/* Read memory data */
#define Fm25v_FSTRD	0x0b	/* Fast read memory data */
#define Fm25v_WRITE	0x02	/* Write memory data */
#define Fm25v_SLEEP	0xb9	/* Enter sleep mode */
#define Fm25v_RDID	0x9F	/* Read device ID */
#define Fm25v_SNR	0xc3	/* Read S/N */

#define DUMMY_BYTE  0xA5	/*Comando ficticio, puede ser cualquier valor, utilizado para la operación de lectura */

Fm25v_T fm25v;

void Fm25v_ConfigGPIO(void);
void Fm25v_Init(void);
void Fm25v_Sleep(void);
uint8_t Fm25v_ReadID(void);

static void Fm25v_SetCS(uint8_t _level);

static void Fm25v_WriteEnable(void);
static void Fm25v_WriteDisable(void);
static void Fm25v_ReadStatus(uint8_t * _pBuf);
static void Fm25v_WriteStatus(uint8_t _ucValue);

uint8_t Fm25v_Read(uint8_t * _pBuf, uint32_t _uiReadAddr);
uint8_t Fm25v_Write(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);
uint8_t Fm25v_FastRead(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);

/*
*********************************************************************************************************
*	函 数 名: sf_ConfigGPIO
*	Función descriptiva: Configure el chip selecto GPIO de serial Flash. Establecer como salida push-pull
*	Parámetros formales: Ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void Fm25v_ConfigGPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* L Habilitar reloj GPIO */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE, ENABLE);

	/* PC4-Fm25v_CS PC5-W25Q_CS PA8-W5500_CS Configure la línea de selección de chip como modo de salida push-pull */
	Fm25v_SetCS(1);		/* Chip select establecido alto, no seleccionado */
	GPIO_SetBits(W25Q_CS_PORT,W25Q_CS_PIN);
	GPIO_SetBits(W5500_CS_PORT,W5500_CS_PIN);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  /* Velocidad máxima del puerto IO */	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		/* establecer como puerto de salida*/
	GPIO_InitStructure.GPIO_Pin = Fm25v_CS_PIN | W25Q_CS_PIN | W5500_CS_PIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  /* Velocidad máxima del puerto IO */	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		/* establecer como puerto de salida*/
	GPIO_InitStructure.GPIO_Pin = W5500_CS_PIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitSpiFlash
*	功能说明: Inicialice la interfaz de hardware serial Flash (configure el reloj SPI y GPIO de STM32)
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void Fm25v_Init(void)
{
	Fm25v_ConfigGPIO();
    strcpy(fm25v.ChipName,"Fm25v10-G");
	fm25v.TotalSize = 0x1ffff;
    Fm25v_ReadID();
}

/*
*********************************************************************************************************
*	函 数 名: Fm25v_SetCS(0)
*	功能说明: Establecer CS. Se utiliza para compartir SPI sobre la marcha.
*	Parámetros formales: Ninguno
	Valor devuelto: Ninguno
*********************************************************************************************************
*/
static void Fm25v_SetCS(uint8_t _level)
{
	if (_level == 0)
	{
		//bsp_SpiBusEnter();	/* Ocupa el bus SPI para compartir bus */
		Fm25v_CS_LOW();
	}
	else
	{
		Fm25v_CS_HIGH();
		//bsp_SpiBusExit();	    /*Liberar el bus SPI para compartir bus */
	}
}

/*
*********************************************************************************************************
*	函 数 名: Fm25v_WriteEnable
*	功能说明: Enviar un comando de habilitación de escritura al dispositivo
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
static void Fm25v_WriteEnable(void)
{
	Fm25v_SetCS(0);						/* habilitar la selección de chips */
	Spi_SendByte(Fm25v_WREN);			/* Enviar comando*/
	Fm25v_SetCS(1);					    /* Desactivar selección de chip */
}
static void Fm25v_WriteDisable(void)
{
	Fm25v_SetCS(0);						/* habilitar la selección de chips */
	Spi_SendByte(Fm25v_WRDI);			/* enviar comando */
	Fm25v_SetCS(1);						/* Desactivar selección de chip */
}

static void Fm25v_ReadStatus(uint8_t * _pBuf)
{
    Fm25v_SetCS(0);					/* habilitar la selección de chips */
    Spi_SendByte(Fm25v_RDSR);		/* Enviar comando, leer registro de estado */
    *_pBuf = Spi_SendByte(DUMMY_BYTE);
    Fm25v_SetCS(1);					/* Desactivar selección de chip*/
}

/*
*********************************************************************************************************
*	函 数 名: Fm25v_WriteStatus
*	功能说明: registro de estado de escritura
*	Parámetros formales: _ucValue: el valor del registro de estado
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
static void Fm25v_WriteStatus(uint8_t _ucValue)
{
    Fm25v_SetCS(0);					/*habilitar la selección de chips */
    Spi_SendByte(Fm25v_WRSR);		/* Enviar comando, escribir registro de estado */
    Spi_SendByte(_ucValue);		    /* Enviar datos: el valor del registro de estado */
    Fm25v_SetCS(1);					/* Desactivar selección de chip */
}

void Fm25v_Sleep(void)
{
    Fm25v_SetCS(0);					/* habilitar la selección de chips */
    Spi_SendByte(Fm25v_SLEEP);		/* Enviar comando, escribir registro de estado */
    Fm25v_SetCS(1);					/* Desactivar selección de chip */
}

uint8_t Fm25v_Read (uint8_t * _pBuf, uint32_t _uiReadAddr)
{
	/* Si la longitud de los datos de lectura es 0 o supera el espacio de direcciones Flash en serie, regrese directamente */
	if(_uiReadAddr > fm25v.TotalSize)
	{
		return 0;
	}

	Fm25v_SetCS(0);						        /* habilitar la selección de chips */
	Spi_SendByte(Fm25v_READ);			        /* Fast Read Operation */
	Spi_SendByte((_uiReadAddr&0xFF0000) >> 16); /* Envía los 8 bits superiores de la dirección de sector */
	Spi_SendByte((_uiReadAddr&0xFF00) >> 8);	/* Envía los 8 bits del medio de la dirección del sector */
	Spi_SendByte(_uiReadAddr&0xFF);			    /* Enviar dirección de sector de 8 bits bajo */
	*_pBuf = Spi_SendByte(DUMMY_BYTE);			/* Lea un byte y guárdelo en pBuf, y el puntero aumentará en 1 después de leer */
	Fm25v_SetCS(1);								/* Desactivar selección de chip */
	return 1;
}

/*
*********************************************************************************************************
*	函 数 名: Fm25v_Write
*	功能说明:
*	形    参:  	_pBuf: búfer de fuente de datos;
*				_uiWriteAddr ：La primera dirección del área objetivo
*				_usSize ：Número de datos
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
uint8_t Fm25v_Write(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
	uint32_t i;
	if((_usSize == 0) ||(_uiWriteAddr + _usSize) > fm25v.TotalSize)
	{
		return 0;
	}
	Fm25v_WriteEnable();				/*Enviar comando de habilitación de escritura */
	
	Fm25v_SetCS(0);						/* habilitar la selección de chips */
	Spi_SendByte(Fm25v_WRITE);			/* Write Operation */
	Spi_SendByte((_uiWriteAddr & 0xFF0000) >> 16);	/* Envía los 8 bits superiores de la dirección de sector */
	Spi_SendByte((_uiWriteAddr & 0xFF00) >> 8);		/* Envía los 8 bits del medio de la dirección del sector */
	Spi_SendByte(_uiWriteAddr & 0xFF);				/* Enviar dirección de sector de 8 bits bajo */

	for( i = 0; i < _usSize; i++)
	{
		Spi_SendByte(*_pBuf++);			/* enviar datos */
	}
	Fm25v_SetCS(1);						/* deshabilitar selección de chip*/
	return 1;
}

/*
*********************************************************************************************************
*	Nombre de la función: Fm25v_FastRead
*	功能说明: Leer varios bytes rápidamente. El número de bytes no puede exceder la capacidad del chip.
*	parámetro formal:_pBuf: búfer de fuente de datos;
*						_uiReadAddr ：primera dirección
*						_usSize ：El número de datos no puede exceder la capacidad total del chip
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
uint8_t Fm25v_FastRead(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
	/* Si la longitud de los datos de lectura es 0 o supera el espacio de direcciones Flash en serie, regrese directamente */
	if((_uiSize == 0) ||(_uiReadAddr + _uiSize) > fm25v.TotalSize)
	{
		return 0;
	}

	Fm25v_SetCS(0);									/*habilitar la selección de chips*/
	Spi_SendByte(Fm25v_FSTRD);			/* Fast Read Operation */
	Spi_SendByte((_uiReadAddr & 0xFF0000) >> 16);	/* Envía los 8 bits superiores de la dirección de sector */
	Spi_SendByte((_uiReadAddr & 0xFF00) >> 8);		/* Envía los 8 bits del medio de la dirección del sector */
	Spi_SendByte(_uiReadAddr & 0xFF);				/* Enviar dirección de sector de 8 bits bajo */
	Spi_SendByte(DUMMY_BYTE);
	while (_uiSize--)
	{
		*_pBuf++ = Spi_SendByte(DUMMY_BYTE);			/* Lea un byte y guárdelo en pBuf, y el puntero aumentará en 1 después de leer */
	}
	Fm25v_SetCS(1);									/* Desactivar selección de chip */
	return 1;
}

/*
*********************************************************************************************************
*	函 数 名: Fm25v_ReadID
*	功能说明: Leer ID de dispositivo
*	Parámetros formales: ninguno
*	valor de retorno:
*********************************************************************************************************
*/
uint8_t Fm25v_ReadID(void)
{
	uint8_t i;

	Fm25v_SetCS(0);							/* habilitar la selección de chips */
	Spi_SendByte(Fm25v_RDID);				/* Enviar comando de lectura de ID */
	for(i=0;i<=8;i++)
    {
		fm25v.ChipID[i] = Spi_SendByte(DUMMY_BYTE);		/* Leer el i-ésimo byte del ID */
    }
	Fm25v_SetCS(1);							/* Desactivar selección de chip */
    
    //ID read finish, then check if it is ok;
    for(i=0;i<9;i++)
    {
        if(fm25v.ChipID[i] != Fm25v_ID[i]) return 0;
    }
    return 1;
}

/*
*********************************************************************************************************
*	函 数 名: Fm25v_ReadInfo
*	功能说明: Lea la identificación del dispositivo y complete los parámetros del dispositivo
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void Fm25v_ReadInfo(void)
{
	/*Identificación automática de modelos Flash seriales*/
    #if DEBUG
	printf("ChipID = %x%x%x%x%x%x%x%x%x ",
        fm25v.ChipID[0],
        fm25v.ChipID[1],
        fm25v.ChipID[2],
        fm25v.ChipID[3],
        fm25v.ChipID[4],
        fm25v.ChipID[5],
        fm25v.ChipID[6],
        fm25v.ChipID[7],
        fm25v.ChipID[8]
        );
    #endif
}

