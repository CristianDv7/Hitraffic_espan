/*
*********************************************************************************************************
*
*	模块名称 : 通道模块
*	文件名称 : Channel.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2019-12-30  wcx     首发
*
*********************************************************************************************************
*/
#include "Channel.h"
#include "bsp_io.h"
#include "public.h"

ChannelTable            ChannelTab;     //通道表
ChannelStatusType       ChannelStatus;  //通道状态表     逐步舍弃自己定ChannelStateTab
ChannelReadStatusType   ChannelReadStatus;

//默认的通道配置
void ChannelInit(void)
{
    uint8_t i,groupNum,boardNum;
    ChannelTab.Maximum = ChannelMax;       //最大通道设计数量32
    for(i = 0; i < ChannelTab.Maximum; i++)
    {
        ChannelTab.Channel[i].Num = i + 1;
        groupNum = i%4;
        boardNum = i/4;
        if(groupNum <= 2)
        {
            ChannelTab.Channel[i].ControlSource = boardNum + 1;   //相位号
            ChannelTab.Channel[i].ControlType = CCT_VEHICLE;      //控制类型
            ChannelTab.Channel[i].Flash = CFM_Yellow;             //闪光模式
            ChannelTab.Channel[i].Dim = CDM_Green;                //辉度模式
        }
        else
        {
            ChannelTab.Channel[i].ControlSource = 5;              //相位号
            ChannelTab.Channel[i].ControlType = CCT_PEDESTRIAN;   //控制类型
            ChannelTab.Channel[i].Flash = CFM_Red;                //闪光模式
            ChannelTab.Channel[i].Dim = CDM_Green;                //辉度模式
        }
        ChannelTab.Channel[i].Position = boardNum + 1;
        ChannelTab.Channel[i].Direction = groupNum + 1;
        ChannelTab.Channel[i].CountdownID = i;
    }
}

uint8_t isPedPhase(uint8_t phaseNum)
{
    uint8_t i;
    for(i = 0; i < ChannelTab.Maximum; i++)
    {
        //控制源相同且为行人相位
        if(ChannelTab.Channel[i].ControlSource == phaseNum && 
           ChannelTab.Channel[i].ControlType == CCT_PEDESTRIAN)
        {
            return 1;
        }
    }
    return 0;
}

uint8_t isVehPhase(uint8_t phaseNum)
{
    uint8_t i;
    for(i = 0; i < ChannelTab.Maximum; i++)
    {
        //控制源相同且为行人相位
        if(ChannelTab.Channel[i].ControlSource == phaseNum && 
           ChannelTab.Channel[i].ControlType == CCT_VEHICLE)
        {
            return 1;
        }
    }
    return 0;
}

uint8_t isVehChannel(uint8_t channelNum)
{
    if(ChannelTab.Channel[channelNum].ControlSource > 0 && ChannelTab.Channel[channelNum].ControlType == CCT_VEHICLE)
    {
        return 1;
    }
    return 0;
}



uint32_t GetAppointChannel(uint8_t Pos, uint8_t Dir)
{
    uint8_t i;
    uint32_t ChannelMask = 0x1;
    uint32_t AppointChannel = 0;

    for(i = 0; i < ChannelTab.Maximum; i++)
    {
        if(ChannelTab.Channel[i].ControlSource != 0)//通道启用
        {
            if(ChannelTab.Channel[i].Position > 0 && ChannelTab.Channel[i].Direction > 0)
            {
                if(((0x01 << (ChannelTab.Channel[i].Position-1))&Pos) && ((0x01 << (ChannelTab.Channel[i].Direction-1))&Dir))//与指定放行相同
                {
                    AppointChannel |= ChannelMask;
                }
            }
        }
        ChannelMask <<= 1;
    }
    return AppointChannel;
}




//闪光模式输出
void AutoFlashMode(void)
{
    uint8_t i;
    uint32_t ChannelMask = 0x1;
    ChannelStatus.Greens = 0;
    ChannelStatus.Yellows = 0;
    ChannelStatus.Reds = 0;
    ChannelStatus.Flash = 0;
    for(i = 0; i < ChannelMax; i++)
    {
        if(ChannelTab.Channel[i].ControlSource != 0)
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
        ChannelMask <<= 1;
    }
}

void AutoAllRedMode(void)
{
    uint8_t i;
    uint32_t ChannelMask = 0x1;
    ChannelStatus.Greens = 0;
    ChannelStatus.Yellows = 0;
    ChannelStatus.Reds = 0;
    ChannelStatus.Flash = 0;
    for(i = 0; i < ChannelMax; i++)
    {
        if(ChannelTab.Channel[i].ControlSource != 0)
        {
            ChannelStatus.Reds |= ChannelMask;
        }
        ChannelMask <<= 1;
    }
}

void AutoLampOffMode(void)
{
    ChannelStatus.Greens = 0;
    ChannelStatus.Yellows = 0;
    ChannelStatus.Reds = 0;
    ChannelStatus.Flash = 0;
}


//通道状态刷新到驱动端
enum LampDrive_Type ChannelStatusToDrivereg(uint32_t ChannelMask, uint8_t tick10msCount)
{
    enum LampDrive_Type temp;
    if(ChannelStatus.Flash & ChannelMask)
    {
        if(ChannelStatus.Greens & ChannelMask)
        {
            if(tick10msCount < 50)
                temp = LD_BLACK;
            else
                temp = LD_GREEN;
        }
        else if(ChannelStatus.Yellows & ChannelStatus.Reds & ChannelMask)
        {
            if(tick10msCount < 50)
                temp = LD_YELLOW;
            else
                temp = LD_RED;
        }
        else if(ChannelStatus.Reds & ChannelMask)
        {
            if(tick10msCount < 50)
                temp = LD_BLACK;
            else
                temp = LD_RED;
        }
        else //if(ChannelStatus.Yellows & ChannelMask)//有闪灯标志,未职位灯色,默认黄灯
        {
            if(tick10msCount < 50)
                temp = LD_BLACK;
            else
                temp = LD_YELLOW;
        }
    }
    else
    {
        if(ChannelStatus.Greens & ChannelMask)
        {
            temp = LD_GREEN;
        }
        else if(ChannelStatus.Yellows & ChannelMask)
        {
            temp = LD_YELLOW;
        }
        else if(ChannelStatus.Reds & ChannelMask)
        {
            temp = LD_RED;
        }
        else temp = LD_BLACK;
    }
    
    return temp;
}

void LampControl(uint8_t tick10msCount)//10ms刷新一次
{
    uint8_t i;
    uint32_t ChannelMask = 0X1;
    enum LampDrive_Type temp[4];
    
    for(i = 0; i < ChannelMax/4; i++)//遍历所有通道控制源
    {
        temp[0] = ChannelStatusToDrivereg(ChannelMask, tick10msCount);
        ChannelMask <<= 1;
        temp[1] = ChannelStatusToDrivereg(ChannelMask, tick10msCount);
        ChannelMask <<= 1;
        temp[2] = ChannelStatusToDrivereg(ChannelMask, tick10msCount);
        ChannelMask <<= 1;
        temp[3] = ChannelStatusToDrivereg(ChannelMask, tick10msCount);
        ChannelMask <<= 1;
        
        LampDriveReg[i].drivereg.group1 = temp[0];
        LampDriveReg[i].drivereg.group2 = temp[1];
        LampDriveReg[i].drivereg.group3 = temp[2];
        LampDriveReg[i].drivereg.group4 = temp[3];
        LampDriveReg[i].drivereg.run = (tick10msCount < 3)?1:0;
    }
    
    LampDriveOut();
}

