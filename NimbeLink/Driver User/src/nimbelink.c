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
char const Nib_Test_Cmd[] = "nibtest";                  // Nimbelink test command
char const App_Ready_s[] = "app ready";
uint8_t AppReadyFlag = 0;                               // app ready flag for indicate device is ready

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
  HAL_UART_Transmit(&huart1,(uint8_t *)data,len,5000);
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

  // send ATEO command for echo  off
  NIB_SendCmdLen(GSM_SET_ECHO_OFF,AUTO_CMD_LEN);
   //NIB_SendCmdLen(GSM_SET_ECHO_OFF,AUTO_CMD_LEN);

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

    // app ready flag status update
    if(strstr((const char *)NBL_Rcv_Buffer, App_Ready_s) != NULL)
    {
      AppReadyFlag = 1;
    }
    //    NBL_Rcv_BufferIndex = 0;

    // flush (clear) DR REGISTER
    __HAL_UART_FLUSH_DRREGISTER(&huart1);
    // clear interrupt for next interrupt
    __HAL_UART_CLEAR_FLAG(&huart1,UART_FLAG_RXNE);
  }
}
/* check nimbelink receiver flag is TRUE/FALSE in IRQ */
/* compare Received character of array with command */
/* if match command then TEST nimbelink */
/* if not matched command then print Enter valid command */


//char* strFind(const char* mainStr, const char* searchStr)
//{
//  const char *p1 = mainStr;
//  const char *p2 = searchStr;
//
//  // check main array char dont have a null character
//  while(*p1 != '\0')
//  {
//    // compare and check sub array char not null and match with main array char
//    while(*p2 != '\0' && *p2 == *p1)
//    {
//      p1++;
//      p2++;
//    }
//    // if sub array char receive null char then return match index value
//    if(*p2 == '\0')
//    {
//      return (char*)*p1;
//    }
//    p1++;
//  }
//  return NULL;
//}
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
     // NIB_test();
  // perform test based on received command from terminal
  if(NIB_Test_Enable_f == TRUE)
  {
    NIB_test();
    //compare received charcter of array with NIB_TEST_CMD command
      if(strcasecmp((char const *)Dbg_Rcv_Beffer,Nib_Test_Cmd) == 0)
      {
        // test the Nimbelink
        NIB_test();
        NIB_Test_Enable_f = FALSE;
      }
      // command is not valid
      else
      {
        dprintf("Enter Valid Command");
        NIB_Test_Enable_f = FALSE;
      }
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

