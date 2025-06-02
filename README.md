# Neurónová sieť v C

Implementácia viacvrstvovej neurónovej siete (MLP) v jazyku C s demonštráciou na MNIST datasete.

## Prehľad

Tento projekt implementuje:
- Plne prepojenú neurónovú sieť (MLP)
- Aktivačné funkcie (ReLU, Softmax)
- Stratové funkcie (MSE, Cross-entropy)
- Optimalizáciu pomocou gradientného zostupu
- Načítanie a spracovanie MNIST datasetu
- Ukladanie a načítavanie natrénovaných modelov

## Štruktúra projektu

```
.
├── include/             # Hlavičkové súbory
│   ├── Activation.h    # Aktivačné funkcie
│   ├── Data.h          # Správa dát a dataset
│   ├── Evaluation.h    # Vyhodnocovanie modelu
│   ├── Layer.h         # Vrstva neurónovej siete
│   ├── Loss.h          # Stratové funkcie
│   └── NeuralNetwork.h # Neurónová sieť
├── src/                # Zdrojové súbory
│   ├── Activation.c
│   ├── Data.c
│   ├── Evaluation.c
│   ├── Layer.c
│   ├── Loss.c
│   └── NeuralNetwork.c
├── examples/           # Príklady použitia
│   ├── mnist_example.c    # Trénovanie na MNIST
│   └── mnist_load_test.c  # Test načítaného modelu
├── data/              # MNIST dáta (nie sú súčasťou repozitára)
├── models/            # Uložené modely
└── Makefile
```

## Požiadavky

- GCC kompilátor
- GNU Make
- MNIST dataset (nie je súčasťou repozitára)

## Inštalácia

1. Naklonujte repozitár:
```bash
git clone https://github.com/yourusername/neural-network-c.git
cd neural-network-c
```

2. Stiahnite MNIST dataset a rozbaľte ho do adresára `data/`:
- train-images-idx3-ubyte
- train-labels-idx1-ubyte
- t10k-images-idx3-ubyte
- t10k-labels-idx1-ubyte

3. Skompilujte projekt:
```bash
make
```

## Použitie

### Trénovanie modelu

```bash
./bin/mnist_example
```

Tento príkaz:
1. Načíta MNIST dataset
2. Vytvorí a natrénuje neurónovú sieť
3. Vyhodnotí model na testovacej množine
4. Uloží natrénovaný model do `models/mnist_model.bin`

### Testovanie načítaného modelu

```bash
./bin/mnist_load_test
```

Tento príkaz:
1. Načíta uložený model
2. Otestuje ho na testovacej množine
3. Vypíše metriky výkonnosti

## Architektúra

### Neurónová sieť
- Viacvrstvová (MLP)
- Plne prepojené vrstvy
- Konfigurovateľný počet vrstiev a neurónov
- ReLU aktivácia v skrytých vrstvách
- Softmax aktivácia vo výstupnej vrstve

### Trénovanie
- Mini-batch gradientný zostup
- Cross-entropy stratová funkcia
- Voliteľná validačná množina
- Gradient clipping pre stabilitu

### Vyhodnocovanie
- Presnosť klasifikácie
- Confusion matrix
- Precision, Recall, F1-skóre
- Detailné štatistiky pre každú triedu

## Výkonnosť

Na MNIST datasete dosahuje model typicky:
- Trénovacia presnosť: ~98%
- Testovacia presnosť: ~92-93%
- Trénovací čas: ~2-3 minúty (CPU)
