#ifndef OVERLAP_H
#define OVERLAP_H

#include "public.h"


#define OverlapTabSize  0x00B0


typedef enum{OT_OTHER = 1, OT_NORMAL, OT_MINUSGREENYELLOW} Overlap_Type;

typedef struct
{
    uint8_t  Num;                //Número 1-255 
    uint8_t  Type;               //Tipo 1 Otro 2 Regular 3 Amarillo Verde Stop
    uint8_t  IncludedPhases[2];  //2 Fase madre La fase del vehículo seguida por la liberación
    uint8_t  ModifierPhases[2];  //fase correcta
    uint8_t  TrailGreen;         //Siga la luz verde 0-255 El tiempo de luz verde para continuar pasando después de la siguiente fase sigue a la fase madre y se libera la luz verde, unidad: segundo.
    uint8_t  TrailClear;
    uint8_t  TrailYellow;        //Siga la luz amarilla 0-255 Cuando la luz verde o el tiempo de parpadeo verde de la siguiente fase no es cero, el tiempo de liberación de la luz amarilla. Cuando tanto la luz verde como el tiempo de parpadeo verde son cero, el tiempo de luz amarilla utiliza el tiempo de luz amarilla de la fase principal.
    uint8_t  TrailRed;           //Siga la luz roja 0-255 Cuando la luz verde o el tiempo de destello verde de la siguiente fase no es cero, el tiempo rojo completo de liberación. Cuando tanto la luz verde como el tiempo de parpadeo verde son cero, el tiempo rojo completo usa el tiempo rojo completo de la fase principal.
}OverlapType; //¸seguir fase

typedef struct 
{
    uint8_t     Maximum;
    OverlapType Overlap[OverlapMax];    //10 byte * 16
    uint8_t     Reserve[15];
}OverlapTable; //seguir la tabla de fases      //176

typedef struct
{
    uint32_t Reds;
    uint32_t Yellows;
    uint32_t Greens;
    uint32_t Flashs;
}OverlapStatusType;


extern uint16_t     IncludedPhases[OverlapMax];     //Fase principal La siguiente fase del vehículo de motor
extern uint16_t     ModifierPhases[OverlapMax];     //fase correcta
extern uint16_t     OverlapCounter[OverlapMax];      //¸Siga las estadísticas de tiempo de fase
extern OverlapTable         OverlapTab;  //tabla de fases de vueltas
extern OverlapStatusType    OverlapStatus;




void OverlapInit(void);
//¸número de fase de seguimiento + tabla de fase de seguimiento = índice de fase de seguimiento
uint8_t GetOverlapIndex(OverlapTable* Overlap_tab, uint8_t OverlapNum);

#endif
