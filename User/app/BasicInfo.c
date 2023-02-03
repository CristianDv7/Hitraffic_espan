/*
*********************************************************************************************************
*
*	模块名称 : 
*	文件名称 : BasicInfo.c
*	版    本 : V1.0
*	说    明 : 
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2019-12-30  wcx     首发
*
*********************************************************************************************************
*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "BasicInfo.h"


DeviceInfo_TypeDef  DeviceInfo;
BasicInfo_TypeDef   BasicInfo;


void get_cpuid(uint8_t *pdata)
{
    uint32_t Device_Serial[3];
    Device_Serial[0] = *(volatile uint32_t*)(0x1FFFF7E8);
    Device_Serial[1] = *(volatile uint32_t*)(0x1FFFF7EC);
    Device_Serial[2] = *(volatile uint32_t*)(0x1FFFF7F0);
    
    pdata[2] = (Device_Serial[0]&0xff);
    pdata[1] = (Device_Serial[0]>>8);
    pdata[0] = (Device_Serial[0]>>16);
    printf("_cpuid %08x %08x %08x\n\r",Device_Serial[0],Device_Serial[1],Device_Serial[2]);
}


void DeviceInfoInit(void)
{
	const uint8_t ManufacturerInfo[128] = "\0H\0I\0T\0R\0A\0F\0F\0I\0C\0 \0S\0A \0S\0e\0c\0h\0n\0o\0l\0o\0g\0y\0 \0C\0o\0.\0,\0 \0L\0t\0d\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";/* 1.1 Informacion del fabricante */
    const uint8_t DeviceInfo_buf[48] = 
    {
        0x20,0x01,0x20,0x01,                    //1.2 设备版本,4byte,高两位硬件,低两位软件
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,//1.3 厂家对信号机的唯一编号16byte,不满16byte,高字节为0
        0x20,0x22,0x04,0x02,0x00,0x00,          //1.4 出厂日期,6byte
        0x20,0x22,0x04,0x01,0x00,0x00,          //1.5 配置日期,6byte
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//保留16字节
    };

    memcpy(DeviceInfo.ManufacturerInfo, ManufacturerInfo, 128);
    memcpy(DeviceInfo.DeviceVersion, DeviceInfo_buf, 48);
}

void BasicInfoInit(void)
{
	const uint8_t IntersectionInfo[128]={0};// 2.1 安装路口信息,128byte 
    uint8_t BasicInfo_buf[48]=
    {
        192,168,  1,122,//IP,4byte
        255,255,255,  0,//SubMask,4byte
        192,168,  1,  1,//Gateway,4byte
          0,161,        //Socket,2byte
        ':','S','W',0x00,0x00,0x01,     //Phy,6byte 
        
        192,168,  1, 100,// 2.3.1 RemoteIP,4byte
        0x17,0x78,      // 2.3.2 RemoteSocket,2byte 6008
          0,            // 2.3.3 通信类型,1byte
        0x01,0x00,0x70,0x80,    // 2.4 时区,value -43200~43200,4byte
        0x00,0x00,0x00,0x01,    // 2.5 信号机编号,上端系统中的唯一编号,4byte
        0x01,           // 2.6 信号机控制的路口数量,1byte,value 1-8
        0x00,           // 2.7 GPS时钟标志,1byte,bool类型,0无效,1有效
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//保留空间 11字节
    };
    get_cpuid(&BasicInfo_buf[17]);//MAC设置与cpuid挂钩,可保证唯一.
    memcpy(BasicInfo.IntersectionInfo, IntersectionInfo, 128);
    memcpy(BasicInfo.IPv4.IP, BasicInfo_buf, 48);
}



