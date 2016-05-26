
/***

History:
[2015-09-30 Ted]: Create
[2016-04-21 Ted]: merge code to FreeRTOS platform as stdio usart

*/


#include "AaThread.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include "AaInclude.h"


#define AATHREAD_MAX_NAME_LENGTH    configMAX_TASK_NAME_LEN


typedef struct SAaThread_t {
    /*!
     *  @brief  
     */
    struct SAaThread_t* next;
    struct SAaThread_t* prev;

    /*!
     *  @brief  
     */
    char                name[AATHREAD_MAX_NAME_LENGTH];        ///< Thread name 
    os_pthread          pthread;      ///< start address of thread function
    osPriority          tpriority;    ///< initial thread priority
    u32                 instances;    ///< maximum number of instances of that thread function
    u32                 stacksize;    ///< stack size requirements in bytes; 0 is default stack size

    /*!
     *  @brief  
     */
    osThreadId          t_id;
    
} SAaThread;


/** mutex for AaThread */  
SAaThread* _aathread_mng_head_ptr = NULL;


/** mutex for AaThread */  
static osMutexDef(aathread_mutex);
osMutexId _aathread_mutex_id;



static SAaThread* AaThreadFindMngPtr(osThreadId t_id);



/** 
 * This is a brief description. 
 * This function should first been called after AaMemInit
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
u8 AaThreadCEInit()
{
    _aathread_mutex_id = osMutexCreate(osMutex(aathread_mutex));
    if(_aathread_mutex_id == NULL) {
        AaSysLogPrint(LOGLEVEL_ERR, FeatureThread, "%s %d: AaThread mutex initialize failed",
                __FUNCTION__, __LINE__);
        return 2;
    }
    AaSysLogPrint(LOGLEVEL_DBG, FeatureThread, "create aathread_mutex success");

    return 0;
}

/** 
 * This is a brief description. 
 * This function should first been called after AaMemInit
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
osThreadId AaThreadCreateStartScheduler(const osThreadDef_t *t_def, void *arg)
{
    osMutexWait(_aathread_mutex_id, osWaitForever);

    SAaThread* new_ptr;
    SAaThread* cur_ptr;

    osThreadId t_id = osThreadCreate(t_def, arg);
    if(t_id == NULL) {
        osMutexRelease(_aathread_mutex_id);
        return NULL;
    }
    
    cur_ptr = _aathread_mng_head_ptr;
    
    if(cur_ptr == NULL) {    // this is the first thread
        new_ptr = AaMemMalloc(sizeof(SAaThread));
        if(new_ptr == NULL) {
            osMutexRelease(_aathread_mutex_id);
            return NULL;
        }
        strcpy(new_ptr->name, t_def->name);
        new_ptr->pthread = t_def->pthread;
        new_ptr->tpriority = t_def->tpriority;
        new_ptr->instances = t_def->instances;
        new_ptr->stacksize = t_def->stacksize;
        new_ptr->t_id = t_id;
        new_ptr->next = NULL;
        new_ptr->prev = NULL;

        _aathread_mng_head_ptr = new_ptr;
    }
    else {
        
        while(cur_ptr->next != NULL) {
            cur_ptr = cur_ptr->next;
        }

        // get the last mng_ptr
        new_ptr = AaMemMalloc(sizeof(SAaThread));
        if(new_ptr == NULL) {
            osMutexRelease(_aathread_mutex_id);
            return NULL;
        }
        strcpy(new_ptr->name, t_def->name);
        new_ptr->pthread = t_def->pthread;
        new_ptr->tpriority = t_def->tpriority;
        new_ptr->instances = t_def->instances;
        new_ptr->stacksize = t_def->stacksize;
        new_ptr->t_id = t_id;
        new_ptr->next = NULL;
        new_ptr->prev = cur_ptr;

        cur_ptr->next = new_ptr;
    }

    osMutexRelease(_aathread_mutex_id);

    return t_id;
}

/** 
 * This is a brief description. 
 * This function should first been called after AaMemInit
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
osThreadId AaThreadCreateStartup(const osThreadDef_t *t_def, void *arg)
{
    SAaThread* new_ptr;
    SAaThread* cur_ptr;

    osThreadId t_id = osThreadCreate(t_def, arg);
    if(t_id == NULL) {
        return NULL;
    }
    
    cur_ptr = _aathread_mng_head_ptr;
    
    if(cur_ptr == NULL) {    // this is the first thread
        new_ptr = AaMemMalloc(sizeof(SAaThread));
        if(new_ptr == NULL) {
            return NULL;
        }
        strcpy(new_ptr->name, t_def->name);
        new_ptr->pthread = t_def->pthread;
        new_ptr->tpriority = t_def->tpriority;
        new_ptr->instances = t_def->instances;
        new_ptr->stacksize = t_def->stacksize;
        new_ptr->t_id = t_id;
        new_ptr->next = NULL;
        new_ptr->prev = NULL;

        _aathread_mng_head_ptr = new_ptr;
    }
    else {
        
        while(cur_ptr->next != NULL) {
            cur_ptr = cur_ptr->next;
        }

        // get the last mng_ptr
        new_ptr = AaMemMalloc(sizeof(SAaThread));
        if(new_ptr == NULL) {
            return NULL;
        }
        strcpy(new_ptr->name, t_def->name);
        new_ptr->pthread = t_def->pthread;
        new_ptr->tpriority = t_def->tpriority;
        new_ptr->instances = t_def->instances;
        new_ptr->stacksize = t_def->stacksize;
        new_ptr->t_id = t_id;
        new_ptr->next = NULL;
        new_ptr->prev = cur_ptr;

        cur_ptr->next = new_ptr;
    }

    return t_id;
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
osStatus AaThreadKill(osThreadId t_id)
{
    osMutexWait(_aathread_mutex_id, osWaitForever);
    
    osStatus err = osThreadTerminate(t_id);
    if(err != osOK ) {
        osMutexRelease(_aathread_mutex_id);
        return err;
    }

    SAaThread* cur_ptr = AaThreadFindMngPtr(t_id);
    SAaThread* pre_ptr = cur_ptr->prev;
    SAaThread* nxt_ptr = cur_ptr->next;

    pre_ptr->next = nxt_ptr;
    nxt_ptr->prev = pre_ptr;
    
    AaMemFree(cur_ptr);

    osMutexRelease(_aathread_mutex_id);

    return err;
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
static SAaThread* AaThreadFindMngPtr(osThreadId t_id)
{
    SAaThread* cur_ptr = _aathread_mng_head_ptr;
    
    if(cur_ptr == NULL) {    // thread manage is empty
        return NULL;
    }
    else {
        do {
            if(cur_ptr->t_id == t_id) {
                break;
            }
            else {
                cur_ptr = cur_ptr->next;
            }
        } while(cur_ptr != NULL);

        if(cur_ptr == NULL) {   // no this t_id
            return NULL;
        }
        
        return cur_ptr;
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
 *      2016-5-22 Huang Shengda
 */  
char* AaThreadGetName(osThreadId t_id)
{
    SAaThread* cur_ptr = AaThreadFindMngPtr(t_id);
    if(cur_ptr == NULL) {
        return NULL;
    }
    
    return cur_ptr->name;
}



// end of file
