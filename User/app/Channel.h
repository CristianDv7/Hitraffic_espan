#ifndef CHANNEL_H
#define CHANNEL_H
#include <stdint.h>


enum LampDrive_Type
{
    LD_BLACK        = 0, 
    LD_RED          = 0x0001, 
    LD_YELLOW       = 0x0002, 
    LD_REDYELLOW    = 0x0003, 
    LD_GREEN        = 0x0004, 
    LD_ALL          = 0x0007,
};

//Tipo de control CHANNEL_CONTROL_TYPE

typedef enum 
{
    CCT_OTHER = 1,      //otro
    CCT_VEHICLE,        // maniobra
    CCT_PEDESTRIAN,     //peatonal
    CCT_OVERLAP,        //seguir
    CCT_FLASH,
    CCT_GREEN,
    CCT_RED,
}CHANNEL_CONTROL_TYPE;  //tipo de control de canal

//Modo de parpadeo CHANNEL_FLASH_MODE
#define CFM_Yellow      ((uint8_t)0x02)     //luz amarilla
#define CFM_Red         ((uint8_t)0x04)     // destello rojo
#define CFM_Alternate   ((uint8_t)0x08)     // parpadea alternativamente

//Modo atenuado CHANNEL_DIM_MODE
#define CDM_Green       ((uint8_t)0x01)     //luz verde
#define CDM_Yellow      ((uint8_t)0x02)     //luz amarillo
#define CDM_Red         ((uint8_t)0x04)     //luz roja
#define CDM_Alternate   ((uint8_t)0x08)     //alternativamente

//Posición de control de canal CHANNEL_POSITION_MODE
#define POS_Other       ((uint8_t)0x00)     //otro
#define POS_East        ((uint8_t)0x01)     //este
#define POS_South       ((uint8_t)0x02)     //sur
#define POS_West        ((uint8_t)0x03)     //Oeste
#define POS_North       ((uint8_t)0x04)     //Norte
#define POS_NorthEast   ((uint8_t)0x05)     //Noreste
#define POS_SouthEast   ((uint8_t)0x06)     //Sureste
#define POS_SouthWest   ((uint8_t)0x07)     //SurOeste
#define POS_NorthWest   ((uint8_t)0x08)     //NorOeste

//Dirección de control de canal CHANNEL_DIRECTION_MODE
#define DIR_Other       ((uint8_t)0x00)     //Otro
#define DIR_Left        ((uint8_t)0x01)     //Gire a la izquierda
#define DIR_Straight    ((uint8_t)0x02)     //Siga derecho
#define DIR_Right       ((uint8_t)0x03)     //Gire a la derecha
#define DIR_Pedestrian  ((uint8_t)0x04)     //peatonal
#define DIR_Turn        ((uint8_t)0x05)     //Giro de vuelta
#define DIR_Bicycle     ((uint8_t)0x06)     //bicicleta
//Left Straight Right Turn Pedestrian Bicycle


//Orientación manual
#define MANUAL_POS_Other       ((uint8_t)0x00)     //Otro
#define MANUAL_POS_East        ((uint8_t)0x01)     //Este
#define MANUAL_POS_South       ((uint8_t)0x02)     //Sur
#define MANUAL_POS_West        ((uint8_t)0x04)     //Oeste
#define MANUAL_POS_North       ((uint8_t)0x08)     //Norte
#define MANUAL_POS_NorthEast   ((uint8_t)0x10)     //Noreste
#define MANUAL_POS_SouthEast   ((uint8_t)0x20)     //Sureste
#define MANUAL_POS_SouthWest   ((uint8_t)0x40)     //SurOeste
#define MANUAL_POS_NorthWest   ((uint8_t)0x80)     //NorOeste

//Dirección manual
#define MANUAL_DIR_Other       ((uint8_t)0x00)     //其他
#define MANUAL_DIR_Left        ((uint8_t)0x01)     //左转
#define MANUAL_DIR_Straight    ((uint8_t)0x02)     //直行
#define MANUAL_DIR_Right       ((uint8_t)0x04)     //右转
#define MANUAL_DIR_Pedestrian  ((uint8_t)0x08)     //人行
#define MANUAL_DIR_Turn        ((uint8_t)0x10)     //掉头
#define MANUAL_DIR_Bicycle     ((uint8_t)0x20)     //自行车


typedef struct
{
    uint8_t Num;            //numero de canal
    uint8_t ControlSource;  //fuente de control
    uint8_t ControlType;    //tipo de control
    uint8_t Flash;          //modo destello
    uint8_t Dim;            //Modo de brillo
    uint8_t Position;       //
    uint8_t Direction;      //
    uint8_t CountdownID;    //... Considere una tabla de configuración de cuenta regresiva separada
}ChannelType; //Definición de datos de canal

typedef struct
{
    uint8_t Maximum;//El número máximo de diseños de canales es 32
    ChannelType Channel[32];
    uint8_t Reserve[15];
}ChannelTable;   //1+8*32+15 = 272 = 0x0110


/********************************************************/
typedef struct//Cada bit representa el estado de color correspondiente de un canal
{
    uint32_t     Reds;
    uint32_t     Yellows;
    uint32_t     Greens;
    uint32_t     Flash;     //1 parpadea, 0 no parpadea
}ChannelStatusType;
/********************************************************/
typedef struct//Cada bit representa el estado de color correspondiente de un canal
{
    uint32_t     Reds;
    uint32_t     Yellows;
    uint32_t     Greens;
}ChannelReadStatusType;

//Tabla de parámetros de estado de canal personalizado
typedef enum 
{
    BLACK = 0,
    GREEN,
    YELLOW,
    RED,
    RED_YELLOW,         //rojo brillante y amarillo
    RED_YELLOW_FLASH,   //Rojo y amarillo parpadeando alternativamente
    GREEN_FLASH,
    YELLOW_FLASH,
    RED_FLASH,
}ChannelStateMode;
/********************************************************/


extern ChannelTable             ChannelTab;         //tabla de canales
extern ChannelStatusType        ChannelStatus;      //Tabla de estado de canales
extern ChannelReadStatusType    ChannelReadStatus;


void ChannelInit(void);
void AutoFlashMode(void);
void AutoAllRedMode(void);
void AutoLampOffMode(void);

uint8_t isVehPhase(uint8_t phaseNum);
uint8_t isPedPhase(uint8_t phaseNum);
uint32_t GetAppointChannel(uint8_t Pos, uint8_t Dir);

void LampControl(uint8_t tick10msCount);

#endif
