// Predikčný endpoint

const express = require('express');
const router = express.Router();
const { logger } = require('./logger');
const { preprocessImage, validateImageInput } = require('./imageProcessor');
const neuralNetwork = require('./index');

// POST /api/predict
router.post('/', async (req, res) => {
    const requestId = `req_${Date.now()}`;
    
    try{
        const { pixels, options = {} } = req.body;
        
        // Validácia
        const validation = validateImageInput(pixels);
        if(!validation.valid){
            return res.status(400).json({
                error: 'Invalid input data',
                details: validation.errors
            });
        }
        
        // Preprocessing
        const processedPixels = preprocessImage(pixels, {
            normalize: options.normalize !== false,
            center: options.center !== false,
            threshold: options.threshold || 0.1,
            applyThresh: options.applyThreshold !== false
        });
        
        // Predikcia
        const predictions = neuralNetwork.predict(processedPixels);
        
        if(!predictions || predictions.length !== 10){
            throw new Error('Invalid prediction result');
        }
        
        // Výsledky
        const predictedDigit = predictions.reduce((maxIdx, curr, idx, arr) => 
            curr > arr[maxIdx] ? idx : maxIdx, 0);
        
        const sum = predictions.reduce((acc, val) => acc + val, 0);
        const probabilities = predictions.map(p => p / sum);
        
        logger.info('Predikcia dokončená', {
            requestId,
            digit: predictedDigit,
            confidence: predictions[predictedDigit].toFixed(4)
        });
        
        res.json({
            prediction: predictedDigit,
            confidence: predictions[predictedDigit],
            probabilities: probabilities
        });
        
    } catch(error){
        logger.error('Chyba predikcie', { requestId, error: error.message });
        res.status(500).json({
            error: 'Prediction failed',
            message: error.message
        });
    }
});

module.exports = router;
