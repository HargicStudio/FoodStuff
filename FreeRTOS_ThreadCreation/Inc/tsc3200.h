
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "AaInclude.h"

#define   TCS_S0_PIN                  GPIO_PIN_0
#define   TCS_S0_GPIO_PORT            GPIOC
#define   TCS_S0_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOC_CLK_ENABLE()
#define   TCS_S0_GPIO_CLK_DISABLE()   __HAL_RCC_GPIOC_CLK_DISABLE()

#define   TCS_S1_PIN                 GPIO_PIN_1   
#define   TCS_S1_GPIO_PORT           GPIOC
#define   TCS_S1_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOC_CLK_ENABLE()
#define   TCS_S1_GPIO_CLK_DISABLE()  __HAL_RCC_GPIOC_CLK_DISABLE()

#define   TCS_S2_PIN                 GPIO_PIN_2  
#define   TCS_S2_GPIO_PORT           GPIOC
#define   TCS_S2_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOC_CLK_ENABLE()
#define   TCS_S2_GPIO_CLK_DISABLE()  __HAL_RCC_GPIOC_CLK_DISABLE()

#define   TCS_S3_PIN                GPIO_PIN_3 
#define   TCS_S3_GPIO_PORT          GPIOC
#define   TCS_S3_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()
#define   TCS_S3_GPIO_CLK_DISABLE() __HAL_RCC_GPIOC_CLK_DISABLE()

#define    TCS_S0_Clr()       HAL_GPIO_WritePin(TCS_S0_GPIO_PORT, TCS_S0_PIN, GPIO_PIN_RESET)
#define    TCS_S0_Set()       HAL_GPIO_WritePin(TCS_S0_GPIO_PORT, TCS_S0_PIN, GPIO_PIN_SET)

#define    TCS_S1_Clr()       HAL_GPIO_WritePin(TCS_S1_GPIO_PORT, TCS_S1_PIN, GPIO_PIN_RESET)
#define    TCS_S1_Set()       HAL_GPIO_WritePin(TCS_S1_GPIO_PORT, TCS_S1_PIN, GPIO_PIN_SET)

#define    TCS_S2_Clr()       HAL_GPIO_WritePin(TCS_S2_GPIO_PORT, TCS_S2_PIN, GPIO_PIN_RESET) 
#define    TCS_S2_Set()       HAL_GPIO_WritePin(TCS_S2_GPIO_PORT, TCS_S2_PIN, GPIO_PIN_SET) 

#define    TCS_S3_Clr()       HAL_GPIO_WritePin(TCS_S3_GPIO_PORT, TCS_S3_PIN, GPIO_PIN_RESET) 
#define    TCS_S3_Set()       HAL_GPIO_WritePin(TCS_S3_GPIO_PORT, TCS_S3_PIN, GPIO_PIN_SET)


void TCS3200_delayMs(unsigned int nms);
void TCS3200_whitebalance(void);
u16 TCS3200_RED(void);
u16 TCS3200_GREEN(void);
u16 TCS3200_BLUE(void);
static void TCS3200_OutGpioConfig(void);

/**
  * @brief  start TSC3200 from system level
  * @param  none
  * @retval None
  */
void RunTsc3200Thread();



#ifdef __cplusplus
}
#endif

#endif /* __LED_H */




