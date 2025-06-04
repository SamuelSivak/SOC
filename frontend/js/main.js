// MarkNET - Neural Numbers Application
class NeuralNumbers{
    constructor(){
        this.canvas = document.getElementById('drawingCanvas');
        this.ctx = this.canvas.getContext('2d');
        this.isDrawing = false;
        this.lastX = 0;
        this.lastY = 0;
        this.apiEndpoint = 'http://178.128.245.99:3000/api/predict';
        this.modelsEndpoint = 'http://178.128.245.99:3000/api/model';
        this.realTimePrediction = true;
        this.predictionDelay = 500; // ms delay for real-time prediction
        this.predictionTimeout = null;
        this.currentModel = 'mnist_model.bin';
        this.currentLanguage = 'en';
        
        this.setupCanvas();
        this.setupEventListeners();
        this.initializeConfidenceBars();
        this.loadAvailableModels();
        this.initializeLanguage();
    }

    initializeLanguage(){
        // Set initial language
        this.switchLanguage('en');
    }

    setupCanvas(){
        // Set up canvas drawing properties
        this.ctx.strokeStyle = '#000000';
        this.ctx.lineWidth = 15;
        this.ctx.lineCap = 'round';
        this.ctx.lineJoin = 'round';
        
        // Set white background
        this.ctx.fillStyle = '#ffffff';
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
    }

    setupEventListeners(){
        // Mouse events
        this.canvas.addEventListener('mousedown', (e) => this.startDrawing(e));
        this.canvas.addEventListener('mousemove', (e) => this.draw(e));
        this.canvas.addEventListener('mouseup', () => this.stopDrawing());
        this.canvas.addEventListener('mouseout', () => this.stopDrawing());
        
        // Touch events for mobile
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

        // Language switcher
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
        
        // Update active language button
        document.querySelectorAll('.lang-btn').forEach(btn => {
            btn.classList.toggle('active', btn.dataset.lang === lang);
        });

        // Update all elements with data attributes
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

        // Update page language attribute
        document.documentElement.lang = lang;

        // Update status messages if currently displayed
        const statusElement = document.getElementById('predictionResult');
        if(statusElement.textContent.includes('Draw a digit') || statusElement.textContent.includes('Nakreslite číslicu')){
            statusElement.textContent = lang === 'sk' ? 'Nakreslite číslicu pre začatie analýzy' : 'Draw a digit to begin';
        }

        // Update loading text in button if currently loading
        const button = document.querySelector('.btn-predict');
        if(button.innerHTML.includes('Analyzing') || button.innerHTML.includes('Analyzujem')){
            const loadingText = lang === 'sk' ? 'Analyzujem...' : 'Analyzing...';
            button.innerHTML = `<span class="loading"></span> ${loadingText}`;
        }

        console.log(`Language switched to: ${lang}`);
    }

    async loadAvailableModels(){
        try{
            const response = await fetch(`${this.modelsEndpoint}/models`);
            if(!response.ok){
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            const data = await response.json();
            const modelSelect = document.getElementById('modelSelect');
            
            // Clear existing options
            modelSelect.innerHTML = '';

            // Add models to selector with language-aware display names
            for(const model of data.availableModels){
                const option = document.createElement('option');
                option.value = model.filename;
                
                // Create language-aware display names
                let displayName = model.displayName;
                if(this.currentLanguage === 'sk'){
                    if(model.filename.includes('300')){
                        displayName = 'Základný (300 obrázkov)';
                    } else if(model.filename.includes('1500')){
                        displayName = 'Pokročilý (1500 obrázkov)';
                    } else if(model.filename.includes('final')){
                        displayName = 'Finálny model (60 000 obrázkov)';
                    } else if(model.filename.includes('best')){
                        displayName = 'Najlepší model (60 000 obrázkov)';
                    } else{
                        displayName = 'Štandardný model (60 000 obrázkov)';
                    }
                }
                
                option.textContent = displayName;
                option.title = model.description;
                
                if(!model.available){
                    option.disabled = true;
                    const trainingText = this.currentLanguage === 'sk' ? ' (Trénovanie...)' : ' (Training...)';
                    option.textContent += trainingText;
                }

                modelSelect.appendChild(option);
            }

            // Set current model as selected
            modelSelect.value = this.currentModel;

            console.log('Available models loaded:', data.availableModels.length);

        } catch(error){
            console.error('Failed to load available models:', error);
            const warningText = this.currentLanguage === 'sk' ? 
                'Upozornenie: Nepodarilo sa načítať zoznam modelov' : 
                'Warning: Could not load model list';
            this.updateStatus(warningText);
        }
    }

    async switchModel(modelFilename){
        if(modelFilename === this.currentModel){
            return; // Already using this model
        }

        try{
            const statusText = this.currentLanguage === 'sk' ? 'Prepínanie modelu...' : 'Switching model...';
            const waitText = this.currentLanguage === 'sk' ? 'Prosím, čakajte...' : 'Please wait...';
            this.updateStatus(statusText, waitText);
            this.showLoading(true);

            const response = await fetch(`${this.modelsEndpoint}/switch`, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    modelName: modelFilename
                })
            });

            if(!response.ok){
                const errorData = await response.json();
                throw new Error(errorData.message || `HTTP ${response.status}`);
            }

            const result = await response.json();
            
            if(result.success){
                this.currentModel = modelFilename;
                const modelSelect = document.getElementById('modelSelect');
                const selectedOption = modelSelect.selectedOptions[0];
                
                const successText = this.currentLanguage === 'sk' ? 
                    `Model prepnutý: ${selectedOption.textContent}` :
                    `Model switched: ${selectedOption.textContent}`;
                
                const readyText = this.currentLanguage === 'sk' ? 
                    `Vrstvy: ${result.modelInfo.numLayers}, Pripravený na predikciu` :
                    `Layers: ${result.modelInfo.numLayers}, Ready for prediction`;
                
                this.updateStatus(successText, readyText);

                // Clear confidence bars when switching models
                this.initializeConfidenceBars();

                console.log('Model switched successfully:', result);
            } else{
                throw new Error(result.message || 'Model switch failed');
            }

        } catch(error){
            console.error('Model switch error:', error);
            const errorText = this.currentLanguage === 'sk' ? 
                `Nepodarilo sa prepnúť model: ${error.message}` :
                `Model switch failed: ${error.message}`;
            this.updateStatus(errorText);
            
            // Revert selector to previous model
            document.getElementById('modelSelect').value = this.currentModel;
        } finally{
            this.showLoading(false);
        }
    }

    initializeConfidenceBars(){
        // Reset all confidence bars
        for(let i = 0; i < 10; i++){
            const bar = document.querySelector(`[data-digit="${i}"]`);
            const percentage = bar.querySelector('.confidence-percentage');
            const fill = bar.querySelector('.progress-fill');
            
            percentage.textContent = '0%';
            fill.style.transform = 'scaleY(0)';
            bar.classList.remove('winner');
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
        const currentX = e.clientX - rect.left;
        const currentY = e.clientY - rect.top;
        
        this.ctx.beginPath();
        this.ctx.moveTo(this.lastX, this.lastY);
        this.ctx.lineTo(currentX, currentY);
        this.ctx.stroke();
        
        this.lastX = currentX;
        this.lastY = currentY;
        
        // Trigger real-time prediction with debouncing
        if(this.realTimePrediction){
            this.scheduleRealTimePrediction();
        }
    }

    stopDrawing(){
        this.isDrawing = false;
        
        // Final prediction after drawing stops
        if(this.realTimePrediction){
            this.scheduleRealTimePrediction(200); // Shorter delay for final prediction
        }
    }

    scheduleRealTimePrediction(delay = this.predictionDelay){
        // Clear existing timeout
        if(this.predictionTimeout){
            clearTimeout(this.predictionTimeout);
        }
        
        // Schedule new prediction
        this.predictionTimeout = setTimeout(() => {
            this.predict(true); // true indicates real-time prediction
        }, delay);
    }

    clearCanvas(){
        this.ctx.fillStyle = '#ffffff';
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        
        // Reset confidence bars
        this.initializeConfidenceBars();
        
        const clearText = this.currentLanguage === 'sk' ? 
            'Nakreslite číslicu pre začatie analýzy' : 
            'Draw a digit to begin';
        this.updateStatus(clearText);
        
        // Clear any pending predictions
        if(this.predictionTimeout){
            clearTimeout(this.predictionTimeout);
        }
    }

    resizeImageData(imageData, width, height){
        const tempCanvas = document.createElement('canvas');
        tempCanvas.width = width;
        tempCanvas.height = height;
        const tempCtx = tempCanvas.getContext('2d');

        const originalCanvas = document.createElement('canvas');
        originalCanvas.width = imageData.width;
        originalCanvas.height = imageData.height;
        const originalCtx = originalCanvas.getContext('2d');
        originalCtx.putImageData(imageData, 0, 0);

        tempCtx.drawImage(originalCanvas, 0, 0, width, height);
        return tempCtx.getImageData(0, 0, width, height);
    }

    preprocessImage(){
        // Get image data from canvas
        const imageData = this.ctx.getImageData(0, 0, this.canvas.width, this.canvas.height);
        
        // Resize to 28x28
        const resizedData = this.resizeImageData(imageData, 28, 28);
        const data = resizedData.data;
        
        // Convert to grayscale array (0-1)
        const pixels = [];
        for(let i = 0; i < data.length; i += 4){
            const r = data[i];
            const g = data[i + 1];
            const b = data[i + 2];
            // Invert colors since MNIST uses white digits on black background
            const gray = 1 - (0.299 * r + 0.587 * g + 0.114 * b) / 255;
            pixels.push(gray);
        }
        
        return pixels;
    }

    async predict(isRealTime = false){
        try{
            const pixels = this.preprocessImage();
            
            // Check if canvas is empty
            const hasContent = pixels.some(pixel => pixel > 0.1);
            if(!hasContent){
                if(!isRealTime){
                    const emptyText = this.currentLanguage === 'sk' ? 
                        'Prosím, nakreslite číslicu najprv' : 
                        'Please draw a digit first';
                    this.updateStatus(emptyText);
                }
                return;
            }

            // Show loading state for manual predictions
            if(!isRealTime){
                this.showLoading(true);
            }

            const response = await fetch(this.apiEndpoint, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({ 
                    pixels,
                    options: {
                        normalize: true,
                        center: true,
                        threshold: 0.1
                    }
                }),
            });

            if(!response.ok){
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            const result = await response.json();
            
            if(result.error){
                throw new Error(result.error);
            }

            this.displayResults(result, isRealTime);

        } catch(error){
            console.error('Prediction error:', error);
            if(!isRealTime){
                const errorText = this.currentLanguage === 'sk' ? 
                    `Chyba: ${error.message}` : 
                    `Error: ${error.message}`;
                this.updateStatus(errorText);
            }
        } finally{
            if(!isRealTime){
                this.showLoading(false);
            }
        }
    }

    displayResults(result, isRealTime = false){
        const { prediction, confidence, probabilities, metadata } = result;
        
        // Update status with model info
        if(!isRealTime){
            const modelSelect = document.getElementById('modelSelect');
            const currentModelName = modelSelect.selectedOptions[0]?.textContent || 'Unknown';
            
            const predictionText = this.currentLanguage === 'sk' ? 
                `Predikcia: ${prediction}` : 
                `Predicted: ${prediction}`;
            
            const confidenceText = this.currentLanguage === 'sk' ? 
                `Spoľahlivosť: ${(confidence * 100).toFixed(1)}% | Model: ${currentModelName}` :
                `Confidence: ${(confidence * 100).toFixed(1)}% | Model: ${currentModelName}`;
            
            this.updateStatus(predictionText, confidenceText);
        }

        // Animate confidence bars
        this.animateConfidenceBars(probabilities, prediction);
    }

    animateConfidenceBars(probabilities, winnerDigit){
        // Find max probability for scaling
        const maxProb = Math.max(...probabilities);
        
        probabilities.forEach((prob, digit) => {
            const bar = document.querySelector(`[data-digit="${digit}"]`);
            const percentage = bar.querySelector('.confidence-percentage');
            const fill = bar.querySelector('.progress-fill');
            
            // Update percentage text
            const percentText = (prob * 100).toFixed(1) + '%';
            percentage.textContent = percentText;
            
            // Calculate height (scale relative to max probability)
            const height = maxProb > 0 ? (prob / maxProb) : 0;
            
            // Animate the fill bar
            setTimeout(() => {
                fill.style.transform = `scaleY(${height})`;
            }, digit * 50); // Stagger animation
            
            // Highlight winner
            if(digit === winnerDigit){
                setTimeout(() => {
                    bar.classList.add('winner');
                    bar.classList.add('pulse');
                }, 800);
            } else{
                bar.classList.remove('winner', 'pulse');
            }
        });
    }

    updateStatus(primary, secondary = ''){
        document.getElementById('predictionResult').textContent = primary;
        document.getElementById('predictionConfidence').textContent = secondary;
    }

    showLoading(show){
        const button = document.querySelector('.btn-predict');
        if(show){
            const loadingText = this.currentLanguage === 'sk' ? 'Analyzujem...' : 'Analyzing...';
            button.innerHTML = `<span class="loading"></span> ${loadingText}`;
            button.disabled = true;
        } else{
            const analyzeText = this.currentLanguage === 'sk' ? 'Analyzovať' : 'Analyze';
            button.innerHTML = analyzeText;
            button.disabled = false;
        }
    }
}

// Global functions for HTML onclick handlers
function clearCanvas(){
    window.neuralApp.clearCanvas();
}

function predict(){
    window.neuralApp.predict(false);
}

// Initialize application when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    window.neuralApp = new NeuralNumbers();
    console.log('MarkNET - Neural Numbers initialized');
});
