# SCRIPT DE TEST "KAMIKAZE" - RECHERCHE DU POINT DE RUPTURE (X-GROUP)
# ATTENTION : Ce script est conçu pour pousser l'ESP32 dans ses retranchements. 
# Risque de déconnexion, crash et reboot.

$CONTROLLER_IP = "192.168.1.226"
$SIMULATOR_IP = "192.168.1.230"

Write-Host "--- PRÉPARATION DES TESTS DE RUPTURE ---" -ForegroundColor Red

# --- TEST X1 : SATURATION RAM (HEAP ATTACK) ---
# Envoie 50 requêtes simultanées sans attendre de réponse.
# Note : Nécessite que le Socket Guard soit désactivé pour être efficace.
function Test-X1-HeapAttack {
    Write-Host "[X1] Lancement de l'attaque Heap..." -ForegroundColor Yellow
    for ($i=0; $i -lt 50; $i++) {
        Start-Job -ScriptBlock { 
            param($ip) 
            try { Invoke-WebRequest -Uri "http://$ip/binary_sensor/motor_link" -TimeoutSec 1 } catch {}
        } -ArgumentList $CONTROLLER_IP
    }
    Write-Host "50 Sockets ouverts en parallèle. Surveillez le Heap !" -ForegroundColor Cyan
}

# --- TEST X2 : SURCHARGE UART (FREQUENCY ATTACK) ---
# Demande au simulateur d'accélérer sa boucle UART (si supporté par le firmware simu).
function Test-X2-UartOverload {
    Write-Host "[X2] Demande d'overclocking UART au simulateur..." -ForegroundColor Yellow
    # Simule un comportement de bug simu (bombardement)
    for ($i=0; $i -lt 100; $i++) {
        Invoke-RestMethod -Method Post -Uri "http://$SIMULATOR_IP/button/force_ghost_heat/press"
        Start-Sleep -Milliseconds 10
    }
}

# --- TEST X4 : COLLISION SNIPER (INJECTION FLOOD) ---
# Force le Sniper à injecter 4 types de commandes en boucle rapide.
function Test-X4-SniperFlood {
    Write-Host "[X4] Lancement du Sniper Flood..." -ForegroundColor Yellow
    $session = New-Object Microsoft.PowerShell.Commands.WebRequestSession
    for ($i=0; $i -lt 20; $i++) {
        Invoke-RestMethod -Method Post -Uri "http://$CONTROLLER_IP/switch/filtration/toggle"
        Invoke-RestMethod -Method Post -Uri "http://$CONTROLLER_IP/switch/chauffage/toggle"
        Invoke-RestMethod -Method Post -Uri "http://$CONTROLLER_IP/switch/uvc/toggle"
        Start-Sleep -Milliseconds 50
    }
}

Write-Host "Scripts chargés. Utilisez 'Test-X1-HeapAttack', etc. pour tester." -ForegroundColor Green
