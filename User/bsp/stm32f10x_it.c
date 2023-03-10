/*
*********************************************************************************************************
*	                                  
*	Nombre del módulo: módulo de interrupción
*	Nombre del archivo: stm32f10x_it.c
*	Versión: V2.0
*	ilustrar: 
*			Simplemente agregue la función de interrupción requerida. Generalmente, el nombre de la función de interrupción es fijo, a menos que se modifique el archivo de inicio:
*				Libraries\CMSIS\CM3\DeviceSupport\ST\STM32F10x\startup\arm\startup_stm32f10x_hd.s
*			
*			El archivo de inicio es un archivo en lenguaje ensamblador, que define la función de servicio de cada interrupción.Estas funciones usan la palabra clave WEAK, que significa definición débil, por lo que si
*			Si redefinimos la función de servicio (debe tener el mismo nombre) en el archivo c, entonces la función de interrupción del archivo de inicio no será válida automáticamente. Esto es también
*			El concepto de redefinición de funciones es similar al significado de sobrecarga de funciones en C++
。
*				
*	registro de modificación :
*
*
*********************************************************************************************************
*/

#include "stm32f10x_it.h"

#define ERR_INFO "\r\nEnter HardFault_Handler, System Halt.\r\n"

/*
*********************************************************************************************************
*	Cortex-M3 Rutina de servicio de interrupción de excepción del kernel
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*	Nombre de la función: NMI_Handler
*	Descripción de la función: Rutina de servicio de interrupción no enmascarable.
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/  
void NMI_Handler(void)
{
}

/*
*********************************************************************************************************
*	Nombre de la función: HardFault_Handler
*	Descripción de la función: rutina de servicio de interrupción de falla de hardware.
* 	Parámetros formales: ninguno
* 	Valor devuelto: Ninguno
*********************************************************************************************************
*/ 
void HardFault_Handler(void)
{
#if 0
  const char *pError = ERR_INFO;
  uint8_t i;

  for (i = 0; i < sizeof(ERR_INFO); i++)
  {
     USART1->DR = pError[i];
     /* Esperar a que finalice el envío */
     while ((USART1->SR & USART_FLAG_TC) == (uint16_t)RESET);
  }
#endif	
  
#if 1	/* Cuando ocurre una excepción, hace sonar el zumbador */	
	while(1)
	{
		uint16_t m;
		//GPIOD->BSRR = GPIO_Pin_1;
		for (m = 0; m < 10000; m++);
	}
#else
	
  /* Ingrese un bucle infinito cuando ocurra una excepción de falla de hardware */
  while (1)
  {
  }
#endif  
}

/*
*********************************************************************************************************
*	Nombre de la función: MemManage_Handler
*	Descripción de la función: rutina de servicio de interrupción de excepción de administración de memoria.。
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
*********************************************************************************************************
*/   
void MemManage_Handler(void)
{
  /* Introduce un bucle infinito cuando se produce una excepción de gestión de memoria */
  while (1)
  {
  }
}

/*
*********************************************************************************************************
*	Nombre de la función: BusFault_Handler
*	Descripción de la función: Rutina de servicio de interrupción anormal de acceso al bus.
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
*********************************************************************************************************
*/    
void BusFault_Handler(void)
{
  /* Entra en un bucle infinito cuando el bus es anormal */
  while (1)
  {
  }
}

/*
*********************************************************************************************************
*	Nombre de la función: UsageFault_Handler
*	Descripción de la función: Instrucción indefinida o rutina de servicio de interrupción de estado ilegal.
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
*********************************************************************************************************
*/   
void UsageFault_Handler(void)
{
  /* Introduce un bucle infinito cuando el uso es anormal */
  while (1)
  {
  }
}

/*
*********************************************************************************************************
* Nombre de la función: SVC_Handler
* Descripción de la función: Llame a la rutina de servicio de interrupción a través del servicio del sistema de la instrucción SWI.
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
*********************************************************************************************************
*/   
void SVC_Handler(void)
{
}

/*
*********************************************************************************************************
*	Nombre de la función: DebugMon_Handler
*	Descripción de la función: rutina de servicio de interrupción del monitor de depuración.
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
*********************************************************************************************************
*/   
void DebugMon_Handler(void)
{
}

/*
*********************************************************************************************************
*	Nombre de la función: PendSV_Handler
*	Descripción de la función: El servicio del sistema que se puede suspender llama a la rutina de servicio de interrupción.
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
*********************************************************************************************************
*/     
void PendSV_Handler(void)
{
}


/*
*********************************************************************************************************
*	Rutina de servicio de interrupción de periférico interno STM32F10x
*	Los usuarios agregan y usan funciones de servicio de interrupción de periféricos aquí. Para conocer los nombres válidos de las funciones del servicio de interrupción, consulte el archivo de inicio (startup_stm32f10x_xx.s)
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*	Nombre de la función: PPP_IRQHandler
*	Descripción de la función: Rutina de servicio de interrupción de periféricos
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
*********************************************************************************************************
*/    
/* 
	Debido a que las rutinas de servicio de interrupción a menudo están relacionadas con aplicaciones específicas, se utilizarán variables y funciones de los módulos de funciones del usuario. Si se amplía en este documento, un gran número de
	Declaraciones de variables externas o declaraciones de inclusión.
	
	Por lo tanto, recomendamos que solo se escriba una declaración de llamada en este lugar y que el cuerpo de la función de servicio de interrupción se coloque en el módulo de función de usuario correspondiente.
	Agregar una capa de llamadas reducirá la eficiencia de ejecución del código, pero preferimos perder esta eficiencia para mejorar la modularidad del programa.
	
	Agregue la palabra clave extern para hacer referencia directa a las funciones externas utilizadas y evite incluir los archivos de encabezado de otros módulos en el encabezado del archivo.
extern void ppp_ISR(void);	
void PPP_IRQHandler(void)
{
	ppp_ISR();
}
*/
extern void can_ISR(void);
extern void USB_Istr(void);
void USB_LP_CAN1_RX0_IRQHandler(void)
{	
    can_ISR();
}
