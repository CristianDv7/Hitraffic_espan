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
    uint8_t  Num;           //检测器编号
    uint8_t  Index;         //检测器配置参数在检测器表中的索引
    uint8_t  Valid;         //1有效 0无效
    uint8_t  State;         //1有效 0无效
    uint32_t DelayCount;    //单位10ms秒
    uint32_t Time;          //当前状态持续时间，单位10ms
    uint16_t CarsByCycle;   //每周期过车数
    uint16_t CarsByMinute;  //每分钟车辆数  是否统计每周期过车数，而不是每分钟？
}VehDetState_Type;

typedef struct
{
    VehDetState_Type VehDet[VehdetMax];
}VehDetStateTable;
//文件组织：每天一个以分钟为单位的流量记录 文件/信息，连续统计至少一个月


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
    uint8_t Num;            //检测器编号
    uint8_t Options;        //检测器选项
    uint8_t CallPhase;      //请求相位  车辆检测器对应的机动车相位，当该检测器检测到车辆通过的信号时，对应的请求相位将做出反应。
    uint8_t SwitchPhase;    //开关相位  检测器转换相位参数，当指定相位是黄灯或红灯且转换相位是绿灯时，车辆检测器感应将转换到该相位
    uint8_t Extend;         //延长绿    在感应控制下，请求相位绿灯时间延长一次的时间。功能与相位参数的单位延长绿相同。但它只在选择“使用检测器的延长绿”选项后才有效，反之使用“请求相位”的单位延长绿
    uint8_t QueueLimit;     //队列限制  检测器排队限制参数，单位：秒。
    uint8_t NoActivity;     //无响应时间         检测器在该段时间内，没有检测到车辆通过，表示该检测器出现故障，该故障被信号机记入故障日志。该值为零时，该项功能不启用。单位为：分钟。
    uint8_t MaxPresence;    //最大持续响应时间   检测器在该段时间内，一直检测到车辆通过，表示该检测器出现故障，该故障被信号机记入故障日志。该值为零时，该项功能不启用。单位为：分钟。
    uint8_t ErraticCounts;  //最大车辆数    检测器不确定计数诊断参数。如果一个活动检测器出现感应过强，诊断设备就认为存在故障，该检测器被认为是失败的。如果将这个对象的值设为零，将关闭对该检测器的诊断，单位：次/分钟。
    uint8_t FailTime;       //失败时间      检测器的请求在没有得到响应之前，如果失败时间内没有任何请求，则取消本次记录的请求，不再予以响应。
    uint8_t Alarms;         //故障    
    uint8_t ReportedAlarms; //报警    
    uint8_t Reset;          //重置0-1       重置操作放到检测器状态里面?
    uint8_t Reserve;
    uint16_t Delay;         //延迟      检测器的请求时间达到延迟时间设置的长度，才记录一次请求,标准单位10S
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
    uint8_t Volume;     //该值的范围应为0至254，表示在收集期间通过相关检测器编号的交通量。
    uint8_t Occupancy;  //探测器占有率占收集数据或探测器单元诊断信息的体积占用率的百分比。
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

extern VehdetTable              VehdetTab;          //车检器表
extern VehdetStatusTable        VehdetStatusTab;    //车检状态表
extern VolumeOccupancyTable     VolumeOccupancyTab; //车流量占有率表
extern VehDetStateTable         VehDetStateTab;       //车检状态表

void VehicleDetectorInit(void);
void GetVehDetSta(void);//1ms执行一次，检测器状态获取
void VehDetStaCount(void);    //10ms执行一次，检测器状态时间统计




#endif
