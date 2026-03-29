#ifndef OPTIMIZER_H
#define OPTIMIZER_H

/*
 * Súbor: Optimizer.h
 * Popis:
 * Tento súbor obsahuje implementáciu optimalizačných algoritmov
 * pre trénovanie neurónovej siete. Implementovaný je algoritmus
 * Adam (Adaptive Moment Estimation), ktorý kombinuje výhody
 * algoritmov RMSprop a Momentum.
 */

#include "NeuronNetwork.h"

// Typy optimalizátorov
typedef enum {
    OPTIMIZER_SGD,
    OPTIMIZER_ADAM,
    OPTIMIZER_RMSPROP
} OptimizerType;

// Definícia štruktúry pre optimalizátor
typedef struct {
    OptimizerType type;    // Typ optimalizátora
    float learning_rate;   // Rýchlosť učenia
    float* m;              // Prvý moment (pohyblivý priemer gradientov)
    float* v;              // Druhý moment (pohyblivý priemer štvorcov gradientov)
    float beta1;           // Hyperparameter pre prvý moment
    float beta2;           // Hyperparameter pre druhý moment
    float epsilon;         // Malá hodnota pre numerickú stabilitu
    int t;                 // Počítadlo krokov
    int num_params;        // Počet parametrov
} Optimizer;

/*
 * Vytvorí nový optimalizátor
 * Parameter type: Typ optimalizátora
 * Parameter learning_rate: Rýchlosť učenia
 * Parameter beta1: Hyperparameter pre prvý moment (typicky 0.9)
 * Parameter beta2: Hyperparameter pre druhý moment (typicky 0.999)
 * Parameter epsilon: Malá hodnota pre numerickú stabilitu (typicky 1e-8)
 * Parameter num_params: Počet parametrov
 * Funkcia vracia: Smerník na nový optimalizátor alebo NULL pri chybe
 */
Optimizer* optimizer_create(OptimizerType type, float learning_rate, float beta1, float beta2, float epsilon, int num_params);

/*
 * Uvoľní pamäť alokovanú pre optimalizátor
 * Parameter optimizer: Smerník na optimalizátor, ktorý sa má uvoľniť
 * Funkcia vracia: void
 */
void optimizer_free(Optimizer* optimizer);

/*
 * Aktualizuje parametre siete pomocou algoritmu Adam
 * Parameter optimizer: Optimalizátor
 * Parameter network: Neurónová sieť na aktualizáciu
 * Parameter learning_rate: Rýchlosť učenia
 * Funkcia vracia: void
 */
void optimizer_update(Optimizer* optimizer, NeuralNetwork* network, float learning_rate);

/*
 * Vypíše informácie o optimalizátore na štandardný výstup (pre debugovanie)
 * Parameter optimizer: Optimalizátor na výpis
 * Funkcia vracia: void
 */
void optimizer_print(Optimizer* optimizer);

// Aktualizuje parametre pomocou SGD
void sgd_update(Optimizer* optimizer, float* parameters, float* gradients, int size);

// Aktualizuje parametre pomocou Adam
void adam_update(Optimizer* optimizer, float* parameters, float* gradients, int size);

// Aktualizuje parametre pomocou RMSprop
void rmsprop_update(Optimizer* optimizer, float* parameters, float* gradients, int size);

// Resetuje stav optimalizátora
void optimizer_reset(Optimizer* optimizer);

#endif
