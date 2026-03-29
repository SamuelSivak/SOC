// Hlavná Express.js aplikácia

const express = require('express');
const cors = require('cors');
const rateLimit = require('express-rate-limit');
const path = require('path');
const { logger, requestLogger } = require('./logger');
const modelRoutes = require('./model');
const predictRoutes = require('./predict');

const app = express();

app.set('trust proxy', 1);

// Rate limiting
const limiter = rateLimit({
    windowMs: 1 * 60 * 1000,
    max: 60,
    message: {
        error: 'Too many requests',
        message: 'Rate limit exceeded. Please try again later.'
    }
});

app.use(limiter);
app.use(cors({ origin: '*', methods: ['GET', 'POST'] }));
app.use(requestLogger);
app.use(express.json({ limit: '10mb' }));

// Inicializácia neurónovej siete
const neuralNetwork = require('./index');
const modelPath = path.join(__dirname, '../models/mnist_model.bin');

try{
    const success = neuralNetwork.init(modelPath);
    if(!success){
        throw new Error('Model initialization failed');
    }
    logger.info('Model načítaný');
} catch(error){
    logger.error('Chyba inicializácie', { error: error.message });
    process.exit(1);
}

// API routing
app.use('/api/model', modelRoutes);
app.use('/api/predict', limiter, predictRoutes);

// Root endpoint
app.get('/', (req, res) => {
    res.json({
        service: 'Neural Network Backend',
        version: '1.0.0',
        endpoints: {
            listModels: '/api/model/models',
            switchModel: '/api/model/switch',
            predict: '/api/predict'
        }
    });
});

// 404 handler
app.use('*', (req, res) => {
    res.status(404).json({
        error: 'Route not found',
        message: `${req.method} ${req.url} is not a valid endpoint`
    });
});

// Error handler
app.use((error, req, res, next) => {
    logger.error('Chyba', { error: error.message });
    res.status(500).json({
        error: 'Internal server error',
        message: error.message
    });
});

// Graceful shutdown
const shutdown = () => {
    try{
        neuralNetwork.cleanup();
        logger.info('Shutdown dokončený');
    } catch(error){
        logger.error('Chyba pri shutdown', { error: error.message });
    }
    process.exit(0);
};

process.on('SIGTERM', shutdown);
process.on('SIGINT', shutdown);

// Štart servera
const PORT = process.env.PORT || 3000;
const HOST = process.env.HOST || '0.0.0.0';

const server = app.listen(PORT, HOST, () => {
    logger.info(`Server spustený na http://${HOST}:${PORT}`);
});

server.on('error', (error) => {
    logger.error('Chyba servera', { error: error.message });
    process.exit(1);
});

module.exports = app;
