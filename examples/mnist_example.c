#include "../include/NeuronNetwork.h"
#include "../include/Data.h"
#include "../include/Evaluation.h"
#include "../include/Loss.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INPUT_SIZE 784   // 28x28 pixels
#define HIDDEN_SIZE 128  // Veľkosť skrytej vrstvy
#define OUTPUT_SIZE 10   // 10 číslic (0-9)
#define NUM_EPOCHS 10
#define BATCH_SIZE 32
#define LEARNING_RATE 0.001f
#define VALIDATION_RATIO 0.1f

int main() {
    srand(time(NULL));
    printf("MNIST Neural Network Training\n");
    printf("----------------------------\n");

    // Vytvorenie architektúry siete
    int layer_sizes[] = {INPUT_SIZE, HIDDEN_SIZE, OUTPUT_SIZE};
    int num_layers = sizeof(layer_sizes) / sizeof(layer_sizes[0]);

    // Vytvorenie siete
    NeuralNetwork* network = network_create(layer_sizes, num_layers, LEARNING_RATE);
    if(!network) {
        printf("Failed to create neural network\n");
        return 1;
    }

    // Načítanie MNIST datasetu
    Dataset *train_data = NULL, *val_data = NULL, *test_data = NULL;
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

    // Trénovanie
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

    printf("\nTraining started...\n");
    for(int epoch = 0; epoch < NUM_EPOCHS; epoch++) {
        dataset_shuffle(train_data);

        // Trénovanie po batch-och
        for(int i = 0; i < train_data->num_samples; i += BATCH_SIZE) {
            int batch_size = (i + BATCH_SIZE <= train_data->num_samples) ? 
                            BATCH_SIZE : train_data->num_samples - i;
            
            dataset_create_batch(train_data, batch_size, batch, i);
            
            for(int j = 0; j < batch_size; j++) {
                network_train(network, batch->inputs[j], batch->targets[j]);
            }

            // Výpis progresu každých 100 vzoriek
            if(i % 100 == 0) {
                printf("\rEpoch %d/%d: %.1f%% complete", 
                       epoch + 1, NUM_EPOCHS, 
                       100.0f * i / train_data->num_samples);
                fflush(stdout);
            }
        }

        // Validácia
        float val_loss = network_validate(network, val_data, cross_entropy_loss);
        printf("\rEpoch %d/%d completed. Validation loss: %.4f\n", 
               epoch + 1, NUM_EPOCHS, val_loss);
    }

    // Testovanie
    printf("\nTesting the network...\n");
    float test_loss = network_test(network, test_data, cross_entropy_loss, cm);
    float accuracy = confusion_matrix_accuracy(cm);

    printf("Test loss: %.4f\n", test_loss);
    printf("Test accuracy: %.2f%%\n", accuracy * 100);

    // Výpis confusion matrix a metrík
    confusion_matrix_print(cm);

    // Uloženie modelu
    printf("\nSaving the model...\n");
    if(model_save(network, "models/mnist_model.bin") == 0) {
        printf("Model saved successfully\n");
    } else {
        printf("Failed to save the model\n");
    }

    // Upratanie
    network_free(network);
    dataset_free(train_data);
    dataset_free(val_data);
    dataset_free(test_data);
    dataset_free(batch);
    confusion_matrix_free(cm);

    return 0;
} 