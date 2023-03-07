/*
*********************************************************************************************************
*
*	Nombre del módulo: 
*	Nombre del archivo: BasicInfo.c
*	libro de versiones : V1.0
*	ilustrar : 
*	Registro de modificación:
*		Número de versión Fecha Autor Descripción
*		V1.0 2019-12-30 wcx primer lanzamiento
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
        0x20,0x01,0x20,0x01,                    //1.2 Versión del dispositivo, 4 bytes, hardware alto de dos bits, software bajo de dos bits
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,//1.3 El número único del fabricante para la máquina de señales es de 16 bytes, si es menor de 16 bytes, el byte alto es 0
        0x20,0x22,0x04,0x02,0x00,0x00,          //1.4 Fecha de fabricación, 6 bytes
        0x20,0x22,0x04,0x01,0x00,0x00,          //1.5 Fecha de configuración, 6 bytes
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//保留16字节
    };

    memcpy(DeviceInfo.ManufacturerInfo, ManufacturerInfo, 128);
    memcpy(DeviceInfo.DeviceVersion, DeviceInfo_buf, 48);
}

void BasicInfoInit(void)
{
	const uint8_t IntersectionInfo[128]={0};// 2.1 Instalar información de intersección, 128 bytes
    uint8_t BasicInfo_buf[48]=
    {
        192,168,  1,122,//IP,4byte
        255,255,255,  0,//SubMask,4byte
        192,168,  1,  1,//Gateway,4byte
          0,161,        //Socket,2byte
        ':','S','W',0x00,0x00,0x01,     //Phy,6byte 
        
        192,168,  1, 100,// 2.3.1 RemoteIP,4byte
        0x17,0x78,      // 2.3.2 RemoteSocket,2byte 6008
          0,            // 2.3.3 Tipo de comunicación, 1 byte
        0x01,0x00,0x70,0x80,    // 2.4 Zona horaria, valor -43200~43200, 4 bytes
        0x00,0x00,0x00,0x01,    // 2.5 Número de máquina de señal, el número único en el sistema de gama alta, 4 bytes
        0x01,           // 2.6 El número de intersecciones controladas por la máquina de señales, 1 byte, valor 1-8
        0x00,           // 2.7 Indicador de reloj GPS, 1 byte, tipo bool, 0 no es válido, 1 es válido
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//espacio reservado 11 bytes
    };
    get_cpuid(&BasicInfo_buf[17]);//La configuración de MAC está vinculada al cpuid, que se puede garantizar que es único.
    memcpy(BasicInfo.IntersectionInfo, IntersectionInfo, 128);
    memcpy(BasicInfo.IPv4.IP, BasicInfo_buf, 48);
}



