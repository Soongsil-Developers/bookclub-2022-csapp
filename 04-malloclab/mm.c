/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "SSU_csapp_bookclub_2022",
    /* First member's full name */
    "Kim Yoonsu",
    /* First member's email address */
    "nope"
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* 그림 9.45 extend_heap 새 가용블록으로 힙 확장하기 */
static void *extend_heap(size_t words){
    char *bp;
    size_t size;

    size = (words %2) ? (words +1) * WSIZE : words * WSIZE;
    if( (long)(bp = mem_sbrk(size)) == -1 )
        return NULL;
    PUT(HDRP(bp), PACK(size,0)); /* free block header */
    PUT(FTRP(bp), PACK(size,0)); /* free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK((0,1)));

    return coalesced(bp);
}

/* 
 * mm_init - initialize the malloc package.
 * 
 * to use my malloc package, you must call this function first.
 *
 * ret
 * -1   initialization failed.
 * 0    initialization succeeded.
 *
 */
static char *heap_listp;
int mm_init(void)
{
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1 )
        return -1;
    PUT(heap_listp, 0);
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE,1));
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE,1));
    PUT(heap_listp + (3*WSIZE), PACK(0,1));
    heap_listp += (2*WSIZE);

    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *
 * Always allocate a block whose size is a multiple of the alignment.
 * size of block is AT LEAST size(param) byte. it would be bigger than
 * size(param)
 *
 *ret
 * NULL failed.
 * ptr  valid address of allocated block PAYLOAD.
 */
void* mm_malloc(size_t size)
{
    if(size == 0){
        return NULL;
    }

    int     newsize = ALIGN(size + WSIZE);  // add header(4byte)!
    void*   ptr     = mem_sbrk(newsize);    // ptr must be aligned.
    if(ptr == (void*)-1)
        return NULL;
    else{
        *(size_t*)ptr = PACK(newsize,1); // BLOCK size! NOT PAYLOAD size!
                printf("-origin size = %d \n", size);
                printf("-aligned size = %d \n", ALIGN(size));
                printf("-ptr = %p \n", ptr);
                printf("-in malloc %d \n", GET(ptr));
        return (void*)((char*)ptr + WSIZE); // ptr points payload!
    } 
}

/*
 * mm_free - Freeing a block.
 * 그림 9.46 
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesced(bp);
}

static void *coalesced(void *bp){
    size_t prev_alloc = GET_ALLOC( FTRP(PREV_BLKP(bp)) );
    size_t next_alloc = GET_ALLOC( HDRP(NEXT_BLKP(bp)) );
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc){ /* cast 1 */
        return bp;
    }
    else if( prev_alloc && !next_alloc ){ /* case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size,0));
        PUT(FTRP(bp), PACK(size,0));
    }
    else if( !prev_alloc && next_alloc){ /* case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0) );
        bp = PREV_BLKP(bp);
    }
    else{ /* case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 *
 * (not now)
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

//----------mine----------

/*
 * error checked mm_init. 
 */
void Mm_init(void)
{
    if(mm_init() == -1){      
        puts("mm_init failed!");
        exit(1);
    }
}












