#include "../include/Evaluation.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/*
 * Implementácia funkcií pre vyhodnocovanie výkonu neurónovej siete.
 * 
 * Tento modul poskytuje implementácie metrík a nástrojov na vyhodnotenie
 * úspešnosti neurónovej siete. Zahŕňa výpočet presnosti, confusion matrix,
 * ROC krivky a ďalších štatistík potrebných pre analýzu klasifikačných modelov.
 * 
 * Implementované metriky:
 * - Confusion Matrix: matica skutočných vs. predikovaných tried
 * - Accuracy: celková presnosť klasifikácie
 * - ROC krivka: vzťah medzi true positive a false positive rate
 * - AUC: plocha pod ROC krivkou
 */

ConfusionMatrix* confusion_matrix_create(int num_classes){
    /*
     * Vytvorenie confusion matrix pre klasifikáciu
     * Matica má rozmery num_classes x num_classes, kde:
     * - Riadky predstavujú skutočné triedy
     * - Stĺpce predstavujú predikované triedy
     * - Hodnota v bunke [i][j] je počet vzoriek triedy i predikovaných ako trieda j
     */
    ConfusionMatrix* cm = (ConfusionMatrix*)malloc(sizeof(ConfusionMatrix));
    if(!cm) return NULL;

    cm->num_classes = num_classes;
    cm->matrix = (int**)malloc(num_classes * sizeof(int*));
    if(!cm->matrix){
        free(cm);
        return NULL;
    }

    // Alokácia a inicializácia matice
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
    /*
     * Aktualizácia confusion matrix na základe nových predikcií
     * Pre každú vzorku:
     * 1. Nájde predikovanú triedu (argmax z predikčného vektora)
     * 2. Nájde skutočnú triedu (argmax z cieľového vektora)
     * 3. Inkrementuje príslušnú bunku v confusion matrix
     */
    for(int i = 0; i < size; i++){
        int pred_class = 0;
        int true_class = 0;
        float max_pred = predictions[i * cm->num_classes];
        float max_true = targets[i * cm->num_classes];

        // Nájdenie predikovanej triedy
        for(int j = 1; j < cm->num_classes; j++){
            if(predictions[i * cm->num_classes + j] > max_pred){
                max_pred = predictions[i * cm->num_classes + j];
                pred_class = j;
            }
        }

        // Nájdenie skutočnej triedy
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
    /*
     * Výpočet celkovej presnosti klasifikácie
     * Vzorec: accuracy = (TP + TN) / (TP + TN + FP + FN)
     * kde TP = true positives (správne pozitívne)
     *     TN = true negatives (správne negatívne)
     *     FP = false positives (nesprávne pozitívne)
     *     FN = false negatives (nesprávne negatívne)
     */
    int correct = 0;
    int total = 0;

    for(int i = 0; i < cm->num_classes; i++){
        for(int j = 0; j < cm->num_classes; j++){
            if(i == j) correct += cm->matrix[i][j];  // Diagonálne prvky (správne predikcie)
            total += cm->matrix[i][j];
        }
    }

    return total > 0 ? (float)correct / total : 0.0f;
}

ROCCurve* roc_curve_create(float* predictions, float* targets, int size, int num_points){
    /*
     * Vytvorenie ROC (Receiver Operating Characteristic) krivky
     * ROC krivka zobrazuje vzťah medzi:
     * - True Positive Rate (TPR) = TP / (TP + FN)
     * - False Positive Rate (FPR) = FP / (FP + TN)
     * pre rôzne prahy klasifikácie
     */
    ROCCurve* roc = (ROCCurve*)malloc(sizeof(ROCCurve));
    if(!roc) return NULL;

    roc->num_points = num_points;
    roc->tpr = (float*)malloc(num_points * sizeof(float));
    roc->fpr = (float*)malloc(num_points * sizeof(float));
    roc->thresholds = (float*)malloc(num_points * sizeof(float));

    if(!roc->tpr || !roc->fpr || !roc->thresholds){
        roc_curve_free(roc);
        return NULL;
    }

    // Výpočet bodov ROC krivky pre rôzne prahy
    for(int i = 0; i < num_points; i++){
        float threshold = (float)i / (num_points - 1);
        roc->thresholds[i] = threshold;

        int tp = 0, fp = 0, tn = 0, fn = 0;
        for(int j = 0; j < size; j++){
            if(predictions[j] >= threshold){
                if(targets[j] > 0) tp++;
                else fp++;
            }else{
                if(targets[j] > 0) fn++;
                else tn++;
            }
        }

        // Výpočet True Positive Rate a False Positive Rate
        roc->tpr[i] = tp + fn > 0 ? (float)tp / (tp + fn) : 0.0f;
        roc->fpr[i] = fp + tn > 0 ? (float)fp / (fp + tn) : 0.0f;
    }

    return roc;
}

void roc_curve_free(ROCCurve* roc){
    if(roc){
        free(roc->tpr);
        free(roc->fpr);
        free(roc->thresholds);
        free(roc);
    }
}

float roc_curve_auc(ROCCurve* roc){
    /*
     * Výpočet AUC (Area Under Curve) pre ROC krivku
     * AUC predstavuje pravdepodobnosť, že klasifikátor zaradí náhodne
     * vybranú pozitívnu vzorku vyššie ako náhodne vybranú negatívnu
     * 
     * Vzorec (lichobežníková metóda):
     * AUC = Σ (FPR[i] - FPR[i-1]) * (TPR[i] + TPR[i-1]) / 2
     */
    float auc = 0.0f;
    for(int i = 1; i < roc->num_points; i++){
        auc += (roc->fpr[i] - roc->fpr[i-1]) * (roc->tpr[i] + roc->tpr[i-1]) / 2.0f;
    }
    return auc;
}

float network_validate(NeuralNetwork* network, Dataset* val_data, float (*loss_fn)(float*, float*, int)){
    /*
     * Validácia neurónovej siete na validačnej množine
     * Počíta priemernú hodnotu loss funkcie na validačných dátach
     * Vzorec: validation_loss = Σ loss(prediction, target) / N
     * kde N je počet vzoriek vo validačnej množine
     */
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
    /*
     * Testovanie neurónovej siete na testovacej množine
     * 1. Počíta priemernú hodnotu loss funkcie
     * 2. Aktualizuje confusion matrix pre analýzu chýb
     * 3. Umožňuje výpočet rôznych metrík (accuracy, precision, recall)
     */
    float total_loss = 0.0f;
    float* predictions = (float*)malloc(test_data->target_size * sizeof(float));
    if(!predictions) return -1.0f;

    // Reset confusion matrix ak existuje
    if(cm){
        for(int i = 0; i < cm->num_classes; i++){
            memset(cm->matrix[i], 0, cm->num_classes * sizeof(int));
        }
    }

    // Výpočet loss a aktualizácia confusion matrix
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
    /*
     * Uloženie modelu do binárneho súboru
     * Ukladá:
     * 1. Počet vrstiev a ich veľkosti
     * 2. Learning rate
     * 3. Váhy a biasy pre každú vrstvu
     */
    FILE* file = fopen(filename, "wb");
    if(!file) return -1;

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
    return 0;
}

NeuralNetwork* model_load(const char* filename){
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

void confusion_matrix_print(ConfusionMatrix* cm){
    /*
     * Výpis confusion matrix v čitateľnom formáte
     * - Riadky: skutočné triedy
     * - Stĺpce: predikované triedy
     * - Hodnoty: počet vzoriek
     */
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

void roc_curve_print(ROCCurve* roc){
    /*
     * Výpis bodov ROC krivky
     * Pre každý bod vypíše:
     * - Prah (threshold)
     * - True Positive Rate (TPR)
     * - False Positive Rate (FPR)
     */
    printf("\nROC Curve Points:\n");
    printf("%-15s%-15s%-15s\n", "Threshold", "TPR", "FPR");
    for(int i = 0; i < roc->num_points; i++){
        printf("%-15.3f%-15.3f%-15.3f\n", 
               roc->thresholds[i], 
               roc->tpr[i], 
               roc->fpr[i]);
    }
} 