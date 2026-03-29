#include "../include/Loss.h"
#include <math.h>
#include <stdio.h>

const float LOSS_EPSILON = 1e-10f;

float cross_entropy_loss(float* predictions, float* targets, int size){
    float sum = 0.0f;
    for(int i = 0; i < size; i++){
        if(targets[i] > 0){
            sum -= targets[i] * logf(predictions[i] + LOSS_EPSILON);
        }
    }
    return sum;
}

float cross_entropy_derivative(float* predictions, float* targets, int index){
    return -targets[index] / (predictions[index] + LOSS_EPSILON);
}
