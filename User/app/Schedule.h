#ifndef SCHEDULE_H
#define SCHEDULE_H
#include "public.h"


//timeBaseScheduleMonth
#define SM_Reserved     ((uint16_t)0x0001)  //保留 
#define SM_January      ((uint16_t)0x0002)  //1
#define SM_February     ((uint16_t)0x0004)  //2
#define SM_March        ((uint16_t)0x0008)  //3
#define SM_April        ((uint16_t)0x0010)  //4
#define SM_May          ((uint16_t)0x0020)  //5
#define SM_June         ((uint16_t)0x0040)  //6
#define SM_July         ((uint16_t)0x0080)  //7
#define SM_August       ((uint16_t)0x0100)  //8
#define SM_September    ((uint16_t)0x0200)  //9
#define SM_October      ((uint16_t)0x0400)  //10
#define SM_November     ((uint16_t)0x0800)  //11
#define SM_December     ((uint16_t)0x1000)  //12
#define SM_ALL          ((uint16_t)0x1FFE)  //12

//timeBaseScheduleDay
#define SD_Reserved         ((uint8_t)0x01)  //保留 
#define SD_Sunday           ((uint8_t)0x02)  // 
#define SD_Monday           ((uint8_t)0x04)  // 
#define SD_Tuesday          ((uint8_t)0x08)  // 
#define SD_Wednesday        ((uint8_t)0x10)  // 
#define SD_Thursday         ((uint8_t)0x20)  // 
#define SD_Friday           ((uint8_t)0x40)  // 
#define SD_Saturday         ((uint8_t)0x80)  // 

//timeBaseScheduleDate
#define SD_Day_Reserved     ((uint32_t)0x00000001)  //保留 
#define SD_Day_1            ((uint32_t)0x00000002)
#define SD_Day_2            ((uint32_t)0x00000004)
#define SD_Day_3            ((uint32_t)0x00000008)
#define SD_Day_4            ((uint32_t)0x00000010)
#define SD_Day_5            ((uint32_t)0x00000020)
#define SD_Day_6            ((uint32_t)0x00000040)
#define SD_Day_7            ((uint32_t)0x00000080)
#define SD_Day_8            ((uint32_t)0x00000100)
#define SD_Day_9            ((uint32_t)0x00000200)
#define SD_Day_10           ((uint32_t)0x00000400)
#define SD_Day_11           ((uint32_t)0x00000800)
#define SD_Day_12           ((uint32_t)0x00001000)
#define SD_Day_13           ((uint32_t)0x00002000)
#define SD_Day_14           ((uint32_t)0x00004000)
#define SD_Day_15           ((uint32_t)0x00008000)
#define SD_Day_16           ((uint32_t)0x00010000)
#define SD_Day_17           ((uint32_t)0x00020000)
#define SD_Day_18           ((uint32_t)0x00040000)
#define SD_Day_19           ((uint32_t)0x00080000)
#define SD_Day_20           ((uint32_t)0x00100000)
#define SD_Day_21           ((uint32_t)0x00200000)
#define SD_Day_22           ((uint32_t)0x00400000)
#define SD_Day_23           ((uint32_t)0x00800000)
#define SD_Day_24           ((uint32_t)0x01000000)
#define SD_Day_25           ((uint32_t)0x02000000)
#define SD_Day_26           ((uint32_t)0x04000000)
#define SD_Day_27           ((uint32_t)0x08000000)
#define SD_Day_28           ((uint32_t)0x10000000)
#define SD_Day_29           ((uint32_t)0x20000000)
#define SD_Day_30           ((uint32_t)0x40000000)
#define SD_Day_31           ((uint32_t)0x80000000)

typedef struct
{
    uint8_t MonthL;
    uint8_t MonthH;     //1,2       bit0保留 bit 1-12 月
    uint8_t Day;        //3         bit0保留 bit 1-7  周天-周六
    uint8_t Date[4];    //4,5,6,7   bit0保留 bit 1-31 日
}DateType;

typedef struct
{
    uint8_t     Num;        //1      调度计划编号
    DateType    Date;       //2-8    调度计划日期数据
    uint8_t     PlanNum;    //9      时段表号
}ScheduleType; //调度计划定义

typedef struct
{
    uint8_t Maximum;
    ScheduleType Schedule[ScheduleMax];   //40 * 9
    uint8_t Reserve[7];
}ScheduleTable; //调度计划表      8 + 40*9 = 368 = 0x0170




extern ScheduleType     ScheduleNow;
extern ScheduleTable    ScheduleTab;   //调度计划表



void ScheduleDefault(void);
uint8_t GetScheduleIndex(ScheduleTable* ScheduleTab, DateType* Date);


#endif
