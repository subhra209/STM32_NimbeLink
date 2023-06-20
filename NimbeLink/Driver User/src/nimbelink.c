/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_hal.h"
#include "nimbelink.h"


extern UART_HandleTypeDef huart2;

uint8_t NBL_Rcv_Buffer[MAX_NBL_RECV_DATA_LEN];          // receive buffer for nimbelink module
uint8_t NBL_Rcv_BufferIndex = 0;                        // buffer index

// create function for sending command  and length
// printf command in terminal
// create function for write command to nimbelink module
// wait for command respose from nimbelink module
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
void NIB_Write(const void *data, unsigned len)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)data, len, 1000);
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
void NIB_SendCmdLen(const char *cmd,uint16_t Len)
{
  // find out length of string
  if(Len == AUTO_CMD_LEN){
    Len = strlen(cmd);
  }

  // clear receiver buffer
  NIB_Clearbuffer();
  // print command in terminal
  dprintf("NIMBLINK <--- %s", cmd);
  // send command to nimbelink module
  NIB_Write(cmd, Len);
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
uint8_t  NIB_Res_Ok(void)
{
  // command receive valid data
  if(strcmp((char const *)NBL_Rcv_Buffer,"OK") == 0)
  {
    dprintf("NIMBLINK ---> %s", NBL_Rcv_Buffer);
    return TRUE;
  }
  // command received invalid data
  return FALSE;
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

void NIB_Enable_IRQ(void)
{
  // enable UART2 interrupt for receive character
  __HAL_UART_ENABLE_IT(&huart2,UART_FLAG_RXNE);
}

void NIB_test(void)
{
  uint8_t timeout = 3;
  uint8_t status = FALSE;
  //power on Module

  //enable UART IRQ
  NIB_Enable_IRQ();
  // send ATEO command for echo  off
  NIB_SendCmdLen(GSM_SET_ECHO_OFF,AUTO_CMD_LEN);

  // wait for command responce
  while(timeout)
  {
    status = NIB_Res_Ok();
    if(status == TRUE)
    {
      break;
    }
    vTaskDelay(100/ portTICK_PERIOD_MS);
    --timeout;
  }

  if(status == FALSE)
  {
    dprintf("NIMBLINK data not recived");
  }
}
/**
* @brief This function handles USART2 global interrupt.
*/
void USART2_IRQHandler(void)
{
  // check interrupt is occur or not
  if((__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET)
     && (__HAL_UART_GET_IT_SOURCE(&huart2, UART_IT_RXNE) != RESET))
  {
    /*Read data from interrupt and copy it to data variable*/
    uint8_t data = (uint8_t)(huart2.Instance->DR);

    //store received character into buffer
    NBL_Rcv_Buffer[NBL_Rcv_BufferIndex++] = data;

    // module send termination char then index set to 0
//    if(data == '\0')
//    {
//      NBL_Rcv_BufferIndex = 0;
//    }

    __HAL_UART_FLUSH_DRREGISTER(&huart2);
        // clear interrupt for next interrupt
    __HAL_UART_CLEAR_FLAG(&huart2,UART_FLAG_RXNE);
  }


}


