#include "../include/Matrix.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/*
 * Implementácia maticových operácií pre neurónovú sieť.
 * Poskytuje základné operácie ako násobenie, sčítanie,
 * transpozícia a aplikácia funkcií na matice.
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

// Pomocné funkcie pre kontrolu rozmerov
static bool matrix_multiply_dims(Matrix* a, Matrix* b){
    /*
     * Kontroluje, či je možné matice vynásobiť
     * Počet stĺpcov prvej matice musí byť rovný počtu riadkov druhej
     */
    return a->cols == b->rows;
}

static bool matrix_add_dims(Matrix* a, Matrix* b){
    /*
     * Kontroluje, či je možné matice sčítať
     * Matice musia mať rovnaké rozmery
     */
    return (a->rows == b->rows) && (a->cols == b->cols);
}

Matrix* matrix_multiply(Matrix* a, Matrix* b){
    /*
     * Násobí dve matice (A * B)
     * Výsledná matica má rozmery: (a->rows × b->cols)
     */
    if(!matrix_multiply_dims(a, b)){
        return NULL;
    }
    
    Matrix* result = matrix_create(a->rows, b->cols);
    if(!result) return NULL;
    for(int i = 0; i < a->rows; i++){
        for(int j = 0; j < b->cols; j++){
            float sum = 0.0f;
            for(int k = 0; k < a->cols; k++){
                sum += a->data[i * a->cols + k] * b->data[k * b->cols + j];
            }
            result->data[i * result->cols + j] = sum;
        }
    }
    
    return result;
}

Matrix* matrix_transpose(Matrix* matrix){
    /*
     * Vytvorí transponovanú maticu
     * Vymení riadky za stĺpce
     */
    Matrix* result = matrix_create(matrix->cols, matrix->rows);
    if(!result) return NULL;
    for(int i = 0; i < matrix->rows; i++){
        for(int j = 0; j < matrix->cols; j++){
            result->data[j * result->cols + i] = matrix->data[i * matrix->cols + j];
        }
    }
    
    return result;
}

Matrix* matrix_add(Matrix* a, Matrix* b){
    /*
     * Sčíta dve matice (A + B)
     * Matice musia mať rovnaké rozmery
     */
    if(!matrix_add_dims(a, b)){
        return NULL;
    }
    
    Matrix* result = matrix_create(a->rows, a->cols);
    if(!result) return NULL;
    for(int i = 0; i < a->rows * a->cols; i++){
        result->data[i] = a->data[i] + b->data[i];
    }
    
    return result;
}

void matrix_scale(Matrix* matrix, float scalar){
    /*
     * Vynásobí všetky prvky matice skalárom
     */
    for(int i = 0; i < matrix->rows * matrix->cols; i++){
        matrix->data[i] *= scalar;
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

void matrix_apply(Matrix* matrix, float (*func)(float)){
    /*
     * Aplikuje zadanú funkciu na každý prvok matice
     * Používa sa napríklad pre aktivačné funkcie
     */
    for(int i = 0; i < matrix->rows * matrix->cols; i++){
        matrix->data[i] = func(matrix->data[i]);
    }
}

void matrix_random(Matrix* matrix, float min, float max){
    /*
     * Inicializuje maticu náhodnými hodnotami z intervalu [min, max]
     * Používa sa pre inicializáciu váh neurónovej siete
     */
    for(int i = 0; i < matrix->rows * matrix->cols; i++){
        matrix->data[i] = min + (float)rand() / RAND_MAX * (max - min);
    }
}

void matrix_zeros(Matrix* matrix){
    /*
     * Inicializuje maticu nulami
     * Efektívne využíva memset pre rýchle nastavenie
     */
    memset(matrix->data, 0, matrix->rows * matrix->cols * sizeof(float));
}

void matrix_ones(Matrix* matrix){
    /*
     * Inicializuje maticu jednotkami
     */
    for(int i = 0; i < matrix->rows * matrix->cols; i++){
        matrix->data[i] = 1.0f;
    }
}

bool matrix_check_dimensions(Matrix* a, Matrix* b){
    /*
     * Všeobecná kontrola rozmerov pre maticové operácie
     * Kontroluje podmienky pre sčítanie aj násobenie
     */
    return (a->rows == b->rows && a->cols == b->cols) || (a->cols == b->rows);
}

void matrix_print(Matrix* matrix){
    /*
     * Vypíše maticu na štandardný výstup
     * Formátuje čísla na 4 desatinné miesta
     */
    printf("Matica %dx%d:\n", matrix->rows, matrix->cols);
    for(int i = 0; i < matrix->rows; i++){
        for(int j = 0; j < matrix->cols; j++){
            printf("%8.4f ", matrix->data[i * matrix->cols + j]);
        }
        printf("\n");
    }
}
