/*
 * Implementácia aktivačných funkcií pre neurónovú sieť.
 * 
 * Tento modul poskytuje implementácie rôznych aktivačných funkcií,
 * ktoré sa používajú v neurónovej sieti na transformáciu výstupov
 * neurónov. Každá funkcia má svoju základnú implementáciu a príslušnú
 * deriváciu potrebnú pre proces učenia siete.
 * 
 * Implementované funkcie:
 * - Sigmoid: transformuje vstup na interval (0,1)
 * - ReLU: prepúšťa kladné hodnoty, záporné nastavuje na 0
 * - Tanh: transformuje vstup na interval (-1,1)
 * - Softmax: normalizuje vektor na pravdepodobnostné rozdelenie
 */

#include "../include/Activation.h"
#include <math.h>
#include <float.h>

float sigmoid(float x){
    /* 
     * Sigmoidálna aktivačná funkcia
     * Transformuje vstup na hodnotu v rozsahu (0,1)
     * Vzorec: f(x) = 1 / (1 + e^(-x))
     */
    return 1.0f / (1.0f + expf(-x));
}

float sigmoid_derivative(float x){
    /* 
     * Derivácia sigmoidálnej funkcie
     * Vzorec: f'(x) = f(x) * (1 - f(x))
     */
    float s = sigmoid(x);
    return s * (1.0f - s);
}

float relu(float x){
    /*
     * ReLU (Rectified Linear Unit) aktivačná funkcia
     * Prepúšťa len kladné hodnoty, záporné nastavuje na 0
     * Vzorec: f(x) = max(0, x)
     */
    if(x > 0){
        return x;
    } else {
        return 0.0f;
    }
}

float relu_derivative(float x){
    /*
     * Derivácia ReLU funkcie
     * Vzorec: f'(x) = 1 pre x > 0, 0 pre x <= 0
     */
    if(x > 0){
        return 1.0f;
    } else {
        return 0.0f;
    }
}

float tanh_activation(float x){
    /*
     * Hyperbolický tangens ako aktivačná funkcia
     * Transformuje vstup na hodnotu v rozsahu (-1,1)
     */
    return tanhf(x);
}

float tanh_derivative(float x){
    /*
     * Derivácia hyperbolického tangensu
     * Vzorec: f'(x) = 1 - tanh²(x)
     */
    float t = tanhf(x);
    return 1.0f - t * t;
}

void relu_forward(float* input, float* output, int size) {
    for(int i = 0; i < size; i++) {
        output[i] = fmaxf(0.0f, input[i]);
    }
}

void relu_backward(float* input, float* gradient_in, float* gradient_out, int size) {
    for(int i = 0; i < size; i++) {
        // Derivácia ReLU je 1 pre x > 0, inak 0
        gradient_out[i] = input[i] > 0.0f ? gradient_in[i] : 0.0f;
    }
}

void softmax_forward(float* input, float* output, int size) {
    // Nájdenie maxima pre numerickú stabilitu
    float max_val = input[0];
    for(int i = 1; i < size; i++) {
        if(input[i] > max_val) {
            max_val = input[i];
        }
    }

    // Výpočet exp(x - max) a sumy
    float sum = 0.0f;
    for(int i = 0; i < size; i++) {
        output[i] = expf(input[i] - max_val);
        sum += output[i];
    }

    // Normalizácia
    for(int i = 0; i < size; i++) {
        output[i] /= sum;
        // Zabezpečenie numerickej stability
        output[i] = fmaxf(output[i], FLT_EPSILON);
        output[i] = fminf(output[i], 1.0f - FLT_EPSILON);
    }
}

void softmax_backward(float* output, float* gradient_in, float* gradient_out, int size) {
    // Výpočet Jacobian matice * gradient
    for(int i = 0; i < size; i++) {
        float sum = 0.0f;
        for(int j = 0; j < size; j++) {
            // Derivácia softmax: p_i * (delta_ij - p_j)
            float deriv = output[i] * ((i == j) - output[j]);
            sum += deriv * gradient_in[j];
        }
        gradient_out[i] = sum;
    }
}

float softmax(float* x, int size, int index) {
    float max_val = x[0];
    for(int i = 1; i < size; i++) {
        if(x[i] > max_val) max_val = x[i];
    }

    float sum = 0.0f;
    for(int i = 0; i < size; i++) {
        sum += expf(x[i] - max_val);
    }

    float result = expf(x[index] - max_val) / sum;
    return fmaxf(fminf(result, 1.0f - FLT_EPSILON), FLT_EPSILON);
}

float softmax_derivative(float* x, int size, int i, int j) {
    float si = softmax(x, size, i);
    float sj = softmax(x, size, j);
    return si * ((i == j) - sj);
}

// Pomocné funkcie pre kompatibilitu s rozhraním
float activation_relu(float x){
    return relu(x);
}

float activation_relu_derivative(float x){
    return relu_derivative(x);
}
