const express = require('express');
const router = express.Router();
const { logger } = require('./logger');
const neuralNetwork = require('./index');

/**
 * GET /api/health
 * Health check endpoint
 */
router.get('/', async (req, res) => {
    try{
        const startTime = Date.now();
        
        // Check server basics
        const serverHealth = {
            status: 'healthy',
            timestamp: new Date().toISOString(),
            uptime: process.uptime(),
            memory: process.memoryUsage(),
            cpu: process.cpuUsage()
        };
        
        // Check neural network model
        let modelHealth;
        try{
            const modelInfo = neuralNetwork.getModelInfo();
            modelHealth = {
                status: modelInfo.loaded ? 'loaded' : 'not_loaded',
                loaded: modelInfo.loaded,
                numLayers: modelInfo.numLayers || null
            };
        } catch(error){
            logger.error('Model health check failed', { error: error.message });
            modelHealth = {
                status: 'error',
                loaded: false,
                error: error.message
            };
        }
        
        // Test a simple prediction to ensure the model works
        let predictionTest;
        try{
            if(modelHealth.loaded){
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
            logger.error('Prediction test failed', { error: error.message });
            predictionTest = {
                status: 'failed',
                testCompleted: true,
                error: error.message
            };
        }
        
        const responseTime = Date.now() - startTime;
        
        // Determine overall health status
        const overallStatus = (
            serverHealth.status === 'healthy' && 
            modelHealth.status === 'loaded' && 
            predictionTest.status === 'working'
        ) ? 'healthy' : 'degraded';
        
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
        
        // Log health check
        logger.info('Health check completed', {
            status: overallStatus,
            responseTime: responseTime,
            modelLoaded: modelHealth.loaded,
            predictionWorking: predictionTest.status === 'working'
        });
        
        // Return appropriate HTTP status
        const httpStatus = overallStatus === 'healthy' ? 200 : 503;
        res.status(httpStatus).json(healthResponse);
        
    } catch(error){
        logger.error('Health check endpoint error', { error: error.message, stack: error.stack });
        res.status(500).json({
            status: 'error',
            error: 'Health check failed',
            message: error.message,
            timestamp: new Date().toISOString()
        });
    }
});

module.exports = router;
