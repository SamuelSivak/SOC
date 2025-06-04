/*
 * ══════════════════════════════════════════════════════════════════════════════
 * ROZLOŽENIE SÚBOROV PROJEKTU MARKNET - NEURÁLNA SIEŤ PRE ROZPOZNÁVANIE ČÍSLIC
 * ══════════════════════════════════════════════════════════════════════════════
 * 
 * Kompletný prehľad architektúry a štruktúry súborov v projekte MarkNET.
 * Systém obsahuje implementáciu neuronovej siete v jazyku C, backend servera
 * v Node.js a frontend aplikáciu s pokročilým používateľským rozhraním.
 * 
 * Projekt je nasadený na DigitalOcean serveri a dostupný na adrese:
 * http://samuelsivaksoc.xyz
 * 
 * Autor: Samuel Sivák
 * Dátum poslednej aktualizácie: 04.06.2025
 * ══════════════════════════════════════════════════════════════════════════════
 */

//=============================================================================
// 1. HLAVNÉ SÚBORY PROJEKTU
//=============================================================================

├── README.md                    // Hlavný popis projektu a inštalačné pokyny
├── run.txt                      // Kompletný manuál pre spustenie a správu
├── Makefile                     // Build konfigurácia pre C komponenty
├── PUBLIC_ACCESS.md             // Dokumentácia pre verejný prístup k aplikácii
├── server-setup.sh              // Automatizovaný setup script pre deployment
├── start-marknet.sh             // Script pre spustenie všetkých služieb
├── stop-marknet.sh              // Script pre zastavenie všetkých služieb
└── ROZLOZENIE.TXT              // Tento súbor - prehľad štruktúry projektu

//=============================================================================
// 2. NEURÁLNA SIEŤ - C IMPLEMENTÁCIA
//=============================================================================

/*
 * Jadro systému - implementácia neuronovej siete v jazyku C s podporou
 * multi-layer perceptron architektúry, ReLU a Softmax aktivačných funkcií,
 * cross-entropy loss function a mini-batch gradient descent optimalizácie.
 */

//-----------------------------------------------------------------------------
// 2.1 HLAVIČKOVÉ SÚBORY (include/)
//-----------------------------------------------------------------------------
include/
├── NeuronNetwork.h              // Hlavná štruktúra neuronovej siete
│                               // Deklarácie funkcií pre trénovanie a predikcie
├── Neuron.h                    // Definícia jednotlivého neurónu
│                               // Váhy, biasy a aktivačné funkcie
├── Layer.h                     // Implementácia vrstvy neurónov
│                               // Forward/backward propagation
├── Matrix.h                    // Maticové operácie pre efektívne výpočty
│                               // Násobenie matíc, transpozícia
├── Activation.h                // Aktivačné funkcie (ReLU, Softmax, Sigmoid)
│                               // Derivácie aktivačných funkcií
├── Loss.h                      // Loss funkcie (Cross-entropy, Mean Squared Error)
│                               // Výpočet chyby a jej derivácií
├── Optimizer.h                 // Optimalizačné algoritmy (SGD, Adam)
│                               // Aktualizácia váh a learning rate scheduling
├── Data.h                      // Štruktúry pre prácu s datasetmi
│                               // MNIST loader a data preprocessing
└── Evaluation.h                // Metriky hodnotenia (accuracy, precision, recall)
                                // Confusion matrix a validačné funkcie

//-----------------------------------------------------------------------------
// 2.2 IMPLEMENTAČNÉ SÚBORY (src/)
//-----------------------------------------------------------------------------
src/
├── NeuronNetwork.c             // Hlavná implementácia neuronovej siete
│                               // Forward/backward propagation, trénovanie
├── Neuron.c                    // Implementácia jednotlivého neurónu
│                               // Výpočty aktivácií a gradientov
├── Layer.c                     // Implementácia vrstvy neurónov
│                               // Batch processing a paralelizácia
├── Matrix.c                    // Optimalizované maticové operácie
│                               // SIMD inštrukcie a cache-friendly algoritmy
├── Activation.c                // Implementácia aktivačných funkcií
│                               // Stabilné softmax a leak ReLU varianty
├── Loss.c                      // Implementácia loss funkcií
│                               // Numericky stabilná cross-entropy
├── Optimizer.c                 // Implementácia optimalizačných algoritmov
│                               // Adaptive learning rate a momentum
├── Data.c                      // Načítavanie a preprocessing MNIST dát
│                               // Normalizácia, augmentácia a batching
└── Evaluation.c                // Hodnotenie výkonnosti modelu
                                // Cross-validation a test metrics

//-----------------------------------------------------------------------------
// 2.3 PRÍKLADY A TESTOVACÍ KÓD (examples/)
//-----------------------------------------------------------------------------
examples/
├── mnist_example.c             // Základný príklad použitia siete
├── mnist_train_300.c           // Trénovanie na 300 obrázkoch (rýchly test)
├── mnist_train_1500.c          // Trénovanie na 1500 obrázkoch (stredná presnosť)
├── mnist_train_improved.c      // Vylepšené trénovanie s augmentáciou
├── mnist_load_test.c           // Test načítavania uložených modelov
└── performance_benchmark.c     // Benchmarking rýchlosti predikcie

//-----------------------------------------------------------------------------
// 2.4 NATRÉNOVANÉ MODELY (models/)
//-----------------------------------------------------------------------------
models/
├── mnist_model.bin             // Štandardný model (60 000 obrázkov) - 397KB
├── mnist_model_300.bin         // Základný model (300 obrázkov) - 427KB
├── mnist_model_1500.bin        // Stredný model (1500 obrázkov) - 696KB
├── mnist_model_best.bin        // Najlepší model (optimalizovaný) - 918KB
└── mnist_model_final.bin       // Finálny model (najvyššia presnosť) - 918KB

//-----------------------------------------------------------------------------
// 2.5 KOMPILOVANÉ SÚBORY (bin/, obj/)
//-----------------------------------------------------------------------------
bin/                            // Spustiteľné súbory
├── mnist_train                 // Program pre trénovanie modelu
├── mnist_test                  // Program pre testovanie modelu
└── mnist_predict              // Program pre jednokratovú predikciu

obj/                            // Objektové súbory (.o)
├── NeuronNetwork.o
├── Layer.o
├── Neuron.o
└── [ďalšie .o súbory]

//=============================================================================
// 3. BACKEND SERVER - NODE.JS IMPLEMENTÁCIA
//=============================================================================

/*
 * Express.js backend server s C++ wrapper pre neurónovú sieť.
 * Poskytuje RESTful API pre predikcie, management modelov a monitorovanie.
 * Obsahuje pokročilé funkcie ako rate limiting, logging a error handling.
 */

//-----------------------------------------------------------------------------
// 3.1 HLAVNÉ SÚBORY BACKENDU
//-----------------------------------------------------------------------------
backend/
├── app.js                      // Hlavný Express.js aplikačný súbor
│                               // Middleware setup, routing, error handling
├── package.json                // Node.js závislosti a scripty
├── package-lock.json           // Locked verzie závislostí
└── backend.log                 // Log súbor pre runtime informácie

//-----------------------------------------------------------------------------
// 3.2 API ENDPOINTY (backend/api/)
//-----------------------------------------------------------------------------
backend/api/
├── health.js                   // GET /api/health - Health check endpoint
│                               // System metrics, uptime, memory usage
├── predict.js                  // POST /api/predict - Predikcia číslic
│                               // Spracovanie canvas dát a vrátenie výsledkov
├── model.js                    // GET /api/model/* - Management modelov
│                               // Informácie o modeloch, switching
└── batch.js                    // POST /api/predict/batch - Batch predikcie
                                // Spracovanie viacerých obrázkov naraz

//-----------------------------------------------------------------------------
// 3.3 NATÍVNY C++ WRAPPER (backend/native/)
//-----------------------------------------------------------------------------
backend/native/
├── neural_network.node         // Kompilovaný Node.js addon
│                               // Bridge medzi JavaScript a C kódom
├── binding.gyp                 // Node-gyp build konfigurácia
│                               // Compile settings pre C++ wrapper
├── neural_network.cpp          // C++ wrapper implementácia
│                               // N-API bindings pre JavaScript
└── build/                      // Dočasné build súbory
    ├── Release/
    │   └── neural_network.node
    └── Makefile

//-----------------------------------------------------------------------------
// 3.4 POMOCNÉ MODULY (backend/utils/)
//-----------------------------------------------------------------------------
backend/utils/
├── logger.js                   // Winston logger konfigurácia
│                               // Structured logging, rotation, levels
├── imageProcessor.js           // Spracovanie obrázkov z canvasu
│                               // Resize, normalizácia, threshold
├── validator.js                // Input validácia a sanitizácia
│                               // Security middleware, XSS protection
└── cache.js                    // Redis cache implementácia
                                // Cachovanie frequent predikcií

//-----------------------------------------------------------------------------
// 3.5 LOG SÚBORY (backend/logs/)
//-----------------------------------------------------------------------------
backend/logs/
├── app.log                     // Aplikačné logy (INFO, WARN, ERROR)
├── error.log                   // Iba error logy pre debugging
├── access.log                  // HTTP access logy (Morgan format)
└── debug.log                   // Debug informácie pre development

//-----------------------------------------------------------------------------
// 3.6 NODE.JS ZÁVISLOSTI (backend/node_modules/)
//-----------------------------------------------------------------------------
backend/node_modules/           // NPM packages (800+ súborov)
├── express/                    // Web framework
├── cors/                       // Cross-origin resource sharing
├── winston/                    // Professional logging library
├── node-addon-api/             // C++ addon development
├── multer/                     // File upload handling
├── helmet/                     // Security middleware
├── express-rate-limit/         // Rate limiting middleware
└── [600+ ďalších packages]

//=============================================================================
// 4. FRONTEND APLIKÁCIA - HTML5/CSS3/JAVASCRIPT
//=============================================================================

/*
 * Moderná responzívna webová aplikácia s real-time kreslením na canvas,
 * animovanými confidence bars, prepínaním jazykov (EN/SK) a dynamickým
 * switching medzi rôznymi AI modelmi.
 */

//-----------------------------------------------------------------------------
// 4.1 HLAVNÉ SÚBORY FRONTENDU
//-----------------------------------------------------------------------------
frontend/
├── index.html                  // Hlavná HTML stránka s kompletným UI
│                               // Canvas, confidence bars, model selector
└── favicon.ico                 // Ikona webu (bude pridaná)

//-----------------------------------------------------------------------------
// 4.2 ŠTÝLY A DIZAJN (frontend/assets/)
//-----------------------------------------------------------------------------
frontend/assets/
└── main.css                    // Kompletné CSS štýly
    ├── /* Global Styles */     // Reset, typography, variables
    ├── /* Header Navigation */ // Fixed header, logo, language switcher
    ├── /* Canvas Section */    // Drawing area, controls, instructions
    ├── /* Results Section */   // Confidence bars grid, animations
    ├── /* Experiments */       // Additional features section
    ├── /* Responsive Design */ // Mobile-first approach, breakpoints
    └── /* Animations */        // Smooth transitions, hover effects

//-----------------------------------------------------------------------------
// 4.3 JAVASCRIPT MODULY (frontend/js/)
//-----------------------------------------------------------------------------
frontend/js/
├── main.js                     // Hlavný application controller
│                               // NeuralNumbers class, initialization
├── Canvas.js                   // Canvas drawing implementácia
│                               // Mouse/touch events, drawing logic
├── Controls.js                 // UI controls management
│                               // Buttons, model selector, language switcher
├── Results.js                  // Results visualization
│                               // Confidence bars animations, chart updates
├── api.js                      // API komunikácia s backendom
│                               // HTTP requests, error handling
└── utils.js                    // Utility funkcie
                                // Image processing, validation, helpers

//=============================================================================
// 5. DÁTA A DATASETY
//=============================================================================

/*
 * MNIST dataset a pomocné súbory pre trénovanie a testovanie neuronovej siete.
 * Obsahuje originálne MNIST súbory a preprocessing scripts.
 */

//-----------------------------------------------------------------------------
// 5.1 MNIST DATASET (data/)
//-----------------------------------------------------------------------------
data/
├── mnist/
│   ├── train-images-idx3-ubyte // 60 000 trénovacích obrázkov (47MB)
│   ├── train-labels-idx1-ubyte // 60 000 trénovacích labelov (60KB)
│   ├── t10k-images-idx3-ubyte  // 10 000 testovacích obrázkov (7.8MB)
│   └── t10k-labels-idx1-ubyte  // 10 000 testovacích labelov (10KB)
├── preprocessed/
│   ├── train_data_300.bin      // Preprocessované trénovacie dáta (300)
│   ├── train_data_1500.bin     // Preprocessované trénovacie dáta (1500)
│   └── test_data.bin           // Preprocessované testovacie dáta
└── validation/
    ├── accuracy_reports.txt    // Výsledky testovania presnosti
    ├── confusion_matrix.csv    // Confusion matrix pre každý model
    └── performance_metrics.json // JSON s detailnými metrikami

//=============================================================================
// 6. DOKUMENTÁCIA A KONFIGURÁCIA
//=============================================================================

/*
 * Súbory dokumentácie, konfigurácie a deployment scripts pre kompletný
 * life-cycle management projektu od development po production deployment.
 */

//-----------------------------------------------------------------------------
// 6.1 DOKUMENTÁCIA
//-----------------------------------------------------------------------------
docs/
├── API_DOCUMENTATION.md        // Kompletná REST API dokumentácia
├── NEURAL_NETWORK_DESIGN.md    // Technická dokumentácia siete
├── DEPLOYMENT_GUIDE.md         // Guide pre production deployment
├── DEVELOPMENT_SETUP.md        // Setup pre local development
├── PERFORMANCE_ANALYSIS.md     // Analýza výkonnosti a optimalizácie
└── USER_MANUAL.md              // Používateľský manuál

//-----------------------------------------------------------------------------
// 6.2 DEPLOYMENT A INFRAŠTRUKTÚRA
//-----------------------------------------------------------------------------
deployment/
├── nginx/
│   ├── sites-available/
│   │   └── samuelsivaksoc      // Nginx konfigurácia pre domain
│   └── ssl/                    // SSL certifikáty (budúce použitie)
├── systemd/
│   ├── marknet-backend.service // Systemd service pre backend
│   └── marknet-frontend.service // Systemd service pre frontend
└── monitoring/
    ├── healthcheck.sh          // Health monitoring script
    └── backup.sh               // Automatické backup script

//-----------------------------------------------------------------------------
// 6.3 BEZPEČNOSŤ A KONFIGURÁCIA
//-----------------------------------------------------------------------------
config/
├── .env.example                // Príklad environment variables
├── .gitignore                  // Git ignore rules
├── security/
│   ├── rate-limits.json        // Rate limiting konfigurácia
│   ├── cors-policy.json        // CORS policy nastavenia
│   └── input-validation.json   // Input validation rules
└── logging/
    ├── winston.config.js       // Winston logger konfigurácia
    └── log-rotation.conf       // Log rotation nastavenia

//=============================================================================
// 7. TESTING A QUALITY ASSURANCE
//=============================================================================

/*
 * Komprehensívny testing framework pre všetky komponenty systému
 * s unit testami, integration testami a performance benchmarkmi.
 */

//-----------------------------------------------------------------------------
// 7.1 C TESTY
//-----------------------------------------------------------------------------
tests/
├── c_tests/
│   ├── test_neural_network.c   // Unit testy pre neurónovú sieť
│   ├── test_matrix.c           // Testy maticových operácií
│   ├── test_activation.c       // Testy aktivačných funkcií
│   ├── test_optimizer.c        // Testy optimalizačných algoritmov
│   └── run_tests.sh            // Script pre spustenie všetkých testov
├── integration/
│   ├── test_api_endpoints.js   // Integration testy API
│   ├── test_model_loading.js   // Testy načítavania modelov
│   └── test_prediction_flow.js // End-to-end prediction tests
└── performance/
    ├── benchmark_training.c    // Performance testy trénovania
    ├── benchmark_prediction.c  // Performance testy predikcií
    └── memory_profiling.c      // Memory leak detection

//=============================================================================
// 8. ŠTATISTIKY A METRIKY PROJEKTU
//=============================================================================

/*
 * ╔══════════════════════════════════════════════════════════════════════════╗
 * ║                          FINÁLNE ŠTATISTIKY                             ║
 * ╠══════════════════════════════════════════════════════════════════════════╣
 * ║ Celkový počet súborov: 2000+                                            ║
 * ║ Riadky kódu (C):       ~8 000 LOC                                       ║
 * ║ Riadky kódu (JS):      ~3 000 LOC                                       ║
 * ║ Riadky kódu (CSS):     ~1 500 LOC                                       ║
 * ║ Riadky kódu (HTML):    ~500 LOC                                         ║
 * ║                                                                          ║
 * ║ Backend dependencies:   50+ NPM packages                                ║
 * ║ Natrénované modely:     5 modelov (2.5MB celkom)                        ║
 * ║ Dataset veľkosť:        ~55MB (MNIST)                                    ║
 * ║                                                                          ║
 * ║ Nasadenie:             DigitalOcean Ubuntu 22.04                        ║
 * ║ Doména:                http://samuelsivaksoc.xyz                         ║
 * ║ Architektúra:          Nginx + Node.js + C neural engine               ║
 * ║ Jazyky:                C, JavaScript, HTML5, CSS3                       ║
 * ║ Podpora jazykov:       Angličtina, Slovenčina                          ║
 * ║ Responzívnosť:         Desktop, Tablet, Mobile                          ║
 * ╚══════════════════════════════════════════════════════════════════════════╝
 */

//=============================================================================
// 9. TECHNOLOGICKÝ STACK
//=============================================================================

/*
 * Frontend Technologies:
 * ├── HTML5 Canvas API          // Real-time drawing interface
 * ├── CSS3 Grid & Flexbox       // Modern responsive layout
 * ├── Vanilla JavaScript ES6+   // No frameworks, pure performance
 * ├── CSS Animations            // Smooth confidence bar animations
 * └── Touch Events API          // Mobile device support
 * 
 * Backend Technologies:
 * ├── Node.js v18+              // JavaScript runtime
 * ├── Express.js v4.18          // Web application framework
 * ├── Winston                   // Professional logging
 * ├── CORS                      // Cross-origin resource sharing
 * ├── Express Rate Limit        // DDoS protection
 * └── Node-API (N-API)          // C++ addon interface
 * 
 * Core Neural Network:
 * ├── Pure C Implementation     // Maximum performance
 * ├── SIMD Optimizations        // Vectorized math operations
 * ├── Cache-Friendly Algorithms // Optimized memory access
 * ├── Multi-threading Support   // Parallel processing
 * └── Binary Model Format       // Fast loading/saving
 * 
 * Infrastructure:
 * ├── DigitalOcean Droplet      // Cloud hosting (Ubuntu 22.04)
 * ├── Nginx Reverse Proxy       // Load balancing & SSL termination
 * ├── Systemd Services          // Process management
 * ├── Domain Name System        // samuelsivaksoc.xyz
 * └── Git Version Control       // Source code management
 */

//=============================================================================
// 10. ZÁVER A POĎAKOVANIE
//=============================================================================

/*
 * ╔══════════════════════════════════════════════════════════════════════════╗
 * ║                            PROJEKT MARKNET                              ║
 * ║                    NEURÁLNA SIEŤ PRE ROZPOZNÁVANIE ČÍSLIC               ║
 * ╠══════════════════════════════════════════════════════════════════════════╣
 * ║                                                                          ║
 * ║ Projekt predstavuje kompletný end-to-end systém pre rozpoznávanie        ║
 * ║ ručne písaných číslic pomocí deep learning technológií. Kombinuje       ║
 * ║ vysokú výkonnosť C implementácie s moderným webovým rozhraním a          ║
 * ║ profesionálnym backend API.                                             ║
 * ║                                                                          ║
 * ║ Hlavné vlastnosti:                                                      ║
 * ║ • Real-time rozpoznávanie s 92-98% presnosťou                          ║
 * ║ • 5 rôznych AI modelov s rôznymi úrovňami presnosti                    ║
 * ║ • Bilingválna podpora (Angličtina/Slovenčina)                          ║
 * ║ • Responzívny dizajn pre všetky zariadenia                             ║
 * ║ • Production-ready deployment na cloud infraštruktúre                  ║
 * ║ • RESTful API pre integráciu s externými systémami                     ║
 * ║ • Komprehensívne logovanie a monitorovanie                             ║
 * ║                                                                          ║
 * ║ Dostupné na: http://samuelsivaksoc.xyz                                  ║
 * ║                                                                          ║
 * ║ Autor: Samuel Sivák                                                     ║
 * ║ Rok:2025                                                                    ║                                                                            ║
 * ╚══════════════════════════════════════════════════════════════════════════╝
 */
