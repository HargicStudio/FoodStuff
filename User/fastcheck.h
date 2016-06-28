
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _FastCheck_h
#define _FastCheck_h

#ifdef __cplusplus
extern "C" {
#endif



#include "stm32f1xx_hal.h"



/* User can use this section to tailor UART_USB/UARTx instance used and associated
   resources */
/* Definition for UART_USB clock resources */
#define UART_USB                           UART5
#define UART_USB_CLK_ENABLE()              __HAL_RCC_UART5_CLK_ENABLE()
#define UART_USB_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define UART_USB_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

#define UART_USB_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define UART_USB_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for UART_USB Pins */
#define UART_USB_TX_PIN                    GPIO_PIN_12
#define UART_USB_TX_GPIO_PORT              GPIOC
#define UART_USB_RX_PIN                    GPIO_PIN_2
#define UART_USB_RX_GPIO_PORT              GPIOD

/* Definition for UART_USB's NVIC */
#define UART_USB_IRQn                      UART5_IRQn
#define UART_USB_IRQHandler                UART5_IRQHandler



/**
  * @brief  start led from system level
  * @param  none
  * @retval None
  */
void StartFastCheckTask();
void HAL_UART_USB_MspInit(UART_HandleTypeDef *huart);
void HAL_UART_USB_MspDeInit(UART_HandleTypeDef *huart);
void HAL_UART_USB_TxCpltCallback(UART_HandleTypeDef *UartHandle);
void HAL_UART_USB_RxCpltCallback(UART_HandleTypeDef *UartHandle);
void HAL_UART_USB_ErrorCallback(UART_HandleTypeDef *UartHandle);



#ifdef __cplusplus
}
#endif

#endif /* _FastCheck_h */




