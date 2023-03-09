/*
*********************************************************************************************************
*
*	Nombre del módulo: Módulo de temporizador
*	Nombre del archivo: bsp_timer.c
*	Versión: V1.3
*	Descripción: configura el sysstick timer como el cronómetro del sistema. El período de tiempo predeterminado es de 1 ms.
*
*				Se implementaron múltiples temporizadores de software para que los use el programa principal (precisión de 1 ms), puede aumentar o disminuir la cantidad de temporizadores modificando TMR_COUNT
*				Función de retraso de nivel de ms implementada (precisión de 1 ms) y función de retraso de nivel de EE. UU.
*				Se dio cuenta de la función de tiempo de ejecución del sistema (unidad de 1 ms)
*
*********************************************************************************************************
*/

#include "bsp.h"

/*
	Defina TIM para el temporizador de hardware, puede hacer TIM2 - TIM5
	TIM3 y TIM4 son de 16 bits
	TIM2 y TIM5 son 16 bits (103 es 16 bits, 407 es 32 bits)
*/
#define USE_TIM2

#ifdef USE_TIM2
	#define TIM_HARD		TIM2
	#define TIM_HARD_IRQn	TIM2_IRQn
	#define TIM_HARD_RCC	RCC_APB1Periph_TIM2
#endif

#ifdef USE_TIM3
	#define TIM_HARD		TIM3
	#define TIM_HARD_IRQn	TIM3_IRQn
	#define TIM_HARD_RCC	RCC_APB1Periph_TIM3
#endif

#ifdef USE_TIM4
	#define TIM_HARD		TIM4
	#define TIM_HARD_IRQn	TIM4_IRQn
	#define TIM_HARD_RCC	RCC_APB1Periph_TIM4
#endif

#ifdef USE_TIM5
	#define TIM_HARD		TIM5
	#define TIM_HARD_IRQn	TIM5_IRQn
	#define TIM_HARD_RCC	RCC_APB1Periph_TIM5
#endif

/* Estas 2 variables globales están dedicadas a la función bsp_DelayMS() */
static volatile uint32_t s_uiDelayCount = 0;
static volatile uint8_t s_ucTimeOutFlag = 0;

/* Definir variables de estructura de temporizador de software */
static SOFT_TMR s_tTmr[TMR_COUNT];

/*
	Tiempo de funcionamiento global, unidad 1 ms
	Puede representar hasta 24,85 días. Si su producto se ejecuta continuamente durante más de este número, debe considerar el problema de desbordamiento.
*/
__IO uint32_t g_iRunTime = 0;

static void bsp_SoftTimerDec(SOFT_TMR *_tmr);

/* Guardar el puntero de función de devolución de llamada ejecutado después de la interrupción de temporización de TIM */
static void (*s_TIM_CallBack1)(void);
static void (*s_TIM_CallBack2)(void);
static void (*s_TIM_CallBack3)(void);
static void (*s_TIM_CallBack4)(void);

/*
*********************************************************************************************************
*	Nombre de la función: bsp_InitTimer
*	Descripción de la función: configurar la interrupción del sysstick e inicializar la variable del temporizador de software
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_InitTimer(void)
{
	uint8_t i;

	/* Borrar todos los temporizadores de software */
	for (i = 0; i < TMR_COUNT; i++)
	{
		s_tTmr[i].Count = 0;
		s_tTmr[i].PreLoad = 0;
		s_tTmr[i].Flag = 0;
		s_tTmr[i].Mode = TMR_ONCE_MODE;	/* El valor predeterminado es el modo de trabajo de una sola vez */
	}

	/*
		Configure el período de interrupción de sytic como 1 ms e inicie la interrupción de systick.

    	SystemCoreClock es el reloj central del sistema definido en el firmware, para STM32F4XX, generalmente 168MHz

    	El parámetro formal de la función SysTick_Config() indica cuántos ciclos del reloj del núcleo activarán una interrupción de temporización de Systick.
	    	-- SystemCoreClock / 1000 significa que la frecuencia de sincronización es de 1000 Hz, es decir, el período de sincronización es de 1 ms
	    	-- SystemCoreClock / 500 significa que la frecuencia de sincronización es de 500 Hz, es decir, el período de sincronización es de 2 ms
	    	-- SystemCoreClock / 2000 significa que la frecuencia de tiempo es 2000Hz, es decir, el período de tiempo es 500us

    	Para aplicaciones convencionales, generalmente tomamos el período de tiempo como 1 ms. Para CPU de baja velocidad o aplicaciones de bajo consumo, puede configurar el período de tiempo en 10 ms
    */
	SysTick_Config(SystemCoreClock / 1000);
	
#if defined (USE_TIM2) || defined (USE_TIM3)  || defined (USE_TIM4)	|| defined (USE_TIM5)
	bsp_InitHardTimer();
#endif
}

/*
*********************************************************************************************************
*	Nombre de la función: SysTick_ISR
*	Descripción de la función: rutina de servicio de interrupción de SysTick, ingrese una vez cada 1 ms
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
extern void bsp_RunPer1ms(void);
extern void bsp_RunPer10ms(void);
extern void bsp_RunPer100ms(void);

void SysTick_ISR(void)
{
	uint8_t i;

	/* entra cada 1 ms (solo para bsp_DelayMS) */
	if (s_uiDelayCount > 0)
	{
		if (--s_uiDelayCount == 0)
		{
			s_ucTimeOutFlag = 1;
		}
	}

	/* Cada 1 ms, decrementa el contador del temporizador del software */
	for (i = 0; i < TMR_COUNT; i++)
	{
		bsp_SoftTimerDec(&s_tTmr[i]);
	}

	/* El tiempo de ejecución global aumenta en 1 cada 1 ms */
	g_iRunTime++;
	if(g_iRunTime >= 0xFFFFFED8)	/* 4294967 segundo = 49.71 Día Esta variable es de tipo uint32_t, el número máximo es 0xFFFFFFFF */
	{
		g_iRunTime = 0;
	}

	bsp_RunPer1ms();		   /* Llamar a esta función cada 1 ms */
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_SoftTimerDec
*	Descripción de la función: Decrementa todas las variables del temporizador en 1 cada 1 ms. Debe ser llamado periódicamente por SysTick_ISR.
*	Parámetros formales: _tmr: puntero de variable de temporizador
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
static void bsp_SoftTimerDec(SOFT_TMR *_tmr)
{
	if (_tmr->Count > 0)
	{
		/* Si la variable del temporizador se reduce a 1, establece el indicador de llegada del temporizador */
		if (--_tmr->Count == 0)
		{
			_tmr->Flag = 1;

			/* Recargar automáticamente el contador si está en modo automático */
			if(_tmr->Mode == TMR_AUTO_MODE)
			{
				_tmr->Count = _tmr->PreLoad;
			}
		}
	}
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_DelayMS
*	Descripción de la función: retardo de nivel de ms, la precisión del retardo es más o menos 1 ms
*	Parámetros formales: n : duración del retardo, unidad 1 ms. n debe ser mayor que 2
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_DelayMS(uint32_t n)
{
	if (n == 0)
	{
		return;
	}
	else if (n == 1)
	{
		n = 2;
	}

	DISABLE_INT();  			/* desactivar la interrupción */

	s_uiDelayCount = n;
	s_ucTimeOutFlag = 0;

	ENABLE_INT();  				/* habilitar interrupción */

	while (1)
	{
		/*
			Espere a que transcurra el tiempo de retardo
			Nota: el compilador piensa que s_ucTimeOutFlag = 0, por lo que puede ser un error de optimización, por lo que la variable s_ucTimeOutFlag debe declararse como volátil
		*/
		if (s_ucTimeOutFlag == 1)
		{
			break;
		}
	}
}

/*
*********************************************************************************************************
*    Nombre de la función: bsp_DelayUS
*    Descripción de la función: retardo de nivel de EE. UU. Esta función debe llamarse después de que se haya iniciado el temporizador sysstick.
*    Parámetros formales: n: duración del retardo, unidad 1 us
*    Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_DelayUS(uint32_t n)
{
    uint32_t ticks;
    uint32_t told;
    uint32_t tnow;
    uint32_t tcnt = 0;
    uint32_t reload;
    
	reload = SysTick->LOAD;                
    ticks = n * (SystemCoreClock / 1000000);	 /* Número de ticks requeridos */ 
    
    tcnt = 0;
    told = SysTick->VAL;            /* Valor del contador recién ingresado */

    while (1)
    {
        tnow = SysTick->VAL;    
        if (tnow != told)
        {    
            /* SYSTICK es un contador decreciente */  
            if (tnow < told)
            {
                tcnt += told - tnow;    
            }
           /* decremento de recarga */
            else
            {
                tcnt += reload - tnow + told;    
            }        
            told = tnow;

            /* El tiempo excede/igual al tiempo a retrasar, luego sale */
            if (tcnt >= ticks)
            {
            	break;
            }
        }  
    }
}


/*
*********************************************************************************************************
*	Nombre de la función: bsp_StartTimer
*	Descripción de la función: Iniciar un temporizador y establecer el período de tiempo.
*	Parámetros formales: _id: ID del temporizador, rango de valores [0,TMR_COUNT-1]. Los usuarios deben mantener la identificación del temporizador por sí mismos para evitar conflictos de identificación del temporizador。
*				_period : Periodo de tiempo, unidad 1ms
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_StartTimer(uint8_t _id, uint32_t _period)
{
	if (_id >= TMR_COUNT)
	{
		/* Imprime el nombre del archivo del código fuente del error, nombre de la función */
		BSP_Printf("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
		while(1); /* Parámetro anómalo, bloqueo esperando el restablecimiento del mecanismo de vigilancia */
	}

	DISABLE_INT();  			/* desactivar la interrupción */

	s_tTmr[_id].Count = _period;		/* contador en tiempo real */
	s_tTmr[_id].PreLoad = _period;		/* El contador recarga automáticamente el valor, solo funciona el modo automático */
	s_tTmr[_id].Flag = 0;				/* Tiempo de tiempo para marcar */
	s_tTmr[_id].Mode = TMR_ONCE_MODE;	/* Modo de trabajo de una sola vez */

	ENABLE_INT();  				/* habilitar interrupción */
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_StartAutoTimer
*	Descripción de la función: Iniciar un temporizador automático y establecer el período de tiempo.
*	Parámetros formales: _id: ID del temporizador, rango de valores [0,TMR_COUNT-1]. Los usuarios deben mantener las ID de temporizador por sí mismos para evitar conflictos de ID de temporizador.
*				_period : Periodo de tiempo, unidad 10ms
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_StartAutoTimer(uint8_t _id, uint32_t _period)
{
	if (_id >= TMR_COUNT)
	{
		/* Imprime el nombre del archivo del código fuente del error, nombre de la función */
		BSP_Printf("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
		while(1); /* Parámetro anómalo, bloqueo esperando el restablecimiento del mecanismo de vigilancia */
	}

	DISABLE_INT();  		/* desactivar la interrupción */

	s_tTmr[_id].Count = _period;		/* contador en tiempo real */
	s_tTmr[_id].PreLoad = _period;		/* El contador recarga automáticamente el valor, solo funciona el modo automático */
	s_tTmr[_id].Flag = 0;				/* Tiempo de tiempo para marcar */
	s_tTmr[_id].Mode = TMR_AUTO_MODE;	/* Modo de trabajo automático */

	ENABLE_INT();  			/* habilitar interrupción */
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_StopTimer
*	Descripción de la función: detener un temporizador
*	Parámetros formales: _id: ID del temporizador, rango de valores [0,TMR_COUNT-1]. Los usuarios deben mantener las ID de temporizador por sí mismos para evitar conflictos de ID de temporizador.
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_StopTimer(uint8_t _id)
{
	if (_id >= TMR_COUNT)
	{
		/* Imprime el nombre del archivo del código fuente del error, nombre de la función */
		BSP_Printf("Error: file %s, function %s()\r\n", __FILE__, __FUNCTION__);
		while(1); /* Parámetro anómalo, bloqueo esperando el restablecimiento del mecanismo de vigilancia */
	}

	DISABLE_INT();  	/* desactivar la interrupción */

	s_tTmr[_id].Count = 0;				/* Valor inicial del contador en tiempo real */
	s_tTmr[_id].Flag = 0;				/* Tiempo de tiempo para marcar */
	s_tTmr[_id].Mode = TMR_ONCE_MODE;	/* Modo de trabajo automático */

	ENABLE_INT();  		/* habilitar interrupción */
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_CheckTimer
*	Descripción de la función: compruebe si el temporizador está agotado
*	Parámetros formales: _id: ID del temporizador, rango de valores [0,TMR_COUNT-1]. Los usuarios deben mantener las ID de temporizador por sí mismos para evitar conflictos de ID de temporizador.
*				_period : Periodo de tiempo, unidad 1ms
*	Valor de retorno: retorno 0 significa que el temporizador no ha llegado, 1 significa que el temporizador ha llegado
*********************************************************************************************************
*/
uint8_t bsp_CheckTimer(uint8_t _id)
{
	if (_id >= TMR_COUNT)
	{
		return 0;
	}

	if (s_tTmr[_id].Flag == 1)
	{
		s_tTmr[_id].Flag = 0;
		return 1;
	}
	else
	{
		return 0;
	}
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_GetRunTime
*	Descripción de la función: obtenga el tiempo de ejecución de la CPU, la unidad es de 1 ms. Puede representar hasta 24,85 días. Si su producto se ejecuta continuamente durante más de este número, debe considerar el problema de desbordamiento.
*	Parámetros formales: ninguno
*	Valor devuelto: tiempo de ejecución de la CPU, unidad 1 ms
*********************************************************************************************************
*/
int32_t bsp_GetRunTime(void)
{
	int32_t runtime;

	DISABLE_INT();  	/* desactivar la interrupción */

	runtime = g_iRunTime;	/* Esta variable se reescribe en la interrupción de Systick, por lo que debe protegerse desactivando la interrupción */

	ENABLE_INT();  		/* habilitar interrupción */

	return runtime;
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_CheckRunTime
*	Descripción de la función: Calcular la diferencia entre el tiempo de ejecución actual y el momento dado. Se manejan los bucles de contador.
*	Parámetro formal: _LastTime última vez
*	Valor devuelto: la diferencia entre la hora actual y la hora pasada, la unidad es 1ms
*********************************************************************************************************
*/
int32_t bsp_CheckRunTime(int32_t _LastTime)
{
	int32_t now_time;
	int32_t time_diff;

	DISABLE_INT();  	/* desactivar la interrupción */

	now_time = g_iRunTime;	/* Esta variable se reescribe en la interrupción de Systick, por lo que debe protegerse desactivando la interrupción */

	ENABLE_INT();  		/* habilitar interrupción */
	
	if (now_time >= _LastTime)
	{
		time_diff = now_time - _LastTime;
	}
	else
	{
		time_diff = 0xFFFFFED8 - _LastTime + now_time;
	}

	return time_diff;
}

/*
*********************************************************************************************************
*	Nombre de la función: SysTick_Handler
*	Descripción de la función: Rutina de servicio de interrupción del temporizador de ticks del sistema. Se hace referencia a esta función en el archivo de inicio.
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void SysTick_Handler(void)
{
	SysTick_ISR();
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_InitHardTimer
*	Descripción de la función: Configure TIMx para sincronización de hardware de nivel estadounidense. TIMx funcionará libremente y nunca se detendrá.
*			TIMx puede usar TIM entre TIM2 - TIM5, estos TIM tienen 4 canales, cuelgan en APB1, reloj de entrada = SystemCoreClock / 2
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
#if defined (USE_TIM2) || defined (USE_TIM3)  || defined (USE_TIM4)	|| defined (USE_TIM5)
void bsp_InitHardTimer(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	uint32_t usPeriod;
	uint16_t usPrescaler;
	uint32_t uiTIMxCLK;

  	/* Habilitar reloj TIM */
	RCC_APB1PeriphClockCmd(TIM_HARD_RCC, ENABLE);

    /*-----------------------------------------------------------------------
		La función void SetSysClock(void) en el archivo system_stm32f4xx.c configura el reloj de la siguiente manera:

		HCLK = SYSCLK / 1     (AHB1Periph)
		PCLK2 = HCLK / 2      (APB2Periph)
		PCLK1 = HCLK / 4      (APB1Periph)

		Debido a que el preescalador APB1 != 1, entonces TIMxCLK en APB1 = PCLK1 x 2 = SystemCoreClock / 2;
		Debido a que el preescalador APB2 != 1, entonces TIMxCLK en APB2 = PCLK2 x 2 = SystemCoreClock;

		APB1 con temporizador TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14
		APB2 con temporizador TIM1, TIM8 ,TIM9, TIM10, TIM11

	----------------------------------------------------------------------- */
	uiTIMxCLK = SystemCoreClock / 2;

	usPrescaler = uiTIMxCLK / 1000000 ;	/* division de frecuencia al periodo 1us */
	
#if defined (USE_TIM2) || defined (USE_TIM5) 
	//usPeriod = 0xFFFFFFFF;	/* 407 admite temporizador de 32 bits */
	usPeriod = 0xFFFF;	/* 103 admite 16 bits */
#else
	usPeriod = 0xFFFF;
#endif
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = usPeriod;
	TIM_TimeBaseStructure.TIM_Prescaler = usPrescaler;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM_HARD, &TIM_TimeBaseStructure);

	//TIM_ARRPreloadConfig(TIMx, ENABLE);

	/* TIMx enable counter */
	TIM_Cmd(TIM_HARD, ENABLE);

	/* Configurar interrupción de temporización TIM (Actualización) */
	{
		NVIC_InitTypeDef NVIC_InitStructure;	/* La estructura de interrupción se define en misc.h */

		NVIC_InitStructure.NVIC_IRQChannel = TIM_HARD_IRQn;

		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;	/* Menor prioridad que el puerto serie */
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_StartHardTimer
*	Descripción de la función: use TIM2-5 como un temporizador único y ejecute la función de devolución de llamada después de que expire el temporizador. Se pueden iniciar 4 temporizadores al mismo tiempo sin interferir entre sí.
*            La precisión de tiempo es más o menos 10us (principalmente consumido en el tiempo de ejecución de llamar a esta función, y la función se compensa para reducir el error）
*			 TIM2 y TIM5 son temporizadores de 16 bits.
*			 TIM3 y TIM4 son temporizadores de 16 bits。
*	Parámetro formal: _CC: número de canal de captura, 1, 2, 3, 4
*             _uiTimeOut: tiempo de espera, unidad 1us. Para el temporizador de 16 bits, el máximo es 65,5 ms; para el temporizador de 32 bits, el máximo es 4294 segundos
*            _pCallBack: la función que se ejecutará después de que expire el temporizador
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void * _pCallBack)
{
    uint32_t cnt_now;
    uint32_t cnt_tar;

    /*
        Ejecute la siguiente declaración, duración = 18us (mida el cambio de IO por el analizador lógico)
        bsp_StartTimer2(3, 500, (void *)test1);
    */
    if (_uiTimeOut < 5)
    {
        ;
    }
    else
    {
        _uiTimeOut -= 5;
    }

    cnt_now = TIM_GetCounter(TIM_HARD);    	/* lee el valor actual del contador */
    cnt_tar = cnt_now + _uiTimeOut;			/* Calcular el valor del contador capturado */
    if (_CC == 1)
    {
        s_TIM_CallBack1 = (void (*)(void))_pCallBack;

        TIM_SetCompare1(TIM_HARD, cnt_tar);      	/* Establecer el contador de comparación de captura CC1 */
        TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC1);
		TIM_ITConfig(TIM_HARD, TIM_IT_CC1, ENABLE);	/* Habilitar interrupción CC1 */

    }
    else if (_CC == 2)
    {
		s_TIM_CallBack2 = (void (*)(void))_pCallBack;

        TIM_SetCompare2(TIM_HARD, cnt_tar);      	/* Establecer el contador de comparación de captura CC2 */
		TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC2);
		TIM_ITConfig(TIM_HARD, TIM_IT_CC2, ENABLE);	/* Habilitar interrupción CC2 */
    }
    else if (_CC == 3)
    {
        s_TIM_CallBack3 = (void (*)(void))_pCallBack;

        TIM_SetCompare3(TIM_HARD, cnt_tar);      	/* Establecer el contador de comparación de captura CC3 */
        TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC3);
		TIM_ITConfig(TIM_HARD, TIM_IT_CC3, ENABLE);	/* Habilitar interrupción CC3 */
    }
    else if (_CC == 4)
    {
        s_TIM_CallBack4 = (void (*)(void))_pCallBack;

        TIM_SetCompare4(TIM_HARD, cnt_tar);      	/* Establecer el contador de comparación de captura CC4 */
		TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC4);
		TIM_ITConfig(TIM_HARD, TIM_IT_CC4, ENABLE);	/* Habilitar interrupción CC4 */
    }
	else
    {
        return;
    }
}
#endif

/*
*********************************************************************************************************
*	Nombre de la función: TIMx_IRQHandler
*	Descripción de la función: Rutina de servicio de interrupción TIM
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/

#ifdef USE_TIM2
void TIM2_IRQHandler(void)
#endif

#ifdef USE_TIM3
void TIM3_IRQHandler(void)
#endif

#ifdef USE_TIM4
void TIM4_IRQHandler(void)
#endif

#ifdef USE_TIM5
void TIM5_IRQHandler(void)
#endif
{
    if (TIM_GetITStatus(TIM_HARD, TIM_IT_CC1))
    {
        TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC1);
        TIM_ITConfig(TIM_HARD, TIM_IT_CC1, DISABLE);	/* deshabilitar la interrupción CC1 */

       /* Primero deshabilite la interrupción y luego ejecute la función de devolución de llamada. Debido a que la función de devolución de llamada puede necesitar reiniciar el temporizador */
        s_TIM_CallBack1();
    }

    if (TIM_GetITStatus(TIM_HARD, TIM_IT_CC2))
    {
        TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC2);
        TIM_ITConfig(TIM_HARD, TIM_IT_CC2, DISABLE);	/* deshabilitar la interrupción CC2 */

        /* Primero deshabilite la interrupción y luego ejecute la función de devolución de llamada. Debido a que la función de devolución de llamada puede necesitar reiniciar el temporizador */
        s_TIM_CallBack2();
    }

    if (TIM_GetITStatus(TIM_HARD, TIM_IT_CC3))
    {
        TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC3);
        TIM_ITConfig(TIM_HARD, TIM_IT_CC3, DISABLE);	/* deshabilitar la interrupción CC3 */

        /* Primero deshabilite la interrupción y luego ejecute la función de devolución de llamada. Debido a que la función de devolución de llamada puede necesitar reiniciar el temporizador */
        s_TIM_CallBack3();
    }

    if (TIM_GetITStatus(TIM_HARD, TIM_IT_CC4))
    {
        TIM_ClearITPendingBit(TIM_HARD, TIM_IT_CC4);
        TIM_ITConfig(TIM_HARD, TIM_IT_CC4, DISABLE);	/* deshabilitar la interrupción CC4 */

        /* Primero deshabilite la interrupción y luego ejecute la función de devolución de llamada. Debido a que la función de devolución de llamada puede necesitar reiniciar el temporizador */
        s_TIM_CallBack4();
    }
}
