/*
*********************************************************************************************************
*
*	模块名称 : 车辆检测器模块
*	文件名称 : DetectorVehicle.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2019-12-30  wcx     首发
*
*********************************************************************************************************
*/
#include "stm32f10x.h"
#include "public.h"
#include "Vehdet.h"

//n(0-15)
#define VD(n)                   ((GPIOF->IDR & (0x0001 << (n)))? 1 : 0)


VehdetTable             VehdetTab;          //车检器表
VehdetStatusTable       VehdetStatusTab;    //车检状态表
VolumeOccupancyTable    VolumeOccupancyTab; //车流量占有率表
VehDetStateTable        VehDetStateTab;     //车检状态表

uint8_t GetVehicleDetectorIndex(VehdetTable* VehdetTab, uint8_t DetectorNum)
{
    uint8_t     i;
    uint8_t     temp = 0xff;
    
    for(i = 0; i < VehdetMax; i++)
    {
        if(VehdetTab->vehdet[i].Num == DetectorNum)
        {
            temp = i;
            break;
        }
    }
    
    return temp;
}

void VehicleDetectorConfigInit(void)
{   
    VehdetTab.vehdet[0].Num = 1;
    VehdetTab.vehdet[0].Options = 0;
    VehdetTab.vehdet[0].CallPhase = 1;
    VehdetTab.vehdet[0].SwitchPhase = 4;
    VehdetTab.vehdet[0].Delay = 0;
    VehdetTab.vehdet[0].Extend = 5;
    VehdetTab.vehdet[0].QueueLimit = 0;
    VehdetTab.vehdet[0].NoActivity = 0;
    VehdetTab.vehdet[0].MaxPresence = 0;
    VehdetTab.vehdet[0].ErraticCounts = 0;
    VehdetTab.vehdet[0].FailTime = 100;
    VehdetTab.vehdet[0].Alarms = 0;
    VehdetTab.vehdet[0].ReportedAlarms = 0;
    VehdetTab.vehdet[0].Reset = 0;
    VehdetTab.vehdet[0].Reserve = 0xff;
    
    VehdetTab.vehdet[1].Num = 2;
    VehdetTab.vehdet[1].Options = 0;
    VehdetTab.vehdet[1].CallPhase = 2;
    VehdetTab.vehdet[1].SwitchPhase = 1;
    VehdetTab.vehdet[1].Delay = 0;
    VehdetTab.vehdet[1].Extend = 5;
    VehdetTab.vehdet[1].QueueLimit = 0;
    VehdetTab.vehdet[1].NoActivity = 0;
    VehdetTab.vehdet[1].MaxPresence = 0;
    VehdetTab.vehdet[1].ErraticCounts = 0;
    VehdetTab.vehdet[1].FailTime = 100;
    VehdetTab.vehdet[1].Alarms = 0;
    VehdetTab.vehdet[1].ReportedAlarms = 0;
    VehdetTab.vehdet[1].Reset = 0;
    VehdetTab.vehdet[1].Reserve = 0xff;
    
    VehdetTab.vehdet[2].Num = 3;
    VehdetTab.vehdet[2].Options = 0;
    VehdetTab.vehdet[2].CallPhase = 3;
    VehdetTab.vehdet[2].SwitchPhase = 2;
    VehdetTab.vehdet[2].Delay = 0;
    VehdetTab.vehdet[2].Extend = 5;
    VehdetTab.vehdet[2].QueueLimit = 0;
    VehdetTab.vehdet[2].NoActivity = 0;
    VehdetTab.vehdet[2].MaxPresence = 0;
    VehdetTab.vehdet[2].ErraticCounts = 0;
    VehdetTab.vehdet[2].FailTime = 100;
    VehdetTab.vehdet[2].Alarms = 0;
    VehdetTab.vehdet[2].ReportedAlarms = 0;
    VehdetTab.vehdet[2].Reset = 0;
    VehdetTab.vehdet[2].Reserve = 0xff;
    
    VehdetTab.vehdet[3].Num = 4;
    VehdetTab.vehdet[3].Options = 0;
    VehdetTab.vehdet[3].CallPhase = 4;
    VehdetTab.vehdet[3].SwitchPhase = 3;
    VehdetTab.vehdet[3].Delay = 0;
    VehdetTab.vehdet[3].Extend = 5;
    VehdetTab.vehdet[3].QueueLimit = 0;
    VehdetTab.vehdet[3].NoActivity = 0;
    VehdetTab.vehdet[3].MaxPresence = 0;
    VehdetTab.vehdet[3].ErraticCounts = 0;
    VehdetTab.vehdet[3].FailTime = 100;
    VehdetTab.vehdet[3].Alarms = 0;
    VehdetTab.vehdet[3].ReportedAlarms = 0;
    VehdetTab.vehdet[3].Reset = 0;
    VehdetTab.vehdet[3].Reserve = 0xff;
}

void VehicleDetectorStateInit(void) //每次配置车检器参数初始化一遍
{
    uint8_t     i, VehDetNo, StateIndex = 0;
    for(i = 0; i < VehdetMax; i++)
    {
        VehDetNo = VehdetTab.vehdet[i].Num;
        if(IsVehdet(VehDetNo))    //有效的检测器
        {
            VehDetStateTab.VehDet[StateIndex].Num = VehDetNo;
            VehDetStateTab.VehDet[StateIndex].Index = i;
            VehDetStateTab.VehDet[StateIndex].Valid = 0;
            VehDetStateTab.VehDet[StateIndex].Time = 0;
            VehDetStateTab.VehDet[StateIndex].State = 0;
            VehDetStateTab.VehDet[StateIndex].CarsByCycle = 0;
            VehDetStateTab.VehDet[StateIndex].CarsByMinute = 0;
            StateIndex++;
        }
        else break;
    }
}

void VehicleDetectorInit(void)
{
    VehicleDetectorConfigInit();
    VehicleDetectorStateInit();
}

void GetVehDetSta(void)//10ms执行一次，检测器状态获取
{
    uint8_t i,StateTemp,DetectorNum;
    static uint8_t changeTime[VehdetMax] = {0};
    for(i = 0; i < VehdetMax; i++) //只检测启用的检测器
    {
        DetectorNum = VehDetStateTab.VehDet[i].Num;  //检测器号DetectorNum 
        StateTemp = VD(DetectorNum - 1); //对应硬件的号码为 号DetectorNum-1 
        
        if(VehDetStateTab.VehDet[i].State != StateTemp)
        {
            changeTime[i]++;
            if(changeTime[i] >= FILTER_TIME)
            {
                changeTime[i] = 0;
                VehDetStateTab.VehDet[i].State = StateTemp;
            }
        }
        else
        {
            changeTime[i] = 0;
        }
    }
}

void VehDetStaCount(void)    //10ms执行一次，检测器状态时间统计
{
    uint8_t i;
    static uint8_t StatePre[VehdetMax] = { 0 };
    for(i = 0; i < VehdetMax; i++)//只统计启用的检测器
    {
        if(StatePre[i] != VehDetStateTab.VehDet[i].State)
        {
            StatePre[i] = VehDetStateTab.VehDet[i].State;
            VehDetStateTab.VehDet[i].Time = 0;
            if(StatePre[i])
                VehDetStateTab.VehDet[i].DelayCount = 1;
        }
        else
        {
            VehDetStateTab.VehDet[i].Time++;
            if(VehDetStateTab.VehDet[i].DelayCount)
            {
                uint8_t VehDetIndex = VehDetStateTab.VehDet[i].Index;

                VehDetStateTab.VehDet[i].DelayCount++;
                if(VehDetStateTab.VehDet[i].DelayCount > VehdetTab.vehdet[VehDetIndex].Delay * 1000)
                {
                    VehDetStateTab.VehDet[i].DelayCount = 0;
                    VehDetStateTab.VehDet[i].Valid = 1;
                }
            }
        }
    }
}





