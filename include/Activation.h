#ifndef ACTIVATION_H
#define ACTIVATION_H

/*
 * Súbor: Activation.h
 * Popis:
 * Tento súbor obsahuje implementáciu aktivačných funkcií pre neurónovú sieť.
 * Aktivačné funkcie transformujú výstup neurónu na požadovaný rozsah hodnôt
 * a pridávajú nelinearitu do siete. Implementované sú základné aktivačné
 * funkcie ako ReLU (Rectified Linear Unit), Sigmoid, Tanh a Softmax, vrátane
 * ich derivácií potrebných pre proces učenia siete.
 */

#include <math.h>

/*
 * Sigmoid aktivačná funkcia
 * Parameter x: Vstupná hodnota
 * Funkcia vracia: Hodnotu v rozsahu (0,1)
 */
float sigmoid(float x);

/*
 * Derivácia sigmoid aktivačnej funkcie
 * Parameter x: Vstupná hodnota
 * Funkcia vracia: Hodnotu derivácie
 */
float sigmoid_derivative(float x);

/*
 * Aplikuje ReLU (Rectified Linear Unit) aktivačnú funkciu
 * f(x) = max(0, x)
 * Parameter x: Vstupná hodnota
 * Funkcia vracia: Aktivovanú hodnotu
 */
float relu(float x);

/*
 * Vypočíta deriváciu ReLU aktivačnej funkcie
 * f'(x) = 1 pre x > 0, 0 inak
 * Parameter x: Vstupná hodnota
 * Funkcia vracia: Hodnotu derivácie
 */
float relu_derivative(float x);

/*
 * Tanh aktivačná funkcia
 * Parameter x: Vstupná hodnota
 * Funkcia vracia: Hodnotu v rozsahu (-1,1)
 */
float tanh_activation(float x);

/*
 * Derivácia tanh aktivačnej funkcie
 * Parameter x: Vstupná hodnota
 * Funkcia vracia: Hodnotu derivácie
 */
float tanh_derivative(float x);

/*
 * Aplikuje Softmax aktivačnú funkciu na vektor hodnôt
 * f(x_i) = exp(x_i) / sum(exp(x_j)) pre všetky j
 * Parameter values: Pole vstupných hodnôt
 * Parameter size: Veľkosť vstupného poľa
 * Funkcia vracia: void (výsledok je uložený späť do vstupného poľa)
 */
float softmax(float* x, int size, int index);

/*
 * Vypočíta deriváciu Softmax aktivačnej funkcie
 * Parameter values: Pole vstupných hodnôt (už po aplikácii Softmax)
 * Parameter size: Veľkosť vstupného poľa
 * Parameter i: Index výstupu
 * Parameter j: Index vstupu
 * Funkcia vracia: Hodnota derivácie
 */
float softmax_derivative(float* x, int size, int i, int j);

/*
 * Aplikuje ReLU na celý vektor hodnôt
 * Parameter input: Vstupný vektor
 * Parameter output: Výstupný vektor (môže byť totožný so vstupom)
 * Parameter size: Veľkosť vektora
 * Funkcia vracia: void
 */
void relu_forward(float* input, float* output, int size);

/*
 * Vypočíta gradient ReLU pre celý vektor
 * Parameter input: Pôvodný vstup do ReLU
 * Parameter gradient_in: Gradient z vyššej vrstvy
 * Parameter gradient_out: Výstupný gradient
 * Parameter size: Veľkosť vektora
 * Funkcia vracia: void
 */
void relu_backward(float* input, float* gradient_in, float* gradient_out, int size);

/*
 * Aplikuje softmax na celý vektor hodnôt
 * Parameter input: Vstupný vektor
 * Parameter output: Výstupný vektor (môže byť totožný so vstupom)
 * Parameter size: Veľkosť vektora
 * Funkcia vracia: void
 */
void softmax_forward(float* input, float* output, int size);

/*
 * Vypočíta gradient softmax pre celý vektor
 * Parameter output: Výstup zo softmax funkcie
 * Parameter gradient_in: Gradient z vyššej vrstvy
 * Parameter gradient_out: Výstupný gradient
 * Parameter size: Veľkosť vektora
 * Funkcia vracia: void
 */
void softmax_backward(float* output, float* gradient_in, float* gradient_out, int size);

#endif
