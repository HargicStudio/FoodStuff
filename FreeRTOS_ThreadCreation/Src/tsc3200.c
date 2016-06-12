

/***

History:
[2016-05-21 Ted]: Create

*/


#include "tsc3200.h"
#include <stdio.h>
#include "cmsis_os.h"
#include "AaInclude.h"



/** RunLedThread handler id */  
osThreadId _runtsc3200_id;


u16  Rgena,Ggena,Bgena;
u32  Ramount,Gamount,Bamount;
u16  amount;


/**
  * @brief  Configures EXTI line 0 (connected to PA.08 pin) in interrupt mode
  * @param  None
  * @retval None
  */
static void TCS3200_OutGpioConfig(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Enable GPIOA clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* Configure PA.08 pin as input floating */
  GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = GPIO_PIN_8;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Enable and set EXTI line 0 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  
    /* Configure PA.09 pin as input floating */
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure.Pull = GPIO_PULLUP;
  GPIO_InitStructure.Pin = GPIO_PIN_9;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
  
}

static void TCS3200_LedGpioConfig(void)
{
  GPIO_InitTypeDef   GPIO_InitStruct;
  /* -1- Enable GPIO Clock (to be able to program the configuration registers) */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  
  /* -2- Configure IO in output push-pull mode to drive external LEDs */
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  
  /* 暂时初始补光灯全部开启 */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
  
}

static void TCS3200_CtrlGpioConfig(unsigned char s0, unsigned char s1)
{
    GPIO_InitTypeDef   GPIO_InitStruct;

    /* TCS_S0_PIN OUTPUT */
    TCS_S0_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin = TCS_S0_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TCS_S0_GPIO_PORT, &GPIO_InitStruct);
    
    /* TCS_S1_PIN OUTPUT */
    TCS_S1_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin = TCS_S1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TCS_S1_GPIO_PORT, &GPIO_InitStruct);
    
    /* TCS_S2_PIN OUTPUT */
    TCS_S2_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin = TCS_S2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TCS_S2_GPIO_PORT, &GPIO_InitStruct);
    
    /* TCS_S3_PIN OUTPUT */
    TCS_S3_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin = TCS_S3_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(TCS_S3_GPIO_PORT, &GPIO_InitStruct);
  
    if (s0)
    {
      TCS_S0_Set();
    }
    else
    {
      TCS_S0_Clr();
    }
    
    if (s1)
    {
      TCS_S1_Set();
    }
    else
    {
      TCS_S1_Clr();
    }
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_8)
  {
      amount++;
  }
}


static void TCS3200_init(void)
{
  TCS3200_CtrlGpioConfig(1, 1);
  TCS3200_LedGpioConfig();
  TCS3200_OutGpioConfig();
}


void TCS3200_delayMs(unsigned int nms)
{
  HAL_Delay(nms);
  //osDelay(nms);
}

/*******************************************
*
*		  白平衡
*
*******************************************/
void TCS3200_whitebalance(void)
{
    //红色通道
    TCS_S2_Clr();
    TCS_S3_Clr();
	amount=0;			 //开始计数
	TCS3200_delayMs(10);
	Rgena = amount;      //求出红色因子      
	amount=0;
//----------------------------------
    //绿色通道
    TCS_S2_Set();
    TCS_S3_Set();
    
	amount=0;
	TCS3200_delayMs(10);
	Ggena = amount;	 //求出绿色因子
	amount=0;
//----------------------------------
    //蓝色通道
    TCS_S2_Clr();
    TCS_S3_Set();
	amount=0;
	TCS3200_delayMs(10);
	Bgena = amount;	  //求出蓝色因子
	amount=0;

    //关闭通道  
    TCS_S2_Set();
    TCS_S3_Clr();
}  

u16 TCS3200_RED(void)
{
    TCS_S2_Clr();
    TCS_S3_Clr();
	amount=0;
	TCS3200_delayMs(10);
	Ramount=(u32) amount*255/Rgena;	 //取R值
    //printf("[TCS3200_RED] amount = %d, Ramount = %d\n\r", amount, Ramount); 
	if(Ramount>255) Ramount = 255;
	return Ramount;
}

u16 TCS3200_GREEN(void)
{
    TCS_S2_Set();
    TCS_S3_Set();
	amount=0;
	TCS3200_delayMs(10);
	Gamount=(u32) amount*255/Ggena;	//取G值
    //printf("[TCS3200_GREEN] amount = %d, Gamount = %d\n\r", amount, Gamount);
	if(Gamount>255) Gamount = 255;
	return Gamount;
}

u16 TCS3200_BLUE(void)
{

    TCS_S2_Clr();
    TCS_S3_Set();
	amount=0;
	TCS3200_delayMs(10);
	Bamount=(u32) amount*255/Bgena;//去B值
    //printf("[TCS3200_BLUE] amount = %d, Bamount = %d\n\r", amount, Bamount);
	if(Bamount>255) Bamount = 255;
	return Bamount;
} 


/**
  * @brief  run tsc3200 thread
  * @param  thread not used
  * @retval None
  */
static void RunTsc3200Thread(void const *argument)
{
  (void) argument;
   u8 R=0x00,G=0x00,B=0x00;
   
  AaSysLogPrint(LOGLEVEL_INF, FeatureTsc3200, "RunTsc3200Thread started");
  //AaTagRegister(AATAG_CCS_DEAMON_ONLINE, GetCCSOnlineTag);

  /*刚启动进行白平衡校正，需用对冲白底板 */
  TCS3200_whitebalance();
  AaSysLogPrint(LOGLEVEL_DBG, FeatureTsc3200, "<TCS3200_whitebalance>Rgena = %d, Ggena = %d, Bgena = %d",
                                               Rgena, Ggena, Bgena);
  for (;;)
  {
    osDelay(1000);
    R = TCS3200_RED();
    G = TCS3200_GREEN();	//取RGB值
    B = TCS3200_BLUE();
    AaSysLogPrint(LOGLEVEL_DBG, FeatureTsc3200, "[TCS3200] R = %d, G = %d, B = %d", R, G, B);
  }
}


/**
  * @brief  start led from system level
  * @param  none
  * @retval None
  */
void StartRunTsc3200Task()
{
    TCS3200_init();

    osThreadDef(RunTsc3200, RunTsc3200Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    _runtsc3200_id = AaThreadCreateStartup(osThread(RunTsc3200), NULL);
}




// end of file





