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

void relu_forward(float* input, float* output, int size) {
    for(int i = 0; i < size; i++) {
        output[i] = fmaxf(0.0f, input[i]);
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
