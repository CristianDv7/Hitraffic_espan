#ifndef PATTERN_H
#define PATTERN_H

#include "public.h"

typedef struct
{
    uint8_t Num;               //1 Número de esquema
    uint8_t CycleTimeL;        //2 ciclo
    uint8_t CycleTimeH;        //
    uint8_t OffsetTime;        //3 diferencia de fase
    uint8_t SplitNum;          //4 Relación de señal verde
    uint8_t SequenceNum;       //5 Número de fase
    uint8_t WorkMode;          //6 Modo operativo
}PatternType;             //información del programa

typedef struct
{
    uint8_t     Maximum;
    PatternType Pattern[PatternMax];  //7*100
    uint8_t     Reserve[3];
}PatternTable;           // Tabla de esquemas 704 = 16 * 44 = 0x02C0

extern uint16_t         NowCycleTime;
extern PatternType      PatternNow; //Esquema de ejecución actual
extern PatternTable     PatternTab; //tabla de programas


uint8_t GetPatternIndex(PatternTable* PatternTab, uint8_t PatternNum);
void PatternDefault(void);

#endif
