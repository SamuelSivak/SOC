/*
 * ══════════════════════════════════════════════════════════════════════════════
 * MARKNET - FRONTEND APLIKÁCIA PRE NEURÓNOVU SIEŤ
 * ══════════════════════════════════════════════════════════════════════════════
 * 
 * Implementácia používateľského rozhrania pre rozpoznávanie ručne písaných číslic
 * pomocou neurónovej siete. Aplikácia poskytuje kresliacé plátno, real-time
 * predikcie a vizualizáciu výsledkov s podporou viacerých jazykov.
 * 
 * Technológie: HTML5 Canvas, JavaScript ES6, RESTful API
 * Backend: Node.js Express server s C neurónovou sieťou
 * 
 * Autor: Samuel Sivák  
 * Verzia: 1.0.0
 * ══════════════════════════════════════════════════════════════════════════════
 */

//=============================================================================
// KONFIGURAČNÉ KONŠTANTY
//=============================================================================

const CONFIG = {
    API_ENDPOINT: 'http://178.128.245.99:3000/api/predict',
    MODELS_ENDPOINT: 'http://178.128.245.99:3000/api/model',
    PREDICTION_DELAY: 500,           // ms delay pre real-time predikciu
    CANVAS_WIDTH: 280,               // šírka kresliacieho plátna
    CANVAS_HEIGHT: 280,              // výška kresliacieho plátna
    STROKE_WIDTH: 15,                // hrúbka čiary
    STROKE_COLOR: '#000000',         // farba čiary
    BACKGROUND_COLOR: '#ffffff'      // farba pozadia
};

//=============================================================================
// HLAVNÁ TRIEDA APLIKÁCIE
//=============================================================================

/*
 * Hlavná trieda pre správu neurónového rozpoznávacieho systému
 * Spravuje kresliacie plátno, API komunikáciu, jazykové prepínanie
 * a vizualizáciu výsledkov predikcie
 */
class NeuralNumbers{
    
    /*
     * Konštruktor - inicializácia aplikácie
     * Nastaví canvas, event listenery a načíta dostupné modely
     */
    constructor(){
        // DOM elementy a canvas kontext
        this.canvas = document.getElementById('drawingCanvas');
        this.ctx = this.canvas.getContext('2d');
        
        // Stav kreslenia
        this.isDrawing = false;
        this.lastX = 0;
        this.lastY = 0;
        
        // API konfigurácia
        this.apiEndpoint = CONFIG.API_ENDPOINT;
        this.modelsEndpoint = CONFIG.MODELS_ENDPOINT;
        
        // Predikčná logika
        this.realTimePrediction = true;
        this.predictionDelay = CONFIG.PREDICTION_DELAY;
        this.predictionTimeout = null;
        
        // Aplikačný stav
        this.currentModel = 'mnist_model.bin';
        this.currentLanguage = 'en';
        
        // Inicializácia komponentov
        this.setupCanvas();
        this.setupEventListeners();
        this.initializeConfidenceBars();
        this.loadAvailableModels();
        this.initializeLanguage();
    }

    //=========================================================================
    // INICIALIZAČNÉ METÓDY
    //=========================================================================

    /*
     * Inicializácia jazykového rozhrania
     * Nastaví predvolený jazyk na angličtinu
     */
    initializeLanguage(){
        this.switchLanguage('en');
    }

    /*
     * Konfigurácia kresliacieho plátna
     * Nastaví vlastnosti kreslenia a biely pozadí
     */
    setupCanvas(){
        // Nastavenie vlastností kreslenia
        this.ctx.strokeStyle = CONFIG.STROKE_COLOR;
        this.ctx.lineWidth = CONFIG.STROKE_WIDTH;
        this.ctx.lineCap = 'round';
        this.ctx.lineJoin = 'round';
        
        // Nastavenie bieleho pozadia
        this.ctx.fillStyle = CONFIG.BACKGROUND_COLOR;
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
    }

    /*
     * Nastavenie event listenerov pre interakciu s používateľom
     * Obsahuje podporu pre myš, dotyk a jazykové prepínanie
     */
    setupEventListeners(){
        // Myšové udalosti pre kreslenie
        this.canvas.addEventListener('mousedown', (e) => this.startDrawing(e));
        this.canvas.addEventListener('mousemove', (e) => this.draw(e));
        this.canvas.addEventListener('mouseup', () => this.stopDrawing());
        this.canvas.addEventListener('mouseout', () => this.stopDrawing());
        
        // Dotykové udalosti pre mobilné zariadenia
        this.canvas.addEventListener('touchstart', (e) => {
            e.preventDefault();
            const touch = e.touches[0];
            const mouseEvent = new MouseEvent('mousedown', {
                clientX: touch.clientX,
                clientY: touch.clientY
            });
            this.canvas.dispatchEvent(mouseEvent);
        });
        
        this.canvas.addEventListener('touchmove', (e) => {
            e.preventDefault();
            const touch = e.touches[0];
            const mouseEvent = new MouseEvent('mousemove', {
                clientX: touch.clientX,
                clientY: touch.clientY
            });
            this.canvas.dispatchEvent(mouseEvent);
        });
        
        this.canvas.addEventListener('touchend', (e) => {
            e.preventDefault();
            const mouseEvent = new MouseEvent('mouseup', {});
            this.canvas.dispatchEvent(mouseEvent);
        });

        // Jazykový prepínač
        document.querySelectorAll('.lang-btn').forEach(btn => {
            btn.addEventListener('click', (e) => this.switchLanguage(e.target.dataset.lang));
        });

        // Výber modelu
        document.getElementById('modelSelect').addEventListener('change', (e) => {
            this.switchModel(e.target.value);
        });
    }

    //=========================================================================
    // JAZYKOVÉ FUNKCIE
    //=========================================================================

    /*
     * Prepnutie jazyka rozhrania
     * Aktualizuje všetky textové elementy podľa vybraného jazyka
     * 
     * Parametre:
     * - lang: kód jazyka ('en' alebo 'sk')
     */
    switchLanguage(lang){
        if(lang === this.currentLanguage) return;
        
        this.currentLanguage = lang;
        
        // Aktualizácia aktívneho tlačidla jazyka
        document.querySelectorAll('.lang-btn').forEach(btn => {
            btn.classList.toggle('active', btn.dataset.lang === lang);
        });

        // Aktualizácia všetkých elementov s data atribútmi
        document.querySelectorAll('[data-en][data-sk]').forEach(element => {
            const text = element.getAttribute(`data-${lang}`);
            if(text){
                if(element.tagName === 'TITLE'){
                    element.textContent = text;
                } else if(element.tagName === 'OPTION'){
                    element.textContent = text;
                } else{
                    element.textContent = text;
                }
            }
        });

        // Aktualizácia jazyka stránky
        document.documentElement.lang = lang;

        // Aktualizácia stavových správ ak sú zobrazené
        const statusElement = document.getElementById('predictionResult');
        if(statusElement.textContent.includes('Draw a digit') || statusElement.textContent.includes('Nakreslite číslicu')){
            statusElement.textContent = lang === 'sk' ? 'Nakreslite číslicu pre začatie analýzy' : 'Draw a digit to begin';
        }

        // Aktualizácia loading textu v tlačidle
        const button = document.querySelector('.btn-predict');
        if(button.innerHTML.includes('Analyzing') || button.innerHTML.includes('Analyzujem')){
            const loadingText = lang === 'sk' ? 'Analyzujem...' : 'Analyzing...';
            button.innerHTML = `<span class="loading"></span> ${loadingText}`;
        }

        // Obnovenie zoznamu modelov s novým jazykom
        this.loadAvailableModels();

        console.log(`Jazyk prepnutý na: ${lang}`);
    }

    //=========================================================================
    // SPRÁVA MODELOV
    //=========================================================================

    /*
     * Načítanie zoznamu dostupných modelov z API
     * Naplní model selector s dostupnými možnosťami
     */
    async loadAvailableModels(){
        try{
            const response = await fetch(`${this.modelsEndpoint}/models`);
            if(!response.ok){
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            const data = await response.json();
            const modelSelect = document.getElementById('modelSelect');
            
            // Vyčistenie existujúcich možností
            modelSelect.innerHTML = '';

            // Pridanie modelov do selectu s jazykovým zobrazením
            for(const model of data.availableModels){
                const option = document.createElement('option');
                option.value = model.filename;
                
                /*
                 * Použitie viacjazyčných názvov z API
                 * Ak API poskytuje displayNames objekt, použije správny jazyk
                 * Inak fallback na pôvodný displayName
                 */
                let displayName = model.displayName; // Fallback pre backward compatibility
                let description = model.description;   // Fallback pre backward compatibility
                
                if(model.displayNames && model.displayNames[this.currentLanguage]){
                    displayName = model.displayNames[this.currentLanguage];
                }
                
                if(model.descriptions && model.descriptions[this.currentLanguage]){
                    description = model.descriptions[this.currentLanguage];
                }
                
                option.textContent = displayName;
                option.title = description;
                
                // Označenie nedostupných modelov
                if(!model.available){
                    option.disabled = true;
                    const trainingText = this.currentLanguage === 'sk' ? ' (Trénovanie...)' : ' (Training...)';
                    option.textContent += trainingText;
                }

                modelSelect.appendChild(option);
            }

            // Nastavenie aktuálneho modelu ako vybraný
            modelSelect.value = this.currentModel;

            console.log('Dostupné modely načítané:', data.availableModels.length);

        } catch(error){
            console.error('Chyba pri načítavaní modelov:', error);
            const warningText = this.currentLanguage === 'sk' ? 
                'Upozornenie: Nepodarilo sa načítať zoznam modelov' : 
                'Warning: Could not load model list';
            this.updateStatus(warningText);
        }
    }

    /*
     * Prepnutie na iný model neurónovej siete
     * Pošle požiadavku na server pre zmenu modelu
     * 
     * Parametre:
     * - modelFilename: názov súboru modelu
     */
    async switchModel(modelFilename){
        if(modelFilename === this.currentModel){
            return; // Už používame tento model
        }

        try{
            const statusText = this.currentLanguage === 'sk' ? 'Prepínanie modelu...' : 'Switching model...';
            const waitText = this.currentLanguage === 'sk' ? 'Prosím, čakajte...' : 'Please wait...';
            this.updateStatus(statusText, waitText);
            this.showLoading(true);

            const response = await fetch(`${this.modelsEndpoint}/switch`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    modelName: modelFilename
                })
            });

            if(!response.ok){
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            const result = await response.json();
            this.currentModel = modelFilename;
            
            const successText = this.currentLanguage === 'sk' ? 
                `Model prepnutý na: ${modelFilename}` : 
                `Model switched to: ${modelFilename}`;
            this.updateStatus(successText);
            
            console.log('Model úspešne prepnutý:', result);

        } catch(error){
            console.error('Chyba pri prepínaní modelu:', error);
            const errorText = this.currentLanguage === 'sk' ? 
                'Chyba pri prepínaní modelu' : 
                'Error switching model';
            this.updateStatus(errorText, error.message);
            
            // Obnovenie pôvodného výberu
            document.getElementById('modelSelect').value = this.currentModel;
        } finally{
            this.showLoading(false);
        }
    }

    //=========================================================================
    // KRESLIACIE FUNKCIE
    //=========================================================================

    /*
     * Inicializácia grafov spoľahlivosti pre všetky číslice 0-9
     * Nastaví všetky hodnoty na 0% a resetuje animácie
     */
    initializeConfidenceBars(){
        for(let i = 0; i < 10; i++){
            const bar = document.querySelector(`[data-digit="${i}"]`);
            if(bar){
                const percentage = bar.querySelector('.confidence-percentage');
                const progressFill = bar.querySelector('.progress-fill');
                
                percentage.textContent = '0%';
                progressFill.style.width = '0%';
                progressFill.style.backgroundColor = '#e0e0e0';
                bar.classList.remove('winner');
            }
        }
    }

    /*
     * Začatie kreslenia na plátno
     * Nastaví počiatočnú pozíciu a aktivuje režim kreslenia
     * 
     * Parametre:
     * - e: mouse event objekt
     */
    startDrawing(e){
        this.isDrawing = true;
        const rect = this.canvas.getBoundingClientRect();
        this.lastX = e.clientX - rect.left;
        this.lastY = e.clientY - rect.top;
    }

    /*
     * Kreslenie čiary na plátno počas pohybu myši
     * Vykreslí čiaru od poslednej pozície k aktuálnej
     * 
     * Parametre:
     * - e: mouse event objekt
     */
    draw(e){
        if(!this.isDrawing) return;

        const rect = this.canvas.getBoundingClientRect();
        const currentX = e.clientX - rect.left;
        const currentY = e.clientY - rect.top;

        this.ctx.beginPath();
        this.ctx.moveTo(this.lastX, this.lastY);
        this.ctx.lineTo(currentX, currentY);
        this.ctx.stroke();

        this.lastX = currentX;
        this.lastY = currentY;

        // Naplánovanie real-time predikcie
        if(this.realTimePrediction){
            this.scheduleRealTimePrediction();
        }
    }

    /*
     * Ukončenie kreslenia
     * Deaktivuje režim kreslenia a spustí finálnu predikciu
     */
    stopDrawing(){
        if(!this.isDrawing) return;
        this.isDrawing = false;
        
        // Spustenie finálnej predikcie po skončení kreslenia
        if(this.realTimePrediction){
            this.scheduleRealTimePrediction(100); // Kratší delay pre finálnu predikciu
        }
    }

    /*
     * Naplánovanie real-time predikcie s debouncing
     * Zabráni príliš častým volaniam API počas kreslenia
     * 
     * Parametre:
     * - delay: oneskorenie v milisekundách (predvolene z konfigurácie)
     */
    scheduleRealTimePrediction(delay = this.predictionDelay){
        // Zrušenie predchádzajúceho timeoutu
        if(this.predictionTimeout){
            clearTimeout(this.predictionTimeout);
        }
        
        // Naplánovanie novej predikcie
        this.predictionTimeout = setTimeout(() => {
            this.predict(true);
        }, delay);
    }

    /*
     * Vymazanie kresliacieho plátna
     * Resetuje plátno na biely pozadí a predikčné výsledky
     */
    clearCanvas(){
        // Vymazanie plátna
        this.ctx.fillStyle = CONFIG.BACKGROUND_COLOR;
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        
        // Reset predikčných výsledkov
        this.initializeConfidenceBars();
        
        // Reset stavových správ
        const defaultText = this.currentLanguage === 'sk' ? 
            'Nakreslite číslicu pre začatie analýzy' : 
            'Draw a digit to begin';
        this.updateStatus(defaultText);
        
        console.log('Plátno vymazané');
    }

    //=========================================================================
    // SPRACOVANIE OBRAZU A PREDIKCIA
    //=========================================================================

    /*
     * Zmena veľkosti obrazových dát na požadované rozmery
     * Používa bilineárnu interpoláciu pre kvalitné škálovanie
     * 
     * Parametre:
     * - imageData: pôvodné obrazové dáta
     * - width: cieľová šírka
     * - height: cieľová výška
     * 
     * Návratová hodnota: pole pixelov v novej veľkosti
     */
    resizeImageData(imageData, width, height){
        const canvas = document.createElement('canvas');
        const ctx = canvas.getContext('2d');
        canvas.width = width;
        canvas.height = height;
        
        // Bilineárna interpolácia pre lepšiu kvalitu
        ctx.imageSmoothingEnabled = true;
        ctx.imageSmoothingQuality = 'high';
        
        ctx.drawImage(this.canvas, 0, 0, width, height);
        return ctx.getImageData(0, 0, width, height);
    }

    /*
     * Predspracovanie obrazu pre neurónovú sieť
     * Konvertuje canvas na 28x28 grayscale a normalizuje hodnoty
     * 
     * Návratová hodnota: pole 784 normalizovaných pixelov (0.0-1.0)
     */
    preprocessImage(){
        // Získanie obrazových dát a zmena veľkosti na 28x28
        const resizedImageData = this.resizeImageData(this.canvas.getContext('2d').getImageData(0, 0, this.canvas.width, this.canvas.height), 28, 28);
        const pixels = [];
        
        // Konverzia RGBA na grayscale a normalizácia
        for(let i = 0; i < resizedImageData.data.length; i += 4){
            // Výpočet grayscale hodnoty z RGB
            const r = resizedImageData.data[i];
            const g = resizedImageData.data[i + 1];
            const b = resizedImageData.data[i + 2];
            const gray = (r + g + b) / 3;
            
            // Normalizácia na rozsah 0.0-1.0 (invertované pre MNIST)
            const normalized = 1.0 - (gray / 255.0);
            pixels.push(normalized);
        }
        
        return pixels;
    }

    /*
     * Vykonanie predikcie pomocí neurónovej siete
     * Pošle spracovaný obraz na API a zobrazí výsledky
     * 
     * Parametre:
     * - isRealTime: či ide o real-time predikciu (ovplyvňuje UI feedback)
     */
    async predict(isRealTime = false){
        try{
            // Kontrola či je niečo nakreslené
            const imageData = this.ctx.getImageData(0, 0, this.canvas.width, this.canvas.height);
            const hasContent = imageData.data.some((pixel, index) => {
                return index % 4 < 3 && pixel < 250; // RGB kanály, ignoruje alfa
            });

            if(!hasContent){
                const emptyText = this.currentLanguage === 'sk' ? 
                    'Nakreslite číslicu pre začatie analýzy' : 
                    'Draw a digit to begin';
                this.updateStatus(emptyText);
                this.initializeConfidenceBars();
                return;
            }

            // Zobrazenie loading stavu len pre manuálne predikcie
            if(!isRealTime){
                this.showLoading(true);
                const analyzingText = this.currentLanguage === 'sk' ? 'Analyzujem...' : 'Analyzing...';
                this.updateStatus(analyzingText);
            }

            // Predspracovanie obrazu
            const pixels = this.preprocessImage();

            // API volanie
            const response = await fetch(this.apiEndpoint, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    pixels: pixels,
                    options: {
                        normalize: true,
                        center: true,
                        threshold: 0.1
                    }
                })
            });

            if(!response.ok){
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            const result = await response.json();
            this.displayResults(result, isRealTime);

        } catch(error){
            console.error('Chyba pri predikcii:', error);
            const errorText = this.currentLanguage === 'sk' ? 
                'Chyba pri rozpoznávaní' : 
                'Recognition error';
            this.updateStatus(errorText, error.message);
        } finally{
            if(!isRealTime){
                this.showLoading(false);
            }
        }
    }

    //=========================================================================
    // ZOBRAZENIE VÝSLEDKOV
    //=========================================================================

    /*
     * Zobrazenie výsledkov predikcie v používateľskom rozhraní
     * Aktualizuje grafy spoľahlivosti a stavové správy
     * 
     * Parametre:
     * - result: objekt s výsledkami z API
     * - isRealTime: či ide o real-time aktualizáciu
     */
    displayResults(result, isRealTime = false){
        const prediction = result.prediction;
        const confidence = (result.confidence * 100).toFixed(1);
        const probabilities = result.probabilities || result.rawProbabilities;

        // Aktualizácia hlavného výsledku
        const resultText = this.currentLanguage === 'sk' ? 
            `Predikcia: ${prediction}` : 
            `Prediction: ${prediction}`;
        const confidenceText = this.currentLanguage === 'sk' ? 
            `Spoľahlivosť: ${confidence}%` : 
            `Confidence: ${confidence}%`;
        
        this.updateStatus(resultText, confidenceText);

        // Animácia grafov spoľahlivosti
        this.animateConfidenceBars(probabilities, prediction);

        // Logging pre debugging
        if(!isRealTime){
            console.log('Predikčný výsledok:', {
                digit: prediction,
                confidence: confidence + '%',
                responseTime: result.metadata?.responseTime
            });
        }
    }

    /*
     * Animácia grafov spoľahlivosti pre všetky číslice
     * Zobrazí pravdepodobnosti s farebnými indikátormi
     * 
     * Parametre:
     * - probabilities: pole pravdepodobností pre číslice 0-9
     * - winnerDigit: najviac pravdepodobná číslica
     */
    animateConfidenceBars(probabilities, winnerDigit){
        probabilities.forEach((prob, digit) => {
            const bar = document.querySelector(`[data-digit="${digit}"]`);
            if(!bar) return;

            const percentage = (prob * 100).toFixed(1);
            const percentageElement = bar.querySelector('.confidence-percentage');
            const progressFill = bar.querySelector('.progress-fill');

            // Aktualizácia textu a šírky
            percentageElement.textContent = percentage + '%';
            progressFill.style.width = percentage + '%';

            // Farebné rozlíšenie podľa hodnoty
            let color = '#e0e0e0'; // Predvolená šedá
            if(digit === winnerDigit){
                color = prob > 0.8 ? '#2ecc71' : prob > 0.5 ? '#f39c12' : '#e74c3c';
                bar.classList.add('winner');
            } else{
                bar.classList.remove('winner');
                if(prob > 0.1) color = '#3498db'; // Modrá pre významné pravdepodobnosti
            }

            progressFill.style.backgroundColor = color;
            
            // Animácia s miernym oneskorením pre vizuálny efekt
            setTimeout(() => {
                progressFill.style.transition = 'width 0.3s ease, background-color 0.3s ease';
            }, digit * 20);
        });
    }

    //=========================================================================
    // POMOCNÉ FUNKCIE UI
    //=========================================================================

    /*
     * Aktualizácia stavových správ v používateľskom rozhraní
     * 
     * Parametre:
     * - primary: hlavná stavová správa
     * - secondary: vedľajšia správa (voliteľné)
     */
    updateStatus(primary, secondary = ''){
        document.getElementById('predictionResult').textContent = primary;
        document.getElementById('predictionConfidence').textContent = secondary;
    }

    /*
     * Zobrazenie/skrytie loading animácie
     * 
     * Parametre:
     * - show: true pre zobrazenie, false pre skrytie
     */
    showLoading(show){
        const button = document.querySelector('.btn-predict');
        if(show){
            const loadingText = this.currentLanguage === 'sk' ? 'Analyzujem...' : 'Analyzing...';
            button.innerHTML = `<span class="loading"></span> ${loadingText}`;
            button.disabled = true;
        } else{
            const buttonText = this.currentLanguage === 'sk' ? 'Analyzovať' : 'Analyze';
            button.innerHTML = buttonText;
            button.disabled = false;
        }
    }
}

//=============================================================================
// GLOBÁLNE FUNKCIE PRE KOMPATIBILITU
//=============================================================================

/*
 * Globálna funkcia pre vymazanie plátna
 * Volá metódu clearCanvas() z hlavnej inštancie aplikácie
 */
function clearCanvas(){
    if(window.neuralApp) window.neuralApp.clearCanvas();
}

/*
 * Globálna funkcia pre spustenie predikcie
 * Volá metódu predict() z hlavnej inštancie aplikácie
 */
function predict(){
    if(window.neuralApp) window.neuralApp.predict();
}

//=============================================================================
// INICIALIZÁCIA APLIKÁCIE
//=============================================================================

/*
 * Spustenie aplikácie po načítaní DOM
 * Vytvorí globálnu inštanciu aplikácie
 */
document.addEventListener('DOMContentLoaded', () => {
    window.neuralApp = new NeuralNumbers();
    console.log('MarkNET aplikácia úspešne inicializovaná');
});
