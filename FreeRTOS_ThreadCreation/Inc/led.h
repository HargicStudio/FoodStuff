
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C" {
#endif



#include "stm32f1xx_hal.h"


/**
  * @brief  start led from system level
  * @param  none
  * @retval None
  */
void StartRunLedTask();



#ifdef __cplusplus
}
#endif

#endif /* __LED_H */




