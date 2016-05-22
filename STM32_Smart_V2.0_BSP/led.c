

/***

History:
[2016-05-21 Ted]: Create

*/


#include "led.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>


/**
  * @brief  Configures LED GPIO.
  * @param  Led: LED to be configured. 
  *          This parameter can be one of the following values:
  *     @arg LED
  * @retval None
  */
void LED_Init(void)
{
  GPIO_InitTypeDef  gpioinitstruct = {0};
  
  /* Enable the GPIO_LED Clock */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* Configure the GPIO_LED pin */
  gpioinitstruct.Pin    = GPIO_PIN_13;
  gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
  gpioinitstruct.Pull   = GPIO_NOPULL;
  gpioinitstruct.Speed  = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &gpioinitstruct);

  /* Reset PIN to switch off the LED */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);

  AaSysLogPrint(LOGLEVEL_INF, SystemStartup, "led initialize success");
}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on. 
  *   This parameter can be one of following parameters:
  *     @arg LED2
  * @retval None
  */
void LED_On()
{
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); 
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off. 
  *   This parameter can be one of following parameters:
  *     @arg LED2
  * @retval None
  */
void LED_Off()
{
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); 
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled. 
  *   This parameter can be one of following parameters:
  *            @arg  LED2
  * @retval None
  */
void LED_Toggle(void)
{
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}



// end of file





