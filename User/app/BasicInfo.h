#ifndef BASICINFO_H
#define BASICINFO_H
#include <stdint.h>


//设备信息********************************************************************************************
typedef struct //main 1
{
	uint8_t ManufacturerInfo[128];  // 1.1 Información del fabricante 128 bytes, 0x00-0x7f
	uint8_t DeviceVersion[4];       // 1.2 Versión del dispositivo, 4 bytes, hardware alto de dos bits, software bajo de dos bits, 0x80-0x83
	uint8_t DeviceNumber[16];       // 1.3 Número de dispositivo 16 bytes, 0x84-0x93
	uint8_t ProductionDate[6];      // 1.4 Fecha de fábrica, 6 bytes, 0x94-0x9a
	uint8_t ConfigurationDate[6];   // 1.5 Fecha de configuración, 6 bytes, 0x9b-0xa1
    uint8_t Reserve[16];
}DeviceInfo_TypeDef;                // 1 Información del dispositivo

typedef struct
{
	uint8_t IP[4];                  // 2.2.1 Dirección IP, 4 bytes, 0x124-0x127
	uint8_t SubMask[4];             // 2.2.2 Submáscara, 4 bytes, 0x128-0x12b
	uint8_t GetWay[4];              // 2.2.3 GetWay,4byte,0x12c-0x12f
    uint8_t Socket[2];              // puerto
    uint8_t MAC[6];                 // Dirección MAC
}IPv4Info_TypeDef;                  // 2.2 Tabla de información de IPv4 20 bytes

typedef struct
{
	uint8_t RemoteIP[4];            // 2.3.1 RemoteIP,4byte,0x130-0x133
    uint8_t RemoteSocket[2];        // 2.3.2 RemoteSocket,2byte,0x134-0x135
    uint8_t CommunicationType;      // 2.3.3 Tipo de comunicación, 1 byte, valor 1-3, 0x136
}IPv4RemoteInfo_TypeDef;            // 2.3 Tabla de información de la computadora central IPv4

typedef struct //main 2
{
	uint8_t IntersectionInfo[128];      // 2.1 Instalar información de intersección, 128 bytes, 0x0a4-0x123
    IPv4Info_TypeDef IPv4;              // 2.2 Tabla de información de IPv4 20
    IPv4RemoteInfo_TypeDef IPv4Remote;  // 2.3 Tabla de información de la computadora central IPv4 7
    uint8_t TimeZone[4];                // 2.4 Zona horaria, valor -43200~43200, 4 bytes, 0x137-0x13a; TimeZone[0] == 1 zona este, 0 zona oeste 1, 2, 3 son segundos de zona horaria, byte alto primero
    uint8_t TscNumber[4];               // 2.5 Número de señal, el número único en el sistema superior, 4 bytes, 0x13b-0x13e
    uint8_t ControlIntersectionNumber;  // 2.6 Número de intersecciones controladas por la máquina de señales, 1 byte, valor 1-8, 0x13f
    uint8_t GpsClockFlag;               // 2.7 Indicador de reloj GPS, 1 byte, tipo bool, 0 no es válido, 1 es válido, 0x140
    //IPv6Info_TypeDef IPv6;            // 2.8 Hoja de información de IPv6
    //IPv6RemoteInfo_TypeDef IPv6Remote;// 2.9 Tabla de información de la computadora central IPv6
    uint8_t Reserve[11];
}BasicInfo_TypeDef; //128 + 20 + 7 + 8 + 13 = 176 



extern DeviceInfo_TypeDef   DeviceInfo;
extern BasicInfo_TypeDef    BasicInfo;


void BasicInfoInit(void);
void DeviceInfoInit(void);




#endif
