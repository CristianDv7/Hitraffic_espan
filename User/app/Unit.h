#ifndef UNIT_H
#define UNIT_H
#include "public.h"


typedef struct
{
    uint8_t     StartupFlash;       //开机黄闪时间 0-255
    uint8_t     StartupAllRed;      //开机全红时间 0-255
    uint8_t     AutomaticPedClear;  //1不能 2能    设置自动行人清空时间
    uint8_t     RedRevert;          //最小红灯时间 0-255
    uint8_t     BackupTimeL;
    uint8_t     BackupTimeH;        //系统设置的备份时间 0-65535
    uint8_t     FlowCycle;          //流量采集周期
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
