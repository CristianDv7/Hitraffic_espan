/*
*********************************************************************************************************
*
*	Nombre del módulo: módulo de controlador de indicador LED
*	Nombre del archivo: bsp_led.c
*	Version : V1.0
*	Descripción: Indicadores LED de la unidad
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
    El LED de alto nivel se ilumina Distribución de la línea del puerto:

    LED1       : PE14
    LED2       : PE13
    LED3       : PE12
    LED4       : PE11

    LED5       : PE10
    LED6       : PE9 
    LED7       : PE8
    LED8       : PE7
*/
	#define RCC_ALL_LED 	RCC_APB2Periph_GPIOE	/* El reloj RCC correspondiente al puerto clave */

	#define GPIO_PORT_LED   GPIOE
	
	#define GPIO_PIN_LED1	GPIO_Pin_10
	#define GPIO_PIN_LED2	GPIO_Pin_9
	#define GPIO_PIN_LED3	GPIO_Pin_8
	#define GPIO_PIN_LED4	GPIO_Pin_7
	#define GPIO_PIN_LED5	GPIO_Pin_14
	#define GPIO_PIN_LED6	GPIO_Pin_13
	#define GPIO_PIN_LED7	GPIO_Pin_12
	#define GPIO_PIN_LED8	GPIO_Pin_11
/*
*********************************************************************************************************
*	Nombre de la función: bsp_InitLed
*	Descripción de la función: Configure el GPIO relacionado con el indicador LED, esta función es llamada por bsp_Init().
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_InitLed(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enciende el reloj GPIO */
	RCC_APB2PeriphClockCmd(RCC_ALL_LED, ENABLE);

	/*
		Configure todos los indicadores LED GPIO como modo de salida push-pull
		Cuando GPIO se establece como una salida, el valor del registro de salida GPIO es 0 de forma predeterminada
	*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		/* asigna el valor 0001 0000*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  /* definicion de valor del oscilador */
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED1 | GPIO_PIN_LED2 | GPIO_PIN_LED3 | GPIO_PIN_LED4 | GPIO_PIN_LED5 | GPIO_PIN_LED6 | GPIO_PIN_LED7 | GPIO_PIN_LED8;
	GPIO_Init(GPIO_PORT_LED, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_LedOn
*	Descripción de la función: enciende el indicador LED especificado.
*	Parámetros formales: _no : Número de serie del LED, rango 1 - 8
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_LedOn(uint8_t _no)
{
	if (_no == 1)
	{
		GPIO_PORT_LED->BSRR = GPIO_PIN_LED1;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED->BSRR = GPIO_PIN_LED2;
	}
	else if (_no == 3)
	{
		GPIO_PORT_LED->BSRR = GPIO_PIN_LED3;
	}
	else if (_no == 4)
	{
		GPIO_PORT_LED->BSRR = GPIO_PIN_LED4;
	}
	else if (_no == 5)
	{
		GPIO_PORT_LED->BSRR = GPIO_PIN_LED5;
	}
	else if (_no == 6)
	{
		GPIO_PORT_LED->BSRR = GPIO_PIN_LED6;
	}
	else if (_no == 7)
	{
		GPIO_PORT_LED->BSRR = GPIO_PIN_LED7;
	}
	else if (_no == 8)
	{
		GPIO_PORT_LED->BSRR = GPIO_PIN_LED8;
	}
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_LedOff
*	Descripción de la función: apaga el indicador LED especificado.
*	Parámetros formales: _no : Número de serie del LED, rango 1 - 8
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_LedOff(uint8_t _no)
{
	if (_no == 1)
	{
		GPIO_PORT_LED->BRR = GPIO_PIN_LED1;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED->BRR = GPIO_PIN_LED2;
	}
	else if (_no == 3)
	{
		GPIO_PORT_LED->BRR = GPIO_PIN_LED3;
	}
	else if (_no == 4)
	{
		GPIO_PORT_LED->BRR = GPIO_PIN_LED4;
	}
	else if (_no == 5)
	{
		GPIO_PORT_LED->BRR = GPIO_PIN_LED5;
	}
	else if (_no == 6)
	{
		GPIO_PORT_LED->BRR = GPIO_PIN_LED6;
	}
	else if (_no == 7)
	{
		GPIO_PORT_LED->BRR = GPIO_PIN_LED7;
	}
	else if (_no == 8)
	{
		GPIO_PORT_LED->BRR = GPIO_PIN_LED8;
	}
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_LedToggle
*	Descripción de la función: voltear el indicador LED especificado.
*	Parámetros formales: _no : Número de serie del LED, rango 1 - 8
*	Valor de retorno: código clave
*********************************************************************************************************
*/
void bsp_LedToggle(uint8_t _no)
{
	if (_no == 1)
	{
		GPIO_PORT_LED->ODR ^= GPIO_PIN_LED1;
	}
	else if (_no == 2)
	{
		GPIO_PORT_LED->ODR ^= GPIO_PIN_LED2;
	}
	else if (_no == 3)
	{
		GPIO_PORT_LED->ODR ^= GPIO_PIN_LED3;
	}
	else if (_no == 4)
	{
		GPIO_PORT_LED->ODR ^= GPIO_PIN_LED4;
	}
	else if (_no == 5)
	{
		GPIO_PORT_LED->ODR ^= GPIO_PIN_LED5;
	}
	else if (_no == 6)
	{
		GPIO_PORT_LED->ODR ^= GPIO_PIN_LED6;
	}
	else if (_no == 7)
	{
		GPIO_PORT_LED->ODR ^= GPIO_PIN_LED7;
	}
	else if (_no == 8)
	{
		GPIO_PORT_LED->ODR ^= GPIO_PIN_LED8;
	}
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_IsLedOn
*	Descripción de la función: determina si el indicador LED está encendido.
*	Parámetros formales: _no : Número de serie del LED, rango 1 - 4
*	Valor de retorno: 1 significa que está encendido, 0 significa que no está encendido
*********************************************************************************************************
*/
uint8_t bsp_IsLedOn(uint8_t _no)
{
	if (_no == 1)
	{
		if ((GPIO_PORT_LED->ODR & GPIO_PIN_LED1) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 2)
	{
		if ((GPIO_PORT_LED->ODR & GPIO_PIN_LED2) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 3)
	{
		if ((GPIO_PORT_LED->ODR & GPIO_PIN_LED3) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 4)
	{
		if ((GPIO_PORT_LED->ODR & GPIO_PIN_LED4) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 5)
	{
		if ((GPIO_PORT_LED->ODR & GPIO_PIN_LED5) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 6)
	{
		if ((GPIO_PORT_LED->ODR & GPIO_PIN_LED6) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 7)
	{
		if ((GPIO_PORT_LED->ODR & GPIO_PIN_LED7) == 0)
		{
			return 1;
		}
		return 0;
	}
	else if (_no == 8)
	{
		if ((GPIO_PORT_LED->ODR & GPIO_PIN_LED8) == 0)
		{
			return 1;
		}
		return 0;
	}
	return 0;
}

/*************************************************************************************************/
