#ifndef CHANNEL_H
#define CHANNEL_H
#include <stdint.h>


enum LampDrive_Type
{
    LD_BLACK        = 0, 
    LD_RED          = 0x0001, 
    LD_YELLOW       = 0x0002, 
    LD_REDYELLOW    = 0x0003, 
    LD_GREEN        = 0x0004, 
    LD_ALL          = 0x0007,
};

//控制类型CHANNEL_CONTROL_TYPE
typedef enum 
{
    CCT_OTHER = 1,      //其他
    CCT_VEHICLE,        //机动
    CCT_PEDESTRIAN,     //行人
    CCT_OVERLAP,        //跟随
    CCT_FLASH,
    CCT_GREEN,
    CCT_RED,
}CHANNEL_CONTROL_TYPE;  //通道控制类型

//闪光模式CHANNEL_FLASH_MODE
#define CFM_Yellow      ((uint8_t)0x02)     //黄闪
#define CFM_Red         ((uint8_t)0x04)     //红闪
#define CFM_Alternate   ((uint8_t)0x08)     //交替闪

//辉度模式CHANNEL_DIM_MODE
#define CDM_Green       ((uint8_t)0x01)     //绿灯
#define CDM_Yellow      ((uint8_t)0x02)     //黄灯
#define CDM_Red         ((uint8_t)0x04)     //红灯
#define CDM_Alternate   ((uint8_t)0x08)     //交替

//通道控制方位CHANNEL_POSITION_MODE
#define POS_Other       ((uint8_t)0x00)     //其他
#define POS_East        ((uint8_t)0x01)     //东
#define POS_South       ((uint8_t)0x02)     //南
#define POS_West        ((uint8_t)0x03)     //西
#define POS_North       ((uint8_t)0x04)     //北
#define POS_NorthEast   ((uint8_t)0x05)     //东北
#define POS_SouthEast   ((uint8_t)0x06)     //东南
#define POS_SouthWest   ((uint8_t)0x07)     //西南
#define POS_NorthWest   ((uint8_t)0x08)     //西北

//通道控制方向CHANNEL_DIRECTION_MODE
#define DIR_Other       ((uint8_t)0x00)     //其他
#define DIR_Left        ((uint8_t)0x01)     //左转
#define DIR_Straight    ((uint8_t)0x02)     //直行
#define DIR_Right       ((uint8_t)0x03)     //右转
#define DIR_Pedestrian  ((uint8_t)0x04)     //人行
#define DIR_Turn        ((uint8_t)0x05)     //掉头
#define DIR_Bicycle     ((uint8_t)0x06)     //自行车
//Left Straight Right Turn Pedestrian Bicycle


//手控方位
#define MANUAL_POS_Other       ((uint8_t)0x00)     //其他
#define MANUAL_POS_East        ((uint8_t)0x01)     //东
#define MANUAL_POS_South       ((uint8_t)0x02)     //南
#define MANUAL_POS_West        ((uint8_t)0x04)     //西
#define MANUAL_POS_North       ((uint8_t)0x08)     //北
#define MANUAL_POS_NorthEast   ((uint8_t)0x10)     //东北
#define MANUAL_POS_SouthEast   ((uint8_t)0x20)     //东南
#define MANUAL_POS_SouthWest   ((uint8_t)0x40)     //西南
#define MANUAL_POS_NorthWest   ((uint8_t)0x80)     //西北

//手控方向
#define MANUAL_DIR_Other       ((uint8_t)0x00)     //其他
#define MANUAL_DIR_Left        ((uint8_t)0x01)     //左转
#define MANUAL_DIR_Straight    ((uint8_t)0x02)     //直行
#define MANUAL_DIR_Right       ((uint8_t)0x04)     //右转
#define MANUAL_DIR_Pedestrian  ((uint8_t)0x08)     //人行
#define MANUAL_DIR_Turn        ((uint8_t)0x10)     //掉头
#define MANUAL_DIR_Bicycle     ((uint8_t)0x20)     //自行车


typedef struct
{
    uint8_t Num;            //通道号
    uint8_t ControlSource;  //控制源
    uint8_t ControlType;    //控制类型
    uint8_t Flash;          //闪光模式
    uint8_t Dim;            //辉度模式
    uint8_t Position;       //
    uint8_t Direction;      //
    uint8_t CountdownID;    //……考虑独立一张倒计时配置表
}ChannelType; //通道数据定义

typedef struct
{
    uint8_t Maximum;//最大通道设计数量32
    ChannelType Channel[32];
    uint8_t Reserve[15];
}ChannelTable;   //1+8*32+15 = 272 = 0x0110


/********************************************************/
typedef struct//每bit代表一个通道的对应颜色状态
{
    uint32_t     Reds;
    uint32_t     Yellows;
    uint32_t     Greens;
    uint32_t     Flash;     //1闪，0不闪
}ChannelStatusType;
/********************************************************/
typedef struct//每bit代表一个通道的对应颜色状态
{
    uint32_t     Reds;
    uint32_t     Yellows;
    uint32_t     Greens;
}ChannelReadStatusType;

//自定义通道状态参数表
typedef enum 
{
    BLACK = 0,
    GREEN,
    YELLOW,
    RED,
    RED_YELLOW,         //红黄同亮
    RED_YELLOW_FLASH,   //红黄交替闪
    GREEN_FLASH,
    YELLOW_FLASH,
    RED_FLASH,
}ChannelStateMode;
/********************************************************/


extern ChannelTable             ChannelTab;         //通道表
extern ChannelStatusType        ChannelStatus;      //通道状态表
extern ChannelReadStatusType    ChannelReadStatus;


void ChannelInit(void);
void AutoFlashMode(void);
void AutoAllRedMode(void);
void AutoLampOffMode(void);

uint8_t isVehPhase(uint8_t phaseNum);
uint8_t isPedPhase(uint8_t phaseNum);
uint32_t GetAppointChannel(uint8_t Pos, uint8_t Dir);

void LampControl(uint8_t tick10msCount);

#endif
