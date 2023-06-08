/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Quectel Co., Ltd. 2013
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   example_transpass.c
 *
 * Project:
 * --------
 *   OpenCPU
 *
 * Description:
 * ------------
 *   This example gives an example for transpass setting.
 *   Through Uart port, input the special command, there will be given the response
 *   about transpass operation.
 *
 * Usage:
 * ------
 *   Compile & Run:
 *
 *     Set "C_PREDEF=-D __EXAMPLE_TRANSPASS__" in gcc_makefile file. And compile the 
 *     app using "make clean/new".
 *     Download image bin to module to run.
 * 
 *   Operation:
 *            
 *     Send data from any port that will transmit the data to modem,and the response
 *     will be print out from debug port.
 *
 * Author:
 * -------
 * -------
 *
 *============================================================================
 *             HISTORY
 *----------------------------------------------------------------------------
 * 
 ****************************************************************************/
#ifdef __EXAMPLE_Godrej_RMS2_OLD_Cntroller_V03__ 
#include "custom_feature_def.h"
#include "ql_type.h"
#include "ql_stdlib.h"
#include "ql_trace.h"
#include "ql_timer.h"
#include "ql_uart.h"
#include "ql_error.h"
#include "ql_gprs.h"
#include "ql_fs.h"
#include "ril.h"
#include "ril_network.h"
#include "ril_http.h"
#include "ril_sms.h"
#include "string.h"
#include "ql_system.h"
#include "fota_main.h"
#include "ril_telephony.h"
#include "ril_system.h"
#include "ril_util.h"
#include "ril_location.h"
#include "ql_socket.h"
#include "ql_wtd.h"


#define DEBUG_ENABLE 1
#if DEBUG_ENABLE > 0
#define DEBUG_PORT  UART_PORT1
#define DBG_BUF_LEN   1024
static char DBG_BUFFER[DBG_BUF_LEN];
#define APP_DEBUG(FORMAT,...) {\
    Ql_memset(DBG_BUFFER, 0, DBG_BUF_LEN);\
    Ql_sprintf(DBG_BUFFER,FORMAT,##__VA_ARGS__); \
    if (UART_PORT2 == (DEBUG_PORT)) \
    {\
        Ql_Debug_Trace(DBG_BUFFER);\
    } else {\
        Ql_UART_Write((Enum_SerialPort)(DEBUG_PORT), (u8*)(DBG_BUFFER), Ql_strlen((const char *)(DBG_BUFFER)));\
    }\
}
#else
#define APP_DEBUG(FORMAT,...) 
#endif


/****************************************************************************
Change			:Data Sending Frequency Change From 5min to 1 min 
On Request		:Omkar Nathi(Production)
Date of Request	:15/06/18 
****************************************************************************/

#define APN_USERID      ""
#define APN_PASSWD      ""




#define APP_BIN_URL   "http://1.22.124.222:81/fota/Chiller_1/Chiller_1_FOTA_APP.bin"
#define HTTP_REQUEST  1  // 0=http-get, 1=http-post, 2=http-file
#define TIMEOUT_COUNT 1
#define max 40
#define  PATH_ROOT   ((u8 *)"myroot")
#define LENGTH 100
#define URL_LEN 512
#define READ_BUF_LEN 1024
#define CON_SMS_BUF_MAX_CNT   (1)
#define CON_SMS_SEG_MAX_CHAR  (160)
#define CON_SMS_SEG_MAX_BYTE  (4 * CON_SMS_SEG_MAX_CHAR)
#define CON_SMS_MAX_SEG       (7)

#define LOGIC_WTD1_TMR_ID  (110 + 1)
#define LOGIC_WTD2_TMR_ID  (110 + 2)
#define LOGIC_WTD3_TMR_ID  (110 + 3)

#define TIMER_ID_WATCHDOG_FEED 	TIMER_ID_USER_START+1
#define Stack_timer  		    TIMER_ID_USER_START+2
#define GP_timer 	 		    TIMER_ID_USER_START+3
#define Stack_timer3 			TIMER_ID_USER_START+4
#define Stack_timer4 			TIMER_ID_USER_START+5
#define Stack_timer5 			TIMER_ID_USER_START+6

Enum_PinName  WATCHDOG_FEED_PIN = PINNAME_CTS;

s32 WTD_Id;

typedef struct
{
    u8 aData[CON_SMS_SEG_MAX_BYTE];
    u16 uLen;
} ConSMSSegStruct;

typedef struct
{
    u16 uMsgRef;
    u8 uMsgTot;

    ConSMSSegStruct asSeg[CON_SMS_MAX_SEG];
    bool abSegValid[CON_SMS_MAX_SEG];
} ConSMSStruct;

ConSMSStruct g_asConSMSBuf[CON_SMS_BUF_MAX_CNT];

u8 APN_NAME[20] = "IOT.COM\0";
//u8 APN_NAME[20] = "M2MISAFE\0";
u8 DEVICE_ID[15]="90000000\0"; 
u8 START_OF_THE_STRING[10] = "$#UG";
u8 VERSION[15]="18.09.15\0";
 
u8 strBuf_url[LENGTH] = {0};
u8 strBuf_status[LENGTH] = {0};
u8 strBuf_data[LENGTH] = {0};
u8 strBuf_id[LENGTH] = {0};
u8 strBuf_apn[LENGTH] = {0};
u8 strBuf_ip[LENGTH] = {0};
u8 strBuf_port[LENGTH] = {0};
u8 strBuf_time[LENGTH] = {0};
u8 strBuf_time1[LENGTH] = {0};

u8 filePath1[LENGTH] = {0};
u8 filePath2[LENGTH] = {0};
u8 filePath3[LENGTH] = {0};
u8 filePath4[LENGTH] = {0};
u8 filePath5[LENGTH] = {0};
u8 filePath6[LENGTH] = {0};
u8 filePath7[LENGTH] = {0};
u8 filePath8[LENGTH] = {0};
u8 filePath9[LENGTH] = {0};
u8 filePath10[LENGTH] = {0};
u8 filePath11[LENGTH] = {0};

//Read parameter
char QUERY_01[8]={0x01,0x03,0x00,0x00,0x00,0x14,0x45,0xc5};// 20 - 40 - 45 - 63

// 01 : Address   03: Function code      00 00 : Add Hi Add Lo       00 14 : Length Hi Length Lo        45 c5 : CRC Lo CRC Hi

//PARAMETERS
u32 PARAMETER_01 = 0;
u32 PARAMETER_02 = 0;
u32 PARAMETER_03 = 0;
u32 PARAMETER_04 = 0;
u32 PARAMETER_05 = 0;
u32 PARAMETER_06 = 0;
u32 PARAMETER_07 = 0;
u32 PARAMETER_08 = 0;
u32 PARAMETER_09 = 0;
u32 PARAMETER_10 = 0;
u32 PARAMETER_11 = 0;
u32 PARAMETER_12 = 0;
u32 PARAMETER_13 = 0;
u32 PARAMETER_14 = 0;
u32 PARAMETER_15 = 0;
u32 PARAMETER_16 = 0;
u32 PARAMETER_17 = 0;
u32 PARAMETER_18 = 0;
u32 PARAMETER_19 = 0;
u32 PARAMETER_20 = 0;



//string store
char QUERY_SEND_STAGE = 0 ,QUERY_RCV_STAGE=0;
char module_state = 0;
char pchData3[500]={0};

static u8 m_URL_Buffer[URL_LEN];
 
u16 my_index = 0;
bool SYSTEM_WATCHDOG_FLAG = 1;
bool uart_flag = 1,socket_flag = 1,reset_flag = 1, send_flag1 = 0; //for uart operation
bool start_flag = 0, send_flag = 0, string_flag = 0, fota_flag=0;
bool storage_flag = 0 , send_data_flag = 0;
bool send_1 = 0,modbus_flag = 0,modbus_flag1 = 0, alarm_flag=0 , data_flag = 0;
bool uart_flag2=0, err_flag=0, comm_flag = 0, GPRS_flag = 0;
bool log_book_flag=1;
bool data_send_to_server_flag = 0;
bool GPRS_Connection_Flag = 0;
bool SOCKET_IN_WOULD_BLOCK_FLAG = 0;
bool SERVER2_SETTING_SELECTION_FLAG=0;

char data_send_count=0,data_count=0,database[900]={0};
char DEVICE_LATITUDE[10]="0000000\0", DEVICE_LONGITUDE[10]="0000000\0";
int address_mod[84], data_mod[84];

u32 ST_Interval =  60 * 1000;  
u32 ST_Interval2 = 150;
u32 ST_Interval3 = 8000; 
u32 ST_Interval4 = 2000;
u32 ST_Interval5 = 900*1000;
u32 ST_Interval6 = 120*1000;
u32 Timer_Val_WatchdogFeed = 500;
u32 WATCHDOG_INTERNAL_FEED_TIMER = 2000;

static s32 m_param1 = 0;
static s32 m_param3 = 0;
static s32 m_param4 = 0;
static s32 m_param5 = 0;
static s32 m_param6 = 0;

u8 ST_Interval1[6] = "01:00"; 
u8 ST_Interval12[6] = "00:30"; 
//static u32 GP_timer = 0x101;
static u32 GPT_Interval =15000;
static s32 m_param2 = 0;
char status_var=0, status_var1=0, status_var2=0, status_var3=0, status_var4=0 ;

int PORT_VALUE_CAL(char port_buf[]);
u32 CONVERT_TIME_BUFFER_TO_SECONDS_VALUE(char interval_buf[]);
void READ_DATA_FROM_UFS(int val_mem);
void WRITE_DATA_TO_UFS(int val_mem1, char writeBuffer[]);
void SEND_SMS(int val_mem1, char writeBuffer[],char phno[]);
void SEPARATE_MODBUS_DATA(char *modbus_write_data);
void CALCULATE_CRC(int address_1, int data_1);
void CALCULATE_PARAMETER_FROM_QUERY(char *DATA);
void CREATE_AND_WRITE_QUERY_FOR_MULTIPLE_ADRESSES(int address,int *data_array, int length);
static u8 m_Read_Buffer[1024];
static u8 m_Read_Buffer_fota[1024] = "http://1.22.124.222:81/fota/Chiller_1/Chiller_1_FOTA_APP.bin";

s32 wtdid2,wtdid1;
bool wd_flag = 1, wd_flag1=1;  // watchdog flags

void WTD_Init();
void WTD_DeInit();
void TIMER_INIITIALIZE();
static void HTTP_Program(u8 get_post);
static void Callback_HTTP_DwnldFile(u32 dllSize, u32 cntntLen, s32 errCode);
static void SIM_Card_State_Ind(u32 sim_stat);
//static void Concate_String(u8* string1);
static void LOCATION_PROGRAM(void);
static void Callback_Location(s32 result,ST_LocInfo* loc_info);
static void Hdlr_RecvNewSMS(u32 nIndex, bool bAutoReply);
static bool SMS_Initialize(void);
static bool ConSMSBuf_IsIntact(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx,ST_RIL_SMS_Con *pCon);
static bool ConSMSBuf_AddSeg(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx,ST_RIL_SMS_Con *pCon,u8 *pData,u16 uLen);
static s8 ConSMSBuf_GetIndex(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,ST_RIL_SMS_Con *pCon);
static bool ConSMSBuf_ResetCtx(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx);
static void callback_onTimer(u32 timerId, void* param);

char DECIMAL_TO_HEX(unsigned n);
static Enum_SerialPort m_myVirtualPort = VIRTUAL_PORT2;
static void DELAY(int time_delay);
static void TIMER_HANDLER(u32 timerId, void* param);
static void Callback_UART_Hdlr_Main_Port(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara);
static void Callback_UART_Hdlr_Modbus_Port(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara);
s32 pdpCntxtId;
/************************************************************************/
/* Definition for GPRS PDP context                                      */
/************************************************************************/
static ST_GprsConfig m_GprsConfig = {
    "",  		// APN name
    "",         // User name for APN
    "",         // Password for APN
    0,
    NULL,
    NULL,
};

/************************************************************************/
/* Definition for Server IP Address and Socket Port Number              */
/************************************************************************/
static u8  m_SrvADDR[20] ="20.204.108.125\0";//"202.71.157.186\0";
static u32 m_SrvPort = 8011;

static u8  m_SrvADDR_1[20] = "20.204.108.125\0";//"202.71.157.186\0";
static u32 m_SrvPort_1 = 8011;
/*
static u8  m_SrvADDR[20] ="1.22.124.222\0";//"1.22.124.222\0";
static u32 m_SrvPort = 6969;

static u8  m_SrvADDR_1[20] ="1.22.124.222\0";//"1.22.124.222\0";
static u32 m_SrvPort_1 = 6969;
*/
u8 m_SrvPort1[5] = {0};   

u8 m_SrvPort12[5] = {0};
#define  SOC_RECV_BUFFER_LEN  1460
static s32 m_GprsActState    = 0;   // GPRS PDP activation state, 0= not activated, 1=activated
static s32 m_SocketId        = -1;  // Store socket Id that returned by Ql_SOC_Create()
static s32 m_SocketConnState = 0;   // Socket connection state, 0= disconnected, 1=connected
static u8  m_SocketRcvBuf[SOC_RECV_BUFFER_LEN];


/************************************************************************/
/* Declarations for GPRS and TCP socket callback                        */
/************************************************************************/
//
// This callback function is invoked when GPRS drops down.
static void Callback_GPRS_Deactived(u8 contextId, s32 errCode, void* customParam );
static void Callback_GPRS_Actived(u8 contexId, s32 errCode, void* customParam);

// This callback function is invoked when the socket connection is disconnected by server or network.
static void Callback_Socket_Close(s32 socketId, s32 errCode, void* customParam );
//
// This callback function is invoked when socket data arrives.
static void Callback_Socket_Read(s32 socketId, s32 errCode, void* customParam );
//
// This callback function is invoked in the following case:
// The return value is less than the data length to send when calling Ql_SOC_Send(), which indicates
// the socket buffer is full. Application should stop sending socket data till this callback function
// is invoked, which indicates application can continue to send data to socket.
static void Callback_Socket_Write(s32 socketId, s32 errCode, void* customParam );
static void SOC_CONNECT_TO_SERVER();
static void GPRS_TCP_Program(char PACKET_TYPE);
 
void proc_main_task()
{
    ST_MSG msg;
    bool keepGoing = TRUE;
    s32 ret;
   	u32 i=0, j=0;
	u8 temp_ch;
	s32 handle = -1;
	u32 readenLen = 0;
	s32 iResult = 0;
	s64  space = 0;	
	u32 writenLen = 0;
	char *p2 = NULL; 
    s32 wtdid;
	wd_flag = 0 ;
	ST_UARTDCB port2_dcb;
	
	port2_dcb.baudrate = 9600;
	port2_dcb.dataBits = DB_8BIT;
	port2_dcb.stopBits = SB_ONE;
	port2_dcb.parity   = PB_ODD;
	port2_dcb.flowCtrl = FC_NONE;

    Ql_UART_Register(UART_PORT1, Callback_UART_Hdlr_Main_Port, NULL);
    Ql_UART_Open(UART_PORT1, 115200, FC_NONE);
   // Ql_UART_Register(UART_PORT2, Callback_UART_Hdlr_Modbus_Port, NULL);
    // Ql_UART_OpenEx(UART_PORT2, &port2_dcb);
	// Ql_UART_Register(UART_PORT3, Callback_UART_Hdlr_Main_Port, NULL);
    // Ql_UART_Open(UART_PORT3, 9600, FC_NONE);
	Ql_UART_Register(UART_PORT2, Callback_UART_Hdlr_Modbus_Port, NULL);
    Ql_UART_OpenEx(UART_PORT2, &port2_dcb);
    
    APP_DEBUG("\r\n<--OpenCPU: transpass test!_GODREJ VERSION: %s-->\r\n", VERSION);  

    // Register & open Modem port
    Ql_UART_Register(m_myVirtualPort, Callback_UART_Hdlr_Main_Port, NULL);
    Ql_UART_Open(m_myVirtualPort, 0, 0);
	Enum_FSStorage storage = Ql_FS_UFS;
    APP_DEBUG("\r\n<-- OpenCPU: FILE(UFS) TEST!-->\r\n");

	module_state =1;
	APP_DEBUG("<-- MODULE STATE => %d \r\n", module_state);  
	
	space  = Ql_FS_GetFreeSpace(storage);
    APP_DEBUG("<-- Ql_FS_GetFreeSpace(storage=%d) =%lld\r\n",storage,space);
    
    //check total space
    space = Ql_FS_GetTotalSpace(storage);                
    APP_DEBUG("<-- Ql_FS_GetTotalSpace(storage=%d)=%lld\r\n",storage,space);
	
	ret = Ql_FS_CheckDir(PATH_ROOT);
    if(ret != QL_RET_OK)
    {
		APP_DEBUG("<-- Dir(%s) is not exist, creating.... -->\r\n", PATH_ROOT);
		ret  = Ql_FS_CreateDir(PATH_ROOT);
		if(ret != QL_RET_OK)
		{
			APP_DEBUG("<-- failed!! Create Dir(%s) fail-->\r\n", PATH_ROOT);
			return -1;
		}
		else
		{
			APP_DEBUG("<-- CreateDir(%s) OK! -->\r\n", PATH_ROOT);
		}        
    }
	else
	{
		APP_DEBUG("<--PATH IS ALREADY PRESENT PATH:%s\r\n", PATH_ROOT);
	
	}

	Ql_memset(m_GprsConfig.apnName, 0x0, sizeof(m_GprsConfig.apnName));
	APP_DEBUG("\r\n\n<-- APN BEFORE = %s\r\n", m_GprsConfig.apnName);  
	Ql_memcpy(m_GprsConfig.apnName, APN_NAME, Ql_strlen(APN_NAME));
	APP_DEBUG("<-- APN AFTER = %s -->\r\n\n", m_GprsConfig.apnName);
	
	APP_DEBUG("<----------------------------------------------------\n");
	APP_DEBUG("<----------------------------------------------------\n");
	READ_DATA_FROM_UFS(1);
	READ_DATA_FROM_UFS(2);
	//READ_DATA_FROM_UFS(3);
	READ_DATA_FROM_UFS(4);
	READ_DATA_FROM_UFS(5);
	READ_DATA_FROM_UFS(6);
	READ_DATA_FROM_UFS(7);
	READ_DATA_FROM_UFS(8);
	//READ_DATA_FROM_UFS(9);
	READ_DATA_FROM_UFS(10);
	READ_DATA_FROM_UFS(11);
	APP_DEBUG("<----------------------------------------------------\n");
	APP_DEBUG("<----------------------------------------------------\n");
	
	module_state =2;
	APP_DEBUG("\r\n<-- MODULE STATE => %d \r\n", module_state);
	
	//check total space
    space  = Ql_FS_GetFreeSpace(storage);
    APP_DEBUG("\r\n<--Ql_FS_GetFreeSpace(storage=%d) =%lld-->\r\n",storage,space);
	
	WTD_Init();
	TIMER_INIITIALIZE();
	
	send_flag1 = 0;
    while (keepGoing)
    {
		Ql_OS_GetMessage(&msg);
        switch(msg.message)
        {
            case MSG_ID_RIL_READY:
                APP_DEBUG("<-- RIL is ready -->\r\n");
                Ql_RIL_Initialize();
            case MSG_ID_URC_INDICATION:
                switch (msg.param1)
                {
                case URC_SYS_INIT_STATE_IND:
                    {
						APP_DEBUG("<-- Sys Init Status %d -->\r\n", msg.param2);
						if (SYS_STATE_SMSOK == msg.param2)
						{
							APP_DEBUG("<-- SMS module is ready -->\r\n");
							APP_DEBUG("<-- Initialize SMS-related options -->\r\n");
							iResult = SMS_Initialize();         
							if (!iResult)
							{
								APP_DEBUG("Fail to initialize SMS\r\n");
							}
						}
						break;
					}
                case URC_CFUN_STATE_IND:
                    APP_DEBUG("<-- CFUN Status:%d -->\r\n", msg.param2);
                    break;
					
				case URC_NEW_SMS_IND:
					{
						APP_DEBUG("<-- New SMS Arrives: index=%d\r\n", msg.param2);
						Hdlr_RecvNewSMS((msg.param2), FALSE);
						break;
					}	
					
                case URC_SIM_CARD_STATE_IND:
                    SIM_Card_State_Ind(msg.param2);
                    break;
                case URC_GSM_NW_STATE_IND:
                    APP_DEBUG("<-- GSM Network Status:%d -->\r\n", msg.param2);
                    break;
                case URC_GPRS_NW_STATE_IND:
                    APP_DEBUG("<-- GPRS Network Status:%d -->\r\n", msg.param2);
                    if (NW_STAT_REGISTERED == msg.param2 || NW_STAT_REGISTERED_ROAMING == msg.param2)
                    {
						start_flag = 1;
						SYSTEM_INITIALIZE();
                    }
                    break;
                }
                break;
            default:
                break;
        }   
    }
}

static void SIM_Card_State_Ind(u32 sim_stat)
{
    switch (sim_stat)
    {
    case SIM_STAT_NOT_INSERTED:
        APP_DEBUG("<-- SIM Card Status: NOT INSERTED -->\r\n");
    	break;
    case SIM_STAT_READY:
        APP_DEBUG("<-- SIM Card Status: READY -->\r\n");
        break;
    case SIM_STAT_PIN_REQ:
        APP_DEBUG("<-- SIM Card Status: SIM PIN -->\r\n");
        break;
    case SIM_STAT_PUK_REQ:
        APP_DEBUG("<-- SIM Card Status: SIM PUK -->\r\n");
        break;
    case SIM_STAT_PH_PIN_REQ:
        APP_DEBUG("<-- SIM Card Status: PH-SIM PIN -->\r\n");
        break;
    case SIM_STAT_PH_PUK_REQ:
        APP_DEBUG("<-- SIM Card Status: PH-SIM PUK -->\r\n");
        break;
    case SIM_STAT_PIN2_REQ:
        APP_DEBUG("<-- SIM Card Status: SIM PIN2 -->\r\n");
        break;
    case SIM_STAT_PUK2_REQ:
        APP_DEBUG("<-- SIM Card Status: SIM PUK2 -->\r\n");
        break;
    case SIM_STAT_BUSY:
        APP_DEBUG("<-- SIM Card Status: BUSY -->\r\n");
        break;
    case SIM_STAT_NOT_READY:
        APP_DEBUG("<-- SIM Card Status: NOT READY -->\r\n");
        break;
    default:
        APP_DEBUG("<-- SIM Card Status: ERROR -->\r\n");
        break;
    }
}

static bool SMS_Initialize(void)
{
    s32 iResult = 0;
    u8  nCurrStorage = 0;
    u32 nUsed = 0;
    u32 nTotal = 0;
    
    // Set SMS storage:
    // By default, short message is stored into SIM card. You can change the storage to ME if needed, or
    // you can do it again to make sure the short message storage is SIM card.
    #if 0
    {
        iResult = RIL_SMS_SetStorage(RIL_SMS_STORAGE_TYPE_SM,&nUsed,&nTotal);
        if (RIL_ATRSP_SUCCESS != iResult)
        {
            APP_DEBUG("Fail to set SMS storage, cause:%d\r\n", iResult);
            return FALSE;
        }
        APP_DEBUG("<-- Set SMS storage to SM, nUsed:%u,nTotal:%u -->\r\n", nUsed, nTotal);

        iResult = RIL_SMS_GetStorage(&nCurrStorage, &nUsed ,&nTotal);
        if(RIL_ATRSP_SUCCESS != iResult)
        {
            APP_DEBUG("Fail to get SMS storage, cause:%d\r\n", iResult);
            return FALSE;
        }
        APP_DEBUG("<-- Check SMS storage: curMem=%d, used=%d, total=%d -->\r\n", nCurrStorage, nUsed, nTotal);
    }
    #endif

    // Enable new short message indication
    // By default, the auto-indication for new short message is enalbed. You can do it again to 
    // make sure that the option is open.
    #if 0
    {
        iResult = Ql_RIL_SendATCmd("AT+CNMI=2,1",Ql_strlen("AT+CNMI=2,1"),NULL,NULL,0);
        if (RIL_AT_SUCCESS != iResult)
        {
            APP_DEBUG("Fail to send \"AT+CNMI=2,1\", cause:%d\r\n", iResult);
            return FALSE;
        }
        APP_DEBUG("<-- Enable new SMS indication -->\r\n");
    }
    #endif

    // Delete all existed short messages (if needed)
    iResult = RIL_SMS_DeleteSMS(0, RIL_SMS_DEL_ALL_MSG);
    if (iResult != RIL_AT_SUCCESS)
    {
        APP_DEBUG("Fail to delete all messages, cause:%d\r\n", iResult);
        return FALSE;
    }
    APP_DEBUG("Delete all existed messages\r\n");
    
    return TRUE;
}

static s8 ConSMSBuf_GetIndex(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,ST_RIL_SMS_Con *pCon)
{
	u8 uIdx = 0;
	
    if(    (NULL == pCSBuf) || (0 == uCSMaxCnt) 
        || (NULL == pCon)
      )
    {
        APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! Parameter is INVALID. pCSBuf:%x,uCSMaxCnt:%d,pCon:%x\r\n",pCSBuf,uCSMaxCnt,pCon);
        return -1;
    }

    if((pCon->msgTot) > CON_SMS_MAX_SEG)
    {
        APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! msgTot:%d is larger than limit:%d\r\n",pCon->msgTot,CON_SMS_MAX_SEG);
        return -1;
    }
    
	for(uIdx = 0; uIdx < uCSMaxCnt; uIdx++)  //Match all exist records
	{
        if(    (pCon->msgRef == pCSBuf[uIdx].uMsgRef)
            && (pCon->msgTot == pCSBuf[uIdx].uMsgTot)
          )
        {
            return uIdx;
        }
	}

	for (uIdx = 0; uIdx < uCSMaxCnt; uIdx++)
	{
		if (0 == pCSBuf[uIdx].uMsgTot)  //Find the first unused record
		{
            pCSBuf[uIdx].uMsgTot = pCon->msgTot;
            pCSBuf[uIdx].uMsgRef = pCon->msgRef;
            
			return uIdx;
		}
	}

    APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! No avail index in ConSMSBuf,uCSMaxCnt:%d\r\n",uCSMaxCnt);
    
	return -1;
}

/*****************************************************************************
 * FUNCTION
 *  ConSMSBuf_AddSeg
 *
 * DESCRIPTION
 *  This function is used to add segment in <pCSBuf>
 *  
 * PARAMETERS
 *  <pCSBuf>     The SMS index in storage,it starts from 1
 *  <uCSMaxCnt>  TRUE: The module should reply a SMS to the sender; FALSE: The module only read this SMS.
 *  <uIdx>       Index of <pCSBuf> which will be stored
 *  <pCon>       The pointer of 'ST_RIL_SMS_Con' data
 *  <pData>      The pointer of CON-SMS-SEG data
 *  <uLen>       The length of CON-SMS-SEG data
 *
 * RETURNS
 *  FALSE:   FAIL!
 *  TRUE: SUCCESS.
 *
 * NOTE
 *  1. This is an internal function
 *****************************************************************************/
static bool ConSMSBuf_AddSeg(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx,ST_RIL_SMS_Con *pCon,u8 *pData,u16 uLen)
{
    u8 uSeg = 1;
    
    if(    (NULL == pCSBuf) || (0 == uCSMaxCnt) 
        || (uIdx >= uCSMaxCnt)
        || (NULL == pCon)
        || (NULL == pData)
        || (uLen > (CON_SMS_SEG_MAX_CHAR * 4))
      )
    {
        APP_DEBUG("Enter ConSMSBuf_AddSeg,FAIL! Parameter is INVALID. pCSBuf:%x,uCSMaxCnt:%d,uIdx:%d,pCon:%x,pData:%x,uLen:%d\r\n",pCSBuf,uCSMaxCnt,uIdx,pCon,pData,uLen);
        return FALSE;
    }

    if((pCon->msgTot) > CON_SMS_MAX_SEG)
    {
        APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! msgTot:%d is larger than limit:%d\r\n",pCon->msgTot,CON_SMS_MAX_SEG);
        return FALSE;
    }

    uSeg = pCon->msgSeg;
    pCSBuf[uIdx].abSegValid[uSeg-1] = TRUE;
    Ql_memcpy(pCSBuf[uIdx].asSeg[uSeg-1].aData,pData,uLen);
    pCSBuf[uIdx].asSeg[uSeg-1].uLen = uLen;
    
	return TRUE;
}

/*****************************************************************************
 * FUNCTION
 *  ConSMSBuf_IsIntact
 *
 * DESCRIPTION
 *  This function is used to check the CON-SMS is intact or not
 *  
 * PARAMETERS
 *  <pCSBuf>     The SMS index in storage,it starts from 1
 *  <uCSMaxCnt>  TRUE: The module should reply a SMS to the sender; FALSE: The module only read this SMS.
 *  <uIdx>       Index of <pCSBuf> which will be stored
 *  <pCon>       The pointer of 'ST_RIL_SMS_Con' data
 *
 * RETURNS
 *  FALSE:   FAIL!
 *  TRUE: SUCCESS.
 *
 * NOTE
 *  1. This is an internal function
 *****************************************************************************/
static bool ConSMSBuf_IsIntact(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx,ST_RIL_SMS_Con *pCon)
{
    u8 uSeg = 1;
	
    if(    (NULL == pCSBuf) 
        || (0 == uCSMaxCnt) 
        || (uIdx >= uCSMaxCnt)
        || (NULL == pCon)
      )
    {
        APP_DEBUG("Enter ConSMSBuf_IsIntact,FAIL! Parameter is INVALID. pCSBuf:%x,uCSMaxCnt:%d,uIdx:%d,pCon:%x\r\n",pCSBuf,uCSMaxCnt,uIdx,pCon);
        return FALSE;
    }

    if((pCon->msgTot) > CON_SMS_MAX_SEG)
    {
        APP_DEBUG("Enter ConSMSBuf_GetIndex,FAIL! msgTot:%d is larger than limit:%d\r\n",pCon->msgTot,CON_SMS_MAX_SEG);
        return FALSE;
    }
        
	for (uSeg = 1; uSeg <= (pCon->msgTot); uSeg++)
	{
        if(FALSE == pCSBuf[uIdx].abSegValid[uSeg-1])
        {
            APP_DEBUG("Enter ConSMSBuf_IsIntact,FAIL! uSeg:%d has not received!\r\n",uSeg);
            return FALSE;
        }
	}
    
    return TRUE;
}

/*****************************************************************************
 * FUNCTION
 *  ConSMSBuf_ResetCtx
 *
 * DESCRIPTION
 *  This function is used to reset ConSMSBuf context
 *  
 * PARAMETERS
 *  <pCSBuf>     The SMS index in storage,it starts from 1
 *  <uCSMaxCnt>  TRUE: The module should reply a SMS to the sender; FALSE: The module only read this SMS.
 *  <uIdx>       Index of <pCSBuf> which will be stored
 *
 * RETURNS
 *  FALSE:   FAIL!
 *  TRUE: SUCCESS.
 *
 * NOTE
 *  1. This is an internal function
 *****************************************************************************/

static bool ConSMSBuf_ResetCtx(ConSMSStruct *pCSBuf,u8 uCSMaxCnt,u8 uIdx)
{
    if(    (NULL == pCSBuf) || (0 == uCSMaxCnt) 
        || (uIdx >= uCSMaxCnt)
      )
    {
        APP_DEBUG("Enter ConSMSBuf_ResetCtx,FAIL! Parameter is INVALID. pCSBuf:%x,uCSMaxCnt:%d,uIdx:%d\r\n",pCSBuf,uCSMaxCnt,uIdx);
        return FALSE;
    }
    
    //Default reset
    Ql_memset(&pCSBuf[uIdx],0x00,sizeof(ConSMSStruct));

    //TODO: Add special reset here
    
    return TRUE;
}

void READ_DATA_FROM_UFS(int val_mem)
{
	s32 ret = -1;
	s32 handle = -1;
	u32 readenLen = 0,length_1 =100;
	u32 i, j ;
	u8 *filename;
	u8 *filename1 = "status.txt";
	u8 *filename2 = "url.txt";
	u8 *filename3 = "data.txt";
	u8 *filename4 = "id.txt";
	u8 *filename5 = "apn.txt";
	u8 *filename6 = "ip.txt";
	u8 *filename7 = "port.txt";
	u8 *filename8 = "time.txt";
	u8 *filename9 = "time1.txt";
	u8 *filename10 = "ip1.txt";
	u8 *filename11 = "port1.txt";
	u8 filePath[LENGTH] = {0};
	if(val_mem == 3)
	{
		length_1 = 8 * 1024;
	}
	u8 strBuf[length_1] ;
	for(i=0; i<length_1; i++)
	{
		strBuf[i] = '\0';
	}
	APP_DEBUG("<----------------------------------------------------\r\n");
	APP_DEBUG("<-- FILE NO: %d\r\n", val_mem)
	
	if(val_mem == 1)
	{
		Ql_sprintf(filePath,"%s\\%s\0",PATH_ROOT,filename1);
		Ql_sprintf(filePath1,"%s\\%s\0",PATH_ROOT,filename1);
  
	}
	else if(val_mem == 2)
	{
		Ql_sprintf(filePath,"%s\\%s\0",PATH_ROOT,filename2);
		Ql_sprintf(filePath2,"%s\\%s\0",PATH_ROOT,filename2);
	}
	else if(val_mem == 3)
	{
		Ql_sprintf(filePath,"%s\\%s\0",PATH_ROOT,filename3);
		Ql_sprintf(filePath3,"%s\\%s\0",PATH_ROOT,filename3);
	}
	else if(val_mem == 4)
	{
		Ql_sprintf(filePath,"%s\\%s\0",PATH_ROOT,filename4);
		Ql_sprintf(filePath4,"%s\\%s\0",PATH_ROOT,filename4);
	}
	else if(val_mem == 5)
	{
		Ql_sprintf(filePath,"%s\\%s\0",PATH_ROOT,filename5);
		Ql_sprintf(filePath5,"%s\\%s\0",PATH_ROOT,filename5);
	}
	else if(val_mem == 6)
	{
		Ql_sprintf(filePath,"%s\\%s\0",PATH_ROOT,filename6);
		Ql_sprintf(filePath6,"%s\\%s\0",PATH_ROOT,filename6);
	}
	else if(val_mem == 7)
	{
		Ql_sprintf(filePath,"%s\\%s\0",PATH_ROOT,filename7);
		Ql_sprintf(filePath7,"%s\\%s\0",PATH_ROOT,filename7);
	}
	else if(val_mem == 8)
	{
		Ql_sprintf(filePath,"%s\\%s\0",PATH_ROOT,filename8);
		Ql_sprintf(filePath8,"%s\\%s\0",PATH_ROOT,filename8);
	}
	else if(val_mem == 9)
	{
		Ql_sprintf(filePath,"%s\\%s\0",PATH_ROOT,filename9);
		Ql_sprintf(filePath9,"%s\\%s\0",PATH_ROOT,filename9);
	}
	else if(val_mem == 10)
	{
		Ql_sprintf(filePath,"%s\\%s\0",PATH_ROOT,filename10);
		Ql_sprintf(filePath10,"%s\\%s\0",PATH_ROOT,filename10);
	}
	else if(val_mem == 11)
	{
		Ql_sprintf(filePath,"%s\\%s\0",PATH_ROOT,filename11);
		Ql_sprintf(filePath11,"%s\\%s\0",PATH_ROOT,filename11);
	}
	else{}

	APP_DEBUG("<--filepath=%s\r\n", filePath);
	
    ret = Ql_FS_Check(filePath);
    if(ret != QL_RET_OK)
    {
		APP_DEBUG("<--filepath=%s  does not exist creat file! -->\r\n", filePath);             
    }

	//open file if file does not exist ,creat it
	handle = Ql_FS_Open(filePath,QL_FS_READ_WRITE |QL_FS_CREATE );
    //APP_DEBUG("\r\n<--!! Ql_FS_Open  handle =%d -->\r\n",handle);
	Ql_FS_Flush(handle);       
    ret = Ql_FS_Seek(handle,0,QL_FS_FILE_BEGIN);    
    //read file
    ret = Ql_FS_Read(handle, strBuf, length_1, &readenLen);
    APP_DEBUG("<-- Ql_FS_Read() ret=%d: readedlen=%d, readBuffer=%s-->\r\n\n", ret, readenLen, strBuf);
    //close file
	
	if(val_mem == 1)
	{
		Ql_strcpy(strBuf_status, strBuf);
	}
	else if(val_mem == 2)
	{
		Ql_strcpy(strBuf_url, strBuf);
	}
	else if(val_mem == 3)
	{
		
	}
	else if(val_mem == 4)
	{
		Ql_strcpy(strBuf_id, strBuf);
		if((strBuf_id[0] == '8') && (strBuf_id[1] == '5')) 
		{
			for(i =0; i<15 ; i++ )
			{
				DEVICE_ID[i] = strBuf_id[i];
				if('\0' == strBuf_id[i]){break;}
			}
			APP_DEBUG("<-- ID CHANGE\r\n");
		}
		APP_DEBUG("<-- READ_BUFF_DATA=%s\r\n",strBuf_id);
		APP_DEBUG("<-- DEVICE ID=%s-->\r\n", DEVICE_ID);
	}
	else if(val_mem == 5)
	{
		Ql_strcpy(strBuf_apn, strBuf);
		if(strBuf_apn[0] == '\0')
		{}
		else
		{
			for(i =0; i<25 ; i++)
			{
				APN_NAME[i] = strBuf_apn[i];
				if('\0' == strBuf_apn[i]){break;}
			}
			Ql_memset(m_GprsConfig.apnName, 0x0, sizeof(m_GprsConfig.apnName));
			APP_DEBUG("<-- APN BEFORE COPY= %s\r\n", m_GprsConfig.apnName);  
			Ql_memcpy(m_GprsConfig.apnName, APN_NAME, Ql_strlen(APN_NAME));
			APP_DEBUG("<-- APN AFTER COPY= %s\r\n", m_GprsConfig.apnName);
			APP_DEBUG("<-- APN CHANGE\r\n");
		}
		APP_DEBUG("<-- READ_BUFF_DATA=%s\r\n",strBuf_apn);
		APP_DEBUG("<-- DEVICE APN =%s\r\n", APN_NAME);
	}
	else if(val_mem == 6)
	{
		Ql_strcpy(strBuf_ip, strBuf);
		if(strBuf_ip[0] == '\0')
		{}
		else
		{
			for(i =0; i<15 ; i++)
			{
				m_SrvADDR[i] = strBuf_ip[i];
				if('\0' == strBuf_ip[i]){break;}
			}
			APP_DEBUG("\n<-- IP1 CHANGE\r\n");
		}
		APP_DEBUG("<-- READ_BUFF_DATA=%s\r\n",strBuf_ip);
		APP_DEBUG("<-- DEVICE IP1 =%s-->\r\n", m_SrvADDR);
	}
	else if(val_mem == 7)
	{
		Ql_strcpy(strBuf_port, strBuf);
		if(strBuf_port[0] == '\0')
		{}
		else
		{
			for(i =0; i<10 ; i++)
			{
				m_SrvPort1[i] = strBuf_port[i];
				if('\0' == strBuf_port[i]){break;}
			}
			m_SrvPort = PORT_VALUE_CAL(m_SrvPort1);
			APP_DEBUG("<-- PORT1 CHANGE\r\n");
		}
		APP_DEBUG("<-- READ_BUFF_DATA=%s\r\n",strBuf_port);
		APP_DEBUG("<-- PORT1 IN ARRAY=%s\r\n", m_SrvPort1);
		APP_DEBUG("<-- PORT1 IN DECIMAL=%d\r\n", m_SrvPort);
	}
	else if(val_mem == 8)
	{
		Ql_strcpy(strBuf_time, strBuf);
		if(strBuf_time[0] == '\0')
		{}
		else if(strBuf_time[3] < '6')
		{
			for(i =0; i<5 ; i++)
			{
				ST_Interval1[i] = strBuf_time[i];
				if('\0' == strBuf_time[i]){break;}
			}
			if(i == 5)
			{
				ST_Interval = CONVERT_TIME_BUFFER_TO_SECONDS_VALUE(strBuf_time);
				APP_DEBUG("<-- TIME CHANGE\r\n");
			}
		}
		APP_DEBUG("<-- READ_BUFF_DATA=%s\r\n",strBuf_time);
		APP_DEBUG("<-- TIME1 IN ARRAY=%s\r\n", ST_Interval1);
		APP_DEBUG("<-- TIME1 IN mSEC=%d\r\n", ST_Interval);
	}
	else if(val_mem == 9)
	{
		Ql_strcpy(strBuf_time1, strBuf);
		if(strBuf_time1[0] == '\0')
		{}
		else if(strBuf_time1[3] < '6')
		{
			for(i =0; i<5 ; i++)
			{
				ST_Interval12[i] = strBuf_time1[i];
				if('\0' == strBuf_time1[i]){break;}
			}
			if(i == 5)
			{
				ST_Interval2 = CONVERT_TIME_BUFFER_TO_SECONDS_VALUE(strBuf_time1);
				APP_DEBUG("\nchange time");
			}
		}
		APP_DEBUG("\r\nID1=%s-->\r\n",strBuf_time);
		APP_DEBUG("\r\nID=%s-->\r\n", ST_Interval12);
		APP_DEBUG("\r\nID=%d-->\r\n", ST_Interval2);
	}
	else if(val_mem == 10)
	{
		Ql_strcpy(strBuf_ip, strBuf);
		if(strBuf_ip[0] == '\0')
		{}
		else
		{
			for(i =0; i<15 ; i++)
			{
				m_SrvADDR_1[i] = strBuf_ip[i];
				if('\0' == strBuf_ip[i]){break;}
			}
			APP_DEBUG("<-- CHANGE IP2\r\n");
		}
		APP_DEBUG("<-- READ_BUFF_DATA=%s\r\n",strBuf_ip);
		APP_DEBUG("<-- DEVICE IP2 =%s-->\r\n", m_SrvADDR_1);
	}
	else if(val_mem == 11)
	{
		Ql_strcpy(strBuf_port, strBuf);
		if(strBuf_port[0] == '\0')
		{}
		else
		{
			for(i =0; i<15 ; i++)
			{
				m_SrvPort12[i] = strBuf_port[i];
				if('\0' == strBuf_port[i]){break;}
			}
			m_SrvPort_1 = PORT_VALUE_CAL(m_SrvPort12);
			APP_DEBUG("<-- CHANGE PORT2\r\n");
		}
		APP_DEBUG("<-- READ_BUFF_DATA=%s\r\n",strBuf_port);
		APP_DEBUG("<-- PORT2 IN ARRAY=%s\r\n", m_SrvPort12);
		APP_DEBUG("<-- PORT2 IN DECIMAL=%d\r\n", m_SrvPort_1);
	}
	Ql_FS_Close(handle);  
	APP_DEBUG("<----------------------------------------------------\r\n");
}

void WRITE_DATA_TO_UFS(int val_mem1,char writeBuffer[])
{
	u32 writenLen = 0;
	u32 readenLen = 0;
	s32 position = 0;    
	s32 handle = -1;
	u8 strBuf[LENGTH] = {0},ret;			
		
	APP_DEBUG("1234");		
	switch(val_mem1)
	{
		case 1: handle = Ql_FS_Open(filePath1,QL_FS_READ_WRITE |QL_FS_CREATE );
				APP_DEBUG("\r\n<--!! Ql_FS_Open  handle =%d -->\r\n",handle);
				APP_DEBUG("\r\n<--filepath=%s -->\r\n", filePath1);
				break;
				
		case 2: handle = Ql_FS_Open(filePath2,QL_FS_READ_WRITE |QL_FS_CREATE );
				APP_DEBUG("\r\n<--!! Ql_FS_Open  handle =%d -->\r\n",handle);
				APP_DEBUG("\r\n<--filepath=%s -->\r\n", filePath2);
				break;
				
		case 3: handle = Ql_FS_Open(filePath3,QL_FS_READ_WRITE |QL_FS_CREATE );
				APP_DEBUG("\r\n<--!! Ql_FS_Open  handle =%d -->\r\n",handle);
				APP_DEBUG("\r\n<--filepath=%s -->\r\n", filePath3);
				break;
				
		case 4: handle = Ql_FS_Open(filePath4,QL_FS_READ_WRITE |QL_FS_CREATE );
				APP_DEBUG("\r\n<--!! Ql_FS_Open  handle =%d -->\r\n",handle);
				APP_DEBUG("\r\n<--filepath=%s -->\r\n", filePath4);
				break;
		
		case 5: handle = Ql_FS_Open(filePath5,QL_FS_READ_WRITE |QL_FS_CREATE );
				APP_DEBUG("\r\n<--!! Ql_FS_Open  handle =%d -->\r\n",handle);
				APP_DEBUG("\r\n<--filepath=%s -->\r\n", filePath5);
				break;
		
		case 6: handle = Ql_FS_Open(filePath6,QL_FS_READ_WRITE |QL_FS_CREATE );
				APP_DEBUG("\r\n<--!! Ql_FS_Open  handle =%d -->\r\n",handle);
				APP_DEBUG("\r\n<--filepath=%s -->\r\n", filePath6);
				break;
		
		case 7: handle = Ql_FS_Open(filePath7,QL_FS_READ_WRITE |QL_FS_CREATE );
				APP_DEBUG("\r\n<--!! Ql_FS_Open  handle =%d -->\r\n",handle);
				APP_DEBUG("\r\n<--filepath=%s -->\r\n", filePath7);
				break;
			
		case 8: handle = Ql_FS_Open(filePath8,QL_FS_READ_WRITE |QL_FS_CREATE );
				APP_DEBUG("\r\n<--!! Ql_FS_Open  handle =%d -->\r\n",handle);
				APP_DEBUG("\r\n<--filepath=%s -->\r\n", filePath8);
				break;
		
		case 9: handle = Ql_FS_Open(filePath9,QL_FS_READ_WRITE |QL_FS_CREATE );
				APP_DEBUG("\r\n<--!! Ql_FS_Open  handle =%d -->\r\n",handle);
				APP_DEBUG("\r\n<--filepath=%s -->\r\n", filePath9);
				break;
			
		case 10: handle = Ql_FS_Open(filePath10,QL_FS_READ_WRITE |QL_FS_CREATE );
				APP_DEBUG("\r\n<--!! Ql_FS_Open  handle =%d -->\r\n",handle);
				APP_DEBUG("\r\n<--filepath=%s -->\r\n", filePath10);
				break;
				
		case 11: handle = Ql_FS_Open(filePath11,QL_FS_READ_WRITE |QL_FS_CREATE );
				APP_DEBUG("\r\n<--!! Ql_FS_Open  handle =%d -->\r\n",handle);
				APP_DEBUG("\r\n<--filepath=%s -->\r\n", filePath11);
				break;
	
		default:break;
	}

	Ql_FS_Flush(handle);       
	ret = Ql_FS_Seek(handle,0,QL_FS_FILE_BEGIN);
	APP_DEBUG("\r\n<--!! Ql_FS_Seek   ret =%d-->\r\n",ret);    
	position = Ql_FS_GetFilePosition(handle);
	APP_DEBUG("\r\n<--!! Ql_FS_GetFilePosition   position =%d-->\r\n",position);
	ret = Ql_FS_Truncate(handle);
	 ret = Ql_FS_Write(handle, writeBuffer, Ql_strlen(writeBuffer), &writenLen);
	APP_DEBUG("\r\n<--!! Ql_FS_Write  ret =%d  writenLen =%d-->\r\n",ret,writenLen);
	Ql_FS_Flush(handle);       
	ret = Ql_FS_Seek(handle,0,QL_FS_FILE_BEGIN);    
	ret = Ql_FS_Read(handle, strBuf, LENGTH, &readenLen);
	APP_DEBUG("\r\n<-- Ql_FS_Read() ret=%d: readedlen=%d, readBuffer=%s-->\r\n", ret, readenLen, strBuf);
	Ql_FS_Close(handle);
}

void SEND_SMS(int val_mem1, char aReplyCon1[],char aPhNum1[])
{
	s32 iResult = 0 ;
    u32 uMsgRef = 0;

	if (!Ql_strstr(aPhNum1, "10086"))  // Not reply SMS from operator
        {
            APP_DEBUG("<-- Replying SMS... -->\r\n");
            iResult = RIL_SMS_SendSMS_Text(aPhNum1, Ql_strlen(aPhNum1),LIB_SMS_CHARSET_GSM,(u8*)aReplyCon1,Ql_strlen(aReplyCon1),&uMsgRef);
            if (iResult != RIL_AT_SUCCESS)
            {
                APP_DEBUG("RIL_SMS_SendSMS_Text FAIL! iResult:%u\r\n",iResult);
                return;
            }
            APP_DEBUG("<-- RIL_SMS_SendTextSMS OK. uMsgRef:%d -->\r\n", uMsgRef);
        }

}

void TIMER_INIITIALIZE()
{
	s32 ret;
	
	ret = Ql_Timer_Register(Stack_timer, TIMER_HANDLER, &m_param1);
    if(ret <0)
    {
        APP_DEBUG("\r\n<--failed!!, Ql_Timer_Register: timer(%d) fail ,ret = %d -->\r\n",Stack_timer,ret);
    }
    APP_DEBUG("\r\n<--Register: timerId=%d, param = %d,ret = %d -->\r\n", Stack_timer ,m_param1,ret); 

	ret = Ql_Timer_Register(Stack_timer3, TIMER_HANDLER, &m_param4);
    if(ret <0)
    {
        APP_DEBUG("\r\n<--failed!!, Ql_Timer_Register: timer(%d) fail ,ret = %d -->\r\n",Stack_timer3,ret);
    }
    APP_DEBUG("\r\n<--Register: timerId=%d, param = %d,ret = %d -->\r\n", Stack_timer3 ,m_param4,ret); 
	
	ret = Ql_Timer_Register(Stack_timer4, TIMER_HANDLER, &m_param5);
    if(ret <0)
    {
        APP_DEBUG("\r\n<--failed!!, Ql_Timer_Register: timer(%d) fail ,ret = %d -->\r\n",Stack_timer4,ret);
    }
    APP_DEBUG("\r\n<--Register: timerId=%d, param = %d,ret = %d -->\r\n", Stack_timer4 ,m_param5,ret); 
	
	ret = Ql_Timer_Register(Stack_timer5, TIMER_HANDLER, &m_param6);
    if(ret <0)
    {
        APP_DEBUG("\r\n<--failed!!, Ql_Timer_Register: timer(%d) fail ,ret = %d -->\r\n",Stack_timer5,ret);
    }
    APP_DEBUG("\r\n<--Register: timerId=%d, param = %d,ret = %d -->\r\n", Stack_timer5 ,m_param6,ret); 
	
    //register  a GP-Timer
    ret = Ql_Timer_Register(GP_timer, TIMER_HANDLER, &m_param2);
    if(ret <0)
    {
        APP_DEBUG("\r\n<--failed!!, Ql_Timer_RegisterFast: GP_timer(%d) fail ,ret = %d -->\r\n",GP_timer,ret);
    }
    APP_DEBUG("\r\n<--RegisterFast: timerId=%d, param = %d,ret = %d -->\r\n", GP_timer ,m_param2,ret);
	
}

void SYSTEM_INITIALIZE()
{
	s32 ret;
	if(start_flag == 1)
	{
		start_flag = 0;
		wd_flag = 1;
		
		module_state = 3;
		APP_DEBUG("\r\n module_state = %d \r\n", module_state);  
		LOCATION_PROGRAM();
		Ql_UART_ClrRxBuffer(UART_PORT1);
		Ql_UART_ClrRxBuffer(UART_PORT2);
		Ql_UART_ClrRxBuffer(UART_PORT3);
		//send_flag = 1;
		uart_flag = 1;
		uart_flag2 = 1;
		send_1 = 1;
		send_flag1 = 1;
		QUERY_SEND_STAGE = 1;
		ret = Ql_Timer_Start(Stack_timer,ST_Interval,TRUE);
		
		if(ret < 0)
		{
			APP_DEBUG("\r\n<--failed!! stack timer Ql_Timer_Start ret=%d-->\r\n",ret);        
		}
		APP_DEBUG("\r\n<--stack timer Ql_Timer_Start(ID=%d,Interval=%d,) ret=%d-->\r\n",Stack_timer,ST_Interval,ret);
		ret = Ql_Timer_Start(GP_timer,GPT_Interval,TRUE);
		if(ret < 0)
		{
			APP_DEBUG("\r\n<--failed!! GP-timer Ql_Timer_Start fail, ret=%d-->\r\n",ret);
		}                
		APP_DEBUG("\r\n<--GP-timer Ql_Timer_Start(ID=%d,Interval=%d) ret=%d-->\r\n",GP_timer,GPT_Interval,ret);
		
		ret = Ql_Timer_Start(Stack_timer5 ,ST_Interval5,TRUE);
		
		if(ret < 0)
		{
			APP_DEBUG("\r\n<--failed!! stack timer5 Ql_Timer_Start ret=%d-->\r\n",ret);        
		}
		APP_DEBUG("\r\n<--stack timer Ql_Timer_Start(ID=%d,Interval=%d,) ret=%d-->\r\n",Stack_timer5,ST_Interval5,ret);
		
		module_state = 4;
		APP_DEBUG("\r\n module_state = %d \r\n", module_state);  
		SYSTEM_WATCHDOG_FLAG = 1;
		//GPRS_TCP_Program(0);
	}

}

void FOTA_upgrade()
{
	s32 ret;
	//if(fota_flag == 1)
	//{
		ret = Ql_Timer_Stop(Stack_timer);
			if(ret < 0)
			{
				  APP_DEBUG("\r\n<--failed!! stack timer Ql_Timer_Stop ret=%d-->\r\n",ret);           
			}
			APP_DEBUG("\r\n<--stack timer Ql_Timer_Stop(ID=%d,) ret=%d-->\r\n",Stack_timer,ret);   
		send_flag = 0;
		start_flag = 0;
		Ql_UART_Close(UART_PORT1);
		Ql_UART_ClrRxBuffer(UART_PORT1);
		Ql_UART_Close(UART_PORT2);
		Ql_UART_ClrRxBuffer(UART_PORT2);
		Ql_UART_Close(UART_PORT3);
		Ql_UART_ClrRxBuffer(UART_PORT3);
		ST_GprsConfig apnCfg;
		Ql_memcpy(apnCfg.apnName,   APN_NAME, Ql_strlen(APN_NAME));
		Ql_memcpy(apnCfg.apnUserId, APN_USERID, Ql_strlen(APN_USERID));
		Ql_memcpy(apnCfg.apnPasswd, APN_PASSWD, Ql_strlen(APN_PASSWD));

		Ql_memset(m_URL_Buffer, 0, URL_LEN);
		
		//http://hostname:port/filePath/fileName
		Ql_sprintf(m_URL_Buffer, "%s%s",APP_BIN_URL,m_Read_Buffer_fota);
		APP_DEBUG("\r\n<-- URL:%s-->\r\n",m_URL_Buffer);
		
		Ql_FOTA_StartUpgrade(APP_BIN_URL, &apnCfg, NULL);
		//break;
	
	//}

}

void MODBUS_WRITE_QUERIES()
{
	s32 ret;
	if(modbus_flag1 == 1)
	{
		APP_DEBUG("\r\enter mod write\r\n");
		modbus_flag1 = 0;
		APP_DEBUG("\r\data_count = %d\r\n", data_count);
		if(data_count > 0)
		{
			data_count--;
			APP_DEBUG("\r\n data = %d, address = %d \r\n", data_mod[data_count],address_mod[data_count] );
			CALCULATE_CRC(address_mod[data_count], data_mod[data_count]);
		}
		else
		{
			ret = Ql_Timer_Stop(Stack_timer4);
			if(ret < 0)
			{
				  APP_DEBUG("\r\n<--failed!! stack timer Ql_Timer_Stop3 ret=%d-->\r\n",ret);           
			}
			APP_DEBUG("\r\n<--stack timer Ql_Timer_Stop3(ID=%d,) ret=%d-->\r\n",Stack_timer3,ret); 
			modbus_flag = 1;
			APP_DEBUG("\r\n modbus_flag = 1\r\n");
		}
		APP_DEBUG("\r\exit mod write\r\n");
	}	
}

void MODBUS_READ_QURIES()
{
	if(send_flag1 == 1)
	{
		switch(QUERY_SEND_STAGE)
		{
			case 1: Ql_UART_Write(UART_PORT2, QUERY_01, 8 );
					QUERY_RCV_STAGE = 1;
					QUERY_SEND_STAGE = 1 ;
					send_flag1 = 0;
					APP_DEBUG("\r\n<-- Query No. 1 SEND\r\n");
					break;
					
			default:break;
		}     
	}
}

static void Callback_UART_Hdlr_Main_Port(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{
    u32 rdLen=0;
	int i, k,lenn;
	char my_char[1024]={0};
	s32 ret;
	bool setting_flag = 0;
    char *q=NULL,*p=NULL;
	
	if(m_myVirtualPort == port)
    {
        switch (msg)
        {
            case EVENT_UART_READY_TO_READ:
            {
                Ql_memset(m_Read_Buffer, 0x0, sizeof(m_Read_Buffer));
                rdLen = Ql_UART_Read(port, m_Read_Buffer, sizeof(m_Read_Buffer));

                //APP_DEBUG("\r\nmodem data =%s\r\n",m_Read_Buffer);      
                break;
            }
            default:
            break;
        }
    }
    else if( UART_PORT1 == port | UART_PORT2 == port | UART_PORT3 == port)
    {
        switch (msg)
        {
            case EVENT_UART_READY_TO_READ:
            {	
				Ql_memset(m_Read_Buffer, 0x0, sizeof(m_Read_Buffer));
                rdLen = Ql_UART_Read(port, m_Read_Buffer, sizeof(m_Read_Buffer));
				
				APP_DEBUG("\r\DATA LENGTH = %d\r\n",rdLen);      
				APP_DEBUG("\r\DATA RECIEVED: %s\r\n",m_Read_Buffer);  

				p = Ql_strstr(m_Read_Buffer,"SEND MSG");
				if(p)
				{
					SEND_SMS(1,"MODULE SMS\0","+918087850993\0");
				}
				
				if(m_Read_Buffer[0] == '$')
				{
					setting_flag = 1;//configuration message is start with '$' symbol , if this symobl is presrnt allow message to enter in configuration code.
				}
				
				if(setting_flag == 1)// Device ID setting and Status checking same as SMS module
				{
					setting_flag = 0;
					q = Ql_strstr(m_Read_Buffer,"CVID#");
					if(q)
					{
						u8 device_id_rcv1[15]="";
						
						APP_DEBUG("\r\nenter\r\n");
						for(i = 0 ; i < 8; i++)
						{
							device_id_rcv1[i] = m_Read_Buffer[6 + i];	
						}
						device_id_rcv1[i] = '\0';	
						if(m_Read_Buffer[6 + i] == '#')
						{		
							if((device_id_rcv1[0] != '8') || (device_id_rcv1[1] != '5'))
							{		
								APP_DEBUG("<-- START OF THE ID NOT MATCH ID START WITH 85-->\r\n");	
							}
							else
							{
								APP_DEBUG("<-- ID VALID  -->\r\n");
								for(i=0; i<8; i++)
								{
									DEVICE_ID[i] = device_id_rcv1[i];
								}
								DEVICE_ID[i] = 0;
								APP_DEBUG("<---DEVICE ID: %s --->\r\n",DEVICE_ID);
								WRITE_DATA_TO_UFS(4, DEVICE_ID);	
							}
						}
						else
						{
							APP_DEBUG("<-- LENGTH OF THE ID IS WRONG, LENGTH MUST BE 8 -->\r\n");
						}		
					}
					else
					{
						APP_DEBUG("<-- Not Enter In ID Session  -->\r\n");
					}
					
					q = Ql_strstr(m_Read_Buffer,"CGPR#");
					if(q)
					{
						u8 apn_rcv[30]="",j;
						
						for(i = 0 ; i < 30; i++)
						{
							if(m_Read_Buffer[6 + i] == '#')
							{		
								break;
							}
							else
							{		
								apn_rcv[i] = m_Read_Buffer[6 + i];	
							}
						}
						apn_rcv[i] = '\0';	
						if(i > 25)
						{		
							APP_DEBUG("<-- WRONG SMS APN LENGTH IS GREATER THAN '25' -->\r\n");	
						}
						else if(i == 0)
						{		
							APP_DEBUG("<-- WRONG SMS APN LENGTH IS EQUAL TO 'ZERO' -->\r\n");	
						}
						else
						{
							for(j=0 ; j < i ;j++)
							{
								APN_NAME[j] = apn_rcv[j];
							}
							APN_NAME[j] = '\0';
							
							WRITE_DATA_TO_UFS(5, apn_rcv);	
							APP_DEBUG("<-- APN VALID STORED IN MEMORY -->\r\n");
							Ql_Reset(0);
						}
						APP_DEBUG("\r\n<-- DEVICE APN => %s -->\r\n",APN_NAME);	
					}	
					else
					{
						APP_DEBUG("<-- Not Enter In APN Session  -->\r\n");
					}
					
					q = Ql_strstr(m_Read_Buffer,"$GCON#");
					if(q)
					{
						char msg_1[200];
						Ql_memset(msg_1, 0, 200);
						Ql_sprintf(msg_1,"VERSION-%s,ID=%s,APN=%s,IP=%s,Port=%d,IP1=%s,Port1=%d,ON Time Intrval=%s,OFF Time Intrval=%s\0",VERSION,DEVICE_ID,APN_NAME,m_SrvADDR, m_SrvPort,m_SrvADDR_1,m_SrvPort_1, ST_Interval1, ST_Interval12 );	
						APP_DEBUG("<-- %s -->\r\n", msg_1);
					}
				}          
            }
			break;
		 	
            default:
                break; 
        }
    }
}

static void Callback_UART_Hdlr_Modbus_Port(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{
    u32 rdLen=0;
	int i, k,lenn;
	char my_char[1024]={0};
	s32 ret;
	
	
	bool setting_flag = 0;
    char *q=NULL;
	switch (msg)
    {
		case EVENT_UART_READY_TO_READ:
        {
         
			Ql_memset(m_Read_Buffer, 0x0, sizeof(m_Read_Buffer));
			rdLen = Ql_UART_Read(port, m_Read_Buffer, sizeof(m_Read_Buffer));
			
			APP_DEBUG("\r\n<--PORT 2 SERIAL DATA: %s\r\n",m_Read_Buffer);
			APP_DEBUG("<--LENGTH: %d\r\n",rdLen);      
			if((UART_PORT2 == port) && (uart_flag2 == 1))
			{
				if((rdLen <= 8) && (rdLen != 7))
				{
					APP_DEBUG("\r\n<-- Data Length Equal To or Less Than 8\r\n");   
					break;
				}

				switch(QUERY_RCV_STAGE)
				{
					// read register
					case 1:if(rdLen == 45)  // only for a response
							{
								APP_DEBUG("<-- RESPONSE RCV FOR Query NO. 1: VALID\r\n");   
								CALCULATE_PARAMETER_FROM_QUERY(m_Read_Buffer);
							}
							else if(rdLen == 53) // query + response 
							{
								APP_DEBUG("<-- RESPONSE RCV FOR Query NO. 1: LENGTH NOT MATCH BUT VALID\r\n"); 
								q = m_Read_Buffer;
								q=q+8;
								CALCULATE_PARAMETER_FROM_QUERY(q);
							}
							else
							{
								APP_DEBUG("<-- RESPONSE RCV FOR Query NO. 1: INVALID\r\n");  
								err_flag=1;
							}
						
							comm_flag = 0;
							APP_DEBUG("<-- DATA FLAG = %d\r\n", data_flag);   
							APP_DEBUG("<-- MODBUS_FLAG = %d\r\n", modbus_flag);
							if(data_flag == 1)
							{
								data_flag = 0;
								//GPRS_TCP_Program(1);
								if(socket_flag == 1)
								{
									APP_DEBUG("\r\n socket flag = 1 \r\n");
									APP_DEBUG("\r\n Trying to get gprs onnection \r\n");   
									GPRS_TCP_Program(0);
								}
								else
								{
									APP_DEBUG("\r\n GPRS connection is ok checking for going for socket connection or data sending \r\n");   
									if(SOCKET_IN_WOULD_BLOCK_FLAG == 0)
									{
										APP_DEBUG("\r\n socket not in would block loop \r\n");   
										if(m_SocketConnState > 0)
										{
											APP_DEBUG("\r\n  socket is connected going for data send \r\n");
											GPRS_TCP_Program(1);
										}
										else
										{
											APP_DEBUG("\r\n  socket is not connected going for socket connection \r\n");
											SOC_CONNECT_TO_SERVER();
										}
									}
									else{ APP_DEBUG("\r\n socket in would block loop \r\n");  }
								
								}
								
							}
							uart_flag2 = 0;
							break;
	
					default:break;
							
				}
			}
		}
	}
}

static void Hdlr_RecvNewSMS(u32 nIndex, bool bAutoReply)
{
	s32 iResult = 0,length1 = 0, i=0, j =0 ;
	u32 uMsgRef = 0;
	u8 device_id_rcv[15]="";
	u8 apn_rcv[30]="";
	u8 ip_rcv[15]="";
	u8 ip_rcv2[15]="";
	u8 port_rcv[7]="";
	u8 port_rcv2[7]="";
	u8 time_rcv[7]="";
	u8 time1_rcv[7]="";
	u32 writenLen = 0;
	u32 readenLen = 0;
	s32 position = 0;    
	s32 handle = -1;
	u8  writeBuffer[LENGTH];
	u8 strBuf[LENGTH] = {0};
	char *p=NULL, *q=NULL, *p1=NULL;
	u8 msg_buffer[] = "",ret;
	ST_RIL_SMS_TextInfo *pTextInfo = NULL;
	ST_RIL_SMS_DeliverParam *pDeliverTextInfo = NULL;
	char aPhNum[RIL_SMS_PHONE_NUMBER_MAX_LEN] = {+918087850993};
	char aReplyCon[100];
	bool bResult = FALSE;
    
	Ql_memset(aReplyCon, 0x00, sizeof(aReplyCon));
	
    pTextInfo = Ql_MEM_Alloc(sizeof(ST_RIL_SMS_TextInfo));
    if (NULL == pTextInfo)
    {
        APP_DEBUG("%s/%d:Ql_MEM_Alloc FAIL! size:%u\r\n", sizeof(ST_RIL_SMS_TextInfo), __func__, __LINE__);
        return;
    }
    Ql_memset(pTextInfo, 0x00, sizeof(ST_RIL_SMS_TextInfo));
    iResult = RIL_SMS_ReadSMS_Text(nIndex, LIB_SMS_CHARSET_GSM, pTextInfo);
    if (iResult != RIL_AT_SUCCESS)
    {
        Ql_MEM_Free(pTextInfo);
        APP_DEBUG("Fail to read text SMS[%d], cause:%d\r\n", nIndex, iResult);
        return;
    }        
    
    if ((LIB_SMS_PDU_TYPE_DELIVER != (pTextInfo->type)) || (RIL_SMS_STATUS_TYPE_INVALID == (pTextInfo->status)))
    {
        Ql_MEM_Free(pTextInfo);
        APP_DEBUG("WARNING: NOT a new received SMS.\r\n");    
        return;
    }
    
    pDeliverTextInfo = &((pTextInfo->param).deliverParam);    

    if(TRUE == pDeliverTextInfo->conPres)  //Receive CON-SMS segment
    {
        s8 iBufIdx = 0;
        u8 uSeg = 0;
        u16 uConLen = 0;

        iBufIdx = ConSMSBuf_GetIndex(g_asConSMSBuf,CON_SMS_BUF_MAX_CNT,&(pDeliverTextInfo->con));
        if(-1 == iBufIdx)
        {
            APP_DEBUG("Enter Hdlr_RecvNewSMS,WARNING! ConSMSBuf_GetIndex FAIL! Show this CON-SMS-SEG directly!\r\n");

            APP_DEBUG(
                "status:%u,type:%u,alpha:%u,sca:%s,oa:%s,scts:%s,data length:%u,cp:1,cy:%d,cr:%d,ct:%d,cs:%d\r\n",
                    (pTextInfo->status),
                    (pTextInfo->type),
                    (pDeliverTextInfo->alpha),
                    (pTextInfo->sca),
                    (pDeliverTextInfo->oa),
                    (pDeliverTextInfo->scts),
                    (pDeliverTextInfo->length),
                    pDeliverTextInfo->con.msgType,
                    pDeliverTextInfo->con.msgRef,
                    pDeliverTextInfo->con.msgTot,
                    pDeliverTextInfo->con.msgSeg
            );
            APP_DEBUG("data = %s\r\n",(pDeliverTextInfo->data));

            Ql_MEM_Free(pTextInfo);
        
            return;
        }

        bResult = ConSMSBuf_AddSeg(
                    g_asConSMSBuf,
                    CON_SMS_BUF_MAX_CNT,
                    iBufIdx,
                    &(pDeliverTextInfo->con),
                    (pDeliverTextInfo->data),
                    (pDeliverTextInfo->length)
        );
        if(FALSE == bResult)
        {
            APP_DEBUG("Enter Hdlr_RecvNewSMS,WARNING! ConSMSBuf_AddSeg FAIL! Show this CON-SMS-SEG directly!\r\n");

            APP_DEBUG(
                "status:%u,type:%u,alpha:%u,sca:%s,oa:%s,scts:%s,data length:%u,cp:1,cy:%d,cr:%d,ct:%d,cs:%d\r\n",
                (pTextInfo->status),
                (pTextInfo->type),
                (pDeliverTextInfo->alpha),
                (pTextInfo->sca),
                (pDeliverTextInfo->oa),
                (pDeliverTextInfo->scts),
                (pDeliverTextInfo->length),
                pDeliverTextInfo->con.msgType,
                pDeliverTextInfo->con.msgRef,
                pDeliverTextInfo->con.msgTot,
                pDeliverTextInfo->con.msgSeg
            );
            APP_DEBUG("data = %s\r\n",(pDeliverTextInfo->data));

            Ql_MEM_Free(pTextInfo);
        
            return;
        }

        bResult = ConSMSBuf_IsIntact(
                    g_asConSMSBuf,
                    CON_SMS_BUF_MAX_CNT,
                    iBufIdx,
                    &(pDeliverTextInfo->con)
        );
        if(FALSE == bResult)
        {
            APP_DEBUG(
                "Enter Hdlr_RecvNewSMS,WARNING! ConSMSBuf_IsIntact FAIL! Waiting. cp:1,cy:%d,cr:%d,ct:%d,cs:%d\r\n",
                pDeliverTextInfo->con.msgType,
                pDeliverTextInfo->con.msgRef,
                pDeliverTextInfo->con.msgTot,
                pDeliverTextInfo->con.msgSeg
            );

            Ql_MEM_Free(pTextInfo);

            return;
        }

        //Show the CON-SMS
        APP_DEBUG(
            "status:%u,type:%u,alpha:%u,sca:%s,oa:%s,scts:%s",
            (pTextInfo->status),
            (pTextInfo->type),
            (pDeliverTextInfo->alpha),
            (pTextInfo->sca),
            (pDeliverTextInfo->oa),
            (pDeliverTextInfo->scts)
        );
        
        uConLen = 0;
        for(uSeg = 1; uSeg <= pDeliverTextInfo->con.msgTot; uSeg++)
        {
            uConLen += g_asConSMSBuf[iBufIdx].asSeg[uSeg-1].uLen;
        }

        APP_DEBUG(",data length:%u",uConLen);
        APP_DEBUG("\r\n"); //Print CR LF

        for(uSeg = 1; uSeg <= pDeliverTextInfo->con.msgTot; uSeg++)
        {
            APP_DEBUG("data = %s ,len = %d",
                g_asConSMSBuf[iBufIdx].asSeg[uSeg-1].aData,
                g_asConSMSBuf[iBufIdx].asSeg[uSeg-1].uLen
            );
        }

        APP_DEBUG("\r\n"); //Print CR LF

        //Reset CON-SMS context
        bResult = ConSMSBuf_ResetCtx(g_asConSMSBuf,CON_SMS_BUF_MAX_CNT,iBufIdx);
        if(FALSE == bResult)
        {
            APP_DEBUG("Enter Hdlr_RecvNewSMS,WARNING! ConSMSBuf_ResetCtx FAIL! iBufIdx:%d\r\n",iBufIdx);
        }

        Ql_MEM_Free(pTextInfo);
        
        return;
    }
    
    APP_DEBUG("<-- RIL_SMS_ReadSMS_Text OK. eCharSet:LIB_SMS_CHARSET_GSM,nIndex:%u -->\r\n",nIndex);
    APP_DEBUG("status:%u,type:%u,alpha:%u,sca:%s,oa:%s,scts:%s,data length:%u\r\n",
        pTextInfo->status,
        pTextInfo->type,
        pDeliverTextInfo->alpha,
        pTextInfo->sca,
        pDeliverTextInfo->oa,
        pDeliverTextInfo->scts,
        pDeliverTextInfo->length);
    APP_DEBUG("data = %s\r\n",(pDeliverTextInfo->data));
	
	length1 = Ql_strlen(pDeliverTextInfo->data);
	for(i=0; i < length1 ; i++)
	{
		msg_buffer[i] = (pDeliverTextInfo->data)[i];
	}
    
	APP_DEBUG("%d\r\n", length1);
	APP_DEBUG("data1 = %s\r\n",msg_buffer);
	
	char FIRST_CONDITION_CLEAR_FOR_SMS=0;
	
	p = Ql_strstr(msg_buffer,"*CON#");
	APP_DEBUG("\r\n<---p=%x--->\r\n",p);
	
	if(p)
	{
		q = Ql_strstr(msg_buffer,"CVID#");
		APP_DEBUG("\r\n<---q=%x--->\r\n",q);
		if(q)
		{
			length1 = 10;
			for(i = 0 ; i < 20; i++)
			{
				if(msg_buffer[length1 + i] == '#')
				{		
					break;
				}
				else
				{		
					device_id_rcv[i] = msg_buffer[length1 + i];
				}
			}
			device_id_rcv[i] = '\0';
			//APP_DEBUG("\r\n<---i=%d--->\r\n",i);			
			if(i > 8)
			{			
				Ql_sprintf(aReplyCon,"%s%s\0",msg_buffer,"ID length cross '7'");
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
				APP_DEBUG("<-- WRONG SMS LENGTH IS GREATER THAN 8 -->\r\n");
			}
			else if((device_id_rcv[0] != '8') || (device_id_rcv[1] != '5'))
			{		
				Ql_sprintf(aReplyCon,"%s%s\0",msg_buffer,"ID Not Start with 80");
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
				APP_DEBUG("<-- WRONG SMS ID NOT START WITH '80' -->\r\n");
			}
			else if(i < 8)
			{
				Ql_sprintf(aReplyCon,"%s%s\0",msg_buffer,"ID lenght less than 7");
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
				APP_DEBUG("<-- WRONG SMS LENGTH IS LESS THAN 8 -->\r\n");
			}
			else
			{
				APP_DEBUG("<-- VALID ID -->\r\n");
				for(length1=0 ; length1 < i ;length1++)
				{
					DEVICE_ID[length1] = device_id_rcv[length1];
				}
				WRITE_DATA_TO_UFS(4, DEVICE_ID);	
				Ql_sprintf(aReplyCon,"%s%s\0",msg_buffer,"OK");
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
			}
			FIRST_CONDITION_CLEAR_FOR_SMS = 1;
			APP_DEBUG("<-- DEVICE ID => %s -->\r\n",DEVICE_ID);	
		}
		
		q = Ql_strstr(msg_buffer,"CIP1#");
		if(q)
		{
			length1 = 10;
			for(i = 0 ; i < 30; i++)
			{
				if(msg_buffer[length1 + i] == '#')
				{		
					break;
				}
				else
				{		
					ip_rcv[i] = msg_buffer[length1 + i];
				}
			}
			ip_rcv[i] = '\0';
			length1 =length1+i+1;
			for(j=0 ; j<7; j++)
			{
				if(msg_buffer[length1 + j] == '#')
				{break;}
				else if(msg_buffer[length1 + j] == 0)
				{j = j*7; break;}
				else if(msg_buffer[length1 + j] == 10)
				{j=10; break;}					
				port_rcv[j] = msg_buffer[length1 + j];
			}
			port_rcv[j] = '\0'; 
			
			if(i >= 16)
			{		
				Ql_sprintf(aReplyCon,"%s\0","IP length cross '15'");
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
				APP_DEBUG("<-- WRONG SMS IP INVALID LENGTH CROSS '15' -->\r\n");
			}
			else if((j == 0)||(j >= 6) )
			{
				if(j == 0)
				{ Ql_sprintf(aReplyCon,"%s\0","PORT NOT PRESENT");}
				else{Ql_sprintf(aReplyCon,"%s\0","(PORT LENGTH MORE THAN 5 DIGIT)/(END OF THE STRING MISSING)/ (PORT INVALID)");}
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);	
				APP_DEBUG("<-- WRONG SMS PORT INVALID -->\r\n"); 
			}
			else
			{
				APP_DEBUG("<-- VALID IP & PORT RECIEVED\r\n");
				for(length1=0 ; length1 < i ;length1++)
				{
					m_SrvADDR[length1] = ip_rcv[length1];
				}
				m_SrvADDR[length1] = '\0';
				WRITE_DATA_TO_UFS(6, ip_rcv);	
				
				for(length1=0 ; length1 < j ;length1++)
				{
					m_SrvPort1[length1] = port_rcv[length1];
				}
				m_SrvPort1[length1] = '\0';
				WRITE_DATA_TO_UFS(7, port_rcv);	
				m_SrvPort = PORT_VALUE_CAL(port_rcv);
				APP_DEBUG("\r\nSTORED IP=> %s & PORT=> %d",m_SrvADDR, m_SrvPort);
				
				Ql_sprintf(aReplyCon,"MSG RCV IP=%s, PORT=%d\0", m_SrvADDR, m_SrvPort); 
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
				Ql_Reset(0);
			}
			FIRST_CONDITION_CLEAR_FOR_SMS = 1;
			APP_DEBUG("<-- DEVICE IP=%s, PORT=%s\r\n",m_SrvADDR, m_SrvPort);		
		}
		
		q = Ql_strstr(msg_buffer,"CIP2#");
		if(q)
		{
			length1 = 10;
			for(i = 0 ; i < 30; i++)
			{
				if(msg_buffer[length1 + i] == '#')
				{		
					break;
				}
				else
				{		
					ip_rcv2[i] = msg_buffer[length1 + i];
				}
			}
			ip_rcv2[i] = '\0';
			length1 = length1+i+1;
			for(j=0 ; j<7; j++)
			{
				if(msg_buffer[length1 + j] == '#')
				{break;}
				else if(msg_buffer[length1 + j] == 0)
				{j = j*7; break;}
				else if(msg_buffer[length1 + j] == 10)
				{j=10; break;}
				port_rcv2[j] = msg_buffer[length1 + j];
			}
			port_rcv2[j] = '\0';
			
			if(i >= 16)
			{		
				Ql_sprintf(aReplyCon,"%s%s\0","IP length cross '15'");
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
				APP_DEBUG("<-- WRONG SMS INVALID IP LENGTH CROSS '15' -->\r\n");
			}
			else if((j == 0)||(j >= 6) )
			{
				if(j == 0)
				{ Ql_sprintf(aReplyCon,"%s\0","PORT NOT PRESENT");}
				else{Ql_sprintf(aReplyCon,"%s\0","(PORT LENGTH MORE THAN 5 DIGIT)/(END OF THE STRING MISSING)/ (PORT INVALID)");}
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);	
				APP_DEBUG("<-- WRONG SMS PORT INVALID -->\r\n");			 
			}
			else
			{
				
				APP_DEBUG("<-- VALID IP & PORT RECIEVED\r\n");
				for(length1=0 ; length1 < i ;length1++)
				{
					m_SrvADDR_1[length1] = ip_rcv2[length1];
				}
				m_SrvADDR_1[length1] = '\0';
				WRITE_DATA_TO_UFS(10, ip_rcv2);	
				
				for(length1=0 ; length1 < j ;length1++)
				{
					m_SrvPort12[length1] = port_rcv2[length1];
				}
				m_SrvPort12[length1] = '\0';
				WRITE_DATA_TO_UFS(11, port_rcv2);	
				m_SrvPort_1 = PORT_VALUE_CAL(port_rcv2);
				APP_DEBUG("\r\nSTORED IP2=> %s & PORT2=> %d",m_SrvADDR_1, m_SrvPort_1);
				Ql_sprintf(aReplyCon,"MSG RCV IP2=%s, PORT2=%d\0", m_SrvADDR_1, m_SrvPort_1);
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
				Ql_Reset(0);
			}
			FIRST_CONDITION_CLEAR_FOR_SMS = 1;
			APP_DEBUG("<-- DEVICE IP2=%s, PORT2=%s\r\n",m_SrvADDR_1, m_SrvPort_1);
		}
		
		q = Ql_strstr(msg_buffer,"CGPR#");
		if(q)
		{
			length1 = 10;
			for(i = 0 ; i < 30; i++)
			{
				if(msg_buffer[length1 + i] == '#')
				{		
					break;
				}
				else
				{		
					apn_rcv[i] = msg_buffer[length1 + i];	
				}
			}
			apn_rcv[i] = '\0';
			APP_DEBUG("<-- i=%d -->\r\n",i);			
			if(i > 25)
			{		
				Ql_sprintf(aReplyCon,"%s\0","APN length cross '25'");
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
				APP_DEBUG("<-- WRONG SMS APN LENGTH IS GREATER THAN '25' -->\r\n");	
			}
			else if(i == 0)
			{		
				Ql_sprintf(aReplyCon,"%s\0","APN LENGTH IS '0'");
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
				APP_DEBUG("<-- WRONG SMS APN LENGTH IS EQUAL TO 'ZERO' -->\r\n");	
			}
			else
			{
				for(length1=0 ; length1 < i ;length1++)
				{
					APN_NAME[length1] = apn_rcv[length1];
				}
				APN_NAME[length1] = '\0';
				
				WRITE_DATA_TO_UFS(5, apn_rcv);	
				Ql_sprintf(aReplyCon,"%s%s\0",msg_buffer,"OK");
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
				APP_DEBUG("<-- APN VALID STORED IN MEMORY -->\r\n");
				Ql_Reset(0);
			}
			APP_DEBUG("\r\n<-- DEVICE APN => %s -->\r\n",APN_NAME);	
			FIRST_CONDITION_CLEAR_FOR_SMS = 1;
		}
		
		q = Ql_strstr(msg_buffer,"CMPI#");
		if(q)
		{
			length1 = 10;
			for(i = 0 ; i < 7; i++)
			{
				if(msg_buffer[length1 + i] == '#')
				{		
					break;
				}
				else
				{		
					time_rcv[i] = msg_buffer[length1 + i];
				}
			}			
			time_rcv[i] = '\0';
			APP_DEBUG("\r\n<---i=%d--->\r\n",i);
			length1 =16;
			for(j=0 ; j<5; j++)
			{
				if(msg_buffer[length1 + j] == '#')
				{		
					break;
				}
				if(msg_buffer[length1 + j] == '\0')
				{		
					break;
				}
				time1_rcv[j] = msg_buffer[length1 + j];
			}
			time1_rcv[j] = '\0';
			
			if((i > 5) || (j > 5))
			{		
				Ql_sprintf(aReplyCon,"%s\0","INTERVAL LENNGTH CROSS '5' CHECK FORMAT");
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
				APP_DEBUG("<-- WRONG SMS LENGTH NOT MATCH -->\r\n");
			}
			else if((time_rcv[3] > '5') || (time1_rcv[3] > '5'))
			{				
				Ql_sprintf(aReplyCon,"%s\0","SECONDS ARE GREATER TAHN 59 SEC");
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
				APP_DEBUG("<-- WRONG SMS SECONDS ARE GREATER THAN 59 SEC-->\r\n");
			}
			else
			{
				for(length1=0 ; length1 < i ;length1++)
				{
					ST_Interval1[length1] = time_rcv[length1];
					ST_Interval12[length1] = time1_rcv[length1];
				}
				ST_Interval1[length1] = '\0';
				ST_Interval12[length1] = '\0';
				
				WRITE_DATA_TO_UFS(8, time_rcv);	
				WRITE_DATA_TO_UFS(9, time1_rcv);	
				ST_Interval = CONVERT_TIME_BUFFER_TO_SECONDS_VALUE(ST_Interval1);
				ST_Interval2 = CONVERT_TIME_BUFFER_TO_SECONDS_VALUE(ST_Interval12);
				Ql_sprintf(aReplyCon,"TIME CHANGE MESSAGE RCV, TIME INTERVAL1=> %s\0",ST_Interval1);
				APP_DEBUG("<-- TIME INTERVAL IN ARRAY=> %s, TIME INTERVAL IN mSEC=> %ld -->\r\n",ST_Interval1, ST_Interval);
				Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
				SEND_SMS(1,aReplyCon,aPhNum);
				Ql_Reset(0);
			}
			FIRST_CONDITION_CLEAR_FOR_SMS = 1;
			APP_DEBUG("<-- TIME INTERVAL IN ARRAY=> %s, TIME INTERVAL IN mSEC=> %ld -->\r\n",ST_Interval1, ST_Interval);
		}
		
		q = Ql_strstr(msg_buffer,"DCH#");
		if(q)
		{
			APP_DEBUG("<-- ENTER IN DATA SEND SESSION\r\n");		
			GPRS_TCP_Program(0);	
		}
		
		q = Ql_strstr(msg_buffer,"fota");
		if(q)
		{
			ret = Ql_Timer_Stop(Stack_timer);
			if(ret < 0)
			{
				  APP_DEBUG("\r\n<--failed!! stack timer Ql_Timer_Stop ret=%d-->\r\n",ret);           
			}
			APP_DEBUG("\r\n<--stack timer Ql_Timer_Stop(ID=%d,) ret=%d-->\r\n",Stack_timer,ret);   
			Ql_sprintf(aReplyCon,"%s%s\0",msg_buffer,"OK");
			Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
			SEND_SMS(1,aReplyCon,aPhNum);
			ST_GprsConfig apnCfg;
			Ql_memcpy(apnCfg.apnName  , APN_NAME  , Ql_strlen(APN_NAME));
			Ql_memcpy(apnCfg.apnUserId, APN_USERID, Ql_strlen(APN_USERID));
			Ql_memcpy(apnCfg.apnPasswd, APN_PASSWD, Ql_strlen(APN_PASSWD));

			Ql_memset(m_URL_Buffer, 0, URL_LEN);
			
			//http://hostname:port/filePath/fileName
			//Ql_sprintf(m_URL_Buffer, "%s%s",APP_BIN_URL,m_Read_Buffer_fota);
			APP_DEBUG("<-- URL:%s-->\r\n",APP_BIN_URL);
			Ql_FOTA_StartUpgrade(APP_BIN_URL, &apnCfg, NULL);
			fota_flag = 1;
			FIRST_CONDITION_CLEAR_FOR_SMS = 1;
		}
	}
	else
	{
		p = Ql_strstr(msg_buffer,"*GET#GCON#");
		if(p)
		{	
			Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
			APP_DEBUG("<-- Replying SMS... -->\r\n");
			Ql_sprintf(aReplyCon,"VERSION-%s,ID=%s,APN=%s,IP=%s,Port=%d,IP1=%s,Port1=%d,ON Time Intrval=%s,OFF Time Intrval=%s\0",VERSION,DEVICE_ID,APN_NAME,m_SrvADDR, m_SrvPort,m_SrvADDR_1,m_SrvPort_1, ST_Interval1, ST_Interval12 );
			APP_DEBUG("%s\r\n",aReplyCon);
			SEND_SMS(1,aReplyCon,aPhNum);
			FIRST_CONDITION_CLEAR_FOR_SMS = 1;
		}
		
		p = Ql_strstr(msg_buffer,"*GET#GSTATUS#");// Command-> Send Status Details By SMS
		if(p)
		{	
			u32 signal_strength, ber; 
			
			ret = RIL_NW_GetSignalQuality(&signal_strength, &ber);
			APP_DEBUG("<-- Signal strength:%02d, BER:%d -->\r\n", signal_strength, ber);
			signal_strength= (signal_strength * 100 / 32);
			
			Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
			APP_DEBUG("<-- Replying SMS... -->\r\n");
			Ql_sprintf(aReplyCon,"VERSION-%s,ID=%s,GPRS_Status= %d, signal_strength=%d,socket=%d,Module_State=%d,\0", VERSION, DEVICE_ID,GPRS_flag,  signal_strength, m_SocketConnState, module_state );
			APP_DEBUG("<-- %s\r\n",aReplyCon);
			SEND_SMS(1,aReplyCon,aPhNum);
			FIRST_CONDITION_CLEAR_FOR_SMS = 1;
		}
	}
		
	if(FIRST_CONDITION_CLEAR_FOR_SMS == 0)
	{
		Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
		SEND_SMS(1,"PLEASE CHECK FORMAT MESSAGE INVALID\0",aPhNum);
	}
		
    Ql_strcpy(aPhNum, pDeliverTextInfo->oa);
    Ql_MEM_Free(pTextInfo);
    return;
}


static void GPRS_TCP_Program(char case_value)
{
    s32 ret;
    //s32 pdpCntxtId;
    ST_PDPContxt_Callback callback_gprs_func = {
        Callback_GPRS_Actived,
        Callback_GPRS_Deactived
    };


	if(socket_flag == 1)
	{
		
		//1. Register GPRS callback
		pdpCntxtId = Ql_GPRS_GetPDPContextId();
		if (GPRS_PDP_ERROR == pdpCntxtId)
		{
			APP_DEBUG("No PDP context is available\r\n");
			ret=Ql_GPRS_Deactivate(pdpCntxtId);
			APP_DEBUG("<-- Deactivated GPRS1, cause=%d -->\r\n\r\n", ret);
			ret = RIL_NW_ClosePDPContext();
			APP_DEBUG("<-- Close PDP context, ret=%d -->\r\n", ret);
			return ret;
		}
		else
		{
			//Ql_Reset(0);
		}
		ret = Ql_GPRS_Register(pdpCntxtId, &callback_gprs_func, NULL);
		if (GPRS_PDP_SUCCESS == ret)
		{
			APP_DEBUG("<-- Register GPRS callback function -->\r\n");
		}else{
			APP_DEBUG("<-- Fail to register GPRS, cause=%d. -->\r\n", ret);
			return;
		}

		//2. Configure PDP
		ret = Ql_GPRS_Config(pdpCntxtId, &m_GprsConfig);
		if (GPRS_PDP_SUCCESS == ret)
		{
			APP_DEBUG("<-- Configure PDP context -->\r\n");
		}else{
			APP_DEBUG("<-- Fail to configure GPRS PDP, cause=%d. -->\r\n", ret);
			return;
		}

		//3. Activate GPRS PDP context
		APP_DEBUG("<-- Activating GPRS... -->\r\n");
		ret = Ql_GPRS_Activate(pdpCntxtId);
		if (ret == GPRS_PDP_SUCCESS)
		{
			m_GprsActState = 1;
			APP_DEBUG("<-- Activate GPRS successfully. -->\r\n\r\n");
		}
        if(ret == GPRS_PDP_WOULDBLOCK)
        {
            
			APP_DEBUG("<-- Waiting for GPRS ACTIVED. -->\r\n\r\n");
            return;
         }else{
			APP_DEBUG("<-- Fail to activate GPRS, cause=%d. -->\r\n\r\n", ret);
			return;
		}
		
	}
    //7. Send data to socket
	if(m_SocketConnState > 0)// checking for socket connection
	{
		char data_packet_send_to_server[300];
		s32  dataLen,i;
		u32 signal_strength, ber; 
		
		Ql_memset(data_packet_send_to_server, 0x00, sizeof(data_packet_send_to_server));
		
		if((err_flag == 0) && (comm_flag == 0))
		{
			Ql_sprintf(data_packet_send_to_server,"%s%s#DATA#%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d#*", START_OF_THE_STRING,DEVICE_ID,PARAMETER_01,PARAMETER_02,PARAMETER_03,PARAMETER_04,PARAMETER_05,PARAMETER_06,PARAMETER_07,PARAMETER_08,PARAMETER_09,PARAMETER_10,PARAMETER_11,PARAMETER_12,PARAMETER_13,PARAMETER_14,PARAMETER_15,PARAMETER_16,PARAMETER_17,PARAMETER_18,PARAMETER_19,PARAMETER_20);
		}
		else if(comm_flag == 1)
		{
			comm_flag = 0;
			Ql_sprintf(data_packet_send_to_server,"%s%s#NCWM#*",START_OF_THE_STRING, DEVICE_ID);
		}
		else
		{
			err_flag = 0;
			Ql_sprintf(data_packet_send_to_server,"%s%s#CE#*",START_OF_THE_STRING, DEVICE_ID);
		}
		
		dataLen = strlen(data_packet_send_to_server);
	
		APP_DEBUG("<-- Sending data(len=%d): %s-->\r\n", dataLen, data_packet_send_to_server);
		ret = Ql_SOC_Send(m_SocketId, (u8*)data_packet_send_to_server, dataLen);

		if (ret == dataLen)
		{
			APP_DEBUG("<-- return value = %d\r\n", ret);
			APP_DEBUG("<-- Send socket data successfully. --> \r\n");
			send_flag = 1; 
			wd_flag1 = 1 ;
			
			if(ret == dataLen)
			{
				data_send_to_server_flag =1;
				APP_DEBUG(" data_send_to_server_flag = %d\r\n",data_send_to_server_flag);
			}
        }
		else
		{
            APP_DEBUG("<-- Fail to send socket data. --> \r\n");
            Ql_SOC_Close(m_SocketId);
			ret = Ql_GPRS_DeactivateEx(pdpCntxtId, TRUE);
			APP_DEBUG("<-- Deactivated GPRS, cause=%d -->\r\n\r\n", ret);
			ret=Ql_GPRS_Deactivate(pdpCntxtId);
			APP_DEBUG("<-- Deactivated GPRS1, cause=%d -->\r\n\r\n", ret);
			ret = RIL_NW_ClosePDPContext();
			APP_DEBUG("<-- Close PDP context, ret=%d -->\r\n", ret);
			socket_flag = 1;
			GPRS_Connection_Flag = 0;
            //return;
        }
		APP_DEBUG("<-- Finish -->\r\n");
	}
	
}

//
//
// This callback function is invoked when socket data arrives.
// The program should call Ql_SOC_Recv to read all data out of the socket buffer.
//
static void Callback_Socket_Read(s32 socketId, s32 errCode, void* customParam )
{
    s32 ret;
    s32 offset = 0;
	char *q=NULL;
	
    if (errCode)
    {
        APP_DEBUG("<-- Close socket -->\r\n");
        Ql_SOC_Close(socketId);
        m_SocketId = -1;
		socket_flag = 1;
        return;
    }

    Ql_memset(m_SocketRcvBuf, 0, SOC_RECV_BUFFER_LEN);
    do
    {
        ret = Ql_SOC_Recv(socketId, m_SocketRcvBuf + offset, SOC_RECV_BUFFER_LEN - offset);
        if((ret < SOC_SUCCESS) && (ret != SOC_WOULDBLOCK))
        {
            APP_DEBUG("<-- Fail to receive data, cause=%d.-->\r\n",ret);
            APP_DEBUG("<-- Close socket.-->\r\n");
            Ql_SOC_Close(socketId);
            m_SocketId = -1;
			socket_flag = 1;
            break;
        }
        else if(SOC_WOULDBLOCK == ret)  // Read finish
        {
            APP_DEBUG("<-- Receive data from server,len(%d):%s\r\n", offset, m_SocketRcvBuf);
			SYSTEM_WATCHDOG_FLAG = 1; 
			if((m_SocketRcvBuf[0] == '$') && (m_SocketRcvBuf[offset-1] == '*'))
			{
				q = Ql_strstr(m_SocketRcvBuf, DEVICE_ID );
				if(q)
				{
					char *p=NULL;
					
					//APP_DEBUG("<-- Close socket.-->\r\n");
					//Ql_SOC_Close(socketId);
					//socket_flag = 1;
					
					p = Ql_strstr(m_SocketRcvBuf,"NOUPDATE");
					if(p)
					{
						APP_DEBUG("<-- NO DATA PRESENT AT SERVER OF OVER HEAD TANK\r\n");
					}
					else
					{
						p = Ql_strstr(m_SocketRcvBuf,"NCWM");
						if(p)
						{
							APP_DEBUG("<-- NO COMMUNICATION WITH OVER HEAD TANK CONTROLLER\r\n");
						}
						else
						{
							p = Ql_strstr(m_SocketRcvBuf,"CE");
							if(p)
							{
								APP_DEBUG("<-- NO COMMUNICATION WITH OVER HEAD TANK CONTROLLER\r\n");
							}
							else
							{
								p = Ql_strstr(m_SocketRcvBuf,"DATA");
								if(p)
								{
									APP_DEBUG("<-- DATA RCV FROM\r\n");
									p = p+5;
									
									SEPARATE_MODBUS_DATA(p);
								}
								else
								{
									APP_DEBUG("<-- NO VALID DATA COME FROM SERVER\r\n");
								}
							}
						}
					}
				}
			}
            break;
        }
        else // Continue to read...
        {
            if (SOC_RECV_BUFFER_LEN == offset)  // buffer if full
            {
                APP_DEBUG("<-- Receive data from server,len(%d):%s\r\n", offset, m_SocketRcvBuf);
                Ql_memset(m_SocketRcvBuf, 0, SOC_RECV_BUFFER_LEN);
                offset = 0;
            }else{
                offset += ret;
            }
            continue;
        }
    } while (TRUE);
}

void Callback_Socket_Connect(s32 socketId, s32 errCode, void* customParam )
{
    if (errCode == SOC_SUCCESS)
    {
      

        APP_DEBUG("<--Callback: socket connect successfully.-->\r\n");
        SOCKET_IN_WOULD_BLOCK_FLAG = 0;
        m_SocketConnState = 1;
    }else
    {
        APP_DEBUG("<--Callback: socket connect failure,(socketId=%d),errCode=%d-->\r\n",socketId,errCode);
		if(SERVER2_SETTING_SELECTION_FLAG == 0)                              //server selection considering fail condition of server
		{
			SERVER2_SETTING_SELECTION_FLAG = 1;
		}
		else if(SERVER2_SETTING_SELECTION_FLAG == 1){
			SERVER2_SETTING_SELECTION_FLAG = 0;
		}
		APP_DEBUG("<-- SERVER2_SETTING_SELECTION_FLAG = %d-->\r\n", SERVER2_SETTING_SELECTION_FLAG);
        m_SocketConnState = 0;
		SOCKET_IN_WOULD_BLOCK_FLAG = 0;
		
        Ql_SOC_Close(socketId);
    }
}


void Callback_GPRS_Actived(u8 contexId, s32 errCode, void* customParam)
{
    s32 ret;
	u8 m_ipAddress[4], m_ipAddress_1[4] ; 
	Ql_memset(m_ipAddress,0,5);
	Ql_memset(m_ipAddress_1,0,5);
    
        ST_SOC_Callback callback_soc_func = {
        Callback_Socket_Connect,
        Callback_Socket_Close,
        NULL,
        Callback_Socket_Read,    
        Callback_Socket_Write
    };
        
	
    if(errCode == SOC_SUCCESS)
    {
        
        m_GprsActState = 1;
        socket_flag = 0;
		SOCKET_IN_WOULD_BLOCK_FLAG = 0;
        APP_DEBUG("<--CallBack: active GPRS successfully.-->\r\n");
        
		//4. Register Socket callback
		ret = Ql_SOC_Register(callback_soc_func, NULL);
		if (SOC_SUCCESS == ret)
		{
			APP_DEBUG("<-- Register socket callback function -->\r\n");
		}else{
			APP_DEBUG("<-- Fail to register socket callback, cause=%d. -->\r\n", ret);
			return;
		}
		//socket_flag = 0;
		APP_DEBUG("<-- enter socket\r\n");
		//5. Create socket
		m_SocketId = Ql_SOC_Create(pdpCntxtId, SOC_TYPE_TCP);
		if (m_SocketId >= 0)
		{
			APP_DEBUG("<-- Create socket successfully, socket id=%d. -->\r\n", m_SocketId);
		}else{
			APP_DEBUG("<-- Fail to create socket, cause=%d. -->\r\n", m_SocketId);
			return;
		}		
	
		
		//6. Connect to server
		if( SERVER2_SETTING_SELECTION_FLAG == 0)
		{
			ret = Ql_IpHelper_ConvertIpAddr(m_SrvADDR, (u32 *)m_ipAddress);
			if (SOC_SUCCESS == ret) // ip address is xxx.xxx.xxx.xxx
			{
				APP_DEBUG("<-- Convert Ip Address successfully,m_ipaddress=%d,%d,%d,%d -->\r\n",m_ipAddress[0],m_ipAddress[1],m_ipAddress[2],m_ipAddress[3]);
			}else{
				APP_DEBUG("<-- Fail to convert IP Address --> \r\n");
				return;
			}

			//6.2 Connect to server
			APP_DEBUG("<-- Connecting to server(IP:%d.%d.%d.%d, port:%d)... -->\r\n", m_ipAddress[0],m_ipAddress[1],m_ipAddress[2],m_ipAddress[3], m_SrvPort);
			ret = Ql_SOC_Connect(m_SocketId,(u32) m_ipAddress, m_SrvPort);
			if (SOC_SUCCESS == ret)
			{
				m_SocketConnState = 1;
				APP_DEBUG("<-- Connect to server successfully -->\r\n");
				SOCKET_IN_WOULD_BLOCK_FLAG = 0;
			}
			else
			{
				if(SOC_WOULDBLOCK == ret)
				{
					SOCKET_IN_WOULD_BLOCK_FLAG = 1;
					APP_DEBUG("<-- Waiting to SOC Connect : WOULD_BLOCK -->\r\n");                   
				    return;
				}
				APP_DEBUG("<-- Fail to connect server 1, cause=%d -->\r\n", ret);
				APP_DEBUG("<-- Close socket.-->\r\n");
				Ql_SOC_Close(m_SocketId);
				m_SocketId = -1;
				SERVER2_SETTING_SELECTION_FLAG = 1;
				return;
			}
		}
		else
		{
			ret = Ql_IpHelper_ConvertIpAddr(m_SrvADDR_1, (u32 *)m_ipAddress_1);
			if (SOC_SUCCESS == ret) // ip address is xxx.xxx.xxx.xxx
			{
				APP_DEBUG("<-- Convert Ip Address successfully,m_ipaddress=%d,%d,%d,%d -->\r\n",m_ipAddress_1[0],m_ipAddress_1[1],m_ipAddress_1[2],m_ipAddress_1[3]);
			}else{
				APP_DEBUG("<-- Fail to convert IP Address --> \r\n");
				return;
			}
			//6.2 Connect to server
			APP_DEBUG("<-- Connecting to server(IP:%d.%d.%d.%d, port:%d)... -->\r\n", m_ipAddress_1[0],m_ipAddress_1[1],m_ipAddress_1[2],m_ipAddress_1[3], m_SrvPort_1);
			ret = Ql_SOC_Connect(m_SocketId,(u32) m_ipAddress_1, m_SrvPort_1);
			if (SOC_SUCCESS == ret)
			{
				m_SocketConnState = 1;
				SOCKET_IN_WOULD_BLOCK_FLAG = 0;
				APP_DEBUG("<-- Connect to server successfully -->\r\n");
			}
			else
			{
				if(SOC_WOULDBLOCK == ret)
				{
					SOCKET_IN_WOULD_BLOCK_FLAG = 1;
					APP_DEBUG("<-- Waiting to SOC Connect : WOULD_BLOCK -->\r\n");                   
				   return;
				}
				
				APP_DEBUG("<-- Fail to connect to server 2, cause=%d -->\r\n", ret);
				APP_DEBUG("<-- Close socket.-->\r\n");
				Ql_SOC_Close(m_SocketId);
				m_SocketId = -1;
				SERVER2_SETTING_SELECTION_FLAG = 0;
				return;
			}
		
		}
		
    }
	else
    {
        APP_DEBUG("<--CallBack: active GPRS successfully,errCode=%d-->\r\n",errCode);
    }      
}


static void Callback_GPRS_Deactived(u8 contextId, s32 errCode, void* customParam )
{
    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: deactivated GPRS successfully.-->\r\n"); 
    }else{
        APP_DEBUG("<--CallBack: fail to deactivate GPRS, cause=%d)-->\r\n", errCode); 
    }
    if (1 == m_GprsActState)
    {
        m_GprsActState = 0;
        APP_DEBUG("<-- GPRS drops down -->\r\n"); 
		GPRS_flag = 0;
    }
}
//
//
// This callback function is invoked when the socket connection is disconnected by server or network.
//
static void Callback_Socket_Close(s32 socketId, s32 errCode, void* customParam )
{
    if (errCode == SOC_SUCCESS)
    {
        APP_DEBUG("<--CallBack: close socket successfully.-->\r\n"); 
    }
    else if(errCode == SOC_BEARER_FAIL)
    {   
        APP_DEBUG("<--CallBack: fail to close socket,(socketId=%d,error_cause=%d)-->\r\n", socketId, errCode); 
    }else{
        APP_DEBUG("<--CallBack: close socket failure,(socketId=%d,error_cause=%d)-->\r\n", socketId, errCode); 
    }
    if (0 < m_SocketConnState)
    {
        APP_DEBUG("<-- Socket connection is disconnected -->\r\n"); 
        APP_DEBUG("<-- Close socket at module side -->\r\n"); 
        Ql_SOC_Close(socketId);
        m_SocketConnState = 0;
    }
}

//
// This callback function is invoked in the following case:
// The return value is less than the data length to send when calling Ql_SOC_Send(), which indicates
// the socket buffer is full. Application should stop sending socket data till this callback function
// is invoked, which indicates application can continue to send data to socket.
static void Callback_Socket_Write(s32 socketId, s32 errCode, void* customParam)
{
    if (errCode < 0)
    {
        APP_DEBUG("<-- Socket error(error code:%d), close socket.-->\r\n", errCode);
        Ql_SOC_Close(socketId);
		m_SocketId = -1;        
    }else{
        APP_DEBUG("<-- You can continue to send data to socket -->\r\n");
    }
}

//This function is used for making socket connection with server

void SOC_CONNECT_TO_SERVER()
{
	s32 ret;
	u8 m_ipAddress[4], m_ipAddress_1[4] ; 
	Ql_memset(m_ipAddress,0,5);
	Ql_memset(m_ipAddress_1,0,5);
	SOCKET_IN_WOULD_BLOCK_FLAG = 0;
	
	ST_SOC_Callback callback_soc_func = {
        Callback_Socket_Connect,
        Callback_Socket_Close,
        NULL,
        Callback_Socket_Read,    
        Callback_Socket_Write
    };
	
	ret = Ql_SOC_Register(callback_soc_func, NULL);
	if (SOC_SUCCESS == ret)
	{
		APP_DEBUG("<-- Register socket callback function -->\r\n");
	}else{
		APP_DEBUG("<-- Fail to register socket callback, cause=%d. -->\r\n", ret);
		return;
	}
	//socket_flag = 0;
	APP_DEBUG("<-- enter socket\r\n");
	//5. Create socket
	m_SocketId = Ql_SOC_Create(pdpCntxtId, SOC_TYPE_TCP);
	if (m_SocketId >= 0)
	{
		APP_DEBUG("<-- Create socket successfully, socket id=%d. -->\r\n", m_SocketId);
	}else{
		APP_DEBUG("<-- Fail to create socket, cause=%d. -->\r\n", m_SocketId);
		return;
	}
	
	
	if( SERVER2_SETTING_SELECTION_FLAG == 0)// If server flag is zero going to connect server 1, else going to connect server 2
	{
		ret = Ql_IpHelper_ConvertIpAddr(m_SrvADDR, (u32 *)m_ipAddress);
		if (SOC_SUCCESS == ret) // ip address is xxx.xxx.xxx.xxx
		{
			APP_DEBUG("<-- Convert Ip Address successfully,m_ipaddress=%d,%d,%d,%d -->\r\n",m_ipAddress[0],m_ipAddress[1],m_ipAddress[2],m_ipAddress[3]);
		}else{
			APP_DEBUG("<-- Fail to convert IP Address --> \r\n");
			return;
		}

		//6.2 Connect to server
		APP_DEBUG("<-- Connecting to server(IP:%d.%d.%d.%d, port:%d)... -->\r\n", m_ipAddress[0],m_ipAddress[1],m_ipAddress[2],m_ipAddress[3], m_SrvPort);
		ret = Ql_SOC_Connect(m_SocketId,(u32) m_ipAddress, m_SrvPort);
		if (SOC_SUCCESS == ret)
		{
			m_SocketConnState = 1;
			APP_DEBUG("<-- Connect to server successfully -->\r\n");
			SOCKET_IN_WOULD_BLOCK_FLAG = 0;
		}
		else
		{
			if(SOC_WOULDBLOCK == ret)
			{
				SOCKET_IN_WOULD_BLOCK_FLAG = 1;
				APP_DEBUG("<-- Waiting to SOC Connect : WOULD_BLOCK -->\r\n");                   
			   return;
			}
			APP_DEBUG("<-- Fail to connect server 1, cause=%d -->\r\n", ret);
			APP_DEBUG("<-- Close socket.-->\r\n");
			Ql_SOC_Close(m_SocketId);
			m_SocketId = -1;
			SERVER2_SETTING_SELECTION_FLAG = 1;
			return;
		}
	}
	else
	{
		ret = Ql_IpHelper_ConvertIpAddr(m_SrvADDR_1, (u32 *)m_ipAddress_1);
		if (SOC_SUCCESS == ret) // ip address is xxx.xxx.xxx.xxx
		{
			APP_DEBUG("<-- Convert Ip Address successfully,m_ipaddress=%d,%d,%d,%d -->\r\n",m_ipAddress_1[0],m_ipAddress_1[1],m_ipAddress_1[2],m_ipAddress_1[3]);
		}else{
			APP_DEBUG("<-- Fail to convert IP Address --> \r\n");
			return;
		}
		//6.2 Connect to server
		APP_DEBUG("<-- Connecting to server(IP:%d.%d.%d.%d, port:%d)... -->\r\n", m_ipAddress_1[0],m_ipAddress_1[1],m_ipAddress_1[2],m_ipAddress_1[3], m_SrvPort_1);
		ret = Ql_SOC_Connect(m_SocketId,(u32) m_ipAddress_1, m_SrvPort_1);
		if (SOC_SUCCESS == ret)
		{
			m_SocketConnState = 1;
			SOCKET_IN_WOULD_BLOCK_FLAG = 0;
			APP_DEBUG("<-- Connect to server successfully -->\r\n");
		}
		else
		{
			if(SOC_WOULDBLOCK == ret)
			{
				SOCKET_IN_WOULD_BLOCK_FLAG = 1;
				APP_DEBUG("<-- Waiting to SOC Connect : WOULD_BLOCK -->\r\n");                   
			   return;
			}
			
			APP_DEBUG("<-- Fail to connect to server 2, cause=%d -->\r\n", ret);
			APP_DEBUG("<-- Close socket.-->\r\n");
			Ql_SOC_Close(m_SocketId);
			m_SocketId = -1;
			SERVER2_SETTING_SELECTION_FLAG = 0;
			return;
		}
	
	}
}


void TIMER_HANDLER(u32 timerId, void* param)
{
	s32 iRet = 0;	
	s32 ret;
	//*((s32*)param) +=1;
    switch(timerId)
	{
		case TIMER_ID_WATCHDOG_FEED:
						//COUNT_FOR_STORE_TIME_IN_UFL++;
						if(start_flag == 0)
						{
							if(SYSTEM_WATCHDOG_FLAG == 1)
							{
								SYSTEM_WATCHDOG_FLAG = 0;
								//if(COMMUNICATION_WITH_CONTROLLER_FLAG == 1)
								{
									s32* WTD_Id = (s32*)param;
									Ql_WTD_Feed(*WTD_Id);
									APP_DEBUG("<-- WATCHDOG FEED\r\n");
									//COUNT_FOR_STORE_TIME_IN_UFL = 0;
								}
								//else{APP_DEBUG("\r\n COMMUNICATION_WITH_CONTROLLER_FLAG IS OFF");}
							}
							//else{APP_DEBUG("<-- WATCHDOG TIMER TRIGGER\r\n")}
						}
						break;
						
		case Stack_timer: 
						APP_DEBUG("<-- DATA SEND TO SERVER TIMER -->\r\n");
						send_flag1 = 1;
						QUERY_SEND_STAGE = 1;
						data_flag = 1;

						APP_DEBUG("<-- SEND_flag = 1\r\n");
						APP_DEBUG("<-- Data_flag = %d\r\n",data_flag);
						uart_flag2=1;
						comm_flag = 1;
						ret = Ql_Timer_Start(Stack_timer3,ST_Interval3,FALSE);
						if(ret < 0)
						{
						APP_DEBUG("<-- Failed!! GP-timer Ql_Timer_Start fail, ret=%d-->\r\n",ret);
						}                
						APP_DEBUG("<-- GP-timer Ql_Timer_Start(ID=%d,Interval=%d) ret=%d-->\r\n",Stack_timer3,ST_Interval3,ret);
						MODBUS_READ_QURIES();
						break;
    
		case GP_timer:	
						APP_DEBUG("<-- SYSTEM CHECK TIMER -->\r\n\n");
						if(fota_flag == 1)
						{
							FOTA_upgrade();
						}
						break;
	
		case Stack_timer3:
						APP_DEBUG("<-- GPRS TIME OUT TIMER\r\n\n");
						ret = Ql_Timer_Stop(Stack_timer3);
						if(ret < 0)
						{
							  APP_DEBUG("<--failed!! stack timer Ql_Timer_Stop ret=%d-->\r\n",ret);           
						}
						if(comm_flag == 1)
						{
							 APP_DEBUG("<-- COMM flag = 1, no communication with controller -->\r\n");
							 //GPRS_TCP_Program(1);
							 
							 if(socket_flag == 1)// checking GPRS connection is establish or not
								{
									APP_DEBUG("\r\n socket flag = 1 \r\n");
									APP_DEBUG("\r\n Trying to get gprs onnection \r\n");   
									GPRS_TCP_Program(0);
								}
								else
								{
									APP_DEBUG("\r\n GPRS connection is ok checking for going for socket connection or data sending \r\n");   
									if(SOCKET_IN_WOULD_BLOCK_FLAG == 0)// checking socket is in would block condition or not
									{
										APP_DEBUG("\r\n socket not in would block loop \r\n");
										APP_DEBUG("<-- m_SocketConnState = %d\r\n",m_SocketConnState);
										if(m_SocketConnState > 0)//socket connection establish or not
										{
											APP_DEBUG("\r\n  socket is connected going for data send \r\n");
											GPRS_TCP_Program(1);// if gprs connected and socket is connected go for data send
										}
										else
										{
											APP_DEBUG("\r\n  socket is not connected going for socket connection \r\n");
											SOC_CONNECT_TO_SERVER();// if gprs connected and socket is not connected go for socket connection
										}
									}
									else{ APP_DEBUG("\r\n socket in would block loop \r\n");  }
								
								}
						}
						break;
	
		case Stack_timer4:
						APP_DEBUG("<-- MODBUS QUERY WRITE TIMER\r\n\n", *((s32*)param));
						if(send_flag1 == 0)
						{
							modbus_flag1 = 1;
							MODBUS_WRITE_QUERIES();
						}
						break;

		case Stack_timer5:
						APP_DEBUG("<-- LOGBOOK TRIGGER TIMER -->\r\n", *((s32*)param));
						log_book_flag = 1;
						APP_DEBUG("<-- Log Book Flag = %d\r\n", log_book_flag);		
						break;
					
		default:break;
	}
}

void callback_onTimer(u32 timerId, void* param)
{
    s32* wtdid;
    s32 ret;
    wtdid = (s32*)param;
    APP_DEBUG("<--multitask: callback_onTimer1 wtdid =%d -->\r\n",*wtdid);       
	
	if(LOGIC_WTD3_TMR_ID == timerId)
    {
		if(wd_flag1 == 1)
		{
			Ql_WTD_Feed(*wtdid);
			//Ql_WTD_Stop(*wtdid);
			//Ql_Timer_Stop(timerId);
			wd_flag1 = 0 ;
			APP_DEBUG("\n watchdog feed, watchdog flag = 0 \n");
			 APP_DEBUG("<--multitask: callback_onTimer3 wtdid =%d, timerID =%d -->\r\n",*wtdid, timerId);      
		}
	}
	else if((LOGIC_WTD1_TMR_ID == timerId) || (LOGIC_WTD2_TMR_ID == timerId)) 
    {
		if(wd_flag == 1)
		{
			
			Ql_WTD_Feed(*wtdid);
			Ql_WTD_Stop(*wtdid);
			Ql_Timer_Stop(timerId);
			wd_flag = 0 ;
			APP_DEBUG("<--multitask: callback_onTimer1 or 2 wtdid =%d, timerID =%d -->\r\n",*wtdid, timerId);    
			
			wtdid2 = Ql_WTD_Start(600*1000);
			ret = Ql_Timer_Register(LOGIC_WTD3_TMR_ID, callback_onTimer, &wtdid2);
			if(ret < 0)
			{
				APP_DEBUG("<--main task: register fail ret=%d-->\r\n",ret);
			}
			else
			{
				APP_DEBUG("<--main task: watchdog set",ret);
			}
			ret = Ql_Timer_Start(LOGIC_WTD3_TMR_ID, 65000 ,TRUE);
			
			if(ret < 0)
			{
				APP_DEBUG("<--main task: start timer fail ret=%d-->\r\n",ret);        
			   // return;
			}
			else
			{
				APP_DEBUG("<--main task: watchdog timer set",ret);
			}
			APP_DEBUG("<--main task: start timer OK  ret=%d-->\r\n",ret);
		}
	}
}

void WTD_Init()
{
	s32 ret;
	// Initialize external watchdog:
    ret = Ql_WTD_Init(0, WATCHDOG_FEED_PIN, Timer_Val_WatchdogFeed);	// 0 means external WTD, 1 Means internal WTD
    if (0 == ret)
    {
		#if APP_DEBUG_ENABLE
        APP_DEBUG("\r\n<-- Watchdog Init OK!-->\r\n");    
		#endif		
    }

    // Create a logic watchdog, It feed the external watchdog for the interval.
	// the interval is 30s (can keep 30s or 1min also)
    WTD_Id = Ql_WTD_Start(3*60*1000);	
	
	// Register & start a timer to feed the logic watchdog.
    // The watchdog id will be passed into callback function as parameter.
    ret = Ql_Timer_Register(TIMER_ID_WATCHDOG_FEED, TIMER_HANDLER, &WTD_Id);
    if(ret < 0)
    {
		#if APP_DEBUG_ENABLE 
        APP_DEBUG("<-- Watchdog Timer register fail ret=%d -->\r\n",ret);
		#endif
        return;
    }
	else{APP_DEBUG("<-- Watchdog Timer Register\r\n");}
	
    ret = Ql_Timer_Start(TIMER_ID_WATCHDOG_FEED, 2000,TRUE);
    if(ret < 0)
    {
		#if APP_DEBUG_ENABLE
        APP_DEBUG("<-- Watchdog Timer start fail ret=%d -->\r\n",ret);
		#endif
        return;
    }
	else{APP_DEBUG("<-- Watchdog Timer Start\r\n");}
	
	// #if APP_DEBUG_ENABLE
    // APP_DEBUG("<-- Watchdog Timer start OK  ret=%d-->\r\n",ret);
	// #endif
}

void WTD_DeInit()
{
	#if APP_DEBUG_ENABLE
    APP_DEBUG("<-- Watchdog Timer DeInit -->\r\n");
	#endif
	Ql_Timer_Stop(TIMER_ID_WATCHDOG_FEED);
}


void CALCULATE_PARAMETER_FROM_QUERY(char *DATA)
{
	char i;
	
	PARAMETER_01 =0;
	PARAMETER_02 =0;
	PARAMETER_03 =0;
	PARAMETER_04 =0;
	PARAMETER_05 =0;
	PARAMETER_06 =0;
	PARAMETER_07 =0;
	PARAMETER_08 =0;
	PARAMETER_09 =0;
	PARAMETER_10 =0;
	PARAMETER_11 =0;
	PARAMETER_12 =0;
	PARAMETER_13 =0;
	PARAMETER_14 =0;
	PARAMETER_15 =0;
	PARAMETER_16 =0;
	PARAMETER_17 =0;
	PARAMETER_18 =0;
	PARAMETER_19 =0;
	PARAMETER_20 =0;
	
	APP_DEBUG("\n");
	for(i = 0; i<45; i++)
	{
		APP_DEBUG("%c%c  ",DECIMAL_TO_HEX(DATA[i] / 0x10),DECIMAL_TO_HEX(DATA[i] % 0x10));
	}
	APP_DEBUG("\n");
	DATA = DATA+3;
	
	//it shifts from the 1st postion upto the 8th position by this formula.
	
	PARAMETER_01 = (DATA[0] *256) + DATA[1];
	PARAMETER_02 = (DATA[2] *256)  + DATA[3];
	PARAMETER_03 = (DATA[4] *256)  + DATA[5];
	PARAMETER_04 = (DATA[6] *256)  + DATA[7];
	PARAMETER_05 = (DATA[8] *256)  + DATA[9];
	PARAMETER_06 = (DATA[10] *256) + DATA[11];
	PARAMETER_07 = (DATA[12] *256) + DATA[13];
	PARAMETER_08 = (DATA[14] *256) + DATA[15];
	PARAMETER_09 = (DATA[16] *256) + DATA[17];
	PARAMETER_10 = (DATA[18] *256) + DATA[19];
	PARAMETER_11 = (DATA[20] *256) + DATA[21];
	PARAMETER_12 = (DATA[22] *256) + DATA[23];
	PARAMETER_13 = (DATA[24] *256) + DATA[25];
	PARAMETER_14 = (DATA[26] *256) + DATA[27];
	PARAMETER_15 = (DATA[28] *256) + DATA[29];
	PARAMETER_16 = (DATA[30] *256) + DATA[31];
	PARAMETER_17 = (DATA[32] *256) + DATA[33];
	PARAMETER_18 = (DATA[34] *256) + DATA[35];
	PARAMETER_19 = (DATA[36] *256) + DATA[37];
	PARAMETER_20 = (DATA[38] *256) + DATA[39];
	APP_DEBUG("\r\n<--PARAMETER_01 = %d\r\n<--PARAMETER_02 = %d\r\n<--PARAMETER_03 = %d\r\n<--PARAMETER_04 = %d\r\n<--PARAMETER_05 = %d\r\n<--PARAMETER_06 = %d\r\n<--PARAMETER_07 = %d\r\n<--PARAMETER_08 = %d\r\n<--PARAMETER_09 = %d\r\n<--PARAMETER_10 = %d\r\n<--PARAMETER_11 = %d\r\n<--PARAMETER_12 = %d\r\n<--PARAMETER_13 = %d\r\n<--PARAMETER_14 = %d\r\n<--PARAMETER_15 = %d\r\n<--PARAMETER_16 = %d\r\n<--PARAMETER_17 = %d\r\n<--PARAMETER_18 = %d\r\n<--PARAMETER_19 = %d\r\n<--PARAMETER_20 = %d",PARAMETER_01, PARAMETER_02, PARAMETER_03, PARAMETER_04, PARAMETER_05, PARAMETER_06, PARAMETER_07, PARAMETER_08, PARAMETER_09, PARAMETER_10,PARAMETER_11, PARAMETER_12, PARAMETER_13, PARAMETER_14, PARAMETER_15, PARAMETER_16, PARAMETER_17, PARAMETER_18, PARAMETER_19, PARAMETER_20);
}

int PORT_VALUE_CAL(char writeBuffer11[])
{
	int total=0, total1=0;
	char i;
	for(i=0; i< 5; i++)
	{
		if(writeBuffer11[i] == '\0')
		{
			break;
		}
		total = total * 10;
		total1 = writeBuffer11[i] ;
		total1 = total1 - 48 ;
		total = total + total1;
	}
	return total;
}

u32 CONVERT_TIME_BUFFER_TO_SECONDS_VALUE(char interval_buf[])
{
	u32 sum=0; 
	int sum1=0;
	
	sum = interval_buf[0] - 48;
	sum = sum * 10;
	sum = sum + interval_buf[1] - 48;
	sum = sum * 60 ; 
	sum1 = interval_buf[3] - 48;
	sum1 = sum1 * 10;
	sum1 = sum1 + interval_buf[4] - 48;
	sum = sum + sum1;
	sum = sum * 1000;
	
	return sum;
}

static void LOCATION_PROGRAM(void)
{
    s32 ret;
    u8  pdpCntxtId;

    // Set PDP context
    ret = Ql_GPRS_GetPDPContextId();
    APP_DEBUG("<-- The PDP context id available is: %d (can be 0 or 1)-->\r\n", ret);
    if (ret >= 0)
    {
        pdpCntxtId = (u8)ret;
    }

    ret = RIL_NW_SetGPRSContext(pdpCntxtId);
    APP_DEBUG("<-- Set PDP context id to %d -->\r\n", pdpCntxtId);
    if (ret != RIL_AT_SUCCESS)
    {
        APP_DEBUG("<-- Ql_RIL_SendATCmd error  ret=%d-->\r\n",ret );
    }

    // Request to get location
    APP_DEBUG("<-- Getting module location... -->\r\n");
    ret = RIL_GetLocation(Callback_Location);
    if (ret != RIL_AT_SUCCESS)
    {
        APP_DEBUG("<-- Ql_GetLocation error  ret=%d-->\r\n",ret );
    }
	 ret = Ql_GPRS_DeactivateEx(pdpCntxtId, TRUE);
    APP_DEBUG("<-- Deactivated GPRS, cause=%d -->\r\n\r\n", ret);
}

void Callback_Location(s32 result, ST_LocInfo* loc_info)
{
    int string1, string2,i=0, k=0;
	char test[50]={0},test1[50]={0};
	APP_DEBUG("\r\n<-- Module location: latitude=%f, longitude=%f -->\r\n", loc_info->latitude, loc_info->longitude);
	
	string1 = loc_info->latitude * 1000000;
	string2 = loc_info->longitude * 1000000;
	
	Ql_memset(DEVICE_LATITUDE,0, sizeof(DEVICE_LATITUDE));
	Ql_memset(DEVICE_LONGITUDE,0, sizeof(DEVICE_LONGITUDE));
	
	Ql_sprintf(DEVICE_LATITUDE,"%ld",string1);
	Ql_sprintf(DEVICE_LONGITUDE,"%ld",string2);
	
	APP_DEBUG("<-- DEVICE_LATITUDE: %s ,DEVICE_LONGITUDE: %s \r\n ",DEVICE_LATITUDE,DEVICE_LONGITUDE);		
}

void SEPARATE_MODBUS_DATA(char *modbus_write_data)
{
	char i,length=10;                         
	int data_array[10],address=10;
	
	APP_DEBUG("<-- MODBUS WRITE DATA = %s\r\n", modbus_write_data);	
    // MODBUS WRITE DATA = 00000,00021,00000,00085,00000,00809,00000,00000,00000,00000,00000,00000,00000,00000,00000,40000,00000,07648,00000,00665	
	Ql_memset(data_array, 0x00, sizeof(data_array));
	
	for(i=0; i<20; i++)		
	{
		data_array[i] = ((modbus_write_data[(i*6)] - 48)* 10000) +((modbus_write_data[(i*6)+1] - 48)*1000) +((modbus_write_data[(i*6)+2] - 48) *100) + ((modbus_write_data[(i*6)+3] - 48) *10) + (modbus_write_data[(i*6)+4] - 48);
		APP_DEBUG("<-- data_array[%d] = %d\r\n", i, data_array[i]);
	}
	
	/*data_array[0]=0
	  data_array[1]=21
	  data_array[2]=0
	  data_array[3]=85
	  data_array[4]=0
	  data_array[5]=809
	  data_array[6]=0
	  data_array[7]=0
	  data_array[8]=0
	  data_array[9]=0
	  data_array[10]=0
	  data_array[11]=0
	  data_array[12]=0
	  data_array[13]=0
	  data_array[14]=0
	  data_array[15]=40000
	  data_array[16]=0
	  data_array[17]=07648
	  data_array[18]=0
	  data_array[19]=665
	*/
	
	CREATE_AND_WRITE_QUERY_FOR_MULTIPLE_ADRESSES(address, data_array, length);
}

//here we are directly capturing data and calculating CRC and sending it
void CREATE_AND_WRITE_QUERY_FOR_MULTIPLE_ADRESSES(int address,int *data_array, int length)
{
	unsigned short int crc16;
	char length_of_query,i;
	unsigned char ch_bit_cntr,chcarry,ch_crc;
	length_of_query = ((length * 2) + 9);
	
	unsigned char query_buff[length_of_query];
	
	Ql_memset(query_buff, 0, sizeof(query_buff));
	
	query_buff[0] = 0x01;
	query_buff[1] = 0x10;         // funtion code of Multiple registers. Decimal no 16
	query_buff[2] = address / 256; 
	query_buff[3] = address % 256; 
	query_buff[4] = length/256; 
	query_buff[5] = length%256; 
	query_buff[6] = (length * 2);//no of bytes to read
	
	for(i=0; i<length; i++)
	{
		query_buff[(i*2)+7] = data_array[i]/256;
		query_buff[(i*2)+8] = data_array[i]%256;
	}
	 
	crc16 = 0xffff;
	for(i=0; i<length_of_query - 2; i++)
	{
		ch_crc = query_buff[i];
		crc16 = crc16 ^ ch_crc;
		
		for(ch_bit_cntr = 0; ch_bit_cntr < 8; ch_bit_cntr++)
		{
			chcarry = crc16 & 0x0001;
			crc16 = crc16 >> 1;
			if(chcarry)
			{
				crc16 = (crc16 ^ 0xA001);
			}
		}
	}
	
	ch_crc = crc16 & 0xff;
	query_buff[length_of_query - 2] = ch_crc;
	
	crc16 = crc16 >> 8;
	ch_crc = crc16 & 0xff;
	query_buff[length_of_query - 1] = ch_crc;
	
	APP_DEBUG("<-- CRC1 = %d, CRC2 = %d\r\n\r\n",query_buff[length_of_query - 2], query_buff[length_of_query - 1])
	
	for(i = 0; i<length_of_query; i++)
	{
		APP_DEBUG("%c%c  ",DECIMAL_TO_HEX(query_buff[i] / 0x10),DECIMAL_TO_HEX(query_buff[i] % 0x10));
	}
	APP_DEBUG("\n");
	//write query into PLC
	Ql_UART_Write(UART_PORT2, query_buff, length_of_query );//write 10address in single query
	
	
}

void CALCULATE_CRC(int address_1, int data_1)
{
    unsigned short int crc16;
	unsigned char ch_byte_cntr,ch_bit_cntr,chcarry,ch_crc,tx1buf[8]={0};
	char i;
	APP_DEBUG("<-- CRC CALCULATION FUNCTION\r\n");

	//address_1 = address_1 - 1;             // address adustment for modbus protocol
	tx1buf[0]= 0x02; 
	if((address_1 >= 40000)&&(address_1 < 50000))
	{
		address_1 = address_1 - 40000;
		tx1buf[1]= 0x06; 
	}
	else
	{
		APP_DEBUG("<-- ADDRESS FROM SERVER IS NOT AS PER CONDITION, ADDRESS: %d",address_1);
		return;
	}
	tx1buf[2]= address_1 / 256; 
	tx1buf[3]= address_1 % 256; 
	tx1buf[4]= data_1/256; 
	tx1buf[5]= data_1%256; 
	
	crc16 = 0xffff;

	for(ch_byte_cntr = 0; ch_byte_cntr < 6; ch_byte_cntr++)
	{
		ch_crc = tx1buf[ch_byte_cntr];
		crc16 = crc16 ^ ch_crc;

		for(ch_bit_cntr = 0; ch_bit_cntr < 8; ch_bit_cntr++)
		{
			chcarry = crc16 & 0x0001;
			crc16 = crc16 >> 1;
			if(chcarry)
			{
				crc16 = (crc16 ^ 0xA001);
			}
		}
	}
	
	ch_crc = crc16 & 0xff;
	tx1buf[6] = ch_crc;
	//APP_DEBUG("\r\ch_crc1 = %d string = %s\r\n", ch_crc, tx1buf);  
	
	crc16 = crc16 >> 8;
	ch_crc = crc16 & 0xff;
	tx1buf[7] = ch_crc;
	//APP_DEBUG("\r\ch_crc2 = %d string = %s\r\n", ch_crc, tx1buf);  
	APP_DEBUG("<-- MODBUS QUERY FOR WRITE\r\n");
	
	for(i = 0; i<8; i++)
	{
		APP_DEBUG("%c%c\r\r",DECIMAL_TO_HEX(tx1buf[i] / 0x10),DECIMAL_TO_HEX(tx1buf[i] % 0x10));
	}
	APP_DEBUG("\n");
	Ql_UART_Write(UART_PORT2, tx1buf, 8 );
	APP_DEBUG("<-- MODBUS WRITE QUERY SEND ON UART PORT2\n\r");
 }

void DELAY(int time_delay)
{
	int k,l;
	for(k=0;k<time_delay;k++)
	{
		for(l=0;l<1024;l++)
		{}
	}
	APP_DEBUG("<-- DELAY OVER\r\n");
}

char DECIMAL_TO_HEX(unsigned n)
{
    if (n < 10) {
        return n + '0';
    } else {
        return (n - 10) + 'A';
    }
}

#endif // __EXAMPLE_TRANSPASS__