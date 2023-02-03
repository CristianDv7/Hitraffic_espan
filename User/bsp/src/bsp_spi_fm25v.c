/*
*********************************************************************************************************
*	模块名称 : SPI接口串行FLASH 读写模块
*	文件名称 : bsp_spi_fm25v.c
*	版    本 : V1.0
*	说    明 : 
*
*********************************************************************************************************
*/

#include "bsp.h"


/* 定义串行Flash ID */
const char Fm25v_ID[9] = {0x7F,0x7F,0x7F,0x7F,0x7F,0x7F,0xC2,0x24,0x00};


/* Fm25v10-G片选口线 置低选中 置高不选中 */
#define Fm25v_CS_PORT       GPIOC
#define Fm25v_CS_PIN        GPIO_Pin_4
#define Fm25v_CS_LOW()      Fm25v_CS_PORT->BRR  = Fm25v_CS_PIN
#define Fm25v_CS_HIGH()     Fm25v_CS_PORT->BSRR = Fm25v_CS_PIN

/* Fm25v10-G片选口线 置低选中 置高不选中 */
#define W25Q_CS_PORT        GPIOC
#define W25Q_CS_PIN	        GPIO_Pin_5
#define W25Q_CS_LOW()       W25Q_CS_PORT->BRR  = W25Q_CS_PIN
#define W25Q_CS_HIGH()      W25Q_CS_PORT->BSRR = W25Q_CS_PIN

/* Fm25v10-G片选口线 置低选中 置高不选中 */
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

#define DUMMY_BYTE  0xA5	/* 哑命令，可以为任意值，用于读操作 */

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
*	功能说明: 配置串行Flash的片选GPIO。 设置为推挽输出
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void Fm25v_ConfigGPIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 使能GPIO 时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE, ENABLE);

	/* PC4-Fm25v_CS PC5-W25Q_CS PA8-W5500_CS 配置片选口线为推挽输出模式 */
	Fm25v_SetCS(1);		/* 片选置高，不选中 */
	GPIO_SetBits(W25Q_CS_PORT,W25Q_CS_PIN);
	GPIO_SetBits(W5500_CS_PORT,W5500_CS_PIN);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  /* IO口最大速度 */	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_Pin = Fm25v_CS_PIN | W25Q_CS_PIN | W5500_CS_PIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  /* IO口最大速度 */	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_Pin = W5500_CS_PIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitSpiFlash
*	功能说明: 初始化串行Flash硬件接口（配置STM32的SPI时钟、GPIO)
*	形    参:  无
*	返 回 值: 无
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
*	功能说明: 设置CS。 用于运行中SPI共享。
*	形    参: 无
	返 回 值: 无
*********************************************************************************************************
*/
static void Fm25v_SetCS(uint8_t _level)
{
	if (_level == 0)
	{
		//bsp_SpiBusEnter();	/* 占用SPI总线， 用于总线共享 */
		Fm25v_CS_LOW();
	}
	else
	{
		Fm25v_CS_HIGH();
		//bsp_SpiBusExit();	    /* 释放SPI总线， 用于总线共享 */
	}
}

/*
*********************************************************************************************************
*	函 数 名: Fm25v_WriteEnable
*	功能说明: 向器件发送写使能命令
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
static void Fm25v_WriteEnable(void)
{
	Fm25v_SetCS(0);						/* 使能片选 */
	Spi_SendByte(Fm25v_WREN);			/* 发送命令 */
	Fm25v_SetCS(1);					    /* 禁能片选 */
}
static void Fm25v_WriteDisable(void)
{
	Fm25v_SetCS(0);						/* 使能片选 */
	Spi_SendByte(Fm25v_WRDI);			/* 发送命令 */
	Fm25v_SetCS(1);						/* 禁能片选 */
}

static void Fm25v_ReadStatus(uint8_t * _pBuf)
{
    Fm25v_SetCS(0);					/* 使能片选 */
    Spi_SendByte(Fm25v_RDSR);		/* 发送命令，读状态寄存器 */
    *_pBuf = Spi_SendByte(DUMMY_BYTE);
    Fm25v_SetCS(1);					/* 禁能片选 */
}

/*
*********************************************************************************************************
*	函 数 名: Fm25v_WriteStatus
*	功能说明: 写状态寄存器
*	形    参:  _ucValue : 状态寄存器的值
*	返 回 值: 无
*********************************************************************************************************
*/
static void Fm25v_WriteStatus(uint8_t _ucValue)
{
    Fm25v_SetCS(0);					/* 使能片选 */
    Spi_SendByte(Fm25v_WRSR);		/* 发送命令，写状态寄存器 */
    Spi_SendByte(_ucValue);		    /* 发送数据：状态寄存器的值 */
    Fm25v_SetCS(1);					/* 禁能片选 */
}

void Fm25v_Sleep(void)
{
    Fm25v_SetCS(0);					/* 使能片选 */
    Spi_SendByte(Fm25v_SLEEP);		/* 发送命令，写状态寄存器 */
    Fm25v_SetCS(1);					/* 禁能片选 */
}

uint8_t Fm25v_Read (uint8_t * _pBuf, uint32_t _uiReadAddr)
{
	/* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
	if(_uiReadAddr > fm25v.TotalSize)
	{
		return 0;
	}

	Fm25v_SetCS(0);						        /* 使能片选 */
	Spi_SendByte(Fm25v_READ);			        /* Fast Read Operation */
	Spi_SendByte((_uiReadAddr&0xFF0000) >> 16); /* 发送扇区地址的高8bit */
	Spi_SendByte((_uiReadAddr&0xFF00) >> 8);	/* 发送扇区地址中间8bit */
	Spi_SendByte(_uiReadAddr&0xFF);			    /* 发送扇区地址低8bit */
	*_pBuf = Spi_SendByte(DUMMY_BYTE);			/* 读一个字节并存储到pBuf，读完后指针自加1 */
	Fm25v_SetCS(1);								/* 禁能片选 */
	return 1;
}

/*
*********************************************************************************************************
*	函 数 名: Fm25v_Write
*	功能说明:
*	形    参:  	_pBuf : 数据源缓冲区；
*				_uiWriteAddr ：目标区域首地址
*				_usSize ：数据个数
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t Fm25v_Write(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize)
{
	uint32_t i;
	if((_usSize == 0) ||(_uiWriteAddr + _usSize) > fm25v.TotalSize)
	{
		return 0;
	}
	Fm25v_WriteEnable();				/* 发送写使能命令 */
	
	Fm25v_SetCS(0);						/* 使能片选 */
	Spi_SendByte(Fm25v_WRITE);			/* Write Operation */
	Spi_SendByte((_uiWriteAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
	Spi_SendByte((_uiWriteAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	Spi_SendByte(_uiWriteAddr & 0xFF);				/* 发送扇区地址低8bit */

	for( i = 0; i < _usSize; i++)
	{
		Spi_SendByte(*_pBuf++);			/* 发送数据 */
	}
	Fm25v_SetCS(1);						/* 禁止片选 */
	return 1;
}

/*
*********************************************************************************************************
*	函 数 名: Fm25v_FastRead
*	功能说明: 快速读 取若干字节。字节个数不能超出芯片容量。
*	形    参: _pBuf : 数据源缓冲区；
*						_uiReadAddr ：首地址
*						_usSize ：数据个数,不能超出芯片总容量
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t Fm25v_FastRead(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize)
{
	/* 如果读取的数据长度为0或者超出串行Flash地址空间，则直接返回 */
	if((_uiSize == 0) ||(_uiReadAddr + _uiSize) > fm25v.TotalSize)
	{
		return 0;
	}

	Fm25v_SetCS(0);									/* 使能片选 */
	Spi_SendByte(Fm25v_FSTRD);			/* Fast Read Operation */
	Spi_SendByte((_uiReadAddr & 0xFF0000) >> 16);	/* 发送扇区地址的高8bit */
	Spi_SendByte((_uiReadAddr & 0xFF00) >> 8);		/* 发送扇区地址中间8bit */
	Spi_SendByte(_uiReadAddr & 0xFF);				/* 发送扇区地址低8bit */
	Spi_SendByte(DUMMY_BYTE);
	while (_uiSize--)
	{
		*_pBuf++ = Spi_SendByte(DUMMY_BYTE);			/* 读一个字节并存储到pBuf，读完后指针自加1 */
	}
	Fm25v_SetCS(1);									/* 禁能片选 */
	return 1;
}

/*
*********************************************************************************************************
*	函 数 名: Fm25v_ReadID
*	功能说明: 读取器件ID
*	形    参: 无
*	返 回 值:
*********************************************************************************************************
*/
uint8_t Fm25v_ReadID(void)
{
	uint8_t i;

	Fm25v_SetCS(0);							/* 使能片选 */
	Spi_SendByte(Fm25v_RDID);				/* 发送读ID命令 */
	for(i=0;i<=8;i++)
    {
		fm25v.ChipID[i] = Spi_SendByte(DUMMY_BYTE);		/* 读ID的第i个字节 */
    }
	Fm25v_SetCS(1);							/* 禁能片选 */
    
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
*	功能说明: 读取器件ID,并填充器件参数
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void Fm25v_ReadInfo(void)
{
	/* 自动识别串行Flash型号 */
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

