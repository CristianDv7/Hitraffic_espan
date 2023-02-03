/*
*********************************************************************************************************
*
*	模块名称 : 绿信比模块
*	文件名称 : Split.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2019-12-30  wcx     首发
*
*********************************************************************************************************
*/
#include "public.h"
#include "Split.h"


SplitType       SplitNow;   //当前运行的 绿信比 数据
SplitTable      SplitTab;   //绿信比表
PhaseSplitType  RingSplit[RingMax];    //各环当前绿信比




//相位号 + 绿信比数据 = 绿信比相位索引
uint8_t GetSplitPhaseIndex(SplitType* Split, uint8_t PhaseNum)
{
    uint8_t     i;
    uint8_t     temp = 0xff;
    
    for(i = 0; i < PhaseMax; i++)
    {
        if(Split->Phase[i].PhaseNum == PhaseNum)
        {
            temp = i;
            break;
        }
    }
    
    return temp;
}

void SplitDefault(void)
{
    uint8_t i;
    
    memset(&SplitTab.Maximum,0x00,sizeof(SplitTab));
    SplitTab.Maximum = SplitMax;

    SplitTab.Split[0].Num = 1;
    for(i = 0; i <= 4; i++)//默认配置5个相位
    {
        SplitTab.Split[0].Phase[i].PhaseNum = i + 1;//
        SplitTab.Split[0].Phase[i].Time = 16;       //相位的放行时间。包含绿灯、绿闪、黄灯、全红时间
        SplitTab.Split[0].Phase[i].Mode = SM_None;  //绿信比模式 2-无
        SplitTab.Split[0].Phase[i].Coord = SC_FIXED;//协调配置Coord bit2：1-作为固定相位
    }
}

//默认的绿信比配置
//绿信比1的默认配置
void SplitDataInit(uint8_t n)
{
    uint8_t i;
    SplitTab.Split[n].Num = n + 1;
    for(i = 0; i <= 4; i++)//默认配置5个相位
    {
        SplitTab.Split[n].Phase[i].PhaseNum = i + 1;//
        SplitTab.Split[n].Phase[i].Time = 18;       //相位的放行时间。包含绿灯、绿闪、黄灯、全红时间
        SplitTab.Split[n].Phase[i].Mode = SM_None;  //绿信比模式 2-无
        SplitTab.Split[n].Phase[i].Coord = SC_FIXED;//协调配置Coord bit2：1-作为固定相位
    }
}

void SplitXDataInit(SplitType* Split)
{
    uint8_t n;
    Split->Num = 1;
    for(n=0;n<=4;n++)
    {
        Split->Phase[n].PhaseNum = n + 1;//
        Split->Phase[n].Time = 18;      //相位的放行时间。包含了机动车相位的绿灯、绿闪、黄灯、全红时间
                                            //以及行人相位的放行时间和清空时间。
        Split->Phase[n].Mode = SM_None;  //绿信比模式 2-无
        Split->Phase[n].Coord = SC_FIXED;//协调配置Coord bit2：1-作为固定相位
    }
}

