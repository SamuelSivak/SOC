/*
 * ══════════════════════════════════════════════════════════════════════════════
 * MARKNET - SPRACOVANIE OBRAZOVÝCH DÁT
 * ══════════════════════════════════════════════════════════════════════════════
 * 
 * Implementácia funkcií pre predspracovanie obrazových dát pre neurónovú sieť.
 * Obsahuje normalizáciu, centrovanie, threshold aplikáciu a validáciu vstupných
 * dát pre MNIST digit recognition systém.
 * 
 * Funkcie:
 * - Normalizácia pixelov na rozsah 0-1
 * - Výpočet ťažiska obrazu a centrovanie
 * - Aplikácia prahu pre zlepšenie kontrastu
 * - Kompletný preprocessing pipeline
 * - Validácia vstupných dát
 * 
 * Autor: Samuel Sivák
 * Verzia: 1.0.0
 * ══════════════════════════════════════════════════════════════════════════════
 */

//=============================================================================
// IMPORTY A ZÁVISLOSTI
//=============================================================================

const { logger } = require('./logger');

//=============================================================================
// NORMALIZAČNÉ FUNKCIE
//=============================================================================

/*
 * Normalizácia hodnôt pixelov na rozsah 0-1
 * Zabezpečuje, že všetky pixelové hodnoty sú v správnom rozsahu
 * 
 * Parametre:
 * - pixels: pole pixelových hodnôt
 * 
 * Návratová hodnota: pole normalizovaných pixelových hodnôt
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

//=============================================================================
// FUNKCIE PRE CENTROVANIE OBRAZU
//=============================================================================

/*
 * Výpočet ťažiska (center of mass) obrazu
 * Určuje súradnice kde sa koncentruje najvačšia hmotnosť pixelov
 * 
 * Parametre:
 * - pixels: pole 784 pixelov (28x28)
 * 
 * Návratová hodnota: objekt {x, y} so súradnicami ťažiska
 */
function calculateCenterOfMass(pixels){
    if(pixels.length !== 784){
        throw new Error('Expected 784 pixels (28x28)');
    }
    
    let totalMass = 0;
    let xSum = 0;
    let ySum = 0;
    
    // Výpočet váženého súčtu súradníc
    for(let y = 0; y < 28; y++){
        for(let x = 0; x < 28; x++){
            const idx = y * 28 + x;
            const pixelValue = pixels[idx];
            totalMass += pixelValue;
            xSum += x * pixelValue;
            ySum += y * pixelValue;
        }
    }
    
    // Ak nie je žiadny obsah, vráť center
    if(totalMass === 0){
        return { x: 14, y: 14 };
    }
    
    return {
        x: xSum / totalMass,
        y: ySum / totalMass
    };
}

/*
 * Centrovanie obrazu na základe ťažiska
 * Posúva obraz tak, aby ťažisko bolo v strede plátna
 * 
 * Parametre:
 * - pixels: pole 784 pixelov (28x28)
 * 
 * Návratová hodnota: pole centrovaných pixelov
 */
function centerImage(pixels){
    if(pixels.length !== 784){
        throw new Error('Expected 784 pixels (28x28)');
    }
    
    const centerOfMass = calculateCenterOfMass(pixels);
    const targetCenter = { x: 14, y: 14 };
    
    // Výpočet potrebného posunu
    const shiftX = Math.round(targetCenter.x - centerOfMass.x);
    const shiftY = Math.round(targetCenter.y - centerOfMass.y);
    
    // Ak nie je potrebný posun, vráť originál
    if(shiftX === 0 && shiftY === 0){
        return [...pixels];
    }
    
    const centeredPixels = new Array(784).fill(0);
    
    // Aplikácia posunu na každý pixel
    for(let y = 0; y < 28; y++){
        for(let x = 0; x < 28; x++){
            const originalX = x - shiftX;
            const originalY = y - shiftY;
            
            // Kontrola či originálna pozícia je v rámci hraníc
            if(originalX >= 0 && originalX < 28 && originalY >= 0 && originalY < 28){
                const originalIdx = originalY * 28 + originalX;
                const newIdx = y * 28 + x;
                centeredPixels[newIdx] = pixels[originalIdx];
            }
        }
    }
    
    return centeredPixels;
}

//=============================================================================
// THRESHOLD A KONTRASTOVÉ FUNKCIE
//=============================================================================

/*
 * Aplikácia prahu pre zlepšenie kontrastu
 * Odstraňuje slabé pixely pod určitou hranicou
 * 
 * Parametre:
 * - pixels: pole pixelových hodnôt
 * - threshold: prahovú hodnota (0-1)
 * 
 * Návratová hodnota: pole pixelov po aplikácii prahu
 */
function applyThreshold(pixels, threshold = 0.1){
    return pixels.map(pixel => {
        return pixel > threshold ? pixel : 0;
    });
}

//=============================================================================
// HLAVNÝ PREPROCESSING PIPELINE
//=============================================================================

/*
 * Kompletný preprocessing pipeline pre obrazové dáta
 * Vykonáva sekvenčné spracovanie: normalizácia -> threshold -> centrovanie
 * 
 * Parametre:
 * - pixels: surové pole 784 pixelov
 * - options: objekt s nastaveniami {normalize, center, threshold, applyThresh}
 * 
 * Návratová hodnota: pole spracovaných pixelov pripravených pre NN
 */
function preprocessImage(pixels, options = {}){
    try{
        const {
            normalize = true,
            center = true,
            threshold = 0.1,
            applyThresh = true
        } = options;
        
        logger.debug('Začiatok preprocessingu obrazu', {
            pixelCount: pixels.length,
            options
        });
        
        let processed = [...pixels];
        
        // Krok 1: Normalizácia na rozsah 0-1
        if(normalize){
            processed = normalizePixels(processed);
            logger.debug('Aplikovaná normalizácia');
        }
        
        // Krok 2: Aplikácia prahu
        if(applyThresh && threshold > 0){
            processed = applyThreshold(processed, threshold);
            logger.debug('Aplikovaný prah', { threshold });
        }
        
        // Krok 3: Centrovanie obrazu
        if(center){
            processed = centerImage(processed);
            logger.debug('Aplikované centrovanie');
        }
        
        // Logovanie štatistík
        const nonZeroPixels = processed.filter(p => p > 0).length;
        const avgValue = processed.reduce((sum, p) => sum + p, 0) / processed.length;
        
        logger.debug('Preprocessing dokončený', {
            nonZeroPixels,
            avgValue,
            minValue: Math.min(...processed),
            maxValue: Math.max(...processed)
        });
        
        return processed;
        
    } catch(error){
        logger.error('Preprocessing obrazu zlyhal', { error: error.message });
        throw error;
    }
}

//=============================================================================
// VALIDAČNÉ FUNKCIE
//=============================================================================

/*
 * Validácia vstupných obrazových dát
 * Kontroluje formát, veľkosť a hodnoty pixelov
 * 
 * Parametre:
 * - pixels: pole pixelov na validáciu
 * 
 * Návratová hodnota: objekt {valid: boolean, errors: string[]}
 */
function validateImageInput(pixels){
    const errors = [];
    
    // Kontrola typu
    if(!Array.isArray(pixels)){
        errors.push('Input must be an array');
    } else {
        // Kontrola veľkosti
        if(pixels.length !== 784){
            errors.push(`Expected 784 pixels, got ${pixels.length}`);
        }
        
        // Kontrola hodnôt pixelov
        for(let i = 0; i < pixels.length; i++){
            const pixel = pixels[i];
            if(typeof pixel !== 'number' || isNaN(pixel)){
                errors.push(`Invalid pixel value at index ${i}: ${pixel}`);
                break; // Hlás len prvý neplatný pixel
            }
        }
    }
    
    return {
        valid: errors.length === 0,
        errors
    };
}

//=============================================================================
// EXPORT MODULOV
//=============================================================================

module.exports = {
    normalizePixels,        // Normalizácia pixelov
    calculateCenterOfMass,  // Výpočet ťažiska
    centerImage,           // Centrovanie obrazu
    applyThreshold,        // Aplikácia prahu
    preprocessImage,       // Hlavný preprocessing pipeline
    validateImageInput     // Validácia vstupov
};
