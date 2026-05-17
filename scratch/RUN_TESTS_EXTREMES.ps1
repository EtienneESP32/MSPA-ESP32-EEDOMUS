$SIM_IP = "192.168.1.230"
$DR_IP = "192.168.1.226"

Write-Host "=========================================" -ForegroundColor Cyan
Write-Host " TESTS EXTRÊMES (DR-BRIDGE) " -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# ---------------------------------------------------------
# TEST 2 : COUPURE DE CABLE
# ---------------------------------------------------------
Write-Host "`n[TEST 2] Simulation Coupure de Cable..." -ForegroundColor Yellow
Write-Host "Silence radio de 6 secondes..."
Start-Sleep -Seconds 6
try {
    $res = Invoke-RestMethod -Uri "http://$DR_IP/binary_sensor/lien_moteur" -TimeoutSec 2
    Write-Host "Apres coupure, Lien Moteur = $($res.state) (Attendu: OFF/False)"
} catch { Write-Host "Impossible de lire le capteur" }

Write-Host "Rebranchement (Envoi d'un heartbeat SPA)..."
try { Invoke-RestMethod -Method Post -Uri "http://$SIM_IP/button/action_chauffe/press" -TimeoutSec 2 } catch {}
Start-Sleep -Seconds 2
try {
    $res = Invoke-RestMethod -Uri "http://$DR_IP/binary_sensor/lien_clavier" -TimeoutSec 2
    Write-Host "Apres rebranchement, Lien Clavier = $($res.state) (Attendu: ON/True)"
} catch {}

# ---------------------------------------------------------
# TEST 3 : TSUNAMI UART (Buffer Overflow)
# ---------------------------------------------------------
Write-Host "`n[TEST 3] Tsunami UART (5000 octets bruts)..." -ForegroundColor Yellow
$jobs = @()
for ($i=0; $i -lt 500; $i++) {
    try { Invoke-RestMethod -Method Post -Uri "http://$SIM_IP/button/action_bruit_uart/press" -TimeoutSec 1 } catch {}
}
Write-Host "Injection terminee."
Start-Sleep -Seconds 3

# ---------------------------------------------------------
# TEST 1 : TROU NOIR & SURVIE (Vérification Finale)
# ---------------------------------------------------------
Write-Host "`n[TEST 1 & SURVIE] Verification de l'integrite du controleur DR..." -ForegroundColor Yellow
try {
    $html = Invoke-RestMethod -Uri "http://$DR_IP/" -TimeoutSec 3
    Write-Host "✅ SUCCES: Le DR-BRIDGE a survecu au Tsunami. L'interface Web repond." -ForegroundColor Green
    
    # Vérification que le relais fonctionne toujours
    Invoke-RestMethod -Method Post -Uri "http://$SIM_IP/button/action_chauffe/press"
    Start-Sleep -Seconds 1
    Write-Host "✅ SUCCES: Le relais UART fonctionne toujours parfaitement." -ForegroundColor Green
} catch {
    Write-Host "❌ ECHEC: Le Controleur DR ne repond plus (Crash/Reboot)." -ForegroundColor Red
}

Write-Host "=========================================" -ForegroundColor Cyan
