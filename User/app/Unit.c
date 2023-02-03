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
#include "Unit.h"

UnitTab   Unit;    //单元参数

void UnitInit(void)
{
    memset(&Unit.StartupFlash,0x00,sizeof(Unit));
    Unit.StartupFlash = 6;
    Unit.StartupAllRed = 5;
    Unit.AutomaticPedClear = 2;
    Unit.RedRevert = 6;
    Unit.BackupTimeH = 2;
    Unit.BackupTimeL = 0;
    Unit.GreenConflictDetectFlag = 0;
    Unit.RedGreenConflictDetectFlag = 1;
    Unit.RedFailedDetectFlag = 0;
}

