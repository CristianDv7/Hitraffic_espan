#ifndef COORD_H
#define COORD_H

#include "public.h"

typedef struct
{
    uint8_t OperationalMode;   //modo de operación coordinada 0 automático 1-253 esquema manual 254 inducción local 255 flash
    uint8_t CorrectionMode;    //Método de corrección coordinada 1 otro 2 residente en espera 3 transición suave 4 solo aumento
    uint8_t MaximumMode;       //coordenada max camino 1 otros 2 max 1 3 max 2 4 max restricción
    uint8_t ForceMode;         //Coordinación Obligatorio Modo 1 Otro 2 Flotante 3 Fijo
    uint8_t Reserve[12];
}CoordType;   //hoja de coordinación


extern CoordType   Coord;

void CoordInit(void);

#endif
