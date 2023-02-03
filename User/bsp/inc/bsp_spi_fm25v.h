/*
*********************************************************************************************************
*
*	模块名称 : SPI接口串行FLASH 读写模块
*	文件名称 : bsp_spi_fm25v.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*********************************************************************************************************
*/

#ifndef _BSP_SPI_FM25V_H
#define _BSP_SPI_FM25V_H

#include <stdint.h>

typedef struct
{
	uint8_t ChipID[9];		/* 芯片ID */
	char ChipName[16];		/* 芯片型号字符串，主要用于显示 */
	uint32_t TotalSize;		/* 总容量 */
}Fm25v_T;

void Fm25v_Init(void);
uint8_t Fm25v_ReadID(void);
void Fm25v_ReadInfo(void);

static void Fm25v_WriteEnable(void);
static void Fm25v_WriteDisable(void);

uint8_t Fm25v_Read(uint8_t * _pBuf, uint32_t _uiReadAddr);
uint8_t Fm25v_Write(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);
uint8_t Fm25v_FastRead(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);

extern Fm25v_T fm25v;

#endif
