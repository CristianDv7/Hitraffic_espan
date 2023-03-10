#ifndef	_BSP_W5500_H_
#define	_BSP_W5500_H_

#include <stdint.h>

/***************** Common Register *****************/
#define MR		0x0000
	#define RST		0x80
	#define WOL		0x20
	#define PB		0x10
	#define PPP		0x08
	#define FARP	0x02

#define GAR		0x0001
#define SUBR	0x0005
#define SHAR	0x0009
#define SIPR	0x000f

#define INTLEVEL	0x0013
#define IR		0x0015
	#define CONFLICT	0x80
	#define UNREACH		0x40
	#define PPPOE		0x20
	#define MP			0x10

#define IMR		0x0016
	#define IM_IR7		0x80
	#define IM_IR6		0x40
	#define IM_IR5		0x20
	#define IM_IR4		0x10

#define SIR		0x0017
	#define S7_INT		0x80
	#define S6_INT		0x40
	#define S5_INT		0x20
	#define S4_INT		0x10
	#define S3_INT		0x08
	#define S2_INT		0x04
	#define S1_INT		0x02
	#define S0_INT		0x01
	#define Socket_Int(n)			(1<<n) 

#define SIMR	0x0018
	#define S7_IMR		0x80
	#define S6_IMR		0x40
	#define S5_IMR		0x20
	#define S4_IMR		0x10
	#define S3_IMR		0x08
	#define S2_IMR		0x04
	#define S1_IMR		0x02
	#define S0_IMR		0x01

#define REG_RTR		0x0019
#define RCR		0x001b

#define PTIMER	0x001c
#define PMAGIC	0x001d
#define PHA		0x001e
#define PSID	0x0024
#define PMRU	0x0026

#define UIPR	0x0028
#define UPORT	0x002c

#define PHYCFGR	0x002e
	#define RST_PHY		0x80
	#define OPMODE		0x40
	#define DPX			0x04
	#define SPD			0x02
	#define LINK		0x01

#define VERR	0x0039

/********************* Socket Register *******************/
#define Sn_MR		0x0000
	#define MULTI_MFEN		0x80
	#define BCASTB			0x40
	#define	ND_MC_MMB		0x20
	#define UCASTB_MIP6B	0x10
	#define MR_CLOSE		0x00
	#define MR_TCP		0x01
	#define MR_UDP		0x02
	#define MR_MACRAW		0x04

#define Sn_CR		0x0001
	#define OPEN		0x01
	#define LISTEN		0x02
	#define CONNECT		0x04
	#define DISCON		0x08
	#define CLOSE		0x10
	#define SEND		0x20
	#define SEND_MAC	0x21
	#define SEND_KEEP	0x22
	#define RECV		0x40

#define Sn_IR		0x0002
	#define IR_SEND_OK		0x10
	#define IR_TIMEOUT		0x08
	#define IR_RECV			0x04
	#define IR_DISCON		0x02
	#define IR_CON			0x01

#define Sn_SR		0x0003
	#define SOCK_CLOSED		0x00
	#define SOCK_INIT		0x13
	#define SOCK_LISTEN		0x14
	#define SOCK_ESTABLISHED	0x17
	#define SOCK_CLOSE_WAIT		0x1c
	#define SOCK_UDP		0x22
	#define SOCK_MACRAW		0x02

	#define SOCK_SYNSEND	0x15
	#define SOCK_SYNRECV	0x16
	#define SOCK_FIN_WAI	0x18
	#define SOCK_CLOSING	0x1a
	#define SOCK_TIME_WAIT	0x1b
	#define SOCK_LAST_ACK	0x1d

#define Sn_PORT		0x0004
#define Sn_DHAR	   	0x0006
#define Sn_DIPR		0x000c
#define Sn_DPORTR	0x0010

#define Sn_MSSR		0x0012
#define Sn_TOS		0x0015
#define Sn_TTL		0x0016

#define Sn_RXBUF_SIZE	0x001e
#define Sn_TXBUF_SIZE	0x001f
#define Sn_TX_FSR	0x0020
#define Sn_TX_RD	0x0022
#define Sn_TX_WR	0x0024
#define Sn_RX_RSR	0x0026
#define Sn_RX_RD	0x0028
#define Sn_RX_WR	0x002a

#define Sn_IMR		0x002c
	#define IMR_SENDOK	0x10
	#define IMR_TIMEOUT	0x08
	#define IMR_RECV	0x04
	#define IMR_DISCON	0x02
	#define IMR_CON		0x01

#define Sn_FRAG		0x002d
#define Sn_KPALVTR	0x002f

/*******************************************************************/
/************************ SPI Control Byte *************************/
/*******************************************************************/
/* Operation mode bits */
#define VDM		0x00
#define FDM1	0x01
#define	FDM2	0x02
#define FDM4	0x03

/* Read_Write control bit */
#define RWB_READ	0x00
#define RWB_WRITE	0x04

/* Block select bits */
#define COMMON_R	0x00

/* Socket 0 */
#define S0_REG		0x08
#define S0_TX_BUF	0x10
#define S0_RX_BUF	0x18

/* Socket 1 */
#define S1_REG		0x28
#define S1_TX_BUF	0x30
#define S1_RX_BUF	0x38

/* Socket 2 */
#define S2_REG		0x48
#define S2_TX_BUF	0x50
#define S2_RX_BUF	0x58

/* Socket 3 */
#define S3_REG		0x68
#define S3_TX_BUF	0x70
#define S3_RX_BUF	0x78

/* Socket 4 */
#define S4_REG		0x88
#define S4_TX_BUF	0x90
#define S4_RX_BUF	0x98

/* Socket 5 */
#define S5_REG		0xa8
#define S5_TX_BUF	0xb0
#define S5_RX_BUF	0xb8

/* Socket 6 */
#define S6_REG		0xc8
#define S6_TX_BUF	0xd0
#define S6_RX_BUF	0xd8

/* Socket 7 */
#define S7_REG		0xe8
#define S7_TX_BUF	0xf0
#define S7_RX_BUF	0xf8

#define S_RX_SIZE	2048	/* Definir el tama??o del b??fer de recepci??n del Socket, que se puede modificar seg??n la configuraci??n de W5500_RMSR */
#define S_TX_SIZE	2048    /* Definir el tama??o del b??fer de env??o de Socket, que se puede modificar de acuerdo con la configuraci??n de W5500_TMSR */

//El modo operativo del puerto, 0: modo servidor TCP, 1: modo cliente TCP, 2: modo UDP (difusi??n)
#define TCP_SERVER		0x00	//Modo servidor TCP
#define TCP_CLIENT		0x01	//Modo cliente TCP
#define UDP_MODE		0x02	//Modo UDP (emisi??n)

// Registro de estado del puerto 0, 1: el puerto se inicializa y el puerto 2 est?? conectado (los datos se pueden transmitir normalmente)
#define S_INIT			0x01	//Se inicializa el puerto
#define S_CONN			0x02	//El puerto completa la conexi??n y puede transmitir datos normalmente

//El estado de recepci??n y env??o de datos en el puerto 0, 1: el puerto ha recibido datos, 2: el puerto ha enviado datos completos
#define S_RECEIVE		0x01		// puerto recibi?? un paquete
#define S_TRANSMITOK	0x02		//El puerto env??a un paquete de datos para completar

typedef unsigned char SOCKET;		//Tipo de datos de n??mero de puerto personalizado

/***************----- Definici??n de estructura de datos -----***************/
typedef struct 
{
	unsigned char IP_Addr[4];	//Direcci??n IP local	
	unsigned char Gateway_IP[4];//Direcci??n IP de la puerta de enlace
	unsigned char Sub_Mask[4];	//m??scara de subred
	unsigned char Phy_Addr[6];	//Direcci??n f??sica (MAC)
}NET;

typedef struct 
{
    uint8_t     Num;
    uint8_t     Mode;           //El modo operativo del puerto, 0: modo servidor TCP, 1: modo cliente TCP, 2: modo UDP (difusi??n)
    uint8_t     State;          
    uint8_t     DataState;
	uint16_t    LocalPort;     // puerto local
	uint8_t     DestIP[4];      //Destino Direcci??n IP destino
	uint16_t    DestPort;       //n??mero de puerto de destino
    uint8_t     UdpDIPR[4];	    //Modo UDP (difusi??n), direcci??n IP del host de destino
	uint16_t    UdpDestPort;    //Modo UDP (difusi??n), n??mero de puerto del host de destino
}SOCKET_TYPE;

/***************----- Definici??n de variable de par??metro de red -----***************/
extern NET           Net;
extern SOCKET_TYPE   Socket[8];

/***************----- b??fer de datos del puerto -----***************/
extern unsigned char Rx_Buffer[2048];	//Bufer de datos de recepci??n del puerto
extern unsigned char Tx_Buffer[2048];	// b??fer de datos de env??o del puerto


extern unsigned char W5500_Interrupt;	//Bandera de interrupci??n W5500 (0: sin interrupci??n, 1: interrupci??n)


extern void Delay(unsigned int d);//Funci??n de retardo (ms)
extern void W5500_GPIO_Configuration(void);//Configuraci??n inicial de W5500 GPIO
extern void W5500_NVIC_Configuration(void);//Configuraci??n de prioridad de interrupci??n de pin de recepci??n W5500

extern void W5500_Hardware_Reset(void);//Restablecimiento de hardware W5500
extern void W5500_Init(void);//Inicializa la funci??n de registro W5500
extern unsigned char Detect_Gateway(SOCKET s);  // comprobar servidor de puerta de enlace
extern void Socket_Init(SOCKET s);              //Especificar la inicializaci??n del Socket (0~7)
extern unsigned char Socket_Connect(SOCKET s);  //Establecer el Socket especificado (0~7) como el cliente para conectarse con el servidor remoto
extern unsigned char Socket_Listen(SOCKET s);   //Establecer el Socket especificado (0~7) como el servidor para esperar la conexi??n del host remoto
extern unsigned char Socket_UDP(SOCKET s);     //Establecer el Socket especificado (0~7) en modo UDP
extern unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr);//Especificar Socket (0~7) para recibir procesamiento de datos
extern void Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size); //Especificar Socket (0~7) para enviar procesamiento de datos
extern void W5500_Interrupt_Process(void);// Marco del controlador de interrupciones W5500

extern void W5500_Socket_Set(SOCKET s);
extern void Load_Net_Parameters(void);
extern void W5500_Initialization(void);

#endif

