/*
*********************************************************************************************************
*	                                  
*	Nombre del módulo: módulo de interrupción
*	Nombre del archivo: stm32f10x_it.h
*	Versión: V2.0
*	Descripción: archivo de cabecera
*	Registro de modificación:
*		Número de versión Fecha Autor Descripción
*
*
*********************************************************************************************************
*/

#ifndef __STM32F10x_IT_H
#define __STM32F10x_IT_H

#include "stm32f10x.h"

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

#endif


