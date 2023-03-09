#ifndef ACTION_H
#define ACTION_H

#include "public.h"

/* Consulte NTCIP, no se requiere WorkMode, solo se requiere un Estado, modificado */
typedef struct
{
    uint8_t Num;                  //1 Capítulo 1-255
    uint8_t Pattern;              //2 número de programa 0-255
    uint8_t AuxillaryFunction;    //3 Accesibilidad
    uint8_t SpecialFunction;      //4 funcion especial
}ActionType;    //información de la hoja de acción

typedef struct
{
    uint8_t Maximum;
    ActionType Action[ActionMax];     //4 * 100
    uint8_t Reserve[15];
}ActionTable;    // información de la tabla de acciones 416 = 0x01A0

extern ActionType   Action;         //acción de ejecución actual
extern ActionTable  ActionTab;      //hoja de acción
extern uint8_t      ActionStatus;

uint8_t GetActionIndex(ActionTable* Action_tab, uint8_t ActionNum);
void ActionDataInit(uint8_t n);
void ActionDefault(void);

#endif
