#ifndef PHASESTATECONTROL_H
#define PHASESTATECONTROL_H
#include "stdint.h"
#include "public.h"

typedef struct
{    
    uint8_t         RingNum;        //número de tarjeta
    uint8_t         SeqNum;         //Número de fase de la carrera
    uint8_t         SeqMax;         //Máximo de secuencia de fase
    uint8_t         PhaseChangeFlag;//Indicador de cambio de fase
    uint8_t         CycleOverFlag;  //señal de final de carrera de anillo
    uint8_t         VehicleTransitionTime;
    uint16_t        SecondRemain;   //Cuenta regresiva de segundos
}RingStateType;

typedef struct
{
    uint8_t         Phase1sFlag;
    uint8_t         Phase10msCount; //contador de 10ms
    uint8_t         miniRemainTime;
    uint8_t         NewCycleFlag;   //bandera del nuevo ciclo
    uint8_t         StepMaxRing;    //El número de anillo del que tiene el mayor número de fases.
    uint8_t         CycleStepMax;   //número de fase de anillo máximo
    uint8_t         ValidRings;     //Número válido (no vacío) de timbres
    RingStateType   Ring[RingMax];
    uint8_t         StateNum; //Secuencia de pasos actual (número de pasos)
    uint8_t         StateMax; //Estado de fase (número de pasos) máximo
    uint32_t        State[64];//Lista de estados de fase
}PhaseStateType;




extern PhaseStateType       PhaseState;

uint32_t GetPhaseNexts(void);
void PhaseStatusControl(void);//Ejecutar una vez en 1s
void OverlapStatusControl(void);//Actualizar una vez en 1S
void ChannelStatusControl(void);//Actualizar una vez en 1S
void ChannelStatusToLmap(void);

#endif
