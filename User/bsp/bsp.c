/*
*********************************************************************************************************
* Module name: BSP module (For STM32F103) 
* File name: bsp.c 
* Version : V1.0 
* Description: This is the main file of the hardware underlying driver module. It mainly provides the bsp_Init() function for the main program to call. Each c file of the main program can be opened in * Header Add #include "bsp.h" to include all peripheral driver modules.
*********************************************************************************************************
*/

#include "bsp.h"
#include "PhaseStateControl.h"

/*
*********************************************************************************************************
* Function name: bsp_Init 
* Function Description: Initialize the hardware device. Only needs to be called once. This function configures CPU registers and peripheral registers and initializes some global variables. 
* Global variables. 
* Formal parameters: none 
* Return value: None
*********************************************************************************************************
*/
void bsp_Init(void)
{
	/*
		Since the startup file of the ST firmware library has already 
	executed the initialization of the CPU system clock, there is no
	need to repeat the configuration of the system clock again. The 
	startup file configures the CPU main clock frequency, internal 
	Flash access speed and optional external SRAM FSMC initialization. 
	The default configuration of the system clock is 72MHz, if you need 
	to change it, you can modify the system_stm32f103.c file
	*/

	/* Priority grouping is set to 4 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	bsp_InitLed();		/* GPIO port to configure LED */
    bsp_InitIO();
    LampDriveOff();
    W5500_GPIO_Configuration();
    
	bsp_InitTimer();	/* Initialize the system tick timer (this function will open interrupt) */
	bsp_InitUart();		/* Initialize the serial port driver*/
	can_Init();         /* Initialize STM32 CAN hardware*/
	can_NVIC_Config();  /* Configure CAN interrupt */
    
    bsp_DelayMS(100);
    RtcInit();
    bsp_InitSpiBus();   // Initialize the spi interface
	Ch376t_Init();      // CS INT pin configuration
	Fm25v_Init();	    // Configure PE11-Fm25v_CS PE12-W25Q_CS PE15-W5500_CS
    
	Fm25v_ReadInfo();   /* Automatic identification chip model */
	
	bsp_Init_OLED_gpio();
    
	oled_Init();
	OLED_Clear(); 
}

volatile uint8_t reg1s_flag;
volatile uint8_t reg1ms_flag = 1;
volatile uint8_t reg10ms_flag;
volatile uint8_t reg100ms_flag;

volatile uint16_t reg1ms;
//volatile uint16_t reg1sCount;


void bsp_RunPer100ms(void)
{
    reg100ms_flag = 1;
    OP.Run100ms_flag = 1;
}


void bsp_RunPer10ms(void)
{
    reg10ms_flag = 1;
    OP.Run10ms_flag = 1;
    ChannelStatusToLmap();
    i2c_delay++;
}

void bsp_RunPer1ms(void)
{
    reg1ms_flag = 1;
	if(++reg1ms >= 1000)
    {
        reg1ms = 0;
        reg1s_flag = 1;
        OP.Run1s_flag = 1;
    }
    if(reg1ms%10 == 0)
        bsp_RunPer10ms();
    if(reg1ms%100 == 0)
        bsp_RunPer100ms();
}

/*
*********************************************************************************************************
Function name: bsp_Idle 
* Function description: The function executed when idle. Generally, the main program needs to insert the CPU_IDLE() macro in the body of the for and while loops to call this function. 
* This function is a no-op by default. Users can add the functions of feeding the dog and setting the CPU into sleep mode. 
* Formal parameters: none 
* Return value: None
*********************************************************************************************************
*/
void bsp_Idle(void)
{
	/* --- Feed the dog */ 
	/* --- Let the CPU go to sleep, wake up by Systick timer interrupt or other interrupts */ 
	/* For example, emWin graphics library, you can insert the polling function required by the graphics library */ 
	/* GUI_Exec(); */ 
	/* For example uIP protocol, uip polling function can be inserted */
}
