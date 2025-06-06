# MarkNET - Neurálna sieť pre rozpoznávanie číslic

## Prehľad projektu

MarkNET predstavuje kompletnú implementáciu neuronovej siete pre rozpoznávanie ručne písaných číslic s využitím MNIST datasetu. Projekt kombinuje výkonnú implementáciu v jazyku C s moderným webovým rozhraním a poskytuje RESTful API pre integráciu s externými systémami.

**Autor:** Samuel Sivák  
**Rok:** 2025  
**Webová aplikácia:** http://samuelsivaksoc.xyz

## Architektúra systému

Systém sa skladá z troch hlavných komponentov:

1. **Jadro neuronovej siete** - implementované v jazyku C pre maximálny výkon
2. **Backend server** - Node.js aplikácia s RESTful API
3. **Frontend aplikácia** - webové rozhranie v HTML5/CSS3/JavaScript

## Štruktúra projektu

### Hlavné adresáre

```
├── README.md                    # Dokumentácia projektu
├── .gitignore                   # Git ignore konfigurácia
├── backend/                     # Backend server a API
├── bin/                         # Kompilované spustiteľné súbory
├── data/                        # MNIST dataset a preprocessované dáta
├── docs/                        # Projektová dokumentácia
├── examples/                    # Príklady použitia a testovanie
├── frontend/                    # Webová aplikácia
├── include/                     # Hlavičkové súbory (.h)
├── models/                      # Natrénované modely
└── src/                         # Zdrojové súbory (.c)
```

### Neurálna sieť - C implementácia

#### Hlavičkové súbory (include/)

| Súbor | Popis |
|-------|-------|
| `NeuronNetwork.h` | Hlavná štruktúra neuronovej siete a rozhrania |
| `Neuron.h` | Definícia jednotlivého neurónu |
| `Layer.h` | Implementácia vrstvy neurónov |
| `Matrix.h` | Maticové operácie a výpočty |
| `Activation.h` | Aktivačné funkcie (ReLU, Softmax, Sigmoid) |
| `Loss.h` | Funkcie straty (Cross-entropy, MSE) |
| `Optimizer.h` | Optimalizačné algoritmy (SGD, Adam) |
| `Data.h` | Správa dát a MNIST loader |
| `Evaluation.h` | Metriky hodnotenia výkonnosti |

#### Zdrojové súbory (src/)

| Súbor | Popis |
|-------|-------|
| `NeuronNetwork.c` | Hlavná implementácia neuronovej siete |
| `Neuron.c` | Implementácia neurónu a jeho funkcií |
| `Layer.c` | Implementácia vrstvy s forward/backward propagation |
| `Matrix.c` | Optimalizované maticové operácie |
| `Activation.c` | Implementácia aktivačných funkcií |
| `Loss.c` | Implementácia funkcií straty |
| `Optimizer.c` | Implementácia optimalizačných algoritmov |
| `Data.c` | Načítavanie a preprocessing MNIST dát |
| `Evaluation.c` | Hodnotenie a validácia modelu |

#### Príklady a testy (examples/)

| Súbor | Popis |
|-------|-------|
| `mnist_example.c` | Základný príklad použitia |
| `mnist_train_300.c` | Trénovanie na 300 vzorkách |
| `mnist_train_1500.c` | Trénovanie na 1500 vzorkách |
| `mnist_train_improved.c` | Vylepšené trénovanie s augmentáciou |
| `mnist_load_test.c` | Test načítavania modelov |

#### Natrénované modely (models/)

| Model | Veľkosť | Trénovacie vzorky | Presnosť |
|-------|---------|-------------------|----------|
| `mnist_model.bin` | 397KB | 60 000 | 96-98% |
| `mnist_model_300.bin` | 427KB | 300 | 85-90% |
| `mnist_model_1500.bin` | 696KB | 1 500 | 92-95% |
| `mnist_model_best.bin` | 918KB | 60 000 (optimalizovaný) | 97-98% |
| `mnist_model_final.bin` | 918KB | 60 000 (finálny) | 98% |

### Backend Server - Node.js

#### Hlavné súbory

| Súbor | Popis |
|-------|-------|
| `app.js` | Hlavný Express.js server |
| `index.js` | Vstupný bod aplikácie |
| `package.json` | NPM závislosti a konfigurácia |
| `package-lock.json` | Locked verzie závislostí |

#### API moduly

| Súbor | Popis |
|-------|-------|
| `health.js` | Health check endpoint |
| `predict.js` | Predikčné API |
| `model.js` | Správa modelov |
| `logger.js` | Logovanie aplikácie |
| `imageProcessor.js` | Spracovanie obrázkov |

#### C++ Wrapper

| Súbor | Popis |
|-------|-------|
| `binding.gyp` | Node-gyp build konfigurácia |
| `neural_network_wrapper.c` | C wrapper pre Node.js |

### Frontend aplikácia

#### Štruktúra

```
frontend/
├── index.html                   # Hlavná HTML stránka
├── assets/
│   └── main.css                # CSS štýly
└── js/
    ├── main.js                 # Hlavná aplikačná logika
    ├── canvas.js               # Canvas kreslenie
    ├── api.js                  # API komunikácia
    └── utils.js                # Pomocné funkcie
```

## Technické špecifikácie

### Neurálna sieť

- **Architektúra:** Multi-layer perceptron
- **Vstupná vrstva:** 784 neurónov (28x28 pixelov)
- **Skryté vrstvy:** Konfigurovateľné (štandardne 2-3 vrstvy)
- **Výstupná vrstva:** 10 neurónov (číslice 0-9)
- **Aktivačné funkcie:** ReLU, Softmax
- **Optimalizácia:** Gradient descent, Adam optimizer
- **Loss funkcia:** Cross-entropy

### Backend API

#### Endpointy

| Endpoint | Metóda | Popis |
|----------|---------|-------|
| `/api/health` | GET | Stav systému |
| `/api/predict` | POST | Predikcia číslic |
| `/api/model/switch` | POST | Prepnutie modelu |
| `/api/model/info` | GET | Informácie o modeli |

### Požiadavky na systém

#### Vývojové prostredie

- **Kompilátor:** GCC 7.0+ alebo Clang
- **Node.js:** v14.0+
- **NPM:** v6.0+
- **Operačný systém:** Linux, macOS, Windows (s WSL)

#### Produkčné nasadenie

- **Server:** Ubuntu 20.04+ LTS
- **Pamäť:** Minimum 2GB RAM
- **Procesor:** x86_64 architektura
- **Webserver:** Nginx (odporúčané)
## Licencia a autori
**Autor:** Samuel Sivák  
**Inštitúcia:** Stredná Odborná škola technická Michalovce.
**Rok:** 2025
## Changelog
### Verzia 1.7.3 (2025)
- Prvé vydanie projektu
- Kompletná implementácia neuronovej siete v C
- RESTful API backend
- Webové rozhranie s real-time kreslením
- 5 predtrénovaných modelov s rôznou presnosťou
