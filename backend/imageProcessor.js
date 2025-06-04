const { logger } = require('./logger');

/**
 * Normalize pixel values to 0-1 range
 * @param {Array} pixels - Array of pixel values
 * @returns {Array} Normalized pixel values
 */
function normalizePixels(pixels){
    if(!Array.isArray(pixels)){
        throw new Error('Pixels must be an array');
    }
    
    return pixels.map(pixel => {
        const normalized = Math.max(0, Math.min(1, pixel));
        return normalized;
    });
}

/**
 * Calculate the center of mass of the image
 * @param {Array} pixels - 28x28 pixel array
 * @returns {Object} {x, y} coordinates of center of mass
 */
function calculateCenterOfMass(pixels){
    if(pixels.length !== 784){
        throw new Error('Expected 784 pixels (28x28)');
    }
    
    let totalMass = 0;
    let xSum = 0;
    let ySum = 0;
    
    for(let y = 0; y < 28; y++){
        for(let x = 0; x < 28; x++){
            const idx = y * 28 + x;
            const pixelValue = pixels[idx];
            totalMass += pixelValue;
            xSum += x * pixelValue;
            ySum += y * pixelValue;
        }
    }
    
    if(totalMass === 0){
        return { x: 14, y: 14 }; // Center if no content
    }
    
    return {
        x: xSum / totalMass,
        y: ySum / totalMass
    };
}

/**
 * Center the image based on center of mass
 * @param {Array} pixels - 28x28 pixel array
 * @returns {Array} Centered pixel array
 */
function centerImage(pixels){
    if(pixels.length !== 784){
        throw new Error('Expected 784 pixels (28x28)');
    }
    
    const centerOfMass = calculateCenterOfMass(pixels);
    const targetCenter = { x: 14, y: 14 };
    
    const shiftX = Math.round(targetCenter.x - centerOfMass.x);
    const shiftY = Math.round(targetCenter.y - centerOfMass.y);
    
    // If no shift needed, return original
    if(shiftX === 0 && shiftY === 0){
        return [...pixels];
    }
    
    const centeredPixels = new Array(784).fill(0);
    
    for(let y = 0; y < 28; y++){
        for(let x = 0; x < 28; x++){
            const originalX = x - shiftX;
            const originalY = y - shiftY;
            
            // Check if original position is within bounds
            if(originalX >= 0 && originalX < 28 && originalY >= 0 && originalY < 28){
                const originalIdx = originalY * 28 + originalX;
                const newIdx = y * 28 + x;
                centeredPixels[newIdx] = pixels[originalIdx];
            }
        }
    }
    
    return centeredPixels;
}

/**
 * Apply threshold to improve contrast
 * @param {Array} pixels - Pixel array
 * @param {number} threshold - Threshold value (0-1)
 * @returns {Array} Thresholded pixel array
 */
function applyThreshold(pixels, threshold = 0.1){
    return pixels.map(pixel => {
        return pixel > threshold ? pixel : 0;
    });
}

/**
 * Complete preprocessing pipeline
 * @param {Array} pixels - Raw pixel array (784 elements)
 * @param {Object} options - Preprocessing options
 * @returns {Array} Processed pixel array
 */
function preprocessImage(pixels, options = {}){
    try{
        const {
            normalize = true,
            center = true,
            threshold = 0.1,
            applyThresh = true
        } = options;
        
        logger.debug('Starting image preprocessing', {
            pixelCount: pixels.length,
            options
        });
        
        let processed = [...pixels];
        
        // Step 1: Normalize to 0-1 range
        if(normalize){
            processed = normalizePixels(processed);
            logger.debug('Applied normalization');
        }
        
        // Step 2: Apply threshold
        if(applyThresh && threshold > 0){
            processed = applyThreshold(processed, threshold);
            logger.debug('Applied threshold', { threshold });
        }
        
        // Step 3: Center the image
        if(center){
            processed = centerImage(processed);
            logger.debug('Applied centering');
        }
        
        // Log statistics
        const nonZeroPixels = processed.filter(p => p > 0).length;
        const avgValue = processed.reduce((sum, p) => sum + p, 0) / processed.length;
        
        logger.debug('Preprocessing complete', {
            nonZeroPixels,
            avgValue,
            minValue: Math.min(...processed),
            maxValue: Math.max(...processed)
        });
        
        return processed;
        
    } catch(error){
        logger.error('Image preprocessing failed', { error: error.message });
        throw error;
    }
}

/**
 * Validate input image data
 * @param {Array} pixels - Pixel array to validate
 * @returns {Object} Validation result
 */
function validateImageInput(pixels){
    const errors = [];
    
    if(!Array.isArray(pixels)){
        errors.push('Input must be an array');
    } else {
        if(pixels.length !== 784){
            errors.push(`Expected 784 pixels, got ${pixels.length}`);
        }
        
        for(let i = 0; i < pixels.length; i++){
            const pixel = pixels[i];
            if(typeof pixel !== 'number' || isNaN(pixel)){
                errors.push(`Invalid pixel value at index ${i}: ${pixel}`);
                break; // Only report first invalid pixel
            }
        }
    }
    
    return {
        valid: errors.length === 0,
        errors
    };
}

module.exports = {
    normalizePixels,
    calculateCenterOfMass,
    centerImage,
    applyThreshold,
    preprocessImage,
    validateImageInput
};
