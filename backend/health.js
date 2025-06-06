/*
 * ══════════════════════════════════════════════════════════════════════════════
 * MARKNET - HEALTH CHECK API ENDPOINT
 * ══════════════════════════════════════════════════════════════════════════════
 * 
 * REST API endpoint pre monitoring zdravia a stavu MarkNET systému.
 * Kontroluje stav servera, načítaný model neurónovej siete a funkčnosť
 * predikcie. Poskytuje detailné informácie o výkone a dostupnosti.
 * 
 * Endpointy:
 * - GET /api/health - Kompletná diagnostika systému
 * 
 * Kontroly:
 * - Server status (uptime, pamäť, CPU)
 * - Neural network model status
 * - Prediction functionality test
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
const neuralNetwork = require('./index');

//=============================================================================
// HEALTH CHECK ENDPOINT
//=============================================================================

/*
 * GET /api/health
 * Hlavný health check endpoint pre diagnostiku systému
 * Vykonáva kompletné testovanie všetkých komponentov
 * 
 * Návratové kódy:
 * - 200: Systém je plne funkčný
 * - 503: Systém má problémy (degraded)
 * - 500: Kritická chyba v health check
 */
router.get('/', async (req, res) => {
    try{
        const startTime = Date.now();
        
        //=====================================================================
        // KONTROLA STAVU SERVERA
        //=====================================================================
        
        /*
         * Základné kontroly servera - uptime, pamäť, CPU využitie
         * Zbiera systémové metriky pre monitoring
         */
        const serverHealth = {
            status: 'healthy',
            timestamp: new Date().toISOString(),
            uptime: process.uptime(),
            memory: process.memoryUsage(),
            cpu: process.cpuUsage()
        };
        
        //=====================================================================
        // KONTROLA STAVU NEURÓNOVEJ SIETE
        //=====================================================================
        
        /*
         * Kontrola či je model načítaný a dostupný
         * Získa informácie o architektúre siete
         */
        let modelHealth;
        try{
            const modelInfo = neuralNetwork.getModelInfo();
            modelHealth = {
                status: modelInfo.loaded ? 'loaded' : 'not_loaded',
                loaded: modelInfo.loaded,
                numLayers: modelInfo.numLayers || null
            };
        } catch(error){
            logger.error('Kontrola modelu zlyhala', { error: error.message });
            modelHealth = {
                status: 'error',
                loaded: false,
                error: error.message
            };
        }
        
        //=====================================================================
        // TEST FUNKČNOSTI PREDIKCIE
        //=====================================================================
        
        /*
         * Vykonanie testovej predikcie na dummy dátach
         * Overuje že neurónová sieť správne funguje
         */
        let predictionTest;
        try{
            if(modelHealth.loaded){
                // Test s prázdnymi pixelmi (28x28 nuly)
                const testPixels = new Array(784).fill(0);
                const testPrediction = neuralNetwork.predict(testPixels);
                
                predictionTest = {
                    status: Array.isArray(testPrediction) && testPrediction.length === 10 ? 'working' : 'failed',
                    testCompleted: true
                };
            } else {
                predictionTest = {
                    status: 'skipped',
                    testCompleted: false,
                    reason: 'Model not loaded'
                };
            }
        } catch(error){
            logger.error('Test predikcie zlyhal', { error: error.message });
            predictionTest = {
                status: 'failed',
                testCompleted: true,
                error: error.message
            };
        }
        
        //=====================================================================
        // VYHODNOTENIE CELKOVÉHO STAVU
        //=====================================================================
        
        const responseTime = Date.now() - startTime;
        
        /*
         * Určenie celkového zdravia systému
         * Healthy len ak všetky komponenty fungují správne
         */
        const overallStatus = (
            serverHealth.status === 'healthy' && 
            modelHealth.status === 'loaded' && 
            predictionTest.status === 'working'
        ) ? 'healthy' : 'degraded';
        
        /*
         * Štruktúrovaná odpoveď s detailnými informáciami
         * Obsahuje všetky kontroly a metriky
         */
        const healthResponse = {
            status: overallStatus,
            checks: {
                server: serverHealth,
                model: modelHealth,
                prediction: predictionTest
            },
            responseTime: `${responseTime}ms`,
            environment: process.env.NODE_ENV || 'development'
        };
        
        //=====================================================================
        // LOGOVANIE A ODPOVEĎ
        //=====================================================================
        
        // Logovanie výsledkov health check
        logger.info('Health check dokončený', {
            status: overallStatus,
            responseTime: responseTime,
            modelLoaded: modelHealth.loaded,
            predictionWorking: predictionTest.status === 'working'
        });
        
        // Vrátenie HTTP odpovede s príslušným status kódom
        const httpStatus = overallStatus === 'healthy' ? 200 : 503;
        res.status(httpStatus).json(healthResponse);
        
    } catch(error){
        logger.error('Chyba v health check endpointe', { 
            error: error.message, 
            stack: error.stack 
        });
        
        res.status(500).json({
            status: 'error',
            error: 'Health check failed',
            message: error.message,
            timestamp: new Date().toISOString()
        });
    }
});

//=============================================================================
// EXPORT ROUTER MODULU
//=============================================================================

module.exports = router;
