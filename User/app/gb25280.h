#ifndef __GB25280_H
#define __GB25280_H
#include <stdint.h>

#define Frame_right         0xa0
#define Frame_wrong         0xa1
#define CheckSum_failed     0xa2
#define Unsupport_OT        0xa3
#define Unsupport_DL        0xa4
#define Unsupport_OB        0xa5
#define DataError           0xa6

//0 encabezado de trama - 1 versi髇 - 2 enviar - 3 recibir - 4 enlace - 5 醨ea - 6 intersecci髇 alta - 7 intersecci髇 baja - 8 operaci髇 - 9 objeto - 10 reservado - * datos * - verificaci髇 - fin de trama
//Definici髇 de datos del protocolo de comunicaci髇
#define frame_head  0xC0
#define frame_tail  0xC0
#define frame_ver   0x20
#define ID_PC       0x20
#define ID_TSC      0x10




//data link code 数据链路码
#define DL_Com              0x01
#define DL_BasicInfo        0x02
#define DL_CharacPar        0x03
#define DL_Intervention     0x04

//Operation type操作类型
#define OT                  0x80
#define OT_Query            0x80
#define OT_Set              0x81
#define OT_InitiativeReport 0x82
#define OT_QueryAnswer      0x83
#define OT_SetAnswer        0x84
#define OT_ErrorAnswer      0x85

//Object对象标识
#define O_Linking         0x01
#define O_TrafficFlow     0x02
#define O_WorkState       0x03
#define O_SignalState     0x04
#define O_CurrentTime     0x05
#define O_Channel         0x06
#define O_Phase           0x07
#define O_Pattern         0x08
#define O_Schedule        0x09
#define O_WorkMode        0x0a
#define O_Error           0x0b
#define O_Version         0x0c
#define O_CharacParVer    0x0d
#define O_IDCODE          0x0e
#define O_RemoteControl   0x0f
#define O_Vehdet          0x10
#define O_Plan            0x11
#define O_Action          0x12
#define O_Sequence        0x13
#define O_Split           0x14
#define O_Unit            0x15
#define O_Coord           0x16
#define O_Overlap         0x17
#define O_Peddet          0x18 

#define O_PeddetState     0x28

#define O_Alarm           0xA0
#define O_BasicInfo       0xBD
#define O_DeviceInfo      0xBE
#define O_Call            0xBF

#define TypeAlarm_Light         0x10
#define TypeAlarm_Detetor       0x30
#define TypeAlarm_Device        0x40
#define TypeAlarm_Environment   0x60

#define DoorState       0x70


//O_RemoteControl远程控制指令
#define Restart         0x24
#define Shutdown        0x25
#define ExtendTime      0x26
#define ClearError      0x27
#define Manual_auto     0x30//restaurar automáticamente
#define Manual_start    0x31//arranque manual

//modo manual
#define Manual_NONE             0x00
#define Manual_yellowflash      0x30//
#define Manual_allred           0x31
#define Manual_lampoff          0x32
#define Manual_nextphase        0x33
#define Manual_appoint          0x34

//datos de palabras reservadas
#define Reserve_Data      0x01

typedef struct 
{
    uint8_t  n;
    uint8_t  *pdata0;
    uint8_t  *pdata1;
    uint8_t  *pdata2;
    uint8_t  *pdata3;
    uint16_t length0;
    uint16_t length1;
    uint16_t length2;
    uint16_t length3;
}SEND_TYPE;

typedef struct 
{
    uint8_t Head;
    uint8_t Version;
    uint8_t Sendid;
    uint8_t Reciveid;

    uint8_t DataLinkCode;
    uint8_t AreaNum;
    uint8_t IntersectionL;
    uint8_t IntersectionH;
    
    uint8_t OperationType;
    uint8_t ObjectCode;
    uint8_t Reserve;
}GBMSG_TYPE;

extern SEND_TYPE send;

void SignalStateAutoReport(SEND_TYPE* send);
void DoorAlarmReport(SEND_TYPE* send);

uint8_t gb25280_Process(uint8_t *pd,uint16_t size,SEND_TYPE* send);



#endif

