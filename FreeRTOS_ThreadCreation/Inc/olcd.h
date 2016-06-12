
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OLCD_H
#define __OLCD_H

#ifdef __cplusplus
extern "C" {
#endif



#include "stm32f1xx_hal.h"



#define OLED_MODE   0
#define XLevelL     0x00
#define XLevelH     0x10
#define OLCD_MAX_COLUMN     128
#define Max_Row     64
#define Brightness  0xFF 
#define X_WIDTH     128
#define Y_WIDTH     64  



/* Definition for SPIx clock resources */
#define SPIx                             SPI2
#define SPIx_CLK_ENABLE()                __HAL_RCC_SPI2_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPIx_MISO_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPIx_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

/* Definition for SPIx Pins */
#define SPIx_SCK_PIN                     GPIO_PIN_13
#define SPIx_SCK_GPIO_PORT               GPIOB
#define SPIx_MISO_PIN                    GPIO_PIN_14
#define SPIx_MISO_GPIO_PORT              GPIOB
#define SPIx_MOSI_PIN                    GPIO_PIN_15
#define SPIx_MOSI_GPIO_PORT              GPIOB

/* Definition for SPIx's NVIC */
#define SPIx_IRQn                        SPI2_IRQn
#define SPIx_IRQHandler                  SPI2_IRQHandler



u8 StartOlcdTask();



#ifdef __cplusplus
}
#endif

#endif /* __OLCD_H */




