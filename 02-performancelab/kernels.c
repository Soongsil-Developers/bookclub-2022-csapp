/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * Please fill in the following team struct 
 */
team_t team = {
    "SoongsilDev",              /* Team name */

    "주재현",     /* First member full name */
    "jake010417@gmail.com",  /* First member email address */

    "",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
};

int sum1;
/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/



/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j, k, l;
    
    int dim1 = dim - 1;
    int bsize = 16;
    //dim * dim 회  dim/16 * dim/16 * 16 * 16 
    //블록행력곱 이용 pixel 16 bit
    for(i = 0; i< dim; i += bsize)
        for (j = 0; j < dim; j += bsize)
            for (k = i; k < i+bsize; k++)
                for (l = j; l < j+bsize; l++)
                    dst[RIDX(dim1-l, k, dim)] = src[RIDX(k, l, dim)];

            
    //naive_rotate(dim, src, dst);
    }

/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    add_rotate_function(&naive_rotate, naive_rotate_descr);   
    add_rotate_function(&rotate, rotate_descr);   
    /* ... Register additional test functions here */
}


/***************
 * SMOOTH KERNEL
 **************/

/***************************************************************
 * Various typedefs and helper functions for the smooth function
 * You may modify these any way you like.
 **************************************************************/

/* A struct used to compute averaged pixel value */
typedef struct {
    int red;
    int green;
    int blue;
    int num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int min(int a, int b) { return (a < b ? a : b); }
static int max(int a, int b) { return (a > b ? a : b); }

/* 
 * initialize_pixel_sum - Initializes all fields of sum to 0 
 */
static void initialize_pixel_sum(pixel_sum *sum) 
{
    sum->red = sum->green = sum->blue = 0;
    sum->num = 0;
    return;
}

/* 
 * accumulate_sum - Accumulates field values of p in corresponding 
 * fields of sum 
 */
static void accumulate_sum(pixel_sum *sum, pixel p) 
{
    sum->red += (int) p.red;
    sum->green += (int) p.green;
    sum->blue += (int) p.blue;
    sum->num++;
    return;
}

/* 
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel 
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) 
{
    current_pixel->red = (unsigned short) (sum.red/sum.num);
    current_pixel->green = (unsigned short) (sum.green/sum.num);
    current_pixel->blue = (unsigned short) (sum.blue/sum.num);
    return;
}

/* 
 * avg - Returns averaged pixel value at (i,j) 
 */
static pixel avg(int dim, int i, int j, pixel *src) 
{
    int ii, jj;
    pixel_sum sum;
    pixel current_pixel;

    initialize_pixel_sum(&sum);
    for(ii = max(i-1, 0); ii <= min(i+1, dim-1); ii++)  0 1
	for(jj = max(j-1, 0); jj <= min(j+1, dim-1); jj++)  0 1
	    accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

    assign_sum_to_pixel(&current_pixel, sum);

    
    return current_pixel;
}

/******************************************************
 * Your different versions of the smooth kernel go here
 ******************************************************/

/*
 * naive_smooth - The naive baseline version of smooth 
 */
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(i, j, dim)] = avg(dim, i, j, src);

}

/*
 * smooth - Your current working version of smooth. 
 * IMPORTANT: This is the version you will be graded on
 */

char smooth_descr[] = "smooth: Current working version";
void smooth(int dim, pixel *src, pixel *dst) 
{
    int i, j, tmp;
    /*
        smooth는 해당 위치 포함 주변 9개 값의 평균 모서리는 포함 4개 변은 포함 6개
        naive_smooth는 for문 안에 avg 호출, 후 avg 안에서 min max,accumulate_sum 호출로 호출을 줄인다면 성능 최적화 가능
                                                  분기 예측 
    */
    //0 ~ dim - 1 #define RIDX(i,j,n) ((i)*(n)+(j))
    //각 모서리 4개씩 평균 (0,0) , (0,dim - 1), (dim - 1, 0), (dim - 1, dim - 1) / 각 변 6개씩 평균 -> for문 / 나머지 9개 평균 -> for문
    //왼쪽 위
    tmp = RIDX(0, 0, dim); //tmp = 0
    dst[tmp].red = (src[tmp].red + src[tmp + 1].red + src[tmp + dim].red + src[tmp + dim + 1].red) / 4;
    dst[tmp].green = (src[tmp].green + src[tmp + 1].green + src[tmp + dim].green + src[tmp + dim + 1].green) / 4;
    dst[tmp].blue = (src[tmp].blue + src[tmp + 1].blue + src[tmp + dim].blue + src[tmp + dim + 1].blue) / 4;
    //왼쪽 아래
    tmp = RIDX(dim -1, 0, dim);//tmp = (dim - 1) * dim
    dst[tmp].red = (src[tmp].red + src[tmp + 1].red + src[tmp - dim].red + src[tmp - dim + 1].red) / 4;
    dst[tmp].green = (src[tmp].green + src[tmp + 1].green + src[tmp - dim].green + src[tmp - dim + 1].green) / 4;    
    dst[tmp].blue = (src[tmp].blue + src[tmp + 1].blue + src[tmp - dim].blue + src[tmp - dim + 1].blue) / 4;
    //오른쪽 위
    tmp = RIDX(0, dim - 1, dim);//tmp = dim - 1
    dst[tmp].red = (src[tmp].red + src[tmp - 1].red + src[tmp + dim].red + src[tmp + dim - 1].red) / 4;
    dst[tmp].green = (src[tmp].green + src[tmp - 1].green + src[tmp + dim].green + src[tmp + dim - 1].green) / 4;
    dst[tmp].blue = (src[tmp].blue + src[tmp - 1].blue + src[tmp + dim].blue + src[tmp + dim - 1].blue) / 4;
    //오른쪽 아래
    tmp = RIDX(dim - 1, dim - 1, dim);//tmp = (dim - 1) * dim + dim - 1
    dst[tmp].red = (src[tmp].red + src[tmp - 1].red + src[tmp - dim].red + src[tmp - dim - 1].red) / 4;
    dst[tmp].green = (src[tmp].green + src[tmp - 1].green + src[tmp - dim].green + src[tmp - dim - 1].green) / 4;
    dst[tmp].blue = (src[tmp].blue + src[tmp - 1].blue + src[tmp - dim].blue + src[tmp - dim - 1].blue) / 4;

    //각 변  윗변 i = 0 j 1 ~ dim - 2 , 아랫변 i = dim - 1 j 1 ~ dim - 2, 좌변 i 1 ~ dim - 2 j = 0, 우변 i 1 ~ dim - 2 j = dim - 1
    for(i  = 1; i < dim - 1; i++){
        //좌변
        tmp = RIDX(i, 0, dim);
        dst[tmp].red = (src[tmp].red + src[tmp - dim].red + src[tmp - dim + 1].red + src[tmp + 1].red + src[tmp + dim].red + src[tmp + dim + 1].red) / 6;
        dst[tmp].green = (src[tmp].green + src[tmp - dim].green + src[tmp - dim + 1].green + src[tmp + 1].green + src[tmp + dim].green + src[tmp + dim + 1].green) / 6;
        dst[tmp].blue = (src[tmp].blue + src[tmp - dim].blue + src[tmp - dim + 1].blue + src[tmp + 1].blue + src[tmp + dim].blue + src[tmp + dim + 1].blue) / 6;
        //우변
        tmp = RIDX(i, dim - 1, dim);
        dst[tmp].red = (src[tmp].red + src[tmp - dim].red + src[tmp - dim - 1].red + src[tmp - 1].red + src[tmp + dim].red + src[tmp + dim - 1].red) / 6;
        dst[tmp].green = (src[tmp].green + src[tmp - dim].green + src[tmp - dim - 1].green + src[tmp - 1].green + src[tmp + dim].green + src[tmp + dim - 1].green) / 6;
        dst[tmp].blue = (src[tmp].blue + src[tmp - dim].blue + src[tmp - dim - 1].blue + src[tmp - 1].blue + src[tmp + dim].blue + src[tmp + dim - 1].blue) / 6;
        //윗변
        tmp = RIDX(0, i, dim);
        dst[tmp].red = (src[tmp].red + src[tmp + dim].red + src[tmp + dim - 1].red + src[tmp - 1].red + src[tmp + 1].red + src[tmp + dim + 1].red) / 6;
        dst[tmp].green = (src[tmp].green + src[tmp + dim].green + src[tmp + dim - 1].green + src[tmp - 1].green + src[tmp + 1].green + src[tmp + dim + 1].green) / 6;
        dst[tmp].blue = (src[tmp].blue + src[tmp + dim].blue + src[tmp + dim - 1].blue + src[tmp - 1].blue + src[tmp + 1].blue + src[tmp + dim + 1].blue) / 6;
        //아랫변
        tmp = RIDX(dim - 1, i, dim);
        dst[tmp].red = (src[tmp].red + src[tmp - dim].red + src[tmp - dim - 1].red + src[tmp - 1].red + src[tmp + 1].red + src[tmp - dim + 1].red) / 6;
        dst[tmp].green = (src[tmp].green + src[tmp - dim].green + src[tmp - dim - 1].green + src[tmp - 1].green + src[tmp + 1].green + src[tmp - dim + 1].green) / 6;
        dst[tmp].blue = (src[tmp].blue + src[tmp - dim].blue + src[tmp - dim - 1].blue + src[tmp - 1].blue + src[tmp + 1].blue + src[tmp - dim + 1].blue) / 6;
    }
    
    
    //나머지
    for (i = 1; i < dim - 1; i++)
	    for (j = 1; j < dim - 1; j++){
            tmp = RIDX(i,j,dim);
            dst[tmp].red = (src[tmp].red + src[tmp - dim].red + src[tmp + dim].red + src[tmp + 1].red + src[tmp - 1].red + src[tmp + dim + 1].red + src[tmp + dim - 1].red + src[tmp - dim + 1].red + src[tmp - dim - 1].red) / 9;
            dst[tmp].green = (src[tmp].green + src[tmp - dim].green + src[tmp + dim].green + src[tmp + 1].green + src[tmp - 1].green + src[tmp + dim + 1].green + src[tmp + dim - 1].green + src[tmp - dim + 1].green + src[tmp - dim - 1].green) / 9;
            dst[tmp].blue = (src[tmp].blue + src[tmp - dim].blue + src[tmp + dim].blue + src[tmp + 1].blue + src[tmp - 1].blue + src[tmp + dim + 1].blue + src[tmp + dim - 1].blue + src[tmp - dim + 1].blue + src[tmp - dim - 1].blue) / 9;
        }


    
    //naive_smooth(dim, src, dst);
}


/********************************************************************* 
 * register_smooth_functions - Register all of your different versions
 *     of the smooth kernel with the driver by calling the
 *     add_smooth_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_smooth_functions() {
    add_smooth_function(&smooth, smooth_descr);
    add_smooth_function(&naive_smooth, naive_smooth_descr);
    /* ... Register additional test functions here */
}

