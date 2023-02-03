/*
*********************************************************************************************************
*
*	模块名称 : I2C总线驱动模块
*	文件名称 : bsp_i2c_gpio.h
*	版    本 : V1.0
*	说    明 : 头文件。
*********************************************************************************************************
*/

#ifndef _BSP_I2C_H
#define _BSP_I2C_H
#include <stdint.h>

#define LM75_ADDR   0x90



void bsp_InitI2C(void);
//void I2C_ByteWrite(uint8_t Addr,uint8_t addr,uint8_t dataValue);
//uint8_t I2C_ByteRead(uint8_t Addr,uint8_t addr);
//uint8_t I2C_Master_BufferWrite(uint8_t* pBuffer, uint16_t NumByteToWrite, uint16_t DataAddress, uint8_t SlaveAddress);


void Temperature_Read(void);

#endif
