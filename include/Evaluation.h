#ifndef EVALUATION_H
#define EVALUATION_H

/*
 * Súbor: Evaluation.h
 * Popis:
 * Tento súbor obsahuje implementáciu vyhodnocovacích metrík
 * a nástrojov pre neurónovú sieť. Zahŕňa implementáciu
 * confusion matrix, ROC krivky, validácie a testovania modelu,
 * ako aj funkcie pre ukladanie a načítanie natrénovaných modelov.
 */

#include "NeuronNetwork.h"
#include "Data.h"

// Definícia štruktúry pre confusion matrix
typedef struct {
    int** matrix;        // Matica predikcií
    int num_classes;     // Počet tried
} ConfusionMatrix;

// Definícia štruktúry pre ROC krivku
typedef struct {
    float* tpr;         // True Positive Rate
    float* fpr;         // False Positive Rate
    float* thresholds;  // Threshold values
    int num_points;     // Počet bodov krivky
} ROCCurve;

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
 * Vytvorí ROC krivku pre binárnu klasifikáciu
 * Parameter predictions: Pole predikovaných pravdepodobností
 * Parameter targets: Pole skutočných hodnôt
 * Parameter size: Počet vzoriek
 * Parameter num_thresholds: Počet prahových hodnôt pre krivku
 * Funkcia vracia: Smerník na novú ROC krivku alebo NULL pri chybe
 */
ROCCurve* roc_curve_create(float* predictions, float* targets, int size, int num_thresholds);

/*
 * Uvoľní pamäť alokovanú pre ROC krivku
 * Parameter roc: Smerník na ROC krivku, ktorá sa má uvoľniť
 * Funkcia vracia: void
 */
void roc_curve_free(ROCCurve* roc);

/*
 * Vypočíta AUC (Area Under Curve) pre ROC krivku
 * Parameter roc: ROC krivka
 * Funkcia vracia: Hodnotu AUC v rozsahu [0,1]
 */
float roc_curve_auc(ROCCurve* roc);

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

/*
 * Vypíše ROC krivku na štandardný výstup
 * Parameter roc: ROC krivka na výpis
 * Funkcia vracia: void
 */
void roc_curve_print(ROCCurve* roc);

#endif 