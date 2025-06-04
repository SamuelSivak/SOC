const express = require('express');
const router = express.Router();
const path = require('path');
const fs = require('fs');
const { logger } = require('./logger');
const neuralNetwork = require('./index');

/**
 * GET /api/model/info
 * Get detailed model information
 */
router.get('/info', async (req, res) => {
    try{
        const startTime = Date.now();
        
        // Get basic model info from native module
        const modelInfo = neuralNetwork.getModelInfo();
        
        if(!modelInfo.loaded){
            return res.status(404).json({
                error: 'Model not loaded',
                message: 'Neural network model is not currently loaded',
                loaded: false
            });
        }
        
        // Get model file information
        const modelPath = path.join(__dirname, '../../models/mnist_model.bin');
        let fileStats = null;
        try{
            fileStats = fs.statSync(modelPath);
        } catch(error){
            logger.warn('Could not read model file stats', { error: error.message });
        }
        
        // Gather performance metrics (if available)
        const performanceMetrics = {
            // These would typically come from training logs or stored metadata
            // For now, we'll use typical MNIST performance values
            trainingAccuracy: '~98%',
            testingAccuracy: '~92-93%',
            trainingTime: '~2-3 minutes (CPU)',
            note: 'Performance metrics are typical values for MNIST dataset'
        };
        
        // Model architecture info
        const architectureInfo = {
            type: 'Multi-layer Perceptron (MLP)',
            inputSize: 784, // 28x28 pixels
            outputSize: 10,  // digits 0-9
            numLayers: modelInfo.numLayers,
            activationFunctions: {
                hidden: 'ReLU',
                output: 'Softmax'
            },
            lossFunction: 'Cross-entropy',
            optimizer: 'Mini-batch Gradient Descent'
        };
        
        // Dataset information
        const datasetInfo = {
            name: 'MNIST',
            description: 'Handwritten digit recognition dataset',
            trainingImages: 60000,
            testingImages: 10000,
            imageSize: '28x28 pixels',
            classes: 10, // digits 0-9
            format: 'Grayscale'
        };
        
        const responseTime = Date.now() - startTime;
        
        const modelInfoResponse = {
            loaded: true,
            model: {
                version: '1.0.0',
                architecture: architectureInfo,
                performance: performanceMetrics,
                file: fileStats ? {
                    path: 'models/mnist_model.bin',
                    size: `${(fileStats.size / 1024).toFixed(2)} KB`,
                    created: fileStats.birthtime,
                    modified: fileStats.mtime
                } : null
            },
            dataset: datasetInfo,
            capabilities: [
                'Digit recognition (0-9)',
                'Real-time prediction',
                'Confidence scoring',
                'Image preprocessing'
            ],
            responseTime: `${responseTime}ms`,
            timestamp: new Date().toISOString()
        };
        
        logger.info('Model info requested', {
            responseTime: responseTime,
            modelLoaded: true
        });
        
        res.json(modelInfoResponse);
        
    } catch(error){
        logger.error('Model info endpoint error', { error: error.message, stack: error.stack });
        res.status(500).json({
            error: 'Failed to get model information',
            message: error.message,
            loaded: false,
            timestamp: new Date().toISOString()
        });
    }
});

/**
 * GET /api/model/models
 * List available models
 */
router.get('/models', (req, res) => {
    try{
        const modelsDir = path.join(__dirname, '../../models');
        const models = [];
        
        try{
            const files = fs.readdirSync(modelsDir);
            const modelFiles = new Set(); // Use Set to avoid duplicates
            
            for(const file of files){
                if(file.endsWith('.bin') && !modelFiles.has(file)){
                    modelFiles.add(file);
                    const filePath = path.join(modelsDir, file);
                    const stats = fs.statSync(filePath);
                    
                    // Determine training level and description based on filename
                    let trainingLevel = 'unknown';
                    let description = 'MNIST model';
                    let displayName = file.replace('.bin', '');
                    
                    if(file.includes('300')){
                        trainingLevel = '300 images';
                        description = 'Basic model trained on 300 images - Lower accuracy but faster training';
                        displayName = 'Basic (300 images)';
                    } else if(file.includes('1500')){
                        trainingLevel = '1500 images';
                        description = 'Intermediate model trained on 1500 images - Balanced accuracy and training time';
                        displayName = 'Intermediate (1500 images)';
                    } else if(file.includes('final')){
                        trainingLevel = '60000 images';
                        description = 'Final optimized model on complete MNIST dataset - Highest accuracy';
                        displayName = 'Final Model (60,000 images)';
                    } else if(file.includes('best')){
                        trainingLevel = '60000 images';
                        description = 'Best performance model on complete MNIST dataset - Optimized for accuracy';
                        displayName = 'Best Model (60,000 images)';
                    } else if(file === 'mnist_model.bin'){
                        trainingLevel = '60000 images';
                        description = 'Standard MNIST model on complete dataset';
                        displayName = 'Standard Model (60,000 images)';
                    } else{
                        trainingLevel = '60000 images';
                        description = 'MNIST model on complete dataset';
                        displayName = file.replace('mnist_model_', '').replace('.bin', '').toUpperCase() + ' Model';
                    }
                    
                    models.push({
                        filename: file,
                        name: file.replace('.bin', ''),
                        displayName: displayName,
                        trainingLevel: trainingLevel,
                        description: description,
                        size: `${(stats.size / 1024).toFixed(2)} KB`,
                        created: stats.birthtime,
                        modified: stats.mtime,
                        available: true
                    });
                }
            }
        } catch(error){
            logger.error('Error reading models directory', { error: error.message });
        }

        // Add placeholder entries only for models that don't exist yet
        const expectedModels = [
            {
                filename: 'mnist_model_300.bin',
                name: 'mnist_model_300',
                displayName: 'Basic (300 images)',
                trainingLevel: '300 images',
                description: 'Basic model trained on 300 images - Lower accuracy but faster training'
            },
            {
                filename: 'mnist_model_1500.bin',
                name: 'mnist_model_1500',
                displayName: 'Intermediate (1500 images)',
                trainingLevel: '1500 images',
                description: 'Intermediate model trained on 1500 images - Balanced accuracy and training time'
            }
        ];

        for(const expectedModel of expectedModels){
            // Only add if not already in the list
            if(!models.find(m => m.filename === expectedModel.filename)){
                expectedModel.available = false;
                expectedModel.size = 'Training...';
                expectedModel.created = null;
                expectedModel.modified = null;
                models.push(expectedModel);
            }
        }

        // Sort models by training level and remove any remaining duplicates
        const uniqueModels = models.filter((model, index, self) => 
            index === self.findIndex(m => m.filename === model.filename)
        );

        uniqueModels.sort((a, b) => {
            const order = { '300 images': 1, '1500 images': 2, '60000 images': 3 };
            const aOrder = order[a.trainingLevel] || 99;
            const bOrder = order[b.trainingLevel] || 99;
            if(aOrder !== bOrder) return aOrder - bOrder;
            // If same training level, sort by filename
            return a.filename.localeCompare(b.filename);
        });
        
        res.json({
            availableModels: uniqueModels,
            currentModel: 'mnist_model.bin', // Currently loaded model
            modelsDirectory: 'models/',
            timestamp: new Date().toISOString()
        });
        
    } catch(error){
        logger.error('Models list endpoint error', { error: error.message });
        res.status(500).json({
            error: 'Failed to list models',
            message: error.message
        });
    }
});

/**
 * POST /api/model/switch
 * Switch to a different model
 */
router.post('/switch', async (req, res) => {
    try{
        const { modelName } = req.body;
        
        if(!modelName){
            return res.status(400).json({
                error: 'Missing model name',
                message: 'modelName parameter is required'
            });
        }

        const modelsDir = path.join(__dirname, '../../models');
        const modelPath = path.join(modelsDir, modelName);

        // Validate model file exists
        if(!fs.existsSync(modelPath)){
            return res.status(404).json({
                error: 'Model not found',
                message: `Model file ${modelName} does not exist`
            });
        }

        logger.info('Model switch requested', { 
            modelName, 
            modelPath,
            requestedBy: req.ip 
        });

        // Attempt to load the new model
        try{
            const success = neuralNetwork.init(modelPath);
            if(!success){
                throw new Error('Failed to initialize neural network with new model');
            }

            // Get model information to confirm it loaded correctly
            const modelInfo = neuralNetwork.getModelInfo();
            if(!modelInfo.loaded){
                throw new Error('Model loaded but reports as not loaded');
            }

            logger.info('Model switched successfully', {
                modelName,
                numLayers: modelInfo.numLayers,
                loaded: modelInfo.loaded
            });

            res.json({
                success: true,
                message: `Successfully switched to model: ${modelName}`,
                modelInfo: {
                    filename: modelName,
                    loaded: modelInfo.loaded,
                    numLayers: modelInfo.numLayers
                },
                timestamp: new Date().toISOString()
            });

        } catch(loadError){
            logger.error('Failed to load new model', {
                modelName,
                error: loadError.message
            });

            res.status(500).json({
                error: 'Model load failed',
                message: `Failed to load model ${modelName}: ${loadError.message}`,
                modelName
            });
        }

    } catch(error){
        logger.error('Model switch endpoint error', { 
            error: error.message, 
            stack: error.stack 
        });
        
        res.status(500).json({
            error: 'Model switch failed',
            message: error.message,
            timestamp: new Date().toISOString()
        });
    }
});

module.exports = router;
