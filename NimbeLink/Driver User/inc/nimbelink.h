/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NIMBELINK_H__
#define __NIMBELINK_H__

#define MAX_NBL_RECV_DATA_LEN   128
#define MAX_DBG_RECV_DATA_LEN   128
#define AUTO_CMD_LEN ((uint16_t)-1)

#define GSM_SET_ECHO_OFF                        "ATE0\r\n"
#define GSM_GET_SUBSCRIBER_NUMBER               "AT+CGSN\r\n"
#define GSM_GET_MANUFACTURER_NAME               "AT+CGMI\r\n"


void NIB_Enable_IRQ(void);
void NIB_SendCmdLen(const char *cmd,uint16_t Len);
void NIB_Clearbuffer(void);
uint8_t  NIB_Res_Ok();
void NIB_CMD_Process(void);

void NIB_test(void);
//char* strFind(const char* mainStr, const char* searchStr);
uint8_t NIB_powerOn(void);

#endif  //#ifndef __NIMBELINK_H__
