/*
 * File      : mem.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2008 - 2012, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2008-7-12      Bernard      the first version
 * 2010-06-09     Bernard      fix the end stub of heap
 *                             fix memory check in AaMemRealloc function
 * 2010-07-13     Bernard      fix RT_ALIGN issue found by kuronca
 * 2010-10-14     Bernard      fix AaMemRealloc issue when realloc a NULL pointer.
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *         Simon Goldschmidt
 *
 */


#include "AaMem.h"
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>


/** for debug */
#define AAMEM_STATUS_DEBUG



#ifdef AAMEM_USING_HOOK
static void (*aamem_malloc_hook)(void *ptr, u32 size);
static void (*aamem_free_hook)(void *ptr);

/**
 * @addtogroup Hook
 */

/*@{*/

/**
 * This function will set a hook function, which will be invoked when a memory
 * block is allocated from heap memory.
 *
 * @param hook the hook function
 */
void AaMemMallocSetHook(void (*hook)(void *ptr, u32 size))
{
    aamem_malloc_hook = hook;
}

/**
 * This function will set a hook function, which will be invoked when a memory
 * block is released to heap memory.
 *
 * @param hook the hook function
 */
void AaMemFreeSetHook(void (*hook)(void *ptr))
{
    aamem_free_hook = hook;
}

/*@}*/

#endif



#define HEAP_MAGIC 0x1ea0
struct SHeapMem {
    /* magic and used flag */
    u16 magic;
    u16 used;

    u32 next, prev;
};



/** pointer to the heap: for alignment, heap_ptr is now a pointer instead of an array */
static u8 *heap_ptr;



/** the last entry, always unused! */
static struct SHeapMem *heap_end;



/**
 * @ingroup BasicDef
 *
 * @def RT_ALIGN(size, align)
 * Return the most contiguous size aligned at specified width. RT_ALIGN(13, 4)
 * would return 16.
 */
#define RT_ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1))

/**
 * @ingroup BasicDef
 *
 * @def RT_ALIGN_DOWN(size, align)
 * Return the down number of aligned at specified width. RT_ALIGN_DOWN(13, 4)
 * would return 12.
 */
#define RT_ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))


#define RT_ASSERT (void)


/** minimum malloc memery size */ 
#define MIN_SIZE 8

/** minimum align size */ 
#define RT_ALIGN_SIZE 4



#define MIN_SIZE_ALIGNED     RT_ALIGN(MIN_SIZE, RT_ALIGN_SIZE)
#define SIZEOF_STRUCT_MEM    RT_ALIGN(sizeof(struct SHeapMem), RT_ALIGN_SIZE)



static struct SHeapMem *lfree;   /* pointer to the lowest free block */


/*
static osSemaphoreDef(heap_sem);
static osSemaphoreId _heap_sem_id;
*/
sem_t _heap_sem_id;

static u32 mem_size_aligned;



#ifdef AAMEM_STATUS_DEBUG
static u32 used_mem, max_mem;
#endif



static void plug_holes(struct SHeapMem *mem)
{
    struct SHeapMem *nmem;
    struct SHeapMem *pmem;

    RT_ASSERT((u8 *)mem >= heap_ptr);
    RT_ASSERT((u8 *)mem < (u8 *)heap_end);
    RT_ASSERT(mem->used == 0);

    /* plug hole forward */
    nmem = (struct SHeapMem *)&heap_ptr[mem->next];
    if (mem != nmem &&
        nmem->used == 0 &&
        (u8 *)nmem != (u8 *)heap_end)
    {
        /* if mem->next is unused and not end of heap_ptr,
         * combine mem and mem->next
         */
        if (lfree == nmem)
        {
            lfree = mem;
        }
        mem->next = nmem->next;
        ((struct SHeapMem *)&heap_ptr[nmem->next])->prev = (u8 *)mem - heap_ptr;
    }

    /* plug hole backward */
    pmem = (struct SHeapMem *)&heap_ptr[mem->prev];
    if (pmem != mem && pmem->used == 0)
    {
        /* if mem->prev is unused, combine mem and mem->prev */
        if (lfree == mem)
        {
            lfree = pmem;
        }
        pmem->next = mem->next;
        ((struct SHeapMem *)&heap_ptr[mem->next])->prev = (u8 *)pmem - heap_ptr;
    }
}

/**
 * @ingroup SystemInit
 *
 * This function will initialize system heap memory.
 *
 * @param begin_addr the beginning address of system heap memory.
 * @param end_addr the end address of system heap memory.
 */
void AaMemHeapInit(void *begin_addr, void *end_addr)
{
    struct SHeapMem *mem;
    u32 begin_align = RT_ALIGN((u32)begin_addr, RT_ALIGN_SIZE);
    u32 end_align = RT_ALIGN_DOWN((u32)end_addr, RT_ALIGN_SIZE);

    /* alignment addr */
    if ((end_align > (2 * SIZEOF_STRUCT_MEM)) &&
        ((end_align - 2 * SIZEOF_STRUCT_MEM) >= begin_align))
    {
        /* calculate the aligned memory size */
        mem_size_aligned = end_align - begin_align - 2 * SIZEOF_STRUCT_MEM;
    }
    else
    {
        printf("mem init, error begin address 0x%x, and end address 0x%x\n", (u32)begin_addr, (u32)end_addr);

        return;
    }

    /* point to begin address of heap */
    heap_ptr = (u8 *)begin_align;

    printf("mem init, heap begin address 0x%x, size %d\n", (u32)heap_ptr, mem_size_aligned);

    /* initialize the start of the heap */
    mem        = (struct SHeapMem *)heap_ptr;
    mem->magic = HEAP_MAGIC;
    mem->next  = mem_size_aligned + SIZEOF_STRUCT_MEM;
    mem->prev  = 0;
    mem->used  = 0;

    /* initialize the end of the heap */
    heap_end        = (struct SHeapMem *)&heap_ptr[mem->next];
    heap_end->magic = HEAP_MAGIC;
    heap_end->used  = 1;
    heap_end->next  = mem_size_aligned + SIZEOF_STRUCT_MEM;
    heap_end->prev  = mem_size_aligned + SIZEOF_STRUCT_MEM;

    //_heap_sem_id = osSemaphoreCreate (osSemaphore(heap_sem), 1);
    sem_init(&_heap_sem_id, 0, 1);

    /* initialize the lowest-free pointer to the start of the heap */
    lfree = (struct SHeapMem *)heap_ptr;
}

/**
 * @addtogroup MM
 */

/*@{*/

/**
 * Allocate a block of memory with a minimum of 'size' bytes.
 *
 * @param size is the minimum size of the requested block in bytes.
 *
 * @return pointer to allocated memory or NULL if no free memory was found.
 */
void *AaMemMalloc(u32 size)
{
    u32 ptr, ptr2;
    struct SHeapMem *mem, *mem2;

    if (size == 0)
        return NULL;

    if (size != RT_ALIGN(size, RT_ALIGN_SIZE))
        printf("malloc size %d, but align to %d\n", size, RT_ALIGN(size, RT_ALIGN_SIZE));
    else
        printf("malloc size %d\n", size);

    /* alignment size */
    size = RT_ALIGN(size, RT_ALIGN_SIZE);

    if (size > mem_size_aligned)
    {
        printf("no memory\n");

        return NULL;
    }

    /* every data block must be at least MIN_SIZE_ALIGNED long */
    if (size < MIN_SIZE_ALIGNED)
        size = MIN_SIZE_ALIGNED;

    /* take memory semaphore */
    sem_wait(&_heap_sem_id);

    for (ptr = (u8 *)lfree - heap_ptr;
         ptr < mem_size_aligned - size;
         ptr = ((struct SHeapMem *)&heap_ptr[ptr])->next)
    {
        mem = (struct SHeapMem *)&heap_ptr[ptr];

        if ((!mem->used) && (mem->next - (ptr + SIZEOF_STRUCT_MEM)) >= size)
        {
            /* mem is not used and at least perfect fit is possible:
             * mem->next - (ptr + SIZEOF_STRUCT_MEM) gives us the 'user data size' of mem */

            if (mem->next - (ptr + SIZEOF_STRUCT_MEM) >=
                (size + SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED))
            {
                /* (in addition to the above, we test if another struct SHeapMem (SIZEOF_STRUCT_MEM) containing
                 * at least MIN_SIZE_ALIGNED of data also fits in the 'user data space' of 'mem')
                 * -> split large block, create empty remainder,
                 * remainder must be large enough to contain MIN_SIZE_ALIGNED data: if
                 * mem->next - (ptr + (2*SIZEOF_STRUCT_MEM)) == size,
                 * struct SHeapMem would fit in but no data between mem2 and mem2->next
                 * @todo we could leave out MIN_SIZE_ALIGNED. We would create an empty
                 *       region that couldn't hold data, but when mem->next gets freed,
                 *       the 2 regions would be combined, resulting in more free memory
                 */
                ptr2 = ptr + SIZEOF_STRUCT_MEM + size;

                /* create mem2 struct */
                mem2       = (struct SHeapMem *)&heap_ptr[ptr2];
                mem2->used = 0;
                mem2->next = mem->next;
                mem2->prev = ptr;

                /* and insert it between mem and mem->next */
                mem->next = ptr2;
                mem->used = 1;

                if (mem2->next != mem_size_aligned + SIZEOF_STRUCT_MEM)
                {
                    ((struct SHeapMem *)&heap_ptr[mem2->next])->prev = ptr2;
                }
#ifdef AAMEM_STATUS_DEBUG
                used_mem += (size + SIZEOF_STRUCT_MEM);
                if (max_mem < used_mem)
                    max_mem = used_mem;
#endif
            }
            else
            {
                /* (a mem2 struct does no fit into the user data space of mem and mem->next will always
                 * be used at this point: if not we have 2 unused structs in a row, plug_holes should have
                 * take care of this).
                 * -> near fit or excact fit: do not split, no mem2 creation
                 * also can't move mem->next directly behind mem, since mem->next
                 * will always be used at this point!
                 */
                mem->used = 1;
#ifdef AAMEM_STATUS_DEBUG
                used_mem += mem->next - ((u8*)mem - heap_ptr);
                if (max_mem < used_mem)
                    max_mem = used_mem;
#endif
            }
            /* set memory block magic */
            mem->magic = HEAP_MAGIC;

            if (mem == lfree)
            {
                /* Find next free block after mem and update lowest free pointer */
                while (lfree->used && lfree != heap_end)
                    lfree = (struct SHeapMem *)&heap_ptr[lfree->next];

                RT_ASSERT(((lfree == heap_end) || (!lfree->used)));
            }
            
            sem_post(&_heap_sem_id);
            
            RT_ASSERT((u32)mem + SIZEOF_STRUCT_MEM + size <= (u32)heap_end);
            RT_ASSERT((u32)((u8 *)mem + SIZEOF_STRUCT_MEM) % RT_ALIGN_SIZE == 0);
            RT_ASSERT((((u32)mem) & (RT_ALIGN_SIZE-1)) == 0);

            printf("allocate memory at 0x%x, size: %d\n",
                        (u32)((u8 *)mem + SIZEOF_STRUCT_MEM),
                        (u32)(mem->next - ((u8 *)mem - heap_ptr)));

            //RT_OBJECT_HOOK_CALL(aamem_malloc_hook, (((void *)((u8 *)mem + SIZEOF_STRUCT_MEM)), size));

            /* return the memory data except mem struct */
            return (u8 *)mem + SIZEOF_STRUCT_MEM;
        }
    }

    sem_post(&_heap_sem_id);

    return NULL;
}

/**
 * This function will change the previously allocated memory block.
 *
 * @param rmem pointer to memory allocated by AaMemMalloc
 * @param newsize the required new size
 *
 * @return the changed memory block address
 */
void *AaMemRealloc(void *rmem, u32 newsize)
{
    u32 size;
    u32 ptr, ptr2;
    struct SHeapMem *mem, *mem2;
    void *nmem;

    /* alignment size */
    newsize = RT_ALIGN(newsize, RT_ALIGN_SIZE);
    if (newsize > mem_size_aligned)
    {
        printf("realloc: out of memory\n");

        return NULL;
    }

    /* allocate a new memory block */
    if (rmem == NULL)
        return AaMemMalloc(newsize);

    sem_wait(&_heap_sem_id);

    if ((u8 *)rmem < (u8 *)heap_ptr ||
        (u8 *)rmem >= (u8 *)heap_end)
    {
        /* illegal memory */
        sem_post(&_heap_sem_id);

        return rmem;
    }

    mem = (struct SHeapMem *)((u8 *)rmem - SIZEOF_STRUCT_MEM);

    ptr = (u8 *)mem - heap_ptr;
    size = mem->next - ptr - SIZEOF_STRUCT_MEM;
    if (size == newsize)
    {
        /* the size is the same as */
        sem_post(&_heap_sem_id);

        return rmem;
    }

    if (newsize + SIZEOF_STRUCT_MEM + MIN_SIZE < size)
    {
        /* split memory block */
#ifdef AAMEM_STATUS_DEBUG
        used_mem -= (size - newsize);
#endif

        ptr2 = ptr + SIZEOF_STRUCT_MEM + newsize;
        mem2 = (struct SHeapMem *)&heap_ptr[ptr2];
        mem2->magic= HEAP_MAGIC;
        mem2->used = 0;
        mem2->next = mem->next;
        mem2->prev = ptr;
        mem->next = ptr2;
        if (mem2->next != mem_size_aligned + SIZEOF_STRUCT_MEM)
        {
            ((struct SHeapMem *)&heap_ptr[mem2->next])->prev = ptr2;
        }

        plug_holes(mem2);

        sem_post(&_heap_sem_id);

        return rmem;
    }
    sem_post(&_heap_sem_id);

    /* expand memory */
    nmem = AaMemMalloc(newsize);
    if (nmem != NULL) /* check memory */
    {
        memcpy(nmem, rmem, size < newsize ? size : newsize);
        AaMemFree(rmem);
    }

    return nmem;
}

/**
 * This function will contiguously allocate enough space for count objects
 * that are size bytes of memory each and returns a pointer to the allocated
 * memory.
 *
 * The allocated memory is filled with bytes of value zero.
 *
 * @param count number of objects to allocate
 * @param size size of the objects to allocate
 *
 * @return pointer to allocated memory / NULL pointer if there is an error
 */
void *AaMemCalloc(u32 count, u32 size)
{
    void *p;

    /* allocate 'count' objects of size 'size' */
    p = AaMemMalloc(count * size);

    /* zero the memory */
    if (p)
        memset(p, 0, count * size);

    return p;
}

/**
 * This function will release the previously allocated memory block by
 * AaMemMalloc. The released memory block is taken back to system heap.
 *
 * @param rmem the address of memory which will be released
 */
void AaMemFree(void *rmem)
{
    struct SHeapMem *mem;

    if (rmem == NULL)
        return;
    RT_ASSERT((((u32)rmem) & (RT_ALIGN_SIZE-1)) == 0);
    RT_ASSERT((u8 *)rmem >= (u8 *)heap_ptr &&
              (u8 *)rmem < (u8 *)heap_end);

    //RT_OBJECT_HOOK_CALL(aamem_free_hook, (rmem));

    if ((u8 *)rmem < (u8 *)heap_ptr ||
        (u8 *)rmem >= (u8 *)heap_end)
    {
        printf("illegal memory\n");

        return;
    }

    /* Get the corresponding struct SHeapMem ... */
    mem = (struct SHeapMem *)((u8 *)rmem - SIZEOF_STRUCT_MEM);

    printf("release memory 0x%x, size: %d\n",
                  (u32)rmem,
                  (u32)(mem->next - ((u8 *)mem - heap_ptr)));


    /* protect the heap from concurrent access */
    sem_wait(&_heap_sem_id);

    /* ... which has to be in a used state ... */
    RT_ASSERT(mem->used);
    RT_ASSERT(mem->magic == HEAP_MAGIC);
    /* ... and is now unused. */
    mem->used  = 0;
    mem->magic = 0;

    if (mem < lfree)
    {
        /* the newly freed struct is now the lowest */
        lfree = mem;
    }

#ifdef AAMEM_STATUS_DEBUG
    used_mem -= (mem->next - ((u8*)mem - heap_ptr));
#endif

    /* finally, see if prev or next are free also */
    plug_holes(mem);
    sem_post(&_heap_sem_id);
}


#ifdef AAMEM_STATUS_DEBUG
void AaMemInfo(u32 *total, u32 *used, u32 *max_used)
{
    if (total != NULL)
        *total = mem_size_aligned;
    if (used  != NULL)
        *used = used_mem;
    if (max_used != NULL)
        *max_used = max_mem;
}

void AaMemList(void)
{
    printf("total memory: %d\n", mem_size_aligned);
    printf("used memory : %d\n", used_mem);
    printf("maximum allocated memory: %d\n", max_mem);
}
#endif

/*@}*/


