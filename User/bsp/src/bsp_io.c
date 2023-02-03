#include "bsp.h"


//信号输出口定义 PC0-LAMP_LAT   PC1-LAMP_CLK    PC2-LAMP_DAT 
#define LAMP_PORT       GPIOC
#define LAMP_LAT_PIN    GPIO_Pin_0
#define LAMP_CLK_PIN    GPIO_Pin_1
#define LAMP_DAT_PIN    GPIO_Pin_2

#define LAMP_LAT_Clr()	LAMP_PORT->BRR = LAMP_LAT_PIN
#define LAMP_LAT_Set()	LAMP_PORT->BSRR = LAMP_LAT_PIN
#define LAMP_CLK_Clr()	LAMP_PORT->BRR = LAMP_CLK_PIN
#define LAMP_CLK_Set()	LAMP_PORT->BSRR = LAMP_CLK_PIN
#define LAMP_DAT_Clr()	LAMP_PORT->BRR = LAMP_DAT_PIN
#define LAMP_DAT_Set()	LAMP_PORT->BSRR = LAMP_DAT_PIN


//信号状态检测输入口定义
//DET_LOAD PC14 
#define DET_LOAD_PIN    GPIO_Pin_14
#define DET_LOAD_PORT   GPIOC
#define DET_LOAD_Clr()	DET_LOAD_PORT->BRR = DET_LOAD_PIN
#define DET_LOAD_Set()	DET_LOAD_PORT->BSRR = DET_LOAD_PIN
//DET_CLK PC15
#define DET_CLK_PIN     GPIO_Pin_15
#define DET_CLK_PORT    GPIOC
#define DET_CLK_Clr()	DET_CLK_PORT->BRR = DET_CLK_PIN
#define DET_CLK_Set()	DET_CLK_PORT->BSRR = DET_CLK_PIN
//DET_IN PE6 
#define DET_IN_PIN		GPIO_Pin_6
#define DET_IN_PORT	    GPIOE
#define DET_IN          (DET_IN_PORT->IDR & DET_IN_PIN)

//手控板信号输入
#define KEY_IN_PIN		GPIO_Pin_5
#define KEY_IN_PORT	    GPIOE
#define KEY_IN          (GPIOE->IDR & GPIO_Pin_5)
//KEY_LOAD PD7 
#define KEY_LOAD_PIN    GPIO_Pin_7
#define KEY_LOAD_PORT   GPIOD
#define KEY_LOAD_Clr()	GPIOD->BRR = GPIO_Pin_7
#define KEY_LOAD_Set()	GPIOD->BSRR = GPIO_Pin_7
//KEY_CLK PD6
#define KEY_CLK_PIN     GPIO_Pin_6
#define KEY_CLK_PORT    GPIOD
#define KEY_CLK_Clr()	GPIOD->BRR = GPIO_Pin_6
#define KEY_CLK_Set()	GPIOD->BSRR = GPIO_Pin_6

Input8_Type peddet_hw;
uint8_t rf_state = 0;

union DRIVE_REG LampDriveReg[8];



static uint8_t PEDDET_STATE(void);
static void peddet_init(Input8_Type* Input, uint8_t (*SATET)(void));




void bsp_InitIO(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 打开GPIO时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG, ENABLE);

	/*  PC输出
        PC0-LAMP_LAT            PC1-LAMP_CLK        PC2-LAMP_DAT 
        PC3-WIFI_SYS_RESET      PC4-Ch376t_CS 
        PC6-LED8                PC7-LED7 
        PC8-LED6                PC9-LED5 
        PC14-DET_LOAD           PC15-DET_CLK
	*/
    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       /* IO口最大速度 */
	GPIO_InitStructure.GPIO_Pin = WIFI_WDT_PIN | RTC_IRQ_PIN; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    //信号输出口定义   PC0-LAMP_LAT   PC1-LAMP_CLK    PC2-LAMP_DAT  PC3-WIFI_RST PC13-PPS
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       /* IO口最大速度 */
	GPIO_InitStructure.GPIO_Pin = LAMP_LAT_PIN | LAMP_CLK_PIN | LAMP_DAT_PIN | DET_LOAD_PIN | DET_CLK_PIN | WIFI_RST_PIN | PPS_PIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    DET_CLK_Clr();
    PPS_OFF();
    
    //DET_IN信号检测(PE6), KEY_IN手控板输入(PE5)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  /* IO口最大速度 */
	GPIO_InitStructure.GPIO_Pin = DET_IN_PIN | KEY_IN_PIN | 0X001F;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    /* 引脚初始化配置(PD8-15 in0 - in8) */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin  = 0xff00;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    /* BEEP 引脚初始化配置(PD1) */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin  = BEEP_PIN | KEY_LOAD_PIN | KEY_CLK_PIN;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	BEEP_OFF();
    KEY_CLK_Clr();

    //GPIOF
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  /* IO口最大速度 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;             //GPIOF输入
	GPIO_Init(GPIOF, &GPIO_InitStructure);
    //GPIOG
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	  /* IO口最大速度 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;             //GPIOG输入
	GPIO_Init(GPIOG, &GPIO_InitStructure);
    
    peddet_init(&peddet_hw, PEDDET_STATE);
}

void rf315m_scan(void)
{
    static uint8_t counter = 0;
    if(VTI)
	{
		if(counter < 0xff) counter++;
		if((rf_state == 0)&&(counter > 2))
		{
			rf_state = ATI;
            printf("rf_state = %d\r\n",rf_state);
		}
	}
	else
	{
        rf_state = 0;
		counter = 0;
	}
}

static uint8_t PEDDET_STATE(void)
{
    return ((GPIOD->IDR & 0xFF00)>>8);
}

static void peddet_init(Input8_Type* Input, uint8_t (*SATET)(void))
{
    Input->State = SATET;
    Input->rising = 0;
    Input->failing = 0;
    Input->stab = 0;
    Input->temp = 0;
    memset(&Input->counter, 0x00, 8);
}

void peddet_scan(Input8_Type* Input)//10ms
{
	uint8_t i;
	uint8_t temp_var1;
	uint8_t temp_var2;
	Input->temp = Input->State();

	temp_var1 = Input->temp ^ Input->stab;
	if(temp_var1 != 0)
	{
		temp_var2 = 0x01;
		for(i = 0; i < 8; i++)
		{
			if((temp_var1 & temp_var2)==temp_var2)
			{
				if((Input->stab & temp_var2)==temp_var2)//mei dao you
				{
					if(++Input->counter[i] >= 10)//信号持续100ms
					{
						Input->stab ^= temp_var2;
						Input->counter[i] = 0;	
                        Input->failing |= temp_var2;
                        PeddetStateTab.peddet[i].Valid = 0;
                        PeddetStateTab.peddet[i].Presence = 0;
                        
                        #if DEBUG
                        printf("failing = %02x",Input->failing);
                        #endif
					}
				}
				else//you dao mei
				{
					if(++Input->counter[i] >= 10)
					{
						Input->stab ^= temp_var2;
						Input->counter[i] = 0;
                        Input->rising |= temp_var2;
                        PeddetStateTab.peddet[i].Valid = 1;
                        PeddetStateTab.peddet[i].NoActivity = 0;
                        PeddetStateTab.peddet[i].ErraticCounts++;
                        
                        #if DEBUG 
                        printf("rising = %02x",Input->rising);
                        #endif 
					}
				}
			}
			else
			{
				Input->counter[i] = 0;	
			}
			temp_var2 <<= 1;
		}
	}
	else
	{
		memset(&Input->counter, 0x00, 8);
	}
}

void ChangeDM13H(void)
{
	unsigned char i,ss,k,n,temp;
	//four triggering clock pulses (CLK) with high level latch (LAT )
	//DM13H will change to command data input mode (CD mode) at falling edge of the latch pulse (LAT )
    
    DISABLE_INT();
    LAMP_LAT_Set();for(k=0;k<2;k++);
    LAMP_CLK_Clr();for(k=0;k<2;k++);
    LAMP_CLK_Set();for(k=0;k<2;k++);
    LAMP_CLK_Clr();for(k=0;k<2;k++);
    LAMP_CLK_Set();for(k=0;k<2;k++);
    LAMP_CLK_Clr();for(k=0;k<2;k++);
    LAMP_CLK_Set();for(k=0;k<2;k++);
    LAMP_CLK_Clr();for(k=0;k<2;k++);
    LAMP_CLK_Set();for(k=0;k<2;k++);
    LAMP_CLK_Clr();for(k=0;k<2;k++);
    LAMP_LAT_Clr();for(k=0;k<2;k++);

    for(n = 0; n < 8; n++)
    {
        ss = 0x40;
        temp = 0x40;
        for(i=0; i<=6;i++)
        {
            if(ss & temp)
            {
                LAMP_DAT_Set();
            }
            else
            {
                LAMP_DAT_Clr();
            }

            for(k=0;k<2;k++);
            LAMP_CLK_Set();
            for(k=0;k<6;k++);
            LAMP_CLK_Clr();
            temp >>= 1;
        }
        //SV
        LAMP_DAT_Set(); for(k=0;k<2;k++);
        LAMP_CLK_Set(); for(k=0;k<2;k++);
        LAMP_CLK_Clr(); for(k=0;k<2;k++);
        //NU
        LAMP_DAT_Clr(); for(k=0;k<2;k++);
        LAMP_CLK_Set(); for(k=0;k<2;k++);
        LAMP_CLK_Clr(); for(k=0;k<2;k++);
        //SLP
        LAMP_DAT_Clr(); for(k=0;k<2;k++);
        LAMP_CLK_Set(); for(k=0;k<2;k++);
        LAMP_CLK_Clr(); for(k=0;k<2;k++);
    }

	LAMP_LAT_Set(); for(k=0;k<2;k++);
	LAMP_LAT_Clr(); 
    ENABLE_INT();
}

void LampDriveDataInit(void)
{
	uint8_t n;
    for(n = 0; n < 8; n++)
    {
        LampDriveReg[n].value = 0;
        LampDriveReg[n].drivereg.power = 1;
        LampDriveReg[n].drivereg.run = 0;
        LampDriveReg[n].drivereg.error = 0;
        LampDriveReg[n].drivereg.special = 0;
    }
    ChangeDM13H();
}

void LampDriveOff(void)//10ms执行一次
{
	uint8_t i,n;
	uint16_t temp_var;

	LAMP_DAT_Clr();
	LAMP_LAT_Clr();
	LAMP_CLK_Clr();
    
	for(n = 0; n < 8; n++)
	{
        temp_var = LampDriveReg[7-n].value;
		for(i = 0; i < 4; i++)
		{
			if(temp_var & 0x8000)
				LAMP_DAT_Set();
			else
				LAMP_DAT_Clr();
			LAMP_CLK_Set();
			temp_var <<= 1;
			LAMP_CLK_Clr();
		}
        
        LAMP_DAT_Clr();
		for(i = 0; i < 12; i++)
		{
			LAMP_CLK_Set();
			LAMP_CLK_Clr();
		}
	}
	LAMP_LAT_Set();
	LAMP_LAT_Clr();
}

void LampDriveOut(void)//10ms执行一次
{
	uint8_t i,n;
	uint16_t temp_var;

	LAMP_DAT_Clr();
	LAMP_LAT_Clr();
	LAMP_CLK_Clr();
    
	for(n = 0; n < 8; n++)
	{
        temp_var = LampDriveReg[7-n].value;
		for(i = 0; i < 16; i++)
		{
			if(temp_var & 0x8000)
				LAMP_DAT_Set();
			else
				LAMP_DAT_Clr();
			LAMP_CLK_Set();
			temp_var <<= 1;
			LAMP_CLK_Clr();
		}
	}
	LAMP_LAT_Set();
	LAMP_LAT_Clr();
}

uint16_t switch_state_stab = 0;
uint16_t current_state_stab = 0;
uint16_t current_stab = 0;
uint16_t red_state_stab = 0;
uint16_t green_state_stab = 0;
uint16_t red_state = 0;
uint16_t green_state = 0;

void lamp_state_detect(void)
{
    static uint8_t switch_count[16];
    static uint8_t current_count[16];
    static uint8_t red_count[16];
    static uint8_t green_count[16];
    
    static uint16_t switch_state_temp = 0;
    static uint16_t current_state_temp = 0;
    static uint16_t red_state_temp = 0;
    static uint16_t green_state_temp = 0;
    
    uint16_t switch_temp = 0x0001;
    uint16_t current_temp = 0x0001;
    uint16_t voltage_temp = 0x0001;
    
    uint8_t  n, i;
    uint16_t temp16bit;
    
    DET_LOAD_Clr();for(n=0;n<2;n++);//数据装载进移位寄存器,管脚数据透明化 
    DET_LOAD_Set();for(n=0;n<2;n++);//可移位读取 load 72M_单指令周期13.88ns，50M——20ns，165至少16ns
    
    for(n=0;n<4;n++)
    {
        //switch---------------------
        for(i=0;i<4;i++)
        {
            if(DET_IN)
                switch_state_temp &= ~switch_temp;
            else 
                switch_state_temp |= switch_temp;
            DET_CLK_Set();
            switch_temp <<= 1;
            DET_CLK_Clr();
        }
        
        //                             2 1 4 3 
        //                             1 2 3 4 
        //current--------------------- 2 1 8 4 
        for(i=0;i<4;i++)
        {
            if(DET_IN)
                current_state_temp |= current_temp;
            else 
                current_state_temp &= ~current_temp;
            DET_CLK_Set();
            current_temp <<= 1;
            DET_CLK_Clr();
        }
        
        //voltage---------------------
        for(i=0;i<4;i++)
        {
            if(DET_IN)
                green_state_temp &= ~voltage_temp;
            else 
                green_state_temp |= voltage_temp;
            DET_CLK_Set();
            __NOP();
            DET_CLK_Clr();
            
            if(DET_IN)
                red_state_temp &= ~voltage_temp;
            else 
                red_state_temp |= voltage_temp;
            DET_CLK_Set();
            voltage_temp <<= 1;
            DET_CLK_Clr();
        }
    }
    
    //switch_count
    temp16bit = switch_state_temp ^ switch_state_stab;
    if(temp16bit != 0)
    {
        switch_temp = 0x8000;
        for(i=0;i<16;i++)
        {
            if((temp16bit & switch_temp)==switch_temp)
            {
                if((switch_state_stab & switch_temp)==switch_temp)//you dao mei
                {
                    if(++switch_count[i] >= 20)
                    {
                        switch_state_stab ^= switch_temp;
                        switch_count[i] = 0;
                    }
                }
                else//mei dao you
                {
                    if(++switch_count[i] >= 5)
                    {
                        switch_state_stab ^= switch_temp;
                        switch_count[i] = 0;
                    }
                }
            }
            else
            {
                switch_count[i] = 0;
            }
            switch_temp >>= 1;
        }
    }
    else
    {
        for(i=0;i<16;i++) switch_count[i] = 0;
    }
    
    //current_count
    temp16bit = current_state_temp ^ current_state_stab;
    if(temp16bit != 0)
    {
        current_temp = 0x8000;
        for(i=0;i<16;i++)
        {
            if((temp16bit & current_temp)==current_temp)//有改变
            {
                if((current_state_stab & current_temp)==current_temp)//you dao mei
                {
                    if(++current_count[i] >= 20)
                    {
                        current_state_stab ^= current_temp;
                        current_count[i] = 0;
                    }
                }
                else//mei dao you
                {
                    if(++current_count[i] >= 5)
                    {
                        current_state_stab ^= current_temp;
                        current_count[i] = 0;
                    }
                }
            }
            else//没有改变
            {
                current_count[i] = 0;
            }
            current_temp >>= 1;
        }
    }
    else
    {
        for(i=0;i<16;i++) current_count[i] = 0;
    }
    
    //red_count
    temp16bit = red_state_temp ^ red_state_stab;
    if(temp16bit != 0)
    {
        voltage_temp = 0x0001;
        for(i=0;i<16;i++)
        {
            if((temp16bit & voltage_temp)==voltage_temp)
            {
                if((red_state_stab & voltage_temp)==voltage_temp)//you dao mei
                {
                    if(++red_count[i] >= 21)
                    {
                        red_state_stab ^= voltage_temp;
                        red_count[i] = 0;
                    }
                }
                else//mei dao you
                {
                    if(++red_count[i] >= 2)
                    {
                        red_state_stab ^= voltage_temp;
                        red_count[i] = 0;
                    }
                }
            }
            else
            {
                red_count[i] = 0;
            }
            voltage_temp <<= 1;
        }
    }
    else
    {
        for(i=0;i<16;i++) red_count[i] = 0;
    }
    
    //green_count
    temp16bit = green_state_temp ^ green_state_stab;
    if(temp16bit != 0)
    {
        voltage_temp = 0x0001;
        for(i=0;i<16;i++)
        {
            if((temp16bit & voltage_temp)==voltage_temp)
            {
                if((green_state_stab & voltage_temp)==voltage_temp)//you dao mei
                {
                    if(++green_count[i] >= 21)
                    {
                        green_state_stab ^= voltage_temp;
                        green_count[i] = 0;
                    }
                }
                else//mei dao you
                {
                    if(++green_count[i] >= 2)
                    {
                        green_state_stab ^= voltage_temp;
                        green_count[i] = 0;
                    }
                }
            }
            else
            {
                green_count[i] = 0;
            }
            voltage_temp <<= 1;
        }
    }
    else
    {
        for(i=0;i<16;i++) green_count[i] = 0;
    }
}

uint8_t manual_scan(void)
{
	uint8_t n,i,temp_var;
	uint8_t temp = 0;
    
    KEY_LOAD_Clr();for(n=0;n<5;n++);//数据装载进移位寄存器,管脚数据透明化 
    KEY_LOAD_Set();for(n=0;n<5;n++);//可移位读取    load 72M_单指令周期13.88ns，50M——20ns，165至少16ns

    temp_var = 0x80;
    for(i=0;i<8;i++)
    {
        if(KEY_IN)
            temp |= temp_var;
        else 
            temp &= ~temp_var;
        KEY_CLK_Set();
        temp_var >>= 0x01;
        KEY_CLK_Clr();
    }
    return temp;
}

