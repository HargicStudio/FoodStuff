

/***

History:
[2016-05-21 Ted]: Create

*/


#include "fastcheck.h"
#include "cmsis_os.h"
#include "AaInclude.h"



/** FastCheckThread handler id */  
osThreadId _fastcheck_id;

UART_HandleTypeDef UartHandle_usb;


/**
  * @brief  run led thread
  * @param  thread not used
  * @retval None
  */
static void FastCheckThread(void const *argument)
{
  (void) argument;

  AaSysLogPrint(LOGLEVEL_INF, FeatureFstChk, "FastCheckThread started");

//  AaTagRegister(AATAG_CCS_DEAMON_ONLINE, GetCCSOnlineTag);

  for (;;)
  {
  }
}

/**
  * @brief  start led from system level
  * @param  none
  * @retval None
  */
void StartFastCheckTask()
{

    osThreadDef(FastCheck, FastCheckThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    _fastcheck_id = AaThreadCreateStartup(osThread(FastCheck), NULL);
}

/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - DMA configuration for transmission request by peripheral 
  *           - NVIC configuration for DMA interrupt request enable
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_USB_MspInit(UART_HandleTypeDef *huart)
{  
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  UART_USB_TX_GPIO_CLK_ENABLE();
  UART_USB_RX_GPIO_CLK_ENABLE();


  /* Enable UART_USB clock */
  UART_USB_CLK_ENABLE();
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = UART_USB_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(UART_USB_TX_GPIO_PORT, &GPIO_InitStruct);

  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = UART_USB_RX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;

  HAL_GPIO_Init(UART_USB_RX_GPIO_PORT, &GPIO_InitStruct);
  
  /* NVIC for USART, to catch the TX complete */
  HAL_NVIC_SetPriority(UART_USB_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(UART_USB_IRQn);
}

/**
  * @brief UART MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_USB_MspDeInit(UART_HandleTypeDef *huart)
{

  /*##-1- Reset peripherals ##################################################*/
  UART_USB_FORCE_RESET();
  UART_USB_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure UART_USB Tx as alternate function  */
  HAL_GPIO_DeInit(UART_USB_TX_GPIO_PORT, UART_USB_TX_PIN);
  /* Configure UART_USB Rx as alternate function  */
  HAL_GPIO_DeInit(UART_USB_RX_GPIO_PORT, UART_USB_RX_PIN);
}

/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of DMA Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_UART_USB_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* Set transmission flag: trasfer complete*/
  AaSysLogSendCplt();
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_USB_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /* Set transmission flag: trasfer complete*/
  
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_USB_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
}



// end of file





