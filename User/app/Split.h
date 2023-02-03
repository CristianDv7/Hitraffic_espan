#ifndef SPLIT_H
#define SPLIT_H
#include "public.h"


/*
  1. Tiempo de fase SplitTime 0-255
        Tiempo de liberación de fase. Incluyendo la luz verde, el destello verde, la luz amarilla y el tiempo rojo completo de la fase del vehículo de motor
        Y el tiempo de liberación y tiempo de despeje de la fase peatonal.
    2. Modo de relación de señal verde SplitMode
    Bandera de 1 variable: la fase se emite como una bandera variable en este modo de relación de señal verde
    2-ninguno
    3- Respuesta mínima del vehículo:
        Cuando el control es inductivo, la fase del vehículo se aplica en verde mínimo.
        Este atributo tiene prioridad sobre el atributo "Motor Auto Request" en los parámetros de fase.
    4- Respuesta máxima del vehículo:
        Cuando se controla de forma inductiva, la fase del vehículo motorizado se aplica al verde máximo.
        Este atributo tiene prioridad sobre el atributo "Motor Auto Request" en los parámetros de fase.
    5- Respuesta peatonal:
        Durante el control de inducción, la fase peatonal está obligada a obtener el derecho de liberación.
        Este atributo tiene prioridad sobre el atributo "solicitud automática de peatones" en el parámetro de fase.
    6- Respuesta máxima de vehículos/peatones:
        En el control de inducción, la fase de vehículos de motor está obligada a implementar el verde máximo, y la fase de peatones está obligada a obtener el derecho de liberación.
        Esta propiedad tiene una prioridad más alta que la propiedad "solicitud automática de vehículos" y la propiedad "solicitud automática de peatones" en los parámetros de fase.
    7- Ignorar fase
        Esta fase se elimina del esquema en este modo de relación de señal verde.
    3. Configuración del coordinador Coord
        bit0: 1 - Cuando el control es coordinado, esta fase se usa como una fase coordinada para coordinar con otras intersecciones.
        bit1: 1- como fase clave
        bit2: 1-como fase fija
*/
typedef enum 
{
    SM_Other = 1,
    SM_None = 2,
    SM_MinVehRecall = 3,
    SM_MaxVehRecall = 4,
    SM_PedRecall = 5,
    SM_MaxVehPedRecall = 6,
    SM_Omitted = 7,
}SplitMode;

#define SC_NONE       0x00
#define SC_COORD      0x01
#define SC_KEY        0x02
#define SC_FIXED      0x04

//ÏàÎ»Ê±¼ä  ÏàÎ»µÄ·ÅĞĞÊ±¼ä¡£
//°üº¬ÁË»ú¶¯³µÏàÎ»µÄÂÌµÆ¡¢ÂÌÉÁ¡¢»ÆµÆ¡¢È«ºìÊ±¼ä
//ÒÔ¼°ĞĞÈËÏàÎ»µÄ·ÅĞĞÊ±¼äºÍÇå¿ÕÊ±¼ä¡£
typedef struct
{
    uint8_t PhaseNum;           //ÏàÎ»ºÅ
    uint8_t Time;               //ÏàÎ»Ê±¼ä
    uint8_t Mode;               //ÏàÎ»Ä£Ê½
    uint8_t Coord;              //Ğ­µ÷ÅäÖÃ 0-1 
}PhaseSplitType; //ÏàÎ»ÂÌĞÅ±È¶¨Òå

typedef struct
{
    uint8_t         Num;     //ÂÌĞÅ±ÈºÅ
    PhaseSplitType  Phase[PhaseMax];    //16
}SplitType;    //ÂÌĞÅ±ÈÊı¾İ

typedef struct
{
    uint8_t     Maximum;
    SplitType   Split[SplitMax];                //20
    uint8_t     Reserve[11];
}SplitTable;     //ÂÌĞÅ±È±í 65 * 20 + 12 = 1312 = 82 * 16 = 0x0520 


extern SplitType        SplitNow;
extern SplitTable       SplitTab;  //ÂÌĞÅ±È±í 
extern PhaseSplitType   RingSplit[RingMax];      //ÂÌĞÅ±È

uint8_t GetSplitPhaseIndex(SplitType* Split, uint8_t PhaseNum);

void SplitDefault(void); 
void SplitDataInit(uint8_t n);
void SplitXDataInit(SplitType* Split);


//coordPatternStatus    //2.5.10
//localFreeStatus       //2.5.11
//coordCycleStatus      //2.5.12
//coordSyncStatus       //2.5.13
//systemPatternControl  //2.5.14
//systemSyncControl     //2.5.15


#endif
