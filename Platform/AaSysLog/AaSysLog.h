
/***

History:
[2015-09-30 Ted]: Create
[2016-04-21 Ted]: transplant code to FreeRTOS platform as stdio usart
[2016-05-21 Ted]: support BipBuffer and DMA for uart tx debug

*/

#ifndef _AASYSLOG_H
#define _AASYSLOG_H

#ifdef __cplusplus
 extern "C" {
#endif 


#include "cmsis_os.h"
#include "glo_def.h"
#include "AaInclude.h"
#include "stm32f1xx_hal.h"



/** 
 * debug log level
 * The detail description. 
 */  
typedef enum ELogLevel_t {
    LOGLEVEL_ALL = 0,
    LOGLEVEL_DBG = 1,
    LOGLEVEL_INF = 2,
    LOGLEVEL_WRN = 3,
    LOGLEVEL_ERR = 4,
} ELogLevel;



/** 
 * The brief description. 
 * The detail description. 
 */  
typedef struct SAaSysLog_t {
    /*!
     *  @brief  
     */
    void (*processGetBip_callback)(char* addr, u32 len);

    /*!
     *  @brief  
     */
    void (*processPrint_callback)(ELogLevel level, char* feature_id, const char* fmt, ...);
} SAaSysLog;



extern SAaSysLog _aasyslog_mng;

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
 *      2016-5-21 Huang Shengda
 */  
#define AaSysLogPrint(level, feature_id, fmt, ...)     do { \
                                                         if(_aasyslog_mng.processPrint_callback == NULL) {  \
                                                            break;  \
                                                         }  \
                                                        _aasyslog_mng.processPrint_callback(level, feature_id, fmt, ##__VA_ARGS__);  \
                                                       } while(0)




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
 *      2016-5-21 Huang Shengda
 */  
u8 AaSysLogCEInit();

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
 *      2016-5-21 Huang Shengda
 */  
u8 AaSysLogCreateDeamon();

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
 *      2016-5-21 Huang Shengda
 */  
u8 AaSysLogGetBipRegister(void(*function)(char*, u32));

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
 *      2016-5-21 Huang Shengda
 */  
u8 AaSysLogSendCplt();

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
 *      2016-5-21 Huang Shengda
 */  
u8 AaSysLogProcessPrintDefault();


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
 *      2016-5-21 Huang Shengda
 */  
u8 AaSysLogProcessPrintStartup();

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
 *      2016-5-21 Huang Shengda
 */  
u8 AaSysLogProcessPrintRunning();



#ifdef __cplusplus
}
#endif

#endif // _AASYSLOG_H

// end of file
