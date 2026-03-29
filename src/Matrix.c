#include "../include/Matrix.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/*
 * Implementácia maticových operácií pre neurónovú sieť.
 * Poskytuje základné operácie pre prácu s maticami.
 */

Matrix* matrix_create(int rows, int cols){
    /*
     * Vytvorí novú maticu zadaných rozmerov
     * Alokuje pamäť pre štruktúru a dátové pole
     */
    Matrix* matrix = (Matrix*)malloc(sizeof(Matrix));
    if(!matrix) return NULL;
    matrix->rows = rows;
    matrix->cols = cols;
    matrix->data = (float*)malloc(rows * cols * sizeof(float));
    
    if(!matrix->data){
        free(matrix);
        return NULL;
    }
    
    return matrix;
}

void matrix_free(Matrix* matrix){
    /*
     * Uvoľní pamäť alokovanú pre maticu
     * Kontroluje existenciu matice pred uvoľnením
     */
    if(matrix){
        free(matrix->data);
        free(matrix);
    }
}

Matrix* matrix_copy(Matrix* matrix){
    /*
     * Vytvorí kópiu matice
     * Alokuje novú pamäť a skopíruje všetky dáta
     */
    Matrix* result = matrix_create(matrix->rows, matrix->cols);
    if(!result) return NULL;
    memcpy(result->data, matrix->data, matrix->rows * matrix->cols * sizeof(float));
    return result;
}
