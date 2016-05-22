/***

History:
[2015-09-30 Ted]: Create
[2016-04-21 Ted]: merge code to FreeRTOS platform as stdio usart
[2016-05-21 Ted]: support BipBuffer and DMA for uart tx debug

*/


#include "AaSysLog.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include "BipBuffer.h"


/** uart tx buffer size */  
#define AASYSLOG_BIPBUFFER_SIZE     (1024*2)


#define AASYSLOGDEAMON_STACK_SIZE   ( (u16) 0x80 )


/** syslog index, will increase for every package log */
static u8 _aasyslog_index = 0;

/** log level for log filter */
static ELogLevel _print_level;

/** bip buffer manage for uart tx */  
static CBipBuffer* _p_bip_buffer = NULL;


/** AaSysLog manage */ 
SAaSysLog _aasyslog_mng = { .processGetBip_callback = NULL, .processPrint_callback = NULL };


/** mutex for AaSysLogPrint */  
static osMutexDef(aasyslog_mutex);
osMutexId _aasyslog_mutex_id;


/** mutex for AaSysLogPrint */ 
static osMutexDef(bipbuf_mutex);
osMutexId _bipbuf_mutex_id;


/** Deamon thread for AaSysLog */  
osThreadId _aasyslogdeamon_id;



static void AaSysLogDeamonThread(void const *arg);
static u8 AaSysLogProcessPrintRegister(void (*function)(ELogLevel , char* , const char* , ...));
static void AaSysLogPrintByPolling(ELogLevel level, char* feature_id, const char* fmt, ...);
static void AaSysLogPrintByPollingWithMutex(ELogLevel level, char* feature_id, const char* fmt, ...);
static void AaSysLogPrintByDMA(ELogLevel level, char* feature_id, const char* fmt, ...);
static char* CBipoBuffer_Reserve(CBipBuffer* _this, u32 size);
static void CBipoBuffer_Commit(CBipBuffer* _this, u32 size);
static void* CBipoBuffer_Get(CBipBuffer* _this, u32* cBlockSizePtr);
static void CBipoBuffer_Decommit(CBipBuffer* _this, u32 size);
static void CBipoBuffer_Print(CBipBuffer* _this);



/** Signal flag for AaSysLogDeamon */  
#define SIG_BIT_TX          (1<<0)
#define SIG_BIT_TX_CPLT     (1<<1)


/** 
 * This is a brief description. 
 * This AaSysLogInit should depand on AaMemInit
 * @param[in]   inArgName input argument description. 
 * @param[out]  always 0 
 * @retval  
 * @retval  
 * @par 
 *      
 * @par 
 *      
 * @par History
 *      2016-5-21 Huang Shengda
 */  
static void AaSysLogDeamonThread(void const *arg)
{
    (void) arg;
    osEvent evt;
    u32 block_size;
    char* block_addr;

    AaSysLogPrint(LOGLEVEL_INF, FeatureLog, "AaSysLogDeamonThread started");

    AaSysLogProcessPrintRunThrPolling();
    
    for(;;) {
        evt = osSignalWait(SIG_BIT_TX, osWaitForever);
        if(evt.status == osEventSignal && evt.value.signals == SIG_BIT_TX) {
            
            block_addr = CBipoBuffer_Get(_p_bip_buffer, &block_size);
            if(block_addr == NULL) {
                continue;
            }

            if(_aasyslog_mng.processGetBip_callback == NULL) {
                continue;
            }
            _aasyslog_mng.processGetBip_callback(block_addr, block_size);

            evt = osSignalWait(SIG_BIT_TX_CPLT, osWaitForever);
            if(evt.status == osEventSignal && evt.value.signals == SIG_BIT_TX_CPLT) {
                CBipoBuffer_Decommit(_p_bip_buffer, block_size);
            }
        }
    }
}


/** 
 * This is a brief description. 
 * This AaSysLogInit should depand on AaMemInit
 * @param[in]   inArgName input argument description. 
 * @param[out]  always 0 
 * @retval  
 * @retval  
 * @par 
 *      
 * @par 
 *      
 * @par History
 *      2016-5-21 Huang Shengda
 */ 
u8 AaSysLogGetBipRegister(void(*function)(char*, u32))
{
    _aasyslog_mng.processGetBip_callback = function;
    return 0;
}


/** 
 * This is a brief description. 
 * This AaSysLogInit should depand on AaMemInit
 * @param[in]   inArgName input argument description. 
 * @param[out]  always 0 
 * @retval  
 * @retval  
 * @par 
 *      
 * @par 
 *      
 * @par History
 *      2016-5-21 Huang Shengda
 */ 
u8 AaSysLogSendCplt()
{
    osSignalSet(_aasyslogdeamon_id, SIG_BIT_TX_CPLT);
    
    return 0;
}

/** 
 * This is a brief description. 
 * This AaSysLogInit should depand on AaMemInit
 * @param[in]   inArgName input argument description. 
 * @param[out]  always 0 
 * @retval  
 * @retval  
 * @par 
 *      
 * @par 
 *      
 * @par History
 *      2016-5-21 Huang Shengda
 */  
u8 AaSysLogCEInit()
{
    AaSysLogProcessPrintStartup();
    
/*
    _p_bip_buffer = AaMemMalloc(AASYSLOG_BIPBUFFER_SIZE);
    if(_p_bip_buffer == NULL) {
        AaSysLogPrint(LOGLEVEL_ERR, FeatureLog, "%s %d: AaSysLog Bip buffer init failed",
                __FUNCTION__, __LINE__);

        return 1;
    }
    
    AaSysLogPrint(LOGLEVEL_DBG, FeatureLog, "get _p_bip_buffer pointer %p", _p_bip_buffer);
    CBipBuffer_Construct(_p_bip_buffer, AASYSLOG_BIPBUFFER_SIZE);
*/

    _aasyslog_mutex_id = osMutexCreate(osMutex(aasyslog_mutex));
    if(_aasyslog_mutex_id == NULL) {
        AaSysLogPrint(LOGLEVEL_ERR, FeatureLog, "%s %d: AaSysLog mutex initialize failed",
                __FUNCTION__, __LINE__);
        return 2;
    }
    AaSysLogPrint(LOGLEVEL_DBG, FeatureLog, "create aasyslog_mutex success");

/*
    _bipbuf_mutex_id = osMutexCreate(osMutex(bipbuf_mutex));
    if(_bipbuf_mutex_id == NULL) {
        AaSysLogPrint(LOGLEVEL_ERR, FeatureLog, "%s %d: BipBuffer mutex initialize failed",
                __FUNCTION__, __LINE__);
        return 3;
    }
    AaSysLogPrint(LOGLEVEL_DBG, FeatureLog, "create bipbuf_mutex success");
*/

    _print_level = LOGLEVEL_ALL;

    AaSysLogPrint(LOGLEVEL_INF, FeatureLog, "AaSysLog initialize success");

    return 0;
}


/** 
 * This is a brief description. 
 * This AaSysLogInit should depand on AaMemInit
 * @param[in]   inArgName input argument description. 
 * @param[out]  always 0 
 * @retval  
 * @retval  
 * @par 
 *      
 * @par 
 *      
 * @par History
 *      2016-5-21 Huang Shengda
 */  
u8 AaSysLogCreateDeamon()
{
    osThreadDef(AaSysLogDeamon, AaSysLogDeamonThread, osPriorityHigh, 0, AASYSLOGDEAMON_STACK_SIZE);
    
    _aasyslogdeamon_id = AaThreadCreate(osThread(AaSysLogDeamon), NULL);
    if(_aasyslogdeamon_id == NULL) {
        AaSysLogPrint(LOGLEVEL_ERR, FeatureLog, "%s %d: AaSysLog Deamon initialize failed",
                __FUNCTION__, __LINE__);
        return 1;
    }
    AaSysLogPrint(LOGLEVEL_DBG, FeatureLog, "create aasyslog deamon success");

    return 0;
}


/** 
 * This is a brief description. 
 * This 
 * @param[in]   inArgName input argument description. 
 * @param[out]  always 0 
 * @retval  
 * @retval  
 * @par 
 *      
 * @par 
 *      
 * @par History
 *      2016-5-21 Huang Shengda
 */  
u8 AaSysLogProcessPrintStartup()
{
    AaSysLogProcessPrintRegister(AaSysLogPrintByPolling);
    return 0;
}

u8 AaSysLogProcessPrintRunThrPolling()
{
    AaSysLogProcessPrintRegister(AaSysLogPrintByPollingWithMutex);
    return 0;
}

u8 AaSysLogProcessPrintRun()
{
    AaSysLogProcessPrintRegister(AaSysLogPrintByDMA);
    return 0;
}


/** 
 * This is a brief description. 
 * This 
 * @param[in]   inArgName input argument description. 
 * @param[out]  always 0 
 * @retval  
 * @retval  
 * @par 
 *      
 * @par 
 *      
 * @par History
 *      2016-5-21 Huang Shengda
 */  
static u8 AaSysLogProcessPrintRegister(void (*function)(ELogLevel , char* , const char* , ...))
{
    _aasyslog_mng.processPrint_callback = function;
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
 *      2016-5-21 Huang Shengda
 */  
static void AaSysLogPrintByPolling(ELogLevel level, char* feature_id, const char* fmt, ...)
{
    if(level < _print_level) {
        return ;
    }
    
    u8 idx = _aasyslog_index++;
    char* str_level;
    va_list args;

    switch(level) {
        case LOGLEVEL_DBG: str_level = "DBG\0"; break;
        case LOGLEVEL_INF: str_level = "INF\0"; break;
        case LOGLEVEL_WRN: str_level = "WRN\0"; break;
        case LOGLEVEL_ERR: str_level = "ERR\0"; break;
        default: str_level = "Unknow\0"; break;
    }

    printf("%02x %s/%s ", idx, feature_id, str_level);

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\r\n");
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
 *      2016-5-21 Huang Shengda
 */  
static void AaSysLogPrintByPollingWithMutex(ELogLevel level, char* feature_id, const char* fmt, ...)
{
    if(level < _print_level) {
        return ;
    }
    
    u8 idx = _aasyslog_index++;
    char* str_level;
    va_list args;

    switch(level) {
        case LOGLEVEL_DBG: str_level = "DBG\0"; break;
        case LOGLEVEL_INF: str_level = "INF\0"; break;
        case LOGLEVEL_WRN: str_level = "WRN\0"; break;
        case LOGLEVEL_ERR: str_level = "ERR\0"; break;
        default: str_level = "Unknow\0"; break;
    }

    osMutexWait(_aasyslog_mutex_id, osWaitForever);

    printf("%02x %dT %s/%s/%s ", idx, osKernelSysTick(), feature_id, str_level, AaThreadGetName(osThreadGetId()));

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    printf("\r\n");

    osMutexRelease(_aasyslog_mutex_id);
}


/** Description of the macro */  
#define PRINT_STRING_MAX_LENGTH     128

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
static void AaSysLogPrintByDMA(ELogLevel level, char* feature_id, const char* fmt, ...)
{
    osMutexWait(_aasyslog_mutex_id, osWaitForever);
    
    if(level < _print_level) {
        return ;
    }
    
    u8 idx = _aasyslog_index++;
    char* str_level;
    va_list args;

    switch(level) {
        case LOGLEVEL_DBG: str_level = "DBG\0"; break;
        case LOGLEVEL_INF: str_level = "INF\0"; break;
        case LOGLEVEL_WRN: str_level = "WRN\0"; break;
        case LOGLEVEL_ERR: str_level = "ERR\0"; break;
        default: str_level = "Unknow\0"; break;
    }

    char* bip_buf_addr = CBipoBuffer_Reserve(_p_bip_buffer, PRINT_STRING_MAX_LENGTH);
    if(bip_buf_addr == NULL) {
        printf("ERR: %s %d: bip buffer reserve failed\r\n", __FUNCTION__, __LINE__);
        goto back;
    }
    u8 len = sprintf(bip_buf_addr, "%02x %dT %s/%s/%s ", idx, osKernelSysTick(), feature_id, str_level, AaThreadGetName(osThreadGetId()));
    CBipoBuffer_Commit(_p_bip_buffer, len);

    bip_buf_addr = CBipoBuffer_Reserve(_p_bip_buffer, PRINT_STRING_MAX_LENGTH);
    if(bip_buf_addr == NULL) {
        printf("%s %d: bip buffer reserve failed", __FUNCTION__, __LINE__);
        goto back;
    }
    va_start(args, fmt);
    len = vsprintf(bip_buf_addr, fmt, args);
    va_end(args);
    CBipoBuffer_Commit(_p_bip_buffer, len);

    bip_buf_addr = CBipoBuffer_Reserve(_p_bip_buffer, strlen("\r\n"));
    if(bip_buf_addr == NULL) {
        printf("%s %d: bip buffer reserve failed", __FUNCTION__, __LINE__);
        goto back;
    }
    len = sprintf(bip_buf_addr, "\r\n");
    CBipoBuffer_Commit(_p_bip_buffer, len);

back:
    osMutexRelease(_aasyslog_mutex_id);
    osSignalSet(_aasyslogdeamon_id, SIG_BIT_TX);
   
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
 *      2016-5-21 Huang Shengda
 */  
static char* CBipoBuffer_Reserve(CBipBuffer* _this, u32 size)
{
    char* addr;
    
    osMutexWait(_bipbuf_mutex_id, osWaitForever);
    addr = CBipBuffer_Reserve(_this, size);
    CBipoBuffer_Print(_this);
    osMutexRelease(_bipbuf_mutex_id);
    
    return addr;
}

static void CBipoBuffer_Commit(CBipBuffer* _this, u32 size)
{
    osMutexWait(_bipbuf_mutex_id, osWaitForever);
    CBipBuffer_Commit(_this, size);
    CBipoBuffer_Print(_this);
    osMutexRelease(_bipbuf_mutex_id);
}

static void* CBipoBuffer_Get(CBipBuffer* _this, u32* cBlockSizePtr)
{
    void* addr;
    
    osMutexWait(_bipbuf_mutex_id, osWaitForever);
    addr = CBipBuffer_Get(_this, cBlockSizePtr);
    CBipoBuffer_Print(_this);
    osMutexRelease(_bipbuf_mutex_id);

    return addr;
}

static void CBipoBuffer_Decommit(CBipBuffer* _this, u32 size)
{
    osMutexWait(_bipbuf_mutex_id, osWaitForever);
    CBipBuffer_Decommit(_this, size);
    CBipoBuffer_Print(_this);
    osMutexRelease(_bipbuf_mutex_id);
}

static void CBipoBuffer_Print(CBipBuffer* _this)
{
    printf("partitionAIndex %d\r\n", _this->partitionAIndex);
    printf("partitionASize %d\r\n", _this->partitionASize);
    printf("partitionBIndex %d\r\n", _this->partitionBIndex);
    printf("partitionBSize %d\r\n", _this->partitionBSize);
    printf("reservedIndex %d\r\n", _this->reservedIndex);
    printf("reservedSize %d\r\n", _this->reservedSize);
    printf("bufferSize %d\r\n", _this->bufferSize);
}



// end of file
