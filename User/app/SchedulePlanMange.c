#include "Schedule.h"
#include "Plan.h"

/*
 * 功能：通过查询调度计划表，获取时段表号，判断时段表是否改变，有改变直接更新
 * 返回：无Plan可用，返回0，否则返回1
 * 执行频率：开机时运行，每天零点运行，日期时间修改时运行，调度计划表修改时运行
 * 
 */
uint8_t SchedulePlanRefresh(ScheduleTable* ScheduleTab, DateType* Date)
{
    uint8_t temp = GetScheduleIndex(ScheduleTab, Date);   //日期 + 调度计划表 = 时段表号
    #if DEBUG 
    printf("ScheduleIndex = %d \r\n",temp);
    #endif
    if(temp < ScheduleMax)
    {
        if(memcmp(&ScheduleNow, &ScheduleTab->Schedule[temp], sizeof(ScheduleNow)) != 0)
        {
            #if DEBUG 
            printf("Schedule Changed \r\n");
            #endif
            memcpy(&ScheduleNow, &ScheduleTab->Schedule[temp], sizeof(ScheduleNow));
        }
        
        if(ScheduleNow.PlanNum != Plan.Num)
        {
            #if DEBUG 
            printf("Plan Changed \r\n");
            #endif
            memcpy(&Plan, &PlanTab.Plan[ScheduleNow.PlanNum - 1], sizeof(Plan));
            return 1;
        }
    }
    return 0;
}
