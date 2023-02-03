/*
*********************************************************************************************************
*
*	模块名称 : 跟随相位模块
*	文件名称 : Overlap.c
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

uint16_t    OverlapCounter[OverlapMax] = {0};     //跟随相位时间统计
uint16_t    IncludedPhases[OverlapMax];     //母相位 所跟随放行的机动车相位
uint16_t    ModifierPhases[OverlapMax];     //修正相位 

OverlapTable        OverlapTab; //搭接相位表
OverlapStatusType   OverlapStatus;

void OverlapInit(void)
{
    memset(&OverlapTab.Maximum,0x00,sizeof(OverlapTab));
    OverlapTab.Maximum = OverlapMax;
}

//跟随相位号 + 跟随相位表 = 跟随相位索引
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


