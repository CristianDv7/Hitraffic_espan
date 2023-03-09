#ifndef PHASE_H
#define PHASE_H

#include "public.h"

/********************************************************/
#define FlashPhase      252
#define AllRedPhase     253
#define AllOffPhase     254


/********************************************************/
typedef struct
{
    uint8_t PhaseNum;
    uint8_t PhaseIndex;
    uint8_t SplitIndex;
    uint8_t PhaseNext;
    uint8_t PhaseNextIndex;
    uint8_t SplitNextIndex;
}PhaseIndexType;


/********************************************************/
typedef struct
{
    uint8_t Num;              //1-número de fase
    uint8_t Walk;             //2-Tiempo de luz verde fase peatón
    uint8_t PedestrianClear;  //3-Tiempo de parpadeo verde fase peatón
    uint8_t MinimumGreen;     //4-Bajo control de inducción, el tiempo mínimo para que la fase ejecute la luz verde
    uint8_t Passage;          //5-Bajo control de inducción, el tiempo de luz verde de fase se extiende una vez

    uint8_t Maximum1;         //6-Bajo control de inducción, el tiempo máximo para la ejecución de la fase luz verde 1
    uint8_t Maximum2;         //7-Bajo control de inducción, la fase ejecuta la luz verde por un tiempo máximo de 2. En general, max green 1 se usa para el tiempo máximo, max green 2 solo se usa cuando se aplica max green 2
    uint8_t YellowChange;     //8-Tiempo de luz amarilla para el final de la luz verde de la fase del vehículo de motor que gira la señal de luz roja
    uint8_t RedClear;         //9-El tiempo de luz roja antes de que se complete la liberación de fase del vehículo de motor y se libere la siguiente fase del mismo anillo
    uint8_t RedRevert;        //10-Después del final de la fase de luz amarilla, el tiempo mínimo de luz roja requerido para pasar la luz verde nuevamente

    uint8_t AddedInitial;     //11-Aumente el valor del intervalo inicial variable a partir de cero después de que cada vehículo detecte un intervalo sin luz verde
    uint8_t MaximumInitial;   //12-Valor máximo para intervalo inicial variable
    uint8_t TimeBeforeReduction;//13-Tiempo antes de comenzar a disminuir linealmente
    uint8_t CarsBeforeReduction;//14-El número de vehículos antes de comenzar a disminuir linealmente
    uint8_t TimeToReduce;     //15-Este objeto puede ser una alternativa a la disminución lineal especificada en NEMA TS 1 y TS 2 para intervalos decrecientes de densidad de flujo

    uint8_t ReduceBy;         //16-Definir tasa de disminución
    uint8_t MinimumGap;       //17-El parámetro mínimo de separación de fases, la separación permisible seguirá disminuyendo hasta que la separación sea igual o menor que el valor mínimo de separación especificado por el equipo de control de separación mínima. La separación permisible permanecerá entonces en el valor prescrito por el dispositivo de control de separación mínima.
    uint8_t DynamicMaxLimit;  //18-Especifica los límites superior e inferior del máximo operativo en el funcionamiento máximo dinámico
    uint8_t DynamicMaxStep;   //19-Especifica el ajuste automático del máximo operativo
    uint8_t Startup;          //20-Estado inicial (activado) 0-otro 1-no activado 2-luz verde 3-vehículo luz verde 4-luz amarilla 5-totalmente roja 50
    uint8_t Ring;             //21-anillo de fase
    uint8_t VehicleClear;     //22-Configuración de tiempo de parpadeo verde móvil autoañadido
    uint8_t OptionsL;         //23,24-opciones de configuración
    uint8_t OptionsH;         
    uint8_t ConcurrencyL;     //25,26 fase concurrente: la fase que se puede liberar al mismo tiempo que esta fase, para juzgar la fase conflictiva
    uint8_t ConcurrencyH;     // Si cambiar la definición de la fase de conflicto, 1 significa conflicto con la fase correspondiente
    uint8_t Expand[6];        //6
}PhaseType;

typedef struct
{
    uint8_t     Maximum;
    PhaseType   Phase[32];
    uint8_t     Reserve[15];
}PhaseTable; //16 + 32*32 = 1040 = 0x0410


/********************************************************/
typedef struct
{
    uint32_t Reds;
    uint32_t Yellows;
    uint32_t Greens;
    uint32_t VehClears;
    uint32_t DontWalks;
    uint32_t PedClears;
    uint32_t Walks;
    uint32_t PhaseOns;
    uint32_t PhaseNexts;
}PhaseStatusType;

/********************************************************/
//1202V0219F PAGE51 Entre los NTCIP originales,\
En bytes, un grupo de 8, definido como un grupo, \
Aquí tenemos procesadores de 32 bits sin agrupar.
typedef struct 
{
    uint32_t PhaseOmit;
    uint32_t PedOmit;
    uint32_t Hold;
    uint32_t ForceOff;
    uint32_t VehCall;
    uint32_t PedCall;
}PhaseControlType;//for remote control

/********************************************************/
extern  PhaseTable          PhaseTab;       //tabla de fases
extern  PhaseStatusType     PhaseStatus;    //
extern  PhaseControlType    PhaseControl;   //
extern  PhaseIndexType      RingPhase[RingMax]; //

extern  uint32_t            PhaseTimes[32];

uint8_t GetPhaseIndex(PhaseTable* Phase_Tab, uint8_t PhaseNum);
void PhaseDefault(void);








#endif
