// Frontend aplikácia

const CONFIG = {
    API_ENDPOINT: 'https://samuelsivaksoc.xyz/api/predict',
    MODELS_ENDPOINT: 'https://samuelsivaksoc.xyz/api/model',
    PREDICTION_DELAY: 500,
    STROKE_WIDTH: 15
};

class NeuralNumbers{
    
    constructor(){
        this.canvas = document.getElementById('drawingCanvas');
        this.ctx = this.canvas.getContext('2d');
        this.isDrawing = false;
        this.lastX = 0;
        this.lastY = 0;
        this.apiEndpoint = CONFIG.API_ENDPOINT;
        this.modelsEndpoint = CONFIG.MODELS_ENDPOINT;
        this.predictionTimeout = null;
        this.currentModel = 'mnist_model.bin';
        this.currentLanguage = 'en';
        
        this.setupCanvas();
        this.setupEventListeners();
        this.initializeConfidenceBars();
        this.loadAvailableModels();
        this.switchLanguage('en');
    }

    setupCanvas(){
        this.ctx.strokeStyle = '#000000';
        this.ctx.lineWidth = CONFIG.STROKE_WIDTH;
        this.ctx.lineCap = 'round';
        this.ctx.lineJoin = 'round';
        this.ctx.fillStyle = '#ffffff';
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
    }

    setupEventListeners(){
        // Myš
        this.canvas.addEventListener('mousedown', (e) => this.startDrawing(e));
        this.canvas.addEventListener('mousemove', (e) => this.draw(e));
        this.canvas.addEventListener('mouseup', () => this.stopDrawing());
        this.canvas.addEventListener('mouseout', () => this.stopDrawing());
        
        // Touch
        this.canvas.addEventListener('touchstart', (e) => {
            e.preventDefault();
            const touch = e.touches[0];
            this.canvas.dispatchEvent(new MouseEvent('mousedown', {
                clientX: touch.clientX,
                clientY: touch.clientY
            }));
        });
        
        this.canvas.addEventListener('touchmove', (e) => {
            e.preventDefault();
            const touch = e.touches[0];
            this.canvas.dispatchEvent(new MouseEvent('mousemove', {
                clientX: touch.clientX,
                clientY: touch.clientY
            }));
        });
        
        this.canvas.addEventListener('touchend', (e) => {
            e.preventDefault();
            this.canvas.dispatchEvent(new MouseEvent('mouseup', {}));
        });

        // Jazyk
        document.querySelectorAll('.lang-btn').forEach(btn => {
            btn.addEventListener('click', (e) => this.switchLanguage(e.target.dataset.lang));
        });

        // Model selector
        document.getElementById('modelSelect').addEventListener('change', (e) => {
            this.switchModel(e.target.value);
        });
    }

    switchLanguage(lang){
        if(lang === this.currentLanguage) return;
        this.currentLanguage = lang;
        
        document.querySelectorAll('.lang-btn').forEach(btn => {
            btn.classList.toggle('active', btn.dataset.lang === lang);
        });

        document.querySelectorAll('[data-en][data-sk]').forEach(el => {
            const text = el.getAttribute(`data-${lang}`);
            if(text) el.textContent = text;
        });

        document.documentElement.lang = lang;
        this.loadAvailableModels();
    }

    // Načítanie modelov z backendu
    async loadAvailableModels(){
        try{
            const response = await fetch(`${this.modelsEndpoint}/models`);
            const data = await response.json();
            const select = document.getElementById('modelSelect');
            
            select.innerHTML = '';

            for(const model of data.availableModels){
                const option = document.createElement('option');
                option.value = model.filename;
                option.textContent = model.displayNames[this.currentLanguage] || model.filename;
                select.appendChild(option);
            }

            select.value = this.currentModel;

        } catch(error){
            console.error('Chyba pri načítavaní modelov:', error);
        }
    }

    // Prepnutie modelu
    async switchModel(modelFilename){
        if(modelFilename === this.currentModel) return;

        try{
            this.showLoading(true);

            const response = await fetch(`${this.modelsEndpoint}/switch`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ modelName: modelFilename })
            });

            if(!response.ok) throw new Error(`HTTP ${response.status}`);

            this.currentModel = modelFilename;
            
            const text = this.currentLanguage === 'sk' ? 
                `Model prepnutý: ${modelFilename}` : 
                `Model switched: ${modelFilename}`;
            this.updateStatus(text);

        } catch(error){
            console.error('Chyba pri prepínaní:', error);
            document.getElementById('modelSelect').value = this.currentModel;
        } finally{
            this.showLoading(false);
        }
    }

    initializeConfidenceBars(){
        for(let i = 0; i < 10; i++){
            const bar = document.querySelector(`[data-digit="${i}"]`);
            if(bar){
                bar.querySelector('.confidence-percentage').textContent = '0%';
                bar.querySelector('.progress-fill').style.width = '0%';
                bar.classList.remove('winner');
            }
        }
    }

    startDrawing(e){
        this.isDrawing = true;
        const rect = this.canvas.getBoundingClientRect();
        this.lastX = e.clientX - rect.left;
        this.lastY = e.clientY - rect.top;
    }

    draw(e){
        if(!this.isDrawing) return;
        
        const rect = this.canvas.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;
        
        this.ctx.beginPath();
        this.ctx.moveTo(this.lastX, this.lastY);
        this.ctx.lineTo(x, y);
        this.ctx.stroke();
        
        this.lastX = x;
        this.lastY = y;
        
        this.scheduleRealTimePrediction();
    }

    stopDrawing(){
        if(!this.isDrawing) return;
        this.isDrawing = false;
        this.scheduleRealTimePrediction(100);
    }

    scheduleRealTimePrediction(delay = CONFIG.PREDICTION_DELAY){
        if(this.predictionTimeout) clearTimeout(this.predictionTimeout);
        this.predictionTimeout = setTimeout(() => this.predict(true), delay);
    }

    clearCanvas(){
        this.ctx.fillStyle = '#ffffff';
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        this.initializeConfidenceBars();
        
        const text = this.currentLanguage === 'sk' ? 
            'Nakreslite číslicu' : 
            'Draw a digit';
        this.updateStatus(text);
    }

    // Zmena veľkosti obrazu
    resizeImageData(width, height){
        const canvas = document.createElement('canvas');
        const ctx = canvas.getContext('2d');
        canvas.width = width;
        canvas.height = height;
        ctx.imageSmoothingEnabled = true;
        ctx.drawImage(this.canvas, 0, 0, width, height);
        return ctx.getImageData(0, 0, width, height);
    }

    // Preprocessing obrazu
    preprocessImage(){
        const resized = this.resizeImageData(28, 28);
        const pixels = [];
        
        for(let i = 0; i < resized.data.length; i += 4){
            const gray = (resized.data[i] + resized.data[i + 1] + resized.data[i + 2]) / 3;
            pixels.push(1.0 - (gray / 255.0));
        }
        
        return pixels;
    }

    // Predikcia - posiela na backend
    async predict(isRealTime = false){
        try{
            const imageData = this.ctx.getImageData(0, 0, this.canvas.width, this.canvas.height);
            const hasContent = imageData.data.some((p, i) => i % 4 < 3 && p < 250);

            if(!hasContent){
                this.initializeConfidenceBars();
                return;
            }

            if(!isRealTime) this.showLoading(true);

            const pixels = this.preprocessImage();

            const response = await fetch(this.apiEndpoint, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ 
                    pixels: pixels,
                    options: { normalize: true, center: false, threshold: 0.0 }
                })
            });

            if(!response.ok) throw new Error(`HTTP ${response.status}`);

            const result = await response.json();
            this.displayResults(result);

        } catch(error){
            console.error('Chyba predikcie:', error);
        } finally{
            if(!isRealTime) this.showLoading(false);
        }
    }

    // Zobrazenie výsledkov z backendu
    displayResults(result){
        const digit = result.prediction;
        const conf = (result.confidence * 100).toFixed(1);
        
        const resultText = this.currentLanguage === 'sk' ? 
            `Predikcia: ${digit}` : `Prediction: ${digit}`;
        const confText = this.currentLanguage === 'sk' ? 
            `Spoľahlivosť: ${conf}%` : `Confidence: ${conf}%`;
        
        this.updateStatus(resultText, confText);
        this.animateConfidenceBars(result.probabilities, digit);
    }

    animateConfidenceBars(probs, winner){
        probs.forEach((prob, digit) => {
            const bar = document.querySelector(`[data-digit="${digit}"]`);
            if(!bar) return;

            const pct = (prob * 100).toFixed(1);
            bar.querySelector('.confidence-percentage').textContent = pct + '%';
            
            const fill = bar.querySelector('.progress-fill');
            fill.style.transform = `scaleY(${prob.toFixed(3)})`;
            fill.style.width = '100%';

            let color = '#e0e0e0';
            if(digit === winner){
                color = prob > 0.8 ? '#2ecc71' : prob > 0.5 ? '#f39c12' : '#e74c3c';
                bar.classList.add('winner');
            } else{
                bar.classList.remove('winner');
                if(prob > 0.1) color = '#3498db';
            }

            fill.style.backgroundColor = color;
            fill.style.transition = 'transform 0.35s ease, background-color 0.3s ease';
        });
    }

    updateStatus(primary, secondary = ''){
        document.getElementById('predictionResult').textContent = primary;
        document.getElementById('predictionConfidence').textContent = secondary;
    }

    showLoading(show){
        const btn = document.querySelector('.btn-predict');
        if(show){
            const text = this.currentLanguage === 'sk' ? 'Analyzujem...' : 'Analyzing...';
            btn.innerHTML = `<span class="loading"></span> ${text}`;
            btn.disabled = true;
        } else{
            btn.innerHTML = this.currentLanguage === 'sk' ? 'Analyzovať' : 'Analyze';
            btn.disabled = false;
        }
    }
}

// Globálne funkcie
function clearCanvas(){
    if(window.neuralApp) window.neuralApp.clearCanvas();
}

function predict(){
    if(window.neuralApp) window.neuralApp.predict();
}

// Inicializácia
document.addEventListener('DOMContentLoaded', () => {
    window.neuralApp = new NeuralNumbers();
});
