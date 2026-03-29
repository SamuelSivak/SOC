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

// Architektúra siete - stredná veľkosť pre stredný dataset
#define INPUT_SIZE 784      // 28x28 pixelov
#define HIDDEN1_SIZE 200    // Prvá skrytá vrstva (ReLU)
#define HIDDEN2_SIZE 100    // Druhá skrytá vrstva (Sigmoid)
#define OUTPUT_SIZE 10      // 10 číslic (0-9) so Softmax

// Hyperparametre tréningu - upravené pre stredný dataset
#define MAX_TRAIN_SAMPLES 1500  // Limit trénovacích vzoriek
#define NUM_EPOCHS 80       // Stredný počet epoch
#define BATCH_SIZE 48       // Stredná veľkosť batch
#define INITIAL_LEARNING_RATE 0.001f
#define MIN_LEARNING_RATE 0.0001f
#define VALIDATION_RATIO 0.15f // Stredný pomer validácie

// Výber optimalizátora: 1 pre Adam, 0 pre SGD
#define USE_ADAM 1

// Parametre early stopping
#define PATIENCE 8          // Stredná trpezlivosť
#define MIN_DELTA 0.0005f   // Stredná minimálna delta

float adjust_learning_rate(float current_lr, int epoch){
    // Stredný pokles pre stredný dataset
    return fmaxf(MIN_LEARNING_RATE, INITIAL_LEARNING_RATE * powf(0.96f, epoch));
}

int main(){
    srand(time(NULL));
    printf("MNIST Neural Network Training - 1500 Images\n");
    printf("-------------------------------------------\n");
    printf("Activations: ReLU -> Sigmoid -> Softmax\n");
    printf("Loss: Cross Entropy\n");
    printf("Optimizer: %s\n\n", USE_ADAM ? "Adam" : "SGD");

    // Vytvorenie architektúry siete
    int layer_sizes[] = {INPUT_SIZE, HIDDEN1_SIZE, HIDDEN2_SIZE, OUTPUT_SIZE};
    int num_layers = sizeof(layer_sizes) / sizeof(layer_sizes[0]);

    // Manuálne vytvorenie siete s vlastnými aktiváciami
    NeuralNetwork* network = (NeuralNetwork*)malloc(sizeof(NeuralNetwork));
    if(!network){
        printf("Failed to allocate network\n");
        return 1;
    }

    network->num_layers = num_layers;
    network->learning_rate = INITIAL_LEARNING_RATE;
    network->layer_sizes = (int*)malloc(num_layers * sizeof(int));
    network->layers = (Layer**)malloc((num_layers - 1) * sizeof(Layer*));
    memcpy(network->layer_sizes, layer_sizes, num_layers * sizeof(int));

    // Vytvorenie vrstiev so špecifickými aktivačnými funkciami
    network->layers[0] = layer_create(layer_sizes[1], layer_sizes[0], ACTIVATION_RELU);     // Skrytá 1: ReLU
    network->layers[1] = layer_create(layer_sizes[2], layer_sizes[1], ACTIVATION_SIGMOID);  // Skrytá 2: Sigmoid
    network->layers[2] = layer_create(layer_sizes[3], layer_sizes[2], ACTIVATION_SOFTMAX);  // Výstup: Softmax

    network->input_data = (float*)malloc(layer_sizes[0] * sizeof(float));
    network->output_data = (float*)malloc(layer_sizes[num_layers - 1] * sizeof(float));

    // Inicializácia váh
    for(int i = 0; i < num_layers - 1; i++){
        layer_randomize(network->layers[i], -0.05f, 0.05f);
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

    if(result != 0){
        printf("Failed to load MNIST dataset\n");
        network_free(network);
        return 1;
    }

    // Obmedzenie trénovacích dát na 1500 vzoriek
    if(train_data->num_samples > MAX_TRAIN_SAMPLES){
        printf("Limiting training data from %d to %d samples\n", 
               train_data->num_samples, MAX_TRAIN_SAMPLES);
        train_data->num_samples = MAX_TRAIN_SAMPLES;
    }

    printf("\nDataset Statistics (Limited):\n");
    printf("Training samples: %d\n", train_data->num_samples);
    printf("Validation samples: %d\n", val_data->num_samples);
    printf("Test samples: %d\n", test_data->num_samples);

    // Vytvorenie confusion matrix
    ConfusionMatrix* cm = confusion_matrix_create(OUTPUT_SIZE);
    if(!cm){
        printf("Failed to create confusion matrix\n");
        network_free(network);
        dataset_free(train_data);
        dataset_free(val_data);
        dataset_free(test_data);
        return 1;
    }

    // Vytvorenie batch datasetu
    Dataset* batch = dataset_create(BATCH_SIZE, INPUT_SIZE, OUTPUT_SIZE);
    if(!batch){
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
    printf("Max training samples: %d\n", MAX_TRAIN_SAMPLES);
    printf("Epochs: %d\n", NUM_EPOCHS);
    printf("Batch size: %d\n", BATCH_SIZE);
    printf("Initial learning rate: %f\n", INITIAL_LEARNING_RATE);
    printf("Network architecture: %d -> %d -> %d -> %d\n", 
           INPUT_SIZE, HIDDEN1_SIZE, HIDDEN2_SIZE, OUTPUT_SIZE);

    printf("\nTraining started...\n");
    time_t start_time = time(NULL);

    // Trénovacia slučka
    for(int epoch = 0; epoch < NUM_EPOCHS; epoch++){
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

        for(int i = 0; i < train_data->num_samples; i += BATCH_SIZE){
            int batch_size = (i + BATCH_SIZE <= train_data->num_samples) ? 
                            BATCH_SIZE : train_data->num_samples - i;
            
            dataset_create_batch(train_data, batch_size, batch, i);
            
            float batch_loss = 0.0f;
            for(int j = 0; j < batch_size; j++){
                // Dopredný priechod
                network_forward(network, batch->inputs[j]);
                
                // Výpočet straty
                batch_loss += cross_entropy_loss(network->output_data, batch->targets[j], OUTPUT_SIZE);
                
                // Spätný priechod - výpočet gradientov
                for(int l = network->num_layers - 2; l >= 0; l--){
                    float* layer_input = (l == 0) ? network->input_data : network->layers[l-1]->outputs;
                    float* layer_target = (l == network->num_layers - 2) ? batch->targets[j] : network->layers[l+1]->deltas;
                    Layer* next_layer = (l == network->num_layers - 2) ? NULL : network->layers[l+1];
                    Layer* layer = network->layers[l];
                    
                    // Výpočet delt
                    if(layer->activation_type == ACTIVATION_SOFTMAX){
                        for(int n = 0; n < layer->num_neurons; n++){
                            float output = fmaxf(fminf(layer->outputs[n], 1.0f - 1e-7f), 1e-7f);
                            layer->deltas[n] = output - layer_target[n];
                            layer->neurons[n]->delta = layer->deltas[n];
                        }
                    }else if(next_layer != NULL){
                        for(int n = 0; n < layer->num_neurons; n++){
                            float sum = 0.0f;
                            for(int k = 0; k < next_layer->num_neurons; k++){
                                sum += next_layer->neurons[k]->weights[n] * next_layer->neurons[k]->delta;
                            }
                            if(layer->activation_type == ACTIVATION_RELU){
                                layer->deltas[n] = sum * relu_derivative(layer->neurons[n]->sum);
                            } else if(layer->activation_type == ACTIVATION_SIGMOID){
                                layer->deltas[n] = sum * sigmoid_derivative(layer->neurons[n]->sum);
                            }
                            layer->neurons[n]->delta = layer->deltas[n];
                        }
                    }
                    
                    // Výpočet gradientov
                    for(int n = 0; n < layer->num_neurons; n++){
                        float delta = layer->neurons[n]->delta;
                        for(int k = 0; k < layer->num_inputs; k++){
                            layer->neurons[n]->gradients[k] = delta * layer_input[k];
                        }
                        layer->neurons[n]->bias_gradient = delta;
                    }
                }
                
                // Aplikácia optimalizátora
                for(int l = 0; l < network->num_layers - 1; l++){
                    Layer* layer = network->layers[l];
                    for(int n = 0; n < layer->num_neurons; n++){
                        Neuron* neuron = layer->neurons[n];
                        if(USE_ADAM){
                            adam_update(optimizers[l], neuron->weights, neuron->gradients, neuron->num_inputs);
                        } else {
                            sgd_update(optimizers[l], neuron->weights, neuron->gradients, neuron->num_inputs);
                        }
                        neuron->bias -= current_lr * neuron->bias_gradient;
                    }
                }
            }
            epoch_loss += batch_loss / batch_size;
            num_batches++;

            // Aktualizácia progresu
            if(i % 1000 == 0){
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
        if(val_loss < best_val_loss - MIN_DELTA){
            best_val_loss = val_loss;
            patience_counter = 0;
            
            // Uloženie najlepšieho modelu
            printf("Saving best model...\n");
            if(model_save(network, "models/mnist_model_1500.bin") == 0){
                printf("Best model saved successfully\n");
            }
        } else{
            patience_counter++;
            if(patience_counter >= PATIENCE){
                printf("\nEarly stopping triggered after %d epochs\n", epoch + 1);
                break;
            }
        }
    }

    // Načítanie najlepšieho modelu pre finálne testovanie
    NeuralNetwork* best_network = network_load("models/mnist_model_1500.bin");
    if(!best_network){
        printf("Failed to load best model for testing\n");
        return 1;
    }

    // Finálne testovanie
    printf("\nTesting the best model...\n");
    float test_loss = network_test(best_network, test_data, cross_entropy_loss, cm);
    float accuracy = confusion_matrix_accuracy(cm);

    printf("\nFinal Results:\n");
    printf("Test loss: %.4f\n", test_loss);
    printf("Test accuracy: %.2f%%\n", accuracy * 100);
    printf("Optimizer: %s\n", USE_ADAM ? "Adam" : "SGD");
    printf("Activations: ReLU -> Sigmoid -> Softmax\n");
    printf("Loss: Cross Entropy\n");

    // Výpis confusion matrix
    printf("\nConfusion Matrix:\n");
    confusion_matrix_print(cm);

    // Upratanie
    for(int i = 0; i < num_layers - 1; i++){
        optimizer_free(optimizers[i]);
    }
    free(optimizers);
    
    network_free(network);
    network_free(best_network);
    dataset_free(train_data);
    dataset_free(val_data);
    dataset_free(test_data);
    dataset_free(batch);
    confusion_matrix_free(cm);

    printf("\nTraining completed in %lld seconds\n", (long long)(time(NULL) - start_time));
    return 0;
} 