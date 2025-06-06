{
  //===========================================================================
  // MARKNET - NODE.JS NATÍVNY MODUL BUILD KONFIGURÁCIA
  //===========================================================================
  //
  // GYP (Generate Your Projects) konfiguračný súbor pre kompiláciu
  // natívneho C/C++ modulu pre Node.js. Definuje zdrojové súbory,
  // závislosti, include paths a compiler flags potrebné pre build.
  //
  // Komponent: Node-API wrapper pre C neurónovú sieť
  // Účel: Bridge medzi JavaScript a C implementáciou MNIST rozpoznávania
  // Build systém: node-gyp
  //
  // Autor: Samuel Sivák
  // Verzia: 1.0.0
  //===========================================================================
  
  "targets": [{
    //=========================================================================
    // ZÁKLADNÁ KONFIGURÁCIA TARGETU
    //=========================================================================
    
    // Názov výsledného natívneho modulu (.node súbor)
    "target_name": "neural_network",
    
    //=========================================================================
    // ZDROJOVÉ SÚBORY
    //=========================================================================
    
    // Zoznam všetkých C zdrojových súborov ktoré sa majú skompilovať
    // Zahŕňa N-API wrapper a všetky komponenty neurónovej siete
    "sources": [ 
      "neural_network_wrapper.c",    // N-API JavaScript bridge
      "../../src/Activation.c",      // Aktivačné funkcie (ReLU, Sigmoid, Softmax)
      "../../src/Data.c",            // Dátové štruktúry a loading
      "../../src/Evaluation.c",      // Evaluačné metriky
      "../../src/Layer.c",           // Implementácia vrstiev neurónovej siete
      "../../src/Loss.c",            // Loss funkcie (Cross-entropy)
      "../../src/Matrix.c",          // Maticové operácie
      "../../src/Neuron.c",          // Implementácia jednotlivých neurónov
      "../../src/NeuronNetwork.c",   // Hlavná logika neurónovej siete
      "../../src/Optimizer.c"        // Optimalizačné algoritmy
    ],
    
    //=========================================================================
    // INCLUDE DIRECTORIES
    //=========================================================================
    
    // Adresáre pre header súbory (.h files)
    "include_dirs": [
      // Node-API headers (automaticky detekované)
      "<!@(node -p \"require('node-addon-api').include\")",
      // Lokálne header súbory projektu
      "../../include"
    ],
    
    //=========================================================================
    // ZÁVISLOSTI
    //=========================================================================
    
    // Externe závislosti potrebné pre build
    "dependencies": [
      // Node-API GYP konfigurácia
      "<!(node -p \"require('node-addon-api').gyp\")"
    ],
    
    //=========================================================================
    // COMPILER FLAGS A NASTAVENIA
    //=========================================================================
    
    // Vypnutie -fno-exceptions pre C kód (povolenie exceptions)
    "cflags!": [ "-fno-exceptions" ],
    "cflags_cc!": [ "-fno-exceptions" ],
    
    // Preprocessor definície
    "defines": [ 
      "NAPI_DISABLE_CPP_EXCEPTIONS"  // Vypnutie C++ exceptions v N-API
    ],
    
    // Linkované knižnice
    "libraries": [
      "-lm"  // Matematická knižnica (math.h funkcie)
    ]
  }]
}
