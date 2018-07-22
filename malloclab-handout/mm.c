/*
 * mm.c - The fastest, least memory-efficient malloc package.
 * 
 * author: Husy
 * explicit linked list + fisrt-fit + boundary tag
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
    /* First member's full name */
    "Team",
    "350980310@qq.cm",      /* login ID of first member */
    "Husy1",
    "1234567",
    "Husy2"
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define CHUNKSIZE  (1<<12)  //每次一扩展就扩展4Kb堆内存
#define MAX(x, y) ((x) > (y)? (x) : (y))  
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1)) //堆size进行内存对齐，对齐的标准是ALIGNMENT
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))  //一个块头部大小
#define DSIZE_T_SIZE (SIZE_T_SIZE*2)   
#define PACK(size, alloc)  ((size) | (alloc)) //将size和分配位打包成一个值
#define GET_SIZE(p)  (GET(p) & ~(ALIGNMENT-1)) //获取头部的size
#define GET_ALLOC(p) (GET(p) & 0x1)  //获取头部的分配位
#define GET(p)       (*(unsigned int *)(p))    //获取当前p指针指向的值
#define PUT(p, val)  (*(unsigned int *)(p) = (val))   //将val赋值给p指针指向的值
#define HDRP(bp)       ((char *)(bp) - SIZE_T_SIZE)         //获取bp指向的内存块的头部
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE_T_SIZE) //获取bp指向的内存块的脚部
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - SIZE_T_SIZE))) //获取bp指向的内存块的下一个内存块
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE_T_SIZE))) //获取bp指向的内存块的上一个内存块

static char *heap_listp = 0;
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
// static void printblock(void *bp); 
// static void checkheap(int verbose);
// static void checkblock(void *bp);

int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4*SIZE_T_SIZE)) == (void *)-1) //line:vm:mm:begininit
        return -1;
    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*SIZE_T_SIZE), PACK(DSIZE_T_SIZE, 1)); /* Prologue header */ 
    PUT(heap_listp + (2*SIZE_T_SIZE), PACK(DSIZE_T_SIZE, 1)); /* Prologue footer */ 
    PUT(heap_listp + (3*SIZE_T_SIZE), PACK(0, 1));     /* Epilogue header */
    heap_listp += DSIZE_T_SIZE; 
    if (extend_heap(CHUNKSIZE/SIZE_T_SIZE) == NULL) 
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 *  注意：头部里面的块大小是整个块的大小，包括头部脚部和有效载荷及填充
 */
void *mm_malloc(size_t size)
{
    if (heap_listp == 0){
        mm_init();
    }
    int newsize = ALIGN(size + DSIZE_T_SIZE); //这里因为有头部和脚部，所以乘2；另外就是理论上来说这里的newsize是size_t类型，但是由于mem_sbrk是int作为参数，我们这里简单地把size_t看作int
    char *bp; 
    if ((bp = find_fit(newsize)) != NULL) {  //找到了的话
        place(bp, newsize);                 
        return bp;
    }

    int extendsize=0;
   //如果找不到合适的
    extendsize = MAX(newsize,CHUNKSIZE);                 //至少扩展4kb
    if ((bp = extend_heap(extendsize)) == NULL)  
        return NULL;                                  
    place(bp, newsize);   //直接从bp指向的块分割。                            
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    if (bp == 0) 
        return;

    size_t size = GET_SIZE(HDRP(bp));
    if (heap_listp == 0){
        mm_init();
    }
    //修改当前块的分配位
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);

}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize;
    void *newptr;

    //重新分配为0就是释放当前块
    if(size == 0) {
        mm_free(ptr);
        return 0;
    }

    //如果当前没有这个块，就是重新分配
    if(ptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    if(!newptr) {
        return 0;
    }

    //要复制原来的数据到新的内存块里去
    oldsize = GET_SIZE(HDRP(ptr));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, ptr, oldsize);

    //释放旧的内存块
    mm_free(ptr);

    return newptr;
}

//分割
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));   

    if ((csize - asize) >= DSIZE_T_SIZE) { 
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
    }
    else { 
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

//寻找，使用first-find算法
static void *find_fit(size_t asize)
{
    void *bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) { 
            return bp;
        }
    }
    return NULL; 
}

//扩展堆的大小，同时要初始化新的空间
static void *extend_heap(size_t words) 
{
    char *bp;
    size_t size;
    size = ALIGN(words);   //分配的大小必须也是内存对齐的
    if ((long)(bp = mem_sbrk(size)) == -1)  
        return NULL;                              

    //初始化新增内存块的头部和脚部以及最后的结尾块
    PUT(HDRP(bp), PACK(size, 0));        //初始化头部
    PUT(FTRP(bp), PACK(size, 0));        //初始化脚部
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));  //重新定义结尾块

    //如果bp之前的块也是空闲块的话，就合并
    return coalesce(bp);
}

//合并，以bp指向的内存块为中心
static void *coalesce(void *bp) 
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {            /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) {      /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size,0));
    }

    else if (!prev_alloc && next_alloc) {      /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    else {                                     /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + 
            GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}