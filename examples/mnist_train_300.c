#include "../include/NeuronNetwork.h"
#include "../include/Data.h"
#include "../include/Evaluation.h"
#include "../include/Loss.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Network architecture - smaller for limited data
#define INPUT_SIZE 784      // 28x28 pixels
#define HIDDEN1_SIZE 128    // Smaller first hidden layer
#define HIDDEN2_SIZE 64     // Smaller second hidden layer
#define OUTPUT_SIZE 10      // 10 digits (0-9)

// Training hyperparameters - adjusted for small dataset
#define MAX_TRAIN_SAMPLES 300  // Limit training samples
#define NUM_EPOCHS 100      // More epochs needed for small dataset
#define BATCH_SIZE 32       // Smaller batch size
#define INITIAL_LEARNING_RATE 0.001f
#define MIN_LEARNING_RATE 0.0001f
#define VALIDATION_RATIO 0.2f // Higher validation ratio

// Early stopping parameters
#define PATIENCE 10         // More patience for small dataset
#define MIN_DELTA 0.001f    // Larger minimum delta

float adjust_learning_rate(float current_lr, int epoch){
    // Slower decay for small dataset
    return fmaxf(MIN_LEARNING_RATE, INITIAL_LEARNING_RATE * powf(0.98f, epoch));
}

int main(){
    srand(time(NULL));
    printf("MNIST Neural Network Training - 300 Images\n");
    printf("------------------------------------------\n");

    // Create network architecture
    int layer_sizes[] = {INPUT_SIZE, HIDDEN1_SIZE, HIDDEN2_SIZE, OUTPUT_SIZE};
    int num_layers = sizeof(layer_sizes) / sizeof(layer_sizes[0]);

    // Create network
    NeuralNetwork* network = network_create(layer_sizes, num_layers, INITIAL_LEARNING_RATE);
    if(!network){
        printf("Failed to create neural network\n");
        return 1;
    }

    // Load MNIST dataset
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

    // Limit training data to 300 samples
    if(train_data->num_samples > MAX_TRAIN_SAMPLES){
        printf("Limiting training data from %d to %d samples\n", 
               train_data->num_samples, MAX_TRAIN_SAMPLES);
        train_data->num_samples = MAX_TRAIN_SAMPLES;
    }

    printf("\nDataset Statistics (Limited):\n");
    printf("Training samples: %d\n", train_data->num_samples);
    printf("Validation samples: %d\n", val_data->num_samples);
    printf("Test samples: %d\n", test_data->num_samples);

    // Create confusion matrix
    ConfusionMatrix* cm = confusion_matrix_create(OUTPUT_SIZE);
    if(!cm){
        printf("Failed to create confusion matrix\n");
        network_free(network);
        dataset_free(train_data);
        dataset_free(val_data);
        dataset_free(test_data);
        return 1;
    }

    // Create batch dataset
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

    // Training variables
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

    // Training loop
    for(int epoch = 0; epoch < NUM_EPOCHS; epoch++){
        // Adjust learning rate
        current_lr = adjust_learning_rate(current_lr, epoch);
        network->learning_rate = current_lr;

        // Training phase
        float epoch_loss = 0.0f;
        int num_batches = 0;
        dataset_shuffle(train_data);

        for(int i = 0; i < train_data->num_samples; i += BATCH_SIZE){
            int batch_size = (i + BATCH_SIZE <= train_data->num_samples) ? 
                            BATCH_SIZE : train_data->num_samples - i;
            
            dataset_create_batch(train_data, batch_size, batch, i);
            
            float batch_loss = 0.0f;
            for(int j = 0; j < batch_size; j++){
                network_train(network, batch->inputs[j], batch->targets[j]);
                batch_loss += cross_entropy_loss(network->output_data, batch->targets[j], OUTPUT_SIZE);
            }
            epoch_loss += batch_loss / batch_size;
            num_batches++;
        }
        epoch_loss /= num_batches;

        // Validation phase
        float val_loss = network_validate(network, val_data, cross_entropy_loss);
        float val_accuracy = network_test(network, val_data, cross_entropy_loss, cm);
        
        printf("Epoch %d/%d - Train Loss: %.4f, Val Loss: %.4f, Val Acc: %.2f%%, LR: %.6f\n",
               epoch + 1, NUM_EPOCHS, epoch_loss, val_loss, val_accuracy * 100, current_lr);

        // Early stopping check
        if(val_loss < best_val_loss - MIN_DELTA){
            best_val_loss = val_loss;
            patience_counter = 0;
            
            // Save the best model
            if(model_save(network, "models/mnist_model_300.bin") == 0){
                printf("300-image model saved successfully\n");
            }
        } else{
            patience_counter++;
            if(patience_counter >= PATIENCE){
                printf("\nEarly stopping triggered after %d epochs\n", epoch + 1);
                break;
            }
        }
    }

    // Final testing
    printf("\nTesting the 300-image model...\n");
    float test_loss = network_test(network, test_data, cross_entropy_loss, cm);
    float accuracy = confusion_matrix_accuracy(cm);

    printf("\nFinal Results (300-image model):\n");
    printf("Test loss: %.4f\n", test_loss);
    printf("Test accuracy: %.2f%%\n", accuracy * 100);
    printf("Training time: %ld seconds\n", time(NULL) - start_time);

    // Print confusion matrix
    printf("\nConfusion Matrix:\n");
    confusion_matrix_print(cm);

    // Cleanup
    network_free(network);
    dataset_free(train_data);
    dataset_free(val_data);
    dataset_free(test_data);
    confusion_matrix_free(cm);
    dataset_free(batch);

    printf("\n300-image model training completed!\n");
    return 0;
} 