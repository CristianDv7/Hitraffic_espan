#ifndef __BSP_IWDG_H
#define __BSP_IWDG_H

#include "stm32f10x.h"

void IWDG_Feed(void);
void bsp_InitIwdg(uint32_t _ulIWDGTime);


#endif 
