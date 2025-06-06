/*
 * ══════════════════════════════════════════════════════════════════════════════
 * MARKNET - NODEJS WRAPPER PRE C NEURÓNOVÚ SIEŤ
 * ══════════════════════════════════════════════════════════════════════════════
 * 
 * JavaScript wrapper trieda pre natívny C modul neurónovej siete.
 * Poskytuje vysokoúrovňové rozhranie pre Node.js aplikácie na prácu
 * s MNIST digit recognition neurónovou sieťou implementovanou v C.
 * 
 * Funkcie:
 * - Inicializácia a cleanup neurónovej siete
 * - Predikcia na základe 28x28 pixelových dát
 * - Získanie informácií o modeli
 * - Error handling a validácia vstupov
 * 
 * Autor: Samuel Sivák
 * Verzia: 1.0.0
 * ══════════════════════════════════════════════════════════════════════════════
 */

//=============================================================================
// IMPORTY A ZÁVISLOSTI
//=============================================================================

const neuralNetwork = require('./build/Release/neural_network.node');

//=============================================================================
// NEURAL NETWORK WRAPPER TRIEDA
//=============================================================================

/*
 * Wrapper trieda pre C neurónovu sieť
 * Spravuje stav inicializácie a poskytuje bezpečné rozhranie
 * pre komunikáciu s natívnym C modulom
 */
class NeuralNetworkWrapper {
    
    /*
     * Konštruktor - inicializácia wrapper objektu
     * Nastaví počiatočný stav na neinicializovaný
     */
    constructor() {
        this.isInitialized = false;
    }

    /*
     * Inicializácia neurónovej siete s modelom
     * Načíta model zo súboru a pripraví sieť na predikcie
     * 
     * Parametre:
     * - modelPath: cesta k súboru s natrénovaným modelom
     * 
     * Návratová hodnota: true ak úspešné, false ak neúspešné
     */
    init(modelPath) {
        // Vyčistenie predchádzajúceho modelu ak existuje
        if(this.isInitialized) {
            this.cleanup();
        }
        
        // Inicializácia natívneho modulu
        const success = neuralNetwork.init(modelPath);
        this.isInitialized = success;
        return success;
    }

    /*
     * Vykonanie predikcie na vstupných dátach
     * Spracuje 28x28 pixelové dáta a vráti pravdepodobnosti pre číslice 0-9
     * 
     * Parametre:
     * - input: pole 784 čísel reprezentujúcich 28x28 pixelov
     * 
     * Návratová hodnota: pole 10 pravdepodobností pre číslice 0-9
     */
    predict(input) {
        // Kontrola inicializácie
        if(!this.isInitialized) {
            throw new Error('Neural network not initialized. Call init() first.');
        }
        
        // Validácia vstupných dát
        if(!Array.isArray(input) || input.length !== 784) {
            throw new Error('Input must be an array of 784 numbers (28x28 pixels)');
        }
        
        // Volanie natívnej predikčnej funkcie
        return neuralNetwork.predict(input);
    }

    /*
     * Získanie informácií o aktuálne načítanom modeli
     * Vráti metadáta o architektúre a stave neurónovej siete
     * 
     * Návratová hodnota: objekt s informáciami o modeli
     */
    getModelInfo() {
        return neuralNetwork.getModelInfo();
    }

    /*
     * Vyčistenie a uvoľnenie zdrojov neurónovej siete
     * Uvoľní pamäť alokovanú natívnym modulom
     */
    cleanup() {
        if(this.isInitialized) {
            neuralNetwork.cleanup();
            this.isInitialized = false;
        }
    }
}

//=============================================================================
// EXPORT SINGLETON INŠTANCIE
//=============================================================================

/*
 * Export singleton inštancie wrapper triedy
 * Zabezpečuje, že v celej aplikácii sa používa jedna inštancia
 */
module.exports = new NeuralNetworkWrapper(); 