#!/bin/bash

echo "MarkNET Server Setup - Začínam inštaláciu..."

# 1. Aktualizácia systému
echo ""
echo "1. Aktualizujem systém..."
apt update && apt upgrade -y
if [ $? -ne 0 ]; then
    echo "CHYBA: Aktualizácia zlyhala"
    exit 1
fi
echo "OK - Systém aktualizovaný"

# 2. Inštalácia závislostí
echo ""
echo "2. Inštalujem závislosti..."
apt install -y nodejs npm python3 gcc make curl htop
if [ $? -ne 0 ]; then
    echo "CHYBA: Inštalácia závislostí zlyhala"
    exit 1
fi
echo "OK - Závislosti nainštalované"
echo "   Node.js: $(node --version)"
echo "   Python3: $(python3 --version)"

# 3. Rozbalenie projektu
echo ""
echo "3. Rozbaľujem projekt..."
cd /root
if [ ! -f "marknet-deploy.tar.gz" ]; then
    echo "CHYBA: Súbor marknet-deploy.tar.gz nebol nájdený"
    exit 1
fi
tar -xzf marknet-deploy.tar.gz
echo "OK - Projekt rozbalený"

# 4. Kompilácia C kódu
echo ""
echo "4. Kompilu­jem neurónovú sieť..."
make clean && make
if [ $? -ne 0 ]; then
    echo "CHYBA: Kompilácia zlyhala"
    exit 1
fi
echo "OK - Neurónová sieť zkompilovaná"

# 5. NPM balíčky
echo ""
echo "5. Inštalujem Node.js balíčky..."
cd backend
npm install --production --no-optional
if [ $? -ne 0 ]; then
    echo "CHYBA: NPM inštalácia zlyhala"
    exit 1
fi
mkdir -p logs
echo "OK - Backend pripravený"

# 6. Konfigurácia frontend
echo ""
echo "6. Nastavujem frontend pre cloud..."
cd ../frontend/js
if [ -f "main.js" ]; then
    cp main.js main.js.backup
    sed -i 's/10\.0\.2\.15/178.128.245.99/g' main.js
    sed -i 's/localhost/178.128.245.99/g' main.js
    echo "OK - Frontend nakonfigurovaný"
else
    echo "CHYBA: main.js nebol nájdený"
    exit 1
fi

# 7. Vytvorenie skriptov
echo ""
echo "7. Vytváram správcovské skripty..."
cd /root

# Spúšťací skript
cat > spustit-marknet.sh << 'EOF'
#!/bin/bash
echo "Spúšťam MarkNET..."

# Backend
cd /root/backend
NODE_ENV=production nohup node app.js > logs/production.log 2>&1 &
BACKEND_PID=$!
echo "Backend: PID $BACKEND_PID"

# Čakanie
sleep 5

# Frontend  
cd /root/frontend
nohup python3 -m http.server 8080 --bind 0.0.0.0 > frontend.log 2>&1 &
FRONTEND_PID=$!
echo "Frontend: PID $FRONTEND_PID"

# Uloženie PID
echo $BACKEND_PID > /tmp/marknet_backend.pid
echo $FRONTEND_PID > /tmp/marknet_frontend.pid

echo ""
echo "MarkNET spustený!"
echo "Frontend: http://178.128.245.99:8080"
echo "Backend: http://178.128.245.99:3000"
echo ""
echo "Zastavenie: ./zastavit-marknet.sh"
EOF

# Zastavovací skript
cat > zastavit-marknet.sh << 'EOF'
#!/bin/bash
echo "Zastavujem MarkNET..."

# Zastavenie backend
if [ -f "/tmp/marknet_backend.pid" ]; then
    kill $(cat /tmp/marknet_backend.pid) 2>/dev/null
else
    pkill -f "node app.js"
fi

# Zastavenie frontend
if [ -f "/tmp/marknet_frontend.pid" ]; then
    kill $(cat /tmp/marknet_frontend.pid) 2>/dev/null
else
    pkill -f "python3.*8080"
fi

# Čistenie
rm -f /tmp/marknet_*.pid

echo "MarkNET zastavený"
EOF

# Status skript
cat > status-marknet.sh << 'EOF'
#!/bin/bash
echo "MarkNET Status:"
echo "==============="

# Kontrola procesov
if pgrep -f "node app.js" > /dev/null; then
    echo "Backend: BEŽÍ (PID: $(pgrep -f 'node app.js'))"
else
    echo "Backend: ZASTAVENÝ"
fi

if pgrep -f "python3.*8080" > /dev/null; then
    echo "Frontend: BEŽÍ (PID: $(pgrep -f 'python3.*8080'))"
else
    echo "Frontend: ZASTAVENÝ"
fi

# Kontrola API
echo ""
curl -s --max-time 3 http://localhost:3000/api/health > /dev/null
if [ $? -eq 0 ]; then
    echo "Backend API: DOSTUPNÉ"
else
    echo "Backend API: NEDOSTUPNÉ"
fi

curl -s --max-time 3 http://localhost:8080 > /dev/null
if [ $? -eq 0 ]; then
    echo "Frontend: DOSTUPNÉ"
else
    echo "Frontend: NEDOSTUPNÉ"
fi

echo ""
echo "Systémové zdroje:"
echo "Pamäť: $(free -h | grep Mem: | awk '{print $3 "/" $2}')"
echo "Disk: $(df -h / | tail -1 | awk '{print $3 "/" $2}')"
EOF

chmod +x *.sh
echo "OK - Skripty vytvorené"

# 8. Finálne info
echo ""
echo "Inštalácia dokončená!"
echo "====================="
echo ""
echo "Spustenie: ./spustit-marknet.sh"
echo "Zastavenie: ./zastavit-marknet.sh" 
echo "Status: ./status-marknet.sh"
echo ""
echo "Po spustení bude dostupné na:"
echo "http://178.128.245.99:8080"
echo ""

# Systémové info
echo "Server info:"
echo "Pamäť: $(free -h | grep Mem: | awk '{print $2}')"
echo "CPU: $(nproc) jadier"
echo "IP: $(curl -s ifconfig.me)"

echo ""
echo "Setup dokončený. Použite ./spustit-marknet.sh na spustenie." 