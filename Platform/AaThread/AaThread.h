
/***

History:
[2015-09-30 Ted]: Create
[2016-04-21 Ted]: transplant code to FreeRTOS platform as stdio usart

*/

#ifndef _AATHREAD_H
#define _AATHREAD_H

#ifdef __cplusplus
 extern "C" {
#endif 


#include "cmsis_os.h"
#include "glo_def.h"



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
 *      2016-5-22 Huang Shengda
 */  
u8 AaThreadCEInit();

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
 *      2016-5-22 Huang Shengda
 */  
osThreadId AaThreadCreateStartScheduler(const osThreadDef_t *t_def, void *arg);

osThreadId AaThreadCreateStartup(const osThreadDef_t *t_def, void *arg);


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
 *      2016-5-22 Huang Shengda
 */  
osStatus AaThreadKill(osThreadId t_id);

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
 *      2016-5-22 Huang Shengda
 */  
char* AaThreadGetName(osThreadId t_id);



#ifdef __cplusplus
}
#endif

#endif // _AATHREAD_H

// end of file
