#ifndef LAYER_H
#define LAYER_H

/*
 * Súbor: Layer.h
 * Popis:
 * Tento súbor obsahuje implementáciu vrstvy neurónovej siete.
 * Každá vrstva sa skladá z neurónov, ktoré sú plne prepojené
 * s neurónmi v nasledujúcej vrstve. Vrstva podporuje dopredné
 * šírenie (forward propagation) a spätné šírenie chyby
 * (backpropagation).
 */

#include "Neuron.h"
#include "Activation.h"

// Definícia štruktúry vrstvy
typedef struct {
    Neuron** neurons;     // Pole neurónov vo vrstve
    float* outputs;       // Výstupy neurónov
    float* deltas;        // Chyby pre backpropagation
    int num_neurons;      // Počet neurónov vo vrstve
    int num_inputs;       // Počet vstupov pre každý neurón
    ActivationType activation_type;  // Typ aktivačnej funkcie
} Layer;

/*
 * Vytvorí novú vrstvu s daným počtom neurónov a vstupov
 * Parameter num_neurons: Počet neurónov vo vrstve
 * Parameter num_inputs: Počet vstupov pre každý neurón
 * Parameter activation_type: Typ aktivačnej funkcie pre vrstvu
 * Funkcia vracia: Smerník na novú vrstvu alebo NULL pri chybe
 */
Layer* layer_create(int num_neurons, int num_inputs, ActivationType activation_type);

/*
 * Uvoľní pamäť alokovanú pre vrstvu
 * Parameter layer: Smerník na vrstvu, ktorá sa má uvoľniť
 * Funkcia vracia: void
 */
void layer_free(Layer* layer);

/*
 * Inicializuje váhy všetkých neurónov vo vrstve náhodnými hodnotami
 * Parameter layer: Vrstva na inicializáciu
 * Parameter min: Minimálna hodnota váh
 * Parameter max: Maximálna hodnota váh
 * Funkcia vracia: void
 */
void layer_randomize(Layer* layer, float min, float max);

/*
 * Vykoná dopredné šírenie (forward propagation) pre vrstvu
 * Parameter layer: Vrstva na výpočet
 * Parameter inputs: Vstupné hodnoty
 * Funkcia vracia: void
 */
void layer_forward(Layer* layer, float* inputs);

/*
 * Vykoná spätné šírenie chyby (backpropagation) pre vrstvu
 * Parameter layer: Vrstva na aktualizáciu
 * Parameter inputs: Vstupné hodnoty z forward pass
 * Parameter deltas: Chyby z nasledujúcej vrstvy
 * Parameter next_layer: Nasledujúca vrstva (NULL pre výstupnú vrstvu)
 * Parameter learning_rate: Rýchlosť učenia
 * Funkcia vracia: void
 */
void layer_backward(Layer* layer, float* inputs, float* deltas, Layer* next_layer, float learning_rate);

/*
 * Vytvorí kópiu vrstvy
 * Parameter layer: Vrstva na kopírovanie
 * Funkcia vracia: Smerník na novú vrstvu (kópiu) alebo NULL pri chybe
 */
Layer* layer_copy(Layer* layer);

/*
 * Vypíše informácie o vrstve na štandardný výstup (pre debugovanie)
 * Parameter layer: Vrstva na výpis
 * Funkcia vracia: void
 */
void layer_print(Layer* layer);

#endif 
