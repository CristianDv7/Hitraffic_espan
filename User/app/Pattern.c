/*
*********************************************************************************************************
*
*	模块名称 : 方案模块
*	文件名称 : Pattern.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2019-12-30  wcx     首发
*
*********************************************************************************************************
*/
#include "Pattern.h"
#include "bsp.h"

uint16_t         NowCycleTime;
PatternType      PatternNow;
PatternTable     PatternTab;    //方案表


//方案号 + 方案表 = 方案索引
uint8_t GetPatternIndex(PatternTable* Pattern_tab, uint8_t PatternNum)
{
    uint8_t     i;
    uint8_t     temp = 0xff;
    
    for(i = 0; i < PatternMax; i++)
    {
        if(Pattern_tab->Pattern[i].Num == PatternNum)
        {
            temp = i;
            break;
        }
    }
    
    return temp;
}

void PatternDefault(void)
{
    memset(&PatternTab.Maximum,0x00,sizeof(PatternTab));
    
    PatternTab.Maximum = PatternMax;
    PatternTab.Pattern[0].Num = 1;
    PatternTab.Pattern[0].CycleTimeH = 0;
    PatternTab.Pattern[0].CycleTimeL = 0;
    PatternTab.Pattern[0].OffsetTime = 0;
    PatternTab.Pattern[0].SplitNum = 1;
    PatternTab.Pattern[0].SequenceNum = 1;
    PatternTab.Pattern[0].WorkMode = FixedTime;
    
    PatternTab.Pattern[1].Num = 2;
    PatternTab.Pattern[1].CycleTimeH = 0;
    PatternTab.Pattern[1].CycleTimeL = 0;
    PatternTab.Pattern[1].OffsetTime = 0;
    PatternTab.Pattern[1].SplitNum = 1;
    PatternTab.Pattern[1].SequenceNum = 2;
    PatternTab.Pattern[1].WorkMode = FixedTime;
}


