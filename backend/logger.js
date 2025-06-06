/*
 * ══════════════════════════════════════════════════════════════════════════════
 * MARKNET - WINSTON LOGGER KONFIGURÁCIA
 * ══════════════════════════════════════════════════════════════════════════════
 * 
 * Konfigurácia profesionálneho logovania pre MarkNET backend server.
 * Používa Winston library pre štruktúrované loggovanie s rotáciou súborov,
 * rôznymi úrovňami logovania a HTTP request trackingom.
 * 
 * Funkcie:
 * - Štruktúrované JSON loggovanie
 * - Automatická rotácia log súborov
 * - Console výstup pre development
 * - HTTP request middleware
 * 
 * Autor: Samuel Sivák
 * Verzia: 1.0.0
 * ══════════════════════════════════════════════════════════════════════════════
 */

//=============================================================================
// IMPORTY A ZÁVISLOSTI
//=============================================================================

const winston = require('winston');
const path = require('path');
const fs = require('fs');

//=============================================================================
// KONFIGURÁCIA ADRESÁROV A SÚBOROV
//=============================================================================

/*
 * Vytvorenie adresára pre log súbory ak neexistuje
 * Zabezpečuje existenciu logs/ adresára s recursive vytvorením
 */
const logsDir = path.join(__dirname, '../logs');
if(!fs.existsSync(logsDir)){
    fs.mkdirSync(logsDir, { recursive: true });
}

//=============================================================================
// FORMÁTOVANIE LOGOV
//=============================================================================

/*
 * Definícia formátu pre log záznamy
 * Kombinuje timestamp, error stacky a JSON formátovanie
 * pre štruktúrované spracovanie logov
 */
const logFormat = winston.format.combine(
    winston.format.timestamp({ format: 'YYYY-MM-DD HH:mm:ss' }),
    winston.format.errors({ stack: true }),
    winston.format.json()
);

//=============================================================================
// WINSTON LOGGER INŠTANCIA
//=============================================================================

/*
 * Vytvorenie hlavnej logger inštancie s konfiguráciou transportov
 * Podporuje rôzne úrovne logovania a automatickú rotáciu súborov
 */
const logger = winston.createLogger({
    level: process.env.LOG_LEVEL || 'info',
    format: logFormat,
    defaultMeta: { service: 'neural-network-backend' },
    transports: [
        /*
         * Error log transport - zapisuje len error a vyššie úrovne
         * Maximálna veľkosť súboru: 5MB, rotácia na 5 súborov
         */
        new winston.transports.File({ 
            filename: path.join(logsDir, 'error.log'), 
            level: 'error',
            maxsize: 5242880, // 5MB
            maxFiles: 5
        }),
        
        /*
         * Combined log transport - zapisuje všetky úrovne
         * Maximálna veľkosť súboru: 5MB, rotácia na 5 súborov
         */
        new winston.transports.File({ 
            filename: path.join(logsDir, 'combined.log'),
            maxsize: 5242880, // 5MB
            maxFiles: 5
        })
    ]
});

//=============================================================================
// DEVELOPMENT CONSOLE TRANSPORT
//=============================================================================

/*
 * Pridanie console transportu pre development prostredie
 * Poskytuje farebný a čitateľný výstup do konzoly
 */
if(process.env.NODE_ENV !== 'production'){
    logger.add(new winston.transports.Console({
        format: winston.format.combine(
            winston.format.colorize(),
            winston.format.simple()
        )
    }));
}

//=============================================================================
// EXPRESS MIDDLEWARE PRE HTTP REQUESTOV
//=============================================================================

/*
 * Express middleware pre automatické logovanie HTTP requestov
 * Zaznamenáva metódu, URL, status kód, trvanie a klientské informácie
 * 
 * Parametre:
 * - req: Express request objekt
 * - res: Express response objekt  
 * - next: Express next funkcia
 */
const requestLogger = (req, res, next) => {
    const start = Date.now();
    
    // Listener pre ukončenie response
    res.on('finish', () => {
        const duration = Date.now() - start;
        logger.info('HTTP Request', {
            method: req.method,
            url: req.url,
            statusCode: res.statusCode,
            duration: `${duration}ms`,
            userAgent: req.get('User-Agent'),
            ip: req.ip
        });
    });
    
    next();
};

//=============================================================================
// EXPORT MODULOV
//=============================================================================

module.exports = {
    logger,           // Hlavná logger inštancia
    requestLogger     // HTTP request middleware
};
