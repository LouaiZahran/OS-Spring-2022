#include <stdio.h>
#include <time.h>
#include <malloc.h>
#include "util.h"

extern struct Matrix matrixA, matrixB, matrixC_per_matrix, matrixC_per_row, matrixC_per_element;
extern FILE *fpA, *fpB, *fpC_per_matrix, *fpC_per_row, *fpC_per_element;
extern clock_t t;

void startTimer(){
    t = clock();
}

double stopTimer(){
    return (double)(clock() - t)/CLOCKS_PER_SEC * 1000;
}

void allocateMatrix(struct Matrix *matrix, int rows, int cols){
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->matrix = malloc(rows * sizeof(int*));
    for(int i=0; i<rows; i++)
        matrix->matrix[i] = malloc(cols * sizeof(int));
}

void readMatrix(struct Matrix* matrix, FILE* fp){
    int rows, cols;
    fscanf(fp, "row=%d col=%d", &rows, &cols);

    allocateMatrix(matrix, rows, cols);

    for(int i=0; i<matrix->rows; i++)
        for(int j=0; j<matrix->cols; j++)
            fscanf(fp, "%d", &(matrix->matrix[i][j]));
}

void printMatrix(struct Matrix matrix, FILE* fp){
    fprintf(fp, "row=%d col=%d\n", matrix.rows, matrix.cols);
    for(int i=0; i<matrix.rows; i++, fprintf(fp, "\n"))
        for(int j=0; j<matrix.cols; j++)
            fprintf(fp, "%d ", matrix.matrix[i][j]);
    fflush(fp);
    fclose(fp);
}

void* multiply_per_matrix(void* args){
    struct Matrix *matrixC = (struct Matrix*)args;
    int rows = matrixA.rows, cols = matrixB.cols, maxK = matrixA.cols;
    allocateMatrix(matrixC, rows, cols);

    for(int k=0; k<maxK; k++)
        for(int i=0; i<rows; i++)
            for(int j=0; j<cols; j++)
                matrixC->matrix[i][j] += matrixA.matrix[i][k] * matrixB.matrix[k][j];
    return NULL;
}

void* multiply_per_row(void* args){
    struct Matrix *matrixC = (struct Matrix*)args;
    int rowToMultiply = matrixC->rows;

    int cols = matrixB.cols, maxK = matrixA.cols;

    for(int k=0; k<maxK; k++)
        for(int j=0; j<cols; j++)
            matrixC->matrix[rowToMultiply][j] += matrixA.matrix[rowToMultiply][k] * matrixB.matrix[k][j];
    return NULL;
}

void* multiply_per_element(void* args){
    struct Matrix *matrixC = (struct Matrix*)args;
    int rowToMultiply = matrixC->rows;
    int colToMultiply = matrixC->cols;

    int maxK = matrixA.cols;

    for(int k=0; k<maxK; k++)
        matrixC->matrix[rowToMultiply][colToMultiply] += matrixA.matrix[rowToMultiply][k] * matrixB.matrix[k][colToMultiply];
    return NULL;
}