/*
 * ══════════════════════════════════════════════════════════════════════════════
 * MARKNET - NODE.JS NAPI WRAPPER PRE C NEURÓNOVÚ SIEŤ
 * ══════════════════════════════════════════════════════════════════════════════
 * 
 * Natívny C modul pre Node.js ktorý poskytuje bridge medzi JavaScript
 * aplikáciou a C implementáciou neurónovej siete. Používa Node-API (N-API)
 * pre stabilné rozhranie nezávislé od verzie V8 JavaScript engine.
 * 
 * Funkcie:
 * - Načítanie a správa modelov neurónovej siete
 * - Forward propagation pre MNIST digit recognition
 * - Získanie metadát o modeli a architektúre
 * - Bezpečná správa pamäte a cleanup
 * 
 * API Endpointy:
 * - init(modelPath)     - Načítanie modelu zo súboru
 * - predict(pixels)     - Predikcia na základe 28x28 pixelov  
 * - getModelInfo()      - Informácie o načítanom modeli
 * - cleanup()           - Uvoľnenie zdrojov
 * 
 * Autor: Samuel Sivák
 * Verzia: 1.0.0
 * ══════════════════════════════════════════════════════════════════════════════
 */

//=============================================================================
// SYSTÉMOVÉ INCLUDES A ZÁVISLOSTI
//=============================================================================

#include <node_api.h>                    /* Node-API rozhranie */
#include <stdlib.h>                      /* Správa pamäte */
#include <string.h>                      /* Stringové operácie */
#include "../include/NeuronNetwork.h" /* Hlavičkový súbor neurónovej siete */
#include "../include/Data.h"          /* Dátové štruktúry */

//=============================================================================
// GLOBÁLNY STAV MODULU
//=============================================================================

/*
 * Globálny pointer na načítanú neurónovú sieť
 * NULL = žiadny model nie je načítaný
 * Non-NULL = aktívny model pripravený na predikcie
 */
static NeuralNetwork* nn = NULL;

//=============================================================================
// INICIALIZAČNÉ FUNKCIE
//=============================================================================

/*
 * init(modelPath) - Načítanie modelu neurónovej siete
 * 
 * Načíta natrénovaný model zo súborového systému a pripraví ho na použitie.
 * Ak už existuje načítaný model, najprv ho uvoľní z pamäte.
 * 
 * JavaScript signature: init(modelPath: string) => boolean
 * 
 * Parametre:
 * - env: N-API environment context
 * - info: callback info s argumentami
 * 
 * Argumenty:
 * - args[0]: string - cesta k súboru modelu
 * 
 * Návratová hodnota: boolean - true pri úspešnom načítaní, false pri chybe
 */
napi_value Init(napi_env env, napi_callback_info info){
    napi_status status;
    size_t argc = 1;
    napi_value args[1];
    
    // Získanie argumentov z JavaScript volania
    status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if(status != napi_ok) return NULL;

    // Extrakcia cesty k modelu ako UTF-8 string
    char modelPath[256];
    size_t str_len;
    status = napi_get_value_string_utf8(env, args[0], modelPath, 256, &str_len);
    if(status != napi_ok) return NULL;

    // Cleanup existujúceho modelu ak je načítaný
    if(nn != NULL){
        network_free(nn);
        nn = NULL;
    }
    
    // Načítanie nového modelu z súboru
    nn = network_load(modelPath);

    // Konverzia C boolean na JavaScript boolean
    napi_value result;
    napi_get_boolean(env, nn != NULL, &result);
    return result;
}

//=============================================================================
// PREDIKČNÉ FUNKCIE
//=============================================================================

/*
 * predict(pixels) - Vykonanie forward propagation
 * 
 * Vykoná predikciu číslice na základe vstupných pixelových dát.
 * Očakáva pole 784 float hodnôt reprezentujúcich 28x28 obrázok.
 * 
 * JavaScript signature: predict(pixels: number[]) => number[]
 * 
 * Parametre:
 * - env: N-API environment context
 * - info: callback info s argumentami
 * 
 * Argumenty:
 * - args[0]: Array<number> - pole 784 pixelových hodnôt (0.0-1.0)
 * 
 * Návratová hodnota: Array<number> - pole 10 pravdepodobností pre číslice 0-9
 */
napi_value Predict(napi_env env, napi_callback_info info){
    napi_status status;
    size_t argc = 1;
    napi_value args[1];
    
    // Získanie argumentov z JavaScript volania
    status = napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    if(status != napi_ok) return NULL;

    // Validácia že argument je pole
    bool is_array;
    status = napi_is_array(env, args[0], &is_array);
    if(status != napi_ok || !is_array) return NULL;

    // Kontrola správnej dĺžky pola (28x28 = 784 pixelov)
    uint32_t length;
    status = napi_get_array_length(env, args[0], &length);
    if(status != napi_ok || length != 784) return NULL;

    // Alokácia pamäte pre vstupné dáta
    float* input = malloc(784 * sizeof(float));
    if(input == NULL) return NULL;
    
    // Konverzia JavaScript pola na C float pole
    for(uint32_t i = 0; i < length; i++){
        napi_value element;
        status = napi_get_element(env, args[0], i, &element);
        if(status != napi_ok){
            free(input);
            return NULL;
        }
        
        double value;
        status = napi_get_value_double(env, element, &value);
        if(status != napi_ok){
            free(input);
            return NULL;
        }
        
        input[i] = (float)value;
    }

    // Vykonanie forward propagation cez neurónovú sieť
    network_forward(nn, input);
    float* predictions = nn->output_data;
    
    // Uvoľnenie vstupnej pamäte
    free(input);

    // Vytvorenie JavaScript pola pre výsledky
    napi_value result;
    status = napi_create_array_with_length(env, 10, &result);
    if(status != napi_ok) return NULL;

    // Kopírovanie výsledkov do JavaScript pola
    for(int i = 0; i < 10; i++){
        napi_value pred;
        status = napi_create_double(env, predictions[i], &pred);
        if(status != napi_ok) return NULL;
        
        status = napi_set_element(env, result, i, pred);
        if(status != napi_ok) return NULL;
    }

    return result;
}

//=============================================================================
// INFORMAČNÉ FUNKCIE
//=============================================================================

/*
 * getModelInfo() - Získanie metadát o modeli
 * 
 * Vráti informácie o aktuálne načítanom modeli vrátane stavu načítania
 * a architektúry neurónovej siete.
 * 
 * JavaScript signature: getModelInfo() => object
 * 
 * Parametre:
 * - env: N-API environment context
 * - info: callback info (bez argumentov)
 * 
 * Návratová hodnota: object - {loaded: boolean, numLayers?: number}
 */
napi_value GetModelInfo(napi_env env, napi_callback_info info){
    napi_status status;
    napi_value result;
    
    // Vytvorenie JavaScript objektu pre výsledok
    status = napi_create_object(env, &result);
    if(status != napi_ok) return NULL;

    // Ak nie je model načítaný, vráť len loaded: false
    if(nn == NULL){
        napi_value loaded;
        status = napi_get_boolean(env, false, &loaded);
        if(status != napi_ok) return NULL;
        
        status = napi_set_named_property(env, result, "loaded", loaded);
        if(status != napi_ok) return NULL;
        
        return result;
    }

    // Model je načítaný - vráť detailné informácie
    napi_value loaded;
    status = napi_get_boolean(env, true, &loaded);
    if(status != napi_ok) return NULL;
    status = napi_set_named_property(env, result, "loaded", loaded);
    if(status != napi_ok) return NULL;

    // Počet vrstiev v neurónovej sieti
    napi_value layers;
    status = napi_create_uint32(env, nn->num_layers, &layers);
    if(status != napi_ok) return NULL;
    status = napi_set_named_property(env, result, "numLayers", layers);
    if(status != napi_ok) return NULL;

    return result;
}

//=============================================================================
// SPRÁVA PAMÄTE A CLEANUP
//=============================================================================

/*
 * cleanup() - Uvoľnenie zdrojov neurónovej siete
 * 
 * Bezpečne uvoľní všetku pamäť alokovanú pre neurónovú sieť.
 * Táto funkcia by mala byť volaná pred ukončením aplikácie.
 * 
 * JavaScript signature: cleanup() => boolean
 * 
 * Parametre:
 * - env: N-API environment context
 * - info: callback info (bez argumentov)
 * 
 * Návratová hodnota: boolean - vždy true (úspešný cleanup)
 */
napi_value Cleanup(napi_env env, napi_callback_info info){
    // Uvoľnenie neurónovej siete ak je načítaná
    if(nn != NULL){
        network_free(nn);
        nn = NULL;
    }

    // Vráť potvrdenie úspešného cleanup
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

//=============================================================================
// N-API MODULE DEFINÍCIE
//=============================================================================

/*
 * Makro pre definíciu N-API metód
 * Zjednodušuje deklaráciu exportovaných funkcií
 */
#define DECLARE_NAPI_METHOD(name, func) \
    { name, 0, func, 0, 0, 0, napi_default, 0 }

/*
 * Init_Addon() - Inicializácia N-API modulu
 * 
 * Registruje všetky exportované funkcie ktoré budú dostupné v JavaScript.
 * Táto funkcia je automaticky volaná pri načítaní modulu.
 * 
 * Parametre:
 * - env: N-API environment context
 * - exports: exports objekt pre registráciu funkcií
 * 
 * Návratová hodnota: exports objekt s registrovanými funkciami
 */
napi_value Init_Addon(napi_env env, napi_value exports){
    napi_status status;
    
    /*
     * Definícia exportovaných funkcií
     * Každá funkcia má svoje JavaScript meno a C implementáciu
     */
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("init", Init),
        DECLARE_NAPI_METHOD("predict", Predict),
        DECLARE_NAPI_METHOD("getModelInfo", GetModelInfo),
        DECLARE_NAPI_METHOD("cleanup", Cleanup)
    };
    
    // Registrácia funkcií v exports objekte
    status = napi_define_properties(env, exports, 4, desc);
    if(status != napi_ok) return NULL;
    
    return exports;
}

/*
 * N-API module entry point
 * Registruje Init_Addon ako inicializačnú funkciu modulu
 */
NAPI_MODULE(NODE_GYP_MODULE_NAME, Init_Addon) 