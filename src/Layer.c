/*
 * Implementácia vrstvy neurónovej siete.
 * 
 * Tento modul implementuje vrstvu neurónovej siete, ktorá sa skladá z kolekcie
 * neurónov. Každá vrstva je plne prepojená s nasledujúcou vrstvou (fully connected).
 * Modul poskytuje funkcionalitu pre dopredné šírenie (forward propagation) a
 * spätné šírenie chyby (backpropagation).
 * 
 * Podporované aktivačné funkcie:
 * - ReLU: f(x) = max(0, x)
 * - Softmax: f(x[i]) = exp(x[i]) / Σ exp(x[j])
 */

#include "../include/Layer.h"
#include "../include/Neuron.h"
#include "../include/Activation.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

Layer* layer_create(int num_neurons, int num_inputs, ActivationType activation_type){
    /*
     * Vytvorenie novej vrstvy neurónovej siete
     * 
     * Parametre:
     * - num_neurons: počet neurónov vo vrstve
     * - num_inputs: počet vstupov pre každý neurón
     * - activation_type: typ aktivačnej funkcie (ReLU alebo Softmax)
     * 
     * Alokuje pamäť pre:
     * 1. Štruktúru vrstvy
     * 2. Pole neurónov
     * 3. Výstupný vektor
     * 4. Vektor chýb pre backpropagation
     */
    Layer* layer = (Layer*)malloc(sizeof(Layer));
    if(!layer) return NULL;

    layer->num_neurons = num_neurons;
    layer->num_inputs = num_inputs;
    layer->activation_type = activation_type;
    
    layer->neurons = (Neuron**)malloc(num_neurons * sizeof(Neuron*));
    layer->outputs = (float*)malloc(num_neurons * sizeof(float));
    layer->deltas = (float*)malloc(num_neurons * sizeof(float));
    
    if(!layer->neurons || !layer->outputs || !layer->deltas){
        layer_free(layer);
        return NULL;
    }

    // Inicializácia neurónov vrstvy
    for(int i = 0; i < num_neurons; i++){
        layer->neurons[i] = neuron_create(num_inputs, activation_type);
        if(!layer->neurons[i]){
            layer_free(layer);
            return NULL;
        }
    }

    return layer;
}

void layer_free(Layer* layer){
    /*
     * Uvoľnenie pamäte alokovanej pre vrstvu
     * Uvoľňuje:
     * 1. Pamäť každého neurónu
     * 2. Pole neurónov
     * 3. Výstupný vektor a vektor chýb
     * 4. Štruktúru vrstvy
     */
    if(layer){
        if(layer->neurons){
            for(int i = 0; i < layer->num_neurons; i++){
                neuron_free(layer->neurons[i]);
            }
            free(layer->neurons);
        }
        free(layer->outputs);
        free(layer->deltas);
        free(layer);
    }
}

void layer_forward(Layer* layer, float* inputs){
    /*
     * Dopredné šírenie signálu vrstvou
     * 
     * 1. Pre každý neurón vypočíta váženú sumu vstupov:
     *    sum = Σ (weight[i] * input[i]) + bias
     * 
     * 2. Aplikuje aktivačnú funkciu:
     *    - Pre ReLU: output = max(0, sum)
     *    - Pre Softmax: output[i] = exp(sum[i]) / Σ exp(sum[j])
     *      s numerickou stabilizáciou: sum[i] = sum[i] - max(sum)
     */
    // Výpočet výstupov všetkých neurónov (vážené sumy)
    for(int i = 0; i < layer->num_neurons; i++){
        layer->outputs[i] = neuron_forward(layer->neurons[i], inputs);
    }

    // Ak je to výstupná vrstva so softmax, aplikujeme softmax na všetky výstupy
    if(layer->activation_type == ACTIVATION_SOFTMAX){
        float max_val = layer->outputs[0];
        for(int i = 1; i < layer->num_neurons; i++){
            if(layer->outputs[i] > max_val){
                max_val = layer->outputs[i];
            }
        }
        
        float sum = 0.0f;
        for(int i = 0; i < layer->num_neurons; i++){
            layer->outputs[i] = expf(layer->outputs[i] - max_val);
            sum += layer->outputs[i];
        }
        
        if(sum > 0.0f){
            for(int i = 0; i < layer->num_neurons; i++){
                layer->outputs[i] /= sum;
                layer->neurons[i]->output = layer->outputs[i];
            }
        }else{
            // Pri nulovej sume použijeme rovnomernú distribúciu
            float uniform_prob = 1.0f / layer->num_neurons;
            for(int i = 0; i < layer->num_neurons; i++){
                layer->outputs[i] = uniform_prob;
                layer->neurons[i]->output = uniform_prob;
            }
        }
    }else{
        // Aplikácia ReLU aktivácie
        for(int i = 0; i < layer->num_neurons; i++){
            layer->outputs[i] = relu(layer->outputs[i]);
            layer->neurons[i]->output = layer->outputs[i];
        }
    }
}

void layer_backward(Layer* layer, float* inputs, float* target_outputs, Layer* next_layer, float learning_rate){
    /*
     * Spätné šírenie chyby a aktualizácia váh
     * 
     * Pre výstupnú vrstvu (Softmax):
     * - delta[i] = output[i] - target[i]
     * 
     * Pre skryté vrstvy (ReLU):
     * - delta[i] = Σ (next_layer_weights[j][i] * next_layer_delta[j]) * relu'(sum[i])
     * kde relu'(x) = 1 pre x > 0, 0 inak
     * 
     * Aktualizácia váh:
     * - weight[i] -= learning_rate * delta * input[i]
     * - bias -= learning_rate * delta
     * 
     * Používa gradient clipping pre prevenciu explodujúcich gradientov
     */
    if(layer->activation_type == ACTIVATION_SOFTMAX){
        // Výpočet chýb pre výstupnú vrstvu so Softmax
        for(int i = 0; i < layer->num_neurons; i++){
            // Orezanie výstupov pre prevenciu delenia nulou
            float output = fmaxf(fminf(layer->outputs[i], 1.0f - 1e-7f), 1e-7f);
            layer->deltas[i] = output - target_outputs[i];
            layer->neurons[i]->delta = layer->deltas[i];
        }
    }else if(next_layer != NULL){
        // Výpočet chýb pre skryté vrstvy s ReLU
        for(int i = 0; i < layer->num_neurons; i++){
            float sum = 0.0f;
            // Použitie chýb z nasledujúcej vrstvy
            for(int j = 0; j < next_layer->num_neurons; j++){
                sum += next_layer->neurons[j]->weights[i] * next_layer->neurons[j]->delta;
            }
            // Aplikácia derivácie ReLU
            layer->deltas[i] = sum * relu_derivative(layer->neurons[i]->sum);
            layer->neurons[i]->delta = layer->deltas[i];
        }
    }

    // Aktualizácia váh všetkých neurónov s orezaním gradientov
    const float max_grad = 1.0f;  // Maximálna povolená veľkosť gradientu
    for(int i = 0; i < layer->num_neurons; i++){
        float delta = layer->neurons[i]->delta;
        
        // Orezanie delty pre prevenciu explodujúcich gradientov
        if(delta > max_grad) delta = max_grad;
        if(delta < -max_grad) delta = -max_grad;
        
        // Aktualizácia váh a biasu s orezanými gradientmi
        for(int j = 0; j < layer->num_inputs; j++){
            float gradient = delta * inputs[j];
            // Orezanie gradientu
            if(gradient > max_grad) gradient = max_grad;
            if(gradient < -max_grad) gradient = -max_grad;
            layer->neurons[i]->weights[j] -= learning_rate * gradient;
        }
        layer->neurons[i]->bias -= learning_rate * delta;
    }
}

void layer_randomize(Layer* layer, float min, float max){
    /*
     * Inicializácia váh a biasov náhodnými hodnotami
     * Používa uniformné rozdelenie v intervale [min, max]
     * 
     * Vhodná inicializácia váh je kľúčová pre:
     * 1. Zabránenie saturácie neurónov
     * 2. Zachovanie rozptylu signálu pri prechode sieťou
     * 3. Urýchlenie konvergencie učenia
     */
    for(int i = 0; i < layer->num_neurons; i++){
        neuron_randomize(layer->neurons[i], min, max);
    }
}

Layer* layer_copy(Layer* layer){
    /*
     * Vytvorenie hlbokej kópie vrstvy
     * Kopíruje:
     * 1. Štruktúru vrstvy
     * 2. Všetky neuróny
     * 3. Váhy a biasy každého neurónu
     */
    if(!layer) return NULL;
    
    Layer* copy = layer_create(layer->num_neurons, layer->num_inputs, layer->activation_type);
    if(!copy) return NULL;
    
    // Kopírovanie váh a biasov neurónov
    for(int i = 0; i < layer->num_neurons; i++){
        memcpy(copy->neurons[i]->weights, layer->neurons[i]->weights, 
               layer->num_inputs * sizeof(float));
        copy->neurons[i]->bias = layer->neurons[i]->bias;
    }
    
    return copy;
}

void layer_print(Layer* layer){
    /*
     * Výpis informácií o vrstve
     * Zobrazuje:
     * 1. Počet neurónov a vstupov
     * 2. Váhy a biasy každého neurónu
     * 3. Aktuálne výstupné hodnoty
     */
    printf("Layer (%d neurons, %d inputs):\n", layer->num_neurons, layer->num_inputs);
    for(int i = 0; i < layer->num_neurons; i++){
        printf("\nNeuron %d:\n", i);
        neuron_print(layer->neurons[i]);
    }
    printf("\nOutputs: ");
    for(int i = 0; i < layer->num_neurons; i++){
        printf("%8.4f ", layer->outputs[i]);
    }
    printf("\n");
}
