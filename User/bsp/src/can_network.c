/*
*********************************************************************************************************
*	                                  
*	Nombre del módulo: programa de red CAN.
*	Nombre del archivo: can_network.c
*	Versión: V1.0
*
*********************************************************************************************************
*/
#include "bsp.h"


/* definir variables globales */
CanTxMsg CanTxMsgStruct;			/* para enviar */
CanRxMsg CanRxMsgStruct;			/* para recibir */


/*
*********************************************************************************************************
*	Nombre de la función: SendCanMsg
*	Descripción de la función: enviar un paquete de datos
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void SendCanMsg(uint8_t *p, uint8_t length)
{
	uint8_t n;
	CanTxMsgStruct.DLC = length;
	for(n = 0;n < length; n++)
	{
		CanTxMsgStruct.Data[n] = *p;
		p++;
	}
	CAN_Transmit(CAN1, &CanTxMsgStruct);
}

/*
*********************************************************************************************************
*	Nombre de la función: can_Init
*	Descripción de la función: configurar el hardware CAN
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void can_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;	
    
    //id    [31:24]     [23:16]                 [15:8]      [7:0]
    //mask  [31:24]     [23:16]                 [15:8]      [7:0]
    //map   stid[10:3]  stid[2:0]exid[17:13]    exid[12:5]  exid[4:0] IDE RTR 0
    
    uint16_t std_id = 0x10;
    uint32_t ext_id = 0;
    uint32_t mask = 0;
    
	/* Las líneas de puerto PA11, PA12 están configuradas en modo AFIO, cambie a la función CAN */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	/* Habilitar reloj GPIO */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	/* Configurar pin de recepción de señal CAN: RX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	/* GPIO configurado como modo de entrada pull-up */
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* Configurar pin de envío de señal CAN: TX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		/* Configurar como salida push-pull multiplexada */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* Establecer la velocidad máxima de GPIO */
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* El pin CAN original y el pin USB son la misma línea de puerto, la función de reasignación de pin cambia el pin CAN a PB8, PB9 */
	GPIO_PinRemapConfig(GPIO_Remap1_CAN1 , ENABLE);	/* Habilitar la reasignación de CAN1 */
	
	/* Habilitar reloj periférico CAN */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	CAN_DeInit(CAN1);				/* Restablecer registros CAN */
	CAN_StructInit(&CAN_InitStructure);		/* Rellenar los miembros de la estructura de parámetros CAN con valores predeterminados */
	
	/*
		TTCM = time triggered communication mode
		ABOM = automatic bus-off management 
		AWUM = automatic wake-up mode
		NART = no automatic retransmission
		RFLM = receive FIFO locked mode 
		TXFP = transmit FIFO priority		
	*/
	CAN_InitStructure.CAN_TTCM = DISABLE;			/* Deshabilitar el modo activado por tiempo (no generar marcas de tiempo), T */
	CAN_InitStructure.CAN_ABOM = DISABLE;			/* deshabilitar la gestión de apagado automático de bus */
	CAN_InitStructure.CAN_AWUM = DISABLE;			/* Deshabilitar el modo de activación automática */
	CAN_InitStructure.CAN_NART = DISABLE;			/* Deshabilitar la retransmisión automática después de una pérdida o error de arbitraje */
	CAN_InitStructure.CAN_RFLM = DISABLE;			/* Deshabilitar el modo de bloqueo FIFO de recepción */
	CAN_InitStructure.CAN_TXFP = DISABLE;			/* Deshabilitar la prioridad FIFO de transmisión */
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	/* Establecer CAN en modo de trabajo normal */
	
	/* 
		Tasa de baudios CAN = RCC_APB1Periph_CAN / Prescaler / (SJW + BS1 + BS2);
		
		SJW = synchronisation_jump_width 
		BS = bit_segment
		
		En este ejemplo, establezca la tasa de baudios de CAN en 500 Kbps		
		Tasa de baudios CAN = 360000000 / 6 / (1 + 6 + 5) / = 500kHz		
	*/
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_8tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq;
	CAN_InitStructure.CAN_Prescaler = 9;
	CAN_Init(CAN1, &CAN_InitStructure);
	
	/* Establecer filtro CAN 0 */
	CAN_FilterInitStructure.CAN_FilterNumber = 0;		/* Número de filtro, 0-13, 14 filtros en total */
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;		/* modo de filtro, establece el modo de máscara de ID */
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	/* filtro de 32 bits */
    //Marco estándar
	CAN_FilterInitStructure.CAN_FilterIdHigh = std_id<<5;               /* Alto 16 bits de ID después de la máscara */
	CAN_FilterInitStructure.CAN_FilterIdLow = 0 | CAN_ID_STD;			/* Los 16 bits inferiores del ID después de la máscara */
    //Marco extendido
	//CAN_FilterInitStructure.CAN_FilterIdHigh = ((ext_id<<3)>>16)&0xffff;/* Alto 16 bits de ID después de la máscara */
	//CAN_FilterInitStructure.CAN_FilterIdLow = (ext_id<<3) | CAN_ID_EXT; /* ID bajo de 16 bits después de la máscara */
    
    // Ajustes de registro de máscara
	//La idea aquí es primero XOR los valores de ID correspondientes al CAN ID estándar y al CAN ID extendido, y luego invertirlos, ¿por qué?
    //XOR es para averiguar qué bits de los dos CAN ID son iguales, y si son iguales, significa que debemos preocuparnos.
    //El bit del código de máscara correspondiente al bit que debe preocuparse debe establecerse en 1, por lo que debe invertirse. Finalmente, se desplaza 3 bits a la izquierda en su conjunto.
    
    //¿Por qué te desplazas 18 bits a la izquierda aquí? Porque se puede ver desde ISO11898,
    //El CAN ID estándar ocupa ID18~ID28, para alinearse con CAN_FilterIdHigh, debe desplazarse a la izquierda 2 bits,
    //Entonces, para corresponder al CAN extendido, se deben desplazar otros 16 bits a la izquierda, por lo tanto, se debe desplazar a la izquierda un total de 2+16=18 bits.
    //También se puede entender de otra manera: mire directamente el contenido de Mapping y descubra que STDID está compensado por 18 bits en relación con EXID[0], por lo que está desplazado hacia la izquierda por 18 bits.
    
	mask =(std_id<<18);
	mask ^=ext_id;//XOR el CAN estándar alineado y el CAN extendido y luego invertir
	mask =~mask;
	mask <<=3;//Luego desplace a la izquierda por 3 bits como un todo
	mask |=0x02; //Recibir solo tramas de datos, no tramas remotas
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=(mask>>16)&0xffff; //Establecer el byte alto del registro de máscara
	CAN_FilterInitStructure.CAN_FilterMaskIdLow=mask&0xffff;   //Establecer el byte bajo del registro de máscara
    
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;		/* enlace de filtro FIFO 0 */
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;				/* habilitar filtro */
	CAN_FilterInit(&CAN_FilterInitStructure);
    /* Rellene los parámetros de envío, también puede rellenar cada vez que envíe */
	CanTxMsgStruct.StdId = 0x29;
	CanTxMsgStruct.ExtId = 0x00;
	CanTxMsgStruct.RTR = CAN_RTR_DATA;	//marco de datos - marco remoto
	CanTxMsgStruct.IDE = CAN_ID_STD;	//CAN_ID_EXD;		//marco estándar-marco extendido
}

/*
*********************************************************************************************************
*	Nombre de la función: can_NVIC_Config
*	Descripción de la función: Configurar interrupción CAN
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/  
void can_NVIC_Config(void)
{
	NVIC_InitTypeDef  NVIC_InitStructure;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Habilitación de interrupción de recepción de mensaje CAN FIFO0 */ 
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}

/*
*********************************************************************************************************
*	Nombre de la función: can_ISR
*	Descripción de la función: Rutina de servicio de interrupción CAN. Esta función se llama en stm32f10x_it.c
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/ 
void can_ISR(void)
{
	CAN_Receive(CAN1, CAN_FIFO0, &CanRxMsgStruct);
    bsp_LedToggle(LED_COM);
	if ((CanRxMsgStruct.StdId == 0x10) && (CanRxMsgStruct.IDE == CAN_ID_STD))// && (CanRxMsgStruct.DLC == 3)
	{
        printf_fifo_hex(CanRxMsgStruct.Data, CanRxMsgStruct.DLC);
        if((Manual_start == CanRxMsgStruct.Data[0] || Manual_auto == CanRxMsgStruct.Data[0]) && CanRxMsgStruct.DLC == 6)
        {
			CanTxMsgStruct.DLC = 6;
            memcpy(CanTxMsgStruct.Data, CanRxMsgStruct.Data, 6);
            CAN_Transmit(CAN1, &CanTxMsgStruct);
            //SendCanMsg(CanTxMsgStruct.Data, 6);
            
            if(Manual_start == CanRxMsgStruct.Data[0])//empezar manualmente
            {
                #if DEBUG
                printf("RemoteGuard\r\n");
                #endif
                if(Manual_appoint == CanRxMsgStruct.Data[1])
                {
                    ManualCtrl.Pos = CanRxMsgStruct.Data[2];
                    ManualCtrl.Dir = CanRxMsgStruct.Data[3];
                    ManualCtrl.AutoTime = CanRxMsgStruct.Data[4]|(CanRxMsgStruct.Data[5]<<8);
                    if(ManualCtrl.Pos == MANUAL_POS_Other || ManualCtrl.Dir == MANUAL_DIR_Other)
                    {
                        if(OP.WorkMode != ManualAppoint)
                        {
                            if(OP.WorkMode < Flashing) OP.WorkModeBK = OP.WorkMode;
                            OP.WorkMode = ManualAppoint;
                            ManualCtrl.StartFlag = 1;
                            ManualCtrl.RemoteCtrlFlag = 1;
                            ManualCtrl.EnforceFlag = 0;
                            ManualCtrl.OrderFlag = 0;
                            ManualCtrl.ExitFlag = 0;
                            //ManualCtrl.Time = 0;
                        }
                        #if DEBUG
                        printf("ManualCtrlT_resident \r\n");
                        #endif
                    }
                    else 
                    {
                        if(OP.WorkMode != ManualAppoint)
                        {
                            if(OP.WorkMode < Flashing) OP.WorkModeBK = OP.WorkMode;
                            OP.WorkMode = ManualAppoint;
                            ManualCtrl.StartFlag = 1;
                        }
                        
                        ManualCtrl.RemoteCtrlFlag = 1;
                        ManualCtrl.EnforceFlag = 1;
                        ManualCtrl.OrderFlag = 1;
                        ManualCtrl.ExitFlag = 0;
                        //ManualCtrl.Time = 0;
                        ManualCtrl.ChannelOnsNext = GetAppointChannel(ManualCtrl.Pos, ManualCtrl.Dir);
                        
                        #if DEBUG
                        printf("ManualCtrlT_Appoint\r\n");
                        printf("ChannelOnsNext = %04x\r\n", ManualCtrl.ChannelOnsNext);
                        printf("Pos = %02x\r\n", ManualCtrl.Pos);
                        printf("Dir = %02x\r\n", ManualCtrl.Dir);
                        #endif
                    }
                }
                else
                {
                    ManualCtrl.RemoteCtrlFlag = 1;
                    ManualCtrl.AutoTime = CanRxMsgStruct.Data[4]|(CanRxMsgStruct.Data[5]<<8);
                    ManualCtrl.Time = ManualCtrl.AutoTime;
                    if(OP.WorkMode < Flashing) OP.WorkModeBK = OP.WorkMode;
                    if(Manual_yellowflash == CanRxMsgStruct.Data[1])
                    {
                        OP.WorkMode = ManualFlashing;
                    }
                    else if(Manual_allred == CanRxMsgStruct.Data[1])
                    {
                        OP.WorkMode = ManualAllRead;
                    }
                    else if(Manual_lampoff == CanRxMsgStruct.Data[1])
                    {
                        OP.WorkMode = ManualLampOff;
                    }
                    else if(Manual_nextphase == CanRxMsgStruct.Data[1])
                    {
                        OP.WorkMode = ManualStep;
                        ManualCtrl.NextStepFlag = 1;
                    }
                }
                OP.WorkMode_Reason = WMR_RemoteGuard;
            }
            else if(Manual_auto == CanRxMsgStruct.Data[0])
            {
                if(OP.WorkMode == ManualAppoint)
                {
                    ManualCtrl.ExitFlag = 1;
                }
                else if(OP.WorkMode >= SPECIAL_MODE)
                {
                    ManualCtrl.LocalCtrlFlag = 0;
                    ManualCtrl.RemoteCtrlFlag = 0;
                    OP.WorkMode = OP.WorkModeBK;
                    OP.WorkMode_Reason = WMR_Normal;
                }
                #if DEBUG
                printf("ManualCtrlT_auto\r\n");
                #endif
            }
        }
	}
}
