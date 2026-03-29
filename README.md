# MarkNET

Neurónová sieť pre rozpoznávanie ručne písaných číslic pomocou MNIST datasetu.

**Autor:** Samuel Sivák  
**Demo:** https://samuelsivaksoc.xyz

## Architektúra

- **Jadro:** C implementácia s optimalizovanými maticovými operáciami
- **Backend:** Node.js REST API s natívnymi C bindingami
- **Frontend:** HTML5 Canvas s real-time predikciami

Sieť: 784 → 256 → 128 → 10 (ReLU, Softmax, Cross-entropy, Adam)

## Štruktúra projektu

```
├── src/              # C implementácia neurónovej siete
├── include/          # Header súbory
├── examples/         # Trénovacie skripty
├── models/           # Natrénované modely (.bin)
├── backend/          # Node.js API server
└── frontend/         # Webové rozhraní
```

## Kompilácia

### C trénovacie programy

```bash
cd docs/config
make
```

Spustiteľné súbory v `bin/`:
- `mnist_train_300` - Rýchle trénovanie (300 vzoriek)
- `mnist_train_1500` - Stredné trénovanie (1500 vzoriek)
- `mnist_train_optimized` - Plné trénovanie (60k vzoriek)

### Backend server

```bash
cd backend
npm install
npm start
```

Na Windows vyžaduje Visual Studio Build Tools pre kompiláciu natívneho modulu.

## API Endpointy

- `POST /api/predict` - Predikcia číslice z 784 pixelov
- `GET /api/model/models` - Zoznam dostupných modelov
- `POST /api/model/switch` - Prepnutie aktívneho modelu

## Modely

| Súbor | Vzorky | Presnosť |
|-------|--------|----------|
| mnist_model_300.bin | 300 | ~85% |
| mnist_model_1500.bin | 1500 | ~92% |
| mnist_model_best.bin | 60000 | ~97% |

## Požiadavky

- GCC 7.0+
- Node.js 14+
- Python (pre node-gyp)
- Visual Studio Build Tools (Windows)

## Licencia

MIT
