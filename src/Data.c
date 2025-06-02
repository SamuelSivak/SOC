#include "../include/Data.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * Implementácia správy dátových množín pre neurónovú sieť.
 * Poskytuje funkcie pre načítanie, uloženie, normalizáciu
 * a manipuláciu s trénovacími a testovacími dátami.
 */

// Konštanty pre prácu s MNIST datasetom
 const int MNIST_MAGIC_IMAGES = 0x803;
 const int MNIST_MAGIC_LABELS = 0x801;
 const int MNIST_IMAGE_SIZE = 784;  // 28x28 pixelov
 const int MNIST_NUM_CLASSES = 10;

Dataset* dataset_create(int num_samples, int input_size, int target_size){
    /*
     * Vytvorí novú dátovú množinu so zadanými rozmermi
     * Alokuje pamäť pre vstupné a cieľové hodnoty
     */
    Dataset* dataset = (Dataset*)malloc(sizeof(Dataset));
    if(!dataset) return NULL;

    dataset->num_samples = num_samples;
    dataset->input_size = input_size;
    dataset->target_size = target_size;

    // Alokácia pamäte pre vstupy a cieľové hodnoty
    dataset->inputs = (float**)malloc(num_samples * sizeof(float*));
    dataset->targets = (float**)malloc(num_samples * sizeof(float*));

    if(!dataset->inputs || !dataset->targets){
        dataset_free(dataset);
        return NULL;
    }

    // Alokácia pamäte pre jednotlivé vzorky
    for(int i = 0; i < num_samples; i++){
        dataset->inputs[i] = (float*)calloc(input_size, sizeof(float));
        dataset->targets[i] = (float*)calloc(target_size, sizeof(float));
        
        if(!dataset->inputs[i] || !dataset->targets[i]){
            dataset_free(dataset);
            return NULL;
        }
    }

    return dataset;
}

void dataset_free(Dataset* dataset){
    /*
     * Uvoľní pamäť alokovanú pre dátovú množinu
     * vrátane všetkých vstupných a cieľových hodnôt
     */
    if(dataset){
        if(dataset->inputs){
            for(int i = 0; i < dataset->num_samples; i++){
                free(dataset->inputs[i]);
            }
            free(dataset->inputs);
        }
        if(dataset->targets){
            for(int i = 0; i < dataset->num_samples; i++){
                free(dataset->targets[i]);
            }
            free(dataset->targets);
        }
        free(dataset);
    }
}

Dataset* dataset_load_csv(const char* filename, int input_size, int target_size){
    /*
     * Načíta dátovú množinu z CSV súboru
     * Predpokladá formát: vstupné hodnoty, cieľové hodnoty oddelené čiarkami
     */
    FILE* file = fopen(filename, "r");
    if(!file) return NULL;

    // Spočítanie počtu riadkov v súbore
    int num_samples = 0;
    char line[1024];
    while(fgets(line, sizeof(line), file)){
        num_samples++;
    }
    rewind(file);

    // Vytvorenie dátovej množiny
    Dataset* dataset = dataset_create(num_samples, input_size, target_size);
    if(!dataset){
        fclose(file);
        return NULL;
    }

    // Načítanie dát zo súboru
    for(int i = 0; i < num_samples; i++){
        if(!fgets(line, sizeof(line), file)){
            break;
        }

        // Parsovanie vstupných hodnôt
        char* token = strtok(line, ",");
        for(int j = 0; j < input_size; j++){
            if(!token){
                break;
            }
            dataset->inputs[i][j] = atof(token);
            token = strtok(NULL, ",");
        }

        // Parsovanie cieľových hodnôt
        for(int j = 0; j < target_size; j++){
            if(!token){
                break;
            }
            dataset->targets[i][j] = atof(token);
            token = strtok(NULL, ",");
        }
    }

    fclose(file);
    return dataset;
}

void dataset_save_csv(Dataset* dataset, const char* filename){
    /*
     * Uloží dátovú množinu do CSV súboru
     * Formát: vstupné hodnoty, cieľové hodnoty oddelené čiarkami
     */
    FILE* file = fopen(filename, "w");
    if(!file) return;

    for(int i = 0; i < dataset->num_samples; i++){
        // Zápis vstupných hodnôt
        for(int j = 0; j < dataset->input_size; j++){
            fprintf(file, "%f", dataset->inputs[i][j]);
            if(j < dataset->input_size - 1){
                fprintf(file, ",");
            }
        }
        fprintf(file, ",");

        // Zápis cieľových hodnôt
        for(int j = 0; j < dataset->target_size; j++){
            fprintf(file, "%f", dataset->targets[i][j]);
            if(j < dataset->target_size - 1){
                fprintf(file, ",");
            }
        }
        fprintf(file, "\n");
    }

    fclose(file);
}

void dataset_normalize(Dataset* dataset){
    /*
     * Normalizuje vstupné hodnoty do rozsahu [0,1]
     * Používa min-max normalizáciu pre každý vstupný rozmer
     */
    float* min_vals = (float*)malloc(dataset->input_size * sizeof(float));
    float* max_vals = (float*)malloc(dataset->input_size * sizeof(float));

    // Nájdenie minimálnych a maximálnych hodnôt
    for(int j = 0; j < dataset->input_size; j++){
        min_vals[j] = dataset->inputs[0][j];
        max_vals[j] = dataset->inputs[0][j];
        
        for(int i = 1; i < dataset->num_samples; i++){
            if(dataset->inputs[i][j] < min_vals[j]){
                min_vals[j] = dataset->inputs[i][j];
            }
            if(dataset->inputs[i][j] > max_vals[j]){
                max_vals[j] = dataset->inputs[i][j];
            }
        }
    }

    // Aplikácia min-max normalizácie
    for(int i = 0; i < dataset->num_samples; i++){
        for(int j = 0; j < dataset->input_size; j++){
            float range = max_vals[j] - min_vals[j];
            if(range > 0){
                dataset->inputs[i][j] = (dataset->inputs[i][j] - min_vals[j]) / range;
            }
        }
    }

    free(min_vals);
    free(max_vals);
}

void dataset_split(Dataset* dataset, float train_ratio, Dataset** train_data, Dataset** test_data){
    /*
     * Rozdelí dátovú množinu na trénovaciu a testovaciu časť
     * Parametre:
     * - train_ratio: pomer veľkosti trénovacej množiny (0-1)
     */
    int train_size = (int)(dataset->num_samples * train_ratio);
    int test_size = dataset->num_samples - train_size;

    // Vytvorenie nových dátových množín
    *train_data = dataset_create(train_size, dataset->input_size, dataset->target_size);
    *test_data = dataset_create(test_size, dataset->input_size, dataset->target_size);

    if(!*train_data || !*test_data) return;

    // Kopírovanie dát do trénovacej množiny
    for(int i = 0; i < train_size; i++){
        memcpy((*train_data)->inputs[i], dataset->inputs[i], 
               dataset->input_size * sizeof(float));
        memcpy((*train_data)->targets[i], dataset->targets[i], 
               dataset->target_size * sizeof(float));
    }

    // Kopírovanie dát do testovacej množiny
    for(int i = 0; i < test_size; i++){
        memcpy((*test_data)->inputs[i], dataset->inputs[train_size + i], 
               dataset->input_size * sizeof(float));
        memcpy((*test_data)->targets[i], dataset->targets[train_size + i], 
               dataset->target_size * sizeof(float));
    }
}

void dataset_shuffle(Dataset* dataset){
    /*
     * Náhodne premieša vzorky v dátovej množine
     * Zachováva párovanie vstupov a cieľových hodnôt
     */
    srand(time(NULL));
    
    for(int i = dataset->num_samples - 1; i > 0; i--){
        // Fisher-Yates shuffle algoritmus
        int j = rand() % (i + 1);
        
        // Výmena vstupných hodnôt
        float* temp_input = dataset->inputs[i];
        dataset->inputs[i] = dataset->inputs[j];
        dataset->inputs[j] = temp_input;

        // Výmena cieľových hodnôt
        float* temp_target = dataset->targets[i];
        dataset->targets[i] = dataset->targets[j];
        dataset->targets[j] = temp_target;
    }
}

void dataset_create_batch(Dataset* dataset, int batch_size, Dataset* batch, int start_idx){
    /*
     * Vytvorí batch (dávku) vzoriek z dátovej množiny
     * Používa sa pri mini-batch gradientnom zostupe
     */
    if(start_idx + batch_size > dataset->num_samples){
        batch_size = dataset->num_samples - start_idx;
    }

    // Kopírovanie vzoriek do batchu
    for(int i = 0; i < batch_size; i++){
        memcpy(batch->inputs[i], dataset->inputs[start_idx + i], 
               dataset->input_size * sizeof(float));
        memcpy(batch->targets[i], dataset->targets[start_idx + i], 
               dataset->target_size * sizeof(float));
    }
}

void dataset_print(Dataset* dataset){
    /*
     * Vypíše informácie o dátovej množine
     * a prvých niekoľko vzoriek na štandardný výstup
     */
    printf("Informácie o dátovej množine:\n");
    printf("Počet vzoriek: %d\n", dataset->num_samples);
    printf("Veľkosť vstupu: %d\n", dataset->input_size);
    printf("Veľkosť výstupu: %d\n", dataset->target_size);
    printf("\nPrvých niekoľko vzoriek:\n");
    
    int samples_to_show = dataset->num_samples < 5 ? dataset->num_samples : 5;
    for(int i = 0; i < samples_to_show; i++){
        printf("Vzorka %d:\n", i);
        printf("Vstupy: ");
        for(int j = 0; j < dataset->input_size; j++){
            printf("%f ", dataset->inputs[i][j]);
        }
        printf("\nVýstupy: ");
        for(int j = 0; j < dataset->target_size; j++){
            printf("%f ", dataset->targets[i][j]);
        }
        printf("\n\n");
    }
}

int read_int(FILE* f){
    /*
     * Pomocná funkcia pre čítanie 32-bitového integeru z MNIST súborov
     * MNIST používa big-endian formát
     */
    unsigned char b[4];
    fread(b, 1, 4, f);
    return (b[0]<<24) | (b[1]<<16) | (b[2]<<8) | b[3];
}

int dataset_load_mnist(const char* train_images_file, const char* train_labels_file,
                      const char* test_images_file, const char* test_labels_file,
                      Dataset** train_data, Dataset** val_data, Dataset** test_data,
                      float val_ratio) {
    /*
     * Načíta MNIST dataset z binárnych súborov
     * Rozdelí trénovacie dáta na trénovaciu a validačnú množinu
     */
    printf("Pokus o načítanie MNIST dát...\n");
    printf("Súbor trénovacích obrázkov: %s\n", train_images_file);
    printf("Súbor trénovacích značiek: %s\n", train_labels_file);
    printf("Súbor testovacích obrázkov: %s\n", test_images_file);
    printf("Súbor testovacích značiek: %s\n", test_labels_file);

    // Otvorenie všetkých potrebných súborov
    FILE *train_images = fopen(train_images_file, "rb");
    FILE *train_labels = fopen(train_labels_file, "rb");
    FILE *test_images = fopen(test_images_file, "rb");
    FILE *test_labels = fopen(test_labels_file, "rb");

    if(!train_images || !train_labels || !test_images || !test_labels){
        printf("Nepodarilo sa otvoriť jeden alebo viac súborov:\n");
        if(!train_images){
            printf("Nepodarilo sa otvoriť súbor trénovacích obrázkov\n");
        }
        if(!train_labels){
            printf("Nepodarilo sa otvoriť súbor trénovacích značiek\n");
        }
        if(!test_images){
            printf("Nepodarilo sa otvoriť súbor testovacích obrázkov\n");
        }
        if(!test_labels){
            printf("Nepodarilo sa otvoriť súbor testovacích značiek\n");
        }
        
        if(train_images){
            fclose(train_images);
        }
        if(train_labels){
            fclose(train_labels);
        }
        if(test_images){
            fclose(test_images);
        }
        if(test_labels){
            fclose(test_labels);
        }
        return -1;
    }

    printf("Úspešne otvorené všetky súbory\n");

    // Čítanie hlavičiek a počtu vzoriek z trénovacích obrázkov
    int train_magic = read_int(train_images);
    int num_train = read_int(train_images);
    int train_rows = read_int(train_images);
    int train_cols = read_int(train_images);

    printf("Hlavička trénovacích obrázkov:\n");
    printf("Magické číslo: %d\n", train_magic);
    printf("Počet obrázkov: %d\n", num_train);
    printf("Počet riadkov: %d\n", train_rows);
    printf("Počet stĺpcov: %d\n", train_cols);

    // Čítanie hlavičiek a počtu vzoriek z trénovacích značiek
    int train_labels_magic = read_int(train_labels);
    int num_train_labels = read_int(train_labels);

    printf("Hlavička trénovacích značiek:\n");
    printf("Magické číslo: %d\n", train_labels_magic);
    printf("Počet značiek: %d\n", num_train_labels);

    // Čítanie hlavičiek a počtu vzoriek z testovacích obrázkov
    int test_magic = read_int(test_images);
    int num_test = read_int(test_images);
    int test_rows = read_int(test_images);
    int test_cols = read_int(test_images);

    printf("Hlavička testovacích obrázkov:\n");
    printf("Magické číslo: %d\n", test_magic);
    printf("Počet obrázkov: %d\n", num_test);
    printf("Počet riadkov: %d\n", test_rows);
    printf("Počet stĺpcov: %d\n", test_cols);

    // Čítanie hlavičiek a počtu vzoriek z testovacích značiek
    int test_labels_magic = read_int(test_labels);
    int num_test_labels = read_int(test_labels);

    printf("Hlavička testovacích značiek:\n");
    printf("Magické číslo: %d\n", test_labels_magic);
    printf("Počet značiek: %d\n", num_test_labels);

    // Výpočet veľkostí množín
    int val_size = (int)(num_train * val_ratio);
    int train_size = num_train - val_size;

    printf("Vytváranie dátových množín:\n");
    printf("Veľkosť trénovacej množiny: %d\n", train_size);
    printf("Veľkosť validačnej množiny: %d\n", val_size);
    printf("Veľkosť testovacej množiny: %d\n", num_test);

    // Vytvorenie dátových množín
    *train_data = dataset_create(train_size, MNIST_IMAGE_SIZE, MNIST_NUM_CLASSES);
    *val_data = dataset_create(val_size, MNIST_IMAGE_SIZE, MNIST_NUM_CLASSES);
    *test_data = dataset_create(num_test, MNIST_IMAGE_SIZE, MNIST_NUM_CLASSES);

    if(!*train_data || !*val_data || !*test_data){
        printf("Nepodarilo sa vytvoriť dátové množiny\n");
        fclose(train_images);
        fclose(train_labels);
        fclose(test_images);
        fclose(test_labels);
        return -1;
    }

    printf("Úspešne vytvorené dátové množiny\n");

    // Načítanie trénovacích dát
    unsigned char image[MNIST_IMAGE_SIZE];
    unsigned char label;
    for(int i = 0; i < num_train; i++){
        // Načítanie obrázka a značky
        fread(image, 1, MNIST_IMAGE_SIZE, train_images);
        fread(&label, 1, 1, train_labels);

        // Určenie cieľovej množiny (trénovacia alebo validačná)
        Dataset* target = (i < train_size) ? *train_data : *val_data;
        int idx = (i < train_size) ? i : (i - train_size);

        // Normalizácia pixelov na rozsah [0,1] a one-hot kódovanie značiek
        for(int j = 0; j < MNIST_IMAGE_SIZE; j++){
            target->inputs[idx][j] = image[j] / 255.0f;
        }
        for(int j = 0; j < MNIST_NUM_CLASSES; j++){
            target->targets[idx][j] = (j == label) ? 1.0f : 0.0f;
        }
    }

    // Načítanie testovacích dát
    for(int i = 0; i < num_test; i++){
        // Načítanie obrázka a značky
        fread(image, 1, MNIST_IMAGE_SIZE, test_images);
        fread(&label, 1, 1, test_labels);

        // Normalizácia pixelov na rozsah [0,1] a one-hot kódovanie značiek
        for(int j = 0; j < MNIST_IMAGE_SIZE; j++){
            (*test_data)->inputs[i][j] = image[j] / 255.0f;
        }
        for(int j = 0; j < MNIST_NUM_CLASSES; j++){
            (*test_data)->targets[i][j] = (j == label) ? 1.0f : 0.0f;
        }
    }

    // Zatvorenie súborov
    fclose(train_images);
    fclose(train_labels);
    fclose(test_images);
    fclose(test_labels);

    printf("Úspešne načítané všetky MNIST dáta\n");
    return 0;
} 