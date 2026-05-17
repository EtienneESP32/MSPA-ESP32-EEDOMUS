$SIM_IP = "192.168.1.230"
$DR_IP = "192.168.1.226"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host " Lancement Tests DR-BRIDGE (A1 -> H3) " -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Start-Sleep -Seconds 1

# --- GROUPE A : DYNAMIQUE ---
Write-Host "[A1/A2] Test de Non-Dictature absolue (Tentative de Setpoint via DR)..."
try { Invoke-RestMethod -Method Post -Uri "http://$DR_IP/number/consigne_temperature/set?value=40" -TimeoutSec 2 } catch {}
Start-Sleep -Seconds 1

Write-Host "[A3] Injection Température Négative sur SIMU (-2.0°C)..."
Invoke-RestMethod -Method Post -Uri "http://$SIM_IP/number/sim_temp_num/set?value=-2.0"
Start-Sleep -Seconds 2

Write-Host "[A4] Modification Consigne sur SIMU (35°C)..."
Invoke-RestMethod -Method Post -Uri "http://$SIM_IP/number/sim_setpoint_num/set?value=35"
Start-Sleep -Seconds 2

# --- GROUPE C : ALERTES ---
Write-Host "[C1] Déclenchement Alerte Filtre (F1) sur SIMU..."
Invoke-RestMethod -Method Post -Uri "http://$SIM_IP/button/alerte_filtre_f1/press"
Start-Sleep -Seconds 3

Write-Host "[C2] Reset Alerte sur SIMU (Retour à la normale)..."
Invoke-RestMethod -Method Post -Uri "http://$SIM_IP/button/alerte_filtre_f1/press"
Start-Sleep -Seconds 2

# --- GROUPE H : TORTURE ---
Write-Host "[H1] Torture : Oscillation Infernale (Clavier frappé à 10Hz)..." -ForegroundColor Yellow
for ($i=0; $i -lt 20; $i++) {
    Invoke-RestMethod -Method Post -Uri "http://$SIM_IP/button/action_chauffe/press"
    Invoke-RestMethod -Method Post -Uri "http://$SIM_IP/button/action_filtre/press"
    Start-Sleep -Milliseconds 100
}

Write-Host "=========================================" -ForegroundColor Green
Write-Host " TESTS TERMINÉS. " -ForegroundColor Green
Write-Host "=========================================" -ForegroundColor Green
