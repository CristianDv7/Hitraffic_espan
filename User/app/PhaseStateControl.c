#include "PhaseStateControl.h"
#include "Channel.h"
#include "Phase.h"
#include "Split.h"
#include "Overlap.h"

/* 当前运行的各个环的相位状态数据 */
PhaseStateType      PhaseState;

uint32_t GetPhaseNexts(void)
{
    uint32_t    PhaseNexts = 0;
    if(PhaseState.StateNum+1 < PhaseState.StateMax)
        PhaseStatus.PhaseNexts = PhaseState.State[PhaseState.StateNum+1];
    else
        PhaseStatus.PhaseNexts = PhaseState.State[0];
#if PhaseNextLog
    printf("PhaseOns = %04x, PhaseNexts = %04x\r\n",PhaseStatus.PhaseOns, PhaseStatus.PhaseNexts);
#endif
    return PhaseNexts;
}

/* 
 * 相位绿灯时间
 * 通过4个环的有效状态和相位号，放行时间，判断所有相位的状态
 * PhaseStatus
 */
void PhaseGreenCount(void)//1s运行一次
{
    uint8_t     i;
    uint32_t    PhaseMask = 0x1;
    
    for(i = 0; i < PhaseMax; i++)
    {
        if(PhaseStatus.Greens & PhaseMask)
        {
            PhaseTimes[i]++;
        }
        else PhaseTimes[i] = 0;
        PhaseMask <<= 1;
    }
}

/* 
 * 相位状态控制
 * 通过4个环的有效状态和相位号，放行时间，判断所有相位的状态
 * PhaseStatus
 */
void PhaseStatusControl(void)//1s运行一次
{
    uint8_t     i,PhaseNum,PhaseIndex;
    uint32_t    PhaseMask;
    
    PhaseStatus.Reds = 0;
    PhaseStatus.Yellows = 0;
    PhaseStatus.Greens = 0;
    PhaseStatus.VehClears = 0;
    PhaseStatus.DontWalks = 0;
    PhaseStatus.PedClears = 0;
    PhaseStatus.Walks = 0;
    PhaseStatus.PhaseOns = 0;
    
    for(i = 0; i < RingMax; i++)//4个环的相位号是否有当前通道的控制源
    {
        if(PhaseState.Ring[i].SeqMax == 0) continue;//该环未启用
        
        PhaseNum = RingPhase[i].PhaseNum;
        PhaseIndex = RingPhase[i].PhaseIndex;
        PhaseMask = (0x01 << (PhaseNum-1));
        PhaseStatus.PhaseOns |= PhaseMask;
        
        //机动状态算法
        if(PhaseState.Ring[i].SecondRemain > PhaseTab.Phase[PhaseIndex].YellowChange + PhaseTab.Phase[PhaseIndex].RedClear)//机动绿
        {
            PhaseStatus.Greens  |=  PhaseMask;
            PhaseStatus.Yellows &= ~PhaseMask;
            PhaseStatus.Reds    &= ~PhaseMask;
            
            if(PhaseState.Ring[i].SecondRemain > PhaseTab.Phase[PhaseIndex].VehicleClear + PhaseTab.Phase[PhaseIndex].YellowChange + PhaseTab.Phase[PhaseIndex].RedClear)
                PhaseStatus.VehClears &= ~PhaseMask;
            else //机动绿闪
                PhaseStatus.VehClears |=  PhaseMask;
        }
        else if(PhaseState.Ring[i].SecondRemain > PhaseTab.Phase[PhaseIndex].RedClear)//机动黄
        {
            PhaseStatus.Greens  &= ~PhaseMask;
            PhaseStatus.Yellows |=  PhaseMask;
            PhaseStatus.Reds    &= ~PhaseMask;
        }
        else//机动红
        {
            PhaseStatus.Reds        |=  PhaseMask;
            PhaseStatus.Greens      &= ~PhaseMask;
            PhaseStatus.Yellows     &= ~PhaseMask;
            PhaseStatus.VehClears   &= ~PhaseMask;
        }
        
        //行人状态算法
        if(PhaseTab.Phase[PhaseIndex].OptionsH & 0x20)//保持行人放行
        {
            if(PhaseStatus.Greens & PhaseMask)
            {
                if(PhaseStatus.VehClears & PhaseMask) //绿闪
                {
                    PhaseStatus.Walks       &= ~PhaseMask;
                    PhaseStatus.PedClears   |=  PhaseMask;
                    PhaseStatus.DontWalks   &= ~PhaseMask;
                }
                else
                {
                    PhaseStatus.Walks       |=  PhaseMask;
                    PhaseStatus.PedClears   &= ~PhaseMask;
                    PhaseStatus.DontWalks   &= ~PhaseMask;
                }
            }
            else if(PhaseStatus.Yellows & PhaseMask)
            {
                PhaseStatus.Walks       &= ~PhaseMask;
                PhaseStatus.PedClears   |=  PhaseMask;
                PhaseStatus.DontWalks   &= ~PhaseMask;
            }
            else if(PhaseStatus.Reds & PhaseMask)
            {
                PhaseStatus.Walks       &= ~PhaseMask;
                PhaseStatus.PedClears   &= ~PhaseMask;
                PhaseStatus.DontWalks   |=  PhaseMask;
            }
        }
        else //没有勾选保持行人放行
        {
            if(RingSplit[i].Time >= (PhaseTab.Phase[PhaseIndex].Walk + PhaseTab.Phase[PhaseIndex].PedestrianClear + PhaseTab.Phase[PhaseIndex].RedClear))
            {
                if((RingSplit[i].Time - PhaseState.Ring[i].SecondRemain) < PhaseTab.Phase[PhaseIndex].Walk) // <= 改为 < 会相差1秒
                {
                    PhaseStatus.Walks     |=  PhaseMask;
                    PhaseStatus.PedClears &= ~PhaseMask;
                    PhaseStatus.DontWalks &= ~PhaseMask;
                }
                else if((RingSplit[i].Time - PhaseState.Ring[i].SecondRemain) < (PhaseTab.Phase[PhaseIndex].Walk + PhaseTab.Phase[PhaseIndex].PedestrianClear))
                {
                    PhaseStatus.Walks     &= ~PhaseMask;
                    PhaseStatus.PedClears |=  PhaseMask;
                    PhaseStatus.DontWalks &= ~PhaseMask;
                }
                else
                {
                    PhaseStatus.Walks     &= ~PhaseMask;
                    PhaseStatus.PedClears &= ~PhaseMask;
                    PhaseStatus.DontWalks |=  PhaseMask;
                }
            }
            else //没有勾选保持行人放行时,设定的行人放行时间太大
            {
                if(PhaseState.Ring[i].SecondRemain > PhaseTab.Phase[PhaseIndex].PedestrianClear + PhaseTab.Phase[PhaseIndex].RedClear)
                {
                    PhaseStatus.Walks     |=  PhaseMask;
                    PhaseStatus.PedClears &= ~PhaseMask;
                    PhaseStatus.DontWalks &= ~PhaseMask;
                }
                else if(PhaseState.Ring[i].SecondRemain > PhaseTab.Phase[PhaseIndex].RedClear)
                {
                    PhaseStatus.Walks     &= ~PhaseMask;
                    PhaseStatus.PedClears |=  PhaseMask;
                    PhaseStatus.DontWalks &= ~PhaseMask;
                }
                else
                {
                    PhaseStatus.Walks     &= ~PhaseMask;
                    PhaseStatus.PedClears &= ~PhaseMask;
                    PhaseStatus.DontWalks |=  PhaseMask;
                }
            }
        }
    }
    PhaseGreenCount();
    PhaseStatus.PhaseNexts = GetPhaseNexts();
}

//跟随相位状态控制
void OverlapStatusControl(void) //1S刷新一次
{
    uint8_t i;
    uint32_t    OverlapMask = 0x1;
    for(i = 0; i < OverlapMax; i++)//遍历所有通道控制源
    {
        if(OverlapTab.Overlap[i].Num > 0 && OverlapTab.Overlap[i].Num <= OverlapMax)
        {
            OverlapMask = (0x1<<(OverlapTab.Overlap[i].Num - 1));
            if(OverlapTab.Overlap[i].Type == OT_NORMAL || (OverlapTab.Overlap[i].Type == OT_MINUSGREENYELLOW && ModifierPhases[i] == 0))//跟随相位类型为常规
            {
                //printf("Overlap[%d].Type = OT_NORMAL",i);
                //printf("IncludedPhases[i] = %d",IncludedPhases[i]);
                //printf("PhaseOns = %d",PhaseStatus.PhaseOns);
                //printf("Overlap[%d] is active ",i);
                //printf("Overlap[%d].Num = %d is active ",i,OverlapTab.Overlap[i].Num);
                
                if(IncludedPhases[i] & PhaseStatus.PhaseOns)
                {
                    if(OverlapTab.Overlap[i].TrailGreen == 0)//需要判断PhaseNexts是否也是母相位
                    {
                        //printf("PhaseNexts = %04x",PhaseStatus.PhaseNexts);
                        if(IncludedPhases[i] & PhaseStatus.PhaseNexts)
                        {
                            OverlapStatus.Greens    |= OverlapMask;
                            OverlapStatus.Yellows   &=~OverlapMask;
                            OverlapStatus.Reds      &=~OverlapMask;
                            OverlapStatus.Flashs    &=~OverlapMask;
                        }
                        else
                        {
                            if(IncludedPhases[i] & PhaseStatus.Greens)
                            {
                                OverlapStatus.Greens    |= OverlapMask;
                                OverlapStatus.Yellows   &=~OverlapMask;
                                OverlapStatus.Reds      &=~OverlapMask;
                            }
                            else if(IncludedPhases[i] & PhaseStatus.Yellows)
                            {
                                OverlapStatus.Yellows   |= OverlapMask;
                                OverlapStatus.Greens    &=~OverlapMask;
                                OverlapStatus.Reds      &=~OverlapMask;
                            }
                            else
                            {
                                OverlapStatus.Reds      |= OverlapMask;
                                OverlapStatus.Greens    &=~OverlapMask;
                                OverlapStatus.Yellows   &=~OverlapMask;
                            }
                            if(IncludedPhases[i] & PhaseStatus.VehClears)
                                OverlapStatus.Flashs    |= OverlapMask;
                            else
                                OverlapStatus.Flashs    &=~OverlapMask;
                        }
                    }
                    else//母相位结束时需继续放行绿灯的绿灯时间
                    {
                        if(IncludedPhases[i] & PhaseStatus.Greens)
                        {
                            OverlapCounter[i] = 1 + OverlapTab.Overlap[i].TrailGreen + OverlapTab.Overlap[i].TrailClear + OverlapTab.Overlap[i].TrailYellow + OverlapTab.Overlap[i].TrailRed;
                            OverlapStatus.Greens    |= OverlapMask;
                            OverlapStatus.Yellows   &=~OverlapMask;
                            OverlapStatus.Reds      &=~OverlapMask;
                            OverlapStatus.Flashs    &=~OverlapMask;                        
                        }
                        else if(OverlapCounter[i])
                        {
                            OverlapCounter[i]--;
                            if(OverlapCounter[i] > OverlapTab.Overlap[i].TrailClear + OverlapTab.Overlap[i].TrailYellow + OverlapTab.Overlap[i].TrailRed)
                            {
                                OverlapStatus.Greens    |= OverlapMask;
                                OverlapStatus.Yellows   &=~OverlapMask;
                                OverlapStatus.Reds      &=~OverlapMask;
                                OverlapStatus.Flashs    &=~OverlapMask;
                            }
                            else if(OverlapCounter[i] > OverlapTab.Overlap[i].TrailYellow + OverlapTab.Overlap[i].TrailRed)
                            {
                                OverlapStatus.Greens    |= OverlapMask;
                                OverlapStatus.Yellows   &=~OverlapMask;
                                OverlapStatus.Reds      &=~OverlapMask;
                                OverlapStatus.Flashs    |= OverlapMask;
                            }
                            else if(OverlapCounter[i] > OverlapTab.Overlap[i].TrailRed)
                            {
                                OverlapStatus.Greens    &=~OverlapMask;
                                OverlapStatus.Yellows   |= OverlapMask;
                                OverlapStatus.Reds      &=~OverlapMask;
                                OverlapStatus.Flashs    &=~OverlapMask;
                            }
                            else
                            {
                                OverlapStatus.Greens    &=~OverlapMask;
                                OverlapStatus.Yellows   &=~OverlapMask;
                                OverlapStatus.Reds      |= OverlapMask;
                                OverlapStatus.Flashs    &=~OverlapMask;
                            }
                        }
                    }
                }
                else//母相位关闭
                {
                    if(OverlapTab.Overlap[i].TrailGreen == 0)
                    {
                        OverlapStatus.Reds      |= OverlapMask;
                        OverlapStatus.Yellows   &=~OverlapMask;
                        OverlapStatus.Greens    &=~OverlapMask;
                    }
                    else
                    {
                        if(OverlapCounter[i])  
                        {
                            OverlapCounter[i]--;
                            if(OverlapCounter[i] > OverlapTab.Overlap[i].TrailClear + OverlapTab.Overlap[i].TrailYellow + OverlapTab.Overlap[i].TrailRed)
                            {
                                OverlapStatus.Greens    |= OverlapMask;
                                OverlapStatus.Yellows   &=~OverlapMask;
                                OverlapStatus.Reds      &=~OverlapMask;
                                OverlapStatus.Flashs    &=~OverlapMask;
                            }
                            else if(OverlapCounter[i] > OverlapTab.Overlap[i].TrailYellow + OverlapTab.Overlap[i].TrailRed)
                            {
                                OverlapStatus.Greens    |= OverlapMask;
                                OverlapStatus.Yellows   &=~OverlapMask;
                                OverlapStatus.Reds      &=~OverlapMask;
                                OverlapStatus.Flashs    |= OverlapMask;
                            }
                            else if(OverlapCounter[i] > OverlapTab.Overlap[i].TrailRed)
                            {
                                OverlapStatus.Greens    &=~OverlapMask;
                                OverlapStatus.Yellows   |= OverlapMask;
                                OverlapStatus.Reds      &=~OverlapMask;
                                OverlapStatus.Flashs    &=~OverlapMask;
                            }
                            else
                            {
                                OverlapStatus.Greens    &=~OverlapMask;
                                OverlapStatus.Yellows   &=~OverlapMask;
                                OverlapStatus.Reds      |= OverlapMask;
                                OverlapStatus.Flashs    &=~OverlapMask;
                            }
                        }
                    }
                }
            }
            else if(OverlapTab.Overlap[i].Type == OT_MINUSGREENYELLOW)
            {   //当母相位为绿色,修正相位不为绿色，那么跟随相位为绿色
                if((ModifierPhases[i] & PhaseStatus.Greens)==0)//当修正相位不为绿色
                {
                    if((IncludedPhases[i] & PhaseStatus.Greens))//当母相位为绿色
                    {
                        if(OverlapTab.Overlap[i].TrailGreen == 0)
                        {
                            OverlapStatus.Greens    |= OverlapMask;
                            OverlapStatus.Yellows   &=~OverlapMask;
                            OverlapStatus.Reds      &=~OverlapMask;
                            if(IncludedPhases[i] & PhaseStatus.PhaseNexts)
                            {
                                OverlapStatus.Flashs    &=~OverlapMask;
                            }
                            else
                            {
                                if(IncludedPhases[i] & PhaseStatus.VehClears)
                                    OverlapStatus.Flashs    |= OverlapMask;
                                else
                                    OverlapStatus.Flashs    &=~OverlapMask;
                            }
                        }
                        else
                        {
                            OverlapCounter[i] = 1 + OverlapTab.Overlap[i].TrailGreen + OverlapTab.Overlap[i].TrailClear + OverlapTab.Overlap[i].TrailYellow + OverlapTab.Overlap[i].TrailRed;
                            OverlapStatus.Greens    |= OverlapMask;
                            OverlapStatus.Yellows   &=~OverlapMask;
                            OverlapStatus.Reds      &=~OverlapMask;
                            OverlapStatus.Flashs    &=~OverlapMask;
                        }
                    }
                    //当母相位当中任何一个相位为黄色，下相位不为母相位，且修正相位不为黄色时，跟随相位为黄色。
                    else if((IncludedPhases[i] & PhaseStatus.Yellows))//当母相位为黄色
                    {
                        if((IncludedPhases[i] & PhaseStatus.PhaseNexts)==0 && (ModifierPhases[i] & PhaseStatus.Yellows)==0)
                        {
                            OverlapStatus.Greens    &=~OverlapMask;
                            OverlapStatus.Yellows   |= OverlapMask;
                            OverlapStatus.Reds      &=~OverlapMask;
                            OverlapStatus.Flashs    &=~OverlapMask;
                        }
                    }
                    else
                    {
                        if(OverlapTab.Overlap[i].TrailGreen>0 && OverlapCounter[i]>0)  
                        {
                            OverlapCounter[i]--;
                            if(OverlapCounter[i] > OverlapTab.Overlap[i].TrailClear + OverlapTab.Overlap[i].TrailYellow + OverlapTab.Overlap[i].TrailRed)
                            {
                                OverlapStatus.Greens    |= OverlapMask;
                                OverlapStatus.Yellows   &=~OverlapMask;
                                OverlapStatus.Reds      &=~OverlapMask;
                                OverlapStatus.Flashs    &=~OverlapMask;
                            }
                            else if(OverlapCounter[i] > OverlapTab.Overlap[i].TrailYellow + OverlapTab.Overlap[i].TrailRed)
                            {
                                OverlapStatus.Greens    |= OverlapMask;
                                OverlapStatus.Yellows   &=~OverlapMask;
                                OverlapStatus.Reds      &=~OverlapMask;
                                OverlapStatus.Flashs    |= OverlapMask;
                            }
                            else if(OverlapCounter[i] > OverlapTab.Overlap[i].TrailRed)
                            {
                                OverlapStatus.Greens    &=~OverlapMask;
                                OverlapStatus.Yellows   |= OverlapMask;
                                OverlapStatus.Reds      &=~OverlapMask;
                                OverlapStatus.Flashs    &=~OverlapMask;
                            }
                            else
                            {
                                OverlapStatus.Greens    &=~OverlapMask;
                                OverlapStatus.Yellows   &=~OverlapMask;
                                OverlapStatus.Reds      |= OverlapMask;
                                OverlapStatus.Flashs    &=~OverlapMask;
                            }
                        }
                        else
                        {
                            OverlapStatus.Greens    &=~OverlapMask;
                            OverlapStatus.Yellows   &=~OverlapMask;
                            OverlapStatus.Reds      |= OverlapMask;
                            OverlapStatus.Flashs    &=~OverlapMask;
                        }
                    }
                }
                else
                {
                    OverlapStatus.Greens    &=~OverlapMask;
                    OverlapStatus.Yellows   &=~OverlapMask;
                    OverlapStatus.Reds      |= OverlapMask;
                    OverlapStatus.Flashs    &=~OverlapMask;
                }
            }
        }
    }
}

//通道状态刷新
void ChannelStatusControl(void)//1S刷新一次
{
    uint8_t  i;
    uint32_t PhaseMask;
    uint32_t ChannelMask = 0x1;
    
    ChannelStatus.Reds      = 0;
    ChannelStatus.Yellows   = 0;
    ChannelStatus.Greens    = 0;
    ChannelStatus.Flash     = 0;
    
    for(i = 0; i < ChannelMax; i++)//遍历所有通道控制源
    {
        //相位号为0 或者 控制类型为其他 表示No Control (Not In Use)
        //if(ChannelTab.Channel[i].ControlSource == 0 || ChannelTab.Channel[i].ControlType == CCT_OTHER) continue;
        if(ChannelTab.Channel[i].ControlSource == 0)
        {
            ChannelMask <<= 1;
            continue;
        }
        PhaseMask = (0x1 << (ChannelTab.Channel[i].ControlSource-1));
        
        if(ChannelTab.Channel[i].ControlType == CCT_VEHICLE) //Vehicle控制类型为机动车
        {
            if((PhaseStatus.PhaseOns & PhaseMask) == 0) //该通道对应的控制源相位关闭
            {
                ChannelStatus.Reds      |= ChannelMask;
                ChannelStatus.Yellows   &=~ChannelMask;
                ChannelStatus.Greens    &=~ChannelMask;
                ChannelStatus.Flash     &=~ChannelMask;
            }
            else //该通道对应的控制源相位开启
            {
                if(PhaseStatus.Reds & PhaseMask) //相位状态为红
                {
                    ChannelStatus.Reds      |= ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;
                    ChannelStatus.Greens    &=~ChannelMask;
                    ChannelStatus.Flash     &=~ChannelMask;
                }
                else if(PhaseStatus.Yellows & PhaseMask) //相位状态为黄
                {
                    ChannelStatus.Yellows   |= ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Greens    &=~ChannelMask;
                    ChannelStatus.Flash     &=~ChannelMask;
                }
                else if(PhaseStatus.Greens & PhaseMask) //相位状态为绿
                {
                    ChannelStatus.Greens    |= ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;                
                    if(PhaseStatus.VehClears & PhaseMask)
                        ChannelStatus.Flash |= ChannelMask;
                    else
                        ChannelStatus.Flash &=~ChannelMask;
                }
                else //相位状态关闭
                {
                    ChannelStatus.Greens    &=~ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Flash     &=~ChannelMask;
                }
            }
        }
        else if(ChannelTab.Channel[i].ControlType == CCT_PEDESTRIAN)//Pedestrian
        {
            if((PhaseStatus.PhaseOns & PhaseMask) == 0)
            {
                ChannelStatus.Reds      |= ChannelMask;
                ChannelStatus.Yellows   &=~ChannelMask;
                ChannelStatus.Greens    &=~ChannelMask;
                ChannelStatus.Flash     &=~ChannelMask;
            }
            else
            {
                if(PhaseStatus.DontWalks & PhaseMask)//相位状态为红
                {
                    ChannelStatus.Reds      |= ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;
                    ChannelStatus.Greens    &=~ChannelMask;
                    ChannelStatus.Flash     &=~ChannelMask;
                }
                else if(PhaseStatus.PedClears & PhaseMask)//相位状态为绿闪
                {
                    ChannelStatus.Greens    |= ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;                
                    ChannelStatus.Flash     |= ChannelMask;
                }
                else if(PhaseStatus.Walks & PhaseMask)//相位状态为绿
                {
                    ChannelStatus.Greens    |= ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Flash     &=~ChannelMask;
                }
            }
        }
        else if(ChannelTab.Channel[i].ControlType == CCT_OVERLAP)
        {
            if(OverlapStatus.Reds & PhaseMask)//相位状态为红
            {
                ChannelStatus.Reds      |= ChannelMask;
                ChannelStatus.Yellows   &=~ChannelMask;
                ChannelStatus.Greens    &=~ChannelMask;
                ChannelStatus.Flash     &=~ChannelMask;
            }
            else if(OverlapStatus.Yellows & PhaseMask)//相位状态为黄
            {
                ChannelStatus.Yellows   |= ChannelMask;
                ChannelStatus.Reds      &=~ChannelMask;
                ChannelStatus.Greens    &=~ChannelMask;
                ChannelStatus.Flash     &=~ChannelMask;
            }
            else if(OverlapStatus.Greens & PhaseMask)//相位状态为绿
            {
                ChannelStatus.Greens    |= ChannelMask;
                ChannelStatus.Yellows   &=~ChannelMask;  
                ChannelStatus.Reds      &=~ChannelMask;
                if(OverlapStatus.Flashs & PhaseMask)
                    ChannelStatus.Flash |= ChannelMask;
                else
                    ChannelStatus.Flash &=~ChannelMask;
            }
            else//相位状态关闭
            {
                ChannelStatus.Greens    &=~ChannelMask;
                ChannelStatus.Yellows   &=~ChannelMask;
                ChannelStatus.Reds      |= ChannelMask;
                ChannelStatus.Flash     &=~ChannelMask;
            }
        }
        else if(ChannelTab.Channel[i].ControlType == CCT_GREEN)
        {
            ChannelStatus.Greens    |= ChannelMask;
            ChannelStatus.Reds      &=~ChannelMask;
            ChannelStatus.Yellows   &=~ChannelMask;
            ChannelStatus.Flash     &=~ChannelMask;
        }
        else if(ChannelTab.Channel[i].ControlType == CCT_RED)
        {
            ChannelStatus.Reds      |= ChannelMask;
            ChannelStatus.Greens    &=~ChannelMask;
            ChannelStatus.Yellows   &=~ChannelMask;
            ChannelStatus.Flash     &=~ChannelMask;
        }
        else if(ChannelTab.Channel[i].ControlType == CCT_FLASH)
        {
            if(ChannelTab.Channel[i].Flash == CFM_Yellow)          //黄闪
            {
                ChannelStatus.Yellows |= ChannelMask;
            }
            else if(ChannelTab.Channel[i].Flash == CFM_Red)        //红闪
            {
                ChannelStatus.Reds |= ChannelMask;
            }
            else if(ChannelTab.Channel[i].Flash == CFM_Alternate)  //交替
            {
                ChannelStatus.Reds |= ChannelMask;
                ChannelStatus.Yellows |= ChannelMask;
            }
            ChannelStatus.Flash |= ChannelMask;
        }
        else if(ChannelTab.Channel[i].ControlType == CCT_OTHER)
        {
            if((PhaseStatus.PhaseOns & PhaseMask) == 0)
            {
                ChannelStatus.Reds      |= ChannelMask;
                ChannelStatus.Yellows   &=~ChannelMask;
                ChannelStatus.Greens    &=~ChannelMask;
                ChannelStatus.Flash     &=~ChannelMask;
            }
            else
            {
                if(PhaseStatus.Reds & PhaseMask)//相位状态为红
                {
                    ChannelStatus.Reds      |= ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;
                    ChannelStatus.Greens    &=~ChannelMask;
                    ChannelStatus.Flash     &=~ChannelMask;
                }
                else if(PhaseStatus.Yellows & PhaseMask)//相位状态为黄
                {
                    ChannelStatus.Yellows   |= ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Greens    &=~ChannelMask;
                    ChannelStatus.Flash     &=~ChannelMask;
                }
                else if(PhaseStatus.Greens & PhaseMask)//相位状态为绿
                {
                    ChannelStatus.Greens    |= ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;                
                    if(PhaseStatus.VehClears & PhaseMask)
                        ChannelStatus.Flash |= ChannelMask;
                    else
                        ChannelStatus.Flash &=~ChannelMask;
                }
                else//相位状态关闭
                {
                    ChannelStatus.Greens    &=~ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Flash     &=~ChannelMask;
                }
            }
        }
        ChannelMask <<= 1;
    }
}

void ChannelStatusToLmap(void)
{
    LampControl(PhaseState.Phase10msCount); 
}





