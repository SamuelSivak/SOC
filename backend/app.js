const express = require('express');
const cors = require('cors');
const rateLimit = require('express-rate-limit');
const path = require('path');
const { logger, requestLogger } = require('./logger');

// Import routes
const healthRoutes = require('./health');
const modelRoutes = require('./model');
const predictRoutes = require('./predict');

const app = express();

// Trust proxy if behind reverse proxy
app.set('trust proxy', 1);

// General rate limiting - more lenient
const limiter = rateLimit({
    windowMs: 15 * 60 * 1000, // 15 minutes
    max: 500, // Increased from 100 to 500 requests per windowMs
    message: {
        error: 'Too many requests',
        message: 'Rate limit exceeded. Please try again later.',
        retryAfter: '15 minutes'
    },
    standardHeaders: true,
    legacyHeaders: false
});

// Apply rate limiting to all requests
app.use(limiter);

// More lenient rate limiting for prediction endpoints (real-time drawing)
const predictionLimiter = rateLimit({
    windowMs: 1 * 60 * 1000, // 1 minute
    max: 60, // Increased from 20 to 60 predictions per minute for real-time drawing
    message: {
        error: 'Prediction rate limit exceeded',
        message: 'Too many prediction requests. Please wait before trying again.',
        retryAfter: '1 minute'
    }
});

// CORS configuration for public access
app.use(cors({
    origin: '*', // Allow all origins for public access
    methods: ['GET', 'POST'],
    allowedHeaders: ['Content-Type', 'Authorization'],
    credentials: false
}));

// Request logging middleware
app.use(requestLogger);

// Body parsing middleware with size limits
app.use(express.json({
    limit: '10mb', // Increased for image data
    type: 'application/json',
    verify: (req, res, buf) => {
        // Store raw body for debugging if needed
        req.rawBody = buf;
    }
}));

app.use(express.urlencoded({
    limit: '10mb',
    extended: true,
    parameterLimit: 1000
}));

// Request validation middleware
app.use((req, res, next) => {
    // Add request ID for tracing
    req.requestId = `${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
    
    // Log request details
    logger.debug('Request details', {
        requestId: req.requestId,
        method: req.method,
        url: req.url,
        contentLength: req.get('content-length'),
        userAgent: req.get('user-agent'),
        ip: req.ip
    });
    
    next();
});

// Initialize neural network
const neuralNetwork = require('./index');
const modelPath = path.join(__dirname, '../models/mnist_model.bin');

logger.info('Initializing neural network', { modelPath });

try{
    const success = neuralNetwork.init(modelPath);
    if(success){
        logger.info('Neural network model loaded successfully');
        
        // Log model information
        try{
            const modelInfo = neuralNetwork.getModelInfo();
            logger.info('Model information', modelInfo);
        } catch(infoError){
            logger.warn('Could not retrieve model info', { error: infoError.message });
        }
    } else {
        logger.error('Failed to load neural network model');
        throw new Error('Model initialization failed');
    }
} catch(error){
    logger.error('Neural network initialization error', { 
        error: error.message, 
        stack: error.stack 
    });
    process.exit(1);
}

// API Routes
app.use('/api/health', healthRoutes);
app.use('/api/model', modelRoutes);
app.use('/api/predict', predictionLimiter, predictRoutes);

// Legacy route for backwards compatibility
app.use('/predict', predictionLimiter, predictRoutes);

// Root endpoint
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

// 404 handler
app.use('*', (req, res) => {
    logger.warn('404 - Route not found', { 
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

// Global error handler
app.use((error, req, res, next) => {
    logger.error('Unhandled error', {
        error: error.message,
        stack: error.stack,
        url: req.url,
        method: req.method,
        requestId: req.requestId
    });
    
    // Don't leak error details in production
    const isDevelopment = process.env.NODE_ENV === 'development';
    
    res.status(error.status || 500).json({
        error: 'Internal server error',
        message: isDevelopment ? error.message : 'An unexpected error occurred',
        requestId: req.requestId,
        timestamp: new Date().toISOString(),
        ...(isDevelopment && { stack: error.stack })
    });
});

// Graceful shutdown handling
process.on('SIGTERM', () => {
    logger.info('SIGTERM received, shutting down gracefully');
    
    // Cleanup neural network
    try{
        neuralNetwork.cleanup();
        logger.info('Neural network cleaned up');
    } catch(error){
        logger.error('Error during neural network cleanup', { error: error.message });
    }
    
    process.exit(0);
});

process.on('SIGINT', () => {
    logger.info('SIGINT received, shutting down gracefully');
    
    // Cleanup neural network
    try{
        neuralNetwork.cleanup();
        logger.info('Neural network cleaned up');
    } catch(error){
        logger.error('Error during neural network cleanup', { error: error.message });
    }
    
    process.exit(0);
});

// Start server
const PORT = process.env.PORT || 3000;
const HOST = process.env.HOST || '0.0.0.0';

const server = app.listen(PORT, HOST, () => {
    logger.info(`Server running on http://${HOST}:${PORT}`, {
        port: PORT,
        host: HOST,
        environment: process.env.NODE_ENV || 'development',
        processId: process.pid
    });
});

// Handle server errors
server.on('error', (error) => {
    logger.error('Server error', { error: error.message, stack: error.stack });
    process.exit(1);
});

module.exports = app;
