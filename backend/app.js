/*
 * ══════════════════════════════════════════════════════════════════════════════
 * MARKNET - HLAVNÁ EXPRESS.JS APLIKÁCIA
 * ══════════════════════════════════════════════════════════════════════════════
 * 
 * Hlavný súbor Express.js servera pre MarkNET neurónovú sieť backend.
 * Konfigurácia middleware, routing, rate limiting, CORS, error handling
 * a graceful shutdown management. Integruje natívny C modul s REST API.
 * 
 * Komponenty:
 * - Express server s production-ready konfiguráciou
 * - Rate limiting pre API a predikcie
 * - CORS konfigurácia pre public access
 * - Request logging a error handling
 * - Neural network inicializácia a cleanup
 * - Graceful shutdown handling
 * 
 * Endpointy:
 * - GET  /                  - Root API info
 * - /api/health/*           - Health check routes
 * - /api/model/*            - Model management routes
 * - /api/predict/*          - Prediction routes
 * 
 * Autor: Samuel Sivák
 * Verzia: 1.0.0
 * ══════════════════════════════════════════════════════════════════════════════
 */

//=============================================================================
// IMPORTY A ZÁVISLOSTI
//=============================================================================

const express = require('express');
const cors = require('cors');
const rateLimit = require('express-rate-limit');
const path = require('path');
const { logger, requestLogger } = require('./logger');

/*
 * Import route modulov pre organizovanie API endpointov
 * Každý modul spravuje špecifickú funkčnosť aplikácie
 */
const healthRoutes = require('./health');
const modelRoutes = require('./model');
const predictRoutes = require('./predict');

//=============================================================================
// EXPRESS APLIKÁCIA INICIALIZÁCIA
//=============================================================================

const app = express();

/*
 * Konfigurácia proxy nastavenia pre deployment za reverse proxy
 * Potrebné pre správne IP logging a rate limiting
 */
app.set('trust proxy', 1);

//=============================================================================
// RATE LIMITING KONFIGURÁCIA
//=============================================================================

/*
 * Všeobecný rate limiter pre všetky API endpointy
 * Liberálnejšie nastavenie pre public access
 */
const limiter = rateLimit({
    windowMs: 15 * 60 * 1000,  // 15 minút okno
    max: 500,                  // 500 requestov na okno (zvýšené z 100)
    message: {
        error: 'Too many requests',
        message: 'Rate limit exceeded. Please try again later.',
        retryAfter: '15 minutes'
    },
    standardHeaders: true,
    legacyHeaders: false
});

/*
 * Špecializovaný rate limiter pre predikčné endpointy
 * Optimalizovaný pre real-time kreslenie a častejšie predikcie
 */
const predictionLimiter = rateLimit({
    windowMs: 1 * 60 * 1000,   // 1 minúta okno
    max: 60,                   // 60 predikcií za minútu (zvýšené z 20)
    message: {
        error: 'Prediction rate limit exceeded',
        message: 'Too many prediction requests. Please wait before trying again.',
        retryAfter: '1 minute'
    }
});

// Aplikácia rate limiting na všetky requesty
app.use(limiter);

//=============================================================================
// CORS KONFIGURÁCIA
//=============================================================================

/*
 * CORS nastavenie pre public access
 * Umožňuje prístup z akéhokoľvek origin pre demo účely
 */
app.use(cors({
    origin: '*',               // Povolenie všetkých origins pre public access
    methods: ['GET', 'POST'],
    allowedHeaders: ['Content-Type', 'Authorization'],
    credentials: false
}));

//=============================================================================
// MIDDLEWARE KONFIGURÁCIA
//=============================================================================

/*
 * Request logging middleware pre monitoring
 * Zaznamenáva všetky HTTP requesty s detailnými informáciami
 */
app.use(requestLogger);

/*
 * Body parsing middleware s veľkostnými limitmi
 * Podporuje JSON a URL-encoded dáta s increased limitmi pre obrazové dáta
 */
app.use(express.json({
    limit: '10mb',             // Zvýšené pre obrazové dáta
    type: 'application/json',
    verify: (req, res, buf) => {
        // Uloženie raw body pre debugging ak je potrebné
        req.rawBody = buf;
    }
}));

app.use(express.urlencoded({
    limit: '10mb',
    extended: true,
    parameterLimit: 1000
}));

/*
 * Request validation a tracking middleware
 * Pridáva unique request ID a loguje detaily požiadaviek
 */
app.use((req, res, next) => {
    // Pridanie request ID pre tracing
    req.requestId = `${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
    
    // Logovanie detailov požiadavky
    logger.debug('Detaily požiadavky', {
        requestId: req.requestId,
        method: req.method,
        url: req.url,
        contentLength: req.get('content-length'),
        userAgent: req.get('user-agent'),
        ip: req.ip
    });
    
    next();
});

//=============================================================================
// NEURAL NETWORK INICIALIZÁCIA
//=============================================================================

/*
 * Inicializácia natívneho C modulu neurónovej siete
 * Načítanie predtrénovaného MNIST modelu pri štarte servera
 */
const neuralNetwork = require('./index');
const modelPath = path.join(__dirname, '../models/mnist_model.bin');

logger.info('Inicializácia neurónovej siete', { modelPath });

try{
    const success = neuralNetwork.init(modelPath);
    if(success){
        logger.info('Model neurónovej siete úspešne načítaný');
        
        // Logovanie informácií o modeli
        try{
            const modelInfo = neuralNetwork.getModelInfo();
            logger.info('Informácie o modeli', modelInfo);
        } catch(infoError){
            logger.warn('Nepodarilo sa získať info o modeli', { 
                error: infoError.message 
            });
        }
    } else {
        logger.error('Nepodarilo sa načítať model neurónovej siete');
        throw new Error('Model initialization failed');
    }
} catch(error){
    logger.error('Chyba inicializácie neurónovej siete', { 
        error: error.message, 
        stack: error.stack 
    });
    process.exit(1);
}

//=============================================================================
// API ROUTING KONFIGURÁCIA
//=============================================================================

/*
 * Pripojenie route modulov s príslušnými middleware
 * Organizácia API podľa funkčnosti s rate limiting
 */
app.use('/api/health', healthRoutes);
app.use('/api/model', modelRoutes);
app.use('/api/predict', predictionLimiter, predictRoutes);

// Legacy route pre backward compatibility
app.use('/predict', predictionLimiter, predictRoutes);

//=============================================================================
// ROOT ENDPOINT
//=============================================================================

/*
 * GET /
 * Root endpoint poskytujúci základné informácie o API
 * Dokumentácia dostupných endpointov a service info
 */
app.get('/', (req, res) => {
    res.json({
        service: 'Neural Network Backend',
        version: '1.0.0',
        description: 'Backend API for MNIST digit recognition',
        endpoints: {
            health: '/api/health',
            modelInfo: '/api/model/info',
            listModels: '/api/model/models', 
            predict: '/api/predict',
            batchPredict: '/api/predict/batch'
        },
        documentation: 'See README.md for API documentation',
        timestamp: new Date().toISOString()
    });
});

//=============================================================================
// ERROR HANDLING MIDDLEWARE
//=============================================================================

/*
 * 404 handler pre neexistujúce routes
 * Loguje neplatné requesty a poskytuje užitočné informácie
 */
app.use('*', (req, res) => {
    logger.warn('404 - Route nenájdená', { 
        method: req.method, 
        url: req.url,
        ip: req.ip
    });
    
    res.status(404).json({
        error: 'Route not found',
        message: `${req.method} ${req.url} is not a valid endpoint`,
        availableEndpoints: [
            'GET /',
            'GET /api/health',
            'GET /api/model/info',
            'GET /api/model/models',
            'POST /api/predict',
            'POST /api/predict/batch'
        ]
    });
});

/*
 * Globálny error handler pre neočakávané chyby
 * Zabezpečuje že server neprestane fungovať a neodhalí citlivé informácie
 */
app.use((error, req, res, next) => {
    logger.error('Neošetrená chyba', {
        error: error.message,
        stack: error.stack,
        url: req.url,
        method: req.method,
        requestId: req.requestId
    });
    
    // Neodhaľovanie error detailov v produkcii
    const isDevelopment = process.env.NODE_ENV === 'development';
    
    res.status(error.status || 500).json({
        error: 'Internal server error',
        message: isDevelopment ? error.message : 'An unexpected error occurred',
        requestId: req.requestId,
        timestamp: new Date().toISOString(),
        ...(isDevelopment && { stack: error.stack })
    });
});

//=============================================================================
// GRACEFUL SHUTDOWN HANDLING
//=============================================================================

/*
 * SIGTERM signal handler pre graceful shutdown
 * Čistenie zdrojov neurónovej siete pred ukončením
 */
process.on('SIGTERM', () => {
    logger.info('SIGTERM prijatý, graceful shutdown');
    
    // Cleanup neurónovej siete
    try{
        neuralNetwork.cleanup();
        logger.info('Neurónová sieť vyčistená');
    } catch(error){
        logger.error('Chyba pri cleanup neurónovej siete', { 
            error: error.message 
        });
    }
    
    process.exit(0);
});

/*
 * SIGINT signal handler (Ctrl+C) pre graceful shutdown
 * Identické spracovanie ako SIGTERM
 */
process.on('SIGINT', () => {
    logger.info('SIGINT prijatý, graceful shutdown');
    
    // Cleanup neurónovej siete
    try{
        neuralNetwork.cleanup();
        logger.info('Neurónová sieť vyčistená');
    } catch(error){
        logger.error('Chyba pri cleanup neurónovej siete', { 
            error: error.message 
        });
    }
    
    process.exit(0);
});

//=============================================================================
// SERVER ŠTART A KONFIGURÁCIA
//=============================================================================

/*
 * Štart Express servera s konfigurovateľným HOST a PORT
 * Production-ready nastavenia s detailným loggingom
 */
const PORT = process.env.PORT || 3000;
const HOST = process.env.HOST || '0.0.0.0';

const server = app.listen(PORT, HOST, () => {
    logger.info(`Server spustený na http://${HOST}:${PORT}`, {
        port: PORT,
        host: HOST,
        environment: process.env.NODE_ENV || 'development',
        processId: process.pid
    });
});

/*
 * Server error handler pre kritické chyby
 * Ukončenie procesu pri neschopnosti servera fungovať
 */
server.on('error', (error) => {
    logger.error('Chyba servera', { 
        error: error.message, 
        stack: error.stack 
    });
    process.exit(1);
});

//=============================================================================
// EXPORT MODULU
//=============================================================================

module.exports = app;
