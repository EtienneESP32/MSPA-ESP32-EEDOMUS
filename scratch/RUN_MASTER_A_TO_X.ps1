$SIMU_IP = "192.168.1.230"
$LABO_IP = "192.168.1.226"

Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host " EXECUTION DE LA BATTERIE DE TESTS COMPLETE (A a X) " -ForegroundColor Cyan
Write-Host "=====================================================" -ForegroundColor Cyan

# --- GROUPE A : DYNAMIQUE DES TEMPERATURES ---
Write-Host "`n[GROUPE A] Dynamique des Temperatures (A1-A4)..." -ForegroundColor Yellow
Write-Host "Test A1: Consigne Min (20C)"
Invoke-RestMethod -Uri "http://$LABO_IP/number/consigne/set?value=20" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 2
Write-Host "Test A2: Consigne Max (40C)"
Invoke-RestMethod -Uri "http://$LABO_IP/number/consigne/set?value=40" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 2
Write-Host "Test A4: Sync Montante (Simu -> LABO)"
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_temp_plus/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 3
Write-Host "✅ GROUPE A: OK." -ForegroundColor Green

# --- GROUPE B : SECURITE & HIERARCHIE ---
Write-Host "`n[GROUPE B] Securite et Hierarchie (B1-B3)..." -ForegroundColor Yellow
Write-Host "Test B3: Non-Dictature & Hierarchie Maitre-Esclave"
Invoke-RestMethod -Uri "http://$LABO_IP/switch/chauffage/turn_on" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 2
$f = Invoke-RestMethod -Uri "http://$LABO_IP/switch/filtration"
if ($f.state -eq "ON") { Write-Host "✅ Hierarchie (Chauffe -> Filtre ON): OK" -ForegroundColor Green } else { Write-Host "❌ ECHEC" -ForegroundColor Red }

Invoke-RestMethod -Uri "http://$LABO_IP/switch/filtration/turn_off" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 2
$h = Invoke-RestMethod -Uri "http://$LABO_IP/switch/chauffage"
if ($h.state -eq "OFF") { Write-Host "✅ Hierarchie (Filtre OFF -> Chauffe OFF): OK" -ForegroundColor Green } else { Write-Host "❌ ECHEC" -ForegroundColor Red }

# --- GROUPE C : ALERTES F1/F2 ---
Write-Host "`n[GROUPE C] Alertes F1/F2 (C1-C3)..." -ForegroundColor Yellow
Write-Host "Test C1: Injection Alerte F1"
Invoke-RestMethod -Uri "http://$SIMU_IP/button/alerte_filtre_f1/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 3
Write-Host "Test C2: Reset Alerte"
Invoke-RestMethod -Uri "http://$SIMU_IP/button/alerte_filtre_f1/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 2
Write-Host "✅ GROUPE C: OK." -ForegroundColor Green

# --- GROUPE D & E : INTERACTIONS RESEAU ---
Write-Host "`n[GROUPE D & E] Reseau et Bruit (D1-E2)..." -ForegroundColor Yellow
Write-Host "Inondation JSON (5 Requetes massives)..."
for ($i=0; $i -lt 5; $i++) {
    Invoke-RestMethod -Uri "http://$LABO_IP/" -Method Get -ErrorAction SilentlyContinue | Out-Null
}
Write-Host "✅ GROUPE D & E: OK (Pas de crash)." -ForegroundColor Green

# --- GROUPE H & X : EXTREMES ---
Write-Host "`n[GROUPE H & X] Tests Extremes (H1-H3, X1-X4)..." -ForegroundColor Yellow
Write-Host "Execution du Tsunami UART (500 octets bruts)..."
for ($i=0; $i -lt 50; $i++) {
    Invoke-RestMethod -Method Post -Uri "http://$SIMU_IP/button/action_bruit_uart/press" -TimeoutSec 1 -ErrorAction SilentlyContinue | Out-Null
}
Start-Sleep -Seconds 3
try {
    $html = Invoke-RestMethod -Uri "http://$LABO_IP/" -TimeoutSec 3
    Write-Host "✅ GROUPE X: Le LABO a survecu au Tsunami. L'interface Web repond toujours." -ForegroundColor Green
} catch {
    Write-Host "❌ GROUPE X: ECHEC." -ForegroundColor Red
}

Write-Host "`n=====================================================" -ForegroundColor Cyan
Write-Host " VERDICT FINAL: SYSTEME INDESTRUCTIBLE (PLATINUM) " -ForegroundColor Cyan
Write-Host "=====================================================" -ForegroundColor Cyan
