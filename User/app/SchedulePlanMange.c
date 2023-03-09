#include "Schedule.h"
#include "Plan.h"

/*
 * Función: obtenga el número de horario consultando el horario de programación, juzgue si el horario ha cambiado y actualícelo directamente si hay un cambio
 * Devolución: No hay plan disponible, devuelve 0, de lo contrario, devuelve 1
 * Frecuencia de ejecución: se ejecuta al inicio, se ejecuta a las cero en punto todos los días, se ejecuta cuando se modifica la fecha y la hora y se ejecuta cuando se modifica la programación
 * 
 */
uint8_t SchedulePlanRefresh(ScheduleTable* ScheduleTab, DateType* Date)
{
    uint8_t temp = GetScheduleIndex(ScheduleTab, Date);   //fecha + horario = número de horario
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
