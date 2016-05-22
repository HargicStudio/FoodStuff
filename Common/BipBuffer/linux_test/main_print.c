

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "BipBuffer.h"

#define BIPBUFFER_SIZE      128

char _ContinueData[BIPBUFFER_SIZE] = {0};
char _ReadBuffer[BIPBUFFER_SIZE];

static int PrintBipBufferInfo(CBipBuffer* _this);
static int PrintBipBufferStructureInfo(CBipBuffer* _this);
static int write_to_bipbuffer(CBipBuffer* _this, char* data, u32 len);
static int read_from_bipbuffer(CBipBuffer* _this, char* data);
static int PrintSerialData(char* data, u32 len);

int main()
{
    printf("\nHello World! sizeof(u32): %d\n", sizeof(u32));
    
    u32 i;
    for(i=0; i<BIPBUFFER_SIZE; i++) {
        _ContinueData[i] = i + 1;
    }
    
    CBipBuffer* _p_bip_buffer = malloc(BIPBUFFER_SIZE);
    
    CBipBuffer_Construct(_p_bip_buffer, BIPBUFFER_SIZE);
    printf("\nINF: after initialized\n");
    PrintBipBufferInfo(_p_bip_buffer);
    
    write_to_bipbuffer(_p_bip_buffer, _ContinueData, 10);
    write_to_bipbuffer(_p_bip_buffer, _ContinueData, 50);
    read_from_bipbuffer(_p_bip_buffer, _ReadBuffer);
    write_to_bipbuffer(_p_bip_buffer, _ContinueData, 20);
    read_from_bipbuffer(_p_bip_buffer, _ReadBuffer);
    
    return 1;
}

static int write_to_bipbuffer(CBipBuffer* _this, char* data, u32 len)
{
    if(len >= (BIPBUFFER_SIZE - sizeof(CBipBuffer))) {
        printf("ERR: data len %d error", len);
    }
    
    // reserve space
    char* bip_buffer_addr = CBipBuffer_Reserve(_this, len);
    if(bip_buffer_addr == NULL) {
        printf("ERR: bip_buffer_addr is NULL\n");
        return 1;
    }
    printf("\nINF: after reserve %d serial data\n", len);
    PrintBipBufferStructureInfo(_this);
    
    // copy the data
    memcpy(bip_buffer_addr, data, len);
    printf("\nINF: after %d serial data copied\n", len);
    PrintBipBufferInfo(_this);
    
    // commit serial data
    CBipBuffer_Commit(_this, len);
    printf("\nINF: after commit %d serial data\n", len);
    PrintBipBufferStructureInfo(_this);
}

static int read_from_bipbuffer(CBipBuffer* _this, char* data)
{
    memset(data, 0, BIPBUFFER_SIZE);
    printf("\nINF: clear data\n");
    PrintSerialData(data, BIPBUFFER_SIZE);
    
    u32 countinous_size = 0;
    char* bip_buffer_addr = CBipBuffer_Get(_this, &countinous_size);
    if(bip_buffer_addr == NULL) {
        printf("ERR: there is no data in bip buffer\n");
    }
    printf("\nINF: after get %d serial data\n", countinous_size);
    PrintBipBufferStructureInfo(_this);
    
    countinous_size -= 20;
    printf("\nINF: after copy %d serial data\n", countinous_size, countinous_size);
    memcpy(data, bip_buffer_addr, countinous_size);
    PrintSerialData(data, countinous_size);
    
    CBipBuffer_Decommit(_this, countinous_size);
    printf("\nINF: after decommit %d serial data\n", countinous_size);
    PrintBipBufferStructureInfo(_this);
}

static int PrintBipBufferInfo(CBipBuffer* _this)
{
    printf("partitionASize %d\n", _this->partitionASize);
    printf("partitionAIndex %d\n", _this->partitionAIndex);
    printf("partitionBSize %d\n", _this->partitionBSize);
    printf("partitionBIndex %d\n", _this->partitionBIndex);
    printf("reservedSize %d\n", _this->reservedSize);
    printf("reservedIndex %d\n", _this->reservedIndex);
    printf("bufferSize %d\n", _this->bufferSize);
    
    char* _buffer = (char*)_this;
    u32 idx;
    
    for(idx = 0; idx < BIPBUFFER_SIZE; idx++) {
        if(idx % sizeof(u32) == 0) {
            if(idx != 0) {
                printf("\n");
            }
            printf("Addr 0x%08x: ", idx);
        }
        printf("%02x ", _buffer[idx]);
    }
    printf("\n");
}

static int PrintBipBufferStructureInfo(CBipBuffer* _this)
{
    printf("partitionASize %d\n", _this->partitionASize);
    printf("partitionAIndex %d\n", _this->partitionAIndex);
    printf("partitionBSize %d\n", _this->partitionBSize);
    printf("partitionBIndex %d\n", _this->partitionBIndex);
    printf("reservedSize %d\n", _this->reservedSize);
    printf("reservedIndex %d\n", _this->reservedIndex);
    printf("bufferSize %d\n", _this->bufferSize);
}

static int PrintSerialData(char* data, u32 len)
{
    u32 i;
    
    for(i=0; i<len; i++) {
        if(i % sizeof(u32) == 0) {
            if(i != 0) {
                printf("\n");
            }
            printf("Addr 0x%08x: ", i);
        }
        printf("%02x ", data[i]);
    }
    printf("\n");
}
