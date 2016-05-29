
/***

History:
[2016-05-22 Ted]: Create

*/


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include "AaInclude.h"
#include "print_com.h"
#include "led.h"
#include "sense.h"


#define CCSDEAMON_STACK_SIZE        0x80


/** AaMem heap buffer for whole system */  
#define AAMEM_HEAP_BUFFER_SIZE  (1024*4)

char _mem_heap_buf[AAMEM_HEAP_BUFFER_SIZE];


/** Description of the macro */  
osThreadId _ccsdeamon_id;



static u8 CCSDeamonCreateThread();
static void CCSDeamonThread(void const *arg);



/** 
 * System Compute environment initialize
 * initialize the sub service which won't create thread
 * any sub service deamon which need create thread should be initialzed in CCSDeamon thread
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
u8 CCSDeamonCEInit()
{
    // WARNING: don't change the order

    // @first because all service should depand on log
    StdUsartInit();

    // bip buffer has not construct because of memery heap do not setup,
    // so aasyslog should use stdio print interface
    AaSysLogProcessPrintDefault();

    // platform initialize
    AaThreadCEInit();
    AaMemHeapCEInit(_mem_heap_buf, &_mem_heap_buf[AAMEM_HEAP_BUFFER_SIZE - 1]);
    AaSysLogCEInit();

    // after heap momery and bip buffer initialized, log can be input bip buffer
    AaSysLogProcessPrintStartup();

    // alternative CCS service initialization
    AaTagCEInit();

    CCSDeamonCreateThread();
    AaSysLogCreateDeamon();
    AaTagCreateDeamon();
    // start application task
    StartRunLedTask();
    StartSenseTask();

    // create global tag
    AaTagCreate(AATAG_CCS_DEAMON_ONLINE, 0);
    AaTagCreate(AATAG_CCS_STARTUP, 0);

    AaSysLogPrint(LOGLEVEL_DBG, SystemStartup, "System started");

    // as scheduler started, print can be print into bipbuffer and send out by DMA
    AaSysLogGetBipRegister(GetBipAndSendByDMA);
    AaSysLogProcessPrintRunning();
    
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
 *      2016-5-22 Huang Shengda
 */  
static u8 CCSDeamonCreateThread()
{
    osThreadDef(CCSDeamon, CCSDeamonThread, osPriorityHigh, 0, CCSDEAMON_STACK_SIZE);
    
    _ccsdeamon_id = AaThreadCreateStartup(osThread(CCSDeamon), NULL);
    if(_ccsdeamon_id == NULL) {
        AaSysLogPrint(LOGLEVEL_ERR, FeatureCCS, "%s %d: CCS Deamon initialize failed",
                __FUNCTION__, __LINE__);
        return 1;
    }
    AaSysLogPrint(LOGLEVEL_DBG, FeatureCCS, "create CCS deamon success");
    
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
 *      2016-5-22 Huang Shengda
 */  
static void CCSDeamonThread(void const *arg)
{
    (void) arg;

    AaSysLogPrint(LOGLEVEL_INF, FeatureCCS, "CCSDeamonThread started");
    AaTagSetValue(AATAG_CCS_DEAMON_ONLINE, 1);

    // initialize ccs service which need in CCSDeamon thread
    

    // start ccs service thread



    AaTagSetValue(AATAG_CCS_STARTUP, 1);

    u8 i = 0;

    for(;;) {
        osDelay(5000);
        AaMemList();
        AaTagSetValue(AATAG_CCS_DEAMON_ONLINE, i++);
    }
}



// end of file
