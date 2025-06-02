#include "NeuronNetwork.h"
#include "Data.h"
#include "Evaluation.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Program na testovanie načítaného MNIST modelu.
 * Načíta uložený model a otestuje ho na testovacej množine.
 */

// Konštanty pre architektúru siete
static const int INPUT_SIZE = 784;    // 28x28 pixelov
static const int OUTPUT_SIZE = 10;    // 10 číslic (0-9)

// Deklarácie pomocných funkcií
float calculate_accuracy(float* predictions, float* targets, int size);
void print_confusion_matrix(int* matrix, int size);
void calculate_metrics(int* matrix, int size, float* precision, float* recall, float* f1_score);
void print_metrics(float* precision, float* recall, float* f1_score, int size);

int main(){
    printf("Test načítaného MNIST modelu\n");
    printf("----------------------------\n");

    // Načítanie modelu
    printf("Načítavam model zo súboru 'models/mnist_model.bin'...\n");
    NeuralNetwork* network = network_load("models/mnist_model.bin");
    if(!network){
        printf("Chyba: Nepodarilo sa načítať model\n");
        return 1;
    }
    printf("Model úspešne načítaný\n");

    // Načítanie testovacích dát
    Dataset *train_data = NULL, *val_data = NULL, *test_data = NULL;
    int result = dataset_load_mnist(
        "data/train-images-idx3-ubyte",
        "data/train-labels-idx1-ubyte",
        "data/t10k-images-idx3-ubyte",
        "data/t10k-labels-idx1-ubyte",
        &train_data, &val_data, &test_data,
        0.1f  // Validačný pomer nie je potrebný pre testovanie
    );

    if(result != 0){
        printf("Chyba: Nepodarilo sa načítať MNIST dataset\n");
        network_free(network);
        return 1;
    }

    // Kontrola veľkosti vstupných dát
    if(test_data->input_size != INPUT_SIZE){
        printf("Chyba: Nesprávna veľkosť vstupných dát (očakávané: %d, skutočné: %d)\n",
               INPUT_SIZE, test_data->input_size);
        network_free(network);
        dataset_free(train_data);
        dataset_free(val_data);
        dataset_free(test_data);
        return 1;
    }

    printf("\nTestujem model na testovacej množine...\n");
    printf("Počet testovacích vzoriek: %d\n", test_data->num_samples);

    // Vyhodnotenie na testovacej množine
    float test_loss = 0.0f;
    float test_accuracy = 0.0f;
    int* confusion_matrix = (int*)calloc(OUTPUT_SIZE * OUTPUT_SIZE, sizeof(int));
    if(!confusion_matrix){
        printf("Chyba: Nepodarilo sa alokovať pamäť pre confusion matrix\n");
        network_free(network);
        dataset_free(train_data);
        dataset_free(val_data);
        dataset_free(test_data);
        return 1;
    }

    for(int i = 0; i < test_data->num_samples; i++){
        float predictions[OUTPUT_SIZE];
        float* output = network_predict(network, test_data->inputs[i]);
        if(!output){
            printf("Chyba: Nepodarilo sa vykonať predikciu\n");
            free(confusion_matrix);
            network_free(network);
            dataset_free(train_data);
            dataset_free(val_data);
            dataset_free(test_data);
            return 1;
        }
        for(int j = 0; j < OUTPUT_SIZE; j++){
            predictions[j] = output[j];
        }

        // Výpočet loss
        for(int j = 0; j < OUTPUT_SIZE; j++){
            float diff = predictions[j] - test_data->targets[i][j];
            test_loss += diff * diff;
        }

        // Výpočet presnosti a aktualizácia confusion matrix
        test_accuracy += calculate_accuracy(predictions, test_data->targets[i], OUTPUT_SIZE);
        
        // Nájdenie predikovanej a skutočnej triedy
        int predicted_class = 0;
        int true_class = 0;
        float max_prob = predictions[0];

        for(int j = 1; j < OUTPUT_SIZE; j++){
            if(predictions[j] > max_prob){
                max_prob = predictions[j];
                predicted_class = j;
            }
            if(test_data->targets[i][j] > 0.5f){
                true_class = j;
            }
        }

        confusion_matrix[true_class * OUTPUT_SIZE + predicted_class]++;

        // Výpis ukážky predikcií každých 1000 vzoriek
        if((i + 1) % 1000 == 0){
            printf("Vzorka %d - Predikcia: %d, Skutočnosť: %d\n", 
                   i + 1, predicted_class, true_class);
            printf("Pravdepodobnosti: ");
            for(int j = 0; j < OUTPUT_SIZE; j++){
                printf("%.4f ", predictions[j]);
            }
            printf("\n");
        }
    }

    test_loss /= (2.0f * test_data->num_samples);
    test_accuracy /= test_data->num_samples;

    printf("\nVýsledky testovania:\n");
    printf("Test loss: %f\n", test_loss);
    printf("Test accuracy: %.2f%%\n", test_accuracy * 100);

    // Výpis confusion matrix
    print_confusion_matrix(confusion_matrix, OUTPUT_SIZE);

    // Výpočet a výpis metrík pre každú triedu
    float precision[OUTPUT_SIZE];
    float recall[OUTPUT_SIZE];
    float f1_score[OUTPUT_SIZE];

    calculate_metrics(confusion_matrix, OUTPUT_SIZE, precision, recall, f1_score);
    print_metrics(precision, recall, f1_score, OUTPUT_SIZE);

    // Upratanie
    free(confusion_matrix);
    network_free(network);
    dataset_free(train_data);
    dataset_free(val_data);
    dataset_free(test_data);

    return 0;
}

float calculate_accuracy(float* predictions, float* targets, int size){
    int predicted_class = 0;
    int true_class = 0;
    float max_pred = predictions[0];
    float max_true = targets[0];

    for(int i = 1; i < size; i++){
        if(predictions[i] > max_pred){
            max_pred = predictions[i];
            predicted_class = i;
        }
        if(targets[i] > max_true){
            max_true = targets[i];
            true_class = i;
        }
    }

    return (predicted_class == true_class) ? 1.0f : 0.0f;
}

void print_confusion_matrix(int* matrix, int size){
    printf("\nConfusion Matrix:\n");
    printf("Pred\\True");
    for(int i = 0; i < size; i++){
        printf("\t%d", i);
    }
    printf("\n");

    for(int i = 0; i < size; i++){
        printf("%d", i);
        for(int j = 0; j < size; j++){
            printf("\t%d", matrix[i * size + j]);
        }
        printf("\n");
    }
}

void calculate_metrics(int* matrix, int size, float* precision, float* recall, float* f1_score){
    for(int i = 0; i < size; i++){
        int true_positive = matrix[i * size + i];
        int false_positive = 0;
        int false_negative = 0;

        for(int j = 0; j < size; j++){
            if(j != i){
                false_positive += matrix[j * size + i];
                false_negative += matrix[i * size + j];
            }
        }

        precision[i] = true_positive + false_positive > 0 ? 
            (float)true_positive / (true_positive + false_positive) : 0.0f;
        recall[i] = true_positive + false_negative > 0 ? 
            (float)true_positive / (true_positive + false_negative) : 0.0f;
        f1_score[i] = precision[i] + recall[i] > 0 ? 
            2.0f * precision[i] * recall[i] / (precision[i] + recall[i]) : 0.0f;
    }
}

void print_metrics(float* precision, float* recall, float* f1_score, int size){
    printf("\nMetriky pre jednotlivé triedy:\n");
    printf("Trieda\tPrecision\tRecall\t\tF1-Score\n");
    for(int i = 0; i < size; i++){
        printf("%d\t%.4f\t\t%.4f\t\t%.4f\n", i, precision[i], recall[i], f1_score[i]);
    }
} 