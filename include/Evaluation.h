#ifndef EVALUATION_H
#define EVALUATION_H

/*
 * Súbor: Evaluation.h
 * Popis:
 * Tento súbor obsahuje implementáciu vyhodnocovacích metrík
 * a nástrojov pre neurónovú sieť. Zahŕňa implementáciu
 * confusion matrix, validácie a testovania modelu,
 * ako aj funkcie pre ukladanie a načítanie natrénovaných modelov.
 */

#include "NeuronNetwork.h"
#include "Data.h"

// Definícia štruktúry pre confusion matrix
typedef struct {
    int** matrix;        // Matica predikcií
    int num_classes;     // Počet tried
} ConfusionMatrix;

/*
 * Vytvorí novú confusion matrix
 * Parameter num_classes: Počet tried
 * Funkcia vracia: Smerník na novú confusion matrix alebo NULL pri chybe
 */
ConfusionMatrix* confusion_matrix_create(int num_classes);

/*
 * Uvoľní pamäť alokovanú pre confusion matrix
 * Parameter cm: Smerník na confusion matrix, ktorá sa má uvoľniť
 * Funkcia vracia: void
 */
void confusion_matrix_free(ConfusionMatrix* cm);

/*
 * Aktualizuje confusion matrix novými predikciami
 * Parameter cm: Confusion matrix na aktualizáciu
 * Parameter predictions: Pole predikovaných hodnôt
 * Parameter targets: Pole skutočných hodnôt
 * Parameter size: Počet vzoriek
 * Funkcia vracia: void
 */
void confusion_matrix_update(ConfusionMatrix* cm, float* predictions, float* targets, int size);

/*
 * Vypočíta presnosť (accuracy) z confusion matrix
 * Parameter cm: Confusion matrix
 * Funkcia vracia: Hodnotu presnosti v rozsahu [0,1]
 */
float confusion_matrix_accuracy(ConfusionMatrix* cm);

/*
 * Vykoná validáciu siete na validačných dátach
 * Parameter network: Sieť na validáciu
 * Parameter val_data: Validačné dáta
 * Parameter loss_fn: Stratová funkcia
 * Funkcia vracia: Priemernú stratu na validačných dátach
 */
float network_validate(NeuralNetwork* network, Dataset* val_data, float (*loss_fn)(float*, float*, int));

/*
 * Vykoná testovanie siete na testovacích dátach
 * Parameter network: Sieť na testovanie
 * Parameter test_data: Testovacie dáta
 * Parameter loss_fn: Stratová funkcia
 * Parameter cm: Confusion matrix pre uloženie výsledkov (môže byť NULL)
 * Funkcia vracia: Priemernú stratu na testovacích dátach
 */
float network_test(NeuralNetwork* network, Dataset* test_data, float (*loss_fn)(float*, float*, int), ConfusionMatrix* cm);

/*
 * Uloží model do súboru
 * Parameter network: Sieť na uloženie
 * Parameter filename: Cesta k súboru
 * Funkcia vracia: 0 pri úspechu, -1 pri chybe
 */
int model_save(NeuralNetwork* network, const char* filename);

/*
 * Načíta model zo súboru
 * Parameter filename: Cesta k súboru
 * Funkcia vracia: Smerník na načítanú sieť alebo NULL pri chybe
 */
NeuralNetwork* model_load(const char* filename);

/*
 * Vypíše confusion matrix na štandardný výstup
 * Parameter cm: Confusion matrix na výpis
 * Funkcia vracia: void
 */
void confusion_matrix_print(ConfusionMatrix* cm);

#endif
 