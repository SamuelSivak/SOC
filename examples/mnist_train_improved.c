#include "../include/NeuronNetwork.h"
#include "../include/Data.h"
#include "../include/Evaluation.h"
#include "../include/Loss.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Network architecture
#define INPUT_SIZE 784      // 28x28 pixels
#define HIDDEN1_SIZE 256    // First hidden layer
#define HIDDEN2_SIZE 128    // Second hidden layer
#define OUTPUT_SIZE 10      // 10 digits (0-9)

// Training hyperparameters
#define NUM_EPOCHS 50       // Increased number of epochs
#define BATCH_SIZE 64       // Increased batch size for better stability
#define INITIAL_LEARNING_RATE 0.001f
#define MIN_LEARNING_RATE 0.0001f
#define VALIDATION_RATIO 0.1f

// Early stopping parameters
#define PATIENCE 5          // Number of epochs to wait for improvement
#define MIN_DELTA 0.0001f   // Minimum change to qualify as an improvement

float adjust_learning_rate(float current_lr, int epoch) {
    // Decay learning rate over time
    return fmaxf(MIN_LEARNING_RATE, INITIAL_LEARNING_RATE * powf(0.95f, epoch));
}

int main() {
    srand(time(NULL));
    printf("Enhanced MNIST Neural Network Training\n");
    printf("------------------------------------\n");

    // Create network architecture with two hidden layers
    int layer_sizes[] = {INPUT_SIZE, HIDDEN1_SIZE, HIDDEN2_SIZE, OUTPUT_SIZE};
    int num_layers = sizeof(layer_sizes) / sizeof(layer_sizes[0]);

    // Create network
    NeuralNetwork* network = network_create(layer_sizes, num_layers, INITIAL_LEARNING_RATE);
    if(!network) {
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

    if(result != 0) {
        printf("Failed to load MNIST dataset\n");
        network_free(network);
        return 1;
    }

    printf("\nDataset Statistics:\n");
    printf("Training samples: %d\n", train_data->num_samples);
    printf("Validation samples: %d\n", val_data->num_samples);
    printf("Test samples: %d\n", test_data->num_samples);

    // Create confusion matrix
    ConfusionMatrix* cm = confusion_matrix_create(OUTPUT_SIZE);
    if(!cm) {
        printf("Failed to create confusion matrix\n");
        network_free(network);
        dataset_free(train_data);
        dataset_free(val_data);
        dataset_free(test_data);
        return 1;
    }

    // Create batch dataset
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

    // Training variables
    float best_val_loss = INFINITY;
    int patience_counter = 0;
    float current_lr = INITIAL_LEARNING_RATE;

    printf("\nTraining Configuration:\n");
    printf("Epochs: %d\n", NUM_EPOCHS);
    printf("Batch size: %d\n", BATCH_SIZE);
    printf("Initial learning rate: %f\n", INITIAL_LEARNING_RATE);
    printf("Network architecture: %d -> %d -> %d -> %d\n", 
           INPUT_SIZE, HIDDEN1_SIZE, HIDDEN2_SIZE, OUTPUT_SIZE);

    printf("\nTraining started...\n");
    time_t start_time = time(NULL);

    // Training loop
    for(int epoch = 0; epoch < NUM_EPOCHS; epoch++) {
        // Adjust learning rate
        current_lr = adjust_learning_rate(current_lr, epoch);
        network->learning_rate = current_lr;

        // Training phase
        float epoch_loss = 0.0f;
        int num_batches = 0;
        dataset_shuffle(train_data);

        for(int i = 0; i < train_data->num_samples; i += BATCH_SIZE) {
            int batch_size = (i + BATCH_SIZE <= train_data->num_samples) ? 
                            BATCH_SIZE : train_data->num_samples - i;
            
            dataset_create_batch(train_data, batch_size, batch, i);
            
            float batch_loss = 0.0f;
            for(int j = 0; j < batch_size; j++) {
                network_train(network, batch->inputs[j], batch->targets[j]);
                batch_loss += cross_entropy_loss(network->output_data, batch->targets[j], OUTPUT_SIZE);
            }
            epoch_loss += batch_loss / batch_size;
            num_batches++;

            // Progress update
            if(i % 1000 == 0) {
                printf("\rEpoch %d/%d: %.1f%% complete, Loss: %.4f", 
                       epoch + 1, NUM_EPOCHS, 
                       100.0f * i / train_data->num_samples,
                       batch_loss / batch_size);
                fflush(stdout);
            }
        }
        epoch_loss /= num_batches;

        // Validation phase
        float val_loss = network_validate(network, val_data, cross_entropy_loss);
        float val_accuracy = network_test(network, val_data, cross_entropy_loss, cm);
        
        printf("\rEpoch %d/%d completed in %ld seconds\n", 
               epoch + 1, NUM_EPOCHS, time(NULL) - start_time);
        printf("Training loss: %.4f, Validation loss: %.4f, Validation accuracy: %.2f%%\n",
               epoch_loss, val_loss, val_accuracy * 100);
        printf("Learning rate: %.6f\n", current_lr);

        // Early stopping check
        if(val_loss < best_val_loss - MIN_DELTA) {
            best_val_loss = val_loss;
            patience_counter = 0;
            
            // Save the best model
            printf("Saving best model...\n");
            if(model_save(network, "models/mnist_model_best.bin") == 0) {
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

    // Load the best model for final testing
    NeuralNetwork* best_network = network_load("models/mnist_model_best.bin");
    if(!best_network) {
        printf("Failed to load best model for testing\n");
        return 1;
    }

    // Final testing
    printf("\nTesting the best model...\n");
    float test_loss = network_test(best_network, test_data, cross_entropy_loss, cm);
    float accuracy = confusion_matrix_accuracy(cm);

    printf("\nFinal Results:\n");
    printf("Test loss: %.4f\n", test_loss);
    printf("Test accuracy: %.2f%%\n", accuracy * 100);

    // Print confusion matrix and metrics
    printf("\nConfusion Matrix:\n");
    confusion_matrix_print(cm);

    // Save the final model
    printf("\nSaving the final model...\n");
    if(model_save(best_network, "models/mnist_model_final.bin") == 0) {
        printf("Final model saved successfully\n");
    } else {
        printf("Failed to save the final model\n");
    }

    // Cleanup
    network_free(network);
    network_free(best_network);
    dataset_free(train_data);
    dataset_free(val_data);
    dataset_free(test_data);
    dataset_free(batch);
    confusion_matrix_free(cm);

    printf("\nTraining completed in %ld seconds\n", time(NULL) - start_time);
    return 0;
} 