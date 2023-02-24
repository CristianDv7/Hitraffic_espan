#include <stdint.h>
#include "tsc.h"
#include "bsp.h"
#include "fm25v_map.h"


void DeviceInfoWrite(void)
{
    uint32_t WriteAddr = DeviceInfoAddr;
    uint32_t WriteSize = sizeof(DeviceInfo);
    Fm25v_Write(DeviceInfo.ManufacturerInfo, WriteAddr, WriteSize);
}

void BasicInfoWrite(void)
{
    uint32_t WriteAddr = BasicInfoAddr;
    uint32_t WriteSize = sizeof(BasicInfo);
    Fm25v_Write(BasicInfo.IntersectionInfo, WriteAddr, WriteSize);
}

void CoordWrite(void)
{
    uint32_t WriteAddr = CoordAddr;
    uint32_t WriteSize = sizeof(Coord);
    Fm25v_Write(&Coord.OperationalMode, WriteAddr, WriteSize);
}


void UnitWrite(void)
{
    uint32_t WriteAddr = UnitAddr;
    uint32_t WriteSize = sizeof(Unit);
    Fm25v_Write(&Unit.StartupFlash, WriteAddr, WriteSize);
}

void ScheduleWrite(void)
{
    uint32_t WriteAddr = ScheduleAddr;
    uint32_t WriteSize = sizeof(ScheduleTab);
    Fm25v_Write(&ScheduleTab.Maximum, WriteAddr, WriteSize);
}

void PlanWrite(void)
{
    uint32_t WriteAddr = PlanAddr;
    uint32_t WriteSize = sizeof(PlanTab);
    Fm25v_Write(&PlanTab.Maximum, WriteAddr, WriteSize);
}

void ActionWrite(void)
{
    uint32_t WriteAddr = ActionAddr;
    uint32_t WriteSize = sizeof(ActionTab);
    Fm25v_Write(&ActionTab.Maximum, WriteAddr, WriteSize);
}

void PatternWrite(void)
{
    uint32_t WriteAddr = PatternAddr;
    uint32_t WriteSize = sizeof(PatternTab);
    Fm25v_Write(&PatternTab.Maximum, WriteAddr, WriteSize);
}

void ChannelWrite(void)
{
    uint32_t WriteAddr = ChannelAddr;
    uint32_t WriteSize = sizeof(ChannelTab);
    Fm25v_Write(&ChannelTab.Maximum, WriteAddr, WriteSize);
}

void SplitWrite(void)
{
    uint32_t WriteAddr = SplitAddr;
    uint32_t WriteSize = sizeof(SplitTab);
    Fm25v_Write(&SplitTab.Maximum, WriteAddr, WriteSize);
}

void SequenceWrite(void)
{
    uint32_t WriteAddr = SequenceAddr;
    uint32_t WriteSize = sizeof(SeqTab);
    Fm25v_Write(&SeqTab.Maximum, WriteAddr, WriteSize);
}

void PhaseWrite(void)
{
    uint32_t WriteAddr = PhaseAddr;
    uint32_t WriteSize = sizeof(PhaseTab);
    Fm25v_Write(&PhaseTab.Maximum, WriteAddr, WriteSize);
}
void OverlapWrite(void)
{
    uint32_t WriteAddr = OverlapAddr;
    uint32_t WriteSize = sizeof(OverlapTab);
    Fm25v_Write(&OverlapTab.Maximum, WriteAddr, WriteSize);
}
void PeddetWrite(void)
{
    uint32_t WriteAddr = PeddetAddr;
    uint32_t WriteSize = sizeof(PeddetTab);
    Fm25v_Write(&PeddetTab.Maximum, WriteAddr, WriteSize);
}
void VehdetWrite(void)
{
    uint32_t WriteAddr = VehdetAddr;
    uint32_t WriteSize = sizeof(VehdetTab);
    Fm25v_Write(&VehdetTab.vehdet[0].Num, WriteAddr, WriteSize);
}
/******************************************************************************/

void ReadDeviceInfo(void)
{
    uint32_t ReadAddr = DeviceInfoAddr;
    uint32_t ReadSize = sizeof(DeviceInfo);
    Fm25v_FastRead(DeviceInfo.ManufacturerInfo, ReadAddr, ReadSize);
}
void ReadBasicInfo(void)
{
    uint32_t ReadAddr = BasicInfoAddr;
    uint32_t ReadSize = sizeof(BasicInfo);
    Fm25v_FastRead(BasicInfo.IntersectionInfo, ReadAddr, ReadSize);
}
void ReadUnit(void)
{
    uint32_t ReadAddr = UnitAddr;
    uint32_t ReadSize = sizeof(Unit);
    Fm25v_FastRead(&Unit.StartupFlash, ReadAddr, ReadSize);
}
void ReadCoord(void)
{
    uint32_t ReadAddr = CoordAddr;
    uint32_t ReadSize = sizeof(Coord);
    Fm25v_FastRead(&Coord.OperationalMode, ReadAddr, ReadSize);
}
void ReadSchedule(void)
{
    uint32_t ReadAddr = ScheduleAddr;
    uint32_t ReadSize = sizeof(ScheduleTab);
    Fm25v_FastRead(&ScheduleTab.Maximum, ReadAddr, ReadSize);
}
void ReadPlan(void)
{
    uint32_t ReadAddr = PlanAddr;
    uint32_t ReadSize = sizeof(PlanTab);
    Fm25v_FastRead(&PlanTab.Maximum, ReadAddr, ReadSize);
}
void ReadAction(void)
{
    uint32_t ReadAddr = ActionAddr;
    uint32_t ReadSize = sizeof(ActionTab);
    Fm25v_FastRead(&ActionTab.Maximum, ReadAddr, ReadSize);
}
void ReadPattern(void)
{
    uint32_t ReadAddr = PatternAddr;
    uint32_t ReadSize = sizeof(PatternTab);
    Fm25v_FastRead(&PatternTab.Maximum, ReadAddr, ReadSize);
}
void ReadChannel(void)
{
    uint32_t ReadAddr = ChannelAddr;
    uint32_t ReadSize = sizeof(ChannelTab);
    Fm25v_FastRead(&ChannelTab.Maximum, ReadAddr, ReadSize);
}
void ReadSplit(void)
{
    uint32_t ReadAddr = SplitAddr;
    uint32_t ReadSize = sizeof(SplitTab);
    Fm25v_FastRead(&SplitTab.Maximum, ReadAddr, ReadSize);
}
void ReadSequence(void)
{
    uint32_t ReadAddr = SequenceAddr;
    uint32_t ReadSize = sizeof(SeqTab);
    Fm25v_FastRead(&SeqTab.Maximum, ReadAddr, ReadSize);
}

void ReadPhase(void)
{
    uint32_t ReadAddr = PhaseAddr;
    uint32_t ReadSize = sizeof(PhaseTab);
    Fm25v_FastRead(&PhaseTab.Maximum, ReadAddr, ReadSize);
}

void ReadOverlap(void)
{
    uint8_t  i;
    uint32_t ReadAddr = OverlapAddr;
    uint32_t ReadSize = sizeof(OverlapTab);
    Fm25v_FastRead(&OverlapTab.Maximum, ReadAddr, ReadSize);
    for(i=0;i<OverlapMax;i++)
    {
        IncludedPhases[i] = OverlapTab.Overlap[i].IncludedPhases[0]|(OverlapTab.Overlap[i].IncludedPhases[1]<<8);//Fase principal Fase de vehículos de motor de la película estrenada
        ModifierPhases[i] = OverlapTab.Overlap[i].ModifierPhases[0]|(OverlapTab.Overlap[i].ModifierPhases[1]<<8);
    }
}

void ReadPeddet(void)
{
    uint32_t ReadAddr = PeddetAddr;
    uint32_t ReadSize = sizeof(PeddetTab);
    Fm25v_FastRead(&PeddetTab.Maximum, ReadAddr, ReadSize);
}

void ReadVehdet(void)
{
    uint32_t ReadAddr = VehdetAddr;
    uint32_t ReadSize = sizeof(VehdetTab);
    Fm25v_FastRead(&VehdetTab.vehdet[0].Num, ReadAddr, ReadSize);
}
//***************************************************************************

void ParameterTurnToDefault(void)
{
    DeviceInfoInit();    /*Informacion del fabricante, version*/
    DeviceInfoWrite();	 /*Escribe la informacion basica cargada siempre que se haya cambiado StartCode1 o 2*/
    
    BasicInfoInit();		/*Informacion de la IP*/
    BasicInfoWrite();		/*Escribe la informacion siempre que se haya cambiado el codigo StartCode1 o 2*/
    
    CoordInit();				/*Ingresa coordenadas*/
    CoordWrite();    
    
    UnitInit();
    UnitWrite();
    
    ScheduleDefault();	
    ScheduleWrite();
    
    PlanDefault();
    PlanWrite();
    
    ActionDefault();
    ActionWrite();

    PatternDefault();
    PatternWrite();
    
    //The green letter ratio data is initialized to factory settings
    SplitDefault();
    SplitWrite();
    
    SequenceDefault();
    SequenceWrite();

    PhaseDefault();
    PhaseWrite();
    
    //Channel data is initialized to factory settings
    ChannelInit();
    ChannelWrite();
    
    OverlapInit();
    OverlapWrite();
    
    PeddetInit();
    PeddetWrite();
    
    VehicleDetectorInit();
    VehdetWrite();
}

void ParameterRead(void)
{
    ReadDeviceInfo();
    ReadBasicInfo();
    ReadUnit();
    ReadCoord();
    ReadSchedule();
    ReadPlan();
    ReadAction();
    ReadPattern();
    ReadSplit();
    ReadSequence();
    ReadPhase();
    ReadChannel();
    ReadOverlap();
    ReadPeddet();
    ReadVehdet();
}

#define StartCode1  0x55
#define StartCode2  0xAB    /*Cambio de dato para tomar los valores de las variables*/

void Write_StartCode(void)
{
    uint8_t StartCode[16] = { StartCode1,StartCode2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,};
    Fm25v_Write(StartCode, StartCodeAddr, 16);
}

//verifique el código de arranque,
uint8_t Check_StartCode(void)
{
    uint8_t StartCode[2];
    Fm25v_FastRead(StartCode, StartCodeAddr, 2);
    //#if DEBUG
    printf("StartCode = %2x %2x\r\n",StartCode[0],StartCode[1]);
    //#endif
    
    if(StartCode[0] != StartCode1 || StartCode[1] != StartCode2)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void StartProcess(void)
{
    BEEP_ON();					/*Enciende el pin del buzzer*/
		bsp_DelayMS(1000);
		extern void interrup(void);
	
    if(Check_StartCode()==0)
    {
        #if DEBUG
        printf("restore default parameters");
        #endif
        ParameterTurnToDefault();
        Write_StartCode();
        OLED_ShowString(24,0,"DP ");
        ReadDeviceInfo();
        ReadBasicInfo();
    }
    else 
    {
        #if DEBUG
        printf("read parameters\r\n");
        #endif
        ParameterRead();
        OLED_ShowString(24,0,"RP ");
    }
    PeddetStateInit();
    BEEP_OFF();				/*Apaga el buzzer*/
}

