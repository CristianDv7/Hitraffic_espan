#ifndef TSC_H
#define TSC_H

#include "public.h"
#include "Sequence.h"
#include "Channel.h"
#include "Phase.h"
#include "Split.h"
#include "Pattern.h"
#include "Schedule.h"
#include "Action.h"
#include "Plan.h"
#include "Unit.h"
#include "Vehdet.h"
#include "Coord.h"


typedef struct
{
    RtcType utc;
    RtcType local;
    char time_str[16];//UTCtime
    char year_str[8];
    char month_str[8];
    char day_str[8];
	char Latitude[16];
	char NS[2];
	char Longitude[16];
	char EW[2];
    char FS[2];     //定位状态标志
    char numSv[4];  //卫星数量
	char Altitude[4];//海拔
	char Speed[8];
}GpsType;

typedef struct
{
    uint32_t Active;
    uint32_t Time;
    uint32_t TimeRemain;
}PhaseTimeStatusType;


typedef struct 
{
    uint16_t    CycleTime;          //环周期
    uint8_t     Time[PhaseMax];    //环相位时间
}RingTimeType;  //环相位时间   17 byte

typedef struct
{
    RingTimeType    Ring[RingMax];  //环相位时间定义 4个环
}SeqTimeType;        //相序表信息 68 byte

/********************************************************/
typedef struct
{
    uint8_t         ModifierFlag;   //绿波模式,存在偏差,需要修正标志
    uint16_t        ModifierTime;   //需要修正的总剩余时间
    uint16_t        OffsetTime;
    uint16_t        StepTime;       //单步修正的时间
}OffsetTimeModifier_Type;//相位差修正

typedef struct
{
    uint8_t         Num;    //
    uint8_t         Data;   //
}Link_Type;//

#define STATE_0             ((uint32_t)0x00000001)
#define STATE_1             ((uint32_t)0x00000002)
#define STATE_2             ((uint32_t)0x00000004)
#define STATE_3             ((uint32_t)0x00000008)
#define STATE_4             ((uint32_t)0x00000010)
#define STATE_5             ((uint32_t)0x00000020)
#define STATE_6             ((uint32_t)0x00000040)
#define STATE_7             ((uint32_t)0x00000080)
#define STATE_8             ((uint32_t)0x00000100)
#define STATE_9             ((uint32_t)0x00000200)
#define STATE_10            ((uint32_t)0x00000400)
#define STATE_11            ((uint32_t)0x00000800)
#define STATE_12            ((uint32_t)0x00001000)
#define STATE_13            ((uint32_t)0x00002000)
#define STATE_14            ((uint32_t)0x00004000)
#define STATE_15            ((uint32_t)0x00008000)
#define STATE_16            ((uint32_t)0x00010000)
#define STATE_17            ((uint32_t)0x00020000)
#define STATE_18            ((uint32_t)0x00040000)
#define STATE_19            ((uint32_t)0x00080000)
#define STATE_20            ((uint32_t)0x00100000)
#define STATE_21            ((uint32_t)0x00200000)
#define STATE_22            ((uint32_t)0x00400000)
#define STATE_23            ((uint32_t)0x00800000)
#define STATE_24            ((uint32_t)0x01000000)
#define STATE_25            ((uint32_t)0x02000000)
#define STATE_26            ((uint32_t)0x04000000)
#define STATE_27            ((uint32_t)0x08000000)
#define STATE_28            ((uint32_t)0x10000000)
#define STATE_29            ((uint32_t)0x20000000)
#define STATE_30            ((uint32_t)0x40000000)
#define STATE_31            ((uint32_t)0x80000000)

/********************************************************/
typedef struct
{
    uint8_t     RemoteCtrlFlag; //远程手控标志
    uint8_t     LocalCtrlFlag;  //本地手控标志
    uint8_t     KeyCmd;         //按键指令
    uint8_t     NextStepFlag;   //步进标志
    uint8_t     StartFlag;      //刚开始远程手控
    uint8_t     EnforceFlag;    //执行标志
    uint8_t     OrderFlag;      //
    uint8_t     ExitFlag;       //退出标志
    uint8_t     Pos;            //
    uint8_t     Dir;            //
    uint16_t    AutoTime;
    uint16_t    Time;
    uint16_t    MaxiChannelTransitionTime;
//    uint16_t    MaxiTransitionTimeNow;
//    uint16_t    MaxiTransitionTimeNext;
//    uint16_t    MaxiVehicleTransitionTimeNow;
//    uint16_t    MaxiVehicleTransitionTimeNext;
    uint32_t    ChannelOnsBackup;
    uint32_t    ChannelOnsNow;
    uint32_t    ChannelOnsNext;
}MANUAL_CTRL_TYPE;



typedef struct
{
    uint8_t             WorkMode;               //工作模式
    uint8_t             WorkMode_Reason;        //模式原因  ------- 需要修改为按位操作
    uint32_t            STATE;                  //系统状态  按位定义状态0正常1异常
    //0-15为系统底层的状态,16-31为应用的系统状态 比如bit0存储,bit1时钟,bit2参数,bit3,
    uint8_t             SubWorkMode;            //子模式
    uint8_t             WorkModeBK;             //工作模式
    uint32_t            WorkModeCount;          //当前工作模式 已运行时间
    uint32_t            WorkModeExpectTime;     //当前工作模式 预期运行时间
    uint32_t            Reg1sCount;
    OffsetTimeModifier_Type     OffsetTimeModifier;
    uint32_t            Seconds;
    uint32_t            gps_seconds;
    uint8_t             Run1s_flag;             //每1s置位
    uint8_t             Run10ms_flag;
    uint8_t             Run100ms_flag;
    uint8_t             PlanRefreshFlag;
    uint8_t             ActionRefreshFlag;      //动作刷新
    uint8_t             SequenceRefreshFlag;    //相序数据有改变
    uint8_t             NewPatternFlag;
    uint8_t             ShowTimeFlag;
    
    uint8_t             TimeChangeFlag;
    uint8_t             ScheduleDataChangeFlag;
    uint8_t             PlanDataChangeFlag;
    uint8_t             ActionDataChangeFlag;
    uint8_t             PatternDataChangeFlag;
    uint8_t             SequenceDataChangeFlag;
    uint8_t             SplitDataChangeFlag;
    uint8_t             PhaseDataChangeFlag;
    uint8_t             RtcIrqFlag;
    uint8_t             GetVehDetStaFlag;
    
    uint8_t             StartFlag;      //系统重启标志，冷起或者重启都将置1
    uint8_t             RestartFlag;    //重启操作标志，置1系统将重启
    uint8_t             TriggerFlag;    //倒计时触发信号 

    uint8_t             ConnectFlag;
    uint8_t             SendWorkModeAutoFlag;
    uint8_t             SendWorkModeFlag;
    uint8_t             SendDoorAlarm;
    uint8_t             LampStateRefreshFlag; 
    uint8_t             PhaseStatusRefreshFlag;
    
    uint8_t             sync_with_gps_flag; //1需要与同步gps时钟同步，0不需要。
    
    uint16_t            red_green_conflict_reg;
    uint16_t            red_install_reg;
    uint16_t            red_failed_reg;
    uint16_t            green_conflict_reg;
    uint32_t            ParChangeReg;
    
    GpsType             Gps;
}OPType;

#define HardWareVersionH    0x19
#define HardWareVersionL    0x01
#define SoftWareVersionH    0x21
#define SoftWareVersionL    0x07


#define TimePar         0x0001
#define SchedulePar     0x0002
#define PlanPar         0x0004
#define ActionPar       0x0008
#define PatternPar      0x0010
#define SplitPar        0x0020
#define SequencePar     0x0040
#define PhasePar        0x0080
#define ChannelPar      0x0100
#define UnitPar         0x0200
#define CoordPar        0x0400
#define OverlapPar      0x0800

typedef enum{MANUAL_NONE = 0, MANUAL_SW = 0x01, MANUAL_K1 = 0x02, MANUAL_K2 = 0x04, MANUAL_K3 = 0x08, MANUAL_K7 = 0x10, MANUAL_K6 = 0x20, MANUAL_K5 = 0x40,  MANUAL_K4 = 0x80}MANUAL_key;

#define     MANUAL_OFF          MANUAL_NONE
#define     MANUAL_ON           MANUAL_SW
#define     MANUAL_FLASH        MANUAL_K1
#define     MANUAL_AllRed       MANUAL_K2
#define     MANUAL_NextStep     MANUAL_K3
#define     MANUAL_LampOff      MANUAL_K7
#define     MANUAL_ClearError   MANUAL_K7



extern OPType               OP;
extern RtcType              Rtc;        //当前时钟时间
extern DateType             Date;       //当前NTCIP日期
extern TimeType             Time;       //当前NTCIP时间
extern MANUAL_CTRL_TYPE     ManualCtrl;


void LampStateControl(void);    //1s运行一次




void PhaseParInit(void);
void RunPhaseTrans(void);   //1ms运行一次
void RunNewPhase(void);     //1ms运行一次
void RunPhaseGetNext(void); //1ms运行一次
void RunPhaseTimeCalc(void);//10ms运行一次

void RunPhaseInit(SequenceType* Sequence, SplitType* Split);
void RunDataInit(void);

void LampControlProcess(void);


void NewPeriodApply(PlanType* DayPlan,TimeType* Time);
void NewPatternCheck(void);

void PlanActionRefresh(PlanType* DayPlan,TimeType* Time);
void NewPatternApply(void);

void ReadRealTime(void);

uint8_t RingPhaseChange(uint8_t ring);
void RunPhaseStateStartup(void);
void GetSeqTime(void);
void GetPhaseStatusMap(void);


void Input_mange(void);
uint16_t red_install_fail_detect(uint16_t installs);


void RtcIrqCallback(void);


#endif // TSC_H
