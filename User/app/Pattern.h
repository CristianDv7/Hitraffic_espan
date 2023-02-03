#ifndef PATTERN_H
#define PATTERN_H

#include "public.h"

typedef struct
{
    uint8_t Num;               //1 方案号
    uint8_t CycleTimeL;        //2 周期
    uint8_t CycleTimeH;        //
    uint8_t OffsetTime;        //3 相位差
    uint8_t SplitNum;          //4 绿信比号
    uint8_t SequenceNum;       //5 相序号
    uint8_t WorkMode;          //6 工作模式
}PatternType;             //方案信息

typedef struct
{
    uint8_t     Maximum;
    PatternType Pattern[PatternMax];  //7*100
    uint8_t     Reserve[3];
}PatternTable;           //方案表   704 = 16 * 44 =  0x02C0

extern uint16_t         NowCycleTime;
extern PatternType      PatternNow; //当前运行方案 
extern PatternTable     PatternTab; //方案表


uint8_t GetPatternIndex(PatternTable* PatternTab, uint8_t PatternNum);
void PatternDefault(void);

#endif
