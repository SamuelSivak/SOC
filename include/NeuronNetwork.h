#ifndef NEURONNETWORK_H
#define NEURONNETWORK_H

/*
 * Súbor: NeuronNetwork.h
 * Popis:
 * Tento súbor obsahuje implementáciu neurónovej siete.
 * Sieť je implementovaná ako viacvrstvový perceptrón (MLP)
 * s možnosťou konfigurácie počtu vrstiev a neurónov v každej vrstve.
 * Podporuje rôzne aktivačné funkcie, učenie pomocou spätného šírenia
 * chyby (backpropagation) a ukladanie/načítanie natrénovaných modelov.
 */

#include "Layer.h"

// Definícia štruktúry neurónovej siete
typedef struct {
    Layer** layers;       // Pole vrstiev
    int num_layers;       // Počet vrstiev
    int* layer_sizes;     // Veľkosti jednotlivých vrstiev
    float learning_rate;  // Rýchlosť učenia
    float* input_data;    // Vstupné dáta
    float* output_data;   // Výstupné dáta
} NeuralNetwork;

/*
 * Vytvorí novú neurónovú sieť s danou architektúrou
 * Parameter layer_sizes: Pole veľkostí jednotlivých vrstiev
 * Parameter num_layers: Počet vrstiev
 * Parameter learning_rate: Rýchlosť učenia
 * Funkcia vracia: Smerník na novú sieť alebo NULL pri chybe
 */
NeuralNetwork* network_create(int* layer_sizes, int num_layers, float learning_rate);

/*
 * Uvoľní pamäť alokovanú pre sieť
 * Parameter network: Smerník na sieť, ktorá sa má uvoľniť
 * Funkcia vracia: void
 */
void network_free(NeuralNetwork* network);

/*
 * Vykoná dopredné šírenie (forward propagation) pre celú sieť
 * Parameter network: Sieť na výpočet
 * Parameter input: Vstupné hodnoty
 * Funkcia vracia: void
 */
void network_forward(NeuralNetwork* network, float* input);

/*
 * Vykoná spätné šírenie chyby (backpropagation) pre celú sieť
 * Parameter network: Sieť na aktualizáciu
 * Parameter target_output: Očakávané výstupné hodnoty
 * Funkcia vracia: void
 */
void network_backward(NeuralNetwork* network, float* target_output);

/*
 * Vykoná jeden krok učenia siete
 * Parameter network: Sieť na učenie
 * Parameter input: Vstupné dáta
 * Parameter target_output: Očakávané výstupné hodnoty
 * Funkcia vracia: void
 */
void network_train(NeuralNetwork* network, float* input, float* target_output);

/*
 * Vykoná predikciu pomocou siete
 * Parameter network: Sieť na predikciu
 * Parameter input: Vstupné dáta
 * Funkcia vracia: Smerník na predikované hodnoty
 */
float* network_predict(NeuralNetwork* network, float* input);

/*
 * Uloží sieť do súboru
 * Parameter network: Sieť na uloženie
 * Parameter filename: Názov súboru
 * Funkcia vracia: void
 */
void network_save(NeuralNetwork* network, const char* filename);

/*
 * Načíta sieť zo súboru
 * Parameter filename: Názov súboru
 * Funkcia vracia: Smerník na načítanú sieť alebo NULL pri chybe
 */
NeuralNetwork* network_load(const char* filename);

/*
 * Inicializuje váhy celej siete náhodnými hodnotami
 * Parameter network: Sieť na inicializáciu
 * Parameter min: Minimálna hodnota váh
 * Parameter max: Maximálna hodnota váh
 * Funkcia vracia: void
 */
void network_randomize(NeuralNetwork* network, float min, float max);

/*
 * Vypíše informácie o sieti na štandardný výstup (pre debugovanie)
 * Parameter network: Sieť na výpis
 * Funkcia vracia: void
 */
void network_print(NeuralNetwork* network);

#endif
