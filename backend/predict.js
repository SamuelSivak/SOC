const express = require('express');
const router = express.Router();
const { logger } = require('./logger');
const { preprocessImage, validateImageInput } = require('./imageProcessor');
const neuralNetwork = require('./index');

/**
 * POST /api/predict
 * Enhanced prediction endpoint with preprocessing
 */
router.post('/', async (req, res) => {
    const startTime = Date.now();
    const requestId = `req_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
    
    try{
        logger.info('Prediction request received', { requestId });
        
        const { pixels, options = {} } = req.body;
        
        // Validate input
        const validation = validateImageInput(pixels);
        if(!validation.valid){
            logger.warn('Invalid input received', { 
                requestId, 
                errors: validation.errors,
                pixelsLength: pixels ? pixels.length : 'undefined'
            });
            return res.status(400).json({
                error: 'Invalid input data',
                details: validation.errors,
                requestId
            });
        }
        
        logger.debug('Input validation passed', { 
            requestId,
            pixelCount: pixels.length,
            nonZeroPixels: pixels.filter(p => p > 0).length
        });
        
        // Preprocess the image
        let processedPixels;
        try{
            const preprocessOptions = {
                normalize: options.normalize !== false, // default true
                center: options.center !== false,       // default true
                threshold: options.threshold || 0.1,
                applyThresh: options.applyThreshold !== false // default true
            };
            
            processedPixels = preprocessImage(pixels, preprocessOptions);
            
            logger.debug('Image preprocessing completed', {
                requestId,
                preprocessOptions,
                originalNonZero: pixels.filter(p => p > 0).length,
                processedNonZero: processedPixels.filter(p => p > 0).length
            });
            
        } catch(error){
            logger.error('Image preprocessing failed', { 
                requestId, 
                error: error.message 
            });
            return res.status(400).json({
                error: 'Image preprocessing failed',
                message: error.message,
                requestId
            });
        }
        
        // Make prediction
        let predictions;
        try{
            predictions = neuralNetwork.predict(processedPixels);
            
            if(!predictions || !Array.isArray(predictions) || predictions.length !== 10){
                throw new Error(`Invalid prediction result: ${predictions}`);
            }
            
            logger.debug('Neural network prediction completed', {
                requestId,
                predictionsReceived: predictions.length
            });
            
        } catch(error){
            logger.error('Neural network prediction failed', { 
                requestId, 
                error: error.message 
            });
            return res.status(500).json({
                error: 'Prediction failed',
                message: 'Neural network prediction failed',
                details: error.message,
                requestId
            });
        }
        
        // Process results
        const predictedDigit = predictions.reduce((maxIndex, current, currentIndex, arr) => 
            current > arr[maxIndex] ? currentIndex : maxIndex, 0);
        
        const confidence = predictions[predictedDigit];
        const maxConfidence = Math.max(...predictions);
        const minConfidence = Math.min(...predictions);
        
        // Calculate normalized probabilities (ensure they sum to 1)
        const sum = predictions.reduce((acc, val) => acc + val, 0);
        const normalizedProbabilities = predictions.map(p => p / sum);
        
        const responseTime = Date.now() - startTime;
        
        const result = {
            prediction: predictedDigit,
            confidence: confidence,
            probabilities: normalizedProbabilities,
            rawProbabilities: predictions,
            metadata: {
                maxConfidence,
                minConfidence,
                entropy: -normalizedProbabilities.reduce((acc, p) => acc + (p > 0 ? p * Math.log2(p) : 0), 0),
                responseTime: `${responseTime}ms`,
                preprocessingApplied: options,
                requestId
            }
        };
        
        logger.info('Prediction completed successfully', {
            requestId,
            predictedDigit,
            confidence: confidence.toFixed(4),
            responseTime,
            entropy: result.metadata.entropy.toFixed(4)
        });
        
        res.json(result);
        
    } catch(error){
        const responseTime = Date.now() - startTime;
        logger.error('Prediction endpoint error', { 
            requestId, 
            error: error.message, 
            stack: error.stack,
            responseTime
        });
        
        res.status(500).json({
            error: 'Internal server error',
            message: 'An unexpected error occurred during prediction',
            requestId,
            responseTime: `${responseTime}ms`,
            timestamp: new Date().toISOString()
        });
    }
});

/**
 * POST /api/predict/batch
 * Batch prediction endpoint for multiple images
 */
router.post('/batch', async (req, res) => {
    const startTime = Date.now();
    const requestId = `batch_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
    
    try{
        const { images, options = {} } = req.body;
        
        if(!Array.isArray(images)){
            return res.status(400).json({
                error: 'Invalid input',
                message: 'Expected array of images',
                requestId
            });
        }
        
        if(images.length === 0){
            return res.status(400).json({
                error: 'Empty batch',
                message: 'At least one image is required',
                requestId
            });
        }
        
        if(images.length > 10){
            return res.status(400).json({
                error: 'Batch too large',
                message: 'Maximum 10 images per batch',
                requestId
            });
        }
        
        logger.info('Batch prediction request received', { 
            requestId, 
            batchSize: images.length 
        });
        
        const results = [];
        
        for(let i = 0; i < images.length; i++){
            try{
                const validation = validateImageInput(images[i]);
                if(!validation.valid){
                    results.push({
                        index: i,
                        error: 'Invalid input',
                        details: validation.errors
                    });
                    continue;
                }
                
                const processedPixels = preprocessImage(images[i], options);
                const predictions = neuralNetwork.predict(processedPixels);
                
                const predictedDigit = predictions.reduce((maxIndex, current, currentIndex, arr) => 
                    current > arr[maxIndex] ? currentIndex : maxIndex, 0);
                
                results.push({
                    index: i,
                    prediction: predictedDigit,
                    confidence: predictions[predictedDigit],
                    probabilities: predictions
                });
                
            } catch(error){
                results.push({
                    index: i,
                    error: 'Prediction failed',
                    message: error.message
                });
            }
        }
        
        const responseTime = Date.now() - startTime;
        const successCount = results.filter(r => !r.error).length;
        
        logger.info('Batch prediction completed', {
            requestId,
            batchSize: images.length,
            successCount,
            failureCount: images.length - successCount,
            responseTime
        });
        
        res.json({
            results,
            summary: {
                total: images.length,
                successful: successCount,
                failed: images.length - successCount,
                responseTime: `${responseTime}ms`
            },
            requestId
        });
        
    } catch(error){
        logger.error('Batch prediction error', { 
            requestId, 
            error: error.message 
        });
        res.status(500).json({
            error: 'Batch prediction failed',
            message: error.message,
            requestId
        });
    }
});

module.exports = router;
