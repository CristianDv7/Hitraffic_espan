/*
*********************************************************************************************************
*
*	Nombre del módulo: interfaz de operación de chip de reloj
*	Nombre del archivo: sd2405.h
*	Versión: V1.0
*	Ilustrado : 
*	registro de modificación :
*		Número de versión Fecha Autor Descripción
*		V1.0    2019-03-19 wcx     Primer lanzamiento
*
*********************************************************************************************************
*/
#ifndef _SD2405_H
#define _SD2405_H

#include "tsc.h"


#define RTC_IRQ_PORT	    GPIOB
#define RTC_IRQ_PIN	        GPIO_Pin_5


#define ISL1208_ADDR    0xDE
#define SD2405_ADDR     0x64

/* Configuración de registro ISL1208_ADDR */
#define SecReg  0x00
#define MinReg  0x01
#define HourReg 0x02
#define DayReg  0x03
#define MonReg  0x04
#define YearReg 0x05
#define WeekReg 0x06

#define SR_Reg      0x07 //
#define INT_Reg     0x08 //
#define ATR_Reg     0x0A //
#define DTR_Reg     0x0B //

#define SecAlarm    0x0C //
#define MinAlarm    0x0D //
#define HourAlarm   0x0E //
#define DayAlarm    0x0F //
#define MonAlarm    0x10 //
#define WeekAlarm   0x11 //
#define User1       0x12
#define User2       0x13


extern uint8_t i2c_delay;

void RtcInit(void);
void RtcRead(RtcType* Time);
void RtcWrite(RtcType* Time);

void RtcReadTime(void);



void calc_week(RtcType* rtc);
void second_to_rtc(uint32_t seconds, RtcType* rtc);
void utc_to_local(RtcType* local, RtcType* utc, uint8_t* TimeZone);


#endif
