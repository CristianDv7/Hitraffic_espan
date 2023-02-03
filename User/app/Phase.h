#ifndef PHASE_H
#define PHASE_H

#include "public.h"

/********************************************************/
#define FlashPhase      252
#define AllRedPhase     253
#define AllOffPhase     254


/********************************************************/
typedef struct
{
    uint8_t PhaseNum;
    uint8_t PhaseIndex;
    uint8_t SplitIndex;
    uint8_t PhaseNext;
    uint8_t PhaseNextIndex;
    uint8_t SplitNextIndex;
}PhaseIndexType;


/********************************************************/
typedef struct
{
    uint8_t Num;              //1-相位号
    uint8_t Walk;             //2-行人相位绿灯时间
    uint8_t PedestrianClear;  //3-行人相位绿闪时间
    uint8_t MinimumGreen;     //4-在感应控制下，相位执行绿灯的最小时间
    uint8_t Passage;          //5-在感应控制下，相位绿灯时间延长一次的时间

    uint8_t Maximum1;         //6-在感应控制下，相位执行绿灯的最大时间1
    uint8_t Maximum2;         //7-在感应控制下，相位执行绿灯的最大时间2。一般情况下，最大时间使用最大绿1，只有在强制执行最大绿2时才使用最大绿2
    uint8_t YellowChange;     //8-机动车相位绿灯结束转向红灯信号的黄灯时间
    uint8_t RedClear;         //9-机动车相位放行结束，在同环的下一个相位放行之前，放行的红灯时间
    uint8_t RedRevert;        //10-相位黄灯结束后，距再次放行绿灯所需经历的最小红灯时间

    uint8_t AddedInitial;     //11-每接非绿灯时间间隔内的车辆感应后，可变初始间隔从零开始的增加值
    uint8_t MaximumInitial;   //12-可变初始间隔的最大值
    uint8_t TimeBeforeReduction;//13-开始线性减少前的时间
    uint8_t CarsBeforeReduction;//14-开始线性减少前的车辆数
    uint8_t TimeToReduce;     //15-该对象可以成为NEMA TS 1和 TS 2所规定的线性下降的替代品，用于减小流量密度间隔

    uint8_t ReduceBy;         //16-定义递减率
    uint8_t MinimumGap;       //17-最小相位间隔参数，可允许间隔将一直下降，直到间隔等于或小于最小间隔控制设备所规定的最小间隔值。然后，可允许间隔将保持在最小间隔控制设备所规定的值
    uint8_t DynamicMaxLimit;  //18-规定了动态最大操作中运转最大的上限和下限
    uint8_t DynamicMaxStep;   //19-规定了运转最大的自动调整
    uint8_t Startup;          //20-初始状态（启动） 0-其它  1-未启动  2-绿灯  3-机动车绿灯  4-黄灯  5-全红

    uint8_t Ring;             //21-相位所在环
    uint8_t VehicleClear;     //22-自己增加的机动绿闪时间配置
    uint8_t OptionsL;         //23,24-配置选项
    uint8_t OptionsH;         
    uint8_t ConcurrencyL;     //25,26   并发相位：可以和本相位同时放行的相位,以此来判断冲突相位
    uint8_t ConcurrencyH;     // 是否改为冲突相位定义,为1表示与对应相位冲突
    uint8_t Expand[6];        //6
}PhaseType;

typedef struct
{
    uint8_t     Maximum;
    PhaseType   Phase[32];
    uint8_t     Reserve[15];
}PhaseTable; //16 + 32*32 = 1040 = 0x0410


/********************************************************/
typedef struct
{
    uint32_t Reds;
    uint32_t Yellows;
    uint32_t Greens;
    uint32_t VehClears;
    uint32_t DontWalks;
    uint32_t PedClears;
    uint32_t Walks;
    uint32_t PhaseOns;
    uint32_t PhaseNexts;
}PhaseStatusType;

/********************************************************/
//1202V0219F PAGE51 原NTCIP当中，\
以字节为单位，8个一分组，定义为一个group，\
在此，我们32位处理器，无需分组.
typedef struct 
{
    uint32_t PhaseOmit;
    uint32_t PedOmit;
    uint32_t Hold;
    uint32_t ForceOff;
    uint32_t VehCall;
    uint32_t PedCall;
}PhaseControlType;//for remote control

/********************************************************/
extern  PhaseTable          PhaseTab;       //相位表
extern  PhaseStatusType     PhaseStatus;    //
extern  PhaseControlType    PhaseControl;   //
extern  PhaseIndexType      RingPhase[RingMax]; //

extern  uint32_t            PhaseTimes[32];

uint8_t GetPhaseIndex(PhaseTable* Phase_Tab, uint8_t PhaseNum);
void PhaseDefault(void);








#endif
