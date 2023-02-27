#ifndef SPLIT_H
#define SPLIT_H
#include "public.h"


/*
  1. Tiempo de fase SplitTime 0-255
        Tiempo de liberaci髇 de fase. Incluyendo la luz verde, el destello verde, la luz amarilla y el tiempo rojo completo de la fase del veh韈ulo de motor
        Y el tiempo de liberaci髇 y tiempo de despeje de la fase peatonal.
    2. Modo de relaci髇 de se馻l verde SplitMode
    Bandera de 1 variable: la fase se emite como una bandera variable en este modo de relaci髇 de se馻l verde
    2-ninguno
    3- Respuesta m韓ima del veh韈ulo:
        Cuando el control es inductivo, la fase del veh韈ulo se aplica en verde m韓imo.
        Este atributo tiene prioridad sobre el atributo "Motor Auto Request" en los par醡etros de fase.
    4- Respuesta m醲ima del veh韈ulo:
        Cuando se controla de forma inductiva, la fase del veh韈ulo motorizado se aplica al verde m醲imo.
        Este atributo tiene prioridad sobre el atributo "Motor Auto Request" en los par醡etros de fase.
    5- Respuesta peatonal:
        Durante el control de inducci髇, la fase peatonal est� obligada a obtener el derecho de liberaci髇.
        Este atributo tiene prioridad sobre el atributo "solicitud autom醫ica de peatones" en el par醡etro de fase.
    6- Respuesta m醲ima de veh韈ulos/peatones:
        En el control de inducci髇, la fase de veh韈ulos de motor est� obligada a implementar el verde m醲imo, y la fase de peatones est� obligada a obtener el derecho de liberaci髇.
        Esta propiedad tiene una prioridad m醩 alta que la propiedad "solicitud autom醫ica de veh韈ulos" y la propiedad "solicitud autom醫ica de peatones" en los par醡etros de fase.
    7- Ignorar fase
        Esta fase se elimina del esquema en este modo de relaci髇 de se馻l verde.
    3. Configuraci髇 del coordinador Coord
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

//Tiempo de fase El tiempo de liberación de la fase.
//Incluyendo la luz verde, el destello verde, la luz amarilla y el tiempo rojo completo de la fase del vehículo de motor
//Y el tiempo de liberación y tiempo de despeje de la fase peatonal.
typedef struct
{
    uint8_t PhaseNum;           //número de fase
    uint8_t Time;               //tiempo de fase
    uint8_t Mode;               //modo de fase
    uint8_t Coord;              //Configuración de coordinación 0-1
}PhaseSplitType; //Definición de relación de señal de fase verde

typedef struct
{
    uint8_t         Num;     //Relación de señal verde
    PhaseSplitType  Phase[PhaseMax];    //16
}SplitType;    //Datos de relación de letra verde


typedef struct
{
    uint8_t     Maximum;
    SplitType   Split[SplitMax];                //20
    uint8_t     Reserve[11];
}SplitTable;     //Tabla de relación de letras verdes 65 * 20 + 12 = 1312 = 82 * 16 = 0x0520


extern SplitType        SplitNow;
extern SplitTable       SplitTab;  //Tabla de relación de letras verdes
extern PhaseSplitType   RingSplit[RingMax];      //Proporción de letra verde

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
