/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "stdlib.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_hal.h"
#include "nimbelink.h"


// huart2 structure handler
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
// indication flag for nibelink test enabel or disable
extern uint8_t NIB_Test_Enable_f;
// buffer for store charchater of array recieved from UART2 Interrupt
extern uint8_t Dbg_Rcv_Beffer;
// Dbg_Rcv_Beffer index
extern uint8_t Dbg_Rcv_Beffer_Index;

uint8_t NBL_Rcv_Buffer[MAX_NBL_RECV_DATA_LEN];          // receive buffer for nimbelink module
uint8_t NBL_Rcv_BufferIndex = 0;                        // buffer index

uint8_t AppReadyFlag = 0;                               // app ready flag for indicate device is ready

uint16_t gsmReg_timeOut = 180; //3 min - 180 sec

//Variable to store GSM modem Info
char iccid[32];
char imei[20];
char imsi[16];
char imei_hex[7];

WaypointHeader Header;

uint8_t test_data[48] = {0x30,0x3d,0x6a,0x8d,0x4,0x84,0x1c,0x44,0xe7,0x76,0x42,0xe4,0xc9,0xd7,0xeb,0xa5,0x7e,0x9d,0x4f,0x78,0x35,0x08,0xfb,0x88,0xcc,0xcc,0xb2,0x89,0xae,0x4f,0x45,0x2f,0x94,0xb4,0xe3,0xe2,0xf3,0xc1,0x06,0x31,0x14,0x85,0xf3,0x6e,0x88,0xff,0x8d,0x76};
/* Private Function prototype ---------------------------------------------------------*/
void str_to_imei(char *str, char *imei);
uint8_t* MapForward(uint8_t* buff, uint8_t* buff_find, uint8_t len);

/* Function ------------------------------------------------------------------*/

/*******************************************************************************
 * Funtion name : str_to_imei                                               *
 *                                                                             *
 * Description  :             	   * * 				                                    						   *
 * Arguments    : none       						   *
 * Returns      : none                        				   *
 *******************************************************************************/
void str_to_imei(char *str, char *imei)
{
  unsigned long long ff=0, ff2=0, ff3;
  uint8_t i = 0;

  ff = *(str + 14) - '0';
  ff += 10*(*(str + 13) - '0');
  ff += 100*(*(str + 12) - '0');
  ff += 1000*(*(str + 11) - '0');
  ff += 10000*(*(str + 10) - '0');
  ff += 100000*(*(str + 9) - '0');
  ff += 1000000*(*(str + 8) - '0');
  ff += 10000000*(*(str + 7) - '0');

  ff2 = str[6] - '0';
  ff2 += 10*(str[5] - '0');
  ff2 += 100*(str[4] - '0');
  ff2 += 1000*(str[3] - '0');
  ff2 += 10000*(str[2] - '0');
  ff2 += 100000*(str[1] - '0');
  ff2 += 1000000*(str[0] - '0');

  ff3 = ff2 * 100000000 + ff;

  for(i = 0; i < 7; i++)
  {
    imei[i] = ff3 & 0xFF;
    ff3 >>= 8;
  }
}

/*************************************************************************
* Function Name: NIB_Write
* Parameters:1) data - command char address
             2) len - length of command
*
* Return: none
*
* Description: send command to nimbelink module using huart2 interface
*
*************************************************************************/
void NIB_Write(const void *data, uint16_t len)
{
  len = (len == AUTO_CMD_LEN) ? strlen((const char *)data) : len;

  HAL_UART_Transmit(&huart1,(uint8_t *)data, len, 5000);
}
/*************************************************************************
* Function Name: NIB_SendCmdLen
* Parameters:1) data - command char address
             2) len - length of command
*
* Return: none
*
* Description: send command to nimbelink module and print command in terminal
*
*************************************************************************/
RES_TYPE NIB_SendCmd(const char *cmd, const char * pRes1, const char * pRes2, uint16_t timeOut_s)
{
  // clear receiver buffer
  NIB_Clearbuffer();
  // print command in terminal
  dprintf("NIMBLINK <--- %s", cmd);
  // send command to nimbelink module
  NIB_Write(cmd, AUTO_CMD_LEN);

  while(timeOut_s)
  {
    vTaskDelay(1000/ portTICK_PERIOD_MS);
    if(MapForward(NBL_Rcv_Buffer, (uint8_t*)pRes1, strlen(pRes1)) != NULL)
    {
      dprintf("NIMBLINK ---> %s", NBL_Rcv_Buffer);
      return RES_SUCCESS;
    }
    if(MapForward(NBL_Rcv_Buffer, (uint8_t*)pRes2, strlen(pRes2)) != NULL)
    {
      dprintf("NIMBLINK ---> %s", NBL_Rcv_Buffer);
      return RES_ERROR;
    }
    --timeOut_s;
  }
  return RES_TIMEOUT;
}

/*************************************************************************
* Function Name: NIB_powerOn
* Parameters:none

*
* Return:none
*
* Description: Check for valid response is received or not from nimbelink
*
*************************************************************************/
uint8_t NIB_powerOn(void)
{
  if(AppReadyFlag == FALSE)
  {
    if(MapForward(NBL_Rcv_Buffer, (uint8_t*)"APP RDY", 7) != NULL)
    {
      AppReadyFlag = TRUE;
      //NIB_SendCmd(GSM_GET_CSQ, NIB_RES_OK, NIB_RES_ERROR,NIB_RES_TIMEOUT);
      //send echo off cmd
      if(NIB_SendCmd(NIB_SET_ECHO_OFF, NIB_RES_OK, NIB_RES_ERROR, NIB_RES_TIMEOUT) == RES_SUCCESS)
      {
        NIB_getInfo();
        NIB_SendCmd("at+crsm=214,28539,0,0,12,\"FFFFFFFFFFFFFFFFFFFFFFFF\"\r\n",
                    NIB_RES_OK, NIB_RES_ERROR, NIB_RES_TIMEOUT);
        if(NIB_connectServer())
        {

          //send data
          if(NIB_SendCmd("AT+QISEND=0,48", NIB_SOCKET_OK,
                         NIB_RES_ERROR, NIB_RES_TIMEOUT) == RES_SUCCESS)
          {
            uint16_t timeOut = 180;
            //After recieve '>' send data
            NIB_Clearbuffer();
            // send command to nimbelink module
            NIB_Write(test_data, 48);

            //check for SEND OK
            while(timeOut)
            {
              vTaskDelay(1000/ portTICK_PERIOD_MS);
              if(MapForward(NBL_Rcv_Buffer, "SEND OK", strlen("SEND OK")) != NULL)
              {
                dprintf("NIMBLINK ---> %s", NBL_Rcv_Buffer);
                return RES_SUCCESS;
              }
              --timeOut;
            }
          }
        }
      }
    }
  }
  return AppReadyFlag;
}

/*************************************************************************
* Function Name: NIB_getInfo
* Parameters:none

*
* Return:none
*
* Description: Check for valid response is received or not from nimbelink
*
*************************************************************************/
uint8_t NIB_getInfo(void)
{
  uint8_t* pToken = NULL;
  uint8_t i = 0;

  if(NIB_SendCmd(NIB_GET_IMEI, NIB_RES_OK, NIB_RES_ERROR, NIB_RES_TIMEOUT) == RES_SUCCESS)
  {
    //store IMEI number
    pToken = MapForward(NBL_Rcv_Buffer,
                        (unsigned char*)"\n", 1);
    if(pToken != NULL)
    {
      for(i = 0; i < 15; i++)
      {
        imei[i] = pToken[i+1];
      }
      imei[15] = '\0';

      str_to_imei(imei, imei_hex);
      memcpy(Header.imei, imei_hex, 7);
      Header.startByte = 0x90;
    }
  }
  else
  {
    return FALSE;
  }

  if(NIB_SendCmd(NIB_GET_ICCID, NIB_RES_OK, NIB_RES_ERROR, NIB_RES_TIMEOUT) == RES_SUCCESS)
  {
    //store ICCID number
    pToken = MapForward(NBL_Rcv_Buffer,
                        (unsigned char*)"+CCID: ", 7);
    if(pToken != NULL)
    {
      for(i = 0; i < 20; i++)
      {
        iccid[i] = pToken[i + 7];
      }
      iccid[20] = '\0';
    }
  }
  else
  {
    return FALSE;
  }

  if(NIB_SendCmd(NIB_GET_CIMI, NIB_RES_OK, NIB_RES_ERROR, NIB_RES_TIMEOUT) == RES_SUCCESS)
  {
    pToken = MapForward(NBL_Rcv_Buffer,
                        (unsigned char*)"\n", 1);
    if(pToken != NULL)
    {
      for(i = 0; i < 15; i++)
      {
        imsi[i] = pToken[i+1];
      }
      imsi[15] = '\0';
    }
  }
  else
  {
    return FALSE;
  }

  if(NIB_SendCmd(NIB_GET_CGMM, NIB_RES_OK, NIB_RES_ERROR,
                 NIB_RES_TIMEOUT) != RES_SUCCESS)
  {
    return FALSE;
  }

  if(NIB_SendCmd(NIB_GET_CGMI, NIB_RES_OK, NIB_RES_ERROR,
                 NIB_RES_TIMEOUT) != RES_SUCCESS)
  {
    return FALSE;
  }

  if(NIB_SendCmd(NIB_FIRMWARE_VER, NIB_RES_OK, NIB_RES_ERROR,
                 NIB_RES_TIMEOUT) != RES_SUCCESS)
  {
    return FALSE;
  }

  if(NIB_SendCmd(NIB_SIM_READ_BINARY, NIB_RES_OK, NIB_RES_ERROR,
                 NIB_RES_TIMEOUT) != RES_SUCCESS)
  {
    return FALSE;
  }

  if(NIB_SendCmd(NIB_SIM_UPDATE_BINARY, NIB_RES_OK, NIB_RES_ERROR,
                 NIB_RES_TIMEOUT) != RES_SUCCESS)
  {
    return FALSE;
  }

  if(NIB_SendCmd(NIB_ERR_MSG_FORMATE, NIB_RES_OK, NIB_RES_ERROR,
                 NIB_RES_TIMEOUT) != RES_SUCCESS)
  {
    return FALSE;
  }

  if(NIB_SendCmd(/*g_gpBuff*/"AT+CGDCONT=1,\"IP\",\"onomondo\"\r\n",
                 NIB_RES_OK, NIB_RES_ERROR, NIB_RES_TIMEOUT) != RES_SUCCESS)
  {
    return FALSE;
  }

  if(NIB_SendCmd(NIB_CONFIG_RAT_SEARCH_SEQ, NIB_RES_OK, NIB_RES_ERROR,
                 NIB_RES_TIMEOUT) != RES_SUCCESS)
  {
    return FALSE;
  }

  if(NIB_SendCmd(NIB_CMD_CFUN1, NIB_RES_OK, NIB_RES_ERROR,
                 NIB_RES_TIMEOUT) != RES_SUCCESS)
  {
    return FALSE;
  }

  return TRUE;
}

/*************************************************************************
* Function Name: NIB_connectServer
* Parameters:none

*
* Return:none
*
* Description: Check for valid response is received or not from nimbelink
*
*************************************************************************/
uint8_t NIB_connectServer(void)
{
  uint8_t* pToken = NULL;
  uint8_t csq = 0;

  gsmReg_timeOut = 180; //3 min - 180 sec

  //check CSQ signal
  //wait until not getting 99,99
  while(--gsmReg_timeOut)
  {
    // network signal strength db
    if(NIB_SendCmd(NIB_GET_CSQ, NIB_RES_OK, NIB_RES_ERROR,
                   NIB_RES_TIMEOUT) == RES_SUCCESS)
    {
      //check CSQ value
      pToken = MapForward(NBL_Rcv_Buffer,
                          (uint8_t *)"+CSQ: ", 6);
      if(pToken != NULL)
      {
        csq = atoi((const char *)(pToken + 6));
        if(csq < 32)
          break;
      }

    }
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }

  //check CREG Value
  gsmReg_timeOut = 180;
  while(--gsmReg_timeOut)
  {
    if(NIB_SendCmd(NIB_NET_QUERY_CREG, NIB_RES_OK,
                   NIB_RES_ERROR, NIB_RES_TIMEOUT) == RES_SUCCESS)
    {
      //check CREG value
      pToken = MapForward(NBL_Rcv_Buffer, "+CREG: 0,", 9);
      if(pToken != NULL)
      {
        csq = atoi((const char *)(pToken + 9));
        //check CREG value 0,1 or 0,5
        if((csq == 1 || csq == 5))
          break; //Network detected
      }
    }
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }

  if(NIB_SendCmd(NIB_QUERY_CGATT_PS, NIB_RES_OK,
                 NIB_RES_ERROR, NIB_RES_TIMEOUT) == RES_SUCCESS)
  {
    pToken = MapForward(NBL_Rcv_Buffer, "+CGATT: ", 8);
    if(pToken != NULL)
    {
      csq = atoi((const char *)(pToken + 8));
      if(csq == 0)
      {
        NIB_SendCmd(NIB_SET_CGATT_PS, NIB_RES_OK,
                    NIB_RES_ERROR, NIB_RES_TIMEOUT);
      }
    }
  }


  if(NIB_SendCmd(NIB_QUERY_PDP_CONTEXT, NIB_RES_OK,
                 NIB_RES_ERROR, NIB_RES_TIMEOUT) == RES_SUCCESS)
  {
    pToken = MapForward(NBL_Rcv_Buffer, "+CGACT: 1,", 10);
    if(pToken != NULL)
    {
      csq = atoi((const char *)(pToken + 10));
      if(csq == 0)
      {
        NIB_SendCmd(NIB_SET_PDP_CONTEXT, NIB_RES_OK,
                    NIB_RES_ERROR, NIB_RES_TIMEOUT);
      }
    }
  }

  //Check Connected IP Address
  gsmReg_timeOut = 180;
  do
  {
    vTaskDelay(1000/portTICK_PERIOD_MS);
    if(NIB_SendCmd(NIB_CONTEXT_IP_ADD, NIB_RES_OK,
                   NIB_RES_ERROR, NIB_RES_TIMEOUT) == RES_SUCCESS)
    {
      if(strstr((char const*)NBL_Rcv_Buffer, NIB_CONTEXT_IP_NULL) == NULL)
      {
        break;
      }
    }
    --gsmReg_timeOut;
  }while(gsmReg_timeOut);

  //connect soket to staging server
  if(NIB_SendCmd(NIB_SOCKET_DIAL_STAGING, NIB_SOCKET_DAIL_OK,
                 NIB_RES_ERROR, NIB_RES_TIMEOUT) != RES_SUCCESS)
  {
    return FALSE;
  }

  return TRUE;
}
/*************************************************************************
* Function Name: NIB_Clearbuffer
* Parameters: None
* Return: none
*
* Description: Clear NIMBELINK Receive buffer and index
*
*
*************************************************************************/
void NIB_Clearbuffer(void)
{
  /*Clear the Buffer*/
  memset(NBL_Rcv_Buffer, NULL, MAX_NBL_RECV_DATA_LEN);
  NBL_Rcv_BufferIndex = 0;
}
/*************************************************************************
* Function Name: NIB_Enable_IRQ
* Parameters: None
* Return: none
*
* Description: enable UART-2 interrupt register not empty flag
*
*
*************************************************************************/
void NIB_Enable_IRQ(void)
{
  // enable UART2 interrupt for receive character
  __HAL_UART_ENABLE_IT(&huart1,UART_FLAG_RXNE);
}
/*************************************************************************
* Function Name: NIB_test
* Parameters: None
* Return: none
*
* Description: test NIMBELINK Module sending command
*
*
*************************************************************************/
void NIB_test(void)
{
  //power on Module
   //NIB_SendCmd(NIB_SET_ECHO_OFF, "OK", "ERROR", NIB_MAX_RES_TIMEOUT);
  // send ATEO command for echo  off
  // wait for command responce

}
/**
* @brief This function handles USART2 global interrupt.
*/
void USART1_IRQHandler(void)
{
  // check interrupt is occur or not
  if((__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET)
     && (__HAL_UART_GET_IT_SOURCE(&huart1, UART_IT_RXNE) != RESET))
  {
    /*Read data from interrupt and copy it to data variable*/
    uint8_t data = (uint8_t)(huart1.Instance->DR) & (uint8_t)0x00FF;

    if(NBL_Rcv_BufferIndex >= MAX_NBL_RECV_DATA_LEN)
    {
      NBL_Rcv_BufferIndex = 0;
    }
    //store received character into buffer
    NBL_Rcv_Buffer[NBL_Rcv_BufferIndex++] = data;

  }
  // flush (clear) DR REGISTER
  __HAL_UART_FLUSH_DRREGISTER(&huart1);
  // clear interrupt for next interrupt
  __HAL_UART_CLEAR_FLAG(&huart1,UART_FLAG_RXNE);

  if(__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE) != RESET)
  {
    __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_ORE);
  }

}
/* check nimbelink receiver flag is TRUE/FALSE in IRQ */
/* compare Received character of array with command */
/* if match command then TEST nimbelink */
/* if not matched command then print Enter valid command */

uint8_t* MapForward(uint8_t* buff, uint8_t* buff_find, uint8_t len)
{
  uint8_t* ptr = NULL;
  uint8_t i = 0, j = 0;
  for(i = 0; i < MAX_NBL_RECV_DATA_LEN; i++)
  {
    for(j = 0; j < len; j++)
    {
      if(buff[i + j] != buff_find[j])
      {
        break;
      }
    }
    if(j == len)
    {
      ptr = &buff[i];
      return ptr;
    }
  }

  return ptr;
}

/*************************************************************************
* Function Name: NIB_CMD_Process
* Parameters: None
* Return: none
*
* Description: Perfrom operation based on command received from terminal
*
*************************************************************************/
void NIB_CMD_Process(void)
{
  // device is ready and debug command received nimbelink test command
    // send command for test nimbelink
    NIB_getInfo();
}

///*************************************************************************
//* Function Name: DBG_Clearbuffer
//* Parameters: None
//* Return: none
//*
//* Description: Clear Debug Receive buffer and index
//*
//*
//*************************************************************************/
//void DBG_Clearbuffer(void)
//{
//  /*Clear the Buffer*/
//  memset((void *)Dbg_Rcv_Beffer, NULL, MAX_DBG_RECV_DATA_LEN);
//  Dbg_Rcv_Beffer_Index = 0;
//}

