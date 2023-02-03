

/*
*********************************************************************************************************
*
*	模块名称 : 动作模块
*	文件名称 : Action.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2019-12-30  wcx     首发
*
*********************************************************************************************************
*/
#include "public.h"
#include "Action.h"
#include "Tsc.h"
#include "bsp.h"


ActionType  Action;         //当前运行动作 
ActionTable ActionTab;     //动作表
uint8_t     ActionStatus;


//动作表 + 动作号 = 动作表的索引
uint8_t GetActionIndex(ActionTable* Action_tab, uint8_t ActionNum)
{
    uint8_t     i;
    uint8_t     temp = 0xff;
    
    for(i = 0; i < ActionMax; i++)
    {
        if(Action_tab->Action[i].Num == ActionNum)
        {
            temp = i;
            break;
        }
    }
    return temp;   //ActionTab.Action[temp].Pattern;
}

//默认的动作配置
//动作表1指定，运行方案1，无辅助功能，无特殊功能
void ActionDataInit(uint8_t n)
{
    ActionTab.Action[n].Num = n + 1;
    ActionTab.Action[n].Pattern = 1;
    ActionTab.Action[n].AuxillaryFunction = 1;
    ActionTab.Action[n].SpecialFunction = 1;
}

void ActionDefault(void)
{
    memset(&ActionTab.Maximum,0x00,sizeof(ActionTab));
    
    ActionTab.Maximum = ActionMax;
    ActionTab.Action[0].Num = 1;
    ActionTab.Action[0].Pattern = 1;
    ActionTab.Action[0].AuxillaryFunction = 1;
    ActionTab.Action[0].SpecialFunction = 1;
    
    ActionTab.Action[1].Num = 2;
    ActionTab.Action[1].Pattern = 2;
    ActionTab.Action[1].AuxillaryFunction = 1;
    ActionTab.Action[1].SpecialFunction = 1;
}

