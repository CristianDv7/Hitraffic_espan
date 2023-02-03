/*
*********************************************************************************************************
*
* Module name: signal machine operation control 
* File name: tsc.c 
* Version : V1.0 
* illustrate : 
* Modification record: 
* Version Number Date Author Description 
* V1.0 2019-12-04 wcx first launch
*
*********************************************************************************************************
*/

#include "bsp.h" 				/* Low-level hardware driver */
#include "public.h"
#include "SchedulePlanMange.h"
#include "PhaseStateControl.h"

/*
     Analisis de enlace de datos:
     Horario de programacion + fecha numero de franja horaria
     Horario de franjas horarias + tiempo → número de acción
     Lista de Acciones → Número de Esquema + Auxiliar + Especial
     Tabla de esquemas → número de fase + número de relación de se?al verde + diferencia de fase + (ciclo calculado) + (+ ?modo?)
     Tabla de secuencia de fases → anillo en ejecución + secuencia de fase en ejecución (no realice una clasificación obligatoria, realice una selección de fase única)
     Tabla de relación de letras verdes → cada fase tiempo + modo + coordinación
     Tabla de fases → Límite de tiempo de fase + Parámetros de detección + Estrategia para peatones + Opciones de configuración
     Tabla de canales → Tipo de grupo de luces + Fuente de control + Flash + Brillo
    
     Análisis de cambio de datos:
     1. Cambio de fecha: analiza si el número de horario ha cambiado → se?al de cambio de horario
     2. Cambio de hora y minutos: analiza si el número de acción ha cambiado → signo de cambio de acción
     3. Cambio de acción: determina si el contenido de la acción ha cambiado →
    
     4. Si hay un cambio en los datos de la tabla de fecha y hora, los nuevos datos se aplican directamente, porque estos cambios de datos no causarán un cambio repentino en el control de la se?al;
     5. Los cambios de parámetros del canal surten efecto inmediatamente
    
     Análisis de la informacion almacenada en la memoria de la máquina de se?ales:
     1. Horario de programacion Todo
     2. Horario 1
     3. Mesa de accion 1
     4. Una tabla de programas
     5. Tabla de secuencia de fases 1
     6. Relación de letras verdes 1
     7. Tabla de fases Todo
     8. Tabla de canales Todos
    
     Análisis del proceso de operación de la máquina de se?ales:
     1. Al comienzo de cada día y cuando la máquina de se?ales se reinicie, lea la fecha, juzgue la precisión de la fecha, verifique el cronograma de envío, juzgue la prioridad y obtenga el número de la tabla de intervalos de tiempo;
     2. Obtener datos del horario
*/

/* Basic data structure definition */
OPType              OP;
RtcType             Rtc;        //Current RTC time
DateType            Date;       //Fecha NTCIP actual
TimeType            Time;       //Hora actual de NTCIP
SeqTimeType         SeqTime;

MANUAL_CTRL_TYPE    ManualCtrl;



void GetPhaseStateSeqMax(void);
void GreenWaveTimeControl(void);
void GreenWaveTimeChange(void);






void RtcIrqCallback(void)
{
    PhaseState.Phase1sFlag = 1;
    OP.Seconds++;
    if((OP.Seconds%60)==0) OP.RtcIrqFlag = 1;
    #if RTCIRQ 
    printf("E5_IRQ \r\n"); 
    #endif
}

void ReadRealTime(void)
{
    static RtcType rtc;
    memcpy(&rtc, &Rtc, sizeof(rtc));	//backup one second ago
    
    Rtc.second++;
    if(OP.RtcIrqFlag == 1 || OP.StartFlag == 1 || OP.TimeChangeFlag == 1)
    {
        RtcReadTime();  //Read RTC time
        OP.Seconds = (Time.Hour * 3600) + (Time.Minute * 60) + Rtc.second;
        if(OP.StartFlag) OP.StartFlag = 0;
        if(OP.RtcIrqFlag) OP.RtcIrqFlag = 0;
        if(OP.TimeChangeFlag) OP.TimeChangeFlag = 0;
    }
    
    OP.ShowTimeFlag = 1;
    if(Rtc.day != rtc.day)          //Date carry, refresh once
        OP.PlanRefreshFlag = 1;     //Period table number refresh restart and every day, refresh once
    if(Rtc.minute != rtc.minute)
        OP.ActionRefreshFlag = 1;   //Action Refresh Every minute, refresh once
}

/********************************************************/
//Function: Obtain the schedule number by querying the scheduling schedule, and determine whether the schedule needs to be updated 
//Boot and run once a day at zero
void RunPlanRefresh(void)
{
    if(OP.PlanRefreshFlag)
    {
        OP.PlanRefreshFlag = 0;
        #if DEBUG
        printf("Plan Refresh \r\n");
        #endif
        if(SchedulePlanRefresh(&ScheduleTab, &Date))
            OP.ActionRefreshFlag = 1; //After the period table is updated, the period needs to be refreshed
    }
}

/* After starting up, the program runs directly without judging the changes of operating parameters */
void NewPeriodApply(PlanType* DayPlan, TimeType* Time)
{
    uint8_t ActionNum, ActionIndex, PatternIndex;
    uint8_t temp = GetPeriodIndex(DayPlan, Time); //time + period table = period index
    if(temp < PeriodMax)//time period index
    {
        ActionNum = DayPlan->Period[temp].ActionNum;
        //Update period information
        Period.Time.Hour = DayPlan->Period[temp].Time.Hour;
        Period.Time.Minute = DayPlan->Period[temp].Time.Minute;
        Period.ActionNum = ActionNum;
        ActionIndex = GetActionIndex(&ActionTab, ActionNum);
        
        if(ActionIndex < ActionMax)//non-empty action
        {
            //Copy action data;
            //Special Features, Accessibility Features, Direct Updates
            memcpy(&Action, &ActionTab.Action[ActionIndex], sizeof(Action));
            
            PatternIndex = GetPatternIndex(&PatternTab, Action.Pattern);
            if(PatternIndex < PatternMax)
            {
                memcpy(&PatternNow, &PatternTab.Pattern[PatternIndex], sizeof(PatternNow));
                
                memcpy(&SequenceNow, &SeqTab.Seq[PatternNow.SequenceNum - 1], sizeof(SequenceNow));
                memcpy(&SplitNow, &SplitTab.Split[PatternNow.SplitNum - 1], sizeof(SplitNow));
                OP.NewPatternFlag = 1;//The new solution is ready, the cycle is complete, and it can be implemented
            }
        }
    }
}

void NewPatternCheck(void)
{
    #if DEBUG 
    printf("NewPatternCheck\r\n");
    #endif
    if(SchedulePlanRefresh(&ScheduleTab, &Date))
    {
        #if DEBUG 
        printf("FindSchedulePlan\r\n");
        #endif
        NewPeriodApply(&Plan, &Time);
        OP.ActionRefreshFlag = 1;
    }
}

/********************************************************/
void RunPlanActionRefresh(void)
{
    if(OP.ActionRefreshFlag)
    {
        OP.ActionRefreshFlag = 0;
        #if DEBUG 
        printf("Action Refresh\r\n");
        #endif
        PlanActionRefresh(&Plan, &Time);
    }
}

//Function: Query the time period table and get the action number of the current time 
//Pass parameters: period table + time
void PlanActionRefresh(PlanType* DayPlan, TimeType* Time)
{
    uint8_t ActionIndex, PatternIndex;
    //time + period table = period index
    uint8_t PeriodIndex = GetPeriodIndex(DayPlan, Time);
    if(PeriodIndex < PeriodMax)
    {
        //If the time period parameter changes, update the time period parameter
        if(memcmp(&Period, &DayPlan->Period[PeriodIndex], sizeof(Period)) != 0)
        {
            #if DEBUG 
            printf("Period Changed\r\n");
            #endif
            memcpy(&Period, &DayPlan->Period[PeriodIndex], sizeof(Period));
        }
        
        ActionIndex = GetActionIndex(&ActionTab, Period.ActionNum);
        if(ActionIndex < ActionMax)//non-empty action
        {
            if(memcmp(&Action, &ActionTab.Action[ActionIndex], sizeof(Action)) != 0)
            {
                #if DEBUG
                printf("Action Changed\r\n");
                #endif
                memcpy(&Action, &ActionTab.Action[ActionIndex], sizeof(Action));
            }
        }
        
        PatternIndex = GetPatternIndex(&PatternTab, Action.Pattern);
        if(PatternIndex < PatternMax)
        {
            //If the phase sequence and green signal ratio parameters are changed, it will not be updated here
            //Therefore, if the phase sequence and green signal ratio parameters are changed, change the Pattern sequence and green signal ratio to update the operating parameters
            #if DEBUG
            printf_fifo_dec(&PatternNow.Num, sizeof(PatternNow));
            printf_fifo_dec(&PatternTab.Pattern[PatternIndex].Num, sizeof(PatternNow));
            #endif
            if(memcmp(&PatternNow, &PatternTab.Pattern[PatternIndex], sizeof(PatternNow)) != 0)
            {
                #if DEBUG
                printf("Action.Pattern = %d\r\n", Action.Pattern);
                printf("PatternIndex = %d\r\n", PatternIndex);
                printf("Pattern Changed\r\n");
                printf("Prepare New Pattern\r\n");
                #endif
                
                memcpy(&PatternNow, &PatternTab.Pattern[PatternIndex], sizeof(PatternNow));
                memcpy(&SequenceNow, &SeqTab.Seq[PatternNow.SequenceNum - 1], sizeof(SequenceNow));
                memcpy(&SplitNow, &SplitTab.Split[PatternNow.SplitNum - 1], sizeof(SplitNow));
                OP.NewPatternFlag = 1;
            }
        }
    }
}

/********************************************************/
void NewPatternApply(void)
{
    #if DEBUG
    printf("0New Pattern Apply\r\n");
    #endif

    OP.WorkMode = PatternNow.WorkMode;
    OP.WorkMode_Reason = WMR_Normal;
    if(OP.WorkMode == Flashing)AutoFlashMode();
    else if(OP.WorkMode == AllRed)AutoAllRedMode();
    else if(OP.WorkMode == LampOff)AutoLampOffMode();
    else if(OP.WorkMode == FixedTime || OP.WorkMode == LineCtrl)
    {
        GetPhaseStateSeqMax();//Obtain the maximum phase number of each ring, as well as the maximum value of the phase number and the corresponding ring number through the continuity table
        GetSeqTime();//Obtain phase sequence time and cycle through phase sequence table and green signal ratio
        
        if(OP.WorkMode == LineCtrl) GreenWaveTimeControl();//LineCtrl
        RunPhaseInit(&SequenceNow,  &SplitNow);
        GetPhaseStatusMap();
    }
    else if(OP.WorkMode == VehicleSense)//induction mode
    {
        GetPhaseStateSeqMax();  //Obtain the maximum phase number of each ring, as well as the maximum value of the phase number and the corresponding ring number through the continuity table
        GetSeqTime();           //Obtain phase sequence time and cycle through phase sequence table and green signal ratio
        
        RunPhaseInit(&SequenceNow,  &SplitNow);
        GetPhaseStatusMap();
    }
    bsp_LedToggle(LED_NEWPLAN);
}

void RunGetVehDetState(void)
{
    if(OP.GetVehDetStaFlag)
    {
        OP.GetVehDetStaFlag = 0;
        GetVehDetSta(); 
        VehDetStaCount();
    }
}

void RunModeProcess(void)
{
    uint8_t temp;
    if(OP.ScheduleDataChangeFlag)//Scheduling plan parameters have changed
    {
        OP.ScheduleDataChangeFlag = 0;
        OP.PlanRefreshFlag = 1;
    }
    if(OP.PlanDataChangeFlag)
    {
        OP.PlanDataChangeFlag = 0;
        #if DEBUG
        printf("Plan Data Changed \r\n");
        #endif
        temp = ScheduleNow.PlanNum - 1;
        memcpy(&Plan, &PlanTab.Plan[temp], sizeof(Plan));
        //After the period table is updated, the period needs to be refreshed
        OP.ActionRefreshFlag = 1;
    }
    if(OP.ActionDataChangeFlag)
    {
        OP.ActionDataChangeFlag = 0;
        OP.ActionRefreshFlag = 1;
    }
    if(OP.PatternDataChangeFlag)
    {
        OP.PatternDataChangeFlag = 0;
        OP.ActionRefreshFlag = 1;
    }
    if(OP.SequenceDataChangeFlag)
    {
        OP.SequenceDataChangeFlag = 0;
        PatternNow.SequenceNum = 0;
        OP.ActionRefreshFlag = 1;
    }
    if(OP.SplitDataChangeFlag)
    {
        OP.SplitDataChangeFlag = 0;
        PatternNow.SplitNum = 0;
        OP.ActionRefreshFlag = 1;
    }
    RunPlanRefresh();
    RunPlanActionRefresh();
}

void manual_mange(void)//Run once in 10ms
{
    static uint8_t Value = 0xff, Count = 0;
    uint8_t cmd;
    uint8_t temp = manual_scan();
    if(Value != temp)
    {
        if(++Count > 10)
        {
            Count = 0;
            cmd = Value^temp;
            Value = temp;
            if(cmd & Value)//button release
            {
                if(cmd == 0x01)//close manually
                {
                    ManualCtrl.KeyCmd = 0;
                }
            }
            else//button pressed
            {
                ManualCtrl.KeyCmd = cmd;
            }
            #if DEBUG
            printf("temp = %x, cmd = %x, ManualCmd = %x \r\n", temp, cmd, ManualCtrl.KeyCmd);
            #endif
        }
    }
    else
    {
        Count = 0;
    }
}

void manual_control(void)
{
    //ManualCtrl.KeyCmd
    //bit0 is the manual switch, 0 turns on the manual, 1 turns off the manual
    //bit1 yellow flash
    if(ManualCtrl.KeyCmd < 0xff)//Hand control has action
    {
        if(ManualCtrl.LocalCtrlFlag == 0) //non-manual mode
        {
            if(ManualCtrl.KeyCmd == MANUAL_ON)//Toggle switch, from automatic to manual
            {
                ManualCtrl.LocalCtrlFlag = 1;
                //if(OP.WorkMode < SPECIAL_MODE) OP.WorkModeBK = OP.WorkMode;
                if(OP.WorkMode < Flashing)//
                {
                    OP.WorkModeBK = OP.WorkMode;
                    OP.WorkMode = ManualStep;
                    ManualCtrl.Time = defaultAutoExitRemoteTime;
                }
                OP.WorkMode_Reason = WMR_LocalManual;
            }
            else if(ManualCtrl.KeyCmd == MANUAL_ClearError)//Manually clear the fault
            {
                if(OP.WorkMode_Reason)
                {
                    if(WMR_RedGreenConflict == OP.WorkMode_Reason)
                    {
                        OP.red_green_conflict_reg = 0;//clear fault
                        OP.WorkMode_Reason = WMR_Normal;//Make the signal machine flash yellow normally instead of flashing yellow when fault occurs
                    }
                    if(WMR_GreenConflict == OP.WorkMode_Reason)
                    {
                        OP.green_conflict_reg = 0;//clear fault
                        OP.WorkMode_Reason = WMR_Normal;//Make the signal machine flash yellow normally instead of flashing yellow when fault occurs
                    }
                    if(WMR_RedFailed == OP.WorkMode_Reason)
                    {
                        OP.red_install_reg = red_install_fail_detect(0);//borrar falla
                        OP.WorkMode_Reason = WMR_Normal;//Make the signal machine flash yellow normally instead of flashing yellow when fault occurs
                    }
                }
                #if DEBUG
                printf("MANUALClearError\r\n");
                #endif
            }
/*
            else if(ManualCtrl.KeyCmd == MANUAL_K1)//K1
            {
                if(OP.WorkMode != ManualAppoint)
                {
                    if(OP.WorkMode < Flashing) OP.WorkModeBK = OP.WorkMode;
                    OP.WorkMode = ManualAppoint;
                    OP.WorkMode_Reason = WMR_LocalManual;
                    ManualCtrl.StartFlag = 1;
                    ManualCtrl.LocalCtrlFlag = 1;
                    ManualCtrl.EnforceFlag = 0;
                    ManualCtrl.OrderFlag = 0;
                    ManualCtrl.ExitFlag = 0;
                    ManualCtrl.AutoTime = defaultAutoExitRemoteTime;
                }
                #if DEBUG
                printf("MANUAL_K1\r\n");
                #endif
            }
            else if(ManualCtrl.KeyCmd == MANUAL_K2)//
            {
                if(OP.WorkMode != ManualAppoint)
                {
                    if(OP.WorkMode < Flashing) OP.WorkModeBK = OP.WorkMode;
                    OP.WorkMode = ManualAppoint;
                    ManualCtrl.StartFlag = 1;
                }
                ManualCtrl.LocalCtrlFlag = 1;
                OP.WorkMode_Reason = WMR_LocalManual;
                ManualCtrl.Pos = 0x01;
                ManualCtrl.Dir = 0x07;
                ManualCtrl.EnforceFlag = 1;
                ManualCtrl.OrderFlag = 1;
                ManualCtrl.ExitFlag = 0;
                ManualCtrl.AutoTime = defaultAutoExitRemoteTime;
                ManualCtrl.ChannelOnsNext = GetAppointChannel(ManualCtrl.Pos, ManualCtrl.Dir);
                #if DEBUG
                printf("MANUAL_K2\r\n");
                #endif
            }
            else if(ManualCtrl.KeyCmd == MANUAL_K5)//
            {
                if(OP.WorkMode == ManualAppoint)
                {
                    ManualCtrl.ExitFlag = 1;
                }
                #if DEBUG
                printf("MANUAL_K5\r\n");
                #endif
            }
*/
        }
        else //manual mode
        {
            if(ManualCtrl.KeyCmd)
            {
                if(ManualCtrl.KeyCmd == MANUAL_FLASH)//yellow flash
                {
                    OP.WorkMode = ManualFlashing;
                    OP.WorkMode_Reason = WMR_LocalManual;
                    ManualCtrl.Time = defaultAutoExitRemoteTime;
                    #if DEBUG
                    printf("ManualFlashing\r\n");
                    #endif
                }
                else if(ManualCtrl.KeyCmd == MANUAL_AllRed)//all red
                {
                    OP.WorkMode = ManualAllRead;
                    OP.WorkMode_Reason = WMR_LocalManual;
                    ManualCtrl.Time = defaultAutoExitRemoteTime;
                    #if DEBUG
                    printf("ManualAllRead\r\n");
                    #endif
                }
                else if(ManualCtrl.KeyCmd == MANUAL_NextStep)//Next step
                {
                    OP.WorkMode = ManualStep;
                    OP.WorkMode_Reason = WMR_LocalManual;
                    ManualCtrl.Time = defaultAutoExitRemoteTime;
                    ManualCtrl.NextStepFlag = 1;
                    #if DEBUG
                    printf("ManualNextStep\r\n");
                    #endif
                }
//                else if(ManualCtrl.KeyCmd == MANUAL_LampOff)//turn off the lights
//                {
//                    OP.WorkMode = ManualLampOff;
//                    OP.WorkMode_Reason = WMR_LocalManual;
//                    #if DEBUG
//                    printf("ManualLampOff\r\n");
//                    #endif
//                }
                else if(ManualCtrl.KeyCmd == MANUAL_K4)//
                {
                    if(OP.WorkMode != ManualAppoint)
                    {
                        if(OP.WorkMode < Flashing) OP.WorkModeBK = OP.WorkMode;
                        ManualCtrl.StartFlag = 1;
                    }
                    OP.WorkMode = ManualAppoint;
                    OP.WorkMode_Reason = WMR_LocalManual;
                    ManualCtrl.LocalCtrlFlag = 1;
                    ManualCtrl.Pos = 0x01;
                    ManualCtrl.Dir = 0x07;
                    ManualCtrl.EnforceFlag = 1;
                    ManualCtrl.OrderFlag = 1;
                    ManualCtrl.ExitFlag = 0;
                    //ManualCtrl.Time = 0;
                    ManualCtrl.AutoTime = defaultAutoExitRemoteTime;
                    ManualCtrl.ChannelOnsNext = GetAppointChannel(ManualCtrl.Pos, ManualCtrl.Dir);
                    #if DEBUG
                    printf("MANUAL_K3\r\n");
                    #endif
                }
                else if(ManualCtrl.KeyCmd == MANUAL_K5)//
                {
                    if(OP.WorkMode != ManualAppoint)
                    {
                        if(OP.WorkMode < Flashing) OP.WorkModeBK = OP.WorkMode;
                        ManualCtrl.StartFlag = 1;
                    }
                    OP.WorkMode = ManualAppoint;
                    OP.WorkMode_Reason = WMR_LocalManual;
                    ManualCtrl.LocalCtrlFlag = 1;
                    ManualCtrl.Pos = 0x02;
                    ManualCtrl.Dir = 0x07;
                    ManualCtrl.EnforceFlag = 1;
                    ManualCtrl.OrderFlag = 1;
                    ManualCtrl.ExitFlag = 0;
                    //ManualCtrl.Time = 0;
                    ManualCtrl.AutoTime = defaultAutoExitRemoteTime;
                    ManualCtrl.ChannelOnsNext = GetAppointChannel(ManualCtrl.Pos, ManualCtrl.Dir);
                    #if DEBUG
                    printf("MANUAL_K3\r\n");
                    #endif
                }
                else if(ManualCtrl.KeyCmd == MANUAL_K6)//
                {
                    if(OP.WorkMode != ManualAppoint)
                    {
                        if(OP.WorkMode < Flashing) OP.WorkModeBK = OP.WorkMode;
                        ManualCtrl.StartFlag = 1;
                    }
                    OP.WorkMode = ManualAppoint;
                    OP.WorkMode_Reason = WMR_LocalManual;
                    ManualCtrl.LocalCtrlFlag = 1;
                    ManualCtrl.Pos = 0x04;
                    ManualCtrl.Dir = 0x07;
                    ManualCtrl.EnforceFlag = 1;
                    ManualCtrl.OrderFlag = 1;
                    ManualCtrl.ExitFlag = 0;
                    //ManualCtrl.Time = 0;
                    ManualCtrl.AutoTime = defaultAutoExitRemoteTime;
                    ManualCtrl.ChannelOnsNext = GetAppointChannel(ManualCtrl.Pos, ManualCtrl.Dir);
                    #if DEBUG
                    printf("MANUAL_K3\r\n");
                    #endif
                }
                else if(ManualCtrl.KeyCmd == MANUAL_K7)//
                {
                    if(OP.WorkMode != ManualAppoint)
                    {
                        if(OP.WorkMode < Flashing) OP.WorkModeBK = OP.WorkMode;
                        ManualCtrl.StartFlag = 1;
                    }
                    OP.WorkMode = ManualAppoint;
                    OP.WorkMode_Reason = WMR_LocalManual;
                    ManualCtrl.LocalCtrlFlag = 1;
                    ManualCtrl.Pos = 0x08;
                    ManualCtrl.Dir = 0x07;
                    ManualCtrl.EnforceFlag = 1;
                    ManualCtrl.OrderFlag = 1;
                    ManualCtrl.ExitFlag = 0;
                    //ManualCtrl.Time = 0;
                    ManualCtrl.AutoTime = defaultAutoExitRemoteTime;
                    ManualCtrl.ChannelOnsNext = GetAppointChannel(ManualCtrl.Pos, ManualCtrl.Dir);
                    #if DEBUG
                    printf("MANUAL_K3\r\n");
                    #endif
                }
            }
            else//Toggle switch, from manual to automatic
            {
                if(OP.WorkMode == ManualAppoint)
                {
                    ManualCtrl.ExitFlag = 1;
                }
                else
                {
                    ManualCtrl.LocalCtrlFlag = 0;
                    if(OP.WorkMode >= SPECIAL_MODE) OP.WorkMode = OP.WorkModeBK;
                    OP.WorkMode_Reason = WMR_Normal;
                }
            }
        }
        ManualCtrl.KeyCmd = 0xff;
    }
}

void rf315m_mange(void)
{
    if(rf_state > 0)
    {
        if(rf_state == 0x08)
        {
            ManualCtrl.LocalCtrlFlag = 1;
            if(OP.WorkMode < SPECIAL_MODE) OP.WorkModeBK = OP.WorkMode;
            ManualCtrl.KeyCmd = MANUAL_FLASH;
        }
        else if(rf_state == 0x04)
        {
            ManualCtrl.LocalCtrlFlag = 1;
            if(OP.WorkMode < SPECIAL_MODE) OP.WorkModeBK = OP.WorkMode;
            ManualCtrl.KeyCmd = MANUAL_AllRed;
        }
        else if(rf_state == 0x01)
        {
            ManualCtrl.LocalCtrlFlag = 1;
            if(OP.WorkMode < SPECIAL_MODE) OP.WorkModeBK = OP.WorkMode;
            ManualCtrl.KeyCmd = MANUAL_NextStep;
        }
        else if(rf_state == 0x02)
        {
            ManualCtrl.KeyCmd = MANUAL_OFF;
        }
    }
}

//In the same light group, the detection of red and green conflicts 
//Return: 0-15 bit corresponds to a conflict in the light group, 0 has no conflict
uint16_t Red_Green_conflict_detect(uint16_t red_state, uint16_t green_state)//200ms
{
    static uint8_t c200ms[16] = {0};
    static uint16_t conflict_reg = 0;
    uint16_t i = 0, temp_group = 0x1, temp_var = 0x8000, lamp_state;

    lamp_state = red_state & green_state;
    for(i = 0; i < 16; i++)
    {
        if((lamp_state & temp_var) == temp_var)
        {
            if(++c200ms[i] >= 10)//lasts more than 2 seconds
            {
                c200ms[i] = 0;
                conflict_reg |= temp_group;
            }
        }
        else
        {
            c200ms[i] = 0;
            conflict_reg &= ~temp_group;
        }
        temp_group <<= 1;
        temp_var >>= 1;
    }

    return conflict_reg;
}

//phase 1  1111 0111 
//         0000 1000
//         0000 1001
uint16_t Green_conflict_detect(void)
{
    static uint8_t c200ms[16] = {0};
    static uint16_t conflict_reg = 0;
    uint32_t i = 0, phaseindex, temp_phase = 0x1, temp_concurrency, temp_release;

    for(i = 0; i < 16;i++)
    {
        if(PhaseStatus.Greens & temp_phase)//The light group has been released
        {
            temp_release = PhaseStatus.Greens & (~temp_phase);
            phaseindex = GetPhaseIndex(&PhaseTab, i+1);
            temp_concurrency = PhaseTab.Phase[phaseindex].ConcurrencyL | (PhaseTab.Phase[phaseindex].ConcurrencyH<<8);
            temp_concurrency = (~temp_concurrency)&0xffff;
            if(temp_concurrency & temp_release)
            {
                if(++c200ms[i] >= 10)//lasts more than 2 seconds
                {
                    c200ms[i] = 0;
                    conflict_reg |= temp_phase;
                }
            }
            else
            {
                c200ms[i] = 0;
                conflict_reg &= ~temp_phase;
            }
        }
        else 
        {
            c200ms[i] = 0;
            conflict_reg &= ~temp_phase;
        }
        temp_phase <<= 1;
    }

    return conflict_reg;
}

uint16_t red_install_fail_detect(uint16_t installs)
{
    static uint8_t c200ms[16] = {0};
    static uint16_t red_install_reg = 0;
    uint16_t i = 0, temp_group = 0x1, temp_var = 0x8000;
    
    if(installs == 0) { red_install_reg = 0; OP.red_failed_reg = 0; return 0;}
    for(i = 0; i < 16; i++)
    {
        if(red_state & temp_var)//A red light on a road has an output
        {
            if(current_stab & temp_var)//and there is current
            {
                if(++c200ms[i] >= 10)//lasts more than 2 seconds
                {
                    c200ms[i] = 0;
                    red_install_reg |= temp_group;
                }
            }
            else if(red_install_reg & temp_group)//sin corriente e instalado
            {
                if(++c200ms[i] >= 10)//Failure lasts longer than 2 seconds
                {
                    c200ms[i] = 0;
                    OP.red_failed_reg |= temp_group;
                }
            }
            else c200ms[i] = 0;
        }
        else c200ms[i] = 0;
        
        temp_group <<= 1;
        temp_var >>= 1;
    }
    
    return red_install_reg;
}

void fail_conflict_detect(void)
{
    red_state = ((red_state_stab&0xf) << 12) | 
                ((red_state_stab&0xf0) << 4) |
                ((red_state_stab&0xf00) >> 4) |
                ((red_state_stab&0xf000) >> 12);
    green_state =   ((green_state_stab&0xf) << 12) | 
                    ((green_state_stab&0xf0) << 4) |
                    ((green_state_stab&0xf00) >> 4) |
                    ((green_state_stab&0xf000) >> 12);
    current_stab = ((current_state_stab&0x3333) << 2) |
                   ((current_state_stab&0xcccc) >> 2);
    current_stab =  ((current_stab&0xf) << 12) | 
                    ((current_stab&0xf0) << 4) |
                    ((current_stab&0xf00) >> 4) |
                    ((current_stab&0xf000) >> 12);

    if(Unit.RedGreenConflictDetectFlag)
    {
        OP.red_green_conflict_reg = Red_Green_conflict_detect(red_state, green_state);
        if(OP.red_green_conflict_reg)
        {
            if(OP.WorkMode_Reason == WMR_Normal)
            {
                OP.WorkMode_Reason = WMR_RedGreenConflict;
                OP.WorkModeBK = OP.WorkMode;
                OP.WorkMode = Flashing;
            }
        }
    }
    
    if(Unit.GreenConflictDetectFlag)
    {
        OP.green_conflict_reg = Green_conflict_detect();
        if(OP.green_conflict_reg)
        {
            if(OP.WorkMode_Reason == WMR_Normal)
            {
                OP.WorkMode_Reason = WMR_GreenConflict;
                OP.WorkModeBK = OP.WorkMode;
                OP.WorkMode = Flashing;
            }
        }
    }
    
    if(Unit.RedFailedDetectFlag)
    {
        OP.red_install_reg = red_install_fail_detect(0xff);
        if(OP.red_failed_reg)
        {
            if(OP.WorkMode_Reason == WMR_Normal)
            {
                OP.WorkMode_Reason = WMR_RedFailed;
                OP.WorkModeBK = OP.WorkMode;
                OP.WorkMode = Flashing;
            }
        }
    }
}

void Input_mange(void)
{
    static uint8_t reg1s = 5;
    static uint8_t reg10ms = 10;
    static uint8_t reg200ms = 20;
    
    if(reg1ms_flag) //1ms information processing
    {
        reg1ms_flag = 0;
        lamp_state_detect();//1ms
        if(++reg10ms >= 10)//10ms
        {
            reg10ms = 0;
            OP.GetVehDetStaFlag = 1;
            peddet_scan(&peddet_hw);    //8-way button input status detection
            rf315m_scan();              //rf315M manual input
            rf315m_mange();
            manual_mange();

            if(++reg200ms >= 20)//200ms
            {
                reg200ms = 0;
                fail_conflict_detect();
                if(++reg1s >= 5)//1s
                {
                    reg1s = 0;
                    PeddetStateGet();//Detector de pedestrian
                }
            }
        }
    }
}

void Auto_adjust_time(void)
{
    if(OP.sync_with_gps_flag)//wcxmask
    {
        RtcWrite(&OP.Gps.local);
        OP.TimeChangeFlag = 1;
        OP.sync_with_gps_flag = 0;
        #if DEBUG > 2
        printf("sync_with_gps, Seconds = %d, gps_seconds = %d\n", OP.Seconds, OP.gps_seconds);
        #endif
    }
}

void LampControlProcess(void)
{
    if(OP.Run10ms_flag) //10ms information processing
    {
        OP.Run10ms_flag = 0;
        RunPhaseTimeCalc();
    }
    
    if(OP.Run1s_flag)
    {
        OP.Run1s_flag = 0;
        OP.Reg1sCount++;
        Auto_adjust_time();
        ReadRealTime();
        bsp_LedToggle(LED_RUN);
        PPS_Toggle();
    }
    LampStateControl();
}

//Green wave control, phase difference time calculation and control
void GreenWaveTimeControl(void)
{
    OP.OffsetTimeModifier.OffsetTime = ((OP.Seconds + NowCycleTime - PatternNow.OffsetTime)%NowCycleTime);//相位偏差时间
    if(OP.OffsetTimeModifier.OffsetTime==0)
    {
        OP.OffsetTimeModifier.ModifierFlag = 0;
    }
    else
    {
        OP.OffsetTimeModifier.ModifierTime = NowCycleTime - OP.OffsetTimeModifier.OffsetTime;
        if(OP.OffsetTimeModifier.OffsetTime <= (NowCycleTime * 4 / 10))
        {
            OP.OffsetTimeModifier.ModifierFlag = 1;
            if(OP.OffsetTimeModifier.OffsetTime <= (NowCycleTime * 2 / 10))
                OP.OffsetTimeModifier.StepTime = OP.OffsetTimeModifier.OffsetTime/PhaseState.CycleStepMax + 1;
            else
                OP.OffsetTimeModifier.StepTime = OP.OffsetTimeModifier.OffsetTime/PhaseState.CycleStepMax/2 + 1;
        }
        else
        {
            OP.OffsetTimeModifier.ModifierFlag = 2;
            if(OP.OffsetTimeModifier.ModifierTime <= (NowCycleTime * 3 / 10))
                OP.OffsetTimeModifier.StepTime = OP.OffsetTimeModifier.ModifierTime/PhaseState.CycleStepMax + 1;
            else
                OP.OffsetTimeModifier.StepTime = OP.OffsetTimeModifier.ModifierTime/PhaseState.CycleStepMax/2 + 1;
        }
    }

    #if DEBUG 
    printf("OP.Seconds = %d, NowCycleTime = %d \r\n",OP.Seconds,NowCycleTime);
    printf("OffsetTime = %d \r\n", OP.OffsetTimeModifier.OffsetTime);
    printf("ModifierTime = %d \r\n", OP.OffsetTimeModifier.ModifierTime);
    printf("ModifierFlag = %d \r\n", OP.OffsetTimeModifier.ModifierFlag);
    printf("CycleStepMax = %d \r\n", PhaseState.CycleStepMax);
    printf("StepTime = %d \r\n", OP.OffsetTimeModifier.StepTime);
    #endif
}

void GreenWaveTimeChange(void)
{
    uint8_t ring;
    if(OP.OffsetTimeModifier.ModifierFlag == 1)
    {
        if(OP.OffsetTimeModifier.OffsetTime > OP.OffsetTimeModifier.StepTime)
        {
            for(ring = 0; ring < RingMax; ring++)
            {
                if(PhaseState.Ring[ring].SeqMax) PhaseState.Ring[ring].SecondRemain -= OP.OffsetTimeModifier.StepTime;
            }
            OP.OffsetTimeModifier.OffsetTime -= OP.OffsetTimeModifier.StepTime;
            #if DEBUG
            printf("greenware -%d\r\n",OP.OffsetTimeModifier.StepTime);
            printf("OffsetTime = %d\r\n",OP.OffsetTimeModifier.OffsetTime);
            #endif
        }
        else
        {
            for(ring = 0; ring < RingMax; ring++)
            {
                if(PhaseState.Ring[ring].SeqMax) PhaseState.Ring[ring].SecondRemain -= OP.OffsetTimeModifier.OffsetTime;
            }
            OP.OffsetTimeModifier.OffsetTime = 0;
            OP.OffsetTimeModifier.ModifierTime = 0;
            OP.OffsetTimeModifier.ModifierFlag = 0;
            OP.OffsetTimeModifier.StepTime = 0;
            #if DEBUG
            printf("greenware matched\r\n");
            printf("OffsetTime = %d\r\n",OP.OffsetTimeModifier.OffsetTime);
            #endif
        }
    }
    else if(OP.OffsetTimeModifier.ModifierFlag == 2)
    {
        if(OP.OffsetTimeModifier.ModifierTime > OP.OffsetTimeModifier.StepTime)
        {
            for(ring = 0; ring < RingMax; ring++)
            {
                if(PhaseState.Ring[ring].SeqMax) PhaseState.Ring[ring].SecondRemain += OP.OffsetTimeModifier.StepTime;
            }
            OP.OffsetTimeModifier.ModifierTime -= OP.OffsetTimeModifier.StepTime;
            #if DEBUG
            printf("greenware +%d\r\n",OP.OffsetTimeModifier.StepTime);
            printf("ModifierTime = %d\r\n",OP.OffsetTimeModifier.ModifierTime);
            #endif
        }
        else
        {
            for(ring = 0; ring < RingMax; ring++)
            {
                if(PhaseState.Ring[ring].SeqMax) PhaseState.Ring[ring].SecondRemain += OP.OffsetTimeModifier.ModifierTime;
            }
            OP.OffsetTimeModifier.OffsetTime = 0;
            OP.OffsetTimeModifier.ModifierTime = 0;
            OP.OffsetTimeModifier.ModifierFlag = 0;
            OP.OffsetTimeModifier.StepTime = 0;
            #if DEBUG
            printf("greenware matched\r\n");
            printf("ModifierTime = %d\r\n",OP.OffsetTimeModifier.ModifierTime);
            #endif
        }
    }
}

/*
 * 当前运行的相位改变
 * 重新赋值相位 和 绿信比 数据
 */
uint8_t RingPhaseChange(uint8_t ring)
{
    uint8_t ChangeFlag = 0,SeqNum,PhaseNum,PhaseIndex,PhaseNext,SplitPhaseIndex,SplitNextPhaseIndex;
    if(PhaseState.Ring[ring].PhaseChangeFlag)
    {
        PhaseState.Ring[ring].PhaseChangeFlag = 0;
        SeqNum = PhaseState.Ring[ring].SeqNum;
        
        //此处根据相位号，查表获取相位索引 和 绿信比相位索引
        PhaseNum = SequenceNow.Ring[ring].Phase[SeqNum];
        if(SeqNum+1 < PhaseState.Ring[ring].SeqMax)
            PhaseNext = SequenceNow.Ring[ring].Phase[SeqNum+1];
        else
            PhaseNext = SequenceNow.Ring[ring].Phase[0];
        
        RingPhase[ring].PhaseIndex = GetPhaseIndex(&PhaseTab, PhaseNum);
        RingPhase[ring].PhaseNextIndex = GetPhaseIndex(&PhaseTab, PhaseNext);
        PhaseIndex = RingPhase[ring].PhaseIndex;
        
        //通过索引，复制运行的相位和绿信比
        RingPhase[ring].PhaseNum = PhaseNum;
        RingPhase[ring].PhaseNext = PhaseNext;
        
        SplitPhaseIndex = GetSplitPhaseIndex(&SplitNow, PhaseNum);
        SplitNextPhaseIndex = GetSplitPhaseIndex(&SplitNow, PhaseNext);
        memcpy(&RingSplit[ring], &SplitNow.Phase[SplitPhaseIndex], sizeof(PhaseSplitType));
        
        RingPhase[ring].SplitIndex = SplitPhaseIndex;
        RingPhase[ring].SplitNextIndex = SplitNextPhaseIndex;
        
        PhaseState.Ring[ring].SecondRemain = RingSplit[ring].Time;
        PhaseState.Ring[ring].VehicleTransitionTime = PhaseTab.Phase[PhaseIndex].VehicleClear + 
                                                      PhaseTab.Phase[PhaseIndex].YellowChange + 
                                                      PhaseTab.Phase[PhaseIndex].RedClear;
        if(PhaseState.StepMaxRing == ring && OP.OffsetTimeModifier.ModifierFlag > 0)
        {
            ChangeFlag = 1;
            #if DEBUG
            printf("ChangeFlag = 1\r\n");
            #endif
        }
    }
    return ChangeFlag;
}

//要把 PhaseState 当中的一些参数剥离出去, 相应的功能函数分离出去 wcx
void GetPhaseStateSeqMax(void)
{
    uint8_t ring;
    PhaseState.ValidRings = 0;
    PhaseState.CycleStepMax = 0;
    for(ring = 0; ring < RingMax; ring++)
    {
        PhaseState.Ring[ring].SeqMax = GetSeqMax(&SequenceNow.Ring[ring]);//获取此环的相位数量
        if(PhaseState.Ring[ring].SeqMax)
        {
            PhaseState.ValidRings++;
            if(PhaseState.Ring[ring].SeqMax > PhaseState.CycleStepMax)
            {
                PhaseState.CycleStepMax = PhaseState.Ring[ring].SeqMax;
                PhaseState.StepMaxRing = ring;
            }
        }
    }
    #if DEBUG
    printf("ValidRings = %d, StepMaxRing = %d, CycleStepMax = %d\r\n",\
            PhaseState.ValidRings, PhaseState.StepMaxRing, PhaseState.CycleStepMax);
    #endif
}

/*
    运行的相位数据初始化，从相序和绿信比数据中获取运行的4个环的相位数据和绿信比
*/
void RunPhaseInit(SequenceType* Sequence, SplitType* SplitX)
{
    uint8_t ring, ChangeFlag = 0;
    
    for(ring = 0; ring < RingMax; ring++)
    {
        PhaseState.Ring[ring].SeqNum = 0;
        if(PhaseState.Ring[ring].SeqMax > 0)//非空环
        {
            PhaseState.Ring[ring].PhaseChangeFlag = 1;
            if(RingPhaseChange(ring)) ChangeFlag = 1;
            PhaseState.Ring[ring].RingNum = Sequence->Ring[ring].RingNum;
            PhaseState.Ring[ring].CycleOverFlag = 0;
        }
        else
        {
            PhaseState.Ring[ring].RingNum = 0;
            PhaseState.Ring[ring].CycleOverFlag = 1;
        }
    }
    PhaseState.Phase10msCount = 0;
    PhaseState.Phase1sFlag = 0;
    if(ChangeFlag == 1 && OP.WorkMode == LineCtrl) GreenWaveTimeChange();
}


/*
    创作不基于任何硬件的软件系统
*/
void RunDataInit(void)
{
    ChannelReadStatus.Greens = 0;
    ChannelReadStatus.Yellows = 0;
    ChannelReadStatus.Reds = 0;
    
    OP.RestartFlag = 0;
    OP.StartFlag = 1;
    OP.TimeChangeFlag = 1;
    OP.ConnectFlag = 0;
    OP.SendWorkModeAutoFlag = 0;
    OP.SendWorkModeFlag = 0;
    OP.SendDoorAlarm = 0;
    OP.sync_with_gps_flag = 0;

    LampDriveDataInit();//初始化等驱动数据及13H
}


/*
 * 启动步序的 PhaseState 赋值
 */
void RunPhaseStateStartup(void)
{
    uint8_t ring;
    for(ring = 0; ring < RingMax; ring++)
    {
        PhaseState.Ring[ring].RingNum = 0;
        PhaseState.Ring[ring].CycleOverFlag = 1;
    }
    
    PhaseState.Phase1sFlag = 0;
    PhaseState.Phase10msCount = 0;
    PhaseState.Ring[0].SeqNum = 0;
    PhaseState.Ring[0].SeqMax = 2;
    PhaseState.Ring[0].RingNum = 1;
    PhaseState.Ring[0].CycleOverFlag = 0;
    PhaseState.Ring[0].SecondRemain = Unit.StartupFlash;
    AutoFlashMode();
    OP.WorkMode = StarupMode;
    OP.red_install_reg = 0;
    OP.red_failed_reg = 0;
    
    #if DEBUG
    printf("Startup Flashing\r\n");
    #endif
}

void StartupProcess(void)//启动模式
{
    if(PhaseState.Ring[0].SecondRemain) PhaseState.Ring[0].SecondRemain--;
    if(PhaseState.Ring[0].SecondRemain == 0)
    {
        if(++PhaseState.Ring[0].SeqNum <= PhaseState.Ring[0].SeqMax)
        {
            if(PhaseState.Ring[0].SeqNum == 1)
            {
                if(OP.PlanRefreshFlag)
                {
                    OP.PlanRefreshFlag = 0;
                    NewPatternCheck();
                }
                if(OP.NewPatternFlag)
                {
                    #if DEBUG
                    printf("Startup All Red\r\n");
                    #endif
                    PhaseState.Ring[0].SeqNum = 1;
                    PhaseState.Ring[0].SecondRemain = Unit.StartupAllRed;
                    AutoAllRedMode(); 
                }
                else//否则无运行方案
                {
                    #if DEBUG
                    printf("Startup No Pattern\r\n");
                    #endif
                    PhaseState.Ring[0].SeqNum = 0;
                    OP.PlanRefreshFlag = 1;//重新检查有无方案
                    OLED_ShowString(48,0,"NP");
                }
            }
            else if(PhaseState.Ring[0].SeqNum == 2)
            {
                if(OP.NewPatternFlag)
                {
                    OP.NewPatternFlag = 0;
                    NewPatternApply();
                }
            }
        }
    }
}

uint8_t FindMiniTransitionRing(void)
{
    uint8_t i, ring = 0, MiniSecondRemainTime = 0xff;
    for(i = 0; i<RingMax; i++)
    {
        if(PhaseState.Ring[i].SeqMax > 0 && PhaseState.Ring[i].SecondRemain > 0)//非空环
        {
            if(MiniSecondRemainTime > PhaseState.Ring[i].SecondRemain)
            {
                ring = i;
                MiniSecondRemainTime = PhaseState.Ring[i].SecondRemain;
            }
        }
    }
    return ring;
}

uint8_t isInTransitionStep(void)
{
    //找到最小剩余时间的环
    uint8_t MiniRemainTimeRing = FindMiniTransitionRing();
    //剩余时间小于机动转换时间,自动切换到下一相位状态
    if(PhaseState.Ring[MiniRemainTimeRing].SecondRemain <= PhaseState.Ring[MiniRemainTimeRing].VehicleTransitionTime) return PhaseState.Ring[MiniRemainTimeRing].SecondRemain;
    return 0;
}

//返回0-无需切换  返回1-正在切换或者已切换信号
uint8_t ManualStepProcess(void)//手动模式
{
    uint8_t i, k=0, PhaseChangeBit = 0, MiniRemainTimeRing;
    
    //找到最小剩余时间的环
    MiniRemainTimeRing = FindMiniTransitionRing();
    //剩余时间小于机动转换时间,自动切换到下一相位状态
    if(PhaseState.Ring[MiniRemainTimeRing].SecondRemain <= PhaseState.Ring[MiniRemainTimeRing].VehicleTransitionTime) ManualCtrl.NextStepFlag = 1;
    if(ManualCtrl.NextStepFlag == 0)return 0;
    
    if(PhaseState.Ring[MiniRemainTimeRing].SecondRemain > PhaseState.Ring[MiniRemainTimeRing].VehicleTransitionTime + 1)
    {
        while(1)
        {
            for(i = 0; i < RingMax; i++)
            {
                if(PhaseState.Ring[i].SeqMax)
                {
                    if(PhaseState.Ring[i].SecondRemain) PhaseState.Ring[i].SecondRemain--;
                    if(i == MiniRemainTimeRing && PhaseState.Ring[i].SecondRemain <= (PhaseState.Ring[i].VehicleTransitionTime + 1)) k = 0x55;
                }
            }
            if(k)break;
        }
    }
    
    for(i = 0; i < RingMax; i++)
    {
        if(PhaseState.Ring[i].SeqMax)
        {
            if(PhaseState.Ring[i].SecondRemain) PhaseState.Ring[i].SecondRemain--;
            if(PhaseState.Ring[i].SecondRemain == 0)
            {
                if(PhaseState.Ring[i].SeqNum < PhaseState.Ring[i].SeqMax)
                {
                    PhaseState.Ring[i].SeqNum++;
                }
                if(PhaseState.Ring[i].SeqNum < PhaseState.Ring[i].SeqMax)
                {
                    PhaseState.Ring[i].PhaseChangeFlag = 1;
                    PhaseChangeBit = 1;
                    RingPhaseChange(i);
                }
                else 
                {
                    if(PhaseState.Ring[i].CycleOverFlag==0)
                    {
                        PhaseState.Ring[i].CycleOverFlag = 1;
                        PhaseChangeBit = 1;
                    }
                }
            }
        }
    }
    
    if(PhaseChangeBit)
    {
        PhaseState.StateNum++;
        ManualCtrl.NextStepFlag = 0;
    }
    
    if( PhaseState.Ring[0].CycleOverFlag & \
        PhaseState.Ring[1].CycleOverFlag & \
        PhaseState.Ring[2].CycleOverFlag & \
        PhaseState.Ring[3].CycleOverFlag)  //4个环相位都运行完毕
    {
        PhaseState.NewCycleFlag = 1; //新的周期
    }

    if(PhaseState.NewCycleFlag)//重新运行方案
    {
        PhaseState.NewCycleFlag = 0;
        for(i = 0; i < RingMax; i++)
        {
            if(PhaseState.Ring[i].SeqMax)
            {
                PhaseState.StateNum = 0;
                PhaseState.Ring[i].SeqNum = 0;
                PhaseState.Ring[i].CycleOverFlag = 0;
                PhaseState.Ring[i].PhaseChangeFlag = 1;
                RingPhaseChange(i);
            }
        }
    }
    return 1;
}


//暂定：指定放行只考虑机动车的最大转换时间,以后再考虑行人的安全过渡时间
void GetMaxiTransitionTime(uint32_t ChannelOns)
{
    uint8_t i;
    uint8_t ChannelTransitionTime = 0;
    uint8_t MaxiChannelTransitionPhase = 0;
    
    uint32_t ChannelMask = 0x1;
    ManualCtrl.MaxiChannelTransitionTime = 0;

    for(i = 0; i < ChannelTab.Maximum; i++)
    {
        if(ChannelOns & ChannelMask)//通道启用
        {
            uint8_t phaseNum, phaseIndex;
            phaseNum = ChannelTab.Channel[i].ControlSource;
            phaseIndex = GetPhaseIndex(&PhaseTab, phaseNum);
            if(ChannelTab.Channel[i].ControlType == CCT_VEHICLE)
            {
                ChannelTransitionTime = PhaseTab.Phase[phaseIndex].VehicleClear + PhaseTab.Phase[phaseIndex].YellowChange + PhaseTab.Phase[phaseIndex].RedClear;
                if(ManualCtrl.MaxiChannelTransitionTime < ChannelTransitionTime)
                {
                    ManualCtrl.MaxiChannelTransitionTime = ChannelTransitionTime;
                    MaxiChannelTransitionPhase = phaseNum;
                    #if DEBUG
                    printf("MaxiChannelTransitionPhase = %d\n",MaxiChannelTransitionPhase);
                    printf("ManualCtrl.MaxiChannelTransitionTime = %d\n",ManualCtrl.MaxiChannelTransitionTime);
                    #endif
                }
            }
            else if(ChannelTab.Channel[i].ControlType == CCT_PEDESTRIAN)
            {
                ChannelTransitionTime = PhaseTab.Phase[phaseIndex].PedestrianClear + PhaseTab.Phase[phaseIndex].RedClear;
                if(ManualCtrl.MaxiChannelTransitionTime < ChannelTransitionTime)
                {
                    ManualCtrl.MaxiChannelTransitionTime = ChannelTransitionTime;
                    MaxiChannelTransitionPhase = phaseNum;
                    #if DEBUG
                    printf("MaxiChannelTransitionPhase = %d\n",MaxiChannelTransitionPhase);
                    printf("ManualCtrl.MaxiChannelTransitionTime = %d\n",ManualCtrl.MaxiChannelTransitionTime);
                    #endif
                }
            }
        }
        ChannelMask <<= 1;
    }
}

void RemoteChannelStatusCtrl(void)
{
    uint8_t i;
    uint32_t ChannelMask = 0x1;

    for(i = 0; i < ChannelTab.Maximum; i++)
    {
        if(ChannelTab.Channel[i].ControlSource)
        {
            if(ManualCtrl.ChannelOnsNow & ChannelMask)//现在是绿灯状态
            {
                if((ManualCtrl.ChannelOnsNext & ChannelMask)==0)//下一步骤灭灯
                {
                    uint8_t phaseNum, phaseIndex;
                    phaseNum = ChannelTab.Channel[i].ControlSource;
                    phaseIndex = GetPhaseIndex(&PhaseTab, phaseNum);
                    
                    if(ChannelTab.Channel[i].ControlType == CCT_PEDESTRIAN)
                    {
                        if(ManualCtrl.Time > PhaseTab.Phase[phaseIndex].PedestrianClear + PhaseTab.Phase[phaseIndex].RedClear)
                        {
                            ChannelStatus.Greens    |= ChannelMask;
                            ChannelStatus.Reds      &=~ChannelMask;
                            ChannelStatus.Yellows   &=~ChannelMask; 
                            ChannelStatus.Flash     &=~ChannelMask;
                        }
                        else if(ManualCtrl.Time > PhaseTab.Phase[phaseIndex].RedClear)
                        {
                            ChannelStatus.Greens    |= ChannelMask;
                            ChannelStatus.Reds      &=~ChannelMask;
                            ChannelStatus.Yellows   &=~ChannelMask; 
                            ChannelStatus.Flash     |= ChannelMask;
                        }
                        else//全红过渡
                        {
                            ChannelStatus.Reds      |= ChannelMask;
                            ChannelStatus.Yellows   &=~ChannelMask;
                            ChannelStatus.Greens    &=~ChannelMask;
                            ChannelStatus.Flash     &=~ChannelMask;
                        }
                    }
                    else //if(ChannelTab.Channel[i].ControlType == CCT_VEHICLE)//其他控制类型全按照机动灯来控制
                    {
                        if(ManualCtrl.Time > PhaseTab.Phase[phaseIndex].VehicleClear + PhaseTab.Phase[phaseIndex].YellowChange + PhaseTab.Phase[phaseIndex].RedClear)
                        {
                            ChannelStatus.Greens    |= ChannelMask;
                            ChannelStatus.Reds      &=~ChannelMask;
                            ChannelStatus.Yellows   &=~ChannelMask; 
                            ChannelStatus.Flash     &=~ChannelMask;
                        }
                        else if(ManualCtrl.Time > PhaseTab.Phase[phaseIndex].YellowChange + PhaseTab.Phase[phaseIndex].RedClear)
                        {
                            ChannelStatus.Greens    |= ChannelMask;
                            ChannelStatus.Reds      &=~ChannelMask;
                            ChannelStatus.Yellows   &=~ChannelMask; 
                            ChannelStatus.Flash     |= ChannelMask;
                        }
                        else if(ManualCtrl.Time > PhaseTab.Phase[phaseIndex].RedClear)
                        {
                            ChannelStatus.Yellows   |= ChannelMask;
                            ChannelStatus.Reds      &=~ChannelMask;
                            ChannelStatus.Greens    &=~ChannelMask;
                            ChannelStatus.Flash     &=~ChannelMask;
                        }
                        else//全红过渡
                        {
                            ChannelStatus.Reds      |= ChannelMask;
                            ChannelStatus.Yellows   &=~ChannelMask;
                            ChannelStatus.Greens    &=~ChannelMask;
                            ChannelStatus.Flash     &=~ChannelMask;
                        }
                    }
                }
                else//下一步骤继续亮灯
                {
                    ChannelStatus.Greens    |= ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask; 
                    ChannelStatus.Flash     &=~ChannelMask;
                }
            }
            else//红灯
            {
                ChannelStatus.Reds      |= ChannelMask;
                ChannelStatus.Yellows   &=~ChannelMask;
                ChannelStatus.Greens    &=~ChannelMask;
                ChannelStatus.Flash     &=~ChannelMask;
            }
        }
        else//未启用黑灯
        {
            ChannelStatus.Greens    &=~ChannelMask;
            ChannelStatus.Reds      &=~ChannelMask;
            ChannelStatus.Yellows   &=~ChannelMask; 
            ChannelStatus.Flash     &=~ChannelMask;
        }
        ChannelMask <<= 1;
    }
}

/*
20220728: WorkPoint
1、检查所有当前运行相位
2、检查指定放行对应的相位

之前思路，根据通道的方位找相位，再根据相位放通道，就会产生相同相位不同方位的灯也放行了，所以思路有误。
变更为：根据通道的方位找通道，之后切换通道。
*/
void AppointCtrlProcess(void)//指定放行模式
{
    #if DEBUG
    printf("ManualCtrl.ManualCtrlTFlag = %d\n",ManualCtrl.RemoteCtrlFlag);
    printf("ManualCtrl.StartFlag = %d\n",ManualCtrl.StartFlag);
    printf("ManualCtrl.EnforceFlag = %d\n",ManualCtrl.EnforceFlag);
    printf("ManualCtrl.Time = %d\n",ManualCtrl.Time);
    #endif
    if(ManualCtrl.Time)ManualCtrl.Time--;
    
    if(ManualCtrl.EnforceFlag == 0)//
    {
        if(ManualCtrl.StartFlag == 1)//刚开始指定放行,无执行指令
        {
            ManualCtrl.Time = ManualCtrl.AutoTime;
            ManualCtrl.NextStepFlag = 0;
            if(ManualStepProcess() == 0)//过渡时间之前,就切换到下一步,并等待指令
            {
                ManualCtrl.StartFlag = 0;
                ManualCtrl.ChannelOnsNow = ChannelStatus.Greens;
                ManualCtrl.ChannelOnsBackup = ChannelStatus.Greens;
            }
            else//正处于安全过渡时间
            {
                OP.LampStateRefreshFlag = 1; //每1s刷新一次通道状态
                return;
            }
        }
        else
        {
            if(ManualCtrl.Time == ManualCtrl.MaxiChannelTransitionTime + 1)//无执行指令,自动递减到过渡步骤,就自动退出手动状态
            {
                ManualCtrl.ChannelOnsNext = ManualCtrl.ChannelOnsBackup;
            }
            if(ManualCtrl.Time == 0)
            {
                uint8_t i;
                ManualCtrl.RemoteCtrlFlag = 0;
                ManualCtrl.LocalCtrlFlag = 0;
                OP.WorkMode = OP.WorkModeBK;
                OP.WorkMode_Reason = WMR_Normal;
                for(i = 0; i<RingMax; i++)
                {
                    if(PhaseState.Ring[i].SeqMax > 0)//非空环
                    {
                        PhaseState.Ring[i].SecondRemain += 20;
                    }
                }
            }
        }
    }
    else//执行指定放行指令
    {
        if(ManualCtrl.StartFlag == 1)//刚开始指定放行,有执行指令————直接从自动运行转到指定放行
        {
            ManualCtrl.StartFlag = 0;
            ManualCtrl.Time = isInTransitionStep();
            if(ManualCtrl.Time==0)//非过渡模式
            {
                ManualCtrl.ChannelOnsBackup = ChannelStatus.Greens;
                ManualCtrl.Time = ManualCtrl.AutoTime;
            }
            else //过渡模式
            {
                ManualCtrl.ChannelOnsBackup = ChannelStatus.Greens | ChannelStatus.Yellows;
            }
        }
        
        if(ManualCtrl.OrderFlag)
        {
            ManualCtrl.OrderFlag = 0;
            ManualCtrl.ChannelOnsNow = ChannelStatus.Greens | ChannelStatus.Yellows;
            if(ManualCtrl.ChannelOnsNow != ManualCtrl.ChannelOnsNext)
            {
                ManualCtrl.MaxiChannelTransitionTime = 0;
                GetMaxiTransitionTime(ManualCtrl.ChannelOnsNow);
                if(ManualCtrl.Time > ManualCtrl.MaxiChannelTransitionTime + 1)
                {
                    ManualCtrl.Time = ManualCtrl.MaxiChannelTransitionTime + 1;
                }
            }
        }
        
        if(ManualCtrl.Time == 0)
        {
            ManualCtrl.Time = ManualCtrl.AutoTime;//时间到0
            ManualCtrl.EnforceFlag = 0;
            ManualCtrl.ChannelOnsNow = ManualCtrl.ChannelOnsNext;
        }
    }

    if(ManualCtrl.ExitFlag)
    {
        ManualCtrl.ExitFlag = 0;
        ManualCtrl.EnforceFlag = 0;
        ManualCtrl.ChannelOnsNow = ChannelStatus.Greens | ChannelStatus.Yellows;
        ManualCtrl.ChannelOnsNext = ManualCtrl.ChannelOnsBackup;
        if(ManualCtrl.ChannelOnsNow != ManualCtrl.ChannelOnsNext)
        {
            ManualCtrl.MaxiChannelTransitionTime = 0;
            GetMaxiTransitionTime(ManualCtrl.ChannelOnsNow);
            if(ManualCtrl.Time > ManualCtrl.MaxiChannelTransitionTime + 1)
            {
                ManualCtrl.Time = ManualCtrl.MaxiChannelTransitionTime + 1;
            }
        }
    }

    RemoteChannelStatusCtrl();  
}

void FixedTimeProcess(void)//定周期模式
{
    uint8_t i, PhaseChangeBit = 0;
    //检查各环的相位是否运行结束，结束置位环的 PhaseChangeFlag
    //检查各环是否运行结束，单个环结束置位环的 CycleOverFlag 
    //进一步检查 所有环的CycleOverFlag，来判断是否运行新的周期
    for(i = 0; i < RingMax; i++)
    {
        if(PhaseState.Ring[i].SeqMax)
        {
            if(PhaseState.Ring[i].SecondRemain) PhaseState.Ring[i].SecondRemain--;
            if(PhaseState.Ring[i].SecondRemain == 0)
            {
                if(PhaseState.Ring[i].SeqNum < PhaseState.Ring[i].SeqMax)
                {
                    PhaseState.Ring[i].SeqNum++;
                }
                if(PhaseState.Ring[i].SeqNum < PhaseState.Ring[i].SeqMax)
                {
                    PhaseState.Ring[i].PhaseChangeFlag = 1;
                    PhaseChangeBit = 1;
                    RingPhaseChange(i);
                }
                else 
                {
                    if(PhaseState.Ring[i].CycleOverFlag==0)
                    {
                        PhaseState.Ring[i].CycleOverFlag = 1;
                        PhaseChangeBit = 1;
                    }
                }
            }
        }
    }
    if(PhaseChangeBit) PhaseState.StateNum++;

    if( PhaseState.Ring[0].CycleOverFlag & \
        PhaseState.Ring[1].CycleOverFlag & \
        PhaseState.Ring[2].CycleOverFlag & \
        PhaseState.Ring[3].CycleOverFlag)  //4个环相位都运行完毕
    {
        PhaseState.NewCycleFlag = 1; //新的周期
    }

    if(PhaseState.NewCycleFlag)//重新运行方案
    {
        PhaseState.NewCycleFlag = 0;
        RunModeProcess();//新的周期判断方案是否改变
        
        if(OP.NewPatternFlag) //新的方案
        {
            OP.NewPatternFlag = 0;
            NewPatternApply();
        }
        else //不是新的方案
        {
            for(i = 0; i < RingMax; i++)
            {
                if(PhaseState.Ring[i].SeqMax)
                {
                    PhaseState.StateNum = 0;
                    PhaseState.Ring[i].SeqNum = 0;
                    PhaseState.Ring[i].CycleOverFlag = 0;
                    PhaseState.Ring[i].PhaseChangeFlag = 1;
                    RingPhaseChange(i);
                }
            }
        }
    }
}

void LineCtrlProcess(void)//定周期模式
{
    uint8_t i, PhaseChangeBit = 0, ChangeFlag = 0;
    //检查各环的相位是否运行结束，结束置位环的 PhaseChangeFlag
    //检查各环是否运行结束，单个环结束置位环的 CycleOverFlag 
    //进一步检查 所有环的CycleOverFlag，来判断是否运行新的周期
    for(i = 0; i < RingMax; i++)
    {
        if(PhaseState.Ring[i].SeqMax)
        {
            if(PhaseState.Ring[i].SecondRemain) PhaseState.Ring[i].SecondRemain--;
            if(PhaseState.Ring[i].SecondRemain == 0)
            {
                if(PhaseState.Ring[i].SeqNum < PhaseState.Ring[i].SeqMax)
                {
                    PhaseState.Ring[i].SeqNum++;
                }
                if(PhaseState.Ring[i].SeqNum < PhaseState.Ring[i].SeqMax)
                {
                    PhaseState.Ring[i].PhaseChangeFlag = 1;
                    PhaseChangeBit = 1;
                    if(RingPhaseChange(i))ChangeFlag = 1;
                }
                else 
                {
                    if(PhaseState.Ring[i].CycleOverFlag==0)
                    {
                        PhaseState.Ring[i].CycleOverFlag = 1;
                        PhaseChangeBit = 1;
                    }
                }
            }
        }
    }
    if(ChangeFlag) GreenWaveTimeChange();
    if(PhaseChangeBit) PhaseState.StateNum++;
    
    if(PhaseState.Ring[0].CycleOverFlag&PhaseState.Ring[1].CycleOverFlag&
       PhaseState.Ring[2].CycleOverFlag&PhaseState.Ring[3].CycleOverFlag)  //4个环相位都运行完毕
    {
        PhaseState.NewCycleFlag = 1; //新的周期
    }

    if(PhaseState.NewCycleFlag)//重新运行方案
    {
        PhaseState.NewCycleFlag = 0;
        RunModeProcess();//新的周期判断方案是否改变
        
        if(OP.NewPatternFlag) //新的方案
        {
            OP.NewPatternFlag = 0;
            NewPatternApply();
        }
        else //不是新的方案
        {
            for(i = 0; i < RingMax; i++)
            {
                if(PhaseState.Ring[i].SeqMax)
                {
                    PhaseState.StateNum = 0;
                    PhaseState.Ring[i].SeqNum = 0;
                    PhaseState.Ring[i].CycleOverFlag = 0;
                    PhaseState.Ring[i].PhaseChangeFlag = 1;
                    if(RingPhaseChange(i))ChangeFlag = 1;
                }
            }
            if(ChangeFlag) GreenWaveTimeChange();
            else GreenWaveTimeControl(); //不需调整,就重新计算是否匹配相位差
        }
    }
}

void FlashingProcess(void)//闪光模式
{
    PhaseState.NewCycleFlag = 0;
    //这里的算法使得冲突黄闪后,信号机自动恢复.
    //改变算法,因为冲突后,信号机黄闪,冲突就不存在了.
    if(OP.WorkMode_Reason == WMR_Normal)
    {
        RunModeProcess();//新的周期判断方案是否改变
        if(OP.WorkMode != PatternNow.WorkMode) OP.NewPatternFlag = 1;
        if(OP.NewPatternFlag) //新的方案
        {
            OP.NewPatternFlag = 0;
            NewPatternApply();
        }
    }
    AutoFlashMode();
}

void AllRedProcess(void)//全红模式
{
    PhaseState.NewCycleFlag = 0;
    RunModeProcess();//新的周期判断方案是否改变
    if(OP.NewPatternFlag) //新的方案
    {
        OP.NewPatternFlag = 0;
        NewPatternApply();
    }
    AutoAllRedMode();
}

void LampOffProcess(void)//关灯模式
{
    PhaseState.NewCycleFlag = 0;
    RunModeProcess();//新的周期判断方案是否改变
    if(OP.NewPatternFlag) //新的方案
    {
        OP.NewPatternFlag = 0;
        NewPatternApply();
    }
    AutoLampOffMode();
}

void SplitModeManage(void)//感应模式下绿信比模式管理
{
    uint8_t i, OmittedFlag;
    for(i = 0; i < RingMax; i++)
    {
        if((RingSplit[i].Coord & SC_FIXED) == 0)//非固定相位
        {
            if(RingSplit[i].Mode == SM_MinVehRecall)//最小响应
            {
                RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].MinimumGreen;
            }
            else if(RingSplit[i].Mode == SM_MaxVehRecall)//最大响应
            {
                RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].Maximum1;
            }
            else if(RingSplit[i].Mode == SM_MaxVehPedRecall)//最小响应
            {
                RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].Maximum1;
            }
            else if(RingSplit[i].Mode == SM_Omitted)//最小响应
            {
                RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].Maximum1;
            }
            else if(RingSplit[i].Mode == SM_PedRecall)//行人响应
            {
                RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].Maximum1;
            }
        }
    }
}

//感应模式下：
//机动策略
//1、剩余时间小于[单位延长绿+安全时间]，那么剩余时间[增加到][单位延长绿+安全时间],不是[增加][单位延长绿]+[安全时间]
//2、如果配置了[保持行人放行]，那么行人绿闪时间 应等于 机动安全时间
//3、

//行人策略
//1、先实现单环模式下的控制逻辑：有行人请求，才执行，无请求，不执行。
//2、思考多环控制模式下，各环时间与相位冲突的执行策略
//

//找出有请求的IO的序号，然后查行人检测器表对应的检测器号，获取请求的相位号。
//再判断相位号是否在环当中，且相位差模式为行人请求模式。如果满足条件即置位请求标志。
//
//PhaseStatus.PhaseNexts
//
//
//
//
//
//1、正在执行的相位时间完成，检查下一相位是否为行人请求相位
//2、判断该相位是否控制了机动灯组,控制了机动灯组则按照机动灯组执行,没有,则检查按钮生效情况.
//2、判断下相位当中行人相位对应的按钮是否有效
//3、按钮有效，执行相位，无效，跳过该相位。
//

uint8_t CheckNextPhaseSplitMode(uint8_t ring)//检查对应环下相位的绿信比模式
{

    return 0;
}

//0正常执行,1跳过下相位
uint8_t peddet_Control(uint8_t ring)
{
    uint8_t i, temp;

    //下相位是否配置了 pedestrianRecall
    #if DEBUG
    printf("Num = %d, Next = %d, Index = %d, NextIndex = %d, SplitIndex = %d, SplitNextIndex = %d\n",
            RingPhase[ring].PhaseNum,
            RingPhase[ring].PhaseNext,
            RingPhase[ring].PhaseIndex,
            RingPhase[ring].PhaseNextIndex,
            RingPhase[ring].SplitIndex,
            RingPhase[ring].SplitNextIndex
    );
    
    printf("Next Phase split.mode = %d\n",SplitNow.Phase[RingPhase[ring].SplitNextIndex].Mode);
    #endif

    if(SplitNow.Phase[RingPhase[ring].SplitNextIndex].Mode != SM_PedRecall) return 0;
    //下相位是否不为人行相位或者为机动相位
    if(isVehPhase(RingPhase[ring].PhaseNext) == 1 || isPedPhase(RingPhase[ring].PhaseNext) == 0) return 0;
    //至此,说明下相位为人行相位(无机动channel),且设置了 pedestrianRecall
    
    if(peddet_hw.rising & 0x7f)//第8号输入用于开门检测
    {
        temp = 0x01;
        for(i = 0; i < 8; i++)
        {
            if(peddet_hw.rising & temp)
            {
                uint8_t PeddetIndex, CallPhase;                
                if(PeddetTab.peddet[i].Num == (i + 1)) PeddetIndex = i;
                else { temp <<= 1; continue;}
                
                CallPhase = PeddetTab.peddet[PeddetIndex].CallPhase;
                if(CallPhase == 0){ temp <<= 1; continue;}

                if(RingPhase[ring].PhaseNext == CallPhase) 
                {
                    peddet_hw.rising &= ~temp;
                    return 0;
                }
            }
            temp <<= 1;
        }
    }
    //至此,说明下相位对应按钮无动作
    return 1;
}


void VehicleSenseProcess(void)//感应模式
{
    uint8_t i, PhaseChangeBit = 0;
    uint8_t VehicleClear, YellowChange, RedClear;
    uint8_t Walk, PedestrianClear;
    uint8_t RemainTime;
    for(i = 0; i < RingMax; i++)
    {
        if(PhaseState.Ring[i].SeqMax)
        {
            if(PhaseState.Ring[i].SecondRemain) PhaseState.Ring[i].SecondRemain--;
            if(PhaseState.Ring[i].SecondRemain == 0)
            {
                while(PhaseState.Ring[i].SeqNum + 1 < PhaseState.Ring[i].SeqMax)
                {
                    if(peddet_Control(i) == 1)//1跳过下相位
                    {
                        PhaseState.Ring[i].SeqNum++;
                        PhaseState.Ring[i].PhaseChangeFlag = 1;
                        RingPhaseChange(i);
                        PhaseState.Ring[i].SecondRemain = 0;
                    }
                    else break;
                }
                
                if(++PhaseState.Ring[i].SeqNum < PhaseState.Ring[i].SeqMax)
                {
                    PhaseState.Ring[i].PhaseChangeFlag = 1;
                    PhaseChangeBit = 1;
                    #if DEBUG
                    printf("SeqNum = %d\n", PhaseState.Ring[i].SeqNum);
                    #endif
                    RingPhaseChange(i);
                }
                else 
                {
                    if(PhaseState.Ring[i].CycleOverFlag==0)
                    {
                        PhaseState.Ring[i].CycleOverFlag = 1;
                        PhaseChangeBit = 1;
                    }
                }
            }
            else if(0)
            {
                VehicleClear = PhaseTab.Phase[RingPhase[i].PhaseIndex].VehicleClear;
                YellowChange = PhaseTab.Phase[RingPhase[i].PhaseIndex].YellowChange;
                RedClear = PhaseTab.Phase[RingPhase[i].PhaseIndex].RedClear;
                Walk = PhaseTab.Phase[RingPhase[i].PhaseIndex].Walk;
                PedestrianClear = PhaseTab.Phase[RingPhase[i].PhaseIndex].PedestrianClear;
                
                RemainTime = PhaseState.Ring[i].SecondRemain;
                if((RingSplit[i].Coord & SC_FIXED) == 0)//非固定相位
                {
                    if(RingSplit[i].Mode == SM_MinVehRecall)//最小响应
                    {
                        RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].MinimumGreen;
                    }
                    else if(RingSplit[i].Mode == SM_MaxVehRecall)//最大响应
                    {
                        RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].Maximum1;
                    }
                    else if(RingSplit[i].Mode == SM_MaxVehRecall)//最小响应
                    {
                        RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].Maximum1;
                    }
                    else if(RingSplit[i].Mode == SM_PedRecall)//行人响应
                    {
                        RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].Maximum1;
                    }
                }
                if(RemainTime > VehicleClear)
                    ;
            }
        }
    }
    
    if(PhaseChangeBit) PhaseState.StateNum++;
    if( PhaseState.Ring[0].CycleOverFlag & \
        PhaseState.Ring[1].CycleOverFlag & \
        PhaseState.Ring[2].CycleOverFlag & \
        PhaseState.Ring[3].CycleOverFlag)  //4个环相位都运行完毕
    {
        PhaseState.NewCycleFlag = 1; //新的周期
    }
    
    if(PhaseState.NewCycleFlag)//重新运行方案
    {
        PhaseState.NewCycleFlag = 0;
        RunModeProcess();//新的周期判断方案是否改变
        #if DEBUG
        printf("NewCycle\n");
        #endif
        if(OP.NewPatternFlag) //新的方案
        {
            OP.NewPatternFlag = 0;
            NewPatternApply();
        }
        else //不是新的方案
        {
            for(i = 0; i < RingMax; i++)
            {
                if(PhaseState.Ring[i].SeqMax)
                {
                    PhaseState.StateNum = 0;
                    //PhaseState.Ring[i].SeqNum = 0;
                    PhaseState.Ring[i].CycleOverFlag = 0;
                    //printf("SeqNum = %d\n", PhaseState.Ring[i].SeqNum);
                    while(1)
                    {
                        if(peddet_Control(i) == 1)//1跳过下相位
                        {
                            if(++PhaseState.Ring[i].SeqNum >= PhaseState.Ring[i].SeqMax)PhaseState.Ring[i].SeqNum = 0;
                            PhaseState.Ring[i].PhaseChangeFlag = 1;
                            RingPhaseChange(i);
                            PhaseState.Ring[i].SecondRemain = 0;
                            //printf("SeqNum = %d\n", PhaseState.Ring[i].SeqNum);
                        }
                        else break;
                    }
                    //printf("SeqNum = %d\n", PhaseState.Ring[i].SeqNum);
                    if(++PhaseState.Ring[i].SeqNum >= PhaseState.Ring[i].SeqMax)PhaseState.Ring[i].SeqNum = 0;
                    //printf("SeqNum = %d\n", PhaseState.Ring[i].SeqNum);
                    PhaseState.Ring[i].PhaseChangeFlag = 1;
                    RingPhaseChange(i);
                }
            }
        }
    }
}

/*
ErrorProcess: 系统错误处理程序
如果系统处于降级黄闪模式,那么判断降级的原因,,

*/
void ErrorProcess(void)
{


}


/*
    需要判断相位的绿信比模式，判断是否延长,是否切换等
*/
void RunPhaseTimeCalc(void)//10ms运行一次
{
    PhaseState.Phase10msCount++;
    if(OP.SendWorkModeAutoFlag == 1)//已联机
    {
        if(PhaseState.Phase10msCount == 1)
        {
            OP.SendWorkModeFlag = 1;
        }
        else if(PhaseState.Phase10msCount == 51 && ChannelStatus.Flash > 0)
        {
            OP.SendWorkModeFlag = 1;
        }
    }
    
    if(PhaseState.Phase1sFlag)
    {
        PhaseState.Phase1sFlag = 0;
        PhaseState.Phase10msCount = 0;
        manual_control();//手动控制功能
//        if(ManualCtrl.LocalCtrlFlag)//手动模式
//        {
//            if(OP.WorkMode == ManualFlashing) //手动闪光
//            {
//                AutoFlashMode();
//            }
//            else if(OP.WorkMode == ManualAllRead)//手动全红
//            {
//                AutoAllRedMode();
//            }
//            else if(OP.WorkMode == ManualLampOff)//手动关灯
//            {
//                AutoLampOffMode();
//            }
//            else if(OP.WorkMode == ManualStep)//手动
//            {
//                ManualStepProcess();
//                OP.LampStateRefreshFlag = 1;    //每1s刷新一次通道状态
//            }
//            return;
//        }
        
        if(ManualCtrl.RemoteCtrlFlag || ManualCtrl.LocalCtrlFlag)
        {
            if(OP.WorkMode == ManualAppoint)//手动
            {
                AppointCtrlProcess();
            }
            else
            {
                if(ManualCtrl.Time)
                {
                    if(--ManualCtrl.Time == 0)
                    {
                        ManualCtrl.RemoteCtrlFlag = 0;
                        ManualCtrl.LocalCtrlFlag = 0;
                        OP.WorkMode = OP.WorkModeBK;
                        OP.WorkMode_Reason = WMR_Normal;
                    }
                }
                if(OP.WorkMode == ManualFlashing) //手动闪光
                {
                    AutoFlashMode();
                }
                else if(OP.WorkMode == ManualAllRead)//手动全红
                {
                    AutoAllRedMode();
                }
                else if(OP.WorkMode == ManualLampOff)//手动关灯
                {
                    AutoLampOffMode();
                }
                else if(OP.WorkMode == ManualStep)//手动下一步
                {
                    ManualStepProcess();
                    OP.LampStateRefreshFlag = 1;    //每1s刷新一次通道状态
                }
            }
            return;
        }
        
        if(OP.WorkMode == FixedTime)    //定周期的执行策略
        {
            FixedTimeProcess();
            OP.LampStateRefreshFlag = 1;    //每1s刷新一次通道状态
        }
        else if(OP.WorkMode == LineCtrl)    //线协调(绿波)的执行策略
        {
            LineCtrlProcess();
            OP.LampStateRefreshFlag = 1;    //每1s刷新一次通道状态
        }
        else if(OP.WorkMode == VehicleSense)//感应的执行策略
        {
            VehicleSenseProcess();
            OP.LampStateRefreshFlag = 1;    //每1s刷新一次通道状态
        }
        else if(OP.WorkMode == Flashing)//闪光的执行策略
        {
            FlashingProcess();
        }
        else if(OP.WorkMode == AllRed)//全红的执行策略
        {
            AllRedProcess();
        }
        else if(OP.WorkMode == LampOff)//全红的执行策略
        {
            LampOffProcess();
        }
        else if(OP.WorkMode == StarupMode)//启动模式
        {
            StartupProcess();
        }
    }
}

void GetSeqTime(void)
{
    uint8_t     i = 0,step = 0,PhaseNum = 0,SplitPhaseIndex = 0,PhaseIndex = 0;
    uint16_t    CycleTime = 0;
    for(i = 0; i < RingMax; i++)
    {
        SeqTime.Ring[i].CycleTime = 0;
        for(step = 0; step < PhaseMax; step++)
        {
            PhaseNum = SequenceNow.Ring[i].Phase[step];
            if(IsPhase(PhaseNum))
            {
                PhaseIndex = GetPhaseIndex(&PhaseTab, PhaseNum);
                SplitPhaseIndex = GetSplitPhaseIndex(&SplitNow, PhaseNum);
                if(SplitPhaseIndex < PhaseMax)
                {
                    SeqTime.Ring[i].Time[step] = SplitNow.Phase[SplitPhaseIndex].Time;
                }
                else//绿信比表中未定义该相位,使用最小相位时间
                {
                    SeqTime.Ring[i].Time[step] = PhaseTab.Phase[PhaseIndex].MinimumGreen+PhaseTab.Phase[PhaseIndex].YellowChange+PhaseTab.Phase[PhaseIndex].RedClear;
                }
                PhaseState.Ring[i].VehicleTransitionTime = PhaseTab.Phase[PhaseIndex].VehicleClear + PhaseTab.Phase[PhaseIndex].YellowChange + PhaseTab.Phase[PhaseIndex].RedClear;
                SeqTime.Ring[i].CycleTime += SeqTime.Ring[i].Time[step];
            }
            else break;
        }
        #if DEBUG
            printf("SeqTime.Ring[%d].CycleTime = %d\r\n",i,SeqTime.Ring[i].CycleTime);
        #endif
        if(CycleTime < SeqTime.Ring[i].CycleTime) CycleTime = SeqTime.Ring[i].CycleTime;
    }

    NowCycleTime = CycleTime;
    //PatternNow.CycleTimeL = CycleTime&0xff;
    //PatternNow.CycleTimeH =(CycleTime>>8);
    #if DEBUG
    printf("NowCycleTime = %d\r\n",CycleTime);
    #endif
}

void GetPhaseStatusMap(void)
{
    uint8_t i;
    uint8_t ringTime[4] = {0};
    uint8_t ringStep[4] = {0};
    uint8_t ringPhase[4] = {0};
    uint16_t t;
    uint32_t PhaseMask = 0;
    uint32_t PhaseStatus = 0;
    
    for(i = 0; i < RingMax; i++)
    {
        if(PhaseState.Ring[i].SeqMax)
        {
            ringTime[i] = SeqTime.Ring[i].Time[0];
        }
    }
    
    PhaseState.StateNum = 0;
    PhaseState.State[0] = 0;//相位状态1
    
    for(t=1;t<NowCycleTime;t++)
    {
        PhaseMask = 0;
        for(i = 0; i < RingMax; i++)
        {
            if(PhaseState.Ring[i].SeqMax)
            {
                if(ringTime[i]) ringTime[i]--;
                if(ringTime[i] == 0)
                {
                    if(ringStep[i] < PhaseState.Ring[i].SeqMax)ringStep[i]++;
                    ringTime[i] = SeqTime.Ring[i].Time[ringStep[i]];
                }
                ringPhase[i] = SequenceNow.Ring[i].Phase[ringStep[i]];
                PhaseMask |= (0x1<<(ringPhase[i]-1));
            }
        }
        if(PhaseStatus != PhaseMask)
        {
            PhaseStatus = PhaseMask;
            PhaseState.State[PhaseState.StateNum] = PhaseMask;
            #if DEBUG
            printf("PhaseState[%d] = %04x\r\n",PhaseState.PhaseStatusNum,PhaseState.State[PhaseState.PhaseStatusNum]);
            #endif
            PhaseState.StateNum++;
        }
    }
    PhaseState.StateMax = PhaseState.StateNum;
    PhaseState.StateNum = 0;
    #if DEBUG
    printf("PhaseStatusMax = %d\r\n",PhaseState.StateMax);
    #endif
}

/*
    通过当前放行的相位号 相位参数 和 已放行时间, 判断计算各通道状态
*/
void LampStateControl(void)//1s运行一次
{
    if(OP.LampStateRefreshFlag)//灯态刷新
    {
        OP.LampStateRefreshFlag = 0;
        PhaseStatusControl();
        OverlapStatusControl();
        ChannelStatusControl();//根据PhaseStatus,驱动ChannelStatus
    }
}
