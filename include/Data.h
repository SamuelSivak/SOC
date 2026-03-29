#ifndef DATA_H
#define DATA_H

/*
 * Súbor: Data.h 
 * Popis:
 * Tento súbor obsahuje implementáciu správy dátových množín
 * pre neurónovú sieť. Poskytuje funkcie pre načítanie,
 * predspracovanie a manipuláciu s dátami, vrátane podpory
 * pre MNIST dataset.
 */

#include <stdio.h>

// Definícia štruktúry pre dátovú množinu
typedef struct {
    float** inputs;          // Pole vstupných dát
    float** targets;         // Pole cieľových hodnôt
    int num_samples;         // Počet vzoriek
    int input_size;          // Veľkosť vstupného vektora
    int target_size;         // Veľkosť cieľového vektora
} Dataset;

/*
 * Vytvorí novú dátovú množinu
 * Parameter num_samples: Počet vzoriek v množine
 * Parameter input_size: Veľkosť vstupného vektora
 * Parameter target_size: Veľkosť cieľového vektora
 * Funkcia vracia: Smerník na novú dátovú množinu alebo NULL pri chybe
 */
Dataset* dataset_create(int num_samples, int input_size, int target_size);

/*
 * Uvoľní pamäť alokovanú pre dátovú množinu
 * Parameter dataset: Smerník na množinu, ktorá sa má uvoľniť
 * Funkcia vracia: void
 */
void dataset_free(Dataset* dataset);

/*
 * Náhodne zamieša vzorky v dátovej množine
 * Parameter dataset: Množina na zamiešanie
 * Funkcia vracia: void
 */
void dataset_shuffle(Dataset* dataset);

/*
 * Vytvorí mini-batch z dátovej množiny
 * Parameter dataset: Zdrojová množina
 * Parameter batch_size: Veľkosť mini-batch
 * Parameter batch: Cieľová množina pre mini-batch
 * Parameter start_idx: Začiatočný index v zdrojovej množine
 * Funkcia vracia: void
 */
void dataset_create_batch(Dataset* dataset, int batch_size, Dataset* batch, int start_idx);

/*
 * Načíta MNIST dataset z binárnych súborov
 * Parameter train_images_file: Cesta k súboru s trénovacími obrázkami
 * Parameter train_labels_file: Cesta k súboru s trénovacími značkami
 * Parameter test_images_file: Cesta k súboru s testovacími obrázkami
 * Parameter test_labels_file: Cesta k súboru s testovacími značkami
 * Parameter train_data: Výstupný parameter pre trénovaciu množinu
 * Parameter val_data: Výstupný parameter pre validačnú množinu
 * Parameter test_data: Výstupný parameter pre testovaciu množinu
 * Parameter val_ratio: Pomer validačných dát (0-1)
 * Funkcia vracia: 0 pri úspechu, -1 pri chybe
 */
int dataset_load_mnist(const char* train_images_file, const char* train_labels_file,
                      const char* test_images_file, const char* test_labels_file,
                      Dataset** train_data, Dataset** val_data, Dataset** test_data,
                      float val_ratio);

#endif

