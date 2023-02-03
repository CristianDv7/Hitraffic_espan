#ifndef SNMP_H
#define SNMP_H

#include <stdint.h>

typedef struct{
    uint8_t msgType;
    uint8_t msgLength;
    uint8_t snmpVersion;
    uint8_t pduType;
    uint32_t requestID;
    uint8_t errorStatus;
    uint8_t errorIndex;
    uint8_t DataLength;
}SNMP_MSG;


typedef enum {
    get_request = 0,
    get_next_request,
    get_response,
    set_request,
    trap,
}PUDType;

typedef enum {
    coldStart = 0,  //代理进程对自己初始化
    warmStart,      //代理进程对自己重新初始化
    linkDown,       //一个接口已从工作状态变为故障状态(报文中的第一个变量标识此接口)
    linkUp,         //一个接口已从故障状态变为工作状态(报文中的第一个变量标识此接口)
    authenticationFailure,  //从SNMP管理进程收到无效共同体的报文
    egpNeighborLoss,    //一个EGP邻站已变为故障状态(报文中的第一个变量包含邻站IP地址)
    enterpriseSpecific, //在这个特定的代码段中查找trap信息
}trapType;

typedef enum {
    noError = 0,
    tooBig,
    noSuchName,
    BadValue,
    readOnly,
    enErr
}errorStatus;

// Tag Number options for ASN.1 Identifiers, bit positions 0 - 4
// results of TagNumberMask applied to Type of BER TLV Encoded Object
// results only meaningful if Form is Primitive
//#define BOOLEAN           0x01
//#define INTEGER           0x02
//#define BITSTRING         0x03
//#define OCTETSTRING       0x04
//#define ASN1NULL          0x05
//#define OBJECTIDENTIFIER  0x06
//#define OBJECTDESCRIPTOR  0x07
//#define EXTERNAL          0x08
//#define REAL              0x09
//#define ENUMERATEDTYPES   0x0a
//#define SEQUENCE          0x10
//#define SET               0x11
//#define NUMERICSTRING     0x12
//#define PRINTABLESTRING   0x13
//#define TELETEXSTRING     0x14    // T61 String
//#define VIDEOTEXSTRING    0x15
//#define IA5STRING         0x16
//#define UTCTIME           0x17
//#define GENERALIZEDTIME   0x18
//#define GRAPHICSTRING     0x19
//#define VISIBLESTRING     0x1a    // ISO 646 String
//#define GENERALSTRING     0x1b
//#define SKIPIT            0x20    // FDS use

//// SNMP Application Class Tags
//#define IPADDRESS         0x40
//#define COUNTER           0x41
//#define GAUGE             0x42
//#define TIMETICKS         0x43
//#define OPAQUE            0x44
////                        0x45 not defined
//#define COUNTER64         0x46

typedef enum 
{
    BOOL = 0x01, 
    INT = 0x02,
    OCTSTR = 0x04,
    NULLT = 0x05,
    OBJID = 0x06,
    ENUM = 0x0A,
    SEQUENCE = 0x30,
    SETOF = 0x31,
    IPADDR = 0x40,
    COUNTER = 0x41,
    GAUGE = 0x42,
    TIMETICKS = 0x43,
    OPAQUE = 0x44,
    GET = 0xA0,
    GETNEXT = 0xA1,
    GETResp = 0xA2,
    SETO = 0xA3,
    TRAP = 0xA4
} ParType;






#endif
