#include <stdio.h>
#include <assert.h>

extern int      mm_init(void);
extern void*    mm_malloc(size_t size);
extern void     mm_free(void* ptr);
extern void*    mm_realloc(void* ptr, size_t size);
static void*    extend_heap(size_t words);
extern void     Mm_init(void);

#define WSIZE       4
#define DSIZE       8
#define CHUNKSIZE (1<<12)

#define MAX(x,y) ( (x) > (y)? (x) : (y) )
/* single word (4) or double word (8) alignment */
#define ALIGNMENT   8

/* Pack a size and allocated bit int a word */
#define PACK(size, alloc)   ((size) | (alloc))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size)     (((size) + (ALIGNMENT-1)) & ~0x7)

/* Read and write a word at address p */
#define GET(p)          (*(unsigned int*)(p)) 
#define PUT(p, val)     (*(unsigned int*)(p) = (val))
/* Read the size and allocated fields from addres's p (NOT from HEAD!)*/
#define GET_SIZE(p)    (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1) 
                     

/* Ginven block ptr bp, compute address of its header and footer*/
#define HDRP(bp)        ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))
/* 
 * Students work in teams of one or two.  Teams enter their team name, 
 * personal names and login IDs in a struct of this
 * type in their bits.c file.
 */
typedef struct {
    char*   teamname; /* ID1+ID2 or ID1 */
    char*   name1;    /* full name of first member */
    char*   id1;      /* login ID of first member */
    char*   name2;    /* full name of second member (if any) */
    char*   id2;      /* login ID of second member */
} team_t;

extern team_t team;
   
