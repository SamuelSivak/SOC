/*
 * ══════════════════════════════════════════════════════════════════════════════
 * MARKNET - PREDIKČNÉ API ENDPOINTY
 * ══════════════════════════════════════════════════════════════════════════════
 * 
 * REST API endpointy pre vykonávanie predikcií s neurónovou sieťou MarkNET.
 * Podporuje jednotlivé aj batch predikcie s pokročilým preprocessing
 * obrazových dát a detailným response loggingom.
 * 
 * Endpointy:
 * - POST /api/predict       - Jednotlivá predikcia s preprocessing
 * - POST /api/predict/batch - Batch predikcia viacerých obrázkov
 * 
 * Funkcie:
 * - Validácia a preprocessing vstupných dát
 * - Detailné výsledky s confidence scoring
 * - Entropy výpočty pre uncertainty measurement
 * - Request tracking a performance monitoring
 * - Batch processing s error handling
 * 
 * Autor: Samuel Sivák
 * Verzia: 1.0.0
 * ══════════════════════════════════════════════════════════════════════════════
 */

//=============================================================================
// IMPORTY A ZÁVISLOSTI
//=============================================================================

const express = require('express');
const router = express.Router();
const { logger } = require('./logger');
const { preprocessImage, validateImageInput } = require('./imageProcessor');
const neuralNetwork = require('./index');

//=============================================================================
// JEDNOTLIVÁ PREDIKCIA ENDPOINT
//=============================================================================

/*
 * POST /api/predict
 * Hlavný endpoint pre predikciu jednotlivých rukopisných číslic
 * Vykonáva kompletný preprocessing pipeline a vráti detailné výsledky
 * 
 * Body parametre:
 * - pixels: pole 784 čísel (28x28 pixelov)
 * - options: objekt s preprocessing nastaveniami
 * 
 * Návratové kódy:
 * - 200: Úspešná predikcia
 * - 400: Neplatné vstupné dáta
 * - 500: Chyba v neurónovej sieti
 */
router.post('/', async (req, res) => {
    const startTime = Date.now();
    const requestId = `req_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
    
    try{
        logger.info('Žiadosť o predikciu prijatá', { requestId });
        
        const { pixels, options = {} } = req.body;
        
        //=====================================================================
        // VALIDÁCIA VSTUPNÝCH DÁT
        //=====================================================================
        
        /*
         * Kontrola formátu a validity pixelových dát
         * Overenie počtu pixelov a typov hodnôt
         */
        const validation = validateImageInput(pixels);
        if(!validation.valid){
            logger.warn('Neplatné vstupné dáta', { 
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
        
        logger.debug('Validácia vstupov úspešná', { 
            requestId,
            pixelCount: pixels.length,
            nonZeroPixels: pixels.filter(p => p > 0).length
        });
        
        //=====================================================================
        // PREPROCESSING OBRAZOVÝCH DÁT
        //=====================================================================
        
        /*
         * Aplikácia preprocessing pipeline na vstupné pixely
         * Normalizácia, centrovanie a threshold aplikácia
         */
        let processedPixels;
        try{
            const preprocessOptions = {
                normalize: options.normalize !== false,      // default true
                center: options.center !== false,           // default true
                threshold: options.threshold || 0.1,
                applyThresh: options.applyThreshold !== false // default true
            };
            
            processedPixels = preprocessImage(pixels, preprocessOptions);
            
            logger.debug('Preprocessing obrazu dokončený', {
                requestId,
                preprocessOptions,
                originalNonZero: pixels.filter(p => p > 0).length,
                processedNonZero: processedPixels.filter(p => p > 0).length
            });
            
        } catch(error){
            logger.error('Preprocessing obrazu zlyhal', { 
                requestId, 
                error: error.message 
            });
            return res.status(400).json({
                error: 'Image preprocessing failed',
                message: error.message,
                requestId
            });
        }
        
        //=====================================================================
        // VYKONANIE PREDIKCIE
        //=====================================================================
        
        /*
         * Volanie neurónovej siete pre predikciu číslice
         * Kontrola validity výstupných pravdepodobností
         */
        let predictions;
        try{
            predictions = neuralNetwork.predict(processedPixels);
            
            if(!predictions || !Array.isArray(predictions) || predictions.length !== 10){
                throw new Error(`Invalid prediction result: ${predictions}`);
            }
            
            logger.debug('Predikcia neurónovej siete dokončená', {
                requestId,
                predictionsReceived: predictions.length
            });
            
        } catch(error){
            logger.error('Predikcia neurónovej siete zlyhala', { 
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
        
        //=====================================================================
        // SPRACOVANIE VÝSLEDKOV
        //=====================================================================
        
        /*
         * Analýza výstupných pravdepodobností a extrakcia metrík
         * Výpočet confidence, entropy a normalizovaných pravdepodobností
         */
        const predictedDigit = predictions.reduce((maxIndex, current, currentIndex, arr) => 
            current > arr[maxIndex] ? currentIndex : maxIndex, 0);
        
        const confidence = predictions[predictedDigit];
        const maxConfidence = Math.max(...predictions);
        const minConfidence = Math.min(...predictions);
        
        // Výpočet normalizovaných pravdepodobností (súčet = 1)
        const sum = predictions.reduce((acc, val) => acc + val, 0);
        const normalizedProbabilities = predictions.map(p => p / sum);
        
        const responseTime = Date.now() - startTime;
        
        /*
         * Zostavenie štruktúrovanej odpovede s metadátami
         * Zahŕňa entropy pre uncertainty measurement
         */
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
        
        logger.info('Predikcia úspešne dokončená', {
            requestId,
            predictedDigit,
            confidence: confidence.toFixed(4),
            responseTime,
            entropy: result.metadata.entropy.toFixed(4)
        });
        
        res.json(result);
        
    } catch(error){
        const responseTime = Date.now() - startTime;
        logger.error('Chyba v predikčnom endpointe', { 
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

//=============================================================================
// BATCH PREDIKCIA ENDPOINT
//=============================================================================

/*
 * POST /api/predict/batch
 * Endpoint pre hromadné predikcie viacerých obrázkov naraz
 * Optimalizovaný pre efektívne spracovanie malých batch-ov
 * 
 * Body parametre:
 * - images: pole polí pixelov (max 10 obrázkov)
 * - options: spoločné preprocessing nastavenia
 * 
 * Návratové kódy:
 * - 200: Batch spracovaný (aj s chybami v jednotlivých obrázkov)
 * - 400: Neplatný formát batch-u
 * - 500: Kritická chyba v batch processing
 */
router.post('/batch', async (req, res) => {
    const startTime = Date.now();
    const requestId = `batch_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
    
    try{
        const { images, options = {} } = req.body;
        
        //=====================================================================
        // VALIDÁCIA BATCH PARAMETROV
        //=====================================================================
        
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
        
        logger.info('Žiadosť o batch predikciu prijatá', { 
            requestId, 
            batchSize: images.length 
        });
        
        //=====================================================================
        // SPRACOVANIE JEDNOTLIVÝCH OBRÁZKOV
        //=====================================================================
        
        const results = [];
        
        /*
         * Iterácia cez každý obrázok v batch-i
         * Individuálne error handling pre robustnosť
         */
        for(let i = 0; i < images.length; i++){
            try{
                // Validácia jednotlivého obrázka
                const validation = validateImageInput(images[i]);
                if(!validation.valid){
                    results.push({
                        index: i,
                        error: 'Invalid input',
                        details: validation.errors
                    });
                    continue;
                }
                
                // Preprocessing a predikcia
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
        
        //=====================================================================
        // ZOSTAVENIE BATCH ODPOVEDE
        //=====================================================================
        
        const responseTime = Date.now() - startTime;
        const successCount = results.filter(r => !r.error).length;
        
        logger.info('Batch predikcia dokončená', {
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
        logger.error('Chyba v batch predikcii', { 
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

//=============================================================================
// EXPORT ROUTER MODULU
//=============================================================================

module.exports = router;
