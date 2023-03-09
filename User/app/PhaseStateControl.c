#include "PhaseStateControl.h"
#include "Channel.h"
#include "Phase.h"
#include "Split.h"
#include "Overlap.h"

/* Datos de estado de fase de cada anillo actualmente en ejecución */
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
 * Tiempo de fase verde
 * Juzgar el estado de todas las fases a través del estado efectivo y el número de fase de los 4 anillos y el tiempo de liberación
 * PhaseStatus
 */
void PhaseGreenCount(void)//Ejecutar una vez en 1s
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
 * Control de estado de fase
 * Juzgar el estado de todas las fases a través del estado efectivo y el número de fase de los 4 anillos y el tiempo de liberación
 * PhaseStatus
 */
void PhaseStatusControl(void)//Ejecutar una vez en 1s
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
    
    for(i = 0; i < RingMax; i++)//Si el número de fase de los 4 anillos tiene la fuente de control del canal actual
    {
        if(PhaseState.Ring[i].SeqMax == 0) continue;//El anillo no está habilitado.
        
        PhaseNum = RingPhase[i].PhaseNum;
        PhaseIndex = RingPhase[i].PhaseIndex;
        PhaseMask = (0x01 << (PhaseNum-1));
        PhaseStatus.PhaseOns |= PhaseMask;
        
        //Algoritmo de estado de maniobra
        if(PhaseState.Ring[i].SecondRemain > PhaseTab.Phase[PhaseIndex].YellowChange + PhaseTab.Phase[PhaseIndex].RedClear)//motor verde
        {
            PhaseStatus.Greens  |=  PhaseMask;
            PhaseStatus.Yellows &= ~PhaseMask;
            PhaseStatus.Reds    &= ~PhaseMask;
            
            if(PhaseState.Ring[i].SecondRemain > PhaseTab.Phase[PhaseIndex].VehicleClear + PhaseTab.Phase[PhaseIndex].YellowChange + PhaseTab.Phase[PhaseIndex].RedClear)
                PhaseStatus.VehClears &= ~PhaseMask;
            else //Flash verde móvil
                PhaseStatus.VehClears |=  PhaseMask;
        }
        else if(PhaseState.Ring[i].SecondRemain > PhaseTab.Phase[PhaseIndex].RedClear)//Motor amarillo
        {
            PhaseStatus.Greens  &= ~PhaseMask;
            PhaseStatus.Yellows |=  PhaseMask;
            PhaseStatus.Reds    &= ~PhaseMask;
        }
        else//Móvil Rojo
        {
            PhaseStatus.Reds        |=  PhaseMask;
            PhaseStatus.Greens      &= ~PhaseMask;
            PhaseStatus.Yellows     &= ~PhaseMask;
            PhaseStatus.VehClears   &= ~PhaseMask;
        }
        
        //Algoritmo de estado de peatones
        if(PhaseTab.Phase[PhaseIndex].OptionsH & 0x20)//Mantenga a los peatones alejados
        {
            if(PhaseStatus.Greens & PhaseMask)
            {
                if(PhaseStatus.VehClears & PhaseMask) //destello verde
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
        else //Mantenga a los peatones alejados sin hacer tictac
        {
            if(RingSplit[i].Time >= (PhaseTab.Phase[PhaseIndex].Walk + PhaseTab.Phase[PhaseIndex].PedestrianClear + PhaseTab.Phase[PhaseIndex].RedClear))
            {
                if((RingSplit[i].Time - PhaseState.Ring[i].SecondRemain) < PhaseTab.Phase[PhaseIndex].Walk) //<= a < tendrá una diferencia de 1 segundo
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
            else //Cuando no marca Mantener espacio libre para peatones, el tiempo de espacio libre para peatones establecido es demasiado largo
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

//seguir el control de estado de fase
void OverlapStatusControl(void) //1S actualizar una vez más
{
    uint8_t i;
    uint32_t    OverlapMask = 0x1;
    for(i = 0; i < OverlapMax; i++)//Atraviesa todas las fuentes de control de canales
    {
        if(OverlapTab.Overlap[i].Num > 0 && OverlapTab.Overlap[i].Num <= OverlapMax)
        {
            OverlapMask = (0x1<<(OverlapTab.Overlap[i].Num - 1));
            if(OverlapTab.Overlap[i].Type == OT_NORMAL || (OverlapTab.Overlap[i].Type == OT_MINUSGREENYELLOW && ModifierPhases[i] == 0))//Sigue las reglas
            {
                //printf("Overlap[%d].Type = OT_NORMAL",i);
                //printf("IncludedPhases[i] = %d",IncludedPhases[i]);
                //printf("PhaseOns = %d",PhaseStatus.PhaseOns);
                //printf("Overlap[%d] is active ",i);
                //printf("Overlap[%d].Num = %d is active ",i,OverlapTab.Overlap[i].Num);
                
                if(IncludedPhases[i] & PhaseStatus.PhaseOns)
                {
                    if(OverlapTab.Overlap[i].TrailGreen == 0)//Necesidad de juzgar si PhaseNexts es también la fase madre
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
                    else//El tiempo de luz verde para continuar pasando la luz verde al final de la fase madre
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
                else//madre fuera de fase
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
            {   //Cuando la fase principal es verde y la fase de corrección no es verde, la siguiente fase es verde
                if((ModifierPhases[i] & PhaseStatus.Greens)==0)//Cuando la fase corregida no es verde
                {
                    if((IncludedPhases[i] & PhaseStatus.Greens))//cuando el aspecto de la madre es verde
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
                    //Cuando cualquiera de las fases principales es amarilla, la siguiente fase no es la fase principal y la fase de corrección no es amarilla, la siguiente fase es amarilla.
                    else if((IncludedPhases[i] & PhaseStatus.Yellows))//cuando el aspecto de la madre es amarillo
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

//Actualización del estado del canal
void ChannelStatusControl(void)//Actualizar una vez en 1S
{
    uint8_t  i;
    uint32_t PhaseMask;
    uint32_t ChannelMask = 0x1;
    
    ChannelStatus.Reds      = 0;
    ChannelStatus.Yellows   = 0;
    ChannelStatus.Greens    = 0;
    ChannelStatus.Flash     = 0;
    
    for(i = 0; i < ChannelMax; i++)//Atraviesa todas las fuentes de control de canales
    {
     
//El número de fase es 0 o el tipo de control es otro medio Sin control (no en uso)
        //if(ChannelTab.Channel[i].ControlSource == 0 || ChannelTab.Channel[i].ControlType == CCT_OTHER) continue;
        if(ChannelTab.Channel[i].ControlSource == 0)
        {
            ChannelMask <<= 1;
            continue;
        }
        PhaseMask = (0x1 << (ChannelTab.Channel[i].ControlSource-1));
        
        if(ChannelTab.Channel[i].ControlType == CCT_VEHICLE) //El tipo de control del vehículo es un vehículo de motor
        {
            if((PhaseStatus.PhaseOns & PhaseMask) == 0) //La fase de fuente de control correspondiente de este canal está cerrada
            {
                ChannelStatus.Reds      |= ChannelMask;
                ChannelStatus.Yellows   &=~ChannelMask;
                ChannelStatus.Greens    &=~ChannelMask;
                ChannelStatus.Flash     &=~ChannelMask;
            }
            else //La fase de fuente de control correspondiente del canal está encendida
            {
                if(PhaseStatus.Reds & PhaseMask) //El estado de la fase es rojo
                {
                    ChannelStatus.Reds      |= ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;
                    ChannelStatus.Greens    &=~ChannelMask;
                    ChannelStatus.Flash     &=~ChannelMask;
                }
                else if(PhaseStatus.Yellows & PhaseMask) //El estado de la fase es amarillo
                {
                    ChannelStatus.Yellows   |= ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Greens    &=~ChannelMask;
                    ChannelStatus.Flash     &=~ChannelMask;
                }
                else if(PhaseStatus.Greens & PhaseMask) //El estado de la fase es verde
                {
                    ChannelStatus.Greens    |= ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;                
                    if(PhaseStatus.VehClears & PhaseMask)
                        ChannelStatus.Flash |= ChannelMask;
                    else
                        ChannelStatus.Flash &=~ChannelMask;
                }
                else //Estado de fase apagado
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
                if(PhaseStatus.DontWalks & PhaseMask)//El estado de la fase es rojo
                {
                    ChannelStatus.Reds      |= ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;
                    ChannelStatus.Greens    &=~ChannelMask;
                    ChannelStatus.Flash     &=~ChannelMask;
                }
                else if(PhaseStatus.PedClears & PhaseMask)//El estado de la fase parpadea en verde
                {
                    ChannelStatus.Greens    |= ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;                
                    ChannelStatus.Flash     |= ChannelMask;
                }
                else if(PhaseStatus.Walks & PhaseMask)//El estado de la fase es verde
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
            if(OverlapStatus.Reds & PhaseMask)//El estado de la fase es rojo
            {
                ChannelStatus.Reds      |= ChannelMask;
                ChannelStatus.Yellows   &=~ChannelMask;
                ChannelStatus.Greens    &=~ChannelMask;
                ChannelStatus.Flash     &=~ChannelMask;
            }
            else if(OverlapStatus.Yellows & PhaseMask)//El estado de la fase es amarillo
            {
                ChannelStatus.Yellows   |= ChannelMask;
                ChannelStatus.Reds      &=~ChannelMask;
                ChannelStatus.Greens    &=~ChannelMask;
                ChannelStatus.Flash     &=~ChannelMask;
            }
            else if(OverlapStatus.Greens & PhaseMask)//El estado de la fase es verde
            {
                ChannelStatus.Greens    |= ChannelMask;
                ChannelStatus.Yellows   &=~ChannelMask;  
                ChannelStatus.Reds      &=~ChannelMask;
                if(OverlapStatus.Flashs & PhaseMask)
                    ChannelStatus.Flash |= ChannelMask;
                else
                    ChannelStatus.Flash &=~ChannelMask;
            }
            else//Estado de fase apagado
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
            if(ChannelTab.Channel[i].Flash == CFM_Yellow)          //luz amarilla
            {
                ChannelStatus.Yellows |= ChannelMask;
            }
            else if(ChannelTab.Channel[i].Flash == CFM_Red)        //Rojo
            {
                ChannelStatus.Reds |= ChannelMask;
            }
            else if(ChannelTab.Channel[i].Flash == CFM_Alternate)  //alternativamente
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
                if(PhaseStatus.Reds & PhaseMask)//El estado de la fase es rojo
                {
                    ChannelStatus.Reds      |= ChannelMask;
                    ChannelStatus.Yellows   &=~ChannelMask;
                    ChannelStatus.Greens    &=~ChannelMask;
                    ChannelStatus.Flash     &=~ChannelMask;
                }
                else if(PhaseStatus.Yellows & PhaseMask)//El estado de la fase es amarillo
                {
                    ChannelStatus.Yellows   |= ChannelMask;
                    ChannelStatus.Reds      &=~ChannelMask;
                    ChannelStatus.Greens    &=~ChannelMask;
                    ChannelStatus.Flash     &=~ChannelMask;
                }
                else if(PhaseStatus.Greens & PhaseMask)//El estado de la fase es verde
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





