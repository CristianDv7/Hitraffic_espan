/*
*********************************************************************************************************
*
*	Nombre del módulo : seguir el módulo de fase
*	Nombre del archivo : Overlap.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2019-12-30  wcx     首发
*
*********************************************************************************************************
*/
#include "public.h"
#include "Overlap.h"

uint16_t    OverlapCounter[OverlapMax] = {0};     //Sigue las estadísticas
uint16_t    IncludedPhases[OverlapMax];     //Fase principal La siguiente fase del vehículo de motor
uint16_t    ModifierPhases[OverlapMax];     //fase correcta 

OverlapTable        OverlapTab; //tabla de fases de vueltas
OverlapStatusType   OverlapStatus;

void OverlapInit(void)
{
    memset(&OverlapTab.Maximum,0x00,sizeof(OverlapTab));
    OverlapTab.Maximum = OverlapMax;
}

//seguir número de fase + seguir tabla de fase = seguir índice de fase
uint8_t GetOverlapIndex(OverlapTable* Overlap_tab, uint8_t OverlapNum)
{
    uint8_t     i;
    uint8_t     temp = 0xff;
    
    for(i = 0; i < OverlapMax; i++)
    {
        if(Overlap_tab->Overlap[i].Num == OverlapNum)
        {
            temp = i;
            break;
        }
    }
    
    return temp;
}


