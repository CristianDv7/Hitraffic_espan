/*
*********************************************************************************************************
*
*	Nombre del módulo: módulo de controlador de bus I2C
*	Nombre del archivo: bsp_i2c.c
*********************************************************************************************************
*/

#include "bsp.h"

void bsp_Init_RTC_IRQ(void)  
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* GPIOB.5 configuración de línea de interrupción IRQ0 */
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource5);
  /* Configuración de inicialización de interrupción GPIOB.5 */
  	EXTI_InitStructure.EXTI_Line = EXTI_Line5;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   /* Inicializar el registro EXTI periférico de acuerdo con los parámetros especificados en EXTI_InitStruct */
  	EXTI_Init(&EXTI_InitStructure);
	
   /*Habilitar el canal de interrupción externo donde se encuentra el botón*/
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    /*Establezca la prioridad de preferencia, la prioridad de preferencia se establece en 2*/	
  	//NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	
    /*Establecer la subprioridad, la subprioridad se establece en 2*/
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;		
    /* Habilitar paso de interrupción externa */
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	
  /*Inicialice los registros NVIC periféricos de acuerdo con los parámetros especificados en NVIC_InitStruct*/	
  	NVIC_Init(&NVIC_InitStructure);
}

/*
*********************************************************************************************************
*	Nombre de la función: bsp_InitI2C
*	Descripción de la función: Configure el GPIO del bus I2C, que se realiza mediante E/S analógica
*	Parámetros formales: ninguno
*	Valor devuelto: Ninguno
*********************************************************************************************************
*/
void bsp_InitI2C(void)  
{
    GPIO_InitTypeDef GPIO_InitStructure;   
    I2C_InitTypeDef I2C_InitStructure;   
  
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE );   
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);    
  
    /* PB6-I2C1_SCL PB7-I2C1_SDA */  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;   
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;   
    GPIO_Init(GPIOB, &GPIO_InitStructure);   

    /* PB5-RTC_IRQ */  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;   
    I2C_InitStructure.I2C_OwnAddress1 = 0XA0;   
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable; //Habilitar respuesta automática
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;   
    I2C_InitStructure.I2C_ClockSpeed = 50000;
  
    I2C_Init(I2C1, &I2C_InitStructure);
  
    I2C_Cmd(I2C1,ENABLE);
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    bsp_Init_RTC_IRQ(); // Inicializar la interfaz de interrupción RTC de la MCU
}

void I2C_ByteWrite(uint8_t Addr,uint8_t addr,uint8_t dataValue)  
{
    while(I2C_GetFlagStatus(I2C1,I2C_FLAG_BUSY));//
    
    I2C_GenerateSTART(I2C1,ENABLE);  
  
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));  
  
    I2C_Send7bitAddress(I2C1,Addr,I2C_Direction_Transmitter);  
  
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  
    I2C_SendData(I2C1,addr);
  
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_SendData(I2C1,dataValue);

    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTOP(I2C1,ENABLE);
}

uint8_t I2C_ByteRead(uint8_t Addr,uint8_t addr)  
{
    uint8_t dataValue;  
  
    while(I2C_GetFlagStatus(I2C1,I2C_FLAG_BUSY));  
  
    I2C_GenerateSTART(I2C1,ENABLE); //Start
  
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));
  
    I2C_Send7bitAddress(I2C1,Addr,I2C_Direction_Transmitter);//Write Address
  
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  
    I2C_Cmd(I2C1,ENABLE);  
  
    I2C_SendData(I2C1,addr);//Write address
  
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_TRANSMITTED));  
  
    I2C_GenerateSTART(I2C1,ENABLE);//Start
  
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_MODE_SELECT));  
  
    I2C_Send7bitAddress(I2C1,Addr,I2C_Direction_Receiver);
  
    while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));  
  
    I2C_AcknowledgeConfig(I2C1,DISABLE);  
  
    I2C_GenerateSTOP(I2C1,ENABLE);  
  
    while(!(I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_RECEIVED)));  
  
    dataValue=I2C_ReceiveData(I2C1);
  
    return dataValue;  
}

uint8_t I2C_Master_BufferRead(uint8_t* pBuffer, uint8_t NumByteToRead, uint8_t DataAddress, uint8_t SlaveAddress)
{
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));  
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    /* 1.Start */
    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    /* 2.Write Slave Address */
    I2C_Send7bitAddress(I2C1, SlaveAddress, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    /* 3.Write Data Address */
    I2C_SendData(I2C1, DataAddress);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    
    /* 4.Start*/
    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    /* 5.Write Slave Address */
    I2C_Send7bitAddress(I2C1, SlaveAddress, I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    /* 6.conturn Write data */
    while(NumByteToRead)
    {
        if(NumByteToRead==1)
        {
            I2C_AcknowledgeConfig(I2C1, DISABLE);//6.noack
            I2C_GenerateSTOP(I2C1, ENABLE);//STOP
        }

        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));  /* EV7 */
        *pBuffer++ = I2C_ReceiveData(I2C1);
        NumByteToRead--;
    }

    I2C_AcknowledgeConfig(I2C1, ENABLE);
    return 1;
}

uint8_t I2C_Master_BufferWrite(uint8_t* pBuffer, uint16_t NumByteToWrite, uint16_t DataAddress, uint8_t SlaveAddress)
{
    if(NumByteToWrite==0)
        return 1;

    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, SlaveAddress, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    while(NumByteToWrite--)
    {
      I2C_SendData(I2C1, *pBuffer);
      while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
      pBuffer++;
    }

    I2C_GenerateSTOP(I2C1, ENABLE);
    while((I2C1->CR1 & TIM_CR1_CKD_1) == TIM_CR1_CKD_1);
    return 0;
}

void Temperature_Read(void)
{
    uint8_t temp_buff[2];
	uint16_t temp_data;
    #if DEBUG
	float temp_out;
    #endif
    
    I2C_Master_BufferRead(temp_buff,2,0,LM75_ADDR);

	temp_data = ((uint16_t)temp_buff[0]<<3)|(((uint16_t)temp_buff[1]>>5)&0x07);
	if(temp_data & 0x400)
	{
		temp_data = ~temp_data + 1;
		temp_data &= 0x7ff;
		temp_data = -temp_data;
	}
	
    #if DEBUG
    temp_out = (float)temp_data * 0.125f;
	printf("\n\rTemperature_Read = %d %f",temp_data,temp_out);
    #endif
}

void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line5) != RESET) //Comprobar si se produce o no la solicitud de activación de línea EXTI5 especificada
	{
        RtcIrqCallback();
        EXTI_ClearITPendingBit(EXTI_Line5); /* borrar el bit pendiente de la línea EXTI5 */
    }
}



