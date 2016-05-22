/**
*******************************************************************************
* @file                  $HeadURL$
* @version               $LastChangedRevision$
* @date                  $LastChangedDate$
* @author                $Author$
*
* @brief                 SCT_Containers_BipBuffer test implementation
* @module                Containers
*
* Copyright 2014 Nokia Networks. All rights reserved.
*******************************************************************************/

/*!
 *  @addtogroup dgBipBuffer
 */
/*@{*/

/************************** INCLUDED FILES ************************************/
#include "BipBuffer.h"
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include "AaInclude.h"

/************************** MACRO DEFINITIONS *********************************/
#define getDataByteCount()       (_this->partitionASize + _this->partitionBSize)
#define getPartitionBFreeSpace() (_this->partitionAIndex - _this->partitionBIndex - _this->partitionBSize)
#define getPartitionAFreeSpace() (_this->bufferSize - _this->partitionAIndex - _this->partitionASize)


/************************** PUBLIC VARIABLES **********************************/

/************************** PRIVATE VARIABLES *********************************/

/************************** PRIVATE FUNCTIONS *********************************/

/************************** PUBLIC FUNCTIONS **********************************/

/**
*******************************************************************************
*   
*   @brief      Function for construct the bib buffer. 
*
*   @param[in]  _this           Pointer to current instance
*   @param[in]  size            Size of the usable memory area. 
*     
*   @return     Success information of construct.
*   
*******************************************************************************/
i32 CBipBuffer_Construct(CBipBuffer* _this,
                         u32         size)
{
    memset(_this, 0, sizeof(*_this));

    _this->bufferSize = size - offsetof(CBipBuffer, buffer);
    
    /*!
     *  @todo remove this debug init
     */ 
    memset(_this->buffer, 0, _this->bufferSize);
    
    return 0;
}

/**
*******************************************************************************
*   
*   @brief    Function for destroy the bib buffer.
*
*   @param[in]  _this           Pointer to current instance
*
*******************************************************************************/
void CBipBuffer_Destroy(CBipBuffer* _this)
{
    memset(_this, 0, sizeof(*_this));
}

/**
*******************************************************************************
*   
*   @brief      Function for Reserve space in the buffer for a memory
*               write operation. 
* 
*               This function should be used if needed memory is not exactly
*               known. 
*
*   @param[in]  _this           Pointer to current instance
*   @param[in]  wantedSize      Amount of bytes to reserve
*   @param[out] actualSizePtr   Actually reserved size
*   
*   @return     Pointer to the reserved memory. 
*               - Will return NULL if no space that can be allocated.
*               - Will return NULL if a previous reservation has not been 
*                 commited.
*
*******************************************************************************/


/**
*******************************************************************************
*   
*   @brief      Function for Reserve space from the buffer for a memory
*               write operation.
*
*   @param[in]  _this           Pointer to current instance
*   @param[out] size            Amount of space actually reserved
* 
*   @return     will return NULL if not enough space in buffer.
*
*******************************************************************************/
void* CBipBuffer_Reserve(CBipBuffer* _this, u32 size)
{
    u32 freeSpace = 0;

    
    /*
     *  Allways allocate new space from partition B if it exist.
     */
    if( _this->partitionBSize != 0 )
    {
        freeSpace = getPartitionBFreeSpace();

        if( freeSpace == 0 || 
            freeSpace < size )
        {
            return NULL;
        }

        _this->reservedSize = size;
        _this->reservedIndex = _this->partitionBIndex + _this->partitionBSize;
       
        return &_this->buffer[_this->reservedIndex];
    }
    else
    {
        /*
         *  Partition B does not exist, allocate space from partition A
         *  if it there is enough space, otherwise try from partition B.
         */ 
        freeSpace = getPartitionAFreeSpace();
        
        if( freeSpace >= size )
        {
            if( freeSpace == 0 )
            {
                return NULL;
            }
           
            _this->reservedSize = size;
            _this->reservedIndex = _this->partitionAIndex + _this->partitionASize;
            
            return &_this->buffer[_this->reservedIndex];
        }
        else
        {
            if(0 < _this->partitionAIndex && 0 == _this->partitionASize && size <= _this->bufferSize) // if partitionAIndex is in the middle, and partitionASize is equal to 0
            {
                _this->reservedSize = size;
                _this->partitionAIndex = 0;
                _this->reservedIndex = 0;

                 return &_this->buffer[0]; // return starting point of buffer
            }
            else if( _this->partitionAIndex == 0 || _this->partitionAIndex < size || size > _this->bufferSize)
            {
                return NULL;
            }

            _this->reservedSize = size;
            _this->reservedIndex = 0;
           
            return &_this->buffer[0];
        }
    }
}

/**
*******************************************************************************
*   
*   @brief      Function for commit actual needed space at reserve.
*
*
*
*   @param[in]  _this           Pointer to current instance
*   @param[in]  size            Amount of bytes to commit
*
*******************************************************************************/
void CBipBuffer_Commit(CBipBuffer* _this, u32 size)
{
    if( size == 0 )
    {
        /*
         *  Decommit any reservation
         */
        _this->reservedSize = 0;
        _this->reservedIndex = 0;
    
        return;
    }

    /*
     *  Prevent trying to commit more space than asked for.
     */
    if( size > _this->reservedSize )
    {
        size = _this->reservedSize;
    }

    /*
     *  When no regions being used, region A is reserved.
     */
    if( _this->partitionASize == 0 && _this->partitionBSize == 0 )
    {
        _this->partitionAIndex = _this->reservedIndex;
        _this->partitionASize = size;

        _this->reservedIndex = 0;
        _this->reservedSize = 0;
        
        return;
    }

    if( _this->reservedIndex == (_this->partitionASize + _this->partitionAIndex) )
    {
        _this->partitionASize += size;
    }
    else
    {
        _this->partitionBSize += size;
    }

    _this->reservedIndex = 0;
    _this->reservedSize = 0;

}

/**
*******************************************************************************
*   
*   @brief    Function for get pointer to data that can be read out
*             of the buffer.
*
*   @param[in]  _this                     Pointer to current instance
*   @param[out] countinousBlockSizePtr    Size of continous data block that 
*                                         can be read.
*
*******************************************************************************/
void* CBipBuffer_Get(CBipBuffer* _this, u32* countinousBlockSizePtr)
{
    if( _this->partitionASize == 0 )
    {
        *countinousBlockSizePtr = 0;       
        return NULL;
    }

    *countinousBlockSizePtr = _this->partitionASize;
        
    return &_this->buffer[_this->partitionAIndex];
}

/**
*******************************************************************************
*   
*   @brief    Function for decommit space from buffer after data is read
*             from buffer. 
* 
*             Chooses the bigger empty region.
*
*   @param[in]  _this           Pointer to current instance
*   @param[in]  size            Amount of bytes to decommit
*
*******************************************************************************/

/**
*******************************************************************************
*   
*   @brief    Function for decommit space from buffer after data is read
*             from buffer. 
* 
*             Acts like regular ring buffer.
*
*   @param[in]  _this           Pointer to current instance
*   @param[in]  size            Amount of bytes to decommit
*
*******************************************************************************/
void CBipBuffer_Decommit(CBipBuffer* _this, u32 size)
{
    if( size >= _this->partitionASize &&
        _this->partitionBSize != 0 )
    {
        _this->partitionAIndex = _this->partitionBIndex;
        _this->partitionASize = _this->partitionBSize;
        
        _this->partitionBIndex = 0;
        _this->partitionBSize = 0;
    }
    else
    {
        _this->partitionASize -= size;
        _this->partitionAIndex += size;
    }
}

/**
*******************************************************************************
*   
*   @brief      Function for get buffer fullness information.
*
*   @param[in]  _this           Pointer to current instance
*
*   @return     buffer fullness in procents.
*
*******************************************************************************/


/**
*******************************************************************************
*   
*   @brief      Function for query how much data (in total) has been added
*               into buffer.
*
*   @param[in]  _this           Pointer to current instance
*
*   @return     Bytes of data has been added to buffer. 
*
*******************************************************************************/
/* INLINE */ u32 CBipBuffer_HowMuchData(CBipBuffer* _this)
{
    return getDataByteCount(); 
}

/*@}*/
