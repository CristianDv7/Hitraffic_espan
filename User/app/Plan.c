/*
*********************************************************************************************************
*
*	模块名称 : 方案模块
*	文件名称 : Plan.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2019-12-30  wcx     首发
*
*********************************************************************************************************
*/
#include "Plan.h"

PeriodType       Period;     //tiempo de ejecución actual
PlanType         Plan;       //Tabla de horas de funcionamiento actuales
PlanTable        PlanTab;    //calendario


//Verifica si el plan diario está vacío: si está vacío o el parámetro excede PlanMax, devuelve 0, si no está vacío, devuelve 1;
uint8_t PlanEmptyCheck(uint8_t n)
{
    uint8_t temp;
    
    if(n>=PlanMax) return 0;
    temp = PlanTab.Plan[n].Num;
    if(temp>0 && temp <=PlanMax)//El número del plan diario es legal
    {
        temp = PlanTab.Plan[n].Period[0].ActionNum;
        if(temp>0 == temp<=ActionMax)
            return 1;
        else
            return 0;
    }
    else return 0;
}

/* 
    Tabla de períodos + Tiempo = Índice de períodos
*/
uint8_t GetPeriodIndex(PlanType* DayPlan, TimeType* Time)
{
    uint8_t     i;
    uint8_t     temp = 0xff;
	uint16_t    day_by_mins;
	uint16_t    plan_by_mins0;
	uint16_t    plan_by_mins1;
    
    //hora actual + tabla de periodos = número de periodo
	day_by_mins = Time->Hour*60 + Time->Minute;     //minutos
    for(i=0;i<24;i++)
    {
        plan_by_mins0 = DayPlan->Period[i].Time.Hour * 60 + DayPlan->Period[i].Time.Minute;
        
        if(i < 23)
            plan_by_mins1 = DayPlan->Period[i+1].Time.Hour * 60 + DayPlan->Period[i+1].Time.Minute;
        else
            plan_by_mins1 = 0;
        
        if(plan_by_mins0 <= day_by_mins)
        {
            if(day_by_mins < plan_by_mins1 || plan_by_mins1 == 0)
            {
                temp = i;
                break;
            }
        }
        else
        {
            if(plan_by_mins1 == 0)
            {
                temp = i;
                break;
            }
        }
    }
    return temp;
}

void PlanDefault(void)
{
    memset(&PlanTab.Maximum,0x00,sizeof(PlanTab));
    
    PlanTab.Maximum = PlanMax;
    PlanTab.Plan[0].Num = 1;
    PlanTab.Plan[0].Period[0].Time.Hour = 0x00;
    PlanTab.Plan[0].Period[0].Time.Minute = 0x00;
    PlanTab.Plan[0].Period[0].ActionNum = 0x01;
    
    PlanTab.Plan[0].Period[1].Time.Hour = 4;
    PlanTab.Plan[0].Period[1].Time.Minute = 50;
    PlanTab.Plan[0].Period[1].ActionNum = 0x02;
}

