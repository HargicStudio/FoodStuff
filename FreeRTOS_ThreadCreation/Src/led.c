

/***

History:
[2016-05-21 Ted]: Create

*/


#include "led.h"
#include <stdio.h>
#include "cmsis_os.h"
#include "AaInclude.h"



/** RunLedThread handler id */  
osThreadId _runled_id;



static void LedDeviceInit(void);
static void LedOn();
static void LedOff();
static void LedToggle();
static void GetCCSOnlineTag(u32 value);



/**
  * @brief  run led thread
  * @param  thread not used
  * @retval None
  */
static void RunLedThread(void const *argument)
{
  (void) argument;

  AaSysLogPrint(LOGLEVEL_INF, SystemStartup, "RunLedThread started");

  AaTagRegister(AATAG_CCS_DEAMON_ONLINE, GetCCSOnlineTag);

  for (;;)
  {
      LedToggle();
      osDelay(1000);
      AaSysLogPrint(LOGLEVEL_DBG, SystemStartup, "System running");
  }
}


/**
  * @brief  start led from system level
  * @param  none
  * @retval None
  */
void StartRunLedTask()
{
    LedDeviceInit();

    osThreadDef(RunLed, RunLedThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    _runled_id = AaThreadCreateStartup(osThread(RunLed), NULL);
}


/**
  * @brief  initialize LED GPIO.
  * @param  none
  * @retval None
  */
static void LedDeviceInit()
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
  LedOff();

  AaSysLogPrint(LOGLEVEL_INF, SystemStartup, "led device initialize success");
}


/**
  * @brief  Turns selected LED On.
  * @param  none
  * @retval None
  */
static void LedOn()
{
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); 
}

/**
  * @brief  Turns selected LED Off.
  * @param  none
  * @retval None
  */
static void LedOff()
{
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET); 
}

/**
  * @brief  Toggles the selected LED.
  * @param  none
  * @retval None
  */
static void LedToggle()
{
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}


static void GetCCSOnlineTag(u32 value)
{
    AaSysLogPrint(LOGLEVEL_DBG, SystemStartup, "get callback of ccs.online state changed %d", value);
}



// end of file





