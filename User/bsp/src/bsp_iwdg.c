#include "stm32f10x.h"

// función de perro de alimentación
void IWDG_Feed(void)
{
	IWDG_ReloadCounter();
}
/*
*********************************************************************************************************
* Nombre de la función: bsp_InitIwdg
* Descripción de la función: función de configuración de tiempo de vigilancia independiente 
* Parámetro formal: IWDGTime: 0 ---- 0x0FFF * Configuración de tiempo de vigilancia independiente, la unidad es ms, IWDGTime = 1000 es aproximadamente un segundo 
* tiempo
* Valor devuelto: Ninguno	        
*********************************************************************************************************
*/
void bsp_InitIwdg(uint32_t _ulIWDGTime)
{
	/* Detecta la recuperación del sistema desde el restablecimiento del mecanismo de vigilancia independiente*/
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
