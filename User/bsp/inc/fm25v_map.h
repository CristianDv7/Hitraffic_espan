#ifndef _FM25V_MAP_H
#define _FM25V_MAP_H
#include <stdint.h>


//此文件定义底层数据
//铁电存储器的地址划分
//fm25v.TotalSize = 0x20000 = 131072字节 
//range 0x00000~0x1ffff   
//2048 = 0x800  0x20000/0x800 = 25


//启动时判断，铁电0-1号地址空间的启动码，存储数据不为0x5A,0xCC,
//那么调用恢复默认参数的程序，然后置0号地址数据为0x5A,0xCC
//注意先初始化数据再修改启动码。
//启动过程中，屏幕提示首次开机，请等待初始化。
//初始化结束，蜂鸣器长鸣1秒。

//0-1启动码 2-15空余空间
#define StartCodeAddr       0x00000000      //0x0010
#define DeviceInfoAddr      0x00000010      //0x00B0
#define BasicInfoAddr       0x000000C0      //0x00B0
#define UnitAddr            0x00000170      //0x0010
#define CoordAddr           0x00000180      //0x0010
#define ScheduleAddr        0x00000190      //0x0170
#define PlanAddr            0x00000300      //0x04A0
#define ActionAddr          0x000007A0      //0x01A0
#define PatternAddr         0x00000940      //0x02C0
#define SplitAddr           0x00000C00      //0x0520
#define SequenceAddr        0x00001120      //0x0460
#define PhaseAddr           0x00001580      //0x0410
#define ChannelAddr         0x00001990      //0x0110
#define OverlapAddr         0x00001AA0      //0x00B0
#define PeddetAddr          0x00001B50      //0x0040
#define VehdetAddr          0x00001B90      //0x0200
#define PreemptAddr         0x00001D90      //






//基础信息*************************************************************************

//typedef struct
//{
//    uint8_t IPV6[16];               // 2.8.1 信号机IPV6,16byte,0x141-0x150
//    uint8_t SubMaskV6[16];          // 2.8.2 信号机IPV6子网前缀,16byte,0x151-0x160
//    uint8_t GetWayV6[16];           // 2.8.3 IPV6网关,16byte,0x161-0x170
//}IPv6Info_TypeDef;                  // 2.8 IPv6信息表

//typedef struct
//{
//    uint8_t RemoteIPV6[16];         // 2.9.1 信号机IPV6,16byte,0x171-0x180
//    uint16_t RemoteSocketV6;        // 2.9.2 RemoteSocketV6,2byte,0x181-0x182
//    uint8_t CommunicationTypeV6[16];// 2.9.3 通信类型,1byte,value 1-3,0x183
//}IPv6RemoteInfo_TypeDef;            // 2.9 IPv6中心机信息表


//灯组信息********************************************************************************************
//typedef enum { Vehicle=1, NoneVehicle, Pedestrian, VehicleLane, VariableTrafficSigns, BusLane, TramLane, SpecialLane} GroupType;

//typedef struct
//{
//    uint8_t Number;     //编号 - 1-64
//    uint8_t Mask;       //屏蔽 - bool 0正常输出1强制红灯,
//    uint8_t Prohibit;   //禁止 - bool 0正常输出1强制关灯,
//}Control_TypeDef;       //相位 或者 灯组 控制表

//typedef struct
//{
//    uint8_t Number;     // 3.2.1 灯组编号 
//    uint8_t Type;       // 3.2.2 灯组类型 
//}GroupConfig_TypeDef;   // 3.2 灯组配置表

//typedef struct
//{
//    uint8_t Number; // 3.3.1 灯组编号 
//    uint8_t State;  // 3.3.2 灯组状态 
//}GroupState_TypeDef;// 3.3 灯组状态表

//typedef struct //main 3
//{
//    uint8_t TotalNumber;            // 3.1 灯组数量,value 1-64,1byte,0x186,
//    GroupConfig_TypeDef Config[64]; // 3.2 灯组配置表,128byte,0x187-0x206
//    GroupState_TypeDef Status[64];  // 3.3 灯组状态表,128byte,0x207-0x286
//    Control_TypeDef Control[64];    // 3.4 灯组控制表,192byte,0x287-0x346
//}GroupInfo_TypeDef;                 // 3 灯组信息表

//相位*********************************************************************************************
//typedef struct
//{
//    uint8_t State;  // 4.2.1 放行的状态
//    uint8_t Time;   // 4.2.2 放行的时间,单位为S,时间为0表示不执行 
//}Transition_TypeDef;// 4.2 过渡参数

//相位配置表 phase-config-table
//总共 1+8+24+8+8 = 49 byte
//typedef struct
//{
//    uint8_t Number;                 // 4.2.1 相位号,1byte
//    uint8_t Group[8];               // 4.2.2 该相位放行的灯组 byte8-byte1 ~ bit63-bit0,8byte
//    Transition_TypeDef LoseTrans[3];// 4.2.3 - 4.2.8   失去 路权
//    Transition_TypeDef GetTrans[3]; // 4.2.9 - 4.2.14  获得 路权
//    Transition_TypeDef GetStartUpTrans[3]; // 4.2.15 - 4.2.20 开机获得路权
//    Transition_TypeDef LoseStartUpTrans[3];// 4.2.21 - 4.2.26 开机失去路权
//    uint16_t MinimumGreenTime;      // 4.2.27 最小绿灯时间
//    uint16_t MaximumGreenTime1;     // 4.2.28 最大绿灯时间1
//    uint16_t MaximumGreenTime2;     // 4.2.29 最大绿灯时间2
//    uint16_t ExtendedGreenTime;     // 4.2.30 相位延长绿时间
//    uint8_t Requirement[8];         // 4.2.31 相位请求 byte8-byte1 ~ bit63-bit0 代表检测器64-1,为1表示该检测器请求该相位
//}PhaseConfig_TypeDef;               // 4.2 相位配置表 

//typedef struct //main 4
//{
//    uint8_t TotalNumber;               // 4.1 相位数量, value 1-64,
//    PhaseConfig_TypeDef Config[64];    // 4.2 相位配置表 
//    Control_TypeDef Control[64];       // 4.3 相位控制表 
//}PhaseInfo_TypeDef;                    // 4 相位信息

//检测器*********************************************************************************************
//车辆类型     0无车;   1小型车;  2中型车;  3大型车;4公交车;5有轨电车;6特种车辆;
//typedef enum { None=0, SmallCar, MediumCar, LargeCar, Bus, Tram, SpecialCar} CarType;

//typedef struct
//{
//    uint8_t Number;                 // 5.2.1 检测器号,1byte
//    uint8_t Type;                   // 5.2.2 检测器类型,1byte
//    uint16_t FlowCollectPeriod;     // 5.2.3 流量采集周期,2byte
//    uint16_t OccupyCollectCycle;    // 5.2.4 占有率采集周期,2byte
//    uint8_t Position[128];          // 5.2.5 安装位置信息
//}DetectorConfig_TypeDef;            // 5.2 检测器配置表

//typedef struct
//{
//    uint8_t Number;                 // 5.3.1 检测器号,1byte
//    uint8_t Occupy;                 // 5.3.2 车辆存在状态,bool 0没车,有车 1byte
//    uint8_t Speed;                  // 5.3.3 当前车速,1byte
//    CarType Type;                   // 5.3.4 当前车辆类型 
//    uint8_t PlateNumber[16];        // 5.3.5 车牌号,16byte
//    uint16_t QueueLength;           // 5.3.6 排队长度,单位米,2byte
//}DetectorData_TypeDef;              // 5.3 检测器数据表

//main 5
//typedef struct
//{
//    uint8_t TotalNumber;               // 5.1 检测器数量, value 协议范围1-128,实际规范到 1-32
//    DetectorConfig_TypeDef Config[32]; // 5.2 检测器配置表 
//    DetectorData_TypeDef Data[32];     // 5.3 检测器数据表 
//}DetectorInfo_TypeDef;                 // 5 检测器信息 

////相位阶段*********************************************************************************************
//typedef struct
//{
//    uint8_t Number;         // 6.2.1相位阶段编号, 1-64, 1byte
//    uint8_t Phase[8];       // 6.2.2 byte8-byte1 ~ bit63-bit0 代表检测器64-1,1表示该相位在该相位阶段中 
//    uint8_t DelayUp[64];    // 6.2.3 byte1-byte64 代表相位1-相位64 的晚启动的秒数 
//    uint8_t EarlyClose[64]; // 6.2.4 byte1-byte64 代表相位1-相位64 的早结束的秒数 
//}PhaseStageConfig_TypeDef;  // 6.2 相位阶段配置表

//typedef struct
//{
//    uint8_t Number;         // 6.3.1 相位阶段编号, 1-64, 1byte
//    uint8_t State;          // 6.3.2 相位阶段状态, , 1byte
//    uint16_t RunTime;       // 6.3.3 相位阶段运行时间,单位秒, 2byte
//    uint16_t RemainTime;    // 6.3.4 相位阶段剩余时间, 2byte
//}PhaseStageStatus_TypeDef;  // 6.3 相位阶段状态表

//typedef struct
//{
//    uint8_t Number;         // 6.4.1 相位阶段编号, 1-64, 1byte
//    uint8_t SoftwareRequire;// 6.4.2 相位阶段软件需求, bool 0没有软件需求 1中心设置检测器请求, 1byte 
//    uint8_t Mask;           // 6.4.3 相位阶段 bool 0 不屏蔽,1 屏蔽 相位阶段所有相位为红灯, 1byte 
//    uint8_t Prohibit;       // 6.4.4 相位阶段 bool 0 不禁止,1 禁止 相位阶段所有相位为灭灯, 1byte 
//}PhaseStageControl_TypeDef; // 6.4 相位阶段控制表 

//main 6
//typedef struct
//{
//    uint8_t TotalNumber;                    // 6.1 实际配置相位阶段数, 1-64 ,1byte 
//    PhaseStageConfig_TypeDef Config[64];    // 6.2 相位阶段配置表 
//    PhaseStageStatus_TypeDef Status[64];    // 6.3 相位阶段状态表 
//    PhaseStageControl_TypeDef Control[64];  // 6.4 相位阶段控制表 
//}PhaseStageInfo_TypeDef;                    // 6 相位阶段信息

//相位安全信息****************************************************************************************************
//typedef struct
//{
//    uint8_t Number;         // 7.1.1 相位号 
//    uint8_t Sequence[8];    // 7.1.2 冲突序列 ,byte8-byte1 bit63-bit0 对应相位64-相位1,1表示相位号与该相位冲突 
//}PhaseConflict_TypeDef;     // 7.1 相位冲突配置表 

//typedef struct
//{
//    uint8_t Number;          // 7.2.1 相位号 
//    uint8_t TimeSequence[64];// 7.2.2 冲突序列 ,byte8-byte1 bit63-bit0 对应相位64-相位1,1表示相位号与该相位冲突 
//}PhaseGreenGapConfig_TypeDef;// 7.2 相位绿间隔配置表

//typedef struct //main 7
//{
//    PhaseConflict_TypeDef Conflict[64];        // 7.1 相位冲突配置表
//    PhaseGreenGapConfig_TypeDef GreenGap[64];  // 7.2 相位绿间隔配置表
//}PhaseSafeInfo_TypeDef;                        // 7 相位安全信息

//紧急优先***************************************************************************************************
//typedef struct 
//{
//    uint8_t Number;             // 8.2.1 优先信号编号 
//    uint8_t PhaseStage;         // 8.2.2 优先信号申请的相位阶段编号 
//    uint8_t PriorityLevel;      // 8.2.3 优先信号申请的优先级 越小越搞, 0的优先级最高
//    uint8_t Mask;               // 8.2.4 优先信号屏蔽标志 bool 0不屏蔽, 1屏蔽 
//}UrgentPriorityConfig_TypeDef;          // 8.2 优先配置表

//typedef struct 
//{
//    uint8_t Number;             // 8.3.1 优先信号编号 
//    uint8_t State;              // 8.3.2 优先信号申请状态 bool 0没有申请, 1有申请 
//    uint8_t Execution;          // 8.3.3 优先信号执行状态 bool 0没有执行, 1有执行
//}UrgentPriorityState_TypeDef;         // 8.3 优先状态表 

//typedef struct //main 8
//{
//    uint8_t PriorityNumber;                           // 8.1 实际优先数 1-64
//    UrgentPriorityConfig_TypeDef PriorityConfig[64];  // 8.2 优先配置表
//    UrgentPriorityState_TypeDef PriorityState[64];    // 8.3 优先状态表 
//    
//    uint8_t UrgentNumber;                             // 8.4 实际紧急数 1-64
//    UrgentPriorityConfig_TypeDef UrgentConfig[64];    // 8.5 紧急配置表
//    UrgentPriorityState_TypeDef UrgentState[64];      // 8.6 紧急状态表 
//}UrgentPriority_TypeDef;                              // 8 紧急优先

//方案信息*************************************************************************************************
//typedef struct
//{
//    uint8_t Number;                 // 9.2.1 方案编号 
//    uint8_t IntersectionNumber;     // 9.2.2 方案所属路口序号 1-8 用于信号机控制多路口 
//    uint32_t Cycle;                 // 9.2.3 方案周期 单位秒 
//    uint8_t CoordinateStageNumber;  // 9.2.4 方案中协调相位的阶段号 1-16
//    uint16_t PhaseDifference;       // 9.2.5 方案相位差时间 
//    uint8_t Sequence[16];           // 9.2.6 相位阶段序列,每个数表示相位阶段编号, 从0执行到16, 不满16用0填充
//    uint16_t Time[16];
//}PlanConfig_TypeDef;


//typedef struct //main 9
//{
//    uint8_t TotalNumber;                    // 6.1 实际方案数, 1-128 ,1byte 

//}PlanInfo_TypeDef;  // 9 方案信息

#endif
