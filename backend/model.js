// Správa modelov

const express = require('express');
const router = express.Router();
const path = require('path');
const fs = require('fs');
const { logger } = require('./logger');
const neuralNetwork = require('./index');

// Mapovanie názvov modelov
const modelNames = {
    'mnist_model_300.bin': {
        en: 'Basic (300 images)',
        sk: 'Základný (300 obrázkov)'
    },
    'mnist_model_1500.bin': {
        en: 'Intermediate (1500 images)',
        sk: 'Stredný (1500 obrázkov)'
    },
    'mnist_model_best.bin': {
        en: 'Best (60,000 images)',
        sk: 'Najlepší (60 000 obrázkov)'
    },
    'mnist_model_final.bin': {
        en: 'Final (60,000 images)',
        sk: 'Finálny (60 000 obrázkov)'
    },
    'mnist_model_optimized.bin': {
        en: 'Optimized (60,000 images)',
        sk: 'Optimalizovaný (60 000 obrázkov)'
    },
    'mnist_model.bin': {
        en: 'Standard (60,000 images)',
        sk: 'Štandardný (60 000 obrázkov)'
    }
};

// GET /api/model/models - Zoznam modelov
router.get('/models', (req, res) => {
    try{
        const modelsDir = path.join(__dirname, '../models');
        const models = [];
        
        const files = fs.readdirSync(modelsDir);
        
        for(const file of files){
            if(file.endsWith('.bin')){
                const filePath = path.join(modelsDir, file);
                const stats = fs.statSync(filePath);
                const names = modelNames[file] || { en: file, sk: file };
                
                models.push({
                    filename: file,
                    displayNames: names,
                    size: `${(stats.size / 1024).toFixed(2)} KB`
                });
            }
        }
        
        res.json({
            availableModels: models,
            currentModel: 'mnist_model.bin'
        });
        
    } catch(error){
        logger.error('Chyba pri listovaní modelov', { error: error.message });
        res.status(500).json({
            error: 'Failed to list models',
            message: error.message
        });
    }
});

// POST /api/model/switch - Prepnutie modelu
router.post('/switch', async (req, res) => {
    try{
        const { modelName } = req.body;
        
        if(!modelName){
            return res.status(400).json({
                error: 'Missing model name'
            });
        }

        const modelPath = path.join(__dirname, '../models', modelName);

        if(!fs.existsSync(modelPath)){
            return res.status(404).json({
                error: 'Model not found'
            });
        }

        const success = neuralNetwork.init(modelPath);
        if(!success){
            throw new Error('Failed to load model');
        }

        logger.info('Model prepnutý', { modelName });

        res.json({
            success: true,
            message: `Switched to ${modelName}`
        });

    } catch(error){
        logger.error('Chyba pri prepínaní modelu', { error: error.message });
        res.status(500).json({
            error: 'Model switch failed',
            message: error.message
        });
    }
});

module.exports = router;
