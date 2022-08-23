#include <stdio.h>
#include <assert.h>

extern int      mm_init(void);
extern void*    mm_malloc(size_t size);
extern void     mm_free(void* ptr);
extern void*    mm_realloc(void* ptr, size_t size);

//mine
extern void     Mm_init(void);
#define WSIZE       4
#define DSIZE       8
#define CHUNKSIZE   (1 << 12)   // Extend heap by this amount (bytes) : 초기 가용 블록과 힙 확장을 위한 기본 크기

/* MAX함수 정의 */
#define MAX(x, y) ((x) > (y) ? (x) : (y))
/* single word (4) or double word (8) alignment */
#define ALIGNMENT   8

/* Pack a size and allocated bit int a word */
#define PACK(size, alloc)   ((size) | (alloc))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size)     (((size) + (ALIGNMENT-1)) & ~0x7) // ALIGNMENT와 가장 근접한 8배수(ALLIGNMENT배수)로 올림 

/* Read and write a word at address p */
#define GET(p)          (*(unsigned int*)(p)) 
#define PUT(p, val)     (*(unsigned int*)(p) = (val))

// Read the size and allocated field from address p
#define GET_SIZE(p)    (GET(p) & ~0x7)  // header or footer의 사이즈 반환(8의 배수)
#define GET_ALLOC(p)   (GET(p) & 0x1)   // 현재 블록 가용 여부 판단(0이면 alloc, 1이면 free)

// /* Get the size and allocated fields from baseptr's HEADER */
// #define GET_SIZE(bp)    GET_SIZE_BITS( HDRP(bp) )
// #define IS_ALLOC(bp)    GET_ALLOC_BIT( HDRP(bp) )

/* Set block size 
 * NOTE: size must be ALIGNED! */
#define SET_SIZE(bp, size)  do{                                 \
                                assert(size % ALIGNMENT == 0);  \
                            }while(0);
//PUT(HDRP(bp), size);

#define SET_ALLOC(bp,val)   do{                                 \
                                assert(val == 0 || val == 1);   \
                            }while(0);
                                    

/* Ginven block ptr bp, compute address of its header and footer*/
// bp(현재 블록의 포인터)로 현재 블록의 header 위치와 footer 위치 반환
#define HDRP(bp)    ((char *)(bp) - WSIZE)
#define FTRP(bp)    ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define SIZE_T_SIZE     (ALIGN(sizeof(size_t)))

// 다음과 이전 블록의 포인터 반환
#define NEXT_BLKP(bp)   (((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE)))    // 다음 블록 bp 위치 반환(bp + 현재 블록의 크기)
#define PREV_BLKP(bp)   (((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE)))    // 이전 블록 bp 위치 반환(bp - 이전 블록의 크기)
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
   
