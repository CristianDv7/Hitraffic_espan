#ifndef PLAN_H
#define PLAN_H
#include "public.h"


typedef struct
{
    uint8_t Hour;       //1
    uint8_t Minute;     //2
}TimeType;

typedef struct
{
    //uint8_t   EventNum;   //Número de período de tiempo Ahorre espacio de almacenamiento, no defina esta subfase
    TimeType    Time;       //1,2 veces
    uint8_t     ActionNum;  //3 Número de tabla de acciones
}PeriodType;//时段表

typedef struct 
{
    uint8_t     Num;         //1 número de horario de la franja horaria
    PeriodType  Period[PeriodMax]; //96 definiciones de períodos de tiempo, hasta 24 períodos de tiempo 3*24 = 72
}PlanType;    //calendario

typedef struct
{
    uint8_t     Maximum;
    PlanType    Plan[PlanMax];       //16 * 73
    uint8_t     Reserve[15];
}PlanTable;  //时段表 16*74 = 1184 = 0x04A0


extern PeriodType   Period;     //tiempo de ejecución actual
extern PlanType     Plan;
extern PlanTable    PlanTab;    //calendario




void PlanDefault(void);
uint8_t GetPeriodIndex(PlanType* DayPlan, TimeType* Time);


#endif
