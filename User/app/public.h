#ifndef PUBLIC_H
#define PUBLIC_H
#include <stdio.h>
#include <stdint.h>
#include <string.h>


#define RELEASE     1   //1 versi髇 0 depuraci髇

#if RELEASE
    #define DEBUG           0
    #define RTCIRQ          0
    #define PhaseNextLog    0
#else
    #define DEBUG           1
    #define RTCIRQ          1
    #define PhaseNextLog    1
#endif


/* Definici髇 de estructura de datos b醩ica */
typedef struct  //Formato de datos C骴igo BCD
{
	uint8_t second;     //00-59 
	uint8_t minute;     //00-59 
	uint8_t hour;       //00-23 
	uint8_t day;        //00-31 
	uint8_t month;      //01-12 
	uint8_t year;       //00-99 
	uint8_t week;       //00-06 00 Semanal 01 Lunes ... 06 S醔ado
}RtcType;



#define FILTER_TIME     5       //Tiempo de filtrado para la entrada del detector
/********************************************************/
#define ScheduleMax     40
#define PlanMax         16
#define PeriodMax       24
#define ActionMax       100
#define PatternMax      100
#define SequenceMax     16
#define SplitMax        20
#define OverlapMax      16


#define RingMax         4
#define PhaseMax        16

#define IsRing(n)       ((n) > 0 && (n) <= RingMax)
#define IsPhase(n)      ((n) > 0 && (n) <= PhaseMax)

#define ChannelMax      16  //El n鷐ero m醲imo de canales de esta m醧uina
#define VehdetMax       32
#define PeddetMax       8

#define IsVehdet(n)     ((n) > 0 && (n) <= VehdetMax)
#define IsPeddet(n)     ((n) > 0 && (n) <= PeddetMax)



/********************************************************/
//Conversi髇 mutua de datos de tipo Uint8_t, DEC y BCD
#define DEC_to_BCD(x)   ((((x)/10)<<4)+((x)%10))
#define BCD_to_DEC(x)   ((((x)>>4)*10)+((x)&0x0f))
#define MAX(a,b)        (a>=b?:a,b)

#define defaultAutoExitRemoteTime      ((Unit.BackupTimeH<<8) | Unit.BackupTimeL)

/********************************************************/
#define SPECIAL_MODE    0xA0

typedef enum 
{
    StarupMode  = 0,        //secuencia de inicio
    FixedTime,              //01 tiempo fijo
    LineCtrl,               //02 Control remoto local sin cables
    VehicleSense,           //03Inducci髇 de punto 鷑ico 
    Flashing,               //04 per韔do flash
    AllRed,                 //ajuste de tiempo todo rojo  
    LampOff,                //ajuste de tiempo apagar luces
    
    /*****************************************/
    //本地手动
    ManualStep = 0xA0,      //pasos manuales
    ManualAppoint,          //Especificar manualmente la liberaci髇
    ManualFlashing,         //Parpadeo manual
    ManualAllRead,          //Parpadeo amarillo manual revisar
    ManualLampOff,          //Apagado manual revisar
    
    NoPatternFlash,         //Sin esquema parpadeando en amarillo
    ConflictFlash,          //conflicto destello amarillo
    ErrorOff,               //Falla apaga las luces
    
    LampTestMode,           //Las instrucciones espec韋icas de la prueba se pueden usar para la salida de la prueba o para las pruebas internas de hardware y software.
    DegradingFlashing = 0xD0,
    DegradingLampOff,
}RUN_MODE;


typedef enum 
{
    WMR_Normal = 0,
    WMR_LocalManual,
    WMR_RemoteManual,
    WMR_RemoteGuard,
    
    WMR_NoDayPlan = 0xA0,
    WMR_NoAction,
    WMR_NoPattern,
    WMR_NoSequence,
    WMR_NoSplit,
    WMR_RedGreenConflict,
    WMR_GreenConflict,
    WMR_RedFailed,
    
    WMR_WMR_NoPar,
    WMR_Hardware,
    WMR_Other = 0xff,
}WorkMode_Reason_Type;




#endif
