#ifndef _BSP_OLED_H
#define _BSP_OLED_H
#include <stdint.h>



void bsp_Init_OLED_gpio(void);
void oled_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr);
void OLED_ShowString(uint8_t x, uint8_t y, char *chr);


#endif
