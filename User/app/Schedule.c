/*
*********************************************************************************************************
*
*	模块名称 : 调度计划模块
*	文件名称 : Schedule.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2019-12-30  wcx     首发
*
*********************************************************************************************************
*/
#include "public.h"
#include "Schedule.h"
//#include "bsp.h"




ScheduleType     ScheduleNow;      //当前运行的调度计划
ScheduleTable    ScheduleTab;   //调度计划表



void ScheduleDefault(void)
{
    memset(&ScheduleTab.Maximum,0x00,sizeof(ScheduleTab));
    
    ScheduleTab.Maximum = ScheduleMax;
    ScheduleTab.Schedule[0].Num = 1;
    ScheduleTab.Schedule[0].PlanNum = 1;
    ScheduleTab.Schedule[0].Date.MonthH = 0x1f;			// 0001 1111
    ScheduleTab.Schedule[0].Date.MonthL = 0xfe;			// 1111 1110
    ScheduleTab.Schedule[0].Date.Date[0] = 0xfe;		// 1111 1111
    ScheduleTab.Schedule[0].Date.Date[1] = 0xff;		// 1111 1111
    ScheduleTab.Schedule[0].Date.Date[2] = 0xff;		// 1111 1111
    ScheduleTab.Schedule[0].Date.Date[3] = 0xff;
    ScheduleTab.Schedule[0].Date.Day = 0xfe;				// 1111 1110
}

/*
 *  通过日期查询，调度计划表，获取符合条件的调度计划表索引
 *  
 *  
*/
uint8_t GetScheduleIndex(ScheduleTable* ScheduleTab, DateType* Date)
{
    uint8_t  i;
    uint8_t  temp = 0xff;
    uint16_t SchMonth,NowMonth;
    uint32_t SchDate, NowDate;
    
    NowMonth = (Date->MonthH<<8) | Date->MonthL;
    NowDate  = (Date->Date[3]<<24)|(Date->Date[2]<<16)|(Date->Date[1]<<8)|Date->Date[0];
    
#if DEBUG 
    printf("Month = %x, Date = %x \r\n",NowMonth, NowDate);
#endif
    
    for(i = 0; i < ScheduleMax; i++)
    {
        if(ScheduleTab->Schedule[i].Num > 0 && ScheduleTab->Schedule[i].PlanNum > 0)
        {
            SchMonth = (ScheduleTab->Schedule[i].Date.MonthH<<8)  | \
                        ScheduleTab->Schedule[i].Date.MonthL;
            SchDate  = (ScheduleTab->Schedule[i].Date.Date[3]<<24)| \
                       (ScheduleTab->Schedule[i].Date.Date[2]<<16)| \
                       (ScheduleTab->Schedule[i].Date.Date[1]<<8 )| \
                        ScheduleTab->Schedule[i].Date.Date[0];
#if DEBUG 
            printf("SchMonth = %x, SchDate = %x \r\n", SchMonth, SchDate);
#endif
            
            if((SchMonth & NowMonth) && (SchDate & NowDate) && (ScheduleTab->Schedule[i].Date.Day & Date->Day))//满足即确认
            {
                temp = i;
                break;
                //if(NowMonth == SchMonth || SchMonth != SM_ALL)  //指定月份为特殊日 优先级最高
            }
        }
    }
    return temp;
}



