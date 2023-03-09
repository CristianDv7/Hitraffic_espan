#ifndef UNIT_H
#define UNIT_H
#include "public.h"


typedef struct
{
    uint8_t     StartupFlash;       //Iniciar tiempo de parpadeo amarillo 0-255
    uint8_t     StartupAllRed;      //Arranque todo el tiempo rojo 0-255
    uint8_t     AutomaticPedClear;  //1 no puede 2 puede configurar el tiempo de limpieza automática de peatones
    uint8_t     RedRevert;          //Tiempo mínimo de luz roja 0-255
    uint8_t     BackupTimeL;
    uint8_t     BackupTimeH;        //Tiempo de copia de seguridad de la configuración del sistema 0-65535
    uint8_t     FlowCycle;          //Ciclo de recolección de flujo
    uint8_t     FlashStatus;
    uint8_t     Status;
    uint8_t     GreenConflictDetectFlag;
    uint8_t     RedGreenConflictDetectFlag;
    uint8_t     RedFailedDetectFlag;
    uint8_t     Reserve[4];
}UnitTab;   //16


extern UnitTab    Unit;


void UnitInit(void);



#endif
