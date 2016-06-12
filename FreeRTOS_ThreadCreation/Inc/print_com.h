
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PRINT_COM_H
#define __PRINT_COM_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f1xx_hal.h"
#include "AaInclude.h"




/* User can use this section to tailor USARTx/UARTx instance used and associated
   resources */
/* Definition for USARTx clock resources */
#define USARTx                           UART5
#define USARTx_CLK_ENABLE()              __HAL_RCC_UART5_CLK_ENABLE()
#define DMAx_CLK_ENABLE()                //__HAL_RCC_DMA1_CLK_ENABLE()
#define USARTx_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_12
#define USARTx_TX_GPIO_PORT              GPIOC
#define USARTx_RX_PIN                    GPIO_PIN_2
#define USARTx_RX_GPIO_PORT              GPIOD

/* Definition for USARTx's DMA */

#define USARTx_TX_DMA_CHANNEL             //DMA1_Channel4
#define USARTx_RX_DMA_CHANNEL             //DMA1_Channel5



/* Definition for USARTx's NVIC */
#define USARTx_DMA_TX_IRQn                //DMA1_Channel4_IRQn
#define USARTx_DMA_RX_IRQn                //DMA1_Channel5_IRQn
#define USARTx_DMA_TX_IRQHandler          //DMA1_Channel4_IRQHandler
#define USARTx_DMA_RX_IRQHandler          //DMA1_Channel5_IRQHandler

/* Definition for USARTx's NVIC */
#define USARTx_IRQn                      UART5_IRQn
#define USARTx_IRQHandler                UART5_IRQHandler


/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


void StdUsartInit(void);
void GetBipAndSendByDMA(char* addr, u32 len);
void GetBipAndSendByIT(char* addr, u32 len);
void GetBipAndSendByPolling(char* addr, u32 len);




#ifdef __cplusplus
}
#endif

#endif /* __PRINT_COM_H */



