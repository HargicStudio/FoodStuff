
#include "AaMem.h"
#include <stdlib.h>
#include <stdio.h>



#define HEAP_LENGTH		1024


static int PrintBufferInfo(u8* _this, int len);



int main()
{
	int i;

	printf("sizeof(u16): %d\n", sizeof(u16));
	printf("sizeof(u8): %d\n", sizeof(u8));

	u8* heap = malloc(HEAP_LENGTH);

	AaMemHeapInit(heap, &heap[HEAP_LENGTH - 1]);


	printf("INF: malloc 100 space for buffer1\n");
	u8* buffer1 = AaMemMalloc(100);
	if(buffer1 == NULL) {
		printf("ERR: buffer1 is NULL");
	}
	for(i=0; i<100; i++) {
		buffer1[i] = i + 1;
	}
	PrintBufferInfo(buffer1, 100);
	PrintBufferInfo(heap, HEAP_LENGTH);
	AaMemList();


	printf("INF: malloc 100 space for buffer2\n");
	u8* buffer2 = AaMemMalloc(100);
	if(buffer2 == NULL) {
		printf("ERR: buffer2 is NULL");
	}
	for(i=0; i<100; i++) {
		buffer2[i] = i + 1;
	}
	PrintBufferInfo(buffer2, 100);
	PrintBufferInfo(heap, HEAP_LENGTH);
	AaMemList();


	printf("INF: free buffer1\n");
	AaMemFree(buffer1);
	PrintBufferInfo(buffer1, 100);
	PrintBufferInfo(heap, HEAP_LENGTH);
	AaMemList();
}

static int PrintBufferInfo(u8* _this, int len)
{  
    u16* _buffer = (u16*)_this;
    int idx;
    
    for(idx = 0; idx < (len/(sizeof(u16)/sizeof(u8))); idx++) {
        if(idx % 16 == 0) {
            if(idx != 0) {
                printf("\n");
            }
            printf("Addr 0x%08x: ", idx);
        }
        printf("%08x ", _buffer[idx]);
    }
    printf("\n");
}