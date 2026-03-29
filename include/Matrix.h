#ifndef MATRIX_H
#define MATRIX_H

/*
 * Súbor: Matrix.h
 * Popis:
 * Tento súbor obsahuje implementáciu maticových operácií potrebných pre
 * neurónovú sieť. Poskytuje základné operácie ako násobenie matíc,
 * transpozícia, sčítanie a odčítanie, ako aj špeciálne operácie
 * potrebné pre dopredné a spätné šírenie v neurónovej sieti.
 */

#include <stdlib.h>
#include <stdbool.h>

// Definícia štruktúry matice
typedef struct {
    float* data;        // 1D pole hodnôt (row-major order)
    int rows;           // Počet riadkov
    int cols;           // Počet stĺpcov
} Matrix;

/*
 * Vytvorí novú maticu s danými rozmermi
 * Parameter rows: Počet riadkov
 * Parameter cols: Počet stĺpcov
 * Funkcia vracia: Smerník na novú maticu alebo NULL pri chybe
 */
Matrix* matrix_create(int rows, int cols);

/*
 * Uvoľní pamäť alokovanú pre maticu
 * Parameter matrix: Smerník na maticu, ktorá sa má uvoľniť
 * Funkcia vracia: void
 */
void matrix_free(Matrix* matrix);

/*
 * Inicializuje maticu náhodnými hodnotami z daného rozsahu
 * Parameter matrix: Matica na inicializáciu
 * Parameter min: Minimálna hodnota
 * Parameter max: Maximálna hodnota
 * Funkcia vracia: void
 */
void matrix_randomize(Matrix* matrix, float min, float max);

/*
 * Vykoná násobenie matíc (A * B)
 * Parameter a: Prvá matica
 * Parameter b: Druhá matica
 * Funkcia vracia: Smerník na výslednú maticu alebo NULL pri chybe
 */
Matrix* matrix_multiply(Matrix* a, Matrix* b);

/*
 * Vykoná násobenie matice so skalárom
 * Parameter matrix: Vstupná matica
 * Parameter scalar: Skalárna hodnota
 * Funkcia vracia: void (výsledok je uložený do vstupnej matice)
 */
void matrix_multiply_scalar(Matrix* matrix, float scalar);

/*
 * Vykoná transpozíciu matice
 * Parameter matrix: Vstupná matica
 * Funkcia vracia: Smerník na transponovanú maticu alebo NULL pri chybe
 */
Matrix* matrix_transpose(Matrix* matrix);

/*
 * Vykoná sčítanie matíc (A + B)
 * Parameter a: Prvá matica
 * Parameter b: Druhá matica
 * Funkcia vracia: Smerník na výslednú maticu alebo NULL pri chybe
 */
Matrix* matrix_add(Matrix* a, Matrix* b);

/*
 * Vykoná odčítanie matíc (A - B)
 * Parameter a: Prvá matica
 * Parameter b: Druhá matica
 * Funkcia vracia: Smerník na výslednú maticu alebo NULL pri chybe
 */
Matrix* matrix_subtract(Matrix* a, Matrix* b);

/*
 * Aplikuje funkciu na každý prvok matice
 * Parameter matrix: Vstupná matica
 * Parameter func: Ukazovateľ na funkciu, ktorá sa má aplikovať
 * Funkcia vracia: void (výsledok je uložený do vstupnej matice)
 */
void matrix_apply_function(Matrix* matrix, float (*func)(float));

/*
 * Vytvorí kópiu matice
 * Parameter matrix: Matica na kopírovanie
 * Funkcia vracia: Smerník na novú maticu (kópiu) alebo NULL pri chybe
 */
Matrix* matrix_copy(Matrix* matrix);

/*
 * Vypíše maticu na štandardný výstup (pre debugovanie)
 * Parameter matrix: Matica na výpis
 * Funkcia vracia: void
 */
void matrix_print(Matrix* matrix);

#endif
