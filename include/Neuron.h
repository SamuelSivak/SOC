#ifndef NEURON_H
#define NEURON_H

/*
 * Súbor: Neuron.h
 * Popis:
 * Tento súbor obsahuje implementáciu neurónu pre neurónovú sieť.
 * Neurón je základnou výpočtovou jednotkou siete, ktorá obsahuje
 * váhy, bias a aktivačnú funkciu. Podporuje dopredné šírenie signálu
 * a spätné šírenie chyby pre proces učenia.
 */

#include "Matrix.h"

// Typ aktivačnej funkcie
typedef enum {
    ACTIVATION_RELU,    // Rectified Linear Unit
    ACTIVATION_SOFTMAX  // Softmax pre výstupnú vrstvu
} ActivationType;

// Definícia štruktúry neurónu
typedef struct {
    float* weights;        // Váhy vstupov
    float bias;           // Prah (bias)
    float output;         // Výstupná hodnota
    float delta;          // Chyba pre spätné šírenie
    float* gradients;     // Gradienty váh pre učenie
    float bias_gradient;  // Gradient prahu
    int num_inputs;       // Počet vstupov
    float sum;           // Suma pred aktiváciou
    ActivationType activation_type;  // Typ aktivačnej funkcie
} Neuron;

/*
 * Vytvorí nový neurón s daným počtom vstupov
 * Parameter num_inputs: Počet vstupov neurónu
 * Parameter activation_type: Typ aktivačnej funkcie
 * Funkcia vracia: Smerník na nový neurón alebo NULL pri chybe
 */
Neuron* neuron_create(int num_inputs, ActivationType activation_type);

/*
 * Uvoľní pamäť alokovanú pre neurón
 * Parameter neuron: Smerník na neurón, ktorý sa má uvoľniť
 * Funkcia vracia: void
 */
void neuron_free(Neuron* neuron);

/*
 * Inicializuje váhy neurónu náhodnými hodnotami z daného rozsahu
 * Parameter neuron: Neurón na inicializáciu
 * Parameter min: Minimálna hodnota váh
 * Parameter max: Maximálna hodnota váh
 * Funkcia vracia: void
 */
void neuron_randomize(Neuron* neuron, float min, float max);

/*
 * Vykoná forward propagation pre neurón
 * Parameter neuron: Neurón na výpočet
 * Parameter inputs: Pole vstupných hodnôt
 * Funkcia vracia: Výstupná hodnota neurónu
 */
float neuron_forward(Neuron* neuron, float* inputs);

/*
 * Vykoná backward propagation pre neurón
 * Parameter neuron: Neurón na aktualizáciu
 * Parameter inputs: Pole vstupných hodnôt
 * Parameter learning_rate: Rýchlosť učenia
 * Funkcia vracia: void
 */
void neuron_backward(Neuron* neuron, float* inputs, float learning_rate);

/*
 * Aktualizuje váhy neurónu na základe chyby
 * Parameter neuron: Neurón na aktualizáciu
 * Parameter learning_rate: Rýchlosť učenia
 * Funkcia vracia: void
 */
void neuron_update_weights(Neuron* neuron, float learning_rate);

/*
 * Vytvorí kópiu neurónu
 * Parameter neuron: Neurón na kopírovanie
 * Funkcia vracia: Smerník na kópiu neurónu alebo NULL pri chybe
 */
Neuron* neuron_copy(Neuron* neuron);

/*
 * Vypíše informácie o neuróne na štandardný výstup (pre debugovanie)
 * Parameter neuron: Neurón na výpis
 * Funkcia vracia: void
 */
void neuron_print(Neuron* neuron);

#endif
