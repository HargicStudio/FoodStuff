/**
*******************************************************************************
* @file                  $HeadURL$
* @version               $LastChangedRevision$
* @date                  $LastChangedDate$
* @author                $Author$
*
* @brief                 Bi Partion Byte Buffer.
* @module                Containers
* @owner                 Vladimir Cundekovic 
*
* Copyright 2014 Nokia Solutions and Networks. All rights reserved.
*******************************************************************************/

#ifndef BIP_BUFFER_H
#define BIP_BUFFER_H

#ifdef __cplusplus
extern "C"{
#endif

/************************** DOXYGEN GROUPS ************************************/

/*!
 *  @ingroup  dgBuffer
 *  @defgroup dgBipBuffer Bi Partition Byte Buffer
 * 
 *  This module..
 * 
 */
/*@{*/


/************************** INCLUDED FILES ************************************/
//#include <glo_def.h>

/************************** PUBLIC DECLARATIONS *******************************/

/*! 
 *  @brief  Type for Bip buffer.
 */
typedef struct CBipBuffer
{
    /*!
     *  @brief  Continous data partition A size.
     */        
    u32     partitionASize;   
    /*!
     *  @brief  Continous data partition A start index.
     */            
    u32     partitionAIndex;    
    /*! 
     *  @brief  Continous data partition B size.
     */
    u32     partitionBSize;     
    /*!
     *  @brief  Continous data partition B start index.
     */         
    u32     partitionBIndex;    
    /*!
     *  @brief  Reserved data block size.
     *
     *  @todo   do we need nested reserve support for simultanous usage?
     */           
    u32     reservedSize;   
    /*!
     *  @brief  Reserved data block start index.
     */
    u32     reservedIndex;    
    /*!
     *  @brief  Size of the usable buffer area.
     */
    u32     bufferSize;     /* memorySize */    
    /*!
     *  @brief  Pointer to usable buffer base address.
     */
    char    buffer[1];      /* memoryBaseAddress */  
}CBipBuffer;

/************************** PUBLIC INTERFACES *********************************/


/**
*******************************************************************************
*   
*   @brief      Function for construct the bib buffer. 
*
*   @param[in]  _this           Pointer to current instance
*   @param[in]  size            Size of the usable memory area
*  
*   @return     success information of construct
*               - Will return non zero is failed   
*
*******************************************************************************/
i32 CBipBuffer_Construct(CBipBuffer* _this,
                         u32         size);

/**
*******************************************************************************
*   
*   @brief    Function for destroy the bib buffer.
*
*   @param[in]  _this           Pointer to current instance
*
*******************************************************************************/
void CBipBuffer_Destroy(CBipBuffer* _this);

/**
*******************************************************************************
*   
*   @brief      Function for Reserve space in the buffer for a memory
*               write operation.
*
*   @param[in]  _this           Pointer to current instance
*   @param[in]  size            Amount of bytes to reserve
*   
*   @return     Pointer to the reserved memory. 
*               - Will return NULL if no enough space that is wanted.
*               - Will return NULL if a previous reservation has not been 
*                 commited.
*
*******************************************************************************/
void* CBipBuffer_Reserve(CBipBuffer* _this, u32 size);

/**
*******************************************************************************
*   
*   @brief      Function for commit space that has been written in the buffer.
*
*   @param[in]  _this           Pointer to current instance
*   @param[in]  size            Amount of bytes to commit
*
*******************************************************************************/
void CBipBuffer_Commit(CBipBuffer* _this, u32 size);

/**
*******************************************************************************
*   
*   @brief      Function for get pointer to continous data block that can be 
*               read out of the buffer. 
*
*   @param[in]  _this                     Pointer to current instance
*   @param[in]  countinousBlockSizePtr    Size of continous data block that 
*                                         can be read.
*
*   @return     Pointer to first countinous data block.
*               - Will return NULL is buffer is empty
*
*******************************************************************************/
void* CBipBuffer_Get(CBipBuffer* _this, u32* countinousBlockSizePtr);

/**
*******************************************************************************
*   
*   @brief      Function for decommit space from buffer after data is read
*               from buffer. 
*
*   @param[in]  _this           Pointer to current instance
*   @param[in]  size            Amount of bytes to decommit
*
*******************************************************************************/
void CBipBuffer_Decommit(CBipBuffer* _this, u32 size);

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
/* INLINE */ u32 CBipBuffer_HowMuchData(CBipBuffer* _this);

/*@}*/

#ifdef __cplusplus
}
#endif
#endif /* BIP_BUFFER_H */
