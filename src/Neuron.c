#include "../include/Neuron.h"
#include "../include/Activation.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/*
 * Implementácia neurónu pre neurónovú sieť.
 * Každý neurón obsahuje váhy, bias, gradienty pre učenie
 * a implementuje dopredné aj spätné šírenie signálu.
 */

// Konštanta pre numerickú stabilitu
const float NEURON_EPSILON = 1e-10f;

// Deklarácie aktivačných funkcií
float activation_relu(float x);
float activation_relu_derivative(float x);

Neuron* neuron_create(int num_inputs, ActivationType activation_type){
    /*
     * Vytvorí nový neurón so zadaným počtom vstupov a aktivačnou funkciou
     * Inicializuje všetky potrebné polia a hodnoty
     */
    Neuron* neuron = (Neuron*)malloc(sizeof(Neuron));
    if(!neuron) return NULL;

    neuron->num_inputs = num_inputs;
    neuron->weights = (float*)malloc(num_inputs * sizeof(float));
    neuron->gradients = (float*)malloc(num_inputs * sizeof(float));
    
    if(!neuron->weights || !neuron->gradients){
        free(neuron->weights);
        free(neuron->gradients);
        free(neuron);
        return NULL;
    }

    // Inicializácia základných hodnôt
    neuron->delta = 0.0f;
    neuron->output = 0.0f;
    neuron->bias = 0.0f;
    neuron->bias_gradient = 0.0f;
    neuron->activation_type = activation_type;

    // Inicializácia váh pomocou Xavier/Glorot inicializácie
    neuron_randomize(neuron, -0.05f, 0.05f);

    return neuron;
}

void neuron_free(Neuron* neuron){
    /*
     * Uvoľní pamäť alokovanú pre neurón
     * Kontroluje existenciu neurónu pred uvoľnením
     */
    if(neuron){
        free(neuron->weights);
        free(neuron->gradients);
        free(neuron);
    }
}

void neuron_randomize(Neuron* neuron, float min, float max){
    /*
     * Inicializuje váhy neurónu pomocou Xavier/Glorot inicializácie
     * Táto metóda pomáha predchádzať problému miznúcich/explodujúcich gradientov
     * Vzorec: limit = sqrt(6/(n_in + n_out)), kde n_out = 1 pre jeden neurón
     * 
     * Parametre min a max sa používajú pre inicializáciu biasu
     */
    float limit = sqrtf(6.0f / (neuron->num_inputs + 1));
    
    // Inicializácia váh v rozsahu [-limit, limit]
    for(int i = 0; i < neuron->num_inputs; i++){
        neuron->weights[i] = -limit + (float)rand() / RAND_MAX * (2 * limit);
        neuron->gradients[i] = 0.0f;
    }
    
    // Inicializácia biasu v zadanom rozsahu [min, max]
    neuron->bias = min + (float)rand() / RAND_MAX * (max - min);
    neuron->bias_gradient = 0.0f;
}

float neuron_forward(Neuron* neuron, float* inputs){
    /*
     * Dopredné šírenie signálu cez neurón
     * 1. Vypočíta váženú sumu vstupov
     * 2. Pridá bias
     * 3. Aplikuje aktivačnú funkciu (ak nie je softmax)
     */
    float sum = neuron->bias;
    
    // Výpočet váženej sumy
    for(int i = 0; i < neuron->num_inputs; i++){
        sum += inputs[i] * neuron->weights[i];
    }
    
    // Uloženie sumy pre spätné šírenie
    neuron->sum = sum;
    
    // Aplikácia ReLU aktivácie (softmax sa aplikuje na úrovni vrstvy)
    if(neuron->activation_type == ACTIVATION_RELU){
        sum = relu(sum);
    }
    
    neuron->output = sum;
    return sum;
}

void neuron_backward(Neuron* neuron, float* inputs, float learning_rate){
    /*
     * Spätné šírenie chyby cez neurón
     * 1. Vypočíta gradient podľa typu aktivácie
     * 2. Aktualizuje váhy a gradienty
     * 3. Aktualizuje bias
     */
    float gradient = 0.0f;
    
    // Výpočet gradientu podľa typu aktivácie
    if(neuron->activation_type == ACTIVATION_RELU){
        gradient = neuron->delta * relu_derivative(neuron->sum);
    } else if(neuron->activation_type == ACTIVATION_SOFTMAX){
        // Pre softmax je delta už správne vypočítaná vo vrstve
        gradient = neuron->delta;
    }
    
    // Aktualizácia váh a ich gradientov
    for(int i = 0; i < neuron->num_inputs; i++){
        neuron->gradients[i] = gradient * inputs[i];
        neuron->weights[i] -= learning_rate * neuron->gradients[i];
    }
    
    // Aktualizácia biasu
    neuron->bias -= learning_rate * gradient;
}

void neuron_update_weights(Neuron* neuron, float learning_rate){
    /*
     * Aktualizuje váhy neurónu pomocou vypočítaných gradientov
     * Používa sa pri pokročilých optimalizačných metódach
     */
    for(int i = 0; i < neuron->num_inputs; i++){
        neuron->weights[i] -= learning_rate * neuron->gradients[i];
    }
    neuron->bias -= learning_rate * neuron->bias_gradient;
}

Neuron* neuron_copy(Neuron* neuron){
    /*
     * Vytvorí hlbokú kópiu neurónu
     * Kopíruje všetky váhy, gradienty a ostatné hodnoty
     */
    Neuron* copy = neuron_create(neuron->num_inputs, neuron->activation_type);
    if(!copy) return NULL;

    memcpy(copy->weights, neuron->weights, neuron->num_inputs * sizeof(float));
    memcpy(copy->gradients, neuron->gradients, neuron->num_inputs * sizeof(float));
    copy->bias = neuron->bias;
    copy->bias_gradient = neuron->bias_gradient;
    copy->output = neuron->output;
    copy->delta = neuron->delta;

    return copy;
}

void neuron_print(Neuron* neuron){
    /*
     * Vypíše informácie o neuróne na štandardný výstup
     */
    printf("Neurón: bias=%.4f, výstup=%.4f\n", neuron->bias, neuron->output);
}
