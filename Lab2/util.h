#ifndef LAB2_UTIL_H
#define LAB2_UTIL_H

struct Matrix{
    int rows;
    int cols;
    int** matrix;
};

void startTimer();
double stopTimer();
void allocateMatrix(struct Matrix *matrix, int rows, int cols);
void readMatrix(struct Matrix* matrix, FILE* fp);
void printMatrix(struct Matrix matrix, FILE* fp);
void* multiply_per_matrix(void* args);
void* multiply_per_row(void* args);
void* multiply_per_element(void* args);

#endif //LAB2_UTIL_H
