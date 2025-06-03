const express = require('express');
const cors = require('cors');
const path = require('path');
const app = express();

// Enable CORS for frontend
app.use(cors());

// Increase JSON payload limit to handle the image data
app.use(express.json({limit: '50mb'}));

// Load neural network native module
const neuralNetwork = require('./native/build/Release/neural_network');

// Initialize neural network with model
const modelPath = path.join(__dirname, '../models/mnist_model.bin');
console.log('Loading model from:', modelPath);
const success = neuralNetwork.init(modelPath);

if (!success) {
    console.error('Failed to load neural network model');
    process.exit(1);
}
console.log('Neural network model loaded successfully');

// Prediction endpoint
app.post('/predict', (req, res) => {
    try {
        const { pixels } = req.body;
        
        if (!pixels || !Array.isArray(pixels) || pixels.length !== 784) {
            console.error('Invalid input:', {
                hasPixels: !!pixels,
                isArray: Array.isArray(pixels),
                length: pixels ? pixels.length : 0
            });
            return res.status(400).json({ 
                error: 'Invalid input. Expected array of 784 pixels.' 
            });
        }

        console.log('Making prediction with', pixels.length, 'pixels');
        console.log('Sample pixel values:', pixels.slice(0, 10));

        // Get model prediction
        const predictions = neuralNetwork.predict(pixels);
        console.log('Raw predictions:', predictions);
        
        if (!predictions || !Array.isArray(predictions)) {
            console.error('Invalid predictions:', predictions);
            return res.status(500).json({ error: 'Invalid prediction result' });
        }

        // Find the digit with highest probability
        const prediction = predictions.reduce((maxIndex, current, currentIndex, arr) => 
            current > arr[maxIndex] ? currentIndex : maxIndex, 0);

        console.log('Final prediction:', prediction);
        res.json({ prediction, probabilities: predictions });
    } catch (error) {
        console.error('Prediction error:', error);
        res.status(500).json({ error: 'Failed to get prediction: ' + error.message });
    }
});

// Start server
const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
});
