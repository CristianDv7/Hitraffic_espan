#ifndef BASICINFO_H
#define BASICINFO_H
#include <stdint.h>


//设备信息********************************************************************************************
typedef struct //main 1
{
	uint8_t ManufacturerInfo[128];  // 1.1 制造厂商信息128字节,0x00-0x7f
	uint8_t DeviceVersion[4];       // 1.2 设备版本,4byte,高两位硬件,低两位软件,0x80-0x83
	uint8_t DeviceNumber[16];       // 1.3 设备编号16byte, 0x84-0x93
	uint8_t ProductionDate[6];      // 1.4 出厂日期,6byte, 0x94-0x9a
	uint8_t ConfigurationDate[6];   // 1.5 配置日期,6byte, 0x9b-0xa1
    uint8_t Reserve[16];
}DeviceInfo_TypeDef;                // 1 设备信息

typedef struct
{
	uint8_t IP[4];                  // 2.2.1 IP地址,4byte,0x124-0x127
	uint8_t SubMask[4];             // 2.2.2 SubMask,4byte,0x128-0x12b
	uint8_t GetWay[4];              // 2.2.3 GetWay,4byte,0x12c-0x12f
    uint8_t Socket[2];              // 端口
    uint8_t MAC[6];                 // MAC地址
}IPv4Info_TypeDef;                  // 2.2 IPv4信息表 20byte

typedef struct
{
	uint8_t RemoteIP[4];            // 2.3.1 RemoteIP,4byte,0x130-0x133
    uint8_t RemoteSocket[2];        // 2.3.2 RemoteSocket,2byte,0x134-0x135
    uint8_t CommunicationType;      // 2.3.3 通信类型,1byte,value 1-3,0x136
}IPv4RemoteInfo_TypeDef;            // 2.3 IPv4中心机信息表

typedef struct //main 2
{
	uint8_t IntersectionInfo[128];      // 2.1 安装路口信息,128byte,0x0a4-0x123
    IPv4Info_TypeDef IPv4;              // 2.2 IPv4信息表 20
    IPv4RemoteInfo_TypeDef IPv4Remote;  // 2.3 IPv4中心机信息表 7
    uint8_t TimeZone[4];                // 2.4 时区,value -43200~43200,4byte,0x137-0x13a; TimeZone[0] == 1东区,0西区 1,2,3为时区秒数, 高字节在前
    uint8_t TscNumber[4];               // 2.5 信号机编号,上端系统中唯一编号,4byte,0x13b-0x13e
    uint8_t ControlIntersectionNumber;  // 2.6 信号机控制的路口数量,1byte,value 1-8,0x13f
    uint8_t GpsClockFlag;               // 2.7 GPS时钟标志,1byte,bool类型,0无效,1有效,0x140
    //IPv6Info_TypeDef IPv6;            // 2.8 IPv6信息表
    //IPv6RemoteInfo_TypeDef IPv6Remote;// 2.9 IPv6中心机信息表
    uint8_t Reserve[11];
}BasicInfo_TypeDef; //128 + 20 + 7 + 8 + 13 = 176 



extern DeviceInfo_TypeDef   DeviceInfo;
extern BasicInfo_TypeDef    BasicInfo;


void BasicInfoInit(void);
void DeviceInfoInit(void);




#endif
