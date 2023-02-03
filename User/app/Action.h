#ifndef ACTION_H
#define ACTION_H

#include "public.h"

/* 参照NTCIP，不需要WorkMode，只需要一个Status，已修改 */
typedef struct
{
    uint8_t Num;                  //1 动作号1-255
    uint8_t Pattern;              //2 方案号0-255
    uint8_t AuxillaryFunction;    //3 辅助功能
    uint8_t SpecialFunction;      //4 特殊功能
}ActionType;    //动作表 信息

typedef struct
{
    uint8_t Maximum;
    ActionType Action[ActionMax];     //4 * 100
    uint8_t Reserve[15];
}ActionTable;    //动作表 信息  416 = 0x01A0

extern ActionType   Action;         //当前运行动作
extern ActionTable  ActionTab;      //动作表
extern uint8_t      ActionStatus;

uint8_t GetActionIndex(ActionTable* Action_tab, uint8_t ActionNum);
void ActionDataInit(uint8_t n);
void ActionDefault(void);

#endif
