/**********************************************************************************
 * 文件名  ：W5500.c
 * 描述    ：W5500 驱动函数库         
 * 库版本  ：ST_v3.5
**********************************************************************************/

#include "bsp.h"		
#include "bsp_w5500.h"	



/***************----- W5500 GPIO定义 -----***************/
#define W5500_SCS_PORT	GPIOA
#define W5500_SCS		GPIO_Pin_8	//Definir el pin CS de W5500		 
#define W5500_SCS_Clr()	W5500_SCS_PORT->BRR = W5500_SCS
#define W5500_SCS_Set()	W5500_SCS_PORT->BSRR = W5500_SCS


#define W5500_RST_PORT	GPIOC
#define W5500_RST		GPIO_Pin_8	//Definir el pin RST de W5500
#define W5500_RST_Clr()	W5500_RST_PORT->BRR = W5500_RST
#define W5500_RST_Set()	W5500_RST_PORT->BSRR = W5500_RST


#define W5500_INT		GPIO_Pin_9	//Definir el pin INT de W5500
#define W5500_INT_PORT	GPIOC


/***************----- Definición de variable de parámetro de red -----***************/
NET             Net;
SOCKET_TYPE     Socket[8];

/***************----- búfer de datos del puerto -----***************/
unsigned char Rx_Buffer[2048];	//Búfer de datos de recepción de puerto
unsigned char Tx_Buffer[2048];	//Búfer de datos de envío de puerto

unsigned char W5500_Interrupt;	//W5500中断标志(0:无中断,1:有中断)




/*******************************************************************************
* 函数名  : W5500_GPIO_Configuration
* 描述    : W5500 GPIO初始化配置
* 输入    : Ninguno
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : Ninguno
*******************************************************************************/
void W5500_GPIO_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	
	/* Configuración de inicialización de pines W5500_RST (PC8) */
	GPIO_InitStructure.GPIO_Pin  = W5500_RST;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(W5500_RST_PORT, &GPIO_InitStructure);
	W5500_RST_Clr();// Restablecer pin tirado bajo
	
	/* Configuración de inicialización de pin W5500_INT (PC9) */
	GPIO_InitStructure.GPIO_Pin  = W5500_INT;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(W5500_INT_PORT, &GPIO_InitStructure);
    
   /* Inicializar pin CS (PA8) */
	GPIO_InitStructure.GPIO_Pin = W5500_SCS;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(W5500_SCS_PORT, &GPIO_InitStructure);
    W5500_SCS_Set();
}

/*******************************************************************************
* 函数名  : Spi_Send_Short
* 描述    : SPI1 envía 2 bytes de datos (16 bits)
* 输入    : dat: datos de 16 bits a enviar
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : Ninguno
*******************************************************************************/
void Spi_Send_Short(unsigned short dat)
{
	Spi_SendByte(dat>>8);//escribir datos alto
	Spi_SendByte(dat);	//escribir datos bajos
}

/*******************************************************************************
* 函数名  : Write_W5500_1Byte
* 描述    : Escriba 1 byte de datos en el registro de dirección especificado a través de SPI
* 输入    : reg: dirección de registro de 16 bits, dat: datos a escribir
* 输出    : ninguno
* 返回值  : ninguno
* 说明    : ninguno
*******************************************************************************/
void Write_W5500_1Byte(unsigned short reg, unsigned char dat)
{
	W5500_SCS_Clr();

	Spi_Send_Short(reg);//Escribir dirección de registro de 16 bits a través de SPI
	Spi_SendByte(FDM1|RWB_WRITE|COMMON_R);//Escribir byte de control a través de SPI, longitud de datos de 1 byte, escribir datos, seleccionar registro general
	Spi_SendByte(dat);//escribir datos de 1 byte

	W5500_SCS_Set();
}

/*******************************************************************************
* 函数名  : Write_W5500_2Byte
* 描述    : Escriba 2 bytes de datos en el registro de dirección especificado a través de SPI
* 输入    : reg: dirección de registro de 16 bits, dat: datos de 16 bits a escribir (2 bytes)
* 输出    : ninguno
* 返回值  : ninguno
* 说明    : ninguno
*******************************************************************************/
void Write_W5500_2Byte(unsigned short reg, unsigned short dat)
{
	W5500_SCS_Clr();
		
	Spi_Send_Short(reg);//Escriba una dirección de registro de 16 bits a través de SPI
	Spi_SendByte(FDM2|RWB_WRITE|COMMON_R);//Escribir byte de control a través de SPI, longitud de datos de 2 bytes, escribir datos, seleccionar registro general
	Spi_Send_Short(dat);//写16位数据

	W5500_SCS_Set();
}

/*******************************************************************************
* 函数名  : Write_W5500_nByte
* 描述    : Escriba n bytes de datos en el registro de dirección especificado a través de SPI
* 输入    : reg: dirección de registro de 16 bits, *dat_ptr: puntero del búfer de datos que se escribirá, tamaño: longitud de los datos que se escribirán
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : Ninguno
*******************************************************************************/
void Write_W5500_nByte(unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short i;

	W5500_SCS_Clr();
		
	Spi_Send_Short(reg);//Escriba una dirección de registro de 16 bits a través de SPI
	Spi_SendByte(VDM|RWB_WRITE|COMMON_R);//Escribir byte de control a través de SPI, longitud de datos de N bytes, escribir datos, seleccionar registro general

	for(i=0;i<size;i++)//Loop escribe tamaño bytes de datos en el búfer a W5500
	{
		Spi_SendByte(*dat_ptr++);//escribir un byte de datos
	}

    W5500_SCS_Set();
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_1Byte
* 描述    : Escriba 1 byte de datos en el registro del puerto designado a través de SPI1
* 输入    : s: número de puerto, reg: dirección de registro de 16 bits, dat: datos a escribir
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : Ninguno
*******************************************************************************/
void Write_W5500_SOCK_1Byte(SOCKET s, unsigned short reg, unsigned char dat)
{
	W5500_SCS_Clr();
		
	Spi_Send_Short(reg);//Escriba una dirección de registro de 16 bits a través de SPI
	Spi_SendByte(FDM1|RWB_WRITE|(s*0x20+0x08));//Escribir byte de control a través de SPI, longitud de datos de 1 byte, escribir datos, seleccionar el registro del puerto
	Spi_SendByte(dat);//escribir datos de 1 byte

    W5500_SCS_Set();
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_2Byte
* 描述    : Escriba 2 bytes de datos en el registro del puerto designado a través de SPI
* 输入    : s: número de puerto, reg: dirección de registro de 16 bits, dat: datos de 16 bits que se escribirán (2 bytes)
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : Ninguno
*******************************************************************************/
void Write_W5500_SOCK_2Byte(SOCKET s, unsigned short reg, unsigned short dat)
{
	W5500_SCS_Clr();
			
	Spi_Send_Short(reg);//Escriba una dirección de registro de 16 bits a través de SPI
	Spi_SendByte(FDM2|RWB_WRITE|(s*0x20+0x08));//Escribir byte de control a través de SPI, longitud de datos de 2 bytes, escribir datos, seleccionar el registro del puerto
	Spi_Send_Short(dat);//Escribir datos de 16 bits

	W5500_SCS_Set();
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_4Byte
* 描述    : Escriba 4 bytes de datos en el registro del puerto designado a través de SPI
* 输入    : s: número de puerto, reg: dirección de registro de 16 bits, *dat_ptr: puntero de búfer de 4 bytes para escribir
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : Ninguno
*******************************************************************************/
void Write_W5500_SOCK_4Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
	W5500_SCS_Clr();//Configure el SCS de W5500 en un nivel bajo
    
	Spi_Send_Short(reg);//Escriba una dirección de registro de 16 bits a través de SPI
	Spi_SendByte(FDM4|RWB_WRITE|(s*0x20+0x08));//Escribir byte de control a través de SPI, longitud de datos de 4 bytes, escribir datos, seleccionar el registro del puerto

	Spi_SendByte(*dat_ptr++);//Escribir los datos del primer byte
	Spi_SendByte(*dat_ptr++);//Escribir los datos del segundo byte
	Spi_SendByte(*dat_ptr++);//Escribir los datos del tercer byte
	Spi_SendByte(*dat_ptr++);//Escribir los datos del cuarto byte

	W5500_SCS_Set();//Configure el SCS de W5500 en un nivel alto
}

/*******************************************************************************
* 函数名  : Read_W5500_1Byte
* 描述    : Leer datos de 1 byte del registro de dirección especificado W5500
* 输入    : reg: dirección de registro de 16 bits
* 输出    : 无
* 返回值  : 读取到寄存器的1个字节数据
* 说明    : 无
*******************************************************************************/
unsigned char Read_W5500_1Byte(unsigned short reg)
{
	unsigned char temp;

	W5500_SCS_Clr();//Configure el SCS de W5500 en un nivel bajo
			
	Spi_Send_Short(reg);//Escriba una dirección de registro de 16 bits a través de SPI
	Spi_SendByte(FDM1 | RWB_READ | COMMON_R);//Escribir byte de control a través de SPI, longitud de datos de 1 byte, leer datos, seleccionar registro general
	temp = Spi_SendByte(0x00);//Enviar datos ficticios, leer datos

	W5500_SCS_Set();//Configure el SCS de W5500 en un nivel alto
	return temp;//Devolver los datos del registro de lectura
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_1Byte
* 描述    : Leer datos de 1 byte del registro de puerto especificado W5500
* 输入    : s: número de puerto 0-7, reg: dirección de registro de 16 bits
* 输出    : Ninguno
* 返回值  : 1 byte de datos leídos en el registro
* 说明    : Ninguno
*******************************************************************************/
unsigned char Read_W5500_SOCK_1Byte(SOCKET s, unsigned short reg)
{
	unsigned char temp;

	W5500_SCS_Clr();//Configure el SCS de W5500 en un nivel bajo
			
	Spi_Send_Short(reg);//Escriba una dirección de registro de 16 bits a través de SPI
	Spi_SendByte(FDM1 | RWB_READ | (s*0x20+0x08));//Escribir byte de control a través de SPI, longitud de datos de 1 byte, leer datos, seleccionar registro de puerto s
	temp = Spi_SendByte(0x00);//enviar datos ficticios

	W5500_SCS_Set();//Configure el SCS de W5500 en un nivel alto
	return temp;//Devolver los datos del registro de lectura
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_2Byte
* 描述    : Leer datos de 2 bytes del registro de puerto especificado W5500
* 输入    : s: número de puerto, reg: dirección de registro de 16 bits
* 输出    : Ninguno
* 返回值  : 2 bytes de datos (16 bits) leídos en el registro
* 说明    : Ninguno
*******************************************************************************/
unsigned short Read_W5500_SOCK_2Byte(SOCKET s, unsigned short reg)
{
	unsigned short temp;

	W5500_SCS_Clr();//Configure el SCS de W5500 en un nivel bajo

	Spi_Send_Short(reg);//Escriba una dirección de registro de 16 bits a través de SPI
	Spi_SendByte(FDM2|RWB_READ|(s*0x20+0x08));//Escribir byte de control a través de SPI, longitud de datos de 2 bytes, leer datos, seleccionar registro de puerto s

	temp = Spi_SendByte(0x00);//enviar datos ficticios
	temp <<= 8;
	temp |= Spi_SendByte(0x00);//leer datos bajos

	W5500_SCS_Set();//Configure el SCS de W5500 en un nivel alto
	return temp;//Devolver los datos del registro de lectura
}

/*******************************************************************************
* 函数名  : Read_SOCK_Data_Buffer
* 描述    : Leer datos del búfer de datos de recepción W5500
* 输入    : s: número de puerto, *dat_ptr: puntero del búfer de almacenamiento de datos
* 输出    : Ninguno
* 返回值  : La longitud de los datos leídos, rx_size bytes
* 说明    : Ninguno
*******************************************************************************/
unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr)
{
	unsigned short rx_size;
	unsigned short offset, offset1;
	unsigned short i;

	rx_size = Read_W5500_SOCK_2Byte(s,Sn_RX_RSR);
	if(rx_size == 0) return 0;//Devolver si no se reciben datos
	if(rx_size > 1460) rx_size = 1460;

	offset = Read_W5500_SOCK_2Byte(s,Sn_RX_RD);
	offset1 = offset;
	offset &= (S_RX_SIZE-1);//Calcular la dirección física real

    W5500_SCS_Clr();//Configure el SCS de W5500 en un nivel bajo

	Spi_Send_Short(offset);//escribir dirección de 16 bits
	Spi_SendByte(VDM|RWB_READ|(s*0x20+0x18));//Escribir byte de control, longitud de datos de N bytes, leer datos, seleccionar el registro del puerto
	
	if((offset + rx_size) < S_RX_SIZE)//Si la dirección máxima no excede la dirección máxima del registro de búfer de recepción del W5500
	{
		for(i = 0; i < rx_size; i++)//Bucle leer rx_size bytes de datos
		{
			*dat_ptr = Spi_SendByte(0x00);//Enviar un dato no válido
			dat_ptr++;//La dirección del puntero del búfer de almacenamiento de datos se incrementa en 1
		}
	}
	else//Si la dirección máxima excede la dirección máxima del registro de búfer de recepción del W5500
	{
		offset = S_RX_SIZE - offset;
		for(i = 0; i < offset; i++)//Bucle para leer los primeros bytes compensados ​​de datos
		{
			*dat_ptr = Spi_SendByte(0x00);//Enviar un dato no válido
			dat_ptr++;//La dirección del puntero del búfer de almacenamiento de datos se incrementa en 1
		}
		W5500_SCS_Set(); //Configure el SCS de W5500 en un nivel alto

		W5500_SCS_Clr();//Configure el SCS de W5500 en un nivel bajo

		Spi_Send_Short(0x00);//escribir dirección de 16 bits
		Spi_SendByte(VDM|RWB_READ|(s*0x20+0x18));//Escribir byte de control, longitud de datos de N bytes, leer datos, seleccionar el registro del puerto

		for( ; i < rx_size; i++)//Rx_size-offset bytes de datos después de la lectura del bucle
		{
			*dat_ptr = Spi_SendByte(0x00);//Enviar un dato no válido
			dat_ptr++;//La dirección del puntero del búfer de almacenamiento de datos se incrementa en 1
		}
	}
	W5500_SCS_Set(); //Configure el SCS de W5500 en un nivel alto

	offset1 += rx_size;//Actualice la dirección física real, es decir, la dirección de inicio de los siguientes datos de lectura recibidos
	Write_W5500_SOCK_2Byte(s, Sn_RX_RD, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, RECV);//Enviar comando de inicio de recepción
	return rx_size;//Devuelve la longitud de los datos recibidos.
}

/*******************************************************************************
* 函数名  : Write_SOCK_Data_Buffer
* 描述    : Escribir datos en el búfer de envío de datos del W5500
* 输入    : s: número de puerto, *dat_ptr: puntero del búfer de almacenamiento de datos, tamaño: longitud de los datos que se escribirán
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : Ninguno
*******************************************************************************/
void Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short offset,offset1;
	unsigned short i;

		//Puede configurar la IP y el número de puerto del host de destino aquí
	if((Read_W5500_SOCK_1Byte(s, Sn_MR)&0x0f) != SOCK_UDP)// Si falla la apertura del socket
	{
		Write_W5500_SOCK_4Byte(s, Sn_DIPR, Socket[s].UdpDIPR);//Establecer la IP del host de destino 		
		Write_W5500_SOCK_2Byte(s, Sn_DPORTR, Socket[s].UdpDestPort);//Establecer el propósito del puerto de host			
	}

	offset  = Read_W5500_SOCK_2Byte(s,Sn_TX_WR);
	offset1 = offset;
	offset &= (S_TX_SIZE-1);//Calcular la dirección física real

	W5500_SCS_Clr();

	Spi_Send_Short(offset);//escribir dirección de 16 bits
	Spi_SendByte(VDM|RWB_WRITE|(s*0x20+0x10));//Escribir byte de control, longitud de datos de N bytes, escribir datos, seleccionar el registro del puerto s

	if((offset + size) < S_TX_SIZE)//Si la dirección máxima no supera la dirección máxima del registro de búfer de envío del W5500
	{
		for(i = 0; i < size; i++)//Escribir bytes de tamaño de datos en un bucle
		{
			Spi_SendByte(*dat_ptr++);//escribir un byte de datos		
		}
	}
	else//Si la dirección máxima excede la dirección máxima del registro de búfer de envío del W5500
	{
		offset = S_TX_SIZE - offset;
		for(i=0;i<offset;i++)//Loop escribe los primeros bytes compensados ​​de datos
		{
			Spi_SendByte(*dat_ptr++);//escribir un byte de datos
		}
		W5500_SCS_Set();

		W5500_SCS_Clr();

		Spi_Send_Short(0x00);//escribir dirección de 16 bits
		Spi_SendByte(VDM|RWB_WRITE|(s*0x20+0x10));//Escribir byte de control, longitud de datos de N bytes, escribir datos, seleccionar el registro del puerto s

		for(;i<size;i++)//Bytes de datos de desplazamiento de tamaño de escritura en bucle
		{
			Spi_SendByte(*dat_ptr++);//escribir un byte de datos
		}
	}
	W5500_SCS_Set();

	offset1+=size;//Actualice la dirección física real, es decir, la dirección de inicio de los datos que se enviarán la próxima vez que se escriban en el búfer de datos de envío
	Write_W5500_SOCK_2Byte(s, Sn_TX_WR, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, SEND);//enviar iniciar enviar comando
}

/*******************************************************************************
* 函数名  : W5500_Hardware_Reset
* 描述    : Restablecimiento de hardware W5500
* 输入    : Ninguno
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : El pin de reinicio de W5500 mantiene un nivel bajo durante al menos 500 us para reiniciar W5500
*******************************************************************************/
void W5500_Hardware_Reset(void)
{
	W5500_RST_Clr();//Restablecer pin bajado
	bsp_DelayMS(50);
	W5500_RST_Set();//复位引脚Restablecer pin tirado alto拉高
	bsp_DelayMS(200);
    
	//while((Read_W5500_1Byte(PHYCFGR) & LINK)==0);//Espere a que se complete la conexión ethernet
}

/*******************************************************************************
* 函数名  : W5500_Init
* 描述    : Inicializa la función de registro W5500
* 输入    : Ninguno
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : Antes de usar el W5500, primero inicialice el W5500
*******************************************************************************/
void W5500_Init(void)
{
	u8 i=0;
    
	Write_W5500_1Byte(MR, RST);//Reinicio de software W5500, establecido en 1 es válido, borrado automáticamente a 0 después del reinicio
	bsp_DelayMS(10);//Retraso 10ms, defina la función usted mismo
    
// Configure la dirección IP de la puerta de enlace (Gateway), Gateway_IP es una matriz de caracteres sin firmar de 4 bytes, defínala usted mismo 
// El uso de la puerta de enlace puede hacer que la comunicación supere las limitaciones de la subred y, a través de la puerta de enlace, puede acceder a otras subredes o ingresar a Internet
	Write_W5500_nByte(GAR, Net.Gateway_IP, 4);
    
	// Establezca el valor de la máscara de subred (MASK), SUB_MASK es una matriz de caracteres sin firmar de 4 bytes, defínala usted mismo
	//La máscara de subred se usa para operaciones de subred
	Write_W5500_nByte(SUBR,Net.Sub_Mask,4);		
	
	// Establezca la dirección física, PHY_ADDR es una matriz de caracteres sin firmar de 6 bytes, definida por usted mismo, que se utiliza para identificar de forma única el valor de la dirección física del dispositivo de red
	//El valor de la dirección debe aplicarse a IEEE. De acuerdo con las regulaciones de OUI, los primeros 3 bytes son el código del fabricante y los últimos 3 bytes son el número de serie del producto
	//Si define la dirección física usted mismo, tenga en cuenta que el primer byte debe ser un número par
	Write_W5500_nByte(SHAR,Net.Phy_Addr,6);		

	// Configure la dirección IP de esta máquina, IP_ADDR es una matriz de caracteres sin firmar de 4 bytes, defínala usted mismo
	// Tenga en cuenta que la IP de la puerta de enlace debe pertenecer a la misma subred que la IP local; de lo contrario, la máquina local no podrá encontrar la puerta de enlace
	Write_W5500_nByte(SIPR,Net.IP_Addr,4);		
	
	// Configure el tamaño del búfer de envío y el búfer de recepción, consulte el manual de datos W5500
	for(i=0;i<8;i++)
	{
		Write_W5500_SOCK_1Byte(i, Sn_RXBUF_SIZE, 0x02);//Socket Rx memory size=2k
		Write_W5500_SOCK_1Byte(i, Sn_TXBUF_SIZE, 0x02);//Socket Tx mempry size=2k
	}

	//Establezca el tiempo de reintento, el valor predeterminado es 2000 (200ms) 
	//El valor de cada unidad es 100 microsegundos y el valor inicial se establece en 2000 (0x07D0), que es igual a 200 milisegundos
	Write_W5500_2Byte(REG_RTR, 0x07d0);

	//Establecer el número de reintentos, el valor predeterminado es 8 veces 
	//Si el número de retransmisiones supera el valor establecido, se generará una interrupción de tiempo de espera (el bit de tiempo de espera Sn_IR (TIMEOUT) en el registro de interrupción del puerto correspondiente se establece en "1")
	Write_W5500_1Byte(RCR,8);
}

/*******************************************************************************
* 函数名  : Detect_Gateway
* 描述    : comprobar servidor de puerta de enlace
* 输入    : Ninguno
* 输出    : Ninguno
* 返回值  : Devuelve con éxito VERDADERO (0xFF), el error devuelve FALSO (0x00)
* 说明    : Ninguno
*******************************************************************************/
unsigned char Detect_Gateway(SOCKET s)
{
	unsigned char ip_adde[4];
	ip_adde[0] = Net.IP_Addr[0]+1;
	ip_adde[1] = Net.IP_Addr[1]+1;
	ip_adde[2] = Net.IP_Addr[2]+1;
	ip_adde[3] = Net.IP_Addr[3]+1;

	// Verifique la puerta de enlace y obtenga la dirección física de la puerta de enlace
	Write_W5500_SOCK_4Byte(s,Sn_DIPR,ip_adde);  //Escriba un valor de IP diferente de la IP local en el registro de dirección de destino
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);     //Establecer el socket en modo TCP
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);       //abrirSocket
	bsp_DelayMS(5);//Retraso 5ms	
	
	if(Read_W5500_SOCK_1Byte(s,Sn_SR) != SOCK_INIT)//Si el zócalo abre fallado
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//open falló, cierre el Socket
		return FALSE;//devuelve FALSO (0x00)
	}

	Write_W5500_SOCK_1Byte(s,Sn_CR,CONNECT);//Establezca el enchufe en modo de conexión						

	do
	{
		u8 j=0;
		j=Read_W5500_SOCK_1Byte(s,Sn_IR);//Leer registro de bandera de interrupción de Socket0
		if(j!=0)
		Write_W5500_SOCK_1Byte(s,Sn_IR,j);
		bsp_DelayMS(5);//Retraso 5ms
		if((j&IR_TIMEOUT) == IR_TIMEOUT)
		{
			return FALSE;	
		}
		else if(Read_W5500_SOCK_1Byte(s,Sn_DHAR) != 0xff)
		{
			Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//Cerrar el zócalo
			return TRUE;							
		}
	}while(1);
}

/*******************************************************************************
* 函数名  : Socket_Init
* 描述    : Especificar la inicialización de Socket (0~7)
* 输入    : s: el puerto a inicializar
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : Ninguno
*******************************************************************************/
void Socket_Init(SOCKET s)
{    
    //设置分片长度，参考W5500数据手册，该值可以不修改	
    Write_W5500_SOCK_2Byte(Socket[s].Num, Sn_MSSR, 0x05b4);//Número máximo de bytes fragmentados = 1460 (0x5b4)
    //Establecer el número de puerto del puerto 0
    Write_W5500_SOCK_2Byte(Socket[s].Num, Sn_PORT, Socket[s].LocalPort);
    
    if(Socket->Mode == TCP_SERVER)
    {
        //Establecer número de puerto de destino de puerto (remoto)
        Write_W5500_SOCK_2Byte(Socket[s].Num, Sn_DPORTR, Socket[s].DestPort);
        //Establecer la dirección IP de destino del puerto (remoto)
        Write_W5500_SOCK_4Byte(Socket[s].Num, Sn_DIPR, Socket[s].DestIP);	
    }
}

/*******************************************************************************
* 函数名  : Socket_Connect
* 描述    : Establezca el Socket especificado (0 ~ 7) como el cliente para conectarse con el servidor remoto
* 输入    : s: el puerto a configurar
* 输出    : Ninguno
* 返回值  : Devuelve con éxito VERDADERO (0xFF), el error devuelve FALSO (0x00)
* 说明    :Cuando el Socket local funciona en modo cliente, consulte este programa para establecer una conexión con el servidor remoto
*			Si hay una interrupción del tiempo de espera después de que se inicia la conexión, la conexión con el servidor falla y se debe volver a llamar a la conexión del programa
*			Cada vez que se llama al programa, se establece una conexión con el servidor.
*******************************************************************************/
unsigned char Socket_Connect(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);//Establecer el socket en modo TCP
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);//abrirSocket
	bsp_DelayMS(5);//Retraso 5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_INIT)//Si el zócalo abre fallado
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//No se pudo abrir, cierre el zócalo
		return FALSE;//devuelve FALSO (0x00)
	}
	Write_W5500_SOCK_1Byte(s,Sn_CR,CONNECT);//Establezca el enchufe en modo de conexión
	return TRUE;//Devolver VERDADERO, establecer con éxito
}

/*******************************************************************************
* 函数名  : Socket_Listen
* 描述    : Establezca el Socket especificado (0 ~ 7) como el servidor para esperar la conexión desde el host remoto
* 输入    : s:待s: el puerto a configurar设定的端口
* 输出    : Ninguno
* 返回值  : Devuelve con éxito VERDADERO (0xFF), el error devuelve FALSO (0x00)
* 说明    : Cuando el Socket local funciona en modo servidor, consulte el programa, etc. para la conexión del host remoto.
*			Este programa se llama solo una vez para configurar el W5500 en modo servidor
*******************************************************************************/
unsigned char Socket_Listen(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);//Establecer el socket en modo TCP
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);//abrirSocket
	bsp_DelayMS(5);//Retraso 5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_INIT)//Si el zócalo abre fallado
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//No se pudo abrir, cierre el zócalo
		return FALSE;//devuelve FALSO (0x00)
	}	
	Write_W5500_SOCK_1Byte(s,Sn_CR,LISTEN);//Establecer Socket en modo de escucha
	bsp_DelayMS(5);//延时5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_LISTEN)//Si la configuración del zócalo falla
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//La configuración no tiene éxito, cierre el Socket
		return FALSE;//devuelve FALSO (0x00)
	}

	return TRUE;

	//Hasta ahora, se ha completado la apertura del Socket y la configuración del trabajo de escucha. En cuanto a si el cliente remoto establece una conexión con él, debe esperar a que se interrumpa el Socket.
	//Para determinar si la conexión del Socket es exitosa. Consulte el estado de interrupción del zócalo en la hoja de datos del W5500
	//No es necesario configurar la IP de destino y el número de puerto de destino en el modo de escucha del servidor
}

/*******************************************************************************
* 函数名  : Socket_UDP
* 描述    : Establece el Socket especificado (0~7) en modo UDP
* 输入    :s: el puerto a configurar
* 输出    : Ninguno
* 返回值  : Devuelve con éxito VERDADERO (0xFF), el error devuelve FALSO (0x00)
* 说明    : Si el Socket funciona en modo UDP, consulte este programa En modo UDP, la comunicación del Socket no necesita establecer una conexión
*			El programa se llama solo una vez para configurar el W5500 en modo UDP
*******************************************************************************/
unsigned char Socket_UDP(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_UDP); //Establecer Socket en modo UDP*/
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);   // abrirSocket*/
	bsp_DelayMS(5);//延时5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_UDP)//Si el zócalo abre fallado
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//No se pudo abrir, cierre el zócalo
		return FALSE;//devuelve FALSO (0x00)
	}
	else
		return TRUE;

	//Hasta ahora, la apertura del Socket y la configuración del modo UDP se han completado.En este modo, no es necesario establecer una conexión con el host remoto
	//Debido a que Socket no necesita establecer una conexión, puede establecer la IP del host de destino y el número de puerto del Socket de destino antes de enviar datos
	//Si la IP del host de destino y el número de puerto del socket de destino son fijos y no han cambiado durante la operación, también puede configurarlos aquí
}

/*******************************************************************************
* 函数名  : W5500_Interrupt_Process
* 描述    : Estructura del controlador de interrupciones W5500
* 输入    : Ninguno
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : Ninguno
*******************************************************************************/
void W5500_Interrupt_Process(void)
{
	uint8_t SIR_REG, SnIR_REG, n;
IntDispose:
	SIR_REG = Read_W5500_1Byte(SIR);//Registro de bandera de interrupción de puerto de lectura
	if(SIR_REG == 0) return;
	for(n=0; n<8; n++)
	{
		if((SIR_REG & Socket_Int(n)) == Socket_Int(n))//Manejo de socket n case
		{
			SnIR_REG = Read_W5500_SOCK_1Byte(n,Sn_IR);//readSocket n interrumpe el logo del registro
			Write_W5500_SOCK_1Byte(n,Sn_IR, SnIR_REG);
			if(SnIR_REG & IR_CON)//En el modo TCP, el Socket n se conecta con éxito 
			{
				Socket[n].State |= S_CONN;//El estado de conexión de la red es 0x02, el puerto está conectado y puede transmitir datos normalmente
			}
			if(SnIR_REG & IR_DISCON)//Procesamiento de desconexión de socket en modo TCP
			{
				Write_W5500_SOCK_1Byte(n,Sn_CR,CLOSE);//Cierra el puerto y espera a que se vuelva a abrir la conexión 
                Socket_Init(n);		//Especifique la inicialización de Socket (0~7), inicialice el puerto 0
				Socket[n].State = 0;//Estado de conexión de red 0x00, error de conexión de puerto
			}
			if(SnIR_REG & IR_SEND_OK)//La transmisión de datos Socket0 se completó, puede iniciar la función S_tx_process () para enviar datos nuevamente
			{
				Socket[n].DataState |= S_TRANSMITOK;//El puerto envía un paquete para completar
			}
			if(SnIR_REG & IR_RECV)//Socket recibe datos y puede iniciar la función S_rx_process()
			{
				Socket[n].DataState |= S_RECEIVE;//El puerto recibe un paquete
			}
			if(SnIR_REG & IR_TIMEOUT)//Procesamiento de tiempo de espera de transmisión de datos o conexión de socket
			{
				Write_W5500_SOCK_1Byte(n,Sn_CR,CLOSE);// Cierra el puerto y espera a que se vuelva a abrir la conexión 			
				Socket[n].State = 0;//Estado de conexión de red 0x00, error de conexión de puerto
			}
		}
	}
	if(Read_W5500_1Byte(SIR) != 0) 
		goto IntDispose;
}

/*******************************************************************************
* 函数名  : W5500_Socket_Set
* 描述    : Configuración de inicialización del puerto W5500
* 输入    : Ninguno
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : Configure 4 puertos por separado y coloque el puerto en servidor TCP, cliente TCP o modo UDP según el modo de funcionamiento del puerto.
*			La condición de funcionamiento del puerto se puede juzgar a partir del byte de estado del puerto Socket.State
*******************************************************************************/
void W5500_Socket_Set(SOCKET s)
{
	if(Socket[s].State == 0)//configuración de inicialización del puerto 0
	{
		if(Socket[s].Mode == TCP_SERVER)//Modo de servidor TCP
		{
			if(Socket_Listen(s) == TRUE)
				Socket[s].State = S_INIT;
			else
				Socket[s].State = 0;
		}
		else if(Socket[s].Mode == TCP_CLIENT)//Modo de cliente TCP 
		{
			if(Socket_Connect(s)==TRUE)
				Socket[s].State = S_INIT;
			else
				Socket[s].State = 0;
		}
		else//Modo UDP
		{
			if(Socket_UDP(s) == TRUE)
				Socket[s].State = S_INIT | S_CONN;
			else
				Socket[s].State = 0;
		}
	}
}

/*******************************************************************************
* 函数名  : Load_Net_Parameters
* 描述    : cargar parámetros de red
* 输入    : Ninguno
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : Puerta de enlace, máscara, dirección física, dirección IP local, número de puerto, dirección IP de destino, número de puerto de destino, modo de funcionamiento del puerto
*******************************************************************************/
void Load_Net_Parameters(void)
{
	Net.IP_Addr[0] = BasicInfo.IPv4.IP[0];       //Cargue la dirección IP local
	Net.IP_Addr[1] = BasicInfo.IPv4.IP[1];
	Net.IP_Addr[2] = BasicInfo.IPv4.IP[2];
	Net.IP_Addr[3] = BasicInfo.IPv4.IP[3];
    
	Net.Gateway_IP[0] = BasicInfo.IPv4.GetWay[0];    //Cargar parámetros de puerta de enlace
	Net.Gateway_IP[1] = BasicInfo.IPv4.GetWay[1];
	Net.Gateway_IP[2] = BasicInfo.IPv4.GetWay[2];
	Net.Gateway_IP[3] = BasicInfo.IPv4.GetWay[3];
    
	Net.Sub_Mask[0] = BasicInfo.IPv4.SubMask[0];      //Cargando
	Net.Sub_Mask[1] = BasicInfo.IPv4.SubMask[1];
	Net.Sub_Mask[2] = BasicInfo.IPv4.SubMask[2];
	Net.Sub_Mask[3] = BasicInfo.IPv4.SubMask[3];
    
	Net.Phy_Addr[0] = BasicInfo.IPv4.MAC[0];     //cargar dirección física
	Net.Phy_Addr[1] = BasicInfo.IPv4.MAC[1];
	Net.Phy_Addr[2] = BasicInfo.IPv4.MAC[2];
	Net.Phy_Addr[3] = BasicInfo.IPv4.MAC[3];
	Net.Phy_Addr[4] = BasicInfo.IPv4.MAC[4];
	Net.Phy_Addr[5] = BasicInfo.IPv4.MAC[5]; 
    
    Socket[0].Num = 0;
    Socket[0].Mode = UDP_MODE;	   //Cargar el modo de trabajo del puerto 0, modo UDP
    Socket[0].LocalPort = (BasicInfo.IPv4.Socket[0] << 8) | BasicInfo.IPv4.Socket[1];     //Número de puerto 161 para el puerto local 0
    Socket[0].DestIP[0] = BasicInfo.IPv4Remote.RemoteIP[0];
    Socket[0].DestIP[1] = BasicInfo.IPv4Remote.RemoteIP[1];
    Socket[0].DestIP[2] = BasicInfo.IPv4Remote.RemoteIP[2];
    Socket[0].DestIP[3] = BasicInfo.IPv4Remote.RemoteIP[3];
    Socket[0].DestPort  =(BasicInfo.IPv4Remote.RemoteSocket[0] << 8) | BasicInfo.IPv4Remote.RemoteSocket[1];
    
    Socket[1].Num = 1;
    Socket[1].Mode = 3;//TCP_SERVER;             //TCP_SERVER
    Socket[1].LocalPort = 5001;     //Número de puerto 5001 del puerto local 1
    
    //El puerto local 2 está configurado en modo cliente, la dirección del servidor y el puerto deben configurarse
    Socket[2].Num = 2;
    Socket[2].Mode = 3;//TCP_CLIENT;             //TCP_CLIENT
    Socket[2].LocalPort = 5002;    //El número de puerto del puerto local 2 es 5002
    
    Socket[2].DestIP[0] = BasicInfo.IPv4Remote.RemoteIP[0];
    Socket[2].DestIP[1] = BasicInfo.IPv4Remote.RemoteIP[1];
    Socket[2].DestIP[2] = BasicInfo.IPv4Remote.RemoteIP[2];
    Socket[2].DestIP[3] = BasicInfo.IPv4Remote.RemoteIP[3];
    Socket[2].DestPort  =(BasicInfo.IPv4Remote.RemoteSocket[0] << 8) | BasicInfo.IPv4Remote.RemoteSocket[1];
    
    Socket[3].Mode = 3;
    Socket[4].Mode = 3;
    Socket[5].Mode = 3;
    Socket[6].Mode = 3;
    Socket[7].Mode = 3;

    #if DEBUG
    printf("IP = %d.%d.%d.%d\r\n",BasicInfo.IPv4.IP[0],BasicInfo.IPv4.IP[1],BasicInfo.IPv4.IP[2],BasicInfo.IPv4.IP[3]);
    printf("GetWay = %d.%d.%d.%d\r\n",BasicInfo.IPv4.GetWay[0],BasicInfo.IPv4.GetWay[1],BasicInfo.IPv4.GetWay[2],BasicInfo.IPv4.GetWay[3]);
    printf("SubMask = %d.%d.%d.%d\r\n",BasicInfo.IPv4.SubMask[0],BasicInfo.IPv4.SubMask[1],BasicInfo.IPv4.SubMask[2],BasicInfo.IPv4.SubMask[3]);
    printf("LocalPort = %d\r\n",Socket[0].LocalPort);
    
    printf("RemoteIP = %d.%d.%d.%d\r\n",BasicInfo.IPv4Remote.RemoteIP[0],BasicInfo.IPv4Remote.RemoteIP[1],BasicInfo.IPv4Remote.RemoteIP[2],BasicInfo.IPv4Remote.RemoteIP[3]);
    printf("DestPort = %d\r\n",Socket[2].DestPort);    
    #endif
}

/*******************************************************************************
* 函数名  : W5500_Initialization
* 描述    : Configuración de entrega inicial W5500
* 输入    : Ninguno
* 输出    : Ninguno
* 返回值  : Ninguno
* 说明    : Ninguno
*******************************************************************************/
void W5500_Initialization(void)
{
	W5500_Init();		//Inicializa la función de registro W5500
	Detect_Gateway(0);	// comprobar servidor de puerta de enlace 
	//Detect_Gateway(1);	// comprobar servidor de puerta de enlace
    //Detect_Gateway(2);	// comprobar servidor de puerta de enlace
    
	Socket_Init(0);		//Especifique la inicialización del Socket (0~7), inicialice el puerto 0
	//Socket_Init(1);
	//Socket_Init(2);
}


