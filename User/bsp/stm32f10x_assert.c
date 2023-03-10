/*
*********************************************************************************************************
*	                                  
*	module-name : módulo de aserción.
*	Nombre del archivo: stm32f10x_assert.c
*	Versión: V1.0
*	Descripción: proporciona la función de aserción, utilizada principalmente para la depuración de programas. Las funciones de la biblioteca de firmware ST pueden comprobar los parámetros de entrada para mejorar la solidez del programa.
*	Registro de modificación:
*
*
*********************************************************************************************************
*/

#include "stm32f10x.h"	/* Este archivo contiene stm32f10x_conf.h, el archivo stm32f10x_conf.h define USE_FULL_ASSERT */
#include <stdio.h>

/* 
	Las funciones de la biblioteca ST utilizan la función de afirmación del compilador C. Si se define USE_FULL_ASSERT, todas las funciones de la biblioteca ST verificarán los parámetros de la función.
	es correcto o no. Si es incorrecto, se llamará a la función assert_failed() Esta función es un bucle infinito, lo cual es conveniente para que los usuarios verifiquen el código.。
	
	La palabra clave __LINE__ indica el número de línea del código fuente.
	La palabra clave __FILE__ representa el nombre del archivo de código fuente.
	
	Una vez habilitada la función de aserción, el tamaño del código aumentará. Se recomienda que los usuarios solo la habiliten durante la depuración y la prohíban cuando el software se lance oficialmente.

	El usuario puede elegir si habilitar la aserción de la biblioteca de firmware ST. Hay dos formas de habilitar aserciones:
	(1) Defina USE_FULL_ASSERT en las opciones de macro predefinidas del compilador C.
	(2) Quite el comentario de la línea "#define USE_FULL_ASSERT 1" en este archivo.	
*/
#ifdef USE_FULL_ASSERT

/*
*********************************************************************************************************
*	Nombre de la función: assert_failed
*	Parámetros formales: archivo: nombre del archivo de código fuente. La palabra clave __FILE__ representa el nombre del archivo de código fuente.
*			  línea: número de línea de código. La palabra clave __LINE__ indica el número de línea del código fuente
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
	/* 
		Los usuarios pueden agregar su propio código para informar el nombre del archivo del código fuente y el número de línea del código, como imprimir el archivo de error y el número de línea en el puerto serie.
		printf("Wrong parameters value: file %s on line %d\r\n", file, line)
	*/
	
	/* Este es un bucle infinito. Cuando la afirmación falla, el programa se bloqueará aquí, para que los usuarios puedan comprobar si hay errores */
	while (1)
	{
	}
}
#endif
