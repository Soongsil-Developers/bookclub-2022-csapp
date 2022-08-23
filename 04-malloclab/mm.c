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
 * 
 * 내용 정리 : https://github.com/joojae02/TIL/blob/main/csapp/04-malloclab.md
 * 
 *      31                      3     0000
 * header      block size        0 0 a    a 할당 : 1
 * 
 *              데이터
 * 
 *              패딩
 * 
 * footer      block size        0 0 a    a 할당 : 1
 *   
 * 
 * 
 * 힙 영역을 늘릴 수 있어야 함.
 * 가용 블록 여부를 파악할 수 있어야 함.
 * 새롭게 할당하기 위해 적절한 가용블록을 선택할 수 있어야 함.
 * 블록을 할당한 후 남는 부분들을 처리해야 함.
 * 할당이 해제된 블록을 적절하게 합쳐야 함.     
 * 
 * 
 * 
 * 
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
    "noteam",
    /* First member's full name */
    "KUR",
    /* First member's email address */
    "nope"
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

static char *heap_listp;  // 처음에 쓸 큰 가용블록 힙을 만들어줌.
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t a_size);
static void place(void *bp, size_t a_size);
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
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // Create the initial empty heap
    if ((heap_listp =  (4*WSIZE)) == (void *)-1) {  // heap_listp가 힙의 최댓값 이상을 요청한다면 fail
        return -1;
    }

    PUT(heap_listp, 0);                             // Alignment padding 처음 한블럭 패딩
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));    // Prologue header  프롤로그 헤더
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    // Prologue footer  프롤로그 푸터
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));        // Epilogue header  에필로그 헤더 후에 뒤로 밀림
    heap_listp += (2*WSIZE);                        //prologue header와 footer 사이로 포인터

    // Extend the empty heap with a free block of CHUNKSIZE bytes
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) { //힙을 한번 할당
        return -1;
    }
    return 0;
}

/*
extend_heap 사용 경우
  1) 힙이 초기화될 때
  2) mm_malloc이 적당한 fit을 찾지 못했을 때

  새 가용 블록으로 힙 확장

  힙이 초기화 될 때 앞으로 블록을 넣기 위해 사이즈를 늘림
  맞춤 영역을 찾지 못했을 때 늘리는 것
*/
static void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    // Allocate an even number of words  to maintain alignment
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; // words가 홀수면 +1을 해서 공간 할당
    if ((long)(bp = mem_sbrk(size)) == -1) {
        return NULL;
    }

    // initialize free block header/footer and the epilogue header
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    /*
    extend_heap 블록 너머에 오지 않도록 배치한 블록 다음 공간을 블록이라 가정하고 epilogue header 배치
    (실제로는 존재하지 않는 블록)
    */

    // coalesce if the previous block was free
    return coalesce(bp);   
}

/*
  블럭 연결하는 함수
  p.818 9.37 그림 참고
  
  현재 블럭 주변 가용 블럭이 있는지 확인 후 있다면 합침
  합치지 않으면 메모리 낭비가 심함

  오류 단편화



*/
static void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); // 전 블럭 가용 확인
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp))); // 뒷 블럭 가용 확인
    size_t size = GET_SIZE(HDRP(bp));   

    // case1: 앞, 뒤 블록 모두 할당되어 있을 때
    if (prev_alloc && next_alloc) {
        return bp;
    }

    // case2: 앞 블록 할당, 뒷 블록 가용
    else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

    // case3: 앞 블록 가용, 뒷 블록 할당
    else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    // case4: 앞, 뒤 블록 모두 가용
    else {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
} 


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 * 
 * 
 */
void *mm_malloc(size_t size) {
    size_t a_size;       // adjusted block szie
    size_t extend_size;  // Amount to extend heap if no fit
    char *bp;

    // Ignore spurious requests
    if (size == 0) {
        return NULL;
    }

    // Adjust block size to include overhead and alignment reqs
    if (size <= DSIZE) {    // 2words 이하의 사이즈는 4워드로 할당 요청 (header 1word, footer 1word) 8byte
        a_size = 2*DSIZE;
    }
    else {                  // 할당 요청의 용량이 2words 초과 시, 충분한 8byte의 배수의 용량 할당
        a_size = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);
    }

    // Search the free list for a fit
    if ((bp = find_fit(a_size)) != NULL) {   // 적당한 크기의 가용 블록 검색
        place(bp, a_size);                   // 초과 부분을 분할하고 새롭게 할당한 블록의 포인터 반환
        return bp;
    }

    // NO fit found. Get more memory and place the block
    extend_size = MAX(a_size, CHUNKSIZE);
    if ((bp = extend_heap(extend_size/WSIZE)) == NULL) {    // 칸의 개수
        return NULL;
    }
    place(bp, a_size);
    return bp;
}
/*
  가용 가능한 블럭 찾는 함수

  first fit 사용


*/
static void *find_fit(size_t a_size) {
    void *bp;

    for (bp = (char *)heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (a_size <= GET_SIZE(HDRP(bp)))) {
            return bp;
        }
    }
    return NULL;    // NO fit
}
/*
  넣을 위치의 블록 사이즈를 계산

  블록 사이즈가 더 크면, 블록을 할당
  나머지 부분을 쪼개기

  크지 않다면 블럭을 할당
*/
static void place(void *bp, size_t a_size) {
    size_t c_size = GET_SIZE(HDRP(bp));

    if ((c_size - a_size) >= (2 * (DSIZE))) {
        // 요청 용량 만큼 블록 배치
        PUT(HDRP(bp), PACK(a_size, 1));
        PUT(FTRP(bp), PACK(a_size, 1));
        
        bp = NEXT_BLKP(bp);
        // 남은 블록에 header, footer 배치
        PUT(HDRP(bp), PACK(c_size - a_size, 0));
        PUT(FTRP(bp), PACK(c_size - a_size, 0));
    }
    else {      // csize와 aszie 차이가 네 칸(16byte)보다 작다면 해당 블록 통째로 사용
        PUT(HDRP(bp), PACK(c_size, 1));
        PUT(FTRP(bp), PACK(c_size, 1));
    }
}

/*
 * mm_free - Freeing a block does nothing.
 *
 * 블록을 반환
 * 블럭을 가용 상태로 바꿈
 * 
 * 블럭 반환후 가용 블럭이 생겼으므로 coalesce() 호출
 * 가용 블럭 연결
 * 
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 *
 * 크기를 조정할 블록의 위치와 요청 사이즈를 인자
 * size 만큼의 블록을 만들고 bp 위치에 복사
 * 
 * 
 */
void *mm_realloc(void *bp, size_t size)
{
    void *old_bp = bp;
    void *new_bp;
    size_t copySize;
    
    new_bp = mm_malloc(size);
    if (new_bp == NULL)
      return NULL;
    copySize = GET_SIZE(HDRP(old_bp));

    if (size < copySize)
      copySize = size;

    memcpy(new_bp, old_bp, copySize);  // 메모리의 특정한 부분으로부터 얼마까지의 부분을 다른 메모리 영역으로 복사해주는 함수(old_bp로부터 copySize만큼의 문자를 new_bp로 복사해라)
    mm_free(old_bp);
    return new_bp;
}