#ifndef PREEMPT_H
#define PREEMPT_H

#include "public.h"


typedef struct
{
    uint8_t Num;            //优先号
    uint8_t Control;        //优先控制
    uint8_t Link;           //优先链路
    uint16_t Delay;         //延迟
    uint16_t Duration;      //持续时间
    uint8_t MinimumGreen;   //最小绿灯时间
    uint8_t MinimumWalk;    //最小行人时间
    uint8_t EnterPedClear;  //行人清空时间
    uint8_t TrackGreen;     //清轨绿灯时间
    uint8_t DwellGreen;     //驻留绿灯时间
    uint8_t MaximumPresence;//最大请求时间
    uint8_t TrackPhase[16]; //机动车清轨相位
    uint8_t DwellPhase[16]; //机动车驻留相位
    uint8_t DwellPed[16];   //行人驻留相位
    uint8_t ExitPhase[16];  //退出相位
    uint8_t State;          //优先规定
    
    uint8_t TrackOverlap[16];   //跟踪清轨相位
    uint8_t DwellOverlap[16];   //跟踪驻留相位
    uint8_t CyclingPhase[16];   //机动车循环相位
    uint8_t CyclingPed[16];     //行人循环相位
    uint8_t CyclingOverlap[16]; //跟踪循环相位
    
    uint8_t EnterYellowChange;  //进入黄灯时间
    uint8_t EnterRedClear;      //进入全红时间
    uint8_t TrackYellowChange;  //清轨黄灯时间
    uint8_t TrackRedClear;      //清轨全红时间
}Preempt_T;     //优先参数表

typedef struct
{
    Preempt_T Preempt[8];
}Preempt_Tab;     //优先参数表












#endif
