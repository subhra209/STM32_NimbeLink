/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NIMBELINK_H__
#define __NIMBELINK_H__

#define MAX_NBL_RECV_DATA_LEN   128
#define MAX_DBG_RECV_DATA_LEN   128

#define AUTO_CMD_LEN ((uint16_t)-1)

#define NIB_RES_TIMEOUT     180      //3 min


#define NIB_SET_ECHO_OFF                "ATE0\r\n"                         // (OK) set echo mode off
#define NIB_CMD_CFUN1                   "AT+CFUN=1\r\n"                    // (OK) set to full functionality
#define NIB_CPIN                        "AT+CPIN?\r\n"                     // (OK) read CPIN status
#define NIB_GET_IMEI                    "AT+CGSN\r\n"                      // (OK) Get IMEI Number of device
#define NIB_GET_ICCID                   "AT+CCID\r\n"                      // (OK) Get CCID number of device
#define NIB_GET_CIMI                    "AT+CIMI\r\n"                      // (OK) International Mobile Subscriber Identity
#define NIB_GET_CGMM                    "AT+CGMM\r\n"                      // (OK) Get Model Identification
#define NIB_GET_CGMI                    "AT+CGMI\r\n"                      // (OK) Get Manufacturer Identification
#define NIB_FIRMWARE_VER                "AT+CGMR\r\n"                      // (OK) Get Firmware Version Identification
#define NIB_GET_CSQ                     "AT+CSQ\r\n"                       // (OK) Check Signal Quality
#define NIB_NET_QUERY_CREG              "AT+CREG?\r\n"                     // (OK) Get Network Registration Status
#define NIB_SOCKET_CONTEXT_ID           "AT+QIACT=1\r\n"                                       // (OK) activate pdp context
#define NIB_SOCKET_DIAL_STAGING         "AT+QIOPEN=1,0,\"TCP\",\"g.scstg.net\",9221,0,0\r\n"   // CONNECT Socket To Server
#define NIB_SOCKET_SEND_START           "AT+QISEND=0\r\n"                                      // ">"
#define NIB_CONTEXT_IP_ADD              "AT+CGPADDR=1\r\n"                      // (OK) get ip address of pdp context
#define NIB_CONTEXT_IP_NULL             "0.0.0.0"
#define NIB_QUERY_CGATT_PS              "AT+CGATT?\r\n"                         // (OK) Read State of PS
#define NIB_SET_CGATT_PS                "AT+CGATT=1\r\n"                        // (OK) Attach PS
#define NIB_QUERY_PDP_CONTEXT           "AT+CGACT?\r\n"                         // (OK) Read State of PDP Context (activate-deactivate)
#define NIB_SET_PDP_CONTEXT             "AT+CGACT=1,1\r\n"                      // (OK) activate PDP Context
#define NIB_SOCKET_OK                   ">"
#define NIB_SOCKET_CTRL_Z                0x1A                                   // 26
#define NIB_SOCKET_SEND_DATA            "AT+QIRD=0,128\r\n"                     // "AT+QIRD=0,%d"
#define NIB_SOCKET_CLOSE                "AT+QICLOSE=1\r\n"                      //(OK) Close a Socket Service
#define NIB_ERR_MSG_FORMATE             "AT+CMEE=2\r\n"                         //(OK) Enable result code use numeric value

#define DEFINE_PDP_CONTEXT              "AT+CGDCONT=1,\"IP\",\"onomondo\"\r\n"    // Configure pdp context

#define NIB_SIM_READ_BINARY             "AT+CRSM=176,28539,0,0,0\r\n"
#define NIB_SIM_UPDATE_BINARY           "AT+CRSM=214,28539,0,0,12,\"FFFFFFFFFFFFFFFFFFFFFFFF\"\r\n"

#define NIB_CONFIG_RAT_SEARCH_SEQ       "AT+QCFG=\"nwscanseq\",010203,1\r\n"    // (OK) scan network sequence GSM->eMTC->NB-IoT , imidiate effect
#define NIB_QUERY_SOCKET_STATUS         "AT+QISTATE?\r\n"                       // (OK) check state of socket


#define SOCKET_RESPONSE_SUCCESS "SEND OK"


#define NIB_RES_OK                              "OK"
#define NIB_RES_ERROR                           "ERROR"
#define NIB_SOCKET_DAIL_OK                      "+QIOPEN: 0,0"

typedef struct {
  /* HELLO MEssage */
  uint8_t startByte;
  uint8_t imei[7];
  uint8_t flags2[8];
} WaypointHeader;

typedef enum{
RES_SUCCESS = 1,
RES_ERROR,
RES_TIMEOUT
}RES_TYPE;

/*************************************************************************
* Function Name: NIB_SendCmd
* Parameters:1) data - command char address
*            2) pRes1 - Ok value
*            3) pRes2 - Error value
*            4) timeOut_s - Timeout Value
*
* Return: Status (SUCCESS / ERRROR)
*
* Description: send command to nimbelink module and print command in terminal
*
*************************************************************************/
RES_TYPE NIB_SendCmd(const char *cmd, const char * pRes1, const char * pRes2, uint16_t timeOut_s);
/*************************************************************************
* Function Name: NIB_Enable_IRQ
* Parameters: None
* Return: none
*
* Description: enable UART-2 interrupt register not empty flag
*************************************************************************/
void NIB_Enable_IRQ(void);
/*************************************************************************
* Function Name: NIB_Clearbuffer
* Parameters: None
* Return: none
*
* Description: Clear NIMBELINK Receive buffer and index
*************************************************************************/
void NIB_Clearbuffer(void);
/*************************************************************************
* Function Name: NIB_CMD_Process
* Parameters: None
* Return: none
*
* Description: Perfrom operation based on command received from terminal
*************************************************************************/
void NIB_CMD_Process(void);
/*************************************************************************
* Function Name: NIB_powerOn
* Parameters:none

* Return: Status Of AppReadyFlag (TRUE / FALSE)
*
* Description: power on device and get inforamtion of device ,
               connect to server and send data
*************************************************************************/
uint8_t NIB_powerOn(void);
/*************************************************************************
* Function Name: NIB_getInfo
* Parameters:none
*
* Return:Status (TRUE / FALSE)
*
* Description: Get Information of NIMBELINK Module
*************************************************************************/
uint8_t NIB_getInfo(void);
/*************************************************************************
* Function Name: NIB_connectServer
* Parameters:none

* Return: Status (TRUE / FALSE)
*
* Description: Nimbelink Module Connect With Server
*
*************************************************************************/
uint8_t NIB_connectServer(void);
/*************************************************************************
* Function Name: MapForward
* Parameters: None
* Return: ptr (Matched last char address) [ NULL || NOT NULL]
*
* Description: Compare main buff data and Find Buff data (strstr)
*
*************************************************************************/
uint8_t* MapForward(uint8_t* buff, uint8_t* buff_find, uint8_t len);
/*******************************************************************************
 * Funtion name : str_to_imei                                                  *
 *                                                                             *
 * Description  : Convert String To int IMEI number             	       * 				                                    						   *
 * Arguments    : str,imei       					       *
 * Returns      : none                        				       *
 *******************************************************************************/
void str_to_imei(char *str, char *imei);

#endif  //#ifndef __NIMBELINK_H__
