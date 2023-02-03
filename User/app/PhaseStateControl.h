#ifndef PHASESTATECONTROL_H
#define PHASESTATECONTROL_H
#include "stdint.h"
#include "public.h"

typedef struct
{    
    uint8_t         RingNum;        //环号
    uint8_t         SeqNum;         //运行的阶段号
    uint8_t         SeqMax;         //相序最大
    uint8_t         PhaseChangeFlag;//相位切换标志
    uint8_t         CycleOverFlag;  //环运行结束标志
    uint8_t         VehicleTransitionTime;
    uint16_t        SecondRemain;   //秒递减计数
}RingStateType;

typedef struct
{
    uint8_t         Phase1sFlag;
    uint8_t         Phase10msCount; //10ms计数器
    uint8_t         miniRemainTime;
    uint8_t         NewCycleFlag;   //新周期标志位
    uint8_t         StepMaxRing;    //相位数量最多者的环号
    uint8_t         CycleStepMax;   //环相位数量最大值
    uint8_t         ValidRings;     //有效(非空)环数
    RingStateType   Ring[RingMax];
    uint8_t         StateNum; //当前步序(步数)
    uint8_t         StateMax; //相位状态(步数)最大值
    uint32_t        State[64];//相位状态列表
}PhaseStateType;




extern PhaseStateType       PhaseState;

uint32_t GetPhaseNexts(void);
void PhaseStatusControl(void);//1s运行一次
void OverlapStatusControl(void);//1S刷新一次
void ChannelStatusControl(void);//1S刷新一次
void ChannelStatusToLmap(void);

#endif
