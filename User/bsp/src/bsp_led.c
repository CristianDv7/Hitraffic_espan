/*
*********************************************************************************************************
*
*	模块名称 : LED指示灯驱动模块
*	文件名称 : bsp_led.c
*	版    本 : V1.0
*	说    明 : 驱动LED指示灯
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
    LED 高电平点亮 口线分配：

    LED1       : PE14
    LED2       : PE13
    LED3       : PE12
    LED4       : PE11

    LED5       : PE10
    LED6       : PE9 
    LED7       : PE8
    LED8       : PE7
*/
	#define RCC_ALL_LED 	RCC_APB2Periph_GPIOE	/* 按键口对应的RCC时钟 */

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
*	函 数 名: bsp_InitLed
*	功能说明: 配置LED指示灯相关的GPIO,  该函数被 bsp_Init() 调用。
*	形    参:  无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitLed(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 打开GPIO时钟 */
	RCC_APB2PeriphClockCmd(RCC_ALL_LED, ENABLE);

	/*
		配置所有的LED指示灯GPIO为推挽输出模式
		由于将GPIO设置为输出时，GPIO输出寄存器的值缺省是0
	*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		/* asigna el valor 0001 0000*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  /* definicion de valor del oscilador */
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LED1 | GPIO_PIN_LED2 | GPIO_PIN_LED3 | GPIO_PIN_LED4 | GPIO_PIN_LED5 | GPIO_PIN_LED6 | GPIO_PIN_LED7 | GPIO_PIN_LED8;
	GPIO_Init(GPIO_PORT_LED, &GPIO_InitStructure);
}

/*
*********************************************************************************************************
*	函 数 名: bsp_LedOn
*	功能说明: 点亮指定的LED指示灯。
*	形    参:  _no : 指示灯序号，范围 1 - 8
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
*	函 数 名: bsp_LedOff
*	功能说明: 熄灭指定的LED指示灯。
*	形    参:  _no : 指示灯序号，范围 1 - 8
*	返 回 值: 无
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
*	函 数 名: bsp_LedToggle
*	功能说明: 翻转指定的LED指示灯。
*	形    参:  _no : 指示灯序号，范围 1 - 8
*	返 回 值: 按键代码
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
*	函 数 名: bsp_IsLedOn
*	功能说明: 判断LED指示灯是否已经点亮。
*	形    参:  _no : 指示灯序号，范围 1 - 4
*	返 回 值: 1表示已经点亮，0表示未点亮
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
