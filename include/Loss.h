#ifndef LOSS_H
#define LOSS_H

/*
 * Súbor: Loss.h
 * Popis:
 * Tento súbor obsahuje implementáciu stratových funkcií pre neurónovú sieť.
 * Implementovaná je Cross Entropy loss funkcia a jej derivácia pre proces
 * spätnej propagácie (backpropagation).
 */

#include <math.h>

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

#endif
 
