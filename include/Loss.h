#ifndef LOSS_H
#define LOSS_H

/*
 * Súbor: Loss.h
 * Popis:
 * Tento súbor obsahuje implementáciu stratových funkcií pre neurónovú sieť.
 * Implementované sú základné stratové funkcie ako MSE (Mean Squared Error),
 * Cross Entropy a Binary Cross Entropy, vrátane ich derivácií pre proces
 * spätnej propagácie (backpropagation).
 */

#include <math.h>

/*
 * Vypočíta Mean Squared Error (MSE) stratovú funkciu
 * MSE = (1/n) * Σ(y_pred - y_true)²
 * Parameter predictions: Pole predikovaných hodnôt
 * Parameter targets: Pole skutočných hodnôt
 * Parameter size: Veľkosť vstupných polí
 * Funkcia vracia: Hodnotu MSE straty
 */
float mse_loss(float* predictions, float* targets, int size);

/*
 * Vypočíta deriváciu MSE stratovej funkcie
 * d(MSE)/dx = 2(y_pred - y_true)/n
 * Parameter predictions: Pole predikovaných hodnôt
 * Parameter targets: Pole skutočných hodnôt
 * Parameter size: Veľkosť vstupných polí
 * Parameter index: Index, pre ktorý počítame deriváciu
 * Funkcia vracia: Hodnotu derivácie MSE
 */
float mse_derivative(float* predictions, float* targets, int size, int index);

/*
 * Vypočíta Cross Entropy stratovú funkciu
 * CE = -Σ(y_true * log(y_pred))
 * Parameter predictions: Pole predikovaných pravdepodobností
 * Parameter targets: Pole skutočných hodnôt (one-hot encoding)
 * Parameter size: Veľkosť vstupných polí
 * Funkcia vracia: Hodnotu Cross Entropy straty
 */
float cross_entropy_loss(float* predictions, float* targets, int size);

/*
 * Vypočíta deriváciu Cross Entropy stratovej funkcie
 * d(CE)/dx = -y_true/y_pred
 * Parameter predictions: Pole predikovaných pravdepodobností
 * Parameter targets: Pole skutočných hodnôt (one-hot encoding)
 * Parameter index: Index, pre ktorý počítame deriváciu
 * Funkcia vracia: Hodnotu derivácie Cross Entropy
 */
float cross_entropy_derivative(float* predictions, float* targets, int index);

/*
 * Vypočíta Binary Cross Entropy stratovú funkciu
 * BCE = -Σ(y_true * log(y_pred) + (1-y_true) * log(1-y_pred))
 * Parameter predictions: Pole predikovaných pravdepodobností
 * Parameter targets: Pole skutočných hodnôt (0 alebo 1)
 * Parameter size: Veľkosť vstupných polí
 * Funkcia vracia: Hodnotu Binary Cross Entropy straty
 */
float binary_cross_entropy_loss(float* predictions, float* targets, int size);

/*
 * Vypočíta deriváciu Binary Cross Entropy stratovej funkcie
 * d(BCE)/dx = -(y_true/y_pred - (1-y_true)/(1-y_pred))
 * Parameter predictions: Pole predikovaných pravdepodobností
 * Parameter targets: Pole skutočných hodnôt (0 alebo 1)
 * Parameter size: Veľkosť vstupných polí
 * Parameter index: Index, pre ktorý počítame deriváciu
 * Funkcia vracia: Hodnotu derivácie Binary Cross Entropy
 */
float binary_cross_entropy_derivative(float* predictions, float* targets, int size, int index);

#endif 
