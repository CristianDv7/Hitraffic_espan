#ifndef PREEMPT_H
#define PREEMPT_H

#include "public.h"


typedef struct
{
    uint8_t Num;            //número de prioridad
    uint8_t Control;        //control de prioridad
    uint8_t Link;           //enlace de prioridad
    uint16_t Delay;         //Demora
    uint16_t Duration;      //duración
    uint8_t MinimumGreen;   //Tiempo verde mínimo
    uint8_t MinimumWalk;    //El número mínimo de personas
    uint8_t EnterPedClear;  //tiempo de limpieza de peatones
    uint8_t TrackGreen;     //Tiempo de luz verde de autorización de pista
    uint8_t DwellGreen;     //tiempo de permanencia en luz verde
    uint8_t MaximumPresence;//Tiempo máximo de solicitud
    uint8_t TrackPhase[16]; //Fase de limpieza de vía de vehículos de motor
    uint8_t DwellPhase[16]; //Fase de estacionamiento de vehículos de motor
    uint8_t DwellPed[16];   //Fase de estacionamiento de peatones
    uint8_t ExitPhase[16];  //fase de salida
    uint8_t State;          //disposición prioritaria
    
    uint8_t TrackOverlap[16];   //fase de limpieza de pista
    uint8_t DwellOverlap[16];   //pista de fase de permanencia
    uint8_t CyclingPhase[16];   //fase del ciclo del motor
    uint8_t CyclingPed[16];     //fase ciclo peatonal
    uint8_t CyclingOverlap[16]; //fase del ciclo de pista
    
    uint8_t EnterYellowChange;  //entrar en tiempo de luz amarilla
    uint8_t EnterRedClear;      //en pleno tiempo rojo
    uint8_t TrackYellowChange;  //Tiempo de luz amarilla de despeje de vía
    uint8_t TrackRedClear;      //borrando todo el tiempo rojo
}Preempt_T;     //Lista de parámetros prioritarios

typedef struct
{
    Preempt_T Preempt[8];
}Preempt_Tab;     //优先参数表












#endif
