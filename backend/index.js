// Node.js wrapper pre C neurónovú sieť

const neuralNetwork = require('./build/Release/neural_network.node');

class NeuralNetworkWrapper {
    constructor() {
        this.isInitialized = false;
    }

    // Inicializácia siete s modelom
    init(modelPath) {
        if(this.isInitialized) {
            this.cleanup();
        }
        
        const success = neuralNetwork.init(modelPath);
        this.isInitialized = success;
        return success;
    }

    // Predikcia na vstupných dátach (784 pixelov)
    predict(input) {
        if(!this.isInitialized) {
            throw new Error('Neural network not initialized. Call init() first.');
        }
        
        if(!Array.isArray(input) || input.length !== 784) {
            throw new Error('Input must be an array of 784 numbers (28x28 pixels)');
        }
        
        return neuralNetwork.predict(input);
    }

    // Získanie informácií o modeli
    getModelInfo() {
        return neuralNetwork.getModelInfo();
    }

    // Vyčistenie zdrojov
    cleanup() {
        if(this.isInitialized) {
            neuralNetwork.cleanup();
            this.isInitialized = false;
        }
    }
}

module.exports = new NeuralNetworkWrapper();
