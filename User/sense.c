

/***

History:
[2016-05-21 Ted]: Create

*/


#include "sense.h"
#include <stdio.h>
#include <stdbool.h>
#include "cmsis_os.h"
#include "AaInclude.h"



enum {
	TRANSFER_WAIT,
	TRANSFER_COMPLETE,
	TRANSFER_ERROR
};


#define CSLow()     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)
#define CSHigh()    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)



/** Description of the macro */  
#define ADCCONVERTEDVALUES_BUFFER_SIZE  16

/* Max value with a full range of 12 bits */
#define RANGE_12BITS                   ((u32) 4095)

/** Description of the macro */  
#define GetAbsDacVoltage(volt)        (volt*RANGE_12BITS/2.5)


/* ADC handler declaration */
ADC_HandleTypeDef    AdcHandle;


/* Variable containing ADC conversions results */
__IO uint16_t   aADCxConvertedValues[ADCCONVERTEDVALUES_BUFFER_SIZE];


/* Variable to report ADC analog watchdog status:   */
/*   RESET <=> voltage into AWD window   */
/*   SET   <=> voltage out of AWD window */
uint8_t         ubAnalogWatchdogStatus = RESET;  /* Set into analog watchdog interrupt callback */

/* SPI handler declaration */
SPI_HandleTypeDef SpiHandle;


/** RunLedThread handler id */  
osThreadId _sense_id;


/** Description of the macro */  
static osSemaphoreDef(adc_cplt_sem);
osSemaphoreId _adc_cplt_sem_id;


/** Description of the macro */  
static osSemaphoreDef(spi_cplt_sem);
osSemaphoreId _spi_cplt_sem_id;



static void SenseThread(void const *argument);
static u8 SenseDeviceInit();
static void ADC_Config(void);
static void SPI_Config();
static void SPICSInit();



/**
  * @brief  sense handle thread
  * @param  thread not used
  * @retval None
  */
static void SenseThread(void const *argument)
{
    (void) argument;
    
    u16 i;
    u16 adc_dr;
    float voltage;

    float dac_volt = 0.01;
    u16 dac_conf;
    u8 dac_reg[2];

    AaSysLogPrint(LOGLEVEL_INF, FeatureSense, "Sense task started");

    for (;;)
    {
        dac_volt += 0.01;
        if(dac_volt >= 2.0) {
            dac_volt = 0.01;
        }
        dac_conf = 0x1000 | (u16)GetAbsDacVoltage(dac_volt);
        dac_reg[0] = (u8)(dac_conf >> 8);
        dac_reg[1] = (u8)(dac_conf & 0x00ff);
        AaSysLogPrint(LOGLEVEL_DBG, FeatureSense, "dac_volt %f, dac_conf %d(0x%04x), dac_reg[0] 0x%02x, dac_reg[1] 0x%02x", \
                dac_volt, dac_conf, dac_conf, dac_reg[0], dac_reg[1]);

        CSLow();
        if(HAL_SPI_Transmit_IT(&SpiHandle, dac_reg, 2) != HAL_OK) {
            /* Start Error */
            AaSysLogPrint(LOGLEVEL_WRN, FeatureSense, "DAC spi start transmit failed");
            continue;
        }
        osSemaphoreWait(_spi_cplt_sem_id, osWaitForever);
        CSHigh();


        /* Start ADC conversion on regular group with transfer by DMA */
        if (HAL_ADC_Start_DMA(&AdcHandle, (uint32_t *)aADCxConvertedValues, ADCCONVERTEDVALUES_BUFFER_SIZE ) != HAL_OK) {
            /* Start Error */
            AaSysLogPrint(LOGLEVEL_WRN, FeatureSense, "ADC start by dma failed");
            continue;
        }
        osSemaphoreWait(_adc_cplt_sem_id, osWaitForever);

        // get the average adc
        adc_dr = aADCxConvertedValues[0];
        for(i=1; i<ADCCONVERTEDVALUES_BUFFER_SIZE; i++) {
            adc_dr = (aADCxConvertedValues[i] + adc_dr)/2;
        }

        AaSysLogPrint(LOGLEVEL_DBG, FeatureSense, "adc_dr %d", adc_dr);

        voltage = adc_dr*3.3/RANGE_12BITS;
        
        AaSysLogPrint(LOGLEVEL_DBG, FeatureSense, "adc voltage %f", voltage);
        
        osDelay(1000);
    }
}


/**
  * @brief  start sense from system level
  * @param  none
  * @retval None
  */
u8 StartSenseTask()
{
    SenseDeviceInit();

    _adc_cplt_sem_id = osSemaphoreCreate(osSemaphore(adc_cplt_sem), 1);
    if(_adc_cplt_sem_id == NULL) {
        AaSysLogPrint(LOGLEVEL_ERR, FeatureSense, "%s %d: adc_cplt_sem initialize failed",
                __FUNCTION__, __LINE__);
        return 1;
    }
    AaSysLogPrint(LOGLEVEL_INF, FeatureSense, "create adc_cplt_sem success");
    

    _spi_cplt_sem_id = osSemaphoreCreate(osSemaphore(spi_cplt_sem), 1);
    if(_spi_cplt_sem_id == NULL) {
        AaSysLogPrint(LOGLEVEL_ERR, FeatureSense, "%s %d: spi_cplt_sem initialize failed",
                __FUNCTION__, __LINE__);
        return 1;
    }
    AaSysLogPrint(LOGLEVEL_INF, FeatureSense, "create spi_cplt_sem success");
    

    osThreadDef(Sense, SenseThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    _sense_id = AaThreadCreateStartup(osThread(Sense), NULL);
    if(_sense_id == NULL) {
        AaSysLogPrint(LOGLEVEL_ERR, FeatureSense, "%s %d: create Sense task failed",
                __FUNCTION__, __LINE__);
        return 2;
    }
    AaSysLogPrint(LOGLEVEL_INF, FeatureSense, "create Sense task success");

    return 0;
}


/** 
 * This is a brief description. 
 * This is a detail description. 
 * @param[in]   inArgName input argument description. 
 * @param[out]  outArgName output argument description.  
 * @retval  
 * @retval  
 * @par 
 *      
 * @par 
 *      
 * @par History
 *      2016-5-29 Huang Shengda
 */  
static u8 SenseDeviceInit()
{
    ADC_Config();
    SPI_Config();
    SPICSInit();
    
    /* Run the ADC calibration */  
    if (HAL_ADCEx_Calibration_Start(&AdcHandle) != HAL_OK) {
        /* Calibration Error */
        AaSysLogPrint(LOGLEVEL_ERR, FeatureSense, "Sense ADC calibration failed");
        return 1;
    }

    AaSysLogPrint(LOGLEVEL_INF, FeatureSense, "Sense device initialize success");

    return 0;
}


/**
  * @brief  ADC configuration
  * @param  None
  * @retval None
  */
static void ADC_Config(void)
{
  ADC_ChannelConfTypeDef   sConfig;
  ADC_AnalogWDGConfTypeDef AnalogWDGConfig;
  
  /* Configuration of ADCx init structure: ADC parameters and regular group */
  AdcHandle.Instance = ADCx;
  
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  AdcHandle.Init.ScanConvMode          = ADC_SCAN_DISABLE;              /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
  AdcHandle.Init.ContinuousConvMode    = ENABLE;                        /* Continuous mode to have maximum conversion speed (no delay between conversions) */
  AdcHandle.Init.NbrOfConversion       = 1;                             /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.NbrOfDiscConversion   = 1;                             /* Parameter discarded because sequencer is disabled */
  AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;            /* Software start to trig the 1st conversion manually, without external event */

  if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
  {
    /* ADC initialization error */
    AaSysLogPrint(LOGLEVEL_ERR, FeatureSense, "%s %d: Sense ADC HAL initialize failed",
            __FUNCTION__, __LINE__);
  }
  
  /* Configuration of channel on ADCx regular group on sequencer rank 1 */
  /* Note: Considering IT occurring after each ADC conversion if ADC          */
  /*       conversion is out of the analog watchdog window selected (ADC IT   */
  /*       enabled), select sampling time and ADC clock with sufficient       */
  /*       duration to not create an overhead situation in IRQHandler.        */
  sConfig.Channel      = ADCx_CHANNELa;
  sConfig.Rank         = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_41CYCLES_5;
  
  if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
  {
    /* Channel Configuration Error */
    AaSysLogPrint(LOGLEVEL_ERR, FeatureSense, "%s %d: Sense ADC HAL channel config failed",
            __FUNCTION__, __LINE__);
  }
  
  /* Set analog watchdog thresholds in order to be between steps of DAC       */
  /* voltage.                                                                 */
  /*  - High threshold: between DAC steps 1/2 and 3/4 of full range:          */
  /*                    5/8 of full range (4095 <=> Vdda=3.3V): 2559<=> 2.06V */
  /*  - Low threshold:  between DAC steps 0 and 1/4 of full range:            */
  /*                    1/8 of full range (4095 <=> Vdda=3.3V): 512 <=> 0.41V */

  /* Analog watchdog 1 configuration */
  AnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_ALL_REG;
  AnalogWDGConfig.Channel = ADCx_CHANNELa;
  AnalogWDGConfig.ITMode = ENABLE;
  AnalogWDGConfig.HighThreshold = (RANGE_12BITS * 5/8);
  AnalogWDGConfig.LowThreshold = (RANGE_12BITS * 1/8);
  if (HAL_ADC_AnalogWDGConfig(&AdcHandle, &AnalogWDGConfig) != HAL_OK)
  {
    /* Channel Configuration Error */
    AaSysLogPrint(LOGLEVEL_ERR, FeatureSense, "%s %d: Sense ADC HAL analog wdg config failed",
            __FUNCTION__, __LINE__);
  }
  
}

/**
  * @brief ADC MSP initialization
  *        This function configures the hardware resources used in this example:
  *          - Enable clock of ADC peripheral
  *          - Configure the GPIO associated to the peripheral channels
  *          - Configure the DMA associated to the peripheral
  *          - Configure the NVIC associated to the peripheral interruptions
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
  GPIO_InitTypeDef          GPIO_InitStruct;
  static DMA_HandleTypeDef  DmaHandle;
  RCC_PeriphCLKInitTypeDef  PeriphClkInit;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable clock of GPIO associated to the peripheral channels */
  ADCx_CHANNELa_GPIO_CLK_ENABLE();
  
  /* Enable clock of ADCx peripheral */
  ADCx_CLK_ENABLE();

  /* Configure ADCx clock prescaler */
  /* Caution: On STM32F1, ADC clock frequency max is 14MHz (refer to device   */
  /*          datasheet).                                                     */
  /*          Therefore, ADC clock prescaler must be configured in function   */
  /*          of ADC clock source frequency to remain below this maximum      */
  /*          frequency.                                                      */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  /* Enable clock of DMA associated to the peripheral */
  ADCx_DMA_CLK_ENABLE();
  
  /*##-2- Configure peripheral GPIO ##########################################*/
  /* Configure GPIO pin of the selected ADC channel */
  GPIO_InitStruct.Pin = ADCx_CHANNELa_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADCx_CHANNELa_GPIO_PORT, &GPIO_InitStruct);
  
  /*##-3- Configure the DMA ##################################################*/
  /* Configure DMA parameters */
  DmaHandle.Instance = ADCx_DMA;

  DmaHandle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  DmaHandle.Init.PeriphInc           = DMA_PINC_DISABLE;
  DmaHandle.Init.MemInc              = DMA_MINC_ENABLE;
  DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;   /* Transfer from ADC by half-word to match with ADC configuration: ADC resolution 10 or 12 bits */
  DmaHandle.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;   /* Transfer to memory by half-word to match with buffer variable type: half-word */
  DmaHandle.Init.Mode                = DMA_CIRCULAR;              /* DMA in circular mode to match with ADC configuration: DMA continuous requests */
  DmaHandle.Init.Priority            = DMA_PRIORITY_HIGH;
  
  /* Deinitialize  & Initialize the DMA for new transfer */
  HAL_DMA_DeInit(&DmaHandle);
  HAL_DMA_Init(&DmaHandle);

  /* Associate the initialized DMA handle to the ADC handle */
  __HAL_LINKDMA(hadc, DMA_Handle, DmaHandle);
  
  /*##-4- Configure the NVIC #################################################*/

  /* NVIC configuration for DMA interrupt (transfer completion or error) */
  /* Priority: high-priority */
  // NOTES: in FreeRTOS, NVIC interrrupt priority should not larger than 5
  HAL_NVIC_SetPriority(ADCx_DMA_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(ADCx_DMA_IRQn);
  

  /* NVIC configuration for ADC interrupt */
  /* Priority: high-priority */
  // NOTES: in FreeRTOS, NVIC interrrupt priority should not larger than 5
  HAL_NVIC_SetPriority(ADCx_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(ADCx_IRQn);
}

/**
  * @brief ADC MSP de-initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable clock of ADC peripheral
  *          - Revert GPIO associated to the peripheral channels to their default state
  *          - Revert DMA associated to the peripheral to its default state
  *          - Revert NVIC associated to the peripheral interruptions to its default state
  * @param hadc: ADC handle pointer
  * @retval None
  */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
  /*##-1- Reset peripherals ##################################################*/
  ADCx_FORCE_RESET();
  ADCx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks ################################*/
  /* De-initialize GPIO pin of the selected ADC channel */
  HAL_GPIO_DeInit(ADCx_CHANNELa_GPIO_PORT, ADCx_CHANNELa_PIN);

  /*##-3- Disable the DMA ####################################################*/
  /* De-Initialize the DMA associated to the peripheral */
  if(hadc->DMA_Handle != NULL)
  {
    HAL_DMA_DeInit(hadc->DMA_Handle);
  }

  /*##-4- Disable the NVIC ###################################################*/
  /* Disable the NVIC configuration for DMA interrupt */
  HAL_NVIC_DisableIRQ(ADCx_DMA_IRQn);
  
  /* Disable the NVIC configuration for ADC interrupt */
  HAL_NVIC_DisableIRQ(ADCx_IRQn);
}


/**
  * @brief  Conversion complete callback in non blocking mode
  * @param  AdcHandle : AdcHandle handle
  * @note   This example shows a simple way to report end of conversion
  *         and get conversion result. You can add your own implementation.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *AdcHandle)
{
    osSemaphoreRelease(_adc_cplt_sem_id);
}

/**
  * @brief  Conversion DMA half-transfer callback in non blocking mode 
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{

}

/**
  * @brief  Analog watchdog callback in non blocking mode. 
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc)
{

}

/**
  * @brief  ADC error callback in non blocking mode
  *        (ADC conversion with interruption or transfer by DMA)
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{

}


/**
  * @brief  initialize LED GPIO.
  * @param  none
  * @retval None
  */
static void SPICSInit()
{
  GPIO_InitTypeDef  gpioinitstruct = {0};
  
  /* Enable the GPIO_LED Clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* Configure the GPIO_LED pin */
  gpioinitstruct.Pin    = GPIO_PIN_4;
  gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
  gpioinitstruct.Pull   = GPIO_PULLUP;
  gpioinitstruct.Speed  = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &gpioinitstruct);

  /* Reset PIN to switch off the LED */
  CSHigh();

  AaSysLogPrint(LOGLEVEL_INF, SystemStartup, "spi device cs pin initialize success");
}

static void SPI_Config()
{
  /*##-1- Configure the SPI peripheral #######################################*/
  /* Set the SPI parameters */
  SpiHandle.Instance               = SPIx;
  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiHandle.Init.CLKPhase          = SPI_PHASE_2EDGE;
  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
  SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
  SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  SpiHandle.Init.CRCPolynomial     = 7;
  SpiHandle.Init.NSS               = SPI_NSS_SOFT;

  SpiHandle.Init.Mode = SPI_MODE_MASTER;

  if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
  {
    /* Initialization Error */
    AaSysLogPrint(LOGLEVEL_ERR, FeatureSense, "%s %d: SPI HAL channel config failed",
            __FUNCTION__, __LINE__);
  }
  
  /* SPI block is enabled prior calling SPI transmit/receive functions, in order to get CLK signal properly pulled down.
     Otherwise, SPI CLK signal is not clean on this board and leads to errors during transfer */
  __HAL_SPI_ENABLE(&SpiHandle);
}

/**
  * @brief SPI MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - NVIC configuration for SPI interrupt request enable
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
GPIO_InitTypeDef  GPIO_InitStruct;

  if (hspi->Instance == SPIx)
  {
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MISO_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();
    /* Enable SPI clock */
    SPIx_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* SPI SCK GPIO pin configuration  */
    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MISO GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MISO_PIN;
    HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MOSI GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);

    /*##-3- Configure the NVIC for SPI #########################################*/
    /* NVIC for SPI */
    HAL_NVIC_SetPriority(SPIx_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SPIx_IRQn);
  }
}

/**
  * @brief SPI MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
  if(hspi->Instance == SPIx)
  {
    /*##-1- Disable peripherals and GPIO Clocks ################################*/
    /* Configure SPI SCK as alternate function  */
    HAL_GPIO_DeInit(SPIx_SCK_GPIO_PORT, SPIx_SCK_PIN);
    /* Configure SPI MISO as alternate function  */
    HAL_GPIO_DeInit(SPIx_MISO_GPIO_PORT, SPIx_MISO_PIN);
    /* Configure SPI MOSI as alternate function  */
    HAL_GPIO_DeInit(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_PIN);

    /*##-2- Disable the NVIC for SPI ###########################################*/
    HAL_NVIC_DisableIRQ(SPIx_IRQn);
  }
}

/**
  * @brief  TxRx Transfer completed callback.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report end of Interrupt TxRx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  osSemaphoreRelease(_spi_cplt_sem_id);
}

/**
  * @brief  SPI error callbacks.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{

}


// end of file





