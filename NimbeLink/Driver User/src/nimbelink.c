/* Includes ------------------------------------------------------------------*/
#include "string.h"
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

char const App_Ready_s[] = "app ready";
uint8_t AppReadyFlag = 0;                               // app ready flag for indicate device is ready


uint8_t* findBuff(uint8_t* buff, uint8_t* buff_find, uint8_t len);
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
    if(findBuff(NBL_Rcv_Buffer, (uint8_t*)pRes1, strlen(pRes1)) != NULL)
    {
      dprintf("NIMBLINK ---> %s", NBL_Rcv_Buffer);
      return RES_SUCCESS;
    }
    if(findBuff(NBL_Rcv_Buffer, (uint8_t*)pRes2, strlen(pRes2)) != NULL)
    {
      dprintf("NIMBLINK ---> %s", NBL_Rcv_Buffer);
      return RES_ERROR;
    }
    --timeOut_s;
  }
  return RES_TIMEOUT;
}

/*************************************************************************
* Function Name: NIB_Res_Ok
* Parameters:none

*
* Return:none
*
* Description: Check for valid response is received or not from nimbelink
*
*************************************************************************/
uint8_t NIB_Res_Ok(void)
{
    //command receive valid data
    if(strstr((const char *)NBL_Rcv_Buffer,"OK") != NULL)
    {
      dprintf("NIMBLINK ---> %s", NBL_Rcv_Buffer);
      return TRUE;
    }
    else
    {
      // command received invalid data
      return FALSE;
    }
}

uint8_t NIB_powerOn(void)
{
  if(AppReadyFlag == FALSE)
  {
    if(findBuff(NBL_Rcv_Buffer, (uint8_t*)"APP RDY", 7) != NULL)
    {
      AppReadyFlag = TRUE;
      //send echo off cmd
      if(NIB_SendCmd(NIB_SET_ECHO_OFF, "OK", "ERROR", NIB_MAX_RES_TIMEOUT) == RES_SUCCESS)
      {
        NIB_getInfo();
      }
    }
  }
  return AppReadyFlag;
}

uint8_t NIB_getInfo(void)
{
  uint8_t status =  0;
  if(NIB_SendCmd(GSM_GET_IMEI, "OK", "ERROR", NIB_MAX_RES_TIMEOUT) != RES_SUCCESS)
  {
    status =  0;
  }

  if(NIB_SendCmd(GSM_GET_ICCID, "OK", "ERROR", NIB_MAX_RES_TIMEOUT) != RES_SUCCESS)
  {
    status =  0;
  }

  if(NIB_SendCmd(GSM_GET_CIMI, "OK", "ERROR", NIB_MAX_RES_TIMEOUT) != RES_SUCCESS)
  {
    status =  0;
  }

  if(NIB_SendCmd(GSM_GET_CGMM, "OK", "ERROR", NIB_MAX_RES_TIMEOUT) != RES_SUCCESS)
  {
    status =  0;
  }

  if(NIB_SendCmd(GSM_GET_CGMI, "OK", "ERROR", NIB_MAX_RES_TIMEOUT) != RES_SUCCESS)
  {
    status =  0;
  }

  if(NIB_SendCmd(GSM_FIRMWARE_VER, "OK", "ERROR", NIB_MAX_RES_TIMEOUT) != RES_SUCCESS)
  {
    status =  0;
  }

  return status;
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
  uint8_t timeout = 3;
  uint8_t status = FALSE;
  //power on Module
  //NIB_SendCmd(NIB_SET_ECHO_OFF, "OK", "ERROR", NIB_MAX_RES_TIMEOUT);
  // send ATEO command for echo  off
  // wait for command responce
  while(timeout)
  {
    // respose status (TRUE / FALSE)
    status = NIB_Res_Ok();
    // respose valid
    if(status == TRUE)
    {
      break;
    }
    vTaskDelay(100/ portTICK_PERIOD_MS);
    --timeout;
  }

  // respose not valid
  if(status == FALSE)
  {
    dprintf("NIMBLINK data not recived");
  }
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

    //store received character into buffer
    NBL_Rcv_Buffer[NBL_Rcv_BufferIndex++] = data;

//    if((NBL_Rcv_BufferIndex == 1) && (data == 0))
//    {
//      NBL_Rcv_BufferIndex = 0;
//    }

    // app ready flag status update
    if(strstr((const char *)NBL_Rcv_Buffer, App_Ready_s) != NULL)
    {
      AppReadyFlag = 1;
    }
    //    NBL_Rcv_BufferIndex = 0;
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

uint8_t* findBuff(uint8_t* buff, uint8_t* buff_find, uint8_t len)
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
  if(NIB_Test_Enable_f == TRUE)
  {
    // send command for test nimbelink
    NIB_getInfo();
    NIB_Test_Enable_f = FALSE;
  }
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

