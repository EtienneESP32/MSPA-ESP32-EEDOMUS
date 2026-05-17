$SIMU_IP = "192.168.1.230"
$LABO_IP = "192.168.1.226"
$LogFile = "scratch\labo_macro_full_test.txt"

Write-Host "--- DEMARRAGE DU TEST COMPLET (FILTRE MACRO 15s) ---" -ForegroundColor Cyan
Write-Host "Ce script va actionner toutes les fonctions du Simulateur et vérifier que le LABO lisse correctement les affichages."

# Démarrage de la capture des logs du LABO
$process = Start-Process -FilePath "py" -ArgumentList "-m esphome logs esphome/labo/mspa-controller-DR-LABO.yaml --device $LABO_IP" -RedirectStandardOutput $LogFile -PassThru -NoNewWindow
Start-Sleep -Seconds 5

# --- 1. TEST VEILLE (POLLING) ---
Write-Host "[ETAPE 1] Observation Veille/Polling (1s ON / 8s OFF sur Simu)" -ForegroundColor Yellow
Start-Sleep -Seconds 15

# --- 2. TEST CHAUFFE ---
Write-Host "[ETAPE 2] Action: CHAUFFE ON (Clignotement 2s/2s sur Simu)" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_chauffe/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 20

Write-Host "[ETAPE 2] Action: CHAUFFE OFF (Attente du lissage de 15s)" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_chauffe/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 20

# --- 3. TEST FILTRE ---
Write-Host "[ETAPE 3] Action: FILTRE ON (Stable sur Simu)" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_filtre/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 10

Write-Host "[ETAPE 3] Action: FILTRE OFF (Attente du lissage de 15s)" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_filtre/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 20

# --- 4. TEST UVC ---
Write-Host "[ETAPE 4] Action: UVC ON (Stable sur Simu)" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_uvc/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 10

Write-Host "[ETAPE 4] Action: UVC OFF (Attente du lissage de 15s)" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_uvc/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 20

# --- 5. TEST BULLES ---
Write-Host "[ETAPE 5] Action: BULLES (Niveaux 1, 2, 3 puis OFF)" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_bullles/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 5
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_bullles/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 5
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_bullles/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 5
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_bullles/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 10

# --- 6. TEST ALERTE F1 ---
Write-Host "[ETAPE 6] Action: ALERTE F1 (Clignotement rapide 0.5s sur Simu)" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$SIMU_IP/button/alerte_filtre_f1/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 15

Write-Host "[ETAPE 6] Action: ALERTE F1 OFF" -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$SIMU_IP/button/alerte_filtre_f1/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 20

Write-Host "--- FIN DES TESTS ---" -ForegroundColor Cyan
Stop-Process -Id $process.Id -Force
Write-Host "Logs ecrits dans $LogFile"
