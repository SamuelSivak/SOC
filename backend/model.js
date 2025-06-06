/*
 * ══════════════════════════════════════════════════════════════════════════════
 * MARKNET - SPRÁVA MODELOV NEURÓNOVEJ SIETE
 * ══════════════════════════════════════════════════════════════════════════════
 * 
 * REST API pre správu a manipuláciu modelov neurónovej siete MarkNET.
 * Poskytuje endpointy pre získanie informácií o modeloch, listing dostupných
 * modelov a prepínanie medzi rôznymi natrénovanými modelmi MNIST.
 * 
 * Endpointy:
 * - GET  /api/model/info     - Detailné informácie o modeli
 * - GET  /api/model/models   - Zoznam dostupných modelov
 * - POST /api/model/switch   - Prepnutie na iný model
 * 
 * Funkcie:
 * - Metadata o architektúre neurónovej siete
 * - Výkonnostné metriky modelov
 * - Dynamické prepínanie modelov za behu
 * - Validácia a error handling
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
const path = require('path');
const fs = require('fs');
const { logger } = require('./logger');
const neuralNetwork = require('./index');

//=============================================================================
// MODEL INFO ENDPOINT
//=============================================================================

/*
 * GET /api/model/info
 * Získanie detailných informácií o aktuálne načítanom modeli
 * Vracia architektúru, výkonnosť, súborové informácie a capabilities
 * 
 * Návratové kódy:
 * - 200: Úspešne získané informácie o modeli
 * - 404: Model nie je načítaný
 * - 500: Chyba pri získavaní informácií
 */
router.get('/info', async (req, res) => {
    try{
        const startTime = Date.now();
        
        // Získanie základných informácií z natívneho modulu
        const modelInfo = neuralNetwork.getModelInfo();
        
        // Kontrola či je model načítaný
        if(!modelInfo.loaded){
            return res.status(404).json({
                error: 'Model not loaded',
                message: 'Neural network model is not currently loaded',
                loaded: false
            });
        }
        
        //=====================================================================
        // ANALÝZA SÚBOROVÝCH INFORMÁCIÍ
        //=====================================================================
        
        /*
         * Získanie metadát o súbore modelu
         * Veľkosť, dátumy vytvorenia a modifikácie
         */
        const modelPath = path.join(__dirname, '../../models/mnist_model.bin');
        let fileStats = null;
        try{
            fileStats = fs.statSync(modelPath);
        } catch(error){
            logger.warn('Nepodarilo sa prečítať metadáta súboru modelu', { 
                error: error.message 
            });
        }
        
        //=====================================================================
        // VÝKONNOSTNÉ METRIKY
        //=====================================================================
        
        /*
         * Špecifikácia typických výkonnostných metrík pre MNIST dataset
         * V produkčnom prostredí by tieto hodnoty pochádzali z trénovacích logov
         */
        const performanceMetrics = {
            trainingAccuracy: '~98%',
            testingAccuracy: '~92-93%',
            trainingTime: '~2-3 minutes (CPU)',
            note: 'Performance metrics are typical values for MNIST dataset'
        };
        
        //=====================================================================
        // ARCHITEKTÚRA NEURÓNOVEJ SIETE
        //=====================================================================
        
        /*
         * Detailné informácie o štruktúre a architektúre neurónovej siete
         * Zahŕňa aktivačné funkcie, optimizer a loss function
         */
        const architectureInfo = {
            type: 'Multi-layer Perceptron (MLP)',
            inputSize: 784,    // 28x28 pixelov
            outputSize: 10,    // číslice 0-9
            numLayers: modelInfo.numLayers,
            activationFunctions: {
                hidden: 'ReLU',
                output: 'Softmax'
            },
            lossFunction: 'Cross-entropy',
            optimizer: 'Mini-batch Gradient Descent'
        };
        
        //=====================================================================
        // INFORMÁCIE O DATASETE
        //=====================================================================
        
        /*
         * Metadáta o MNIST datasete použitom na trénovanie
         * Počty obrázkov, rozlíšenie, formát a triedy
         */
        const datasetInfo = {
            name: 'MNIST',
            description: 'Handwritten digit recognition dataset',
            trainingImages: 60000,
            testingImages: 10000,
            imageSize: '28x28 pixels',
            classes: 10,       // číslice 0-9
            format: 'Grayscale'
        };
        
        const responseTime = Date.now() - startTime;
        
        //=====================================================================
        // ZOSTAVENIE ODPOVEDE
        //=====================================================================
        
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
        
        logger.info('Informácie o modeli úspešne poskytnuté', {
            responseTime: responseTime,
            modelLoaded: true
        });
        
        res.json(modelInfoResponse);
        
    } catch(error){
        logger.error('Chyba v endpointe model info', { 
            error: error.message, 
            stack: error.stack 
        });
        res.status(500).json({
            error: 'Failed to get model information',
            message: error.message,
            loaded: false,
            timestamp: new Date().toISOString()
        });
    }
});

//=============================================================================
// MODELS LISTING ENDPOINT
//=============================================================================

/*
 * GET /api/model/models
 * Získanie zoznamu všetkých dostupných modelov v systéme
 * Skenuje models/ adresár a vráti informácie o každom modeli
 * 
 * Návratové kódy:
 * - 200: Úspešne získaný zoznam modelov
 * - 500: Chyba pri čítaní adresára modelov
 */
router.get('/models', (req, res) => {
    try{
        const modelsDir = path.join(__dirname, '../models');
        const models = [];
        
        //=====================================================================
        // SKENOVANIE MODELS ADRESÁRA
        //=====================================================================
        
        try{
            const files = fs.readdirSync(modelsDir);
            const modelFiles = new Set(); // Použitie Set na zabránenie duplikátov
            
            /*
             * Iterácia cez všetky .bin súbory v models/ adresári
             * Extrakcia metadát a klasifikácia podľa názvu súboru
             */
            for(const file of files){
                if(file.endsWith('.bin') && !modelFiles.has(file)){
                    modelFiles.add(file);
                    const filePath = path.join(modelsDir, file);
                    const stats = fs.statSync(filePath);
                    
                    // Určenie úrovne tréningu a popisu na základe názvu súboru
                    let trainingLevel = 'unknown';
                    let descriptions = {
                        en: 'MNIST model',
                        sk: 'MNIST model'
                    };
                    let displayNames = {
                        en: file.replace('.bin', ''),
                        sk: file.replace('.bin', '')
                    };
                    
                    /*
                     * Klasifikácia modelov na základe počtu trénovacích obrázkov
                     * Rôzne verzie pre rôzne výkonnostné požiadavky s viacjazyčnou podporou
                     */
                    if(file.includes('300')){
                        trainingLevel = '300 images';
                        descriptions = {
                            en: 'Basic model trained on 300 images - Lower accuracy but faster training',
                            sk: 'Základný model natrénovaný na 300 obrázkoch - Nižšia presnosť ale rýchlejšie trénovanie'
                        };
                        displayNames = {
                            en: 'Basic (300 images)',
                            sk: 'Základný (300 obrázkov)'
                        };
                    } else if(file.includes('1500')){
                        trainingLevel = '1500 images';
                        descriptions = {
                            en: 'Intermediate model trained on 1500 images - Balanced accuracy and training time',
                            sk: 'Stredne pokročilý model natrénovaný na 1500 obrázkoch - Vyvážená presnosť a čas trénovania'
                        };
                        displayNames = {
                            en: 'Intermediate (1500 images)',
                            sk: 'Stredne pokročilý (1500 obrázkov)'
                        };
                    } else if(file.includes('final')){
                        trainingLevel = '60000 images';
                        descriptions = {
                            en: 'Final optimized model on complete MNIST dataset - Highest accuracy',
                            sk: 'Finálny optimalizovaný model na kompletnom MNIST datasete - Najvyššia presnosť'
                        };
                        displayNames = {
                            en: 'Final Model (60,000 images)',
                            sk: 'Finálny model (60 000 obrázkov)'
                        };
                    } else if(file.includes('best')){
                        trainingLevel = '60000 images';
                        descriptions = {
                            en: 'Best performance model on complete MNIST dataset - Optimized for accuracy',
                            sk: 'Najvýkonnejší model na kompletnom MNIST datasete - Optimalizovaný pre presnosť'
                        };
                        displayNames = {
                            en: 'Best Model (60,000 images)',
                            sk: 'Najlepší model (60 000 obrázkov)'
                        };
                    } else if(file === 'mnist_model.bin'){
                        trainingLevel = '60000 images';
                        descriptions = {
                            en: 'Standard MNIST model on complete dataset',
                            sk: 'Štandardný MNIST model na kompletnom datasete'
                        };
                        displayNames = {
                            en: 'Standard Model (60,000 images)',
                            sk: 'Štandardný model (60 000 obrázkov)'
                        };
                    } else{
                        trainingLevel = '60000 images';
                        descriptions = {
                            en: 'MNIST model on complete dataset',
                            sk: 'MNIST model na kompletnom datasete'
                        };
                        const modelName = file.replace('mnist_model_', '').replace('.bin', '').toUpperCase();
                        displayNames = {
                            en: `${modelName} Model`,
                            sk: `${modelName} Model`
                        };
                    }
                    
                    /*
                     * Pridanie modelu do zoznamu s kompletnou metadátami
                     * Zahŕňa veľkosť súboru, dátumy, dostupnosť a viacjazyčné texty
                     */
                    models.push({
                        filename: file,
                        name: file.replace('.bin', ''),
                        displayName: displayNames.en,          // Zachovanie pre backward compatibility
                        displayNames: displayNames,            // Nové viacjazyčné názvy
                        trainingLevel: trainingLevel,
                        description: descriptions.en,          // Zachovanie pre backward compatibility  
                        descriptions: descriptions,            // Nové viacjazyčné popisy
                        size: `${(stats.size / 1024).toFixed(2)} KB`,
                        created: stats.birthtime,
                        modified: stats.mtime,
                        available: true
                    });
                }
            }
        } catch(error){
            logger.error('Chyba pri čítaní adresára modelov', { error: error.message });
        }

        //=====================================================================
        // OČAKÁVANÉ MODELY (PLACEHOLDER ENTRIES)
        //=====================================================================
        
        /*
         * Pridanie placeholder záznamov pre modely ktoré ešte nie sú natrénované
         * Umožňuje frontend-u vedieť o dostupných možnostiach trénovania s viacjazyčnou podporou
         */
        const expectedModels = [
            {
                filename: 'mnist_model_300.bin',
                name: 'mnist_model_300',
                displayName: 'Basic (300 images)',
                displayNames: {
                    en: 'Basic (300 images)',
                    sk: 'Základný (300 obrázkov)'
                },
                trainingLevel: '300 images',
                description: 'Basic model trained on 300 images - Lower accuracy but faster training',
                descriptions: {
                    en: 'Basic model trained on 300 images - Lower accuracy but faster training',
                    sk: 'Základný model natrénovaný na 300 obrázkoch - Nižšia presnosť ale rýchlejšie trénovanie'
                }
            },
            {
                filename: 'mnist_model_1500.bin',
                name: 'mnist_model_1500',
                displayName: 'Intermediate (1500 images)',
                displayNames: {
                    en: 'Intermediate (1500 images)',
                    sk: 'Stredne pokročilý (1500 obrázkov)'
                },
                trainingLevel: '1500 images',
                description: 'Intermediate model trained on 1500 images - Balanced accuracy and training time',
                descriptions: {
                    en: 'Intermediate model trained on 1500 images - Balanced accuracy and training time',
                    sk: 'Stredne pokročilý model natrénovaný na 1500 obrázkoch - Vyvážená presnosť a čas trénovania'
                }
            }
        ];

        /*
         * Pridanie len tých modelov ktoré ešte nie sú v zozname
         * Označenie ako 'Training...' pre modely v procese trénovania
         */
        for(const expectedModel of expectedModels){
            if(!models.find(m => m.filename === expectedModel.filename)){
                expectedModel.available = false;
                expectedModel.size = 'Training...';
                expectedModel.created = null;
                expectedModel.modified = null;
                models.push(expectedModel);
            }
        }

        //=====================================================================
        // FILTROVANIE A TRIEDENIE
        //=====================================================================
        
        /*
         * Odstránenie duplikátov a triedenie podľa úrovne trénovania
         * Zoradenie od najmenších po najväčšie modely
         */
        const uniqueModels = models.filter((model, index, self) => 
            index === self.findIndex(m => m.filename === model.filename)
        );

        uniqueModels.sort((a, b) => {
            const order = { '300 images': 1, '1500 images': 2, '60000 images': 3 };
            const aOrder = order[a.trainingLevel] || 99;
            const bOrder = order[b.trainingLevel] || 99;
            if(aOrder !== bOrder) return aOrder - bOrder;
            // Ak rovnaká úroveň trénovania, triediť podľa názvu súboru
            return a.filename.localeCompare(b.filename);
        });
        
        res.json({
            availableModels: uniqueModels,
            currentModel: 'mnist_model.bin', // Aktuálne načítaný model
            modelsDirectory: 'models/',
            timestamp: new Date().toISOString()
        });
        
    } catch(error){
        logger.error('Chyba v endpointe zoznamu modelov', { error: error.message });
        res.status(500).json({
            error: 'Failed to list models',
            message: error.message
        });
    }
});

//=============================================================================
// MODEL SWITCHING ENDPOINT
//=============================================================================

/*
 * POST /api/model/switch
 * Prepnutie na iný dostupný model neurónovej siete
 * Umožňuje dynamické prepínanie bez restartu servera
 * 
 * Body parametre:
 * - modelName: názov súboru modelu na načítanie
 * 
 * Návratové kódy:
 * - 200: Model úspešne prepnutý
 * - 400: Chýba názov modelu
 * - 404: Model nenájdený
 * - 500: Chyba pri načítavaní modelu
 */
router.post('/switch', async (req, res) => {
    try{
        const { modelName } = req.body;
        
        // Validácia požadovaných parametrov
        if(!modelName){
            return res.status(400).json({
                error: 'Missing model name',
                message: 'modelName parameter is required'
            });
        }

        const modelsDir = path.join(__dirname, '../models');
        const modelPath = path.join(modelsDir, modelName);

        //=====================================================================
        // VALIDÁCIA EXISTENCIE SÚBORU
        //=====================================================================
        
        if(!fs.existsSync(modelPath)){
            return res.status(404).json({
                error: 'Model not found',
                message: `Model file ${modelName} does not exist`
            });
        }

        logger.info('Požiadavka na prepnutie modelu', { 
            modelName, 
            modelPath,
            requestedBy: req.ip 
        });

        //=====================================================================
        // NAČÍTANIE NOVÉHO MODELU
        //=====================================================================
        
        /*
         * Pokus o inicializáciu neurónovej siete s novým modelom
         * Zahŕňa cleanup predchádzajúceho modelu ak je potrebné
         */
        try{
            const success = neuralNetwork.init(modelPath);
            if(!success){
                throw new Error('Failed to initialize neural network with new model');
            }

            // Overenie že model sa správne načítal
            const modelInfo = neuralNetwork.getModelInfo();
            if(!modelInfo.loaded){
                throw new Error('Model loaded but reports as not loaded');
            }

            logger.info('Model úspešne prepnutý', {
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
            logger.error('Nepodarilo sa načítať nový model', {
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
        logger.error('Chyba v endpointe prepínania modelov', { 
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

//=============================================================================
// EXPORT ROUTER MODULU
//=============================================================================

module.exports = router;
