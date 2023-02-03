/*
*********************************************************************************************************
*
*	模块名称 : 相序模块
*	文件名称 : Sequence.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2019-12-30  wcx     首发
*
*********************************************************************************************************
*/
#include "public.h"
#include "Sequence.h"

//当前运行的 和 下一方案的 相序 数据 
uint8_t         RingStatus[RingMax];
SequenceType    SequenceNow;
SequenceTable   SeqTab;   //相序表


/*
    参数：相序指针
    功能：判断该相序有多少个相位
          相序的环号不合法，返回相位个数0
*/
uint8_t GetSeqMax(RingType* Ring)
{
    uint8_t i,phaseMax = 0;
    if(IsRing(Ring->RingNum))
    {
        for(i=0;i<PhaseMax;i++)
        {
            if(IsPhase(Ring->Phase[i]))
                phaseMax++;
            else
                break;
        }
    }

    return phaseMax;
}

void SequenceDefault(void)
{
    memset(&SeqTab.Maximum,0x00,sizeof(SeqTab));
    SeqTab.Maximum = SequenceMax;
    
    SeqTab.Seq[0].Num = 1;
    SeqTab.Seq[0].Ring[0].RingNum = 1;
    SeqTab.Seq[0].Ring[1].RingNum = 0;
    SeqTab.Seq[0].Ring[2].RingNum = 0;
    SeqTab.Seq[0].Ring[3].RingNum = 0;
    
    SeqTab.Seq[0].Ring[0].Phase[0] = 1;
    SeqTab.Seq[0].Ring[0].Phase[1] = 2;
    SeqTab.Seq[0].Ring[0].Phase[2] = 3;
    SeqTab.Seq[0].Ring[0].Phase[3] = 4;
    SeqTab.Seq[0].Ring[0].Phase[4] = 5;
    
    
    SeqTab.Seq[1].Num = 2;
    SeqTab.Seq[1].Ring[0].RingNum = 1;
    SeqTab.Seq[1].Ring[1].RingNum = 0;
    SeqTab.Seq[1].Ring[2].RingNum = 0;
    SeqTab.Seq[1].Ring[3].RingNum = 0;
    
    SeqTab.Seq[1].Ring[0].Phase[0] = 1;
    SeqTab.Seq[1].Ring[0].Phase[1] = 2;
    SeqTab.Seq[1].Ring[0].Phase[2] = 5;
}

//相序Sequence的默认配置
//只有一个环，环1放行5个相位
void SequenceXDataInit(SequenceType* Sequence)
{
    Sequence->Num = 1;
    Sequence->Ring[0].RingNum = 1;//环1有效
    Sequence->Ring[1].RingNum = 0;//只有1个环，其他环无效
    Sequence->Ring[2].RingNum = 0;
    Sequence->Ring[3].RingNum = 0;
    
    Sequence->Ring[0].Phase[0] = 1;
    Sequence->Ring[0].Phase[1] = 2;
    Sequence->Ring[0].Phase[2] = 3;
    Sequence->Ring[0].Phase[3] = 4;
    Sequence->Ring[0].Phase[4] = 5;
    Sequence->Ring[0].Phase[5] = 0;
}
