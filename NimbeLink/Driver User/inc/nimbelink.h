/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NIMBELINK_H__
#define __NIMBELINK_H__

#define MAX_NBL_RECV_DATA_LEN   128
#define MAX_DBG_RECV_DATA_LEN   128

#define AUTO_CMD_LEN ((uint16_t)-1)

#define NIB_RES_TIMEOUT     180      //3 min


#define NIB_SET_ECHO_OFF                "ATE0\r\n"                         //0k
#define NIB_CMD_CFUN1                   "AT+CFUN=1\r\n"
#define NIB_CPIN                        "AT+CPIN?\r\n"
#define NIB_GET_IMEI                    "AT+CGSN\r\n"
#define NIB_GET_ICCID                   "AT+CCID\r\n"
#define NIB_GET_CIMI                    "AT+CIMI\r\n"
#define NIB_GET_CGMM                    "AT+CGMM\r\n"
#define NIB_GET_CGMI                    "AT+CGMI\r\n"
#define NIB_FIRMWARE_VER                "AT+CGMR\r\n"
#define NIB_GET_CSQ                     "AT+CSQ\r\n"
#define NIB_NET_QUERY_CREG              "AT+CREG?\r\n"
#define NIB_SOCKET_CONTEXT_ID           "AT+QIACT=1\r\n"                                       // OK
#define NIB_SOCKET_DIAL_STAGING         "AT+QIOPEN=1,0,\"TCP\",\"g.scstg.net\",9221,0,0\r\n" // CONNECT
#define NIB_SOCKET_SEND_START           "AT+QISEND=0\r\n"                                      //>
#define NIB_CONTEXT_IP_ADD              "AT+CGPADDR=1\r\n"    // get ip address of pdp context
#define NIB_CONTEXT_IP_NULL             "0.0.0.0"
#define NIB_QUERY_CGATT_PS              "AT+CGATT=?\r\n"
#define NIB_SET_CGATT_PS                "AT+CGATT=1\r\n"     // Attach PS
#define NIB_QUERY_PDP_CONTEXT           "AT+CGACT?\r\n"
#define NIB_SET_PDP_CONTEXT             "AT+CGACT=1,1\r\n"
#define NIB_SOCKET_OK                   ">"
#define NIB_SOCKET_CTRL_Z                0x1A               // 26
#define NIB_SOCKET_SEND_DATA            "AT+QIRD=0,128\r\n" // "AT+QIRD=0,%d"
#define NIB_SOCKET_CLOSE                "AT+QICLOSE=1\r\n"
#define NIB_ERR_MSG_FORMATE             "AT+CMEE=2\r\n"    // Enable result code use numeric value

#define DEFINE_PDP_CONTEXT              "AT+CGDCONT=1,\"IP\",\"onomondo\"\r\n"    // Configure pdp context

#define NIB_SIM_READ_BINARY             "AT+CRSM=176,28539,0,0,0\r\n"
#define NIB_SIM_UPDATE_BINARY           "AT+CRSM=214,28539,0,0,12,\"FFFFFFFFFFFFFFFFFFFFFFFF\"\r\n"

#define NIB_CONFIG_RAT_SEARCH_SEQ       "AT+QCFG=\"nwscanseq\",010203,1\r\n"    // scan network sequence GSM->eMTC->NB-IoT , imidiate effect



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


RES_TYPE NIB_SendCmd(const char *cmd, const char * pRes1, const char * pRes2, uint16_t timeOut_s);

void NIB_Enable_IRQ(void);
void NIB_Clearbuffer(void);
void NIB_CMD_Process(void);

void NIB_test(void);
//char* strFind(const char* mainStr, const char* searchStr);
uint8_t NIB_powerOn(void);
uint8_t NIB_getInfo(void);
uint8_t NIB_connectServer(void);

uint8_t* MapForward(uint8_t* buff, uint8_t* buff_find, uint8_t len);

#endif  //#ifndef __NIMBELINK_H__
