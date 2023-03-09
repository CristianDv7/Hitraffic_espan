/*
*********************************************************************************************************
*
*	Nombre del módulo: Módulo de temporizador
*	Nombre del archivo: bsp_timer.h
*	Versión: V1.3
*	Descripción: archivo de cabecera
*
*********************************************************************************************************
*/

#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

#include <stdint.h>

/*
	Defina varias variables globales del temporizador de software aquí
	Tenga en cuenta que se debe agregar __IO o volátil, ya que se accede a esta variable en la interrupción y en el programa principal al mismo tiempo, lo que puede provocar la optimización de errores del compilador.
*/
#define TMR_COUNT	4		/* Número de temporizadores de software (rango de ID de temporizador 0 - 3) */

/* Estructura del temporizador, las variables miembro deben ser volátiles; de lo contrario, puede haber problemas cuando el compilador C optimiza */
typedef enum
{
	TMR_ONCE_MODE = 0,		/* modo de trabajo de una sola vez */
	TMR_AUTO_MODE = 1		/* Modo de trabajo de temporización automática */
}TMR_MODE_E;

/* Estructura del temporizador, las variables miembro deben ser volátiles; de lo contrario, puede haber problemas cuando el compilador C optimiza */
typedef struct
{
	volatile uint8_t Mode;		/* Modo contador, 1 tiro */
	volatile uint8_t Flag;		/* señal de llegada cronometrada */
	volatile uint32_t Count;	/* encimera */
	volatile uint32_t PreLoad;	/* Valor precargado del contador */
}SOFT_TMR;

/* Funciones proporcionadas para ser llamadas por otros archivos C */
void bsp_InitTimer(void);
void bsp_DelayMS(uint32_t n);
void bsp_DelayUS(uint32_t n);
void bsp_StartTimer(uint8_t _id, uint32_t _period);
void bsp_StartAutoTimer(uint8_t _id, uint32_t _period);
void bsp_StopTimer(uint8_t _id);
uint8_t bsp_CheckTimer(uint8_t _id);
int32_t bsp_GetRunTime(void);
int32_t bsp_CheckRunTime(int32_t _LastTime);

void bsp_InitHardTimer(void);
void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void * _pCallBack);

#endif
