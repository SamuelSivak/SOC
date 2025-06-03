#include "../include/Loss.h"
#include <math.h>
#include <stdio.h>

/*
 * Implementácia stratových funkcií pre neurónovú sieť.
 * Každá funkcia má svoju základnú implementáciu a jej deriváciu
 * pre proces spätného šírenia chyby (backpropagation).
 */

// Globálna konštanta pre numerickú stabilitu
const float LOSS_EPSILON = 1e-10f;

float mse_loss(float* predictions, float* targets, int size){
    /*
     * Mean Squared Error (MSE) stratová funkcia
     * Počíta priemernú kvadratickú odchýlku medzi predikciami a skutočnými hodnotami
     * Vzorec: MSE = (1/n) * Σ(y_pred - y_true)²
     */
    float sum = 0.0f;
    
    for(int i = 0; i < size; i++){
        float diff = predictions[i] - targets[i];
        sum += diff * diff;
    }
    
    return sum / size;
}

float mse_derivative(float* predictions, float* targets, int size, int index){
    /*
     * Derivácia MSE stratovej funkcie
     * Vzorec: d(MSE)/dx = 2(y_pred - y_true)/n
     */
    return 2.0f * (predictions[index] - targets[index]) / size;
}

float cross_entropy_loss(float* predictions, float* targets, int size){
    /*
     * Cross Entropy stratová funkcia pre viactriednu klasifikáciu
     * Vzorec: CE = -Σ(y_true * log(y_pred))
     * Pridaná malá konštanta (LOSS_EPSILON) pre numerickú stabilitu
     */
    float sum = 0.0f;
    for(int i = 0; i < size; i++){
        if(targets[i] > 0){
            // Kontrola validity predikcií
            if(predictions[i] <= 0.0f || isnan(predictions[i])){
                printf("[UPOZORNENIE] Neplatná predikcia: predictions[%d]=%f, targets[%d]=%f\n", 
                       i, predictions[i], i, targets[i]);
            }
            sum -= targets[i] * logf(predictions[i] + LOSS_EPSILON);
        }
    }
    
    return sum;
}

float cross_entropy_derivative(float* predictions, float* targets, int index){
    /*
     * Derivácia Cross Entropy stratovej funkcie
     * Vzorec: d(CE)/dx = -y_true/y_pred
     */
    return -targets[index] / (predictions[index] + LOSS_EPSILON);
}

float binary_cross_entropy_loss(float* predictions, float* targets, int size){
    /*
     * Binary Cross Entropy stratová funkcia pre binárnu klasifikáciu
     * Vzorec: BCE = -(1/n) * Σ(y_true * log(y_pred) + (1-y_true) * log(1-y_pred))
     */
    float sum = 0.0f;
    
    for(int i = 0; i < size; i++){
        sum -= targets[i] * logf(predictions[i] + LOSS_EPSILON) + 
               (1.0f - targets[i]) * logf(1.0f - predictions[i] + LOSS_EPSILON);
    }
    
    return sum / size;
}

float binary_cross_entropy_derivative(float* predictions, float* targets, int size, int index){
    /*
     * Derivácia Binary Cross Entropy stratovej funkcie
     * Vzorec: d(BCE)/dx = -(y_true/y_pred - (1-y_true)/(1-y_pred))/n
     */
    float prediction = predictions[index];
    float target = targets[index];
    
    return -(target / (prediction + LOSS_EPSILON) - 
             (1.0f - target) / (1.0f - prediction + LOSS_EPSILON)) / size;
}
