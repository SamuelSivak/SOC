/**
 * @file Optimizer.c
 * @brief Implementácia optimalizačných algoritmov pre neurónovú sieť
 * 
 * Tento súbor implementuje optimalizačné algoritmy používané pri trénovaní
 * neurónovej siete, vrátane SGD (Stochastic Gradient Descent), Adam a RMSprop.
 */

#include "../include/Optimizer.h"
#include "../include/Neuron.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * Implementácia optimalizačných algoritmov pre neurónovú sieť.
 * Obsahuje implementácie SGD, Adam a RMSprop optimalizátorov
 * pre efektívne učenie neurónovej siete.
 */

// Konštanty pre numerickú stabilitu a predvolené hodnoty
const float DEFAULT_BETA1 = 0.9f;
const float DEFAULT_BETA2 = 0.999f;
const float DEFAULT_EPSILON = 1e-8f;

Optimizer* optimizer_create(OptimizerType type, float learning_rate, float beta1, 
                          float beta2, float epsilon, int num_params) {
    Optimizer* optimizer = (Optimizer*)malloc(sizeof(Optimizer));
    if(!optimizer) return NULL;

    optimizer->type = type;
    optimizer->learning_rate = learning_rate;
    optimizer->beta1 = beta1;
    optimizer->beta2 = beta2;
    optimizer->epsilon = epsilon;
    optimizer->num_params = num_params;
    optimizer->t = 0;

    // Alokácia bufferov pre Adam a RMSprop
    if(type == OPTIMIZER_ADAM || type == OPTIMIZER_RMSPROP) {
        optimizer->m = (float*)calloc(num_params, sizeof(float));
        optimizer->v = (float*)calloc(num_params, sizeof(float));
        if(!optimizer->m || !optimizer->v) {
            optimizer_free(optimizer);
            return NULL;
        }
    } else {
        optimizer->m = NULL;
        optimizer->v = NULL;
    }

    return optimizer;
}

void optimizer_free(Optimizer* optimizer) {
    if(optimizer) {
        free(optimizer->m);
        free(optimizer->v);
        free(optimizer);
    }
}

void sgd_update(Optimizer* optimizer, float* parameters, float* gradients, int size) {
    for(int i = 0; i < size; i++) {
        parameters[i] -= optimizer->learning_rate * gradients[i];
    }
}

void adam_update(Optimizer* optimizer, float* parameters, float* gradients, int size) {
    optimizer->t++;
    float alpha = optimizer->learning_rate * 
                 sqrtf(1.0f - powf(optimizer->beta2, optimizer->t)) /
                 (1.0f - powf(optimizer->beta1, optimizer->t));

    for(int i = 0; i < size; i++) {
        // Aktualizácia momentov
        optimizer->m[i] = optimizer->beta1 * optimizer->m[i] + 
                         (1.0f - optimizer->beta1) * gradients[i];
        optimizer->v[i] = optimizer->beta2 * optimizer->v[i] + 
                         (1.0f - optimizer->beta2) * gradients[i] * gradients[i];

        // Aktualizácia parametrov
        parameters[i] -= alpha * optimizer->m[i] / 
                        (sqrtf(optimizer->v[i]) + optimizer->epsilon);
    }
}

void rmsprop_update(Optimizer* optimizer, float* parameters, float* gradients, int size) {
    for(int i = 0; i < size; i++) {
        // Aktualizácia RMS
        optimizer->v[i] = optimizer->beta1 * optimizer->v[i] + 
                         (1.0f - optimizer->beta1) * gradients[i] * gradients[i];

        // Aktualizácia parametrov
        parameters[i] -= optimizer->learning_rate * gradients[i] / 
                        (sqrtf(optimizer->v[i]) + optimizer->epsilon);
    }
}

void optimizer_reset(Optimizer* optimizer) {
    optimizer->t = 0;
    if(optimizer->m) {
        memset(optimizer->m, 0, optimizer->num_params * sizeof(float));
    }
    if(optimizer->v) {
        memset(optimizer->v, 0, optimizer->num_params * sizeof(float));
    }
}
