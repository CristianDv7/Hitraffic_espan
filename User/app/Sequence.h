#ifndef SEQUENCE_H
#define SEQUENCE_H
#include "public.h"


/* RingStatus环状态定义 */
#define ForceOFF    0x20
#define MaxOut      0x10
#define GapOut      0x08

typedef enum
{
    MinGreen = 0, 
    Extension, 
    Maximum, 
    GreenRest, 
    YellowChange, 
    RedClearance, 
    RedRest
}RingStatus_Type;

/******************************************************************************/
typedef struct 
{
    uint8_t RingNum;            //环号
    uint8_t Phase[PhaseMax];    //环相序
}RingType;  //环相序定义   17 byte

typedef struct
{
    uint8_t     Num;            //相序号
    RingType    Ring[RingMax];  //环相序定义 4个环
}SequenceType;        //相序表信息 69 byte

typedef struct
{
    uint8_t         Maximum;
    SequenceType    Seq[SequenceMax];     //16
    uint8_t         Reserve[15];
}SequenceTable;      //相序表  16 + (17*4+1)*16 = 1120 = 0x0460

/******************************************************************************/
typedef struct
{
    uint8_t StopTime;   //bit1,停止计时
    uint8_t ForceOff;   //bit1,强制关停
    uint8_t Max2;       //
    uint8_t MaxInhibit;
    uint8_t PedRecycle;
    uint8_t RedRest;
    uint8_t OmitRedClear;
}RingControlType;
/******************************************************************************/







extern uint8_t          RingStatus[RingMax];
extern SequenceType     SequenceNow;
extern SequenceTable    SeqTab;   //相序表


uint8_t GetSeqMax(RingType* Ring);

void SequenceDefault(void);

#endif
