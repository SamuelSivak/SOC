#include "../include/Evaluation.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

ConfusionMatrix* confusion_matrix_create(int num_classes){
    ConfusionMatrix* cm = (ConfusionMatrix*)malloc(sizeof(ConfusionMatrix));
    if(!cm) return NULL;

    cm->num_classes = num_classes;
    cm->matrix = (int**)malloc(num_classes * sizeof(int*));
    if(!cm->matrix){
        free(cm);
        return NULL;
    }

    for(int i = 0; i < num_classes; i++){
        cm->matrix[i] = (int*)calloc(num_classes, sizeof(int));
        if(!cm->matrix[i]){
            confusion_matrix_free(cm);
            return NULL;
        }
    }

    return cm;
}

void confusion_matrix_free(ConfusionMatrix* cm){
    if(cm){
        if(cm->matrix){
            for(int i = 0; i < cm->num_classes; i++){
                free(cm->matrix[i]);
            }
            free(cm->matrix);
        }
        free(cm);
    }
}

void confusion_matrix_update(ConfusionMatrix* cm, float* predictions, float* targets, int size){
    for(int i = 0; i < size; i++){
        int pred_class = 0;
        int true_class = 0;
        float max_pred = predictions[i * cm->num_classes];
        float max_true = targets[i * cm->num_classes];

        for(int j = 1; j < cm->num_classes; j++){
            if(predictions[i * cm->num_classes + j] > max_pred){
                max_pred = predictions[i * cm->num_classes + j];
                pred_class = j;
            }
        }

        for(int j = 1; j < cm->num_classes; j++){
            if(targets[i * cm->num_classes + j] > max_true){
                max_true = targets[i * cm->num_classes + j];
                true_class = j;
            }
        }

        cm->matrix[true_class][pred_class]++;
    }
}

float confusion_matrix_accuracy(ConfusionMatrix* cm){
    int correct = 0;
    int total = 0;

    for(int i = 0; i < cm->num_classes; i++){
        for(int j = 0; j < cm->num_classes; j++){
            if(i == j) correct += cm->matrix[i][j];
            total += cm->matrix[i][j];
        }
    }

    return total > 0 ? (float)correct / total : 0.0f;
}

float network_validate(NeuralNetwork* network, Dataset* val_data, float (*loss_fn)(float*, float*, int)){
    float total_loss = 0.0f;
    float* predictions = (float*)malloc(val_data->target_size * sizeof(float));
    if(!predictions) return -1.0f;

    for(int i = 0; i < val_data->num_samples; i++){
        network_forward(network, val_data->inputs[i]);
        memcpy(predictions, network->output_data, val_data->target_size * sizeof(float));
        total_loss += loss_fn(predictions, val_data->targets[i], val_data->target_size);
    }

    free(predictions);
    return total_loss / val_data->num_samples;
}

float network_test(NeuralNetwork* network, Dataset* test_data, float (*loss_fn)(float*, float*, int), ConfusionMatrix* cm){
    float total_loss = 0.0f;
    float* predictions = (float*)malloc(test_data->target_size * sizeof(float));
    if(!predictions) return -1.0f;

    if(cm){
        for(int i = 0; i < cm->num_classes; i++){
            memset(cm->matrix[i], 0, cm->num_classes * sizeof(int));
        }
    }

    for(int i = 0; i < test_data->num_samples; i++){
        network_forward(network, test_data->inputs[i]);
        memcpy(predictions, network->output_data, test_data->target_size * sizeof(float));
        total_loss += loss_fn(predictions, test_data->targets[i], test_data->target_size);
        
        if(cm) confusion_matrix_update(cm, predictions, test_data->targets[i], 1);
    }

    free(predictions);
    return total_loss / test_data->num_samples;
}

int model_save(NeuralNetwork* network, const char* filename){
    FILE* file = fopen(filename, "wb");
    if(!file) return -1;

    fwrite(&network->num_layers, sizeof(int), 1, file);
    fwrite(network->layer_sizes, sizeof(int), network->num_layers, file);
    fwrite(&network->learning_rate, sizeof(float), 1, file);

    for(int i = 0; i < network->num_layers - 1; i++){
        Layer* layer = network->layers[i];
        for(int j = 0; j < layer->num_neurons; j++){
            fwrite(layer->neurons[j]->weights, sizeof(float), layer->num_inputs, file);
            fwrite(&layer->neurons[j]->bias, sizeof(float), 1, file);
        }
    }

    fclose(file);
    return 0;
}

NeuralNetwork* model_load(const char* filename){
    FILE* file = fopen(filename, "rb");
    if(!file) return NULL;

    int num_layers;
    fread(&num_layers, sizeof(int), 1, file);

    int* layer_sizes = (int*)malloc(num_layers * sizeof(int));
    fread(layer_sizes, sizeof(int), num_layers, file);

    float learning_rate;
    fread(&learning_rate, sizeof(float), 1, file);

    NeuralNetwork* network = network_create(layer_sizes, num_layers, learning_rate);
    if(!network){
        free(layer_sizes);
        fclose(file);
        return NULL;
    }

    for(int i = 0; i < network->num_layers - 1; i++){
        Layer* layer = network->layers[i];
        for(int j = 0; j < layer->num_neurons; j++){
            fread(layer->neurons[j]->weights, sizeof(float), layer->num_inputs, file);
            fread(&layer->neurons[j]->bias, sizeof(float), 1, file);
        }
    }

    free(layer_sizes);
    fclose(file);
    return network;
}

void confusion_matrix_print(ConfusionMatrix* cm){
    printf("\nConfusion Matrix:\n");
    printf("Predicted ->\n");
    printf("Actual    ");
    for(int i = 0; i < cm->num_classes; i++){
        printf("%8d", i);
    }
    printf("\n");

    for(int i = 0; i < cm->num_classes; i++){
        printf("%8d", i);
        for(int j = 0; j < cm->num_classes; j++){
            printf("%8d", cm->matrix[i][j]);
        }
        printf("\n");
    }
}
