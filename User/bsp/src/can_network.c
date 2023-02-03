/*
*********************************************************************************************************
*	                                  
*	模块名称 : CAN网络程序。
*	文件名称 : can_network.c
*	版    本 : V1.0
*
*********************************************************************************************************
*/
#include "bsp.h"


/* 定义全局变量 */
CanTxMsg CanTxMsgStruct;			/* 用于发送 */
CanRxMsg CanRxMsgStruct;			/* 用于接收 */


/*
*********************************************************************************************************
*	函 数 名: SendCanMsg
*	功能说明: 发送一包数据
*	形    参：无
*	返 回 值: 无
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
*	函 数 名: can_Init
*	功能说明: 配置CAN硬件
*	形    参：无
*	返 回 值: 无
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
    
	/* PA11，PA12口线设置为AFIO模式, 切换到CAN功能 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	/* 使能GPIO时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	/* 配置CAN信号接收引脚: RX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	/* GPIO配置为上拉输入模式 */
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* 配置CAN信号发送引脚: TX */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		/* 配置为复用推挽输出 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/* 设置GPIO最大速度 */
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* 原生的CAN引脚和USB引脚是相同的口线，引脚的重映射功能将CAN引脚切换到PB8，PB9 */
	GPIO_PinRemapConfig(GPIO_Remap1_CAN1 , ENABLE);	/* 使能CAN1的重映射 */
	
	/* 使能CAN外设时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	
	CAN_DeInit(CAN1);											/* 复位CAN寄存器 */
	CAN_StructInit(&CAN_InitStructure);		/* 填充CAN参数结构体成员为缺省值 */
	
	/*
		TTCM = time triggered communication mode
		ABOM = automatic bus-off management 
		AWUM = automatic wake-up mode
		NART = no automatic retransmission
		RFLM = receive FIFO locked mode 
		TXFP = transmit FIFO priority		
	*/
	CAN_InitStructure.CAN_TTCM = DISABLE;			/* 禁止时间触发模式（不生成时间戳), T  */
	CAN_InitStructure.CAN_ABOM = DISABLE;			/* 禁止自动总线关闭管理 */
	CAN_InitStructure.CAN_AWUM = DISABLE;			/* 禁止自动唤醒模式 */
	CAN_InitStructure.CAN_NART = DISABLE;			/* 禁止仲裁丢失或出错后的自动重传功能 */
	CAN_InitStructure.CAN_RFLM = DISABLE;			/* 禁止接收FIFO加锁模式 */
	CAN_InitStructure.CAN_TXFP = DISABLE;			/* 禁止传输FIFO优先级 */
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;	/* 设置CAN为正常工作模式 */
	
	/* 
		CAN 波特率 = RCC_APB1Periph_CAN / Prescaler / (SJW + BS1 + BS2);
		
		SJW = synchronisation_jump_width 
		BS = bit_segment
		
		本例中，设置CAN波特率为500Kbps		
		CAN 波特率 = 360000000 / 6 / (1 + 6 + 5) / = 500kHz		
	*/
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_8tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_7tq;
	CAN_InitStructure.CAN_Prescaler = 9;
	CAN_Init(CAN1, &CAN_InitStructure);
	
	/* 设置CAN滤波器0 */
	CAN_FilterInitStructure.CAN_FilterNumber = 0;		/* 滤波器序号，0-13，共14个滤波器 */
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;		/* 滤波器模式，设置ID掩码模式 */
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;	/* 32位滤波 */
    //标准帧
	CAN_FilterInitStructure.CAN_FilterIdHigh = std_id<<5;               /* 掩码后ID的高16bit */
	CAN_FilterInitStructure.CAN_FilterIdLow = 0 | CAN_ID_STD;			/* 掩码后ID的低16bit */
    //拓展帧
	//CAN_FilterInitStructure.CAN_FilterIdHigh = ((ext_id<<3)>>16)&0xffff;/* 掩码后ID的高16bit */
	//CAN_FilterInitStructure.CAN_FilterIdLow = (ext_id<<3) | CAN_ID_EXT; /* 掩码后ID的低16bit */
    
    //屏蔽寄存器的设置
	//这里的思路是先将标准CAN ID和扩展CAN ID对应的ID值先异或后取反，为什么？
    //异或是为了找出两个CAN ID有哪些位是相同的，是相同的位则说明需要关心，
    //需要关心的位对应的屏蔽码位应该设置为1，因此需要取反一下。最后再整体左移3位。
    
    //这里为什么左移18位？因为从ISO11898中可以看出，
    //标准CAN ID占ID18~ID28，为了与CAN_FilterIdHigh对齐，应左移2位，
    //接着为了与扩展CAN对应，还应该再左移16位，因此，总共应左移2+16＝18位。
    //也可以用另一个方式来理解：直接看Mapping的内容，发现STDID相对EXID[0]偏移了18位,因此左移18位.
    
	mask =(std_id<<18);
	mask ^=ext_id;//将对齐后的标准CAN与扩展CAN异或后取反
	mask =~mask;
	mask <<=3;//再整体左移3位
	mask |=0x02; //只接收数据帧，不接收远程帧
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=(mask>>16)&0xffff; //设置屏蔽寄存器高字节
	CAN_FilterInitStructure.CAN_FilterMaskIdLow=mask&0xffff;   //设置屏蔽寄存器低字节
    
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;		/* 滤波器绑定FIFO 0 */
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;				/* 使能滤波器 */
	CAN_FilterInit(&CAN_FilterInitStructure);
    /* 填充发送参数， 也可以再每次发送的时候填 */
	CanTxMsgStruct.StdId = 0x29;
	CanTxMsgStruct.ExtId = 0x00;
	CanTxMsgStruct.RTR = CAN_RTR_DATA;	//数据帧-远程帧
	CanTxMsgStruct.IDE = CAN_ID_STD;	//CAN_ID_EXD;		//标准帧-拓展帧
}

/*
*********************************************************************************************************
*	函 数 名: can_NVIC_Config
*	功能说明: 配置CAN中断
*	形    参：无
*	返 回 值: 无
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
	
	/* CAN FIFO0 消息接收中断使能 */ 
	CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
}

/*
*********************************************************************************************************
*	函 数 名: can_ISR
*	功能说明: CAN中断服务程序. 这个函数在 stm32f10x_it.c中被调用
*	形    参：无
*	返 回 值: 无
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
            
            if(Manual_start == CanRxMsgStruct.Data[0])//开始手动
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
