$SIMU_IP = "192.168.1.230"
$LABO_IP = "192.168.1.226"
$LogFile = "scratch\labo_gold_full_test.txt"

Write-Host "--- DEMARRAGE DU TEST COMPLET GOLD-V4 ---" -ForegroundColor Cyan

# Démarrage de la capture des logs du LABO
$process = Start-Process -FilePath "py" -ArgumentList "-m esphome logs esphome/labo/mspa-controller-labo.yaml --device $LABO_IP" -RedirectStandardOutput $LogFile -PassThru -NoNewWindow
Start-Sleep -Seconds 5

# --- 1. TEST CHAUFFE (Interaction Active UI -> SPA) ---
Write-Host "[ETAPE 1] Action LABO: CHAUFFE ON (Déclenchement Sniper)" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$LABO_IP/switch/chauffage/turn_on" -Method Post -ErrorAction SilentlyContinue | Out-Null
Write-Host "Attente de 12s (Verification que le Watchdog Diamond 10s n'interfère plus de façon erronée)..."
Start-Sleep -Seconds 12

Write-Host "[ETAPE 2] Action LABO: CHAUFFE OFF (Test de l'Instant-Drop UI)" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$LABO_IP/switch/chauffage/turn_off" -Method Post -ErrorAction SilentlyContinue | Out-Null
Write-Host "Le bouton doit tomber a OFF instantanement (Destruction de l'enveloppe 15s)."
Write-Host "Attente 22s pour verifier la stabilite totale (Watchdog Diamond a 20s)..."
Start-Sleep -Seconds 22

# --- 2. TEST FILTRE ---
Write-Host "[ETAPE 3] Action LABO: FILTRE ON" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$LABO_IP/switch/filtration/turn_on" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 5

Write-Host "[ETAPE 4] Action LABO: FILTRE OFF" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$LABO_IP/switch/filtration/turn_off" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 22

# --- 3. TEST DE NON-DICTATURE ---
Write-Host "[ETAPE 5] Action LABO: CHAUFFE ON (Armement Sniper)" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$LABO_IP/switch/chauffage/turn_on" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 3

Write-Host "[ETAPE 6] Action SIMULATEUR: CHAUFFE OFF (Trahison Physique)" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_chauffe/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Write-Host "Verification que l'ESP s'incline devant le Maitre Physique et annule son Sniper..."
Start-Sleep -Seconds 15

Write-Host "--- FIN DES TESTS GOLD-V4 ---" -ForegroundColor Cyan
Stop-Process -Id $process.Id -Force
Write-Host "Logs ecrits dans $LogFile"
