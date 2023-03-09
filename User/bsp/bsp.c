/*
*********************************************************************************************************
* Nombre del módulo: módulo BSP (para STM32F103)
* Nombre del archivo: bsp.c
* Version : V1.0 
* Descripción: Este es el archivo principal del módulo del controlador subyacente del hardware. Principalmente proporciona la función bsp_Init() para que la llame el programa principal. Cada archivo c del programa principal se puede abrir en 
* Header Add #include "bsp.h" para incluir todos los módulos de controladores periféricos.
*********************************************************************************************************
*/

#include "bsp.h"
#include "PhaseStateControl.h"

/*
*********************************************************************************************************
* Nombre de la función: bsp_Init
* Descripción de la función: Inicializar el dispositivo de hardware. Solo necesita ser llamado una vez. Esta función configura los registros de la CPU y los registros periféricos e inicializa algunas variables globales.
* Variables globales.
* Parámetros formales: ninguno 
* Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_Init(void)
{
/*
Dado que el archivo de inicio de la biblioteca de firmware ST ya ha
ejecutó la inicialización del reloj del sistema de la CPU, no hay
necesita repetir la configuración del reloj del sistema nuevamente. El
El archivo de inicio configura la frecuencia del reloj principal de la CPU, interno
Velocidad de acceso flash e inicialización SRAM FSMC externa opcional.
La configuración predeterminada del reloj del sistema es de 72 MHz, si necesita
para cambiarlo, puede modificar el archivo system_stm32f103.c
*/

	/* La agrupación de prioridad se establece en 4 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	bsp_InitLed();		/* Puerto GPIO para configurar LED */
    bsp_InitIO();
    LampDriveOff();
    W5500_GPIO_Configuration();
    
	bsp_InitTimer();	/* Inicializar el temporizador de ticks del sistema (esta función abrirá la interrupción) */
	bsp_InitUart();		/* Inicializar el controlador del puerto serie*/
	can_Init();        /* Inicializar hardware STM32 CAN*/
	can_NVIC_Config(); /* Configurar interrupción CAN */
    
    bsp_DelayMS(100);
    RtcInit();
    bsp_InitSpiBus();   // Inicializar la interfaz spi
	Ch376t_Init();      // configuración de pin CS INT
	Fm25v_Init();	    // Configurar PE11-Fm25v_CS PE12-W25Q_CS PE15-W5500_CS
    
	Fm25v_ReadInfo();   /* Modelo de chip de identificación automática */
	
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
Nombre de la función: bsp_Idle
* Descripción de la función: la función ejecutada cuando está inactiva. Generalmente, el programa principal necesita insertar la macro CPU_IDLE() en el cuerpo de los bucles for y while para llamar a esta función.
* Esta función no está operativa de forma predeterminada. Los usuarios pueden agregar las funciones de alimentar al perro y configurar la CPU en modo de suspensión. 
* Parámetros formales: ninguno
* Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_Idle(void)
{
	/* --- Alimenta al perro */
	/* --- Deje que la CPU se duerma, se despierte con la interrupción del temporizador Systick u otras interrupciones */
	/* Por ejemplo, la biblioteca de gráficos emWin, puede insertar la función de sondeo requerida por la biblioteca de gráficos */
	/* GUI_Exec(); */ 
	/* Por ejemplo, el protocolo uIP, se puede insertar la función de sondeo uip */
}
