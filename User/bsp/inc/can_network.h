/*
*********************************************************************************************************
*	                                  
*	模块名称 : CAN网络演示程序。
*	文件名称 : can_network.h
*	版    本 : V1.0
*	说    明 : 头文件
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2011-09-01 armfly  ST固件库V3.5.0版本。
*
*	Copyright (C), 2010-2011, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/


#ifndef _CAN_NETWORK_H
#define _CAN_NETWORK_H

#include "bsp.h"



extern CanTxMsg CanTxMsgStruct;
extern CanRxMsg CanRxMsgStruct;

/* 供外部调用的函数声明 */
void can_Init(void);					/* 初始化STM32 CAN硬件 */
void can_NVIC_Config(void);				/* 配置CAN中断 */
void SendCanMsg(uint8_t *p, uint8_t length);

#endif
