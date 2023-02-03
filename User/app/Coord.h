#ifndef COORD_H
#define COORD_H

#include "public.h"

typedef struct
{
    uint8_t OperationalMode;   //协调运行模式 0自动 1-253手动方案 254本地感应 255闪光
    uint8_t CorrectionMode;    //协调校正方式 1其他 2驻留等待 3平滑过渡 4仅增加
    uint8_t MaximumMode;       //协调最大方式 1其他 2最大1 3最大2 4最大约束
    uint8_t ForceMode;         //协调强制方式 1其他 2浮动 3固定
    uint8_t Reserve[12];
}CoordType;   //协调表


extern CoordType   Coord;

void CoordInit(void);

#endif
