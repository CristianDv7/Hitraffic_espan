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
    //uint8_t   EventNum;   //时段号     节省存储空间，不定义该子相
    TimeType    Time;       //1,2 时间
    uint8_t     ActionNum;  //3 动作表号
}PeriodType;//时段表

typedef struct 
{
    uint8_t     Num;         //1  时段计划表号
    PeriodType  Period[PeriodMax]; //96 时段定义,最多分24个时间段 3*24 = 72
}PlanType;    //时段表

typedef struct
{
    uint8_t     Maximum;
    PlanType    Plan[PlanMax];       //16 * 73
    uint8_t     Reserve[15];
}PlanTable;  //时段表 16*74 = 1184 = 0x04A0


extern PeriodType   Period;     //当前运行时段
extern PlanType     Plan;
extern PlanTable    PlanTab;    //时段表




void PlanDefault(void);
uint8_t GetPeriodIndex(PlanType* DayPlan, TimeType* Time);


#endif
