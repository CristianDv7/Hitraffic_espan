#ifndef SEQUENCE_H
#define SEQUENCE_H
#include "public.h"


/* RingStatus definición de estado del anillo */
#define ForceOFF    0x20
#define MaxOut      0x10
#define GapOut      0x08

typedef enum
{
    MinGreen = 0, 
    Extension, 
    Maximum, 
    GreenRest, 
    YellowChange, 
    RedClearance, 
    RedRest
}RingStatus_Type;

/******************************************************************************/
typedef struct 
{
    uint8_t RingNum;            //número de timbre
    uint8_t Phase[PhaseMax];    //secuencia de fase de anillo
}RingType;  //Definición de secuencia de fase de anillo 17 bytes

typedef struct
{
    uint8_t     Num;            //Número de fase
    RingType    Ring[RingMax];  //Definición de secuencia de fase de anillo 4 anillos
}SequenceType;        //Información de la tabla de secuencia de fases 69 bytes

typedef struct
{
    uint8_t         Maximum;
    SequenceType    Seq[SequenceMax];     //16
    uint8_t         Reserve[15];
}SequenceTable;      //Tabla de secuencia de fases 16 + (17*4+1)*16 = 1120 = 0x0460

/******************************************************************************/
typedef struct
{
    uint8_t StopTime;   //bit1, detener el tiempo
    uint8_t ForceOff;   //bit1, apagado forzado
    uint8_t Max2;       //
    uint8_t MaxInhibit;
    uint8_t PedRecycle;
    uint8_t RedRest;
    uint8_t OmitRedClear;
}RingControlType;
/******************************************************************************/







extern uint8_t          RingStatus[RingMax];
extern SequenceType     SequenceNow;
extern SequenceTable    SeqTab;   //Tabla de secuencia de fases


uint8_t GetSeqMax(RingType* Ring);

void SequenceDefault(void);

#endif
