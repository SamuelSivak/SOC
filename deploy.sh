#!/bin/bash

#==============================================================================
# MARKNET - AUTOMATED DEPLOYMENT SCRIPT
#==============================================================================
# Automatizované nasadenie MarkNET aplikácie na Digital Ocean server
# s SSL certifikátom a Nginx reverse proxy
#
# Server: samuelsivaksoc.xyz (178.128.245.99)
# Backend: Node.js + Express na porte 3000
# Frontend: Statické HTML/CSS/JS súbory
# SSL: Let's Encrypt certifikát
#
# Autor: Samuel Sivák
# Verzia: 2.0
#==============================================================================

# Farebné výpisy pre lepšiu čitateľnosť
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Konfiguračné premenné pre nasadenie
SERVER_IP="178.128.245.99"
SERVER_USER="root"
SERVER_PASSWORD="123SamuelSivak"
DOMAIN="samuelsivaksoc.xyz"

# Cesty na serveri - OPRAVENÉ UMIESTNENIE PRE NGINX
BACKEND_DIR="/root/marknet/backend"
FRONTEND_DIR="/var/www/marknet/frontend"
MODELS_DIR="/root/marknet/models"
PROJECT_ROOT="/root/marknet"

echo -e "${CYAN}═══════════════════════════════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}MARKNET DEPLOYMENT - ${DOMAIN}${NC}"
echo -e "${CYAN}═══════════════════════════════════════════════════════════════════════════════${NC}"

#==============================================================================
# FUNKCIE PRE DEPLOYMENT
#==============================================================================

# Funkcia pre SSH príkazy s automatickým heslom
ssh_cmd(){
    sshpass -p "${SERVER_PASSWORD}" ssh -o StrictHostKeyChecking=no "${SERVER_USER}@${SERVER_IP}" "$1"
}

# Funkcia pre SCP upload s automatickým heslom
scp_upload(){
    sshpass -p "${SERVER_PASSWORD}" scp -o StrictHostKeyChecking=no -r "$1" "${SERVER_USER}@${SERVER_IP}:$2"
}

#==============================================================================
# KROK 1: KONTROLA LOKÁLNYCH SÚBOROV
#==============================================================================

echo -e "${YELLOW}[STEP]${NC} Kontrola lokálnych súborov..."

# Kontrola existencie potrebných adresárov
if [ ! -d "backend" ] || [ ! -d "frontend" ] || [ ! -d "models" ]; then
    echo -e "${RED}[ERROR]${NC} Chýbajú potrebné adresáre (backend/frontend/models)"
    exit 1
fi

echo -e "${GREEN}[INFO]${NC} Všetky potrebné adresáre nájdené"

#==============================================================================
# KROK 2: VYTVORENIE ADRESÁROV NA SERVERI
#==============================================================================

echo -e "${YELLOW}[STEP]${NC} Vytvorenie adresárov na serveri..."

ssh_cmd "mkdir -p ${BACKEND_DIR} ${FRONTEND_DIR} ${MODELS_DIR} ${FRONTEND_DIR}/assets ${FRONTEND_DIR}/js"

#==============================================================================
# KROK 3: UPLOAD FRONTEND SÚBOROV
#==============================================================================

echo -e "${YELLOW}[STEP]${NC} Nahrávanie frontend súborov..."

scp_upload "frontend/assets/main.css" "${FRONTEND_DIR}/assets/main.css"
scp_upload "frontend/index.html" "${FRONTEND_DIR}/index.html"
scp_upload "frontend/js/main.js" "${FRONTEND_DIR}/js/main.js"

#==============================================================================
# KROK 4: UPLOAD BACKEND SÚBOROV
#==============================================================================

echo -e "${YELLOW}[STEP]${NC} Nahrávanie backend súborov..."

# Nahranie všetkých backend súborov
scp_upload "backend/app.js" "${BACKEND_DIR}/app.js"
scp_upload "backend/binding.gyp" "${BACKEND_DIR}/binding.gyp"
scp_upload "backend/health.js" "${BACKEND_DIR}/health.js"
scp_upload "backend/imageProcessor.js" "${BACKEND_DIR}/imageProcessor.js"
scp_upload "backend/index.js" "${BACKEND_DIR}/index.js"
scp_upload "backend/logger.js" "${BACKEND_DIR}/logger.js"
scp_upload "backend/logs/" "${BACKEND_DIR}/logs/"
scp_upload "backend/model.js" "${BACKEND_DIR}/model.js"
scp_upload "backend/neural_network_wrapper.c" "${BACKEND_DIR}/neural_network_wrapper.c"
scp_upload "backend/package.json" "${BACKEND_DIR}/package.json"
scp_upload "backend/package-lock.json" "${BACKEND_DIR}/package-lock.json"
scp_upload "backend/predict.js" "${BACKEND_DIR}/predict.js"

#==============================================================================
# KROK 5: UPLOAD C ZDROJOVÝCH SÚBOROV A HEADER FILES
#==============================================================================

echo -e "${YELLOW}[STEP]${NC} Nahrávanie C zdrojových súborov..."

ssh_cmd "mkdir -p ${PROJECT_ROOT}/src ${PROJECT_ROOT}/include"
scp_upload "src/" "${PROJECT_ROOT}/"
scp_upload "include/" "${PROJECT_ROOT}/"

#==============================================================================
# KROK 6: UPLOAD NEURÁLNYCH MODELOV
#==============================================================================

echo -e "${YELLOW}[STEP]${NC} Nahrávanie neurálnych modelov..."

# Nahranie jednotlivých model súborov
scp_upload "models/mnist_model.bin" "${MODELS_DIR}/mnist_model.bin"
scp_upload "models/mnist_model_300.bin" "${MODELS_DIR}/mnist_model_300.bin"
scp_upload "models/mnist_model_1500.bin" "${MODELS_DIR}/mnist_model_1500.bin"
scp_upload "models/mnist_model_best.bin" "${MODELS_DIR}/mnist_model_best.bin"
scp_upload "models/mnist_model_final.bin" "${MODELS_DIR}/mnist_model_final.bin"

#==============================================================================
# KROK 7: INŠTALÁCIA NODE.JS ZÁVISLOSTÍ
#==============================================================================

echo -e "${YELLOW}[STEP]${NC} Inštalácia Node.js závislostí na serveri..."

ssh_cmd "cd ${BACKEND_DIR} && apt-get update && apt-get install -y build-essential python3 && npm install"

#==============================================================================
# KROK 8: REŠTART SLUŽIEB
#==============================================================================

echo -e "${YELLOW}[STEP]${NC} Reštart služieb na serveri..."

# Systemd service pre MarkNET backend
ssh_cmd "cat > /etc/systemd/system/marknet.service << 'EOF'
[Unit]
Description=MarkNET Neural Network Backend
After=network.target

[Service]
Type=simple
User=root
WorkingDirectory=${BACKEND_DIR}
ExecStart=/usr/bin/node app.js
Restart=always
RestartSec=10
Environment=NODE_ENV=production
Environment=PORT=3000

[Install]
WantedBy=multi-user.target
EOF"

# Povolenie a spustenie služieb
ssh_cmd "systemctl daemon-reload && systemctl enable marknet && systemctl restart marknet && systemctl restart nginx"

#==============================================================================
# KROK 9: FINÁLNA KONTROLA
#==============================================================================

echo -e "${YELLOW}[STEP]${NC} Finálna kontrola stavu služieb..."

ssh_cmd "echo '=== Stav služieb ===' && systemctl status nginx marknet --no-pager"

echo -e "${CYAN}═══════════════════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}✅ DEPLOYMENT DOKONČENÝ!${NC}"
echo -e "${CYAN}═══════════════════════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}🌐 Frontend:${NC} https://${DOMAIN}"
echo -e "${GREEN}🔌 Backend API:${NC} https://${DOMAIN}/api/health"
echo -e "${GREEN}🧠 Neural Network:${NC} https://${DOMAIN}/api/predict"
echo -e "${GREEN}📊 Models:${NC} https://${DOMAIN}/api/models"
echo -e "${CYAN}═══════════════════════════════════════════════════════════════════════════════${NC}" 