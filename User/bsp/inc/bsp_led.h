/*
*********************************************************************************************************
*
* Nombre del módulo: módulo de controlador de indicador LED
*	Nombre del archivo: bsp_led.h
*	Version : V1.0
*	Descripción: archivo de cabecera
*
*********************************************************************************************************
*/

#ifndef __BSP_LED_H
#define __BSP_LED_H
#include <stdint.h>


/* Declaración de función para llamadas externas */
void bsp_InitLed(void);
void bsp_LedOn(uint8_t _no);
void bsp_LedOff(uint8_t _no);
void bsp_LedToggle(uint8_t _no);
uint8_t bsp_IsLedOn(uint8_t _no);

#endif
