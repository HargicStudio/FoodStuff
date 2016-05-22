
/***

History:
[2015-09-30 Ted]: Create
[2016-04-21 Ted]: transplant code to FreeRTOS platform as stdio usart

*/

#ifndef _AAMEM_H
#define _AAMEM_H

#ifdef __cplusplus
 extern "C" {
#endif 


//#include "cmsis_os.h"
 	

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;


#define AAMEM_STATUS_DEBUG


void AaMemHeapInit(void *begin_addr, void *end_addr);
void *AaMemMalloc(u32 size);
void *AaMemRealloc(void *rmem, u32 newsize);
void *AaMemCalloc(u32 count, u32 size);
void AaMemFree(void *rmem);

#ifdef AAMEM_STATUS_DEBUG
void AaMemInfo(u32 *total, u32 *used, u32 *max_used);
void AaMemList(void);
#endif



#ifdef __cplusplus
}
#endif

#endif // _AAMEM_H

// end of file

