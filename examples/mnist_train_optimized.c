#include "../include/NeuronNetwork.h"
#include "../include/Data.h"
#include "../include/Evaluation.h"
#include "../include/Loss.h"
#include "../include/Optimizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

// Architektúra siete so zmiešanými aktiváciami
#define INPUT_SIZE 784      // 28x28 pixelov
#define HIDDEN1_SIZE 256    // Prvá skrytá vrstva (ReLU)
#define HIDDEN2_SIZE 128    // Druhá skrytá vrstva (Sigmoid)
#define OUTPUT_SIZE 10      // 10 číslic (0-9) so Softmax

// Hyperparametre tréningu
#define NUM_EPOCHS 50
#define BATCH_SIZE 64
#define INITIAL_LEARNING_RATE 0.001f
#define MIN_LEARNING_RATE 0.0001f
#define VALIDATION_RATIO 0.1f

// Výber optimalizátora
#define USE_ADAM 1          // 1 pre Adam, 0 pre SGD

// Parametre early stopping
#define PATIENCE 5
#define MIN_DELTA 0.0001f

float adjust_learning_rate(float current_lr, int epoch) {
    return fmaxf(MIN_LEARNING_RATE, INITIAL_LEARNING_RATE * powf(0.95f, epoch));
}

// Vlastné vytvorenie siete so špecifickými aktivačnými funkciami pre každú vrstvu
NeuralNetwork* network_create_custom(int* layer_sizes, ActivationType* activations, int num_layers, float learning_rate){
    NeuralNetwork* network = (NeuralNetwork*)malloc(sizeof(NeuralNetwork));
    if(!network) return NULL;

    network->num_layers = num_layers;
    network->learning_rate = learning_rate;
    network->layer_sizes = (int*)malloc(num_layers * sizeof(int));
    network->layers = (Layer**)malloc((num_layers - 1) * sizeof(Layer*));

    if(!network->layer_sizes || !network->layers){
        network_free(network);
        return NULL;
    }

    memcpy(network->layer_sizes, layer_sizes, num_layers * sizeof(int));

    // Vytvorenie vrstiev so špecifikovanými aktivačnými funkciami
    for(int i = 0; i < num_layers - 1; i++){
        network->layers[i] = layer_create(layer_sizes[i + 1], layer_sizes[i], activations[i]);
        if(!network->layers[i]){
            network_free(network);
            return NULL;
        }
    }

    network->input_data = (float*)malloc(layer_sizes[0] * sizeof(float));
    network->output_data = (float*)malloc(layer_sizes[num_layers - 1] * sizeof(float));

    if(!network->input_data || !network->output_data){
        network_free(network);
        return NULL;
    }

    return network;
}

// Výpočet gradientov bez aktualizácie váh
void network_compute_gradients(NeuralNetwork* network, float* target_output){
    for(int i = network->num_layers - 2; i >= 0; i--){
        float* layer_input = (i == 0) ? network->input_data : network->layers[i-1]->outputs;
        float* layer_target = (i == network->num_layers - 2) ? target_output : network->layers[i+1]->deltas;
        Layer* next_layer = (i == network->num_layers - 2) ? NULL : network->layers[i+1];
        
        Layer* layer = network->layers[i];
        
        // Výpočet delt
        if(layer->activation_type == ACTIVATION_SOFTMAX){
            for(int j = 0; j < layer->num_neurons; j++){
                float output = fmaxf(fminf(layer->outputs[j], 1.0f - 1e-7f), 1e-7f);
                layer->deltas[j] = output - layer_target[j];
                layer->neurons[j]->delta = layer->deltas[j];
            }
        }else if(next_layer != NULL){
            for(int j = 0; j < layer->num_neurons; j++){
                float sum = 0.0f;
                for(int k = 0; k < next_layer->num_neurons; k++){
                    sum += next_layer->neurons[k]->weights[j] * next_layer->neurons[k]->delta;
                }
                if(layer->activation_type == ACTIVATION_RELU){
                    layer->deltas[j] = sum * relu_derivative(layer->neurons[j]->sum);
                } else if(layer->activation_type == ACTIVATION_SIGMOID){
                    layer->deltas[j] = sum * sigmoid_derivative(layer->neurons[j]->sum);
                }
                layer->neurons[j]->delta = layer->deltas[j];
            }
        }
        
        // Výpočet gradientov
        for(int j = 0; j < layer->num_neurons; j++){
            float delta = layer->neurons[j]->delta;
            for(int k = 0; k < layer->num_inputs; k++){
                layer->neurons[j]->gradients[k] = delta * layer_input[k];
            }
            layer->neurons[j]->bias_gradient = delta;
        }
    }
}

// Aplikácia optimalizátora na sieť
void network_apply_optimizer(NeuralNetwork* network, Optimizer** optimizers){
    for(int i = 0; i < network->num_layers - 1; i++){
        Layer* layer = network->layers[i];
        
        for(int j = 0; j < layer->num_neurons; j++){
            Neuron* neuron = layer->neurons[j];
            
            // Aktualizácia váh pomocou optimalizátora
            if(optimizers[i]->type == OPTIMIZER_ADAM){
                adam_update(optimizers[i], neuron->weights, neuron->gradients, neuron->num_inputs);
            } else if(optimizers[i]->type == OPTIMIZER_SGD){
                sgd_update(optimizers[i], neuron->weights, neuron->gradients, neuron->num_inputs);
            }
            
            // Aktualizácia bias
            neuron->bias -= network->learning_rate * neuron->bias_gradient;
        }
    }
}

int main() {
    srand(time(NULL));
    printf("=======================================================\n");
    printf("MarkNET - Optimized Training with Adam/SGD\n");
    printf("=======================================================\n");
    printf("Activation Functions: ReLU, Sigmoid, Softmax\n");
    printf("Loss Function: Cross Entropy\n");
    printf("Optimizer: %s\n", USE_ADAM ? "Adam" : "SGD");
    printf("=======================================================\n\n");

    // Definícia architektúry siete so špecifickými aktiváciami
    int layer_sizes[] = {INPUT_SIZE, HIDDEN1_SIZE, HIDDEN2_SIZE, OUTPUT_SIZE};
    ActivationType activations[] = {
        ACTIVATION_RELU,     // Skrytá vrstva 1: ReLU
        ACTIVATION_SIGMOID,  // Skrytá vrstva 2: Sigmoid
        ACTIVATION_SOFTMAX   // Výstupná vrstva: Softmax
    };
    int num_layers = sizeof(layer_sizes) / sizeof(layer_sizes[0]);

    // Vytvorenie siete s vlastnými aktiváciami
    NeuralNetwork* network = network_create_custom(layer_sizes, activations, num_layers, INITIAL_LEARNING_RATE);
    if(!network) {
        printf("Failed to create neural network\n");
        return 1;
    }

    // Vytvorenie optimalizátorov pre každú vrstvu
    Optimizer** optimizers = (Optimizer**)malloc((num_layers - 1) * sizeof(Optimizer*));
    for(int i = 0; i < num_layers - 1; i++){
        int num_params = layer_sizes[i+1] * layer_sizes[i];
        if(USE_ADAM){
            optimizers[i] = optimizer_create(OPTIMIZER_ADAM, INITIAL_LEARNING_RATE, 0.9f, 0.999f, 1e-8f, num_params);
        } else {
            optimizers[i] = optimizer_create(OPTIMIZER_SGD, INITIAL_LEARNING_RATE, 0.0f, 0.0f, 0.0f, num_params);
        }
        if(!optimizers[i]){
            printf("Failed to create optimizer for layer %d\n", i);
            return 1;
        }
    }

    // Načítanie MNIST datasetu
    Dataset *train_data = NULL, *val_data = NULL, *test_data = NULL;
    printf("Loading MNIST dataset...\n");
    int result = dataset_load_mnist(
        "data/train-images-idx3-ubyte",
        "data/train-labels-idx1-ubyte",
        "data/t10k-images-idx3-ubyte",
        "data/t10k-labels-idx1-ubyte",
        &train_data, &val_data, &test_data,
        VALIDATION_RATIO
    );

    if(result != 0) {
        printf("Failed to load MNIST dataset\n");
        network_free(network);
        return 1;
    }

    printf("\nDataset Statistics:\n");
    printf("Training samples: %d\n", train_data->num_samples);
    printf("Validation samples: %d\n", val_data->num_samples);
    printf("Test samples: %d\n", test_data->num_samples);

    // Vytvorenie confusion matrix
    ConfusionMatrix* cm = confusion_matrix_create(OUTPUT_SIZE);
    if(!cm) {
        printf("Failed to create confusion matrix\n");
        network_free(network);
        dataset_free(train_data);
        dataset_free(val_data);
        dataset_free(test_data);
        return 1;
    }

    // Vytvorenie batch datasetu
    Dataset* batch = dataset_create(BATCH_SIZE, INPUT_SIZE, OUTPUT_SIZE);
    if(!batch) {
        printf("Failed to create batch dataset\n");
        network_free(network);
        dataset_free(train_data);
        dataset_free(val_data);
        dataset_free(test_data);
        confusion_matrix_free(cm);
        return 1;
    }

    // Trénovacie premenné
    float best_val_loss = INFINITY;
    int patience_counter = 0;
    float current_lr = INITIAL_LEARNING_RATE;

    printf("\nTraining Configuration:\n");
    printf("Epochs: %d\n", NUM_EPOCHS);
    printf("Batch size: %d\n", BATCH_SIZE);
    printf("Initial learning rate: %f\n", INITIAL_LEARNING_RATE);
    printf("Network architecture: %d -> %d (ReLU) -> %d (Sigmoid) -> %d (Softmax)\n", 
           INPUT_SIZE, HIDDEN1_SIZE, HIDDEN2_SIZE, OUTPUT_SIZE);

    printf("\nTraining started...\n");
    time_t start_time = time(NULL);

    // Trénovacia slučka
    for(int epoch = 0; epoch < NUM_EPOCHS; epoch++) {
        // Úprava learning rate
        current_lr = adjust_learning_rate(current_lr, epoch);
        network->learning_rate = current_lr;
        
        // Aktualizácia learning rate optimalizátorov
        for(int i = 0; i < num_layers - 1; i++){
            optimizers[i]->learning_rate = current_lr;
        }

        // Trénovacia fáza
        float epoch_loss = 0.0f;
        int num_batches = 0;
        dataset_shuffle(train_data);

        for(int i = 0; i < train_data->num_samples; i += BATCH_SIZE) {
            int batch_size = (i + BATCH_SIZE <= train_data->num_samples) ? 
                            BATCH_SIZE : train_data->num_samples - i;
            
            dataset_create_batch(train_data, batch_size, batch, i);
            
            float batch_loss = 0.0f;
            for(int j = 0; j < batch_size; j++) {
                // Dopredný priechod
                network_forward(network, batch->inputs[j]);
                
                // Výpočet straty
                batch_loss += cross_entropy_loss(network->output_data, batch->targets[j], OUTPUT_SIZE);
                
                // Spätný priechod - výpočet gradientov
                network_compute_gradients(network, batch->targets[j]);
                
                // Aplikácia optimalizátora
                network_apply_optimizer(network, optimizers);
            }
            epoch_loss += batch_loss / batch_size;
            num_batches++;

            // Aktualizácia progresu
            if(i % 1000 == 0) {
                printf("\rEpoch %d/%d: %.1f%% complete, Loss: %.4f", 
                       epoch + 1, NUM_EPOCHS, 
                       100.0f * i / train_data->num_samples,
                       batch_loss / batch_size);
                fflush(stdout);
            }
        }
        epoch_loss /= num_batches;

        // Validačná fáza
        float val_loss = network_validate(network, val_data, cross_entropy_loss);
        network_test(network, val_data, cross_entropy_loss, cm);
        float val_accuracy = confusion_matrix_accuracy(cm);
        
        printf("\rEpoch %d/%d completed in %lld seconds\n", 
               epoch + 1, NUM_EPOCHS, (long long)(time(NULL) - start_time));
        printf("Training loss: %.4f, Validation loss: %.4f, Validation accuracy: %.2f%%\n",
               epoch_loss, val_loss, val_accuracy * 100);
        printf("Learning rate: %.6f, Optimizer: %s\n", current_lr, USE_ADAM ? "Adam" : "SGD");

        // Kontrola early stopping
        if(val_loss < best_val_loss - MIN_DELTA) {
            best_val_loss = val_loss;
            patience_counter = 0;
            
            // Uloženie najlepšieho modelu
            printf("Saving best model...\n");
            if(model_save(network, "models/mnist_model_optimized.bin") == 0) {
                printf("Best model saved successfully\n");
            }
        } else {
            patience_counter++;
            if(patience_counter >= PATIENCE) {
                printf("\nEarly stopping triggered after %d epochs\n", epoch + 1);
                break;
            }
        }
    }

    // Načítanie najlepšieho modelu pre finálne testovanie
    NeuralNetwork* best_network = network_load("models/mnist_model_optimized.bin");
    if(!best_network) {
        printf("Failed to load best model, using current model\n");
        best_network = network;
    }

    // Finálne testovanie
    printf("\nTesting the optimized model...\n");
    float test_loss = network_test(best_network, test_data, cross_entropy_loss, cm);
    float accuracy = confusion_matrix_accuracy(cm);

    printf("\n=======================================================\n");
    printf("Final Results:\n");
    printf("=======================================================\n");
    printf("Test loss: %.4f\n", test_loss);
    printf("Test accuracy: %.2f%%\n", accuracy * 100);
    printf("Optimizer: %s\n", USE_ADAM ? "Adam" : "SGD");
    printf("Activations: ReLU -> Sigmoid -> Softmax\n");
    printf("Loss: Cross Entropy\n");
    printf("=======================================================\n");

    // Výpis confusion matrix
    printf("\nConfusion Matrix:\n");
    confusion_matrix_print(cm);

    // Upratanie
    for(int i = 0; i < num_layers - 1; i++){
        optimizer_free(optimizers[i]);
    }
    free(optimizers);
    
    if(best_network != network){
        network_free(best_network);
    }
    network_free(network);
    dataset_free(train_data);
    dataset_free(val_data);
    dataset_free(test_data);
    dataset_free(batch);
    confusion_matrix_free(cm);

    printf("\nTraining completed in %lld seconds\n", (long long)(time(NULL) - start_time));
    return 0;
}
