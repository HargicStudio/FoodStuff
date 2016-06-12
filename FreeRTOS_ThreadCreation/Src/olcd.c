

/***

History:
[2016-05-21 Ted]: Create

*/


#include "olcd.h"
#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"
#include "AaInclude.h"



/** Description of the macro */  
#define OLED_CMD  0
#define OLED_DATA 1


/* SPI handler declaration */
SPI_HandleTypeDef SpiHandle;

/** Olcd handler id */  
osThreadId _olcd_id;

/** semaphore for Olcd */  
static osSemaphoreDef(olcd_sendcplt_sem);
osSemaphoreId _olcd_sendcplt_sem_id;


#define OlcdCSSet()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); 
#define OlcdCSClr()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); 

#define OlcdDCSet()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); 
#define OlcdDCClr()     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); 



static void OlcdThread(void const *argument);
static u8 SpiDeviceInit();
static void OlcdWrite(char* data, u16, u8 cmd);
static void OlcdInit();



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
 *      2016-6-1 Huang Shengda
 */  
static void OlcdThread(void const *argument)
{
    (void) argument;
    osEvent evt;
    u32 block_size;
    char* block_addr;

    AaSysLogPrint(LOGLEVEL_INF, FeatureOlcd, "Olcd task started");

    OlcdInit();

    for(;;) {


    }
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
 *      2016-6-1 Huang Shengda
 */  
u8 StartOlcdTask()
{
    SpiDeviceInit();


    _olcd_sendcplt_sem_id = osSemaphoreCreate(osSemaphore(olcd_sendcplt_sem), 1);
    if(_olcd_sendcplt_sem_id == NULL) {
        AaSysLogPrint(LOGLEVEL_ERR, FeatureOlcd, "%s %d: olcd_sendcplt_sem initialize failed",
                __FUNCTION__, __LINE__);
        return 2;
    }
    AaSysLogPrint(LOGLEVEL_INF, FeatureOlcd, "create olcd_sendcplt_sem success");


    osThreadDef(Olcd, OlcdThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    _olcd_id = AaThreadCreateStartup(osThread(Olcd), NULL);
    if(_olcd_id == NULL) {
        AaSysLogPrint(LOGLEVEL_ERR, FeatureOlcd, "%s %d: create Sense task failed",
                __FUNCTION__, __LINE__);
        return 1;
    }


    AaSysLogPrint(LOGLEVEL_INF, FeatureOlcd, "create Olcd task success");

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
 *      2016-6-1 Huang Shengda
 */  
static u8 SpiDeviceInit()
{
    // initialize DataCmd pin and spi CS pin GPIO
    GPIO_InitTypeDef  gpioinitstruct = {0};
  
    /* Enable the GPIO Clock */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configure the GPIO CS pin */
    gpioinitstruct.Pin    = GPIO_PIN_12;
    gpioinitstruct.Mode   = GPIO_MODE_OUTPUT_PP;
    gpioinitstruct.Pull   = GPIO_PULLUP;
    gpioinitstruct.Speed  = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpioinitstruct);

    /* Configure the GPIO D/C pin */
    gpioinitstruct.Pin    = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &gpioinitstruct);

    OlcdCSClr();
    OlcdDCClr();

    /*##-1- Configure the SPI peripheral #######################################*/
    /* Set the SPI parameters */
    SpiHandle.Instance               = SPIx;
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    SpiHandle.Init.CRCPolynomial     = 7;
    SpiHandle.Init.NSS               = SPI_NSS_SOFT;

    SpiHandle.Init.Mode = SPI_MODE_MASTER;

    if(HAL_SPI_Init(&SpiHandle) != HAL_OK) {
        /* Initialization Error */
        AaSysLogPrint(LOGLEVEL_ERR, FeatureOlcd, "Olcd spi device initialize failed");
        return 1;
    }

    /* SPI block is enabled prior calling SPI transmit/receive functions, in order to get CLK signal properly pulled down.
    Otherwise, SPI CLK signal is not clean on this board and leads to errors during transfer */
    __HAL_SPI_ENABLE(&SpiHandle);

    AaSysLogPrint(LOGLEVEL_INF, FeatureOlcd, "Olcd spi device initialize success");

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
 *      2016-6-1 Huang Shengda
 */  
static void OlcdWrite(char* data, u16 size, u8 cmd)
{

    if(cmd) {
        OlcdDCSet();
    }
    else {
        OlcdDCClr();
    }

    OlcdCSClr();
    HAL_SPI_Transmit_IT(&SpiHandle, data, size);

    osSemaphoreWait(_olcd_sendcplt_sem_id, osWaitForever);

    // reinitialize
    OlcdCSSet();
    OlcdDCSet();
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
 *      2016-6-1 Huang Shengda
 */  
static void OlcdInit()
{
    char data[27] = {0xAE, \   //--turn off oled panel
                    0x00, \   //---set low column address
                    0x10, \   //---set high column address
                    0x40, \   //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
                    0x81, \   //--set contrast control register
                    0xCF, \   //--Set SEG Output Current Brightness
                    0xA1, \   //--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
                    0xC8, \   //Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
                    0xA6, \   //--set normal display
                    0xA8, \   //--set multiplex ratio(1 to 64)
                    0x3f, \   //--1/64 duty
                    0xD3, \   //-set display offset Shift Mapping RAM Counter (0x00~0x3F)
                    0x00, \   //-not offset
                    0xd5, \   //--set display clock divide ratio/oscillator frequency
                    0x80, \   //--set divide ratio, Set Clock as 100 Frames/Sec
                    0xD9, \   //--set pre-charge period
                    0xF1, \   //Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
                    0xDA, \   //--set com pins hardware configuration
                    0x12, \   //
                    0xDB, \   //--set vcomh
                    0x40, \   //Set VCOM Deselect Level
                    0x20, \   //-Set Page Addressing Mode (0x00/0x01/0x02)
                    0x02, \   //
                    0x8D, \   //--set Charge Pump enable/disable
                    0x14, \   //--set(0x10) disable
                    0xA4, \   // Disable Entire Display On (0xa4/0xa5)
                    0xA6};    // Disable Inverse Display On (0xa6/a7)  ];

    // delay 100ms
    osDelay(100);

    OlcdWrite(data, 27, OLED_CMD);

    OlcdClear();
    OlcdSetPos(0, 0);    
    OlcdWrite(0xAF, OLED_CMD); /*display ON*/ 
}


static void OlcdClear()
{
    u8 i;
    u8 tmp_cmd[3] = {0X0, 0x00, 0x10};
    u8 tmp[128];

    memset( tmp, 0x00, 128 );

    for(i=0; i<8; i++) {  
        tmp_cmd[0] = 0xb0 + i;

        OlcdWrite( tmp_cmd, 3, OLED_CMD);
        OlcdWrite( tmp, 128, OLED_DATA);
    }
}


static void OlcdSetPos(u8 x, u8 y) 
{ 
    u8 tmp[3] = {0xb0+y, ((x & 0xf0)>>4)|0x10, (x & 0x0f)|0x01};

    OlcdWrite( tmp, 3, OLED_CMD);
}   


static void OlcdDisplayOn()
{
    u8 tmp[3] = {0X8D, 0X14, 0XAF};

    OlcdWrite( tmp, 3, OLED_CMD);
}


static void OlcdDisplayOff()
{
    u8 tmp[3] = {0X8D, 0X10, 0XAE};

    OlcdWrite( tmp, 3, OLED_CMD);
}   


static void OlcdShowChar(u8 x, u8 y, u8 chr)
{       
    char c = chr - ' ';

    if(x >= OLCD_MAX_COLUMN) {
        x = 0;
        y = y + 2;
    }

    OlcdSetPos(x, y + 1);
    OlcdWrite( (char *)F6x8[c], 6, OLED_DATA );
}


static void OlcdShowString(u8 x, u8 y, char *chr)
{
    u8 j = 0;
    u8 x_t = x, y_t = y;

    while (chr[j]!='\0') { 
        // add for CR/LF
        if( ('\r' == chr[j]) && ('\n' == chr[j+1]) ){  // CR LF
            while(x_t <= 120){  // fill rest chars in current line
                OLED_ShowChar(x_t,y_t,' ');
                x_t += 8;
            }

            j += 2;
        }
        else if( ('\r' == chr[j]) || ('\n' == chr[j]) ){   // CR or LF
            while(x_t <= 120){  // fill rest chars in current line
                OLED_ShowChar(x_t,y_t,' ');
                x_t += 8;
            }

            j += 1;
        }
        else{
            if(x_t>120){  // line end, goto next line
                x_t = 0;
                y_t += 2;

                if(y_t >= 8) {  // can only display 4 line
                    break;
                }
            }
            OLED_ShowChar(x_t,y_t,chr[j]);
            x_t += 8;
            j++;
        }
    }
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
    // NOTES: in FreeRTOS, NVIC interrrupt priority should not larger than 5
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
 *      2016-6-1 Huang Shengda
 */  
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    osSemaphoreRelease(_olcd_sendcplt_sem_id);
}

// end of file





