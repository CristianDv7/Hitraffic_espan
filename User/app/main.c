/*
*********************************************************************************************************
*
* Module name: main program entry 
* File name: main.c 
* Version : V1.0 
* illustrate : 
* Modification record: 
* Version Number Date Author Description 
* V1.0 2019-03-19 wcx first launch
*
*********************************************************************************************************
*/

#include "stm32f10x_flash.h"
#include "bsp.h"				/* Low-level hardware driver*/



/* release date */
#define RELEASE_DATE	"2022-03-20"
char info[] = "Sinowatcher";
char netaddr[] = "http://www.sinowatcher.cn \r\n";

uint8_t beep_count;


unsigned int W5500_Send_Delay_Counter[8] = {0}; //W5500 send delay count variable (ms)

void Process_Socket_Data(SOCKET s);
void SocketProcess(void);							//poll network port
void ScreenProcess(void);
static void PrintfLogo(void);

//	bsp_StartAutoTimer(0, 100);
//  bsp_StartTimer(5, 5000);
//	bsp_StartTimer(4, 500);
//	bsp_StartTimer(2, 1500);
//	bsp_StartAutoTimer(3, 100);

/*
*********************************************************************************************************
* Function name: main 
* Function description: c program entry
*********************************************************************************************************
Important work: 2. The file management driver must be done well

*/
const char buf1[] = "hola";
int main(void)
{	
	uint8_t  read[100] = {0} ;
	uint8_t leer;
#if RELEASE    // usa el # con el objetivo de compilar errores antes de ejecucion
    //open read protection
    if(FLASH_GetReadOutProtectionStatus() != SET) //
    {
        FLASH_Unlock();
        FLASH_ReadOutProtection(ENABLE);
        FLASH_Lock();
    }
#endif
    GotoRestart:									// definicion de goto
	bsp_Init();											/* Inicializacion de hardware */
	PrintfLogo();
	bsp_LedOn(LED_POWER);
		
    bsp_InitUart();  // Codigo iniciado dv7//
		
    OLED_ShowString(0,0,"SU ");
    OLED_ShowString(0,4,"00-00-00");
    OLED_ShowString(0,6,"00-01-01");
    
    StartProcess();					/*Escribe el codigo que se ha guardado por defecto en las variables del micro*/
    //host();               //Udisk 数据读取
    RunDataInit();          //Inicializa los valores de banderas
    ReadRealTime();					/*Comunicacion y lectura del tiempo del modulo reloj por I2C*/
    
    Load_Net_Parameters();		/*Carga los parametros de la configuracion de modulo ether, IP, red, gateway, etc*/
    W5500_Hardware_Reset();		/*Resetea el modulo ether*/
		W5500_Initialization();		/*Inicializa el modulo ether*/
    RunPhaseStateStartup();		/*Inicia la secuencia de fases*/
    bsp_InitIwdg(3000);
    
	while(1)
	{
		
	
		bsp_Idle();									/*No hace nada, funcion en blanco*/
				
        LampControlProcess();   //Estado de control de luces
        Input_mange();          //Proceso de estatus interno
        ScreenProcess();        //Estado de procesos del display
        
        if(reg1s_flag)          //information processing
        {
            reg1s_flag = 0;
            if(OP.RestartFlag == 1)
            {
                OP.RestartFlag = 0;
                goto GotoRestart;
            }
            IWDG_Feed();
        }
				
        SocketProcess();
				bsp_InitUart();
				
			
				
				UART_T uart_t;
				uint8_t _ucByte;
				
				UartSendBuf(&uart_t , &_ucByte, 1);
					
			 if (UartGetChar(&g_tUart5, &leer)!=0)
		 
			{
					BEEP_ON();
					bsp_DelayMS(500);
					BEEP_OFF();
					UartSendBuf(&g_tUart5, (uint8_t *)buf1, strlen(buf1));
					switch (leer)
					{
							case '1':
									UartSendBuf(&g_tUart5, (uint8_t *)buf1, strlen(buf1));
									break;
							 default:
									break;
						 }}
				
	}
}

void SYS_TEST(void)
{
	uint8_t cmd;
    
    if(UartGetChar(&g_tUart1, &cmd)) 
    {
        switch(cmd)
        {
            case '1':	
                printf("1 - cmd\n");
                printf("switch_state_stab = %04x\n", switch_state_stab);
                printf("current_stab      = %04x\n", current_stab);
                printf("red_state         = %04x\n", red_state);
                printf("green_state       = %04x\n", green_state);
                printf("red_install_reg   = %04x\n", OP.red_install_reg);
                break;
            
            case '2':
                printf("2 - cmd\r\n");
                break;
            
            case 'c':
                printf("red_failed_reg          = %04x\n", OP.red_failed_reg);
                printf("red_green_conflict_reg  = %04x\n", OP.red_green_conflict_reg);
                printf("green_conflict_reg      = %04x\n", OP.green_conflict_reg);
                break;
            
            case 't':
                printf("t - beep\r\n");
                beep_count = 100;
                break;
            default: break;
        }
    }
}


//Proceso de muestra del display
void ScreenProcess(void)
{
    if(reg100ms_flag) //100ms procesando informacion
    {
        reg100ms_flag = 0;
        if(beep_count > 0)
        {
            if(--beep_count==0) BEEP_OFF();
            else BEEP_ON();
        }
        #if RELEASE == 0
        SYS_TEST();
        #endif
    }
    
    if(OP.ShowTimeFlag)
    {
        static uint8_t t = 0;
        char string[32] = {0};

        OP.ShowTimeFlag = 0; 
        sprintf(string, "%02x:%02x:%02d", Rtc.hour,Rtc.minute,Rtc.second);
        OLED_ShowString(0, 4, string);
        sprintf(string, "%02x/%02x/%02x", Rtc.year,Rtc.month,Rtc.day);
        OLED_ShowString(0, 6, string);
        if(++t > 5)
        {
            t = 0;
            sprintf(string, "P%02dSQ%02d M:", PatternNow.Num, SequenceNow.Num);
            
            switch (OP.WorkMode)
            {
                case StarupMode:    strcat(string,"Starup");     break;
            	  case FixedTime:     strcat(string,"Fixed ");     break;
            	  case LineCtrl:      strcat(string,"GWave ");     break;
                case VehicleSense:  strcat(string,"Sense ");     break;
                case Flashing:      strcat(string,"Flash ");     break;
                case AllRed:        strcat(string,"AllRed");     break;
                case LampOff:       strcat(string,"OFF   ");     break;
                case ManualFlashing:
                    if(OP.WorkMode_Reason == WMR_LocalManual)
                        strcat(string,"LoFlh ");     
                    else if(OP.WorkMode_Reason == WMR_RemoteManual)
                        strcat(string,"ReFlh "); 
                    else if(OP.WorkMode_Reason == WMR_RemoteGuard)
                        strcat(string,"GuFlh "); 
                    break;
                case ManualAllRead: 
                    if(OP.WorkMode_Reason == WMR_LocalManual)
                        strcat(string,"LoRed ");     
                    else if(OP.WorkMode_Reason == WMR_RemoteManual)
                        strcat(string,"ReRed "); 
                    else if(OP.WorkMode_Reason == WMR_RemoteGuard)
                        strcat(string,"GuRed ");    
                    break;
                case ManualLampOff:
                    if(OP.WorkMode_Reason == WMR_LocalManual)
                        strcat(string,"LoOff ");     
                    else if(OP.WorkMode_Reason == WMR_RemoteManual)
                        strcat(string,"ReOff "); 
                    else if(OP.WorkMode_Reason == WMR_RemoteGuard)
                        strcat(string,"GuOff ");
                    break;
                case ManualStep:
                case ManualAppoint:
                    if(OP.WorkMode_Reason == WMR_LocalManual)
                        strcat(string,"LoMan ");     
                    else if(OP.WorkMode_Reason == WMR_RemoteManual)
                        strcat(string,"ReMan "); 
                    else if(OP.WorkMode_Reason == WMR_RemoteGuard)
                        strcat(string,"GuMan ");
                    break;
            	default:strcat(string,"Other ");    break;
            }
            
            OLED_ShowString(0, 0, string);
            sprintf(string, "%d.%d.%d.%d", Net.IP_Addr[0], Net.IP_Addr[1], Net.IP_Addr[2], Net.IP_Addr[3]);
            OLED_ShowString(0, 2, string);
        }
    }
}

void SocketProcess(void)//轮询网络端口
{
    SOCKET n;
    //W5500端口初始化配置
    W5500_Socket_Set(0);
    //W5500_Socket_Set(1);
    //W5500_Socket_Set(2);
    //W5500中断处理程序框架
    W5500_Interrupt_Process();
    
    for(n=0;n<8;n++)
    {
        if((Socket[n].DataState & S_RECEIVE) == S_RECEIVE)//如果Socket接收到数据
        {
            Socket[n].DataState &= ~S_RECEIVE;
            Process_Socket_Data(n);
        }
    }
    
    if(OP.ConnectFlag == 1)
    {
        if(OP.SendWorkModeFlag == 1)
        {
            OP.SendWorkModeFlag = 0;
            if(Socket[0].State == (S_INIT|S_CONN))
            {
                Socket[0].DataState &= ~S_TRANSMITOK;
                
                Socket[0].UdpDIPR[0] = Rx_Buffer[0];
                Socket[0].UdpDIPR[1] = Rx_Buffer[1];
                Socket[0].UdpDIPR[2] = Rx_Buffer[2];
                Socket[0].UdpDIPR[3] = Rx_Buffer[3];
                Socket[0].UdpDestPort = Rx_Buffer[4]<<8 | Rx_Buffer[5];
                SignalStateAutoReport(&send);
                if(send.n==1)
                    Write_SOCK_Data_Buffer(0, send.pdata0, send.length0);
                else if(send.n==2){
                    Write_SOCK_Data_Buffer(0, send.pdata0, send.length0);
                    Write_SOCK_Data_Buffer(0, send.pdata1, send.length1);
                }else if(send.n==3){
                    Write_SOCK_Data_Buffer(0, send.pdata0, send.length0);
                    Write_SOCK_Data_Buffer(0, send.pdata1, send.length1);
                    Write_SOCK_Data_Buffer(0, send.pdata2, send.length2);
                }else if(send.n==4){
                    Write_SOCK_Data_Buffer(0, send.pdata0, send.length0);
                    Write_SOCK_Data_Buffer(0, send.pdata1, send.length1);
                    Write_SOCK_Data_Buffer(0, send.pdata2, send.length2);
                    Write_SOCK_Data_Buffer(0, send.pdata3, send.length3);
                }
                send.n = 0;
            }
        }
    }
    if(OP.SendDoorAlarm == 1)
    {
        OP.SendDoorAlarm = 0;
        Socket[0].UdpDIPR[0] = BasicInfo.IPv4Remote.RemoteIP[0];
        Socket[0].UdpDIPR[1] = BasicInfo.IPv4Remote.RemoteIP[1];
        Socket[0].UdpDIPR[2] = BasicInfo.IPv4Remote.RemoteIP[2];
        Socket[0].UdpDIPR[3] = BasicInfo.IPv4Remote.RemoteIP[3];
        Socket[0].UdpDestPort = BasicInfo.IPv4Remote.RemoteSocket[0]<<8 | BasicInfo.IPv4Remote.RemoteSocket[1];//协议正确,才设定目标IP和端口

        if(Socket[0].State == (S_INIT|S_CONN))
        {
            Socket[0].DataState &= ~S_TRANSMITOK;
            DoorAlarmReport(&send);
            if(send.n==1)
                Write_SOCK_Data_Buffer(0, send.pdata0, send.length0);
            else if(send.n==2){
                Write_SOCK_Data_Buffer(0, send.pdata0, send.length0);
                Write_SOCK_Data_Buffer(0, send.pdata1, send.length1);
            }else if(send.n==3){
                Write_SOCK_Data_Buffer(0, send.pdata0, send.length0);
                Write_SOCK_Data_Buffer(0, send.pdata1, send.length1);
                Write_SOCK_Data_Buffer(0, send.pdata2, send.length2);
            }else if(send.n==4){
                Write_SOCK_Data_Buffer(0, send.pdata0, send.length0);
                Write_SOCK_Data_Buffer(0, send.pdata1, send.length1);
                Write_SOCK_Data_Buffer(0, send.pdata2, send.length2);
                Write_SOCK_Data_Buffer(0, send.pdata3, send.length3);
            }
            send.n = 0;
        }
    }
}

/*******************************************************************************
* 函数名  : Process_Socket_Data
* 描述    : W5500接收并发送接收到的数据
* 输入    : s:端口号
* 输出    : 无
* 返回值  : 无
* 说明    : 本过程先调用S_rx_process()从W5500的端口接收数据缓冲区读取数据,
*			然后将读取的数据从Rx_Buffer拷贝到Temp_Buffer缓冲区进行处理。
*			处理完毕，将数据从Temp_Buffer拷贝到Tx_Buffer缓冲区。调用S_tx_process()
*			发送数据。
*******************************************************************************/
void Process_Socket_Data(SOCKET s)
{
	unsigned short size;
	size = Read_SOCK_Data_Buffer(s, Rx_Buffer);

	if(s == 0)//UDP
	{
        //memcpy(Tx_Buffer, Rx_Buffer+8, size-8);
        if(gb25280_Process(Rx_Buffer+8,size-8,&send)==Frame_right)//协议正确，返回数据
        {
            OP.ConnectFlag = 1;
            Socket[s].UdpDIPR[0] = Rx_Buffer[0];
            Socket[s].UdpDIPR[1] = Rx_Buffer[1];
            Socket[s].UdpDIPR[2] = Rx_Buffer[2];
            Socket[s].UdpDIPR[3] = Rx_Buffer[3];
            Socket[s].UdpDestPort = Rx_Buffer[4]<<8 | Rx_Buffer[5];//协议正确,才设定目标IP和端口
            if(send.n==1)
                Write_SOCK_Data_Buffer(s, send.pdata0, send.length0);
            else if(send.n==2){
                Write_SOCK_Data_Buffer(s, send.pdata0, send.length0);
                Write_SOCK_Data_Buffer(s, send.pdata1, send.length1);
            }else if(send.n==3){
                Write_SOCK_Data_Buffer(s, send.pdata0, send.length0);
                Write_SOCK_Data_Buffer(s, send.pdata1, send.length1);
                Write_SOCK_Data_Buffer(s, send.pdata2, send.length2);
            }else if(send.n==4){
                Write_SOCK_Data_Buffer(s, send.pdata0, send.length0);
                Write_SOCK_Data_Buffer(s, send.pdata1, send.length1);
                Write_SOCK_Data_Buffer(s, send.pdata2, send.length2);
                Write_SOCK_Data_Buffer(s, send.pdata3, send.length3);
            }
            bsp_LedToggle(LED_COM);
        }
	}
//	else if(s==1)//TCP_SERVER
//	{
//		memcpy(Tx_Buffer, Rx_Buffer, size);
//		if(strncmp((char*)Rx_Buffer, "BEEP_ON", 7))beep_count = 4;
//		else if(strncmp((char*)Rx_Buffer, "BEEP_OFF", 8))beep_count = 0;
//        Write_SOCK_Data_Buffer(s, Tx_Buffer, size);
//	}
//	else if(s==2)//TCP_CLIENT
//	{
//		memcpy(Tx_Buffer, Rx_Buffer, size);
//		if(strncmp((char*)Rx_Buffer, "BEEP_ON", 7))beep_count = 8;
//		else if(strncmp((char*)Rx_Buffer, "BEEP_OFF", 8))beep_count = 0;
//        Write_SOCK_Data_Buffer(s, Tx_Buffer, size);
//	}
}

static void PrintfLogo(void)
{
    #if DEBUG
	printf("*************************************************************\r\n");
	printf("* 发布日期   : %s\r\n", RELEASE_DATE);	/* 打印例程日期 */
	printf("* 固件库版本 : %d.%d.%d\r\n", __STM32F10X_STDPERIPH_VERSION_MAIN,
			__STM32F10X_STDPERIPH_VERSION_SUB1,__STM32F10X_STDPERIPH_VERSION_SUB2);
	printf("* CMSIS版本  : %X.%02X\r\n", __CM3_CMSIS_VERSION_MAIN, __CM3_CMSIS_VERSION_SUB);
	printf("* 赛诺杰交通信号控制机 \r\n");
	printf("* Email : 540917841@qq.com \r\n");
	printf("* Copyright www.sinowatcher.cn \r\n");
	printf("*************************************************************\r\n");    
    #endif
}
