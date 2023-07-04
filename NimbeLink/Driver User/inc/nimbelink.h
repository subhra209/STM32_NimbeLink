/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NIMBELINK_H__
#define __NIMBELINK_H__

#define MAX_NBL_RECV_DATA_LEN   128
#define MAX_DBG_RECV_DATA_LEN   128
#define AUTO_CMD_LEN ((uint16_t)-1)

#define NIB_MAX_RES_TIMEOUT     180      //3 min


#define NIB_SET_ECHO_OFF        "ATE0\r\n"                         //0k
#define CMD_CFUN1               "AT+CFUN=1\r\n"
#define GSM_CPIN                "AT+CPIN?\r\n"
#define GSM_GET_IMEI            "AT+CGSN\r\n"
#define GSM_GET_ICCID           "AT+CCID\r\n"
#define GSM_GET_CIMI            "AT+CIMI\r\n"
#define GSM_GET_CGMM            "AT+CGMM\r\n"
#define GSM_GET_CGMI            "AT+CGMI\r\n"
#define GSM_FIRMWARE_VER        "AT+CGMR\r\n"
#define SIGNAL_QUALITY          "AT+CSQ\r\n"
#define NET_REG_REPORT          "AT+CREG?\r\n"
#define SOCKET_CONTEXT_ID       "AT+QIACT=1\r\n"                                       // OK
#define SOCKET_DIAL_STAGING     "AT+QIOPEN=1,0,\"TCP\",\"g.scstg.net\",9221,0,0\r\n" // CONNECT
#define SOCKET_SEND_START       "AT+QISEND=0\r\n"                                      //>
#define SOCKET_OK               ">"
#define SOCKET_CTRL_Z           0x1A               // 26
#define SOCKET_SEND_DATA        "AT+QIRD=0,128\r\n" // "AT+QIRD=0,%d"
#define SOCKET_CLOSE            "AT+QICLOSE=1\r\n"

#define SOCKET_RESPONSE_SUCCESS "SEND OK"

typedef enum{
RES_SUCCESS = 1,
RES_ERROR,
RES_TIMEOUT
}RES_TYPE;


RES_TYPE NIB_SendCmd(const char *cmd, const char * pRes1, const char * pRes2, uint16_t timeOut_s);

void NIB_Enable_IRQ(void);
void NIB_Clearbuffer(void);
uint8_t  NIB_Res_Ok();
void NIB_CMD_Process(void);

void NIB_test(void);
//char* strFind(const char* mainStr, const char* searchStr);
uint8_t NIB_powerOn(void);
uint8_t NIB_getInfo(void);

#endif  //#ifndef __NIMBELINK_H__
