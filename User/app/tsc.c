/*
*********************************************************************************************************
*
* Module name: control de operación de la máquina de señales
* File name: tsc.c 
* Version : V1.0 
* illustrate : 
* Modification record: 
* Version Number Date Author Description 
* V1.0 2019-12-04 wcx first launch
*
*********************************************************************************************************
*/

#include "bsp.h" 				/* Controlador de hardware de bajo nivel */
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

* Definición de estructura de datos básica */
OPType              OP;
RtcType             Rtc;        //Hora actual de RTC
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
    memcpy(&rtc, &Rtc, sizeof(rtc));	// copia de seguridad hace un segundo
    
    Rtc.second++;
    if(OP.RtcIrqFlag == 1 || OP.StartFlag == 1 || OP.TimeChangeFlag == 1)
    {
        RtcReadTime();  //Leer tiempo RTC
        OP.Seconds = (Time.Hour * 3600) + (Time.Minute * 60) + Rtc.second;
        if(OP.StartFlag) OP.StartFlag = 0;
        if(OP.RtcIrqFlag) OP.RtcIrqFlag = 0;
        if(OP.TimeChangeFlag) OP.TimeChangeFlag = 0;
    }
    
    OP.ShowTimeFlag = 1;
    if(Rtc.day != rtc.day)          //Date carry, refresh once // Llevar fecha, actualizar una vez

        OP.PlanRefreshFlag = 1;    //Reinicio de actualización de número de tabla de períodos y todos los días, actualización una vez
    if(Rtc.minute != rtc.minute)
        OP.ActionRefreshFlag = 1;   //Acción Actualizar cada minuto, actualizar una vez
}

/********************************************************/
//Función: obtenga el número de horario consultando el horario de programación y determine si es necesario actualizar el horario
// Arrancar y ejecutar una vez al día a cero
void RunPlanRefresh(void)
{
    if(OP.PlanRefreshFlag)
    {
        OP.PlanRefreshFlag = 0;
        #if DEBUG
        printf("Plan Refresh \r\n");
        #endif
        if(SchedulePlanRefresh(&ScheduleTab, &Date))
            OP.ActionRefreshFlag = 1; //Después de actualizar la tabla de períodos, el período debe actualizarse
    }
}

/* Después de iniciar, el programa se ejecuta directamente sin juzgar los cambios de los parámetros operativos */
void NewPeriodApply(PlanType* DayPlan, TimeType* Time)
{
    uint8_t ActionNum, ActionIndex, PatternIndex;
    uint8_t temp = GetPeriodIndex(DayPlan, Time); // tabla de tiempo + periodo = índice de periodo
    if(temp < PeriodMax)//índice de período de tiempo
    {
        ActionNum = DayPlan->Period[temp].ActionNum;
        //Actualizar la información del período
        Period.Time.Hour = DayPlan->Period[temp].Time.Hour;
        Period.Time.Minute = DayPlan->Period[temp].Time.Minute;
        Period.ActionNum = ActionNum;
        ActionIndex = GetActionIndex(&ActionTab, ActionNum);
        
        if(ActionIndex < ActionMax)// acción no vacía
        {
            //Copy action data;
           //Funciones especiales, funciones de accesibilidad, actualizaciones directas
            memcpy(&Action, &ActionTab.Action[ActionIndex], sizeof(Action));
            
            PatternIndex = GetPatternIndex(&PatternTab, Action.Pattern);
            if(PatternIndex < PatternMax)
            {
                memcpy(&PatternNow, &PatternTab.Pattern[PatternIndex], sizeof(PatternNow));
                
                memcpy(&SequenceNow, &SeqTab.Seq[PatternNow.SequenceNum - 1], sizeof(SequenceNow));
                memcpy(&SplitNow, &SplitTab.Split[PatternNow.SplitNum - 1], sizeof(SplitNow));
                OP.NewPatternFlag = 1;//La nueva solución está lista, el ciclo está completo y se puede implementar
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

// Función: consultar la tabla de períodos de tiempo y obtener el número de acción de la hora actual
//Pasar parámetros: tabla de periodos + tiempo
void PlanActionRefresh(PlanType* DayPlan, TimeType* Time)
{
    uint8_t ActionIndex, PatternIndex;
    // tabla de tiempo + periodo = índice de periodo
    uint8_t PeriodIndex = GetPeriodIndex(DayPlan, Time);
    if(PeriodIndex < PeriodMax)
    {
        //Si el parámetro del período de tiempo cambia, actualice el parámetro del período de tiempo
        if(memcmp(&Period, &DayPlan->Period[PeriodIndex], sizeof(Period)) != 0)
        {
            #if DEBUG 
            printf("Period Changed\r\n");
            #endif
            memcpy(&Period, &DayPlan->Period[PeriodIndex], sizeof(Period));
        }
        
        ActionIndex = GetActionIndex(&ActionTab, Period.ActionNum);
        if(ActionIndex < ActionMax)// acción no vacía
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
            //Si se cambian los parámetros de secuencia de fase y relación de señal verde, no se actualizará aquí
            //Por lo tanto, si se cambian los parámetros de secuencia de fase y relación de señal verde, cambie la secuencia de patrón y la relación de señal verde para actualizar los parámetros operativos
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
        GetPhaseStateSeqMax();//Obtener el número de fase máximo de cada anillo, así como el valor máximo del número de fase y el número de anillo correspondiente a través de la tabla de continuidad
        GetSeqTime();//Obtenga el tiempo de secuencia de fase y recorra la tabla de secuencia de fase y la relación de señal verde
        
        if(OP.WorkMode == LineCtrl) GreenWaveTimeControl();//LineCtrl
        RunPhaseInit(&SequenceNow,  &SplitNow);
        GetPhaseStatusMap();
    }
    else if(OP.WorkMode == VehicleSense)//modo de inducción
    {
        GetPhaseStateSeqMax(); //Obtener el número de fase máximo de cada anillo, así como el valor máximo del número de fase y el número de anillo correspondiente a través de la tabla de continuidad
        GetSeqTime();           // Obtenga el tiempo de secuencia de fase y recorra la tabla de secuencia de fase y la relación de señal verde
        
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
    if(OP.ScheduleDataChangeFlag)//Los parámetros del plan de programación han cambiado
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
        //Después de actualizar la tabla de períodos, el período debe actualizarse
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

void manual_mange(void)//Ejecutar una vez en 10ms
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
            if(cmd & Value)// soltar el boton
            {
                if(cmd == 0x01)// cerrar manualmente
                {
                    ManualCtrl.KeyCmd = 0;
                }
            }
            else//botón presionado
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
    //bit0 es el interruptor manual, 0 enciende el manual, 1 apaga el manual
    //bit1 parpadeo amarillo
    if(ManualCtrl.KeyCmd < 0xff)//El control manual tiene acción.
    {
        if(ManualCtrl.LocalCtrlFlag == 0) //modo no manual
        {
            if(ManualCtrl.KeyCmd == MANUAL_ON)//TInterruptor de palanca, de automático a manual
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
            else if(ManualCtrl.KeyCmd == MANUAL_ClearError)//Borrar manualmente la falla
            {
                if(OP.WorkMode_Reason)
                {
                    if(WMR_RedGreenConflict == OP.WorkMode_Reason)
                    {
                        OP.red_green_conflict_reg = 0;//borrar falla
                        OP.WorkMode_Reason = WMR_Normal;//Haga que la máquina de señales parpadee en amarillo normalmente en lugar de parpadear en amarillo cuando ocurra una falla
                    }
                    if(WMR_GreenConflict == OP.WorkMode_Reason)
                    {
                        OP.green_conflict_reg = 0;//borrar fallo
                        OP.WorkMode_Reason = WMR_Normal;//Haga que la máquina de señales parpadee en amarillo normalmente en lugar de parpadear en amarillo cuando ocurra una falla
                    }
                    if(WMR_RedFailed == OP.WorkMode_Reason)
                    {
                        OP.red_install_reg = red_install_fail_detect(0);//borrar falla
                        OP.WorkMode_Reason = WMR_Normal;//Haga que la máquina de señales parpadee en amarillo normalmente en lugar de parpadear en amarillo cuando ocurra una falla
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
        else //modo manual
        {
            if(ManualCtrl.KeyCmd)
            {
                if(ManualCtrl.KeyCmd == MANUAL_FLASH)//luz amarilla
                {
                    OP.WorkMode = ManualFlashing;
                    OP.WorkMode_Reason = WMR_LocalManual;
                    ManualCtrl.Time = defaultAutoExitRemoteTime;
                    #if DEBUG
                    printf("ManualFlashing\r\n");
                    #endif
                }
                else if(ManualCtrl.KeyCmd == MANUAL_AllRed)//todo rojo
                {
                    OP.WorkMode = ManualAllRead;
                    OP.WorkMode_Reason = WMR_LocalManual;
                    ManualCtrl.Time = defaultAutoExitRemoteTime;
                    #if DEBUG
                    printf("ManualAllRead\r\n");
                    #endif
                }
                else if(ManualCtrl.KeyCmd == MANUAL_NextStep)//Próximo paso
                {
                    OP.WorkMode = ManualStep;
                    OP.WorkMode_Reason = WMR_LocalManual;
                    ManualCtrl.Time = defaultAutoExitRemoteTime;
                    ManualCtrl.NextStepFlag = 1;
                    #if DEBUG
                    printf("ManualNextStep\r\n");
                    #endif
                }
//                else if(ManualCtrl.KeyCmd == MANUAL_LampOff)//apagar las luces
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
            else//Interruptor de palanca, de manual a automático
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

//En el mismo grupo de luces, la detección de conflictos rojos y verdes 
//Regreso: 0-15 bit corresponde a un conflicto en el grupo de luces, 0 no tiene conflicto
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
            if(++c200ms[i] >= 10)//dura mas de 2 segundos
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
        if(PhaseStatus.Greens & temp_phase)//El grupo de luz ha sido liberado.
        {
            temp_release = PhaseStatus.Greens & (~temp_phase);
            phaseindex = GetPhaseIndex(&PhaseTab, i+1);
            temp_concurrency = PhaseTab.Phase[phaseindex].ConcurrencyL | (PhaseTab.Phase[phaseindex].ConcurrencyH<<8);
            temp_concurrency = (~temp_concurrency)&0xffff;
            if(temp_concurrency & temp_release)
            {
                if(++c200ms[i] >= 10)//dura mas de 2 segundos
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
        if(red_state & temp_var)//Una luz roja en una carretera tiene una salida
        {
            if(current_stab & temp_var)//y hay corriente
            {
                if(++c200ms[i] >= 10)//dura mas de 2 segundos
                {
                    c200ms[i] = 0;
                    red_install_reg |= temp_group;
                }
            }
            else if(red_install_reg & temp_group)//sin corriente e instalado
            {
                if(++c200ms[i] >= 10)//La falla dura más de 2 segundos
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
    
    if(reg1ms_flag) //Procesamiento de información de 1ms
    {
        reg1ms_flag = 0;
        lamp_state_detect();//1ms
        if(++reg10ms >= 10)//10ms
        {
            reg10ms = 0;
            OP.GetVehDetStaFlag = 1;
            peddet_scan(&peddet_hw);    //Detección de estado de entrada de botón de 8 vías
            rf315m_scan();              //entrada manual rf315M
            rf315m_mange();
            manual_mange();

            if(++reg200ms >= 20)//200ms
            {
                reg200ms = 0;
                fail_conflict_detect();
                if(++reg1s >= 5)//1s
                {
                    reg1s = 0;
                    PeddetStateGet();//Detector de peatones
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
    if(OP.Run10ms_flag) //Procesamiento de información de 10ms
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

//Control de onda verde, cálculo y control de tiempo de diferencia de fase
void GreenWaveTimeControl(void)
{
    OP.OffsetTimeModifier.OffsetTime = ((OP.Seconds + NowCycleTime - PatternNow.OffsetTime)%NowCycleTime);//Tiempo de desviación de fase
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
 * La fase actualmente en ejecución cambia
 * Reasignar datos de relación de fase y señal verde
 */
uint8_t RingPhaseChange(uint8_t ring)
{
    uint8_t ChangeFlag = 0,SeqNum,PhaseNum,PhaseIndex,PhaseNext,SplitPhaseIndex,SplitNextPhaseIndex;
    if(PhaseState.Ring[ring].PhaseChangeFlag)
    {
        PhaseState.Ring[ring].PhaseChangeFlag = 0;
        SeqNum = PhaseState.Ring[ring].SeqNum;
        
        //Aquí, según el número de fase, busque en la tabla para obtener el índice de fase y el índice de fase de relación de señal verde
        PhaseNum = SequenceNow.Ring[ring].Phase[SeqNum];
        if(SeqNum+1 < PhaseState.Ring[ring].SeqMax)
            PhaseNext = SequenceNow.Ring[ring].Phase[SeqNum+1];
        else
            PhaseNext = SequenceNow.Ring[ring].Phase[0];
        
        RingPhase[ring].PhaseIndex = GetPhaseIndex(&PhaseTab, PhaseNum);
        RingPhase[ring].PhaseNextIndex = GetPhaseIndex(&PhaseTab, PhaseNext);
        PhaseIndex = RingPhase[ring].PhaseIndex;
        
       //Por índice, copia la fase de ejecución y la relación de la señal verde
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

//Separar algunos parámetros de PhaseState y separar las funciones correspondientes wcx
void GetPhaseStateSeqMax(void)
{
    uint8_t ring;
    PhaseState.ValidRings = 0;
    PhaseState.CycleStepMax = 0;
    for(ring = 0; ring < RingMax; ring++)
    {
        PhaseState.Ring[ring].SeqMax = GetSeqMax(&SequenceNow.Ring[ring]);//Obtener el número de fases de este anillo
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
   Los datos de fase de la operación se inicializan, y los datos de fase y la relación de señal verde de los cuatro anillos en funcionamiento se obtienen a partir de los datos de secuencia de fase y relación de señal verde
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
    Autor de sistemas de software que no se basan en ningún hardware.
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

    LampDriveDataInit();//Inicialización y otros datos de la unidad y 13H
}


/*
 * Asignación PhaseState para iniciar la secuencia
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

void StartupProcess(void)//El modo de inicio
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
                else//De lo contrario, no hay esquema de ejecución.
                {
                    #if DEBUG
                    printf("Startup No Pattern\r\n");
                    #endif
                    PhaseState.Ring[0].SeqNum = 0;
                    OP.PlanRefreshFlag = 1;//Vuelva a consultar los planes
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
    // encuentra el ciclo con el mínimo tiempo restante
    uint8_t MiniRemainTimeRing = FindMiniTransitionRing();
   //El tiempo restante es menor que el tiempo de transición de maniobra, cambia automáticamente al siguiente estado de fase
    if(PhaseState.Ring[MiniRemainTimeRing].SecondRemain <= PhaseState.Ring[MiniRemainTimeRing].VehicleTransitionTime) return PhaseState.Ring[MiniRemainTimeRing].SecondRemain;
    return 0;
}

//regresar 0-no es necesario cambiar regresar 1-conmutación o señal conmutada
uint8_t ManualStepProcess(void)//modo manual
{
    uint8_t i, k=0, PhaseChangeBit = 0, MiniRemainTimeRing;
    
    //Encuentre el ciclo con el menor tiempo restante
    MiniRemainTimeRing = FindMiniTransitionRing();
    //Si el tiempo restante es menor que el tiempo de transición de maniobra, automáticamente cambiará al siguiente estado de fase
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
        PhaseState.Ring[3].CycleOverFlag)  //Se han ejecutado las 4 fases de anillo
    {
        PhaseState.NewCycleFlag = 1; //nuevo ciclo
    }

    if(PhaseState.NewCycleFlag)//escenario de repetición
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


//Tentativo: solo se considera el tiempo de transición máximo de los vehículos motorizados para el espacio libre designado, y el tiempo de transición de seguridad de los peatones se considerará más adelante
void GetMaxiTransitionTime(uint32_t ChannelOns)
{
    uint8_t i;
    uint8_t ChannelTransitionTime = 0;
    uint8_t MaxiChannelTransitionPhase = 0;
    
    uint32_t ChannelMask = 0x1;
    ManualCtrl.MaxiChannelTransitionTime = 0;

    for(i = 0; i < ChannelTab.Maximum; i++)
    {
        if(ChannelOns & ChannelMask)//canal habilitado
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
            if(ManualCtrl.ChannelOnsNow & ChannelMask)//Es una luz verde ahora
            {
                if((ManualCtrl.ChannelOnsNext & ChannelMask)==0)//siguiente paso luz apagada
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
                        else//transición roja completa
                        {
                            ChannelStatus.Reds      |= ChannelMask;
                            ChannelStatus.Yellows   &=~ChannelMask;
                            ChannelStatus.Greens    &=~ChannelMask;
                            ChannelStatus.Flash     &=~ChannelMask;
                        }
                    }
                    else //if(ChannelTab.Channel[i].ControlType == CCT_VEHICLE)//Otros tipos de control se controlan de acuerdo con las luces del motor
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
                        else//transición roja completa
                        {
                            ChannelStatus.Reds      |= ChannelMask;
                            ChannelStatus.Yellows   &=~ChannelMask;
                            ChannelStatus.Greens    &=~ChannelMask;
                            ChannelStatus.Flash     &=~ChannelMask;
                        }
                    }
                }
                else//El siguiente paso sigue iluminándose.
                {
                    ChannelStatus.Greens    |= ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask; 
                    ChannelStatus.Flash     &=~ChannelMask;
                }
            }
            else//luz roja
            {
                ChannelStatus.Reds      |= ChannelMask;
                ChannelStatus.Yellows   &=~ChannelMask;
                ChannelStatus.Greens    &=~ChannelMask;
                ChannelStatus.Flash     &=~ChannelMask;
            }
        }
        else//Luz negra no habilitada
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
1、Verifique todas las fases actualmente en ejecución
2、Verifique la fase correspondiente al lanzamiento especificado

La idea anterior era buscar la fase según el acimut del canal, y luego soltar el canal según la fase, y también se soltarían las luces con la misma fase y diferentes acimutes, así que la idea estaba mal.
Cambiar a: busque el canal de acuerdo con la dirección del canal y luego cambie el canal.
*/
void AppointCtrlProcess(void)//Especificar el modo de liberación
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
        if(ManualCtrl.StartFlag == 1)//Al comienzo del lanzamiento especificado, no hay orden de ejecución.
        {
            ManualCtrl.Time = ManualCtrl.AutoTime;
            ManualCtrl.NextStepFlag = 0;
            if(ManualStepProcess() == 0)//Antes del tiempo de transición, cambie al siguiente paso y espere las instrucciones
            {
                ManualCtrl.StartFlag = 0;
                ManualCtrl.ChannelOnsNow = ChannelStatus.Greens;
                ManualCtrl.ChannelOnsBackup = ChannelStatus.Greens;
            }
            else//iempo de transición seguro
            {
                OP.LampStateRefreshFlag = 1; //Actualizar el estado del canal cada 1s
                return;
            }
        }
        else
        {
            if(ManualCtrl.Time == ManualCtrl.MaxiChannelTransitionTime + 1)//Si no hay una instrucción de ejecución, se reducirá automáticamente al paso de transición y luego saldrá automáticamente del estado manual.
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
                    if(PhaseState.Ring[i].SeqMax > 0)//anillo no vacío
                    {
                        PhaseState.Ring[i].SecondRemain += 20;
                    }
                }
            }
        }
    }
    else//Ejecutar la orden de liberación especificada
    {
        if(ManualCtrl.StartFlag == 1)//Al comienzo de la liberación designada, hay una instrucción de ejecución --- directamente desde la operación automática hasta la liberación designada
        {
            ManualCtrl.StartFlag = 0;
            ManualCtrl.Time = isInTransitionStep();
            if(ManualCtrl.Time==0)//modo sin transición
            {
                ManualCtrl.ChannelOnsBackup = ChannelStatus.Greens;
                ManualCtrl.Time = ManualCtrl.AutoTime;
            }
            else //modo de transición
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
            ManualCtrl.Time = ManualCtrl.AutoTime;//tiempo a 0
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

void FixedTimeProcess(void)//Modo periódico
{
    uint8_t i, PhaseChangeBit = 0;
    //Compruebe si la fase de cada anillo ha terminado y establezca PhaseChangeFlag del anillo al final
    //Verifique si la operación de cada ciclo se completó y el CycleOverFlag del ciclo se establece al final de un solo ciclo
    //Verifique más el CycleOverFlag de todos los anillos para determinar si se debe ejecutar un nuevo ciclo
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
        PhaseState.Ring[3].CycleOverFlag)  //Se han ejecutado las 4 fases de anillo
    {
        PhaseState.NewCycleFlag = 1; //nuevo ciclo
    }

    if(PhaseState.NewCycleFlag)//escenario de repetición
    {
        PhaseState.NewCycleFlag = 0;
        RunModeProcess();//Si el nuevo ciclo juzga si el plan ha cambiado
        
        if(OP.NewPatternFlag) //Nuevo plan
        {
            OP.NewPatternFlag = 0;
            NewPatternApply();
        }
        else //no es una nueva solución
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

void LineCtrlProcess(void)//Modo periódico
{
    uint8_t i, PhaseChangeBit = 0, ChangeFlag = 0;
  //Compruebe si la fase de cada anillo ha terminado y establezca PhaseChangeFlag del anillo al final
    //Compruebe si cada ciclo ha terminado y establezca el CycleOverFlag del ciclo al final de un solo ciclo
    // Verifique más el CycleOverFlag de todos los anillos para determinar si se debe ejecutar un nuevo ciclo
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
       PhaseState.Ring[2].CycleOverFlag&PhaseState.Ring[3].CycleOverFlag)  //Se han ejecutado las 4 fases de anillo
    {
        PhaseState.NewCycleFlag = 1; //nuevo ciclo
    }

    if(PhaseState.NewCycleFlag)//escenario de repetición
    {
        PhaseState.NewCycleFlag = 0;
        RunModeProcess();//Si el nuevo ciclo juzga si el plan ha cambiado
        
        if(OP.NewPatternFlag) //Nuevo plan
        {
            OP.NewPatternFlag = 0;
            NewPatternApply();
        }
        else //no es una nueva solución
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
            else GreenWaveTimeControl(); //Recalcular si hacer coincidir la diferencia de fase sin ajuste
        }
    }
}

void FlashingProcess(void)//modo destello
{
    PhaseState.NewCycleFlag = 0;
    //El algoritmo aquí hace que la señal se recupere automáticamente después de que el conflicto parpadee en amarillo.
    //Cambie el algoritmo, porque después del conflicto, la máquina de señales parpadea en amarillo y el conflicto no existe.
    if(OP.WorkMode_Reason == WMR_Normal)
    {
        RunModeProcess();//Si el nuevo ciclo juzga si el plan ha cambiado
        if(OP.WorkMode != PatternNow.WorkMode) OP.NewPatternFlag = 1;
        if(OP.NewPatternFlag) //Nuevo plan
        {
            OP.NewPatternFlag = 0;
            NewPatternApply();
        }
    }
    AutoFlashMode();
}

void AllRedProcess(void)//modo rojo completo
{
    PhaseState.NewCycleFlag = 0;
    RunModeProcess();//Si el nuevo ciclo juzga si el plan ha cambiado
    if(OP.NewPatternFlag) //Nuevo plan
    {
        OP.NewPatternFlag = 0;
        NewPatternApply();
    }
    AutoAllRedMode();
}

void LampOffProcess(void)//modo de luz apagada
{
    PhaseState.NewCycleFlag = 0;
    RunModeProcess();//Si el nuevo ciclo juzga si el plan ha cambiado
    if(OP.NewPatternFlag) //Nuevo plan
    {
        OP.NewPatternFlag = 0;
        NewPatternApply();
    }
    AutoLampOffMode();
}

void SplitModeManage(void)//Gestión del modo relación señal verde en modo inducción
{
    uint8_t i, OmittedFlag;
    for(i = 0; i < RingMax; i++)
    {
        if((RingSplit[i].Coord & SC_FIXED) == 0)//fase no fija
        {
            if(RingSplit[i].Mode == SM_MinVehRecall)//respuesta mínima
            {
                RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].MinimumGreen;
            }
            else if(RingSplit[i].Mode == SM_MaxVehRecall)//respuesta máxima
            {
                RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].Maximum1;
            }
            else if(RingSplit[i].Mode == SM_MaxVehPedRecall)//Respuesta mínima
            {
                RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].Maximum1;
            }
            else if(RingSplit[i].Mode == SM_Omitted)//respuesta mínima
            {
                RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].Maximum1;
            }
            else if(RingSplit[i].Mode == SM_PedRecall)//respuesta peatonal
            {
                RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].Maximum1;
            }
        }
    }
}

//En modo de detección:  
// estrategia de maniobra  
// 1. El tiempo restante es menor que [extensión de la unidad verde + tiempo de seguridad], luego el tiempo restante [aumenta a] [extensión de la unidad verde + tiempo de seguridad], no [aumenta] [extensión de la unidad verde] + [tiempo de seguridad] 
// 2. Si se configura [Mantener peatones despejados], el tiempo de parpadeo verde de los peatones debe ser igual al tiempo de seguridad de maniobra 
//3,

//Estrategia peatonal 
// 1. Primero implemente la lógica de control en el modo de bucle único: solo ejecute si hay una solicitud de peatones, y no ejecute si no hay solicitud. 
// 2. Piense en la estrategia de implementación de los conflictos de tiempo y fase de cada anillo en el modo de control de bucle múltiple//

// Averigüe el número de serie del IO solicitado, luego verifique el número de detector correspondiente a la tabla de detectores de peatones y obtenga el número de fase solicitado。
// Luego juzgue si el número de fase está en el anillo, y el modo de diferencia de fase es el modo de solicitud de peatones. El indicador de solicitud se establece si se cumple la condición.
//
//PhaseStatus.PhaseNexts
//
//
//
//
//
// 1. El tiempo de la fase que se está ejecutando se completa, verifique si la siguiente fase es una fase de solicitud peatonal 
//2 Determinar si la fase controla el grupo de luces motorizado, si se controla el grupo de luces motorizado se ejecutará de acuerdo al
// grupo de luces motorizado, en caso contrario verificar el efecto del pulsador.
//2.Determinar si es valido el boton correspondiente a la fase peatonal en la fase inferior
// 3. El botón es válido, ejecuta la fase, inválido, salta la fase.
//

uint8_t CheckNextPhaseSplitMode(uint8_t ring)//Verifique el modo de relación de señal verde correspondiente a la fase debajo del anillo
{

    return 0;
}

//0 se ejecuta normalmente, 1 salta a la siguiente fase
uint8_t peddet_Control(uint8_t ring)
{
    uint8_t i, temp;

    //Si la fase inferior está configurada con peatonalRecall
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
  //Si la fase inferior no es una fase peatonal o una fase de maniobra
    if(isVehPhase(RingPhase[ring].PhaseNext) == 1 || isPedPhase(RingPhase[ring].PhaseNext) == 0) return 0;
    //Hasta ahora, muestra que la siguiente fase es la fase peatonal (sin canal de motor) y se ha configurado la llamada peatonal.
    
    if(peddet_hw.rising & 0x7f)//La entrada No. 8 se utiliza para la detección de puerta abierta
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
    //Hasta ahora se nota que el boton correspondiente a la fase inferior no tiene accion
    return 1;
}


void VehicleSenseProcess(void)//modo de inducción
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
                    if(peddet_Control(i) == 1)//1 salta la siguiente fase
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
                if((RingSplit[i].Coord & SC_FIXED) == 0)//fase no fija
                {
                    if(RingSplit[i].Mode == SM_MinVehRecall)//respuesta mínima
                    {
                        RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].MinimumGreen;
                    }
                    else if(RingSplit[i].Mode == SM_MaxVehRecall)//respuesta máxima
                    {
                        RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].Maximum1;
                    }
                    else if(RingSplit[i].Mode == SM_MaxVehRecall)//respuesta mínima
                    {
                        RingSplit[i].Time = PhaseTab.Phase[RingPhase[i].PhaseIndex].Maximum1;
                    }
                    else if(RingSplit[i].Mode == SM_PedRecall)//respuesta peatonal
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
        PhaseState.Ring[3].CycleOverFlag)  //Se han ejecutado las 4 fases de anillo
    {
        PhaseState.NewCycleFlag = 1; //nuevo ciclo
    }
    
    if(PhaseState.NewCycleFlag)//escenario de repetición
    {
        PhaseState.NewCycleFlag = 0;
        RunModeProcess();//Si el nuevo ciclo juzga si el plan ha cambiado
        #if DEBUG
        printf("NewCycle\n");
        #endif
        if(OP.NewPatternFlag) //Nuevo plan
        {
            OP.NewPatternFlag = 0;
            NewPatternApply();
        }
        else //no es una nueva solución
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
                        if(peddet_Control(i) == 1)//1 salta la siguiente fase
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
ErrorProcess: controlador de errores del sistema
Si el sistema está en el modo de parpadeo amarillo degradado, determine
el motivo de la degradación,,
*/
void ErrorProcess(void)
{


}


/*
    Es necesario juzgar el modo de relación de señal verde de la fase, juzgar si extender, si cambiar, etc.
*/
void RunPhaseTimeCalc(void)//Ejecutar una vez en 10ms
{
    PhaseState.Phase10msCount++;
    if(OP.SendWorkModeAutoFlag == 1)//en línea
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
        manual_control();//Función de control manual
//        if(ManualCtrl.LocalCtrlFlag)//modo manual
//        {
//            if(OP.WorkMode == ManualFlashing) //flash manual
//            {
//                AutoFlashMode();
//            }
//            else if(OP.WorkMode == ManualAllRead)//Manual todo rojo
//            {
//                AutoAllRedMode();
//            }
//            else if(OP.WorkMode == ManualLampOff)//Apagar manualmente las luces
//            {
//                AutoLampOffMode();
//            }
//            else if(OP.WorkMode == ManualStep)//manual
//            {
//                ManualStepProcess();
//                OP.LampStateRefreshFlag = 1;    //Actualizar el estado del canal cada 1s
//            }
//            return;
//        }
        
        if(ManualCtrl.RemoteCtrlFlag || ManualCtrl.LocalCtrlFlag)
        {
            if(OP.WorkMode == ManualAppoint)//manual
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
                if(OP.WorkMode == ManualFlashing) //flash manual
                {
                    AutoFlashMode();
                }
                else if(OP.WorkMode == ManualAllRead)//Manual todo rojo
                {
                    AutoAllRedMode();
                }
                else if(OP.WorkMode == ManualLampOff)//Apagar manualmente las luces
                {
                    AutoLampOffMode();
                }
                else if(OP.WorkMode == ManualStep)//siguiente paso manual
                {
                    ManualStepProcess();
                    OP.LampStateRefreshFlag = 1;    //Actualizar el estado del canal cada 1s
                }
            }
            return;
        }
        
        if(OP.WorkMode == FixedTime)    //Estrategia de ejecución periódica
        {
            FixedTimeProcess();
            OP.LampStateRefreshFlag = 1;    //Actualizar el estado del canal cada 1s
        }
        else if(OP.WorkMode == LineCtrl)    //Estrategia de Ejecución de Coordinación de Línea (Onda Verde)
        {
            LineCtrlProcess();
            OP.LampStateRefreshFlag = 1;    //Actualizar el estado del canal cada 1s
        }
        else if(OP.WorkMode == VehicleSense)//Detección de estrategias de ejecución
        {
            VehicleSenseProcess();
            OP.LampStateRefreshFlag = 1;    //Actualizar el estado del canal cada 1s
        }
        else if(OP.WorkMode == Flashing)//Estrategia de implementación flash
        {
            FlashingProcess();
        }
        else if(OP.WorkMode == AllRed)//La estrategia de ejecución de All Red
        {
            AllRedProcess();
        }
        else if(OP.WorkMode == LampOff)//La estrategia de ejecución de All Red
        {
            LampOffProcess();
        }
        else if(OP.WorkMode == StarupMode)//El modo de inicio
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
                else//La fase no está definida en la tabla de relación de señal verde, use el tiempo mínimo de fase
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
    PhaseState.State[0] = 0;//estado de fase 1
    
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
    Juzgar y calcular el estado de cada canal a través del número de fase liberado actual, los parámetros de fase y el tiempo liberado
*/
void LampStateControl(void)//Ejecutar una vez en 1s
{
    if(OP.LampStateRefreshFlag)//Actualización de estado de luz
    {
        OP.LampStateRefreshFlag = 0;
        PhaseStatusControl();
        OverlapStatusControl();
        ChannelStatusControl();//De acuerdo con PhaseStatus, conduce ChannelStatus
    }
}
