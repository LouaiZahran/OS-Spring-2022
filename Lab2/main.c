#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <time.h>
#include <string.h>

struct Matrix{
    int rows;
    int cols;
    int** matrix;
};

struct Matrix matrixA, matrixB, matrixC_per_matrix, matrixC_per_row, matrixC_per_element;
FILE *fpA, *fpB, *fpC_per_matrix, *fpC_per_row, *fpC_per_element;
clock_t t;

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

void init(char* args[]){
    if(args[1] != NULL) {
        char* fileName = malloc(100);
        strcpy(fileName, args[1]);
        fpA = fopen(strcat(fileName, ".txt"), "r");

        strcpy(fileName, args[2]);
        fpB = fopen(strcat(fileName, ".txt"), "r");

        strcpy(fileName, args[3]);
        fpC_per_matrix = fopen(strcat(fileName, "_per_matrix.txt"), "w+");

        strcpy(fileName, args[3]);
        fpC_per_row = fopen(strcat(fileName, "_per_row.txt"), "w+");

        strcpy(fileName, args[3]);
        fpC_per_element = fopen(strcat(fileName, "_per_element.txt"), "w+");
    }else{
        fpA = fopen("a.txt", "r");
        fpB = fopen("b.txt", "r");
        fpC_per_matrix = fopen("c_per_matrix.txt", "w+");
        fpC_per_matrix = fopen("c_per_matrix.txt", "w+");
        fpC_per_matrix = fopen("c_per_matrix.txt", "w+");
    }

    fprintf(fpC_per_matrix, "Method: A thread per matrix\n");
    fprintf(fpC_per_row, "Method: A thread per row\n");
    fprintf(fpC_per_element, "Method: A thread per element\n");

    readMatrix(&matrixA, fpA);
    readMatrix(&matrixB, fpB);

    allocateMatrix(&matrixC_per_matrix, matrixA.rows, matrixB.cols);
    allocateMatrix(&matrixC_per_row, matrixA.rows, matrixB.cols);
    allocateMatrix(&matrixC_per_element, matrixA.rows, matrixB.cols);

}

void startTimer(){
    t = clock();
}

double stopTimer(){
    return (double)(clock() - t)/CLOCKS_PER_SEC * 1000;
}

void task1(){
    double timeTaken;
    printf("Task 1:\n=======\n");

    {
        startTimer();
        pthread_t matrixThread;
        pthread_create(&matrixThread, NULL, &multiply_per_matrix, &matrixC_per_matrix);
        pthread_join(matrixThread, NULL);
        timeTaken = stopTimer();
    }

    printf("Number of threads: %d\nTime taken: %lf ms\n\n", 1, timeTaken);
    printMatrix(matrixC_per_matrix, fpC_per_matrix);

}

void task2(){
    printf("Task 2:\n=======\n");
    double timeTaken;
    int rows = matrixA.rows;
    struct Matrix tmpMatrix[rows];

    {
        startTimer();

        pthread_t rowThreads[rows];
        for (int i = 0; i < rows; i++) {
            tmpMatrix[i] = matrixC_per_row;
            tmpMatrix[i].rows = i;
            pthread_create(&rowThreads[i], NULL, &multiply_per_row, &tmpMatrix[i]);
        }

        for(int i=0; i<rows; i++)
            pthread_join(rowThreads[i], NULL);

        timeTaken = stopTimer();
    }

    printf("Number of threads: %d\nTime taken: %lf ms\n\n", rows, timeTaken);
    printMatrix(matrixC_per_row, fpC_per_row);
}

void task3(){
    printf("Task 3:\n=======\n");
    double timeTaken;
    int rows = matrixA.rows, cols = matrixB.cols;
    struct Matrix tmpMatrix[rows][cols];

    {
        startTimer();
        pthread_t elementThreads[rows][cols];
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                tmpMatrix[i][j] = matrixC_per_element;
                tmpMatrix[i][j].rows = i;
                tmpMatrix[i][j].cols = j;
                pthread_create(&elementThreads[i][j], NULL, &multiply_per_element, &tmpMatrix[i][j]);
            }
        }

        for(int i=0; i<rows; i++)
            for(int j=0; j<cols; j++)
                pthread_join(elementThreads[i][j], NULL);

        timeTaken = stopTimer();
    }

    printf("Number of threads: %d\nTime taken: %lf ms\n\n", rows * cols, timeTaken);
    printMatrix(matrixC_per_element, fpC_per_element);
}

int main(int argC, char* args[]) {
    init(args);

    task1();
    task2();
    task3();

    return 0;
}
