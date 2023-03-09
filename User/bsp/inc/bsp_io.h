#ifndef __BSP_OLED_H
#define __BSP_OLED_H
#include <stdint.h>


#define WIFI_WDT_PORT	    GPIOB
#define WIFI_WDT_PIN	    GPIO_Pin_4
#define WIFI_WDT_ON()	    WIFI_WDT_PORT->BSRR = WIFI_WDT_PIN
#define WIFI_WDT_OFF()	    WIFI_WDT_PORT->BRR = WIFI_WDT_PIN

#define WIFI_RST_PORT	    GPIOC
#define WIFI_RST_PIN	    GPIO_Pin_3
#define WIFI_RST_ON()	    WIFI_RST_PORT->BSRR = WIFI_RST_PIN
#define WIFI_RST_OFF()	    WIFI_RST_PORT->BRR = WIFI_RST_PIN

//蜂鸣器端口定义(PD1)
#define BEEP_PORT	        GPIOD
#define BEEP_PIN	        GPIO_Pin_1
#define BEEP_ON()	        BEEP_PORT->BSRR = BEEP_PIN
#define BEEP_OFF()	        BEEP_PORT->BRR = BEEP_PIN


//PPS
#define PPS_PORT	        GPIOC
#define PPS_PIN	            GPIO_Pin_13
#define PPS_ON()	        PPS_PORT->BSRR = PPS_PIN
#define PPS_OFF()	        PPS_PORT->BRR = PPS_PIN
#define PPS_Toggle()        PPS_PORT->ODR ^= PPS_PIN

#define VTI                 (GPIOE->IDR & GPIO_Pin_4)
#define ATI                 (GPIOE->IDR & 0x000F)

enum LED_TYPE{LED_POWER = 1, LED_RUN, LED_CON, LED_COM, \
              LED_NEWPLAN, LED_GPS, LED_MANUAL ,LED_ERROR};

//struct DRIVE_Typedef  //La definición de orden bajo primero en salir al impulsar la salida
//{
//    uint16_t error:1;
//    uint16_t special:1;
//    uint16_t run:1;
//    uint16_t power:1;
//    uint16_t group4:3;
//    uint16_t group3:3;
//    uint16_t group2:3;
//    uint16_t group1:3; //bit0-G bit1-Y bit2-R
//};

struct DRIVE_Typedef  //La definición de bit alto primero en salir al conducir la salida
{
    uint16_t group1:3; //bit2-G bit1-Y bit0-R
    uint16_t group2:3;
    uint16_t group3:3;
    uint16_t group4:3;
    uint16_t power:1;
    uint16_t run:1;
    uint16_t special:1;
    uint16_t error:1;
};

union DRIVE_REG{
    struct DRIVE_Typedef drivereg;
    uint16_t value;
};




typedef struct
{
    uint8_t (*State)(void);
    uint8_t rising;     //Señal de flanco ascendente válido
    uint8_t failing;    //Señal de flanco descendente válida
    uint8_t stab;       //estado estable
    uint8_t temp;       //estado provisional
    uint8_t counter[8]; //Estadísticas
}Input8_Type;

typedef struct
{
    uint16_t in_rising;
    uint16_t in_failing;
    uint16_t in_stab_state;
    uint16_t in_temp_state;
    uint16_t in_chge_counter[16];
}Input16_Type;

extern uint8_t rf_state;
extern Input8_Type peddet_hw;

extern union DRIVE_REG LampDriveReg[8];
extern union DRIVE_REG LampDriveRegBackup[8];

extern uint16_t switch_state_stab;
extern uint16_t current_state_stab;
extern uint16_t current_stab;

extern uint16_t red_state_stab;
extern uint16_t green_state_stab;
extern uint16_t red_state;
extern uint16_t green_state;


void LampDriveDataInit(void);
void LampDriveOut(void);
void LampDriveOff(void);


void rf315m_scan(void);
void bsp_InitIO(void);
void peddet_scan(Input8_Type* Input);
void lamp_state_detect(void);
uint8_t manual_scan(void);






#endif
