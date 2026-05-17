$SIM_IP = "192.168.1.230"
$DR_IP = "192.168.1.226"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host " Lancement Tests CHAOS (Bruit UART) " -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
Start-Sleep -Seconds 1

# 1. Activation du Chaos
Write-Host "[CHAOS] Activation du Bruit Continu (100ms)..." -ForegroundColor Yellow
Invoke-RestMethod -Method Post -Uri "http://$SIM_IP/switch/sw_chaos/turn_on"
Start-Sleep -Seconds 5

# 2. Test de survie et de parsing pendant le Chaos
Write-Host "[SURVIE] Modification de la consigne (33°C) pendant l'attaque..."
Invoke-RestMethod -Method Post -Uri "http://$SIM_IP/number/sim_setpoint_num/set?value=33"
Start-Sleep -Seconds 5

# 3. Rafale massive
Write-Host "[RAFALE] Injection de rafales d'octets invalides..." -ForegroundColor Red
for ($i=0; $i -lt 5; $i++) {
    Invoke-RestMethod -Method Post -Uri "http://$SIM_IP/button/action_bruit_uart/press"
    Start-Sleep -Milliseconds 500
}
Start-Sleep -Seconds 5

# 4. Arrêt du Chaos
Write-Host "[ARRET] Fin de l'attaque. Désactivation du Bruit." -ForegroundColor Green
Invoke-RestMethod -Method Post -Uri "http://$SIM_IP/switch/sw_chaos/turn_off"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host " TESTS CHAOS TERMINÉS. " -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan
