/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int a1,a2,a3,a4,a5,a6,a7,a8;
    int i,j,k,h;
    if(N==32){
        for(i=0;i<4;i++){
            for(j=0;j<4;j++){
                for(k=i*8;k<(i+1)*8;k++){
                    h=j*8;
                    // B[h][k]=A[k][h];B[h+1][k]=A[k][h+1];B[h+2][k]=A[k][h+2];B[h+3][k]=A[k][h+3];
                    // B[h+4][k]=A[k][h+4];B[h+5][k]=A[k][h+5];B[h+6][k]=A[k][h+6];B[h+7][k]=A[k][h+7];

                    a1=A[k][h];a2=A[k][h+1];a3=A[k][h+2];a4=A[k][h+3];
                    a5=A[k][h+4];a6=A[k][h+5];a7=A[k][h+6];a8=A[k][h+7];

                    B[h][k]=a1;B[h+1][k]=a2;B[h+2][k]=a3;B[h+3][k]=a4;
                    B[h+4][k]=a5;B[h+5][k]=a6;B[h+6][k]=a7;B[h+7][k]=a8;
                }
            }
        }
    }else if(N==64){
        for(i=0;i<8;i++){
            for(j=0;j<8;j++){
                for(k=i*8;k<(i+1)*8;k++){
                    h=j*8;
                    // B[h][k]=A[k][h];B[h+1][k]=A[k][h+1];B[h+2][k]=A[k][h+2];B[h+3][k]=A[k][h+3];
                    // B[h+4][k]=A[k][h+4];B[h+5][k]=A[k][h+5];B[h+6][k]=A[k][h+6];B[h+7][k]=A[k][h+7];

                    a1=A[k][h];a2=A[k][h+1];a3=A[k][h+2];a4=A[k][h+3];
                    a5=A[k][h+4];a6=A[k][h+5];a7=A[k][h+6];a8=A[k][h+7];

                    B[h][k]=a1;B[h+1][k]=a2;B[h+2][k]=a3;B[h+3][k]=a4;
                    B[h+4][k]=a5;B[h+5][k]=a6;B[h+6][k]=a7;B[h+7][k]=a8;
                }
            }
        }
    }
   
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    // int i, j, tmp;
    int i, j;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            // tmp = A[i][j];
            // B[j][i] = tmp;
            B[j][i]= A[i][j];
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

