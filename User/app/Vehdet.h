#ifndef DETECTORVEHICLE_H
#define DETECTORVEHICLE_H

#include "public.h"


typedef struct
{
    uint8_t  GetVehicleDetectorStateFlag;
    uint8_t  DelayCount;
}VehicleDetectorTimer_Type;


typedef struct
{
    uint8_t  Num;           //Número de detector
    uint8_t  Index;         //El índice del parámetro de configuración del detector en la tabla de detectores
    uint8_t  Valid;         //1 válido 0 inválido
    uint8_t  State;         //1 válido 0 inválido
    uint32_t DelayCount;    //Unidad 10ms segundo
    uint32_t Time;          //Duración del estado actual, unidad 10ms
    uint16_t CarsByCycle;   //Número de vehículos por ciclo
    uint16_t CarsByMinute;  //Vehículos por minuto ¿Se cuentan los vehículos por ciclo en lugar de por minuto?
}VehDetState_Type;

typedef struct
{
    VehDetState_Type VehDet[VehdetMax];
}VehDetStateTable;
//Organización de archivos: un archivo de registro de flujo/información en minutos todos los días, estadísticas continuas durante al menos un mes


//Options bits
#define VDO_require             0x80
#define VDO_queue               0x40
#define VDO_strategy            0x20
#define VDO_extend              0x10
#define VDO_redRequireLock      0x08
#define VDO_yellowRequireLock   0x04
#define VDO_recordOccupancy     0x02
#define VDO_recordFlow          0x01

//Alarms bits
#define VDA_otherError          0x80
#define VDA_reserve6            0x40
#define VDA_reserve5            0x20
#define VDA_configError         0x10
#define VDA_communicationError  0x08
#define VDA_senseError          0x04
#define VDA_overlimitError      0x02
#define VDA_noResponseError     0x01

//ReportedAlarms bits
#define VDR_reserve7            0x80
#define VDR_reserve6            0x40
#define VDR_reserve5            0x20
#define VDR_floatingOverlimit   0x10
#define VDR_shortCircuit        0x08
#define VDR_openCircuit         0x04
#define VDR_watchdogFailure     0x02
#define VDR_other               0x01




typedef struct  //14
{
    uint8_t Num;            //Número de detector
    uint8_t Options;        //Opciones de detectores
    uint8_t CallPhase;      //Fase de solicitud La fase del vehículo de motor correspondiente al detector de vehículos, cuando el detector detecta la señal del paso del vehículo, responderá la fase de solicitud correspondiente.
    uint8_t SwitchPhase;    //Parámetro de fase del interruptor del detector de fase del interruptor, cuando la fase especificada es luz amarilla o roja y la fase del interruptor es luz verde, el sentido del detector del vehículo cambiará a esta fase
    uint8_t Extend;         //Extend Green Bajo el control del sensor, solicita que el tiempo de luz verde de fase se extienda por una vez. La función es la misma que la extensión de la unidad verde del parámetro de fase. Pero solo funciona si se selecciona la opción "Usar verde extendido del detector", de lo contrario, use la unidad de "fase de solicitud" para extender verde
    uint8_t QueueLimit;     //Límite de cola El parámetro de límite de cola del detector, unidad: segundo.
    uint8_t NoActivity;     //Sin tiempo de respuesta Si el detector no detecta el paso de vehículos dentro de este período, significa que el detector está defectuoso y la máquina de señales registra la falla en el registro de fallas. Cuando el valor es cero, esta función está deshabilitada. La unidad es: minuto.
    uint8_t MaxPresence;    //El tiempo máximo de respuesta continua Si el detector detecta el paso del vehículo dentro de este período, significa que el detector tiene una falla, y la máquina de señales registra la falla en el registro de fallas. Cuando el valor es cero, esta función está deshabilitada. La unidad es: minuto.
    uint8_t ErraticCounts;  //Número máximo de vehículos Parámetro de diagnóstico de recuento incierto del detector. Si un detector de actividad es sobredetectado, el equipo de diagnóstico asume que hay una falla y se considera que el detector ha fallado. Si el valor de este objeto se establece en cero, el diagnóstico de este detector se desactivará, la unidad: veces/minuto.
    uint8_t FailTime;       //Tiempo de falla Antes de que se responda la solicitud del detector, si no hay ninguna solicitud dentro del tiempo de falla, la solicitud registrada se cancelará y no se dará respuesta.
    uint8_t Alarms;         //Falla  
    uint8_t ReportedAlarms; //Llame a la policía    
    uint8_t Reset;          //Restablecer 0-1 ¿Restablecer la operación en el estado del detector?
    uint8_t Reserve;
    uint16_t Delay;         //Retraso El tiempo de solicitud del detector alcanza la duración del ajuste de tiempo de retraso antes de registrar una solicitud, la unidad estándar es 10S
}Vehdet;

typedef struct
{
    Vehdet vehdet[VehdetMax];
}VehdetTable;


typedef struct
{
    uint32_t Active;
    uint32_t Alarms;
}VehdetStatusTable;

typedef struct
{
    uint8_t Volume;     //Este valor debe oscilar entre 0 y 254 y representa la cantidad de tráfico que pasó por el número de detector asociado durante el período de recolección.
    uint8_t Occupancy;  //Ocupación del detector Porcentaje de ocupación del volumen para recopilar datos o información de diagnóstico de la unidad detectora.
    //Range     Meaning
    //0-200     Detector Occupancy in 0.5% Increments
    //201-209   Reserved
    //210       Max Presence Fault
    //211       No Activity Fault
    //212       Open loop Fault
    //213       Shorted loop Fault
    //214       Excessive Change Fault
    //215       Reserved
    //216       Watchdog Fault
    //217       Erratic Output Fault
    //218-255   Reserved
}VolumeOccupancy_Type;

typedef struct
{
    VolumeOccupancy_Type VehDet[VehdetMax];
}VolumeOccupancyTable;

extern VehdetTable              VehdetTab;          //mesa de detectores de autos
extern VehdetStatusTable        VehdetStatusTab;    //Tabla de estado de inspección de vehículos
extern VolumeOccupancyTable     VolumeOccupancyTab; //Tabla de reparto de tráfico
extern VehDetStateTable         VehDetStateTab;       //Tabla de estado de inspección de vehículos

void VehicleDetectorInit(void);
void GetVehDetSta(void);//Ejecutado una vez en 1 ms, y se adquiere el estado del detector
void VehDetStaCount(void);    //Ejecutado una vez cada 10 ms, las estadísticas de tiempo del estado del detector




#endif
