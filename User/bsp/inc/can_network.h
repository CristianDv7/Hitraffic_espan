/*
*********************************************************************************************************
*	                                  
*	Nombre del módulo: programa de demostración de red CAN.
*	Nombre del archivo: can_network.h
*	Versión: V1.0
*	Descripción: archivo de cabecera
*	Registro de modificación:
*		Número de versión Fecha Autor Descripción
*		v1.0 2011-09-01 biblioteca de firmware armfly ST versión V3.5.0.
*
*	Copyright (C), 2010-2011, Armfly Electronics www.armfly.com
*
*********************************************************************************************************
*/


#ifndef _CAN_NETWORK_H
#define _CAN_NETWORK_H

#include "bsp.h"



extern CanTxMsg CanTxMsgStruct;
extern CanRxMsg CanRxMsgStruct;

/* Declaración de función para llamadas externas */
void can_Init(void);					/* Inicializar hardware STM32 CAN */
void can_NVIC_Config(void);				/* Configurar interrupción CAN */
void SendCanMsg(uint8_t *p, uint8_t length);

#endif
