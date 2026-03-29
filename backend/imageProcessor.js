// Spracovanie obrazu

// Normalizácia pixelov
function normalizePixels(pixels){
    return pixels.map(pixel => Math.max(0, Math.min(1, pixel)));
}

// Výpočet ťažiska
function calculateCenterOfMass(pixels){
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
    
    if(totalMass === 0) return { x: 14, y: 14 };
    
    return {
        x: xSum / totalMass,
        y: ySum / totalMass
    };
}

// Centrovanie obrazu
function centerImage(pixels){
    const centerOfMass = calculateCenterOfMass(pixels);
    const shiftX = Math.round(14 - centerOfMass.x);
    const shiftY = Math.round(14 - centerOfMass.y);
    
    if(shiftX === 0 && shiftY === 0) return [...pixels];
    
    const centered = new Array(784).fill(0);
    
    for(let y = 0; y < 28; y++){
        for(let x = 0; x < 28; x++){
            const origX = x - shiftX;
            const origY = y - shiftY;
            
            if(origX >= 0 && origX < 28 && origY >= 0 && origY < 28){
                centered[y * 28 + x] = pixels[origY * 28 + origX];
            }
        }
    }
    
    return centered;
}

// Aplikácia prahu
function applyThreshold(pixels, threshold){
    return pixels.map(p => p > threshold ? p : 0);
}

// Preprocessing pipeline
function preprocessImage(pixels, options = {}){
    const { normalize = true, center = false, threshold = 0.0, applyThresh = false } = options;
    
    let processed = [...pixels];
    
    if(normalize) processed = normalizePixels(processed);
    if(applyThresh && threshold > 0) processed = applyThreshold(processed, threshold);
    if(center) processed = centerImage(processed);
    
    return processed;
}

// Validácia vstupu
function validateImageInput(pixels){
    const errors = [];
    
    if(!Array.isArray(pixels)){
        errors.push('Input must be an array');
    } else if(pixels.length !== 784){
        errors.push(`Expected 784 pixels, got ${pixels.length}`);
    } else {
        for(let i = 0; i < pixels.length; i++){
            if(typeof pixels[i] !== 'number' || isNaN(pixels[i])){
                errors.push(`Invalid pixel at index ${i}`);
                break;
            }
        }
    }
    
    return {
        valid: errors.length === 0,
        errors
    };
}

module.exports = {
    preprocessImage,
    validateImageInput
};
