#include "stm32f10x.h"

//feed dog function
void IWDG_Feed(void)
{
	IWDG_ReloadCounter();
}
/*
*********************************************************************************************************
* Function name: bsp_InitIwdg 
* Function description: independent watchdog time configuration function 
* Formal parameter: IWDGTime: 0 ---- 0x0FFF * Independent watchdog time setting, the unit is ms, IWDGTime = 1000 is about one second 
* time 
* Return value: None	        
*********************************************************************************************************
*/
void bsp_InitIwdg(uint32_t _ulIWDGTime)
{
	/* Detects system recovery from independent watchdog reset*/
	if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
	{		
		/* clear reset flag */
		RCC_ClearFlag();
	}
    /* Write 0x5555 to allow access to IWDG_PR and IWDG_RLR registers */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	
	/*  LSI/32 frequency division*/
	IWDG_SetPrescaler(IWDG_Prescaler_32);
	
	IWDG_SetReload(_ulIWDGTime);
	
	/* Overload IWDG count */
	IWDG_ReloadCounter();
	
	/* Enable IWDG (LSI oscillator is enabled by hardware) */
	IWDG_Enable();
}
