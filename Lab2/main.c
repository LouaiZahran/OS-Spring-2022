#include <stdio.h>
#include <pthread.h>
#include <malloc.h>

struct Matrix{
    int rows;
    int cols;
    int** matrix;
};

struct Matrix matrixA, matrixB, matrixC_per_matrix, matrixC_per_row, matrixC_per_element;
FILE *fpA, *fpB, *fpC;

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
    int colToMulitply = matrixC->cols;

    int maxK = matrixA.cols;

    for(int k=0; k<maxK; k++)
            matrixC->matrix[rowToMultiply][colToMulitply] += matrixA.matrix[rowToMultiply][k] * matrixB.matrix[k][colToMulitply];
    return NULL;
}

void init(char* args[]){
    if(args[1] != NULL) {
        fpA = fopen(args[1], "r");
        fpB = fopen(args[2], "r");
        fpC = fopen(args[3], "w+");
    }else{
        fpA = fopen("a.txt", "r");
        fpB = fopen("b.txt", "r");
        fpC = fopen("c.txt", "w+");
    }

    readMatrix(&matrixA, fpA);
    readMatrix(&matrixB, fpB);

    allocateMatrix(&matrixC_per_matrix, matrixA.rows, matrixB.cols);
    allocateMatrix(&matrixC_per_row, matrixA.rows, matrixB.cols);
    allocateMatrix(&matrixC_per_element, matrixA.rows, matrixB.cols);

}

int main(int argC, char* args[]) {
    init(args);

    int rows = matrixA.rows, cols = matrixB.cols;
    pthread_t matrixThread, rowThreads[rows], elementThreads[rows][cols];

    pthread_create(&matrixThread, NULL, &multiply_per_matrix, &matrixC_per_matrix);

    struct Matrix tmpMatrix[rows];
    for(int i=0; i<rows; i++) {
        tmpMatrix[i] = matrixC_per_row;
        tmpMatrix[i].rows = i;
        pthread_create(&rowThreads[i], NULL, &multiply_per_row, &tmpMatrix[i]);
    }

    struct Matrix tmpMatrix2[rows][cols];
    for(int i=0; i<rows; i++) {
        for (int j = 0; j < cols; j++) {
            tmpMatrix2[i][j] = matrixC_per_element;
            tmpMatrix2[i][j].rows = i;
            tmpMatrix2[i][j].cols = j;
            pthread_create(&elementThreads[i][j], NULL, &multiply_per_element, &tmpMatrix2[i][j]);
        }
    }

    pthread_join(matrixThread, NULL);
    for(int i=0; i<rows; i++)
        pthread_join(rowThreads[i], NULL);
    for(int i=0; i<rows; i++)
        for(int j=0; j<cols; j++)
            pthread_join(elementThreads[i][j], NULL);

    printMatrix(matrixC_per_matrix, fopen("c_per_matrix.txt", "w+"));
    printMatrix(matrixC_per_row, fopen("c_per_row.txt", "w+"));
    printMatrix(matrixC_per_element, fopen("c_per_element.txt", "w+"));

    return 0;
}
