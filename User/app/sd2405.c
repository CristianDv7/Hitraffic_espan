/*
*********************************************************************************************************
*
*	Nombre del módulo: interfaz de operación de chip de reloj
*	Nombre del archivo: sd2405.c
*	Version : V1.0
*	Ilustrado : 
*	Registro de modificacion :
*		Número de versión Fecha Autor Descripción
*		V1.0    2019-03-19 wcx    Primer lanzamiento
*
*********************************************************************************************************
*/
#include "bsp_i2c.h" 
#include "sd2405.h" 
#include "stm32f10x_i2c.h" 
#include "stdio.h" 

uint8_t i2c_delay = 0;

//slave address
void RTC_WriteReg(uint8_t slave_addr, uint8_t WriteRegAddr, uint8_t WriteValue)
{ 
    i2c_delay = 0;
    I2cStart:
    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if(i2c_delay > 20) return;
        else if(i2c_delay > 10) goto I2cStart;
    }

    I2C_Send7bitAddress(I2C1, slave_addr, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if(i2c_delay > 20) return;
        else if(i2c_delay > 10) goto I2cStart;
    }

    /* 3.Write Data Address */
    I2C_SendData(I2C1, WriteRegAddr);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if(i2c_delay > 20) return;
        else if(i2c_delay > 10) goto I2cStart;
    }
    
    I2C_SendData(I2C1, WriteValue);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if(i2c_delay > 20) return;
        else if(i2c_delay > 10) goto I2cStart;
    }

    I2C_GenerateSTOP(I2C1, ENABLE);
    while((I2C1->CR1 & TIM_CR1_CKD_1) == TIM_CR1_CKD_1)
    {
        if(i2c_delay > 20) return;
        else if(i2c_delay > 10) goto I2cStart;
    }
}

uint8_t RTC_BufferRead(uint8_t slave_addr, uint8_t* pBuffer, uint8_t NumByteToRead, uint8_t DataAddress) 
{
    i2c_delay = 0;
    I2cStart:
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
    {
        if(i2c_delay > 20) return 0;
        else if(i2c_delay > 10) goto I2cStart;
    }
    I2C_AcknowledgeConfig(I2C1, ENABLE);
    /* 1.Start */
    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if(i2c_delay > 20) return 0;
        else if(i2c_delay > 10) goto I2cStart;
    }
    /* 2.Write Slave Address */
    I2C_Send7bitAddress(I2C1, slave_addr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if(i2c_delay > 20) return 0;
        else if(i2c_delay > 10) goto I2cStart;
    }
    /* 3.Write Data Address */
    I2C_SendData(I2C1, DataAddress);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if(i2c_delay > 20) return 0;
        else if(i2c_delay > 10) goto I2cStart;
    }
    
    /* 4.Start*/
    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if(i2c_delay > 20) return 0;
        else if(i2c_delay > 10) goto I2cStart;
    }
    /* 5.Write Slave Address */
    I2C_Send7bitAddress(I2C1, slave_addr, I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if(i2c_delay > 20) return 0;
        else if(i2c_delay > 10) goto I2cStart;
    }

    /* 6.conturn Write data */
    while(NumByteToRead)
    {
        if(NumByteToRead==1)
        {
            I2C_AcknowledgeConfig(I2C1, DISABLE);//6.noack
            I2C_GenerateSTOP(I2C1, ENABLE);//STOP
        }

        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))  /* EV7 */
        {
            if(i2c_delay > 20) return 0;
            else if(i2c_delay > 10) goto I2cStart;
        }
        *pBuffer++ = I2C_ReceiveData(I2C1);
        NumByteToRead--;
    }

    I2C_AcknowledgeConfig(I2C1, ENABLE);
    return 1;
}

uint8_t RTC_BufferWrite(uint8_t slave_addr, uint8_t* pBuffer, uint8_t NumByteToWrite, uint8_t DataAddress)
{
    i2c_delay = 0;
    I2cStart:
    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if(i2c_delay > 20) return 0;
        else if(i2c_delay > 10) goto I2cStart;
    }

    I2C_Send7bitAddress(I2C1, slave_addr, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if(i2c_delay > 20) return 0;
        else if(i2c_delay > 10) goto I2cStart;
    }

    /* 3.Write Data Address */
    I2C_SendData(I2C1, DataAddress);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if(i2c_delay > 20) return 0;
        else if(i2c_delay > 10) goto I2cStart;
    }
    
    while(NumByteToWrite--)
    {
        I2C_SendData(I2C1, *pBuffer);
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        {
            if(i2c_delay > 20) return 0;
            else if(i2c_delay > 10) goto I2cStart;
        }
        pBuffer++;
    }

    I2C_GenerateSTOP(I2C1, ENABLE);
    while((I2C1->CR1 & TIM_CR1_CKD_1) == TIM_CR1_CKD_1)
    {
        if(i2c_delay > 20) return 0;
        else if(i2c_delay > 10) goto I2cStart;
    }
    return 0;
}

void RTC_WriteEnable(void)
{
    RTC_WriteReg(ISL1208_ADDR, SR_Reg,0x10);
}

void RTC_WriteDisable(void)
{
    RTC_WriteReg(ISL1208_ADDR, SR_Reg,0x00);
}

void RtcRead(RtcType* Time)
{
	RTC_BufferRead(ISL1208_ADDR, (uint8_t*)Time, 0x07, 0x00);
    if(Time->year == 0x00 && Time->month == 0x00)
    {
        Time->year = 0x20;
        Time->month = 0x01;
        Time->day = 0x01;
        Time->hour = 0x12;
        Time->minute = 0x00;
        Time->second = 0x00;
        Time->week = 0x03;
        RtcWrite(Time);
    }
    Time->hour &=0x7f;
    Time->second = BCD_to_DEC(Time->second);
}

void RtcWrite(RtcType* Time)
{
    Time->hour |= 0x80;
	RTC_WriteEnable();
    RTC_BufferWrite(ISL1208_ADDR, (uint8_t*)Time,0x07,0x00);
	RTC_WriteDisable();
}

void RtcIrqConfig(void)
{
    RTC_WriteReg(ISL1208_ADDR, SecAlarm, 0x80);
    RTC_WriteReg(ISL1208_ADDR, MinAlarm, 0x00);
    RTC_WriteReg(ISL1208_ADDR, HourAlarm, 0x00);
    RTC_WriteReg(ISL1208_ADDR, DayAlarm, 0x00);
    RTC_WriteReg(ISL1208_ADDR, MonAlarm, 0x00);
    RTC_WriteReg(ISL1208_ADDR, WeekAlarm, 0x00);
    
    RTC_WriteReg(ISL1208_ADDR, INT_Reg, 0xC0);
}

void RtcFreqConfig(void)
{
    RTC_WriteReg(ISL1208_ADDR, INT_Reg, 0x0A);
}

void RtcInit(void)
{
    bsp_InitI2C();
    //RtcIrqConfig();
    RtcFreqConfig();
}

/********************************************************/
/*
   Convierta la fecha del código BCD del chip del reloj al formato de fecha definido por NTCIP
*/
void RtcDateConvert(DateType* Date, RtcType* RtcTime)
{
    uint16_t month;
    uint32_t date;
    
    Date->Day = 0x01 << (RtcTime->week + 1);
    date = 0x00000001 << BCD_to_DEC(RtcTime->day);
    month = 0x0001 << BCD_to_DEC(RtcTime->month);
    
    Date->Date[3] = (date>>24);
    Date->Date[2] = (date>>16);
    Date->Date[1] = (date>>8);
    Date->Date[0] = (date);
    
    Date->MonthH = (month>>8);
    Date->MonthL = (month);
}

void RtcTimeConvert(TimeType* Time, RtcType* RtcTime)
{
    Time->Hour = BCD_to_DEC(RtcTime->hour);
    Time->Minute = BCD_to_DEC(RtcTime->minute);
}

void RtcReadTime(void)
{
    RtcRead(&Rtc);                  //Leer hora RTC
    RtcTimeConvert(&Time, &Rtc);    // conversión de formato de hora
    RtcDateConvert(&Date, &Rtc);    //conversión de formato de fecha
}


//                             0   1   2   3   4   5   6   7   8   9  10  11  12
const uint8_t DayMonth[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// Entrada: año
//Salida: ¿Es el año un año bisiesto? 1 es 0 no lo es
uint8_t Is_Leap_Year(uint16_t year)
{
	if(year%4==0)//debe ser divisible por 4
	{
		if(year%100==0)
		{
			if(year%400==0)return 1;//Si termina en 00, debe ser divisible por 400	   
			else return 0;
		}else return 1;
	}else return 0;
}

void utc_to_local(RtcType* local, RtcType* utc, uint8_t* TimeZone)
{
    uint8_t days;
    uint32_t timezone, seconds, local_seconds;
    
    timezone = (TimeZone[2]<<8)|TimeZone[3];
    seconds = utc->hour * 3600 + utc->minute * 60 + utc->second;
    if(utc->month != 2) days = DayMonth[utc->month];
    else 
    {
        if(Is_Leap_Year(2000+utc->year)) days = 29; else days = 28;
    }
    
    local->year = utc->year;
    local->month = utc->month;
    local->day = utc->day;
    
    if(TimeZone[0])//Distrito Este
    {
        if(seconds + timezone < 86400)
        {
            local_seconds = seconds + timezone;
        }
        else 
        {
            local_seconds = seconds + timezone - 86400;
            if(local->day < days) local->day++;
            else 
            {
                local->day = 1;
                if(local->month < 12) local->month++;
                else 
                {
                    local->month = 1;
                    local->year++;
                }
            }
        }
    }
    else //West End
    {
        if(seconds >= timezone)
        {
            local_seconds = seconds - timezone;
        }
        else 
        {
            local_seconds = seconds + 86400 - timezone;
            if(local->day > 1) local->day--;
            else 
            {
                if(local->month > 1) 
                {
                    local->month--;
                    if(local->month != 2) local->day = DayMonth[local->month];
                    else 
                    {
                        if(Is_Leap_Year(2000+local->year)) local->day = 29; else local->day = 28;
                    }
                }
                else 
                {
                    local->year--;
                    local->month = 12;
                    local->day = 31;
                }
            }
        }
    }
    
    calc_week(local);
    local->year = DEC_to_BCD(local->year);
    local->month = DEC_to_BCD(local->month);
    local->day = DEC_to_BCD(local->day);
    
    second_to_rtc(local_seconds, local);
    OP.gps_seconds = local_seconds;
}

void second_to_rtc(uint32_t seconds, RtcType* rtc)
{
    uint8_t  temp;
    uint32_t seconds_temp = seconds;
    
    temp = seconds_temp/3600;
    rtc->hour = DEC_to_BCD(temp);
    
    temp = (seconds_temp%3600)/60;
    rtc->minute = DEC_to_BCD(temp);
    
    temp = seconds_temp%60;
    rtc->second = DEC_to_BCD(temp);
}

void calc_week(RtcType* rtc)
{
    uint16_t y,m,d,a,b,c;
    //calcular el dia de la semana
    y = rtc->year;
    m = rtc->month;
    d = rtc->day;
    if(m<3)
    {
        m = m+12;
        y = y-1;
    }
    a = y/4;
    b = (m+1)*13/5;
    c = y+a+b+d-1;
    c = c%7;
    rtc->week = c;
}



