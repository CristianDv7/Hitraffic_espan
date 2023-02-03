/*
*********************************************************************************************************
*
*	模块名称 : SPI接口文件管理芯片
*	文件名称 : bsp_spi_ch376t.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*********************************************************************************************************
*/

#ifndef _BSP_SPI_CH376T_H
#define _BSP_SPI_CH376T_H

#include <stdint.h>

#ifndef		TRUE
#define		TRUE	1
#define		FALSE	0
#endif
#ifndef		NULL
#define		NULL	0
#endif

typedef enum
{
	CheckConnect = 0,	/* 连接 */
	DiskMount = 1,		/* 挂载 */
    SetFileName = 2
}USB_FOX;

/* 结构体，成员变量必须是 volatile, 否则C编译器优化时可能有问题 */
typedef struct
{
    volatile uint8_t fox;       /* fox进程 */
	volatile uint8_t SendCmdFlag;      /* 是否检查状态 */
	volatile uint8_t Cmd;		/* usb操作命令 */
	volatile uint8_t Status;	/* 期望返回状态 */
    volatile uint8_t OperFlag;  /* 操作结果 */
	volatile uint32_t Count;	/* 计数器 */
    volatile uint32_t MaxCount; /* 最大计数 */
}USB_STATUS;

#define CMD_RET_SUCESS 0x51
#define	CH376_DAT_BLOCK_LEN	0x40			/* USB, ,  */
#define	CMD01_GET_IC_VER	0x01			
#define	CMD21_SET_BAUDRATE	0x02
#define	CMD00_ENTER_SLEEP	0x03			/* */
#define	CMD00_RESET_ALL		0x05			/*  */
#define	CMD11_CHECK_EXIST	0x06			/*  */
#define	CMD20_CHK_SUSPEND	0x0B			/* */
#define	CMD20_SET_SDO_INT	0x0B
#define	CMD14_GET_FILE_SIZE	0x0C
#define	CMD50_SET_FILE_SIZE	0x0D
#define	CMD11_SET_USB_MODE	0x15
#define	CMD01_GET_STATUS	0x22
#define	CMD00_UNLOCK_USB	0x23
#define	CMD01_RD_USB_DATA0	0x27			/**/
#define	CMD01_RD_USB_DATA	0x28			/* CMD01_RD_USB_DATA0 + CMD00_UNLOCK_USB */
#define	CMD10_WR_USB_DATA7	0x2B			/* */
#define	CMD10_WR_HOST_DATA	0x2C			/**/
#define	CMD01_WR_REQ_DATA	0x2D			/* */
#define	CMD20_WR_OFS_DATA	0x2E			/* */
#define	CMD10_SET_FILE_NAME	0x2F			/* */
#define	CMD0H_DISK_CONNECT	0x30			/* /SD:  */
#define	CMD0H_DISK_MOUNT	0x31			/* :  */
#define	CMD0H_FILE_OPEN		0x32			/* : (),() */
#define	CMD0H_FILE_ENUM_GO	0x33			/* : () */
#define	CMD0H_FILE_CREATE	0x34			/* : , */
#define	CMD0H_FILE_ERASE	0x35			/* : ,,, */
#define	CMD1H_FILE_CLOSE	0x36			/* : () */
#define	CMD1H_DIR_INFO_READ	0x37			/* :  */
#define	CMD0H_DIR_INFO_SAVE	0x38			/* :  */
#define	CMD4H_BYTE_LOCATE	0x39			/* :  */
#define	CMD2H_BYTE_READ		0x3A			/* :  */
#define	CMD0H_BYTE_RD_GO	0x3B			/* :  */
#define	CMD2H_BYTE_WRITE	0x3C			/* :  */
#define	CMD0H_BYTE_WR_GO	0x3D			/* :  */
#define	CMD0H_DISK_CAPACITY	0x3E			/* :  */
#define	CMD0H_DISK_QUERY	0x3F			/* :  */
#define	CMD0H_DIR_CREATE	0x40			/* : (), */
#define	CMD4H_SEC_LOCATE	0x4A			/* :  */
#define	CMD1H_SEC_READ		0x4B			/* /SD:  */
#define	CMD1H_SEC_WRITE		0x4C			/* /SD:  */
#define	CMD0H_DISK_BOC_CMD	0x50			/* /SD: USBBulkOnly */
#define	CMD5H_DISK_READ		0x54			/* /SD: USB */
#define	CMD0H_DISK_RD_GO	0x55			/* /SD: USB */
#define	CMD5H_DISK_WRITE	0x56			/* /SD: USB */
#define	CMD0H_DISK_WR_GO	0x57			/* /SD: USB */

/* ********************************************************************************************************************* */
/*  CH375CH372 */

#define	CMD10_SET_USB_SPEED	0x04			/* USB, CMD11_SET_USB_MODEUSB12Mbps */
/* :  */
/*           00H=12MbpsFullSpeed(), 01H=1.5Mbps(), 02H=1.5MbpsLowSpeed */

#define	CMD11_GET_DEV_RATE	0x0A			/* : USB */
/* : 07H */
/* :  */
/*           411.5MbpsUSB, 12MbpsUSB */

#define	CMD11_GET_TOGGLE	0x0A			/* OUT */
#define	CMD11_READ_VAR8		0x0A			/* 8 */
#define	CMD20_SET_RETRY		0x0B			/* : USB */
#define	CMD20_WRITE_VAR8	0x0B			/* 8 */
#define	CMD14_READ_VAR32	0x0C			/* 32 */
#define	CMD50_WRITE_VAR32	0x0D			/* 32 */
#define	CMD01_DELAY_100US	0x0F			/* 100uS() */
#define	CMD40_SET_USB_ID	0x12			/* : USBVIDPID */
#define	CMD10_SET_USB_ADDR	0x13			/* USB */
#define	CMD01_TEST_CONNECT	0x16			/* /SD: USB */
#define	CMD00_ABORT_NAK		0x17			/* : NAK */
#define	CMD10_SET_ENDP2		0x18			/* (): USB0 */
#define	CMD10_SET_ENDP3		0x19			/* (): USB0 */
#define	CMD10_SET_ENDP4		0x1A			/* (): USB1 */
#define	CMD10_SET_ENDP5		0x1B			/* (): USB1 */
#define	CMD10_SET_ENDP6		0x1C			/* USB2/ */
#define	CMD10_SET_ENDP7		0x1D			/* USB2/ */
#define	CMD00_DIRTY_BUFFER	0x25			/* :  */
#define	CMD10_WR_USB_DATA3	0x29			/* (): USB0 */
#define	CMD10_WR_USB_DATA5	0x2A			/* (): USB1 */
/* ********************************************************* */
/*  CH375CH372  */
#define	CMD1H_CLR_STALL		0x41			/* : - */
#define	CMD1H_SET_ADDRESS	0x45			/* : -USB */
#define	CMD1H_GET_DESCR		0x46			/* : - */
#define	CMD1H_SET_CONFIG	0x49			/* : -USB */
#define	CMD0H_AUTO_SETUP	0x4D			/* /SD: USB */
#define	CMD2H_ISSUE_TKN_X	0x4E			/* : , ,  CMD10_SET_ENDP6/CMD10_SET_ENDP7 + CMD1H_ISSUE_TOKEN */
#define	CMD1H_ISSUE_TOKEN	0x4F			/* : , , CMD2H_ISSUE_TKN_X */
#define	CMD0H_DISK_INIT		0x51			/* /SD: USB */
#define	CMD0H_DISK_RESET	0x52			/* /SD: -USB */
#define	CMD0H_DISK_SIZE		0x53			/* /SD: USB */
#define	CMD0H_DISK_INQUIRY	0x58			/* /SD: USB */
#define	CMD0H_DISK_READY	0x59			/* /SD: USB */
#define	CMD0H_DISK_R_SENSE	0x5A			/* /SD: USB */
#define	CMD0H_RD_DISK_SEC	0x5B			/* :  */
#define	CMD0H_WR_DISK_SEC	0x5C			/* :  */
#define	CMD0H_DISK_MAX_LUN	0x5D			/* : -USB */
/* ************************************************ */
#ifndef	_NO_CH375_COMPATIBLE_
#define	CMD_GET_IC_VER		CMD01_GET_IC_VER
#define	CMD_SET_BAUDRATE	CMD21_SET_BAUDRATE
#define	CMD_ENTER_SLEEP		CMD00_ENTER_SLEEP
#define	CMD_RESET_ALL			CMD00_RESET_ALL
#define	CMD_CHECK_EXIST		CMD11_CHECK_EXIST
#define	CMD_CHK_SUSPEND		CMD20_CHK_SUSPEND
#define	CMD_SET_SDO_INT		CMD20_SET_SDO_INT
#define	CMD_GET_FILE_SIZE	CMD14_GET_FILE_SIZE
#define	CMD_SET_FILE_SIZE	CMD50_SET_FILE_SIZE
#define	CMD_SET_USB_MODE	CMD11_SET_USB_MODE
#define	CMD_GET_STATUS		CMD01_GET_STATUS
#define	CMD_UNLOCK_USB		CMD00_UNLOCK_USB
#define	CMD_RD_USB_DATA0	CMD01_RD_USB_DATA0
#define	CMD_RD_USB_DATA		CMD01_RD_USB_DATA
#define	CMD_WR_USB_DATA7	CMD10_WR_USB_DATA7
#define	CMD_WR_HOST_DATA	CMD10_WR_HOST_DATA
#define	CMD_WR_REQ_DATA		CMD01_WR_REQ_DATA
#define	CMD_WR_OFS_DATA		CMD20_WR_OFS_DATA
#define	CMD_SET_FILE_NAME	CMD10_SET_FILE_NAME
#define	CMD_DISK_CONNECT	CMD0H_DISK_CONNECT
#define	CMD_DISK_MOUNT		CMD0H_DISK_MOUNT
#define	CMD_FILE_OPEN		CMD0H_FILE_OPEN
#define	CMD_FILE_ENUM_GO	CMD0H_FILE_ENUM_GO
#define	CMD_FILE_CREATE		CMD0H_FILE_CREATE
#define	CMD_FILE_ERASE		CMD0H_FILE_ERASE
#define	CMD_FILE_CLOSE		CMD1H_FILE_CLOSE
#define	CMD_DIR_INFO_READ	CMD1H_DIR_INFO_READ
#define	CMD_DIR_INFO_SAVE	CMD0H_DIR_INFO_SAVE
#define	CMD_BYTE_LOCATE		CMD4H_BYTE_LOCATE
#define	CMD_BYTE_READ		CMD2H_BYTE_READ
#define	CMD_BYTE_RD_GO		CMD0H_BYTE_RD_GO
#define	CMD_BYTE_WRITE		CMD2H_BYTE_WRITE
#define	CMD_BYTE_WR_GO		CMD0H_BYTE_WR_GO
#define	CMD_DISK_CAPACITY	CMD0H_DISK_CAPACITY
#define	CMD_DISK_QUERY		CMD0H_DISK_QUERY
#define	CMD_DIR_CREATE		CMD0H_DIR_CREATE
#define	CMD_SEC_LOCATE		CMD4H_SEC_LOCATE
#define	CMD_SEC_READ		CMD1H_SEC_READ
#define	CMD_SEC_WRITE		CMD1H_SEC_WRITE
#define	CMD_DISK_BOC_CMD	CMD0H_DISK_BOC_CMD
#define	CMD_DISK_READ		CMD5H_DISK_READ
#define	CMD_DISK_RD_GO		CMD0H_DISK_RD_GO
#define	CMD_DISK_WRITE		CMD5H_DISK_WRITE
#define	CMD_DISK_WR_GO		CMD0H_DISK_WR_GO
#define	CMD_SET_USB_SPEED	CMD10_SET_USB_SPEED
#define	CMD_GET_DEV_RATE	CMD11_GET_DEV_RATE
#define	CMD_GET_TOGGLE		CMD11_GET_TOGGLE
#define	CMD_READ_VAR8		CMD11_READ_VAR8
#define	CMD_SET_RETRY		CMD20_SET_RETRY
#define	CMD_WRITE_VAR8		CMD20_WRITE_VAR8
#define	CMD_READ_VAR32		CMD14_READ_VAR32
#define	CMD_WRITE_VAR32		CMD50_WRITE_VAR32
#define	CMD_DELAY_100US		CMD01_DELAY_100US
#define	CMD_SET_USB_ID		CMD40_SET_USB_ID
#define	CMD_SET_USB_ADDR	CMD10_SET_USB_ADDR
#define	CMD_TEST_CONNECT	CMD01_TEST_CONNECT
#define	CMD_ABORT_NAK		CMD00_ABORT_NAK
#define	CMD_SET_ENDP2		CMD10_SET_ENDP2
#define	CMD_SET_ENDP3		CMD10_SET_ENDP3
#define	CMD_SET_ENDP4		CMD10_SET_ENDP4
#define	CMD_SET_ENDP5		CMD10_SET_ENDP5
#define	CMD_SET_ENDP6		CMD10_SET_ENDP6
#define	CMD_SET_ENDP7		CMD10_SET_ENDP7
#define	CMD_DIRTY_BUFFER	CMD00_DIRTY_BUFFER
#define	CMD_WR_USB_DATA3	CMD10_WR_USB_DATA3
#define	CMD_WR_USB_DATA5	CMD10_WR_USB_DATA5
#define	CMD_CLR_STALL		CMD1H_CLR_STALL
#define	CMD_SET_ADDRESS		CMD1H_SET_ADDRESS
#define	CMD_GET_DESCR		CMD1H_GET_DESCR
#define	CMD_SET_CONFIG		CMD1H_SET_CONFIG
#define	CMD_AUTO_SETUP		CMD0H_AUTO_SETUP
#define	CMD_ISSUE_TKN_X		CMD2H_ISSUE_TKN_X
#define	CMD_ISSUE_TOKEN		CMD1H_ISSUE_TOKEN
#define	CMD_DISK_INIT		CMD0H_DISK_INIT
#define	CMD_DISK_RESET		CMD0H_DISK_RESET
#define	CMD_DISK_SIZE		CMD0H_DISK_SIZE
#define	CMD_DISK_INQUIRY	CMD0H_DISK_INQUIRY
#define	CMD_DISK_READY		CMD0H_DISK_READY
#define	CMD_DISK_R_SENSE	CMD0H_DISK_R_SENSE
#define	CMD_RD_DISK_SEC		CMD0H_RD_DISK_SEC
#define	CMD_WR_DISK_SEC		CMD0H_WR_DISK_SEC
#define	CMD_DISK_MAX_LUN	CMD0H_DISK_MAX_LUN
#endif

#ifndef	PARA_STATE_INTB
#define	PARA_STATE_INTB		0x80			/* 7: , */
#define	PARA_STATE_BUSY		0x10			/* 4: , */
#endif

#ifndef	SER_CMD_TIMEOUT
#define	SER_CMD_TIMEOUT		32				/* , mS, ,  */
#define	SER_SYNC_CODE1		0x57			/* 1 */
#define	SER_SYNC_CODE2		0xAB			/* 2 */
#endif

#ifndef	CMD_RET_SUCCESS
#define	CMD_RET_SUCCESS		0x51			/*  */
#define	CMD_RET_ABORT		0x5F			/*  */
#endif

/* ********************************************************************************************************************* */
#ifndef	USB_INT_EP0_SETUP

/* , CMD20_CHK_SUSPENDUSB, USB */
#define	USB_INT_USB_SUSPEND	0x05			/* USB */
#define	USB_INT_WAKE_UP		0x06			/*  */

/* 0XHUSB */
/*   : USB_INT_EP1_OUT, USB_INT_EP1_IN, USB_INT_EP2_OUT, USB_INT_EP2_IN */
/*   7-40000 */
/*   3-2, 00=OUT, 10=IN, 11=SETUP */
/*   1-0, 00=0, 01=1, 10=2, 11=USB */
#define	USB_INT_EP0_SETUP	0x0C			/* USB0SETUP */
#define	USB_INT_EP0_OUT		0x00			/* USB0OUT */
#define	USB_INT_EP0_IN		0x08			/* USB0IN */
#define	USB_INT_EP1_OUT		0x01			/* USB1OUT */
#define	USB_INT_EP1_IN		0x09			/* USB1IN */
#define	USB_INT_EP2_OUT		0x02			/* USB2OUT */
#define	USB_INT_EP2_IN		0x0A			/* USB2IN */
/* USB_INT_BUS_RESET	0x0000XX11B */		/* USB */
#define	USB_INT_BUS_RESET1	0x03			/* USB */
#define	USB_INT_BUS_RESET2	0x07			/* USB */
#define	USB_INT_BUS_RESET3	0x0B			/* USB */
#define	USB_INT_BUS_RESET4	0x0F			/* USB */
#endif

/* 1XHUSB */
#ifndef	USB_INT_SUCCESS
#define	USB_INT_SUCCESS		0x14			/* USB */
#define	USB_INT_CONNECT		0x15			/* USB */
#define	USB_INT_DISCONNECT	0x16			/* USB */
#define	USB_INT_BUF_OVER	0x17			/* USB */
#define	USB_INT_USB_READY	0x18			/* USB */
#define	USB_INT_DISK_READ	0x1D			/* USB */
#define	USB_INT_DISK_WRITE	0x1E			/* USB */
#define	USB_INT_DISK_ERR	0x1F			/* USB */
#endif

#ifndef	ERR_DISK_DISCON
#define	ERR_DISK_DISCON		0x82			/* , */
#define	ERR_LARGE_SECTOR	0x84			/* ,512 */
#define	ERR_TYPE_ERROR		0x92			/* ,FAT12/FAT16/BigDOS/FAT32, */
#define	ERR_BPB_ERROR		0xA1			/* ,,WINDOWS */
#define	ERR_DISK_FULL		0xB1			/* ,, */
#define	ERR_FDT_OVER		0xB2			/* (),,FAT12/FAT16512, */
#define	ERR_FILE_CLOSE		0xB4			/* ,, */
#define	ERR_OPEN_DIR		0x41			/* () */
#define	ERR_MISS_FILE		0x42			/* , */
#define	ERR_FOUND_NAME		0x43			/* ,() */
#define	ERR_MISS_DIR		0xB3			/* (), */
#define	ERR_LONG_BUF_OVER	0x48			/*  */
#define	ERR_LONG_NAME_ERR	0x49			/*  */
#define	ERR_NAME_EXIST		0x4A			/* , */
#endif

/*******************************************************/
/* VAR_DISK_STATUS */
#ifndef	DEF_DISK_UNKNOWN
#define	DEF_DISK_UNKNOWN	0x00			/*  */
#define	DEF_DISK_DISCONN	0x01			/*  */
#define	DEF_DISK_CONNECT	0x02			/*  */
#define	DEF_DISK_MOUNTED	0x03			/*  */
#define	DEF_DISK_READY		0x10			/*  */
#define	DEF_DISK_OPEN_ROOT	0x12			/*  */
#define	DEF_DISK_OPEN_DIR	0x13			/*  */
#define	DEF_DISK_OPEN_FILE	0x14			/*  */
#endif

/********************************************/
#ifndef	DEF_SECTOR_SIZE
#define	DEF_SECTOR_SIZE		512				/* USD */
#endif

#ifndef	DEF_WILDCARD_CHAR
#define	DEF_WILDCARD_CHAR	0x2A			/*  '*' */
#define	DEF_SEPAR_CHAR1		0x5C			/*  '\' */
#define	DEF_SEPAR_CHAR2		0x2F			/*  '/' */
#define	DEF_FILE_YEAR		2004			/* : 2004 */
#define	DEF_FILE_MONTH		1				/* : 1 */
#define	DEF_FILE_DATE		1				/* : 1 */
#endif

#ifndef	ATTR_DIRECTORY
/* FAT */
typedef struct _FAT_DIR_INFO
{
	uint8_t	    DIR_Name[11];					/* 00H,,11, */
	uint8_t	    DIR_Attr;						/* 0BH,, */
	uint8_t	    DIR_NTRes;						/* 0CH */
	uint8_t	    DIR_CrtTimeTenth;				/* 0DH,,0.1 */
	uint16_t	DIR_CrtTime;					/* 0EH, */
	uint16_t	DIR_CrtDate;					/* 10H, */
	uint16_t	DIR_LstAccDate;					/* 12H, */
	uint16_t	DIR_FstClusHI;					/* 14H */
	uint16_t	DIR_WrtTime;					/* 16H,,MAKE_FILE_TIME */
	uint16_t	DIR_WrtDate;					/* 18H,,MAKE_FILE_DATE */
	uint16_t	DIR_FstClusLO;					/* 1AH */
	uint32_t	DIR_FileSize;					/* 1CH, */
} FAT_DIR_INFO, *P_FAT_DIR_INFO;			    /* 20H */

/*  */
#define ATTR_READ_ONLY			0x01		/*  */
#define ATTR_HIDDEN				0x02		/*  */
#define ATTR_SYSTEM				0x04		/*  */
#define ATTR_VOLUME_ID			0x08		/*  */
#define ATTR_DIRECTORY			0x10		/*  */
#define ATTR_ARCHIVE			0x20		/*  */
#define ATTR_LONG_NAME			( ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID )	/*  */
#define ATTR_LONG_NAME_MASK		( ATTR_LONG_NAME | ATTR_DIRECTORY | ATTR_ARCHIVE )

/*  uint16_t */
/* Time = (Hour<<11) + (Minute<<5) + (Second>>1) */
#define MAKE_FILE_TIME( h, m, s )	( (h<<11) + (m<<5) + (s>>1) )	/*  */
/*  uint16_t */
/* Date = ((Year-1980)<<9) + (Month<<5) + Day */
#define MAKE_FILE_DATE( y, m, d )	( ((y-1980)<<9) + (m<<5) + d )	/*  */

#define LONE_NAME_MAX_CHAR		(255*2)		/* / */
#define LONG_NAME_PER_DIR		(13*2)		/* / */

#endif

/* *************************************** */
/* SCSI */

#ifndef	SPC_CMD_INQUIRY
#define SPC_CMD_INQUIRY			0x12
#define SPC_CMD_READ_CAPACITY	0x25
#define SPC_CMD_READ10			0x28
#define SPC_CMD_WRITE10			0x2A
#define SPC_CMD_TEST_READY		0x00
#define SPC_CMD_REQUEST_SENSE	0x03
#define SPC_CMD_MODESENSE6		0x1A
#define SPC_CMD_MODESENSE10		0x5A
#define SPC_CMD_START_STOP		0x1B

/* BulkOnly */
typedef struct _BULK_ONLY_CBW
{
	uint32_t	CBW_Sig;
	uint32_t	CBW_Tag;
	uint8_t	    CBW_DataLen0;					/* 08H,: ,048,033 */
	uint8_t	    CBW_DataLen1;
	uint16_t	CBW_DataLen2;
	uint8_t	    CBW_Flag;						/* 0CH,: ,71,0 */
	uint8_t	    CBW_LUN;
	uint8_t	    CBW_CB_Len;						/* 0EH,: ,116 */
	uint8_t	    CBW_CB_Buf[16];					/* 0FH,: ,16 */
} BULK_ONLY_CBW, *P_BULK_ONLY_CBW;			/* BulkOnly, CBW */

/* INQUIRY */
typedef struct _INQUIRY_DATA
{
	uint8_t	DeviceType;					/* 00H,  */
	uint8_t	RemovableMedia;			/* 01H, 71 */
	uint8_t	Versions;						/* 02H,  */
	uint8_t	DataFormatAndEtc;		/* 03H,  */
	uint8_t	AdditionalLength;		/* 04H,  */
	uint8_t	Reserved1;
	uint8_t	Reserved2;
	uint8_t	MiscFlag;						/* 07H,  */
	uint8_t	VendorIdStr[8];			/* 08H,  */
	uint8_t	ProductIdStr[16];		/* 10H,  */
	uint8_t	ProductRevStr[4];		/* 20H,  */
} INQUIRY_DATA, *P_INQUIRY_DATA;		/* 24H */

/* REQUEST SENSE */
typedef struct _SENSE_DATA
{
	uint8_t	ErrorCode;					/* 00H,  */
	uint8_t	SegmentNumber;
	uint8_t	SenseKeyAndEtc;				/* 02H,  */
	uint8_t	Information0;
	uint8_t	Information1;
	uint8_t	Information2;
	uint8_t	Information3;
	uint8_t	AdditSenseLen;				/* 07H,  */
	uint8_t	CmdSpecInfo[4];
	uint8_t	AdditSenseCode;				/* 0CH,  */
	uint8_t	AddSenCodeQual;				/* 0DH,  */
	uint8_t	FieldReplaUnit;
	uint8_t	SenseKeySpec[3];
} SENSE_DATA, *P_SENSE_DATA;			/* 12H */
#endif


#ifndef	MAX_FILE_NAME_LEN
#define MAX_FILE_NAME_LEN		(13+1)		/*  */

/*   */
typedef union _CH376_CMD_DATA
{
	struct {
		uint8_t	mBuffer[ MAX_FILE_NAME_LEN ];
	} Default;

	INQUIRY_DATA	DiskMountInq;		/* CMD0H_DISK_MOUNT: */

	FAT_DIR_INFO	OpenDirInfo;	/* CMD0H_FILE_OPEN: */

	FAT_DIR_INFO	EnumDirInfo;		/* CMD0H_FILE_ENUM_GO:  */

	struct {
		uint8_t	mUpdateFileSz;				/*  */
	} FileCLose;							/* CMD1H_FILE_CLOSE:   */

	struct {
		uint8_t	mDirInfoIndex;				/*   */
	} DirInfoRead;							/* CMD1H_DIR_INFO_READ:  */

	union {
		uint32_t	mByteOffset;				/*   */
		uint32_t	mSectorLba;					/*  */
	} ByteLocate;							/* CMD4H_BYTE_LOCATE:*/

	struct {
		uint16_t	mByteCount;					/*  */
	} ByteRead;								/* CMD2H_BYTE_READ: */

	struct {
		uint16_t	mByteCount;					/* */
	} ByteWrite;							/* CMD2H_BYTE_WRITE: */

	union {
		uint32_t	mSectorOffset;				/*   */
		uint32_t	mSectorLba;					/*  */
	} SectorLocate;							/* CMD4H_SEC_LOCATE:  */

	struct {
		uint8_t	mSectorCount;				/*  */
											/* */
		uint8_t	mReserved1;
		uint8_t	mReserved2;
		uint8_t	mReserved3;
		uint32_t	mStartSector;				/* */
	} SectorRead;							/* CMD1H_SEC_READ:   */

	struct {
		uint8_t	mSectorCount;				/*  */
											/*   */
		uint8_t	mReserved1;
		uint8_t	mReserved2;
		uint8_t	mReserved3;
		uint32_t	mStartSector;			/*   */
	} SectorWrite;							/* CMD1H_SEC_WRITE:  */

	struct {
		uint32_t	mDiskSizeSec;		    /*  */
	} DiskCapacity;							/* CMD0H_DISK_CAPACITY: */

	struct {
		uint32_t	mTotalSector;			/* */
		uint32_t	mFreeSector;			/* */
		uint8_t	mDiskFat;					/*  ,1-FAT12,2-FAT16,3-FAT32 */
	} DiskQuery;							/* CMD_DiskQuery,  */

	BULK_ONLY_CBW	DiskBocCbw;				/*  */
											/* CMD0H_DISK_BOC_CMD: */

	struct {
		uint8_t	mMaxLogicUnit;				/* */
	} DiskMaxLun;							/* CMD0H_DISK_MAX_LUN:  */

	INQUIRY_DATA	DiskInitInq;			/* */
											/* CMD0H_DISK_INIT: */

	INQUIRY_DATA	DiskInqData;			/* */
											/* CMD0H_DISK_INQUIRY: */

	SENSE_DATA		ReqSenseData;			/* : REQUEST SENSE  */
											/* CMD0H_DISK_R_SENSE:  */

	struct {
		uint32_t	mDiskSizeSec;				/* : (32,) */
	} DiskSize;								/* CMD0H_DISK_SIZE: USB */

	struct {
		uint32_t	mStartSector;				/* : LBA(32,) */
		uint8_t	mSectorCount;				/* :  */
	} DiskRead;								/* CMD5H_DISK_READ: USB() */

	struct {
		uint32_t	mStartSector;				/* : LBA(32,) */
		uint8_t	mSectorCount;				/* :  */
	} DiskWrite;							/* CMD5H_DISK_WRITE: USB() */
} CH376_CMD_DATA, *P_CH376_CMD_DATA;

#endif

#ifndef	VAR_FILE_SIZE
#define	VAR_SYS_BASE_INFO	0x20			/*  */
#define	VAR_RETRY_TIMES		0x25			/* USB */
#define	VAR_FILE_BIT_FLAG	0x26			/*  */
#define	VAR_DISK_STATUS		0x2B			/*  */
#define	VAR_SD_BIT_FLAG		0x30			/* SD */
#define	VAR_UDISK_TOGGLE	0x31			/* USBBULK-IN/BULK-OUT */
#define	VAR_UDISK_LUN		0x34			/* USB */
#define	VAR_SEC_PER_CLUS	0x38			/*  */
#define	VAR_FILE_DIR_INDEX	0x3B			/*  */
#define	VAR_CLUS_SEC_OFS	0x3C			/* ,0xFF, */
#define	VAR_DISK_ROOT		0x44			/* FAT16,FAT32(32,) */
#define	VAR_DSK_TOTAL_CLUS	0x48			/* (32,) */
#define	VAR_DSK_START_LBA	0x4C			/* LBA(32,) */
#define	VAR_DSK_DAT_START	0x50			/* LBA(32,) */
#define	VAR_LBA_BUFFER		0x54			/* LBA(32,) */
#define	VAR_LBA_CURRENT		0x58			/* LBA(32,) */
#define	VAR_FAT_DIR_LBA		0x5C			/* LBA(32,) */
#define	VAR_START_CLUSTER	0x60			/* ()(32,) */
#define	VAR_CURRENT_CLUST	0x64			/* (32,) */
#define	VAR_FILE_SIZE		0x68			/* (32,) */
#define	VAR_CURRENT_OFFSET	0x6C			/* ,(32,) */
#endif

#ifndef	DEF_USB_PID_SETUP
#define	DEF_USB_PID_NULL	0x00			/* PID,  */
#define	DEF_USB_PID_SOF		0x05
#define	DEF_USB_PID_SETUP	0x0D
#define	DEF_USB_PID_IN		0x09
#define	DEF_USB_PID_OUT		0x01
#define	DEF_USB_PID_ACK		0x02
#define	DEF_USB_PID_NAK		0x0A
#define	DEF_USB_PID_STALL	0x0E
#define	DEF_USB_PID_DATA0	0x03
#define	DEF_USB_PID_DATA1	0x0B
#define	DEF_USB_PID_PRE		0x0C
#endif

#ifndef	DEF_USB_REQ_TYPE
#define	DEF_USB_REQ_READ	0x80			/*  */
#define	DEF_USB_REQ_WRITE	0x00			/*  */
#define	DEF_USB_REQ_TYPE	0x60			/*  */
#define	DEF_USB_REQ_STAND	0x00			/*  */
#define	DEF_USB_REQ_CLASS	0x20			/*  */
#define	DEF_USB_REQ_VENDOR	0x40			/*  */
#define	DEF_USB_REQ_RESERVE	0x60			/*  */
#endif

#ifndef	DEF_USB_GET_DESCR
#define	DEF_USB_CLR_FEATURE	0x01
#define	DEF_USB_SET_FEATURE	0x03
#define	DEF_USB_GET_STATUS	0x00
#define	DEF_USB_SET_ADDRESS	0x05
#define	DEF_USB_GET_DESCR	0x06
#define	DEF_USB_SET_DESCR	0x07
#define	DEF_USB_GET_CONFIG	0x08
#define	DEF_USB_SET_CONFIG	0x09
#define	DEF_USB_GET_INTERF	0x0A
#define	DEF_USB_SET_INTERF	0x0B
#define	DEF_USB_SYNC_FRAME	0x0C
#endif

#define DEF_INT_TIMEOUT     500
#define ERR_USB_UNKNOWN		0xFA	/* 未知错误 */

#define	STRUCT_OFFSET(s, m)	((uint8_t)( &((s*)0)-> m))

extern	uint8_t	ExchgBuf[4];
extern USB_STATUS Usb_status;

void Ch376t_Init(void);
uint8_t Ch376s_CheckExist(void);
uint8_t Ch376s_SetUsbMode(uint8_t Mode);
void Ch376s_SetSdoInt(uint8_t Cmd);
void Ch376s_CheckUsbConnect(void);
uint8_t Ch376s_SendCmd(uint8_t Cmd);
uint8_t Ch376s_GetStatus(void);
void Ch376s_StatusProcess(void);
void Ch376s_SetFileName(uint8_t* name);
uint8_t	CH376ByteLocate(uint32_t offset);
uint8_t	CH376ByteWrite(uint8_t* buf, uint16_t ReqCount, uint16_t* RealCount);

void host(void);

#endif
