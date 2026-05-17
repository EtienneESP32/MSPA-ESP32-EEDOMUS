$SIMU_IP = "192.168.1.230"
$LABO_IP = "192.168.1.226"

Write-Host "====================================================="
Write-Host " TEST AUTOMATISE BIBLE V3 - ANTIGRAVITY AI"
Write-Host "====================================================="

# V3-1: Test du Miroir
Write-Host "TEST V3-1: Activation de la chauffe et observation du clignotement de l'IHM..."
Invoke-RestMethod -Uri "http://$LABO_IP/switch/chauffage/turn_on" -Method Post | Out-Null
Start-Sleep -Seconds 1

$states = @()
for ($i=0; $i -lt 15; $i++) {
    $res = Invoke-RestMethod -Uri "http://$LABO_IP/switch/chauffage"
    $states += $res.state
    Start-Sleep -Milliseconds 500
}
Write-Host "Etats lus sur l'API LABO en 7.5 secondes :"
Write-Host ($states -join " -> ")
if (($states -contains "ON") -and ($states -contains "OFF")) {
    Write-Host "✅ V3-1 REUSSI : L'IHM Web clignote en direct ! Le filtre 15s est bien detruit." -ForegroundColor Green
} else {
    Write-Host "❌ V3-1 ECHEC : L'IHM est figee sur un seul etat." -ForegroundColor Red
}

# V3-3: Test de la baisse et de la veille
Write-Host "`nTEST V3-3: Forcer une temperature > Consigne pour declencher la veille (1s ON / 8s OFF)..."
Invoke-RestMethod -Uri "http://$SIMU_IP/number/sim_temp/set?value=41.0" -Method Post | Out-Null
Start-Sleep -Seconds 2

$states_veille = @()
for ($i=0; $i -lt 25; $i++) {
    $res = Invoke-RestMethod -Uri "http://$LABO_IP/switch/chauffage"
    $states_veille += $res.state
    Start-Sleep -Milliseconds 500
}
Write-Host "Etats lus en Veille (devrait etre majoritairement OFF avec de rares pulses ON) :"
Write-Host ($states_veille -join " -> ")

# V3-4: Collision / Decouplage Core 0
Write-Host "`nTEST V3-4: Decouplage Multi-Core (Bombardement reseau)..."
$success = 0
for ($i=0; $i -lt 20; $i++) {
    try {
        Invoke-RestMethod -Uri "http://$LABO_IP/" -Method Get -TimeoutSec 1 | Out-Null
        $success++
    } catch {}
}
if ($success -ge 15) {
    Write-Host "✅ V3-4 REUSSI : Le serveur HTTP a survecu au bombardement sans Kernel Panic." -ForegroundColor Green
} else {
    Write-Host "❌ V3-4 ECHEC : Le serveur HTTP a freeze ou crashe." -ForegroundColor Red
}

Write-Host "`n=== TESTS TERMINEES ==="
