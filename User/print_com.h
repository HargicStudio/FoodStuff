
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PRINT_COM_H
#define __PRINT_COM_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f1xx_hal.h"
#include "AaInclude.h"




/* User can use this section to tailor USART_DBG/UARTx instance used and associated
   resources */
/* Definition for USART_DBG clock resources */
#define USART_DBG                           USART1
#define USART_DBG_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE()
#define DMA_DBG_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define USART_DBG_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART_DBG_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define USART_DBG_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define USART_DBG_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for USART_DBG Pins */
#define USART_DBG_TX_PIN                    GPIO_PIN_9
#define USART_DBG_TX_GPIO_PORT              GPIOA
#define USART_DBG_RX_PIN                    GPIO_PIN_10
#define USART_DBG_RX_GPIO_PORT              GPIOA

/* Definition for USART_DBG's DMA */

#define USART_DBG_TX_DMA_CHANNEL             DMA1_Channel4
#define USART_DBG_RX_DMA_CHANNEL             DMA1_Channel5



/* Definition for USART_DBG's NVIC */
#define USART_DBG_DMA_TX_IRQn                DMA1_Channel4_IRQn
#define USART_DBG_DMA_RX_IRQn                DMA1_Channel5_IRQn
#define USART_DBG_DMA_TX_IRQHandler          DMA1_Channel4_IRQHandler
#define USART_DBG_DMA_RX_IRQHandler          DMA1_Channel5_IRQHandler

/* Definition for USART_DBG's NVIC */
#define USART_DBG_IRQn                      USART1_IRQn
#define USART_DBG_IRQHandler                USART1_IRQHandler


/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


void StdUsartInit(void);
void GetBipAndSendByDMA(char* addr, u32 len);
void GetBipAndSendByIT(char* addr, u32 len);
void GetBipAndSendByPolling(char* addr, u32 len);
void HAL_UART_DBG_MspInit(UART_HandleTypeDef *huart);
void HAL_UART_DBG_MspDeInit(UART_HandleTypeDef *huart);
void HAL_UART_DBG_TxCpltCallback(UART_HandleTypeDef *UartHandle);
void HAL_UART_DBG_RxCpltCallback(UART_HandleTypeDef *UartHandle);
void HAL_UART_DBG_ErrorCallback(UART_HandleTypeDef *UartHandle);



#ifdef __cplusplus
}
#endif

#endif /* __PRINT_COM_H */



