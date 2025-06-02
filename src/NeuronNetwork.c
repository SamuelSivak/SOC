#include "../include/NeuronNetwork.h"
#include "../include/Layer.h"
#include "../include/Activation.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * Implementácia neurónovej siete.
 * Sieť je tvorená vrstvami neurónov, kde každá vrstva
 * môže mať rôzny počet neurónov a rôzne aktivačné funkcie.
 * Podporuje učenie pomocou spätného šírenia chyby.
 */

NeuralNetwork* network_create(int* layer_sizes, int num_layers, float learning_rate){
    /*
     * Vytvorí novú neurónovú sieť so zadanou architektúrou
     * Parametre:
     * - layer_sizes: pole veľkostí jednotlivých vrstiev
     * - num_layers: počet vrstiev
     * - learning_rate: rýchlosť učenia
     */
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

    // Kopírovanie veľkostí vrstiev
    memcpy(network->layer_sizes, layer_sizes, num_layers * sizeof(int));

    // Vytvorenie vrstiev s príslušnými aktivačnými funkciami
    for(int i = 0; i < num_layers - 1; i++){
        // Posledná vrstva používa softmax, ostatné ReLU
        ActivationType act_type = (i == num_layers - 2) ? ACTIVATION_SOFTMAX : ACTIVATION_RELU;
        network->layers[i] = layer_create(layer_sizes[i + 1], layer_sizes[i], act_type);
        if(!network->layers[i]){
            network_free(network);
            return NULL;
        }
    }

    // Alokácia pamäte pre vstupné a výstupné dáta
    network->input_data = (float*)malloc(layer_sizes[0] * sizeof(float));
    network->output_data = (float*)malloc(layer_sizes[num_layers - 1] * sizeof(float));

    if(!network->input_data || !network->output_data){
        network_free(network);
        return NULL;
    }

    return network;
}

void network_free(NeuralNetwork* network){
    /*
     * Uvoľní všetku pamäť alokovanú pre sieť
     * vrátane všetkých vrstiev a pomocných polí
     */
    if(network){
        if(network->layers){
            for(int i = 0; i < network->num_layers - 1; i++){
                layer_free(network->layers[i]);
            }
            free(network->layers);
        }
        free(network->layer_sizes);
        free(network->input_data);
        free(network->output_data);
        free(network);
    }
}

void network_forward(NeuralNetwork* network, float* input){
    /*
     * Dopredné šírenie signálu cez celú sieť
     * 1. Skopíruje vstupné dáta
     * 2. Postupne spracuje všetky vrstvy
     * 3. Uloží výsledok do output_data
     */
    
    // Kopírovanie vstupných dát do siete
    memcpy(network->input_data, input, network->layer_sizes[0] * sizeof(float));

    // Postupné spracovanie všetkých vrstiev
    float* current_input = network->input_data;
    for(int i = 0; i < network->num_layers - 1; i++){
        layer_forward(network->layers[i], current_input);
        current_input = network->layers[i]->outputs;
    }

    // Uloženie výsledku do výstupného poľa
    memcpy(network->output_data, current_input, 
           network->layer_sizes[network->num_layers - 1] * sizeof(float));
}

void network_backward(NeuralNetwork* network, float* target_output){
    /*
     * Spätné šírenie chyby cez celú sieť
     * Postupuje od poslednej vrstvy k prvej,
     * aktualizuje váhy a biasy všetkých neurónov
     */
    
    // Spracovanie vrstiev od konca
    for(int i = network->num_layers - 2; i >= 0; i--){
        // Určenie vstupov a cieľových hodnôt pre vrstvu
        float* layer_input = (i == 0) ? network->input_data : network->layers[i-1]->outputs;
        float* layer_target = (i == network->num_layers - 2) ? target_output : network->layers[i+1]->deltas;
        Layer* next_layer = (i == network->num_layers - 2) ? NULL : network->layers[i+1];
        
        // Aktualizácia váh vrstvy
        layer_backward(network->layers[i], layer_input, layer_target, next_layer, network->learning_rate);
    }
}

void network_train(NeuralNetwork* network, float* input, float* target_output){
    /*
     * Trénovanie siete na jednej vzorke
     * 1. Dopredné šírenie pre výpočet predikcie
     * 2. Spätné šírenie pre aktualizáciu váh
     */
    network_forward(network, input);
    network_backward(network, target_output);
}

float* network_predict(NeuralNetwork* network, float* input){
    /*
     * Predikcia výstupu pre zadaný vstup
     * Vykoná len dopredné šírenie bez aktualizácie váh
     */
    network_forward(network, input);
    return network->output_data;
}

void network_randomize(NeuralNetwork* network, float min, float max){
    /*
     * Inicializuje váhy všetkých neurónov v sieti
     * na náhodné hodnoty v zadanom rozsahu
     */
    for(int i = 0; i < network->num_layers - 1; i++){
        layer_randomize(network->layers[i], min, max);
    }
}

void network_print(NeuralNetwork* network){
    /*
     * Vypíše informácie o sieti na štandardný výstup
     * Zobrazí architektúru siete a detaily o každej vrstve
     */
    printf("Neurónová sieť (%d vrstiev):\n", network->num_layers);
    printf("Veľkosti vrstiev: ");
    for(int i = 0; i < network->num_layers; i++){
        printf("%d ", network->layer_sizes[i]);
    }
    printf("\nRýchlosť učenia: %f\n\n", network->learning_rate);

    for(int i = 0; i < network->num_layers - 1; i++){
        printf("Vrstva %d:\n", i + 1);
        layer_print(network->layers[i]);
    }
}

NeuralNetwork* network_load(const char* filename){
    // Načítanie modelu zo súboru
    FILE* file = fopen(filename, "rb");
    if(!file) return NULL;

    // Načítanie základných parametrov
    int num_layers;
    fread(&num_layers, sizeof(int), 1, file);

    int* layer_sizes = (int*)malloc(num_layers * sizeof(int));
    fread(layer_sizes, sizeof(int), num_layers, file);

    float learning_rate;
    fread(&learning_rate, sizeof(float), 1, file);

    // Vytvorenie siete
    NeuralNetwork* network = network_create(layer_sizes, num_layers, learning_rate);
    if(!network){
        free(layer_sizes);
        fclose(file);
        return NULL;
    }

    // Načítanie váh pre každú vrstvu
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

void network_save(NeuralNetwork* network, const char* filename){
    // Uloženie modelu do súboru
    FILE* file = fopen(filename, "wb");
    if(!file) return;

    // Uloženie základných parametrov
    fwrite(&network->num_layers, sizeof(int), 1, file);
    fwrite(network->layer_sizes, sizeof(int), network->num_layers, file);
    fwrite(&network->learning_rate, sizeof(float), 1, file);

    // Uloženie váh pre každú vrstvu
    for(int i = 0; i < network->num_layers - 1; i++){
        Layer* layer = network->layers[i];
        for(int j = 0; j < layer->num_neurons; j++){
            fwrite(layer->neurons[j]->weights, sizeof(float), layer->num_inputs, file);
            fwrite(&layer->neurons[j]->bias, sizeof(float), 1, file);
        }
    }

    fclose(file);
}
