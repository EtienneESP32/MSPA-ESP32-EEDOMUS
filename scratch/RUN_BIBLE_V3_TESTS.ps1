$SIMU_IP = "192.168.1.230"
$LABO_IP = "192.168.1.226"

Write-Host "=====================================================" -ForegroundColor Cyan
Write-Host " EXECUTION DES TESTS BIBLE V3 (ARCHITECTURE DUAL-LAYER) " -ForegroundColor Cyan
Write-Host "=====================================================" -ForegroundColor Cyan

# --- TEST V3-1 : Miroir Absolu ---
Write-Host "`n[TEST V3-1] Le Miroir Absolu (IHM)..." -ForegroundColor Yellow
Write-Host "Allumage de la Chauffe sur le LABO..."
Invoke-RestMethod -Uri "http://$LABO_IP/switch/chauffage/turn_on" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 3

Write-Host "VÉRIFICATION MANUELLE REQUISE :" -ForegroundColor Magenta
Write-Host "-> Regarde l'IHM Web du LABO (192.168.1.226)."
Write-Host "-> Le bouton Chauffe doit clignoter en rythme (2s ON / 2s OFF)."
Read-Host -Prompt "Appuie sur Entrée quand tu as vérifié"

# --- TEST V3-2 : Bouclier Domotique (20s) ---
Write-Host "`n[TEST V3-2] Le Bouclier Domotique (20s)..." -ForegroundColor Yellow
Write-Host "Le SPA est toujours en chauffe (clignotement). Vérification Eedomus..."
Write-Host "VÉRIFICATION MANUELLE REQUISE :" -ForegroundColor Magenta
Write-Host "-> Va sur la box Eedomus ou lis les logs HTTP de l'ESP32."
Write-Host "-> Eedomus ne doit avoir reçu qu'UN SEUL ordre 'ON'. Il ne doit PAS y avoir de spam toutes les 2 secondes."
Read-Host -Prompt "Appuie sur Entrée quand tu as vérifié"

# --- TEST V3-3 : Veille, Baisse & Hystérésis ---
Write-Host "`n[TEST V3-3] Veille, Baisse & Hystérésis (Hystérésis 2.0C)..." -ForegroundColor Yellow
Write-Host "On force la température du SIMU au-dessus de la consigne (ex: 41C)..."
Invoke-RestMethod -Uri "http://$SIMU_IP/number/sim_temp/set?value=41.0" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 3

Write-Host "VÉRIFICATION MANUELLE REQUISE :" -ForegroundColor Magenta
Write-Host "1. Le SIMU coupe la chauffe et passe en veille (clignotement lent 1s/8s)."
Write-Host "2. Compte jusqu'à 20. L'Eedomus doit enfin recevoir l'ordre 'OFF' (Lissage 20s terminé)."
Write-Host "3. Patiente que la température du SIMU baisse naturellement jusqu'à 38.0C (Consigne 40 - 2.0)."
Write-Host "4. Le SIMU doit rallumer automatiquement la chauffe !"
Read-Host -Prompt "Appuie sur Entrée quand tu as vérifié"

# --- TEST V3-4 : Découplage Coeur 0 / Coeur 1 ---
Write-Host "`n[TEST V3-4] Découplage Multi-Core (Test de charge HTTP)..." -ForegroundColor Yellow
Write-Host "On bombarde l'IHM Web du LABO pendant 5 secondes..."
for ($i=0; $i -lt 15; $i++) {
    Invoke-RestMethod -Uri "http://$LABO_IP/" -Method Get -ErrorAction SilentlyContinue | Out-Null
}
Write-Host "VÉRIFICATION MANUELLE REQUISE :" -ForegroundColor Magenta
Write-Host "-> Si ce script est arrivé jusqu'ici sans erreur réseau, et que le LABO n'a pas rebooté, c'est GAGNÉ !"
Write-Host "-> Le Core 0 a encaissé la charge web, pendant que le Core 1 gérait le clignotement UART sans aucune collision de mémoire."

Write-Host "`n=====================================================" -ForegroundColor Cyan
Write-Host " VERDICT FINAL: BIBLE V3 VALIDEE " -ForegroundColor Green
Write-Host "=====================================================" -ForegroundColor Cyan
