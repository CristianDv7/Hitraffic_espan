/*
*********************************************************************************************************
*
* Nombre del módulo: Módulo detector de peatones
* Nombre del archivo: Peddet.c * Versión: V1.0
* illustrate : * Modification record: 
* Version Number Date Author Description 
* V1.0 2019-12-30 wcx first launch 
*
*********************************************************************************************************
*/
#include "public.h"
#include "Peddet.h"
#include "bsp_io.h"
#include "tsc.h"

PeddetTable  PeddetTab;  //Mesa detectora de peatones
PeddetStateTable PeddetStateTab;

void PeddetInit(void)
{
    memset(&PeddetTab.Maximum,0x00,sizeof(PeddetTab));
    PeddetTab.Maximum = PeddetMax;
}

void PeddetStateInit(void)
{
    memset(&PeddetStateTab.peddet[0].Valid,0x00,sizeof(PeddetStateTab));
    PeddetStateTab.Maximum = PeddetMax;
}

void PeddetStateGet(void)//Ciclo de ejecución 1s
{
	uint8_t i;
    static uint8_t reg60s = 0;

    if((peddet_hw.rising | peddet_hw.failing) & 0x80)
    {
        peddet_hw.rising &= 0x7f;
        peddet_hw.failing &= 0x7f;
        OP.SendDoorAlarm = 1;
    }
    for(i = 0; i < 8; i++)
    {
        if(PeddetStateTab.peddet[i].Valid) //tener señal
        {
            if(PeddetStateTab.peddet[i].Presence < 0xffff)PeddetStateTab.peddet[i].Presence++;
        }
        else //sin señal
        {
            if(PeddetStateTab.peddet[i].NoActivity < 0xffff)PeddetStateTab.peddet[i].NoActivity++;
        }
    }
    
    if(++reg60s >= 60)//Revisar una vez en 60s
    {
        reg60s = 0;
        for(i = 0; i < 8; i++)
        {
            if(PeddetTab.peddet[i].NoActivity)
            {
                if(PeddetStateTab.peddet[i].NoActivity >= PeddetTab.peddet[i].NoActivity * 60)
                {
                    PeddetStateTab.peddet[i].Alarms |= PD_NoActivityFault;
                }
                else 
                {
                    PeddetStateTab.peddet[i].Alarms &= ~PD_NoActivityFault;
                }
            }
            
            if(PeddetTab.peddet[i].MaxPresence)
            {
                if(PeddetStateTab.peddet[i].Presence >= PeddetTab.peddet[i].MaxPresence * 60)
                {
                    PeddetStateTab.peddet[i].Alarms |= PD_MaxPresenceFault;
                }
                else 
                {
                    PeddetStateTab.peddet[i].Alarms &= ~PD_MaxPresenceFault;
                }
            }
            
            if(PeddetTab.peddet[i].Erratic)
            {
                if(PeddetStateTab.peddet[i].ErraticCounts > PeddetTab.peddet[i].Erratic)
                {
                    PeddetStateTab.peddet[i].Alarms |= PD_ErraticOutputFault;
                }
                else 
                {
                    PeddetStateTab.peddet[i].Alarms &= ~PD_ErraticOutputFault;
                }
            }
            PeddetStateTab.peddet[i].ErraticCounts = 0;
        }
    }
}










