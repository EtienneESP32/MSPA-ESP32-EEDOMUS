$LABO_IP = "192.168.1.226"
$LogFile = "scratch\labo_deep_hierarchy_test.txt"

Write-Host "--- DEBUT DU TEST DE PROFONDEUR HIERARCHIQUE GOLD-V4 ---" -ForegroundColor Cyan

$process = Start-Process -FilePath "py" -ArgumentList "-m esphome logs esphome/labo/mspa-controller-labo.yaml --device $LABO_IP" -RedirectStandardOutput $LogFile -PassThru -NoNewWindow
Start-Sleep -Seconds 5

# --- TEST 1 : ALLUMAGE CHAUFFAGE ---
Write-Host "[TEST 1] Allumage du Chauffage. La Filtration DOIT s'allumer automatiquement." -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$LABO_IP/switch/chauffage/turn_on" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 3

Write-Host "Vérification des états (Chauffage et Filtration doivent être ON) :"
Invoke-RestMethod -Uri "http://$LABO_IP/switch/chauffage" | Select-Object id, state | Format-Table -HideTableHeaders
Invoke-RestMethod -Uri "http://$LABO_IP/switch/filtration" | Select-Object id, state | Format-Table -HideTableHeaders

Write-Host "Attente de 22s pour prouver que le Watchdog Diamond n'interfère pas..."
Start-Sleep -Seconds 22

# --- TEST 2 : EXTINCTION MAÎTRE PAR LA FILTRATION ---
Write-Host "[TEST 2] Extinction de la Filtration. Le Chauffage DOIT s'éteindre automatiquement." -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$LABO_IP/switch/filtration/turn_off" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 3

Write-Host "Vérification des états (Chauffage et Filtration doivent être OFF instantanément) :"
Invoke-RestMethod -Uri "http://$LABO_IP/switch/chauffage" | Select-Object id, state | Format-Table -HideTableHeaders
Invoke-RestMethod -Uri "http://$LABO_IP/switch/filtration" | Select-Object id, state | Format-Table -HideTableHeaders

Write-Host "Attente de 22s pour prouver que le Sniper ne rallume pas le Chauffage en boucle..."
Start-Sleep -Seconds 22

# --- TEST 3 : ALLUMAGE UVC ---
Write-Host "[TEST 3] Allumage de l'UVC. La Filtration DOIT s'allumer automatiquement." -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$LABO_IP/switch/uvc/turn_on" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 3

Write-Host "Vérification des états (UVC et Filtration doivent être ON) :"
Invoke-RestMethod -Uri "http://$LABO_IP/switch/uvc" | Select-Object id, state | Format-Table -HideTableHeaders
Invoke-RestMethod -Uri "http://$LABO_IP/switch/filtration" | Select-Object id, state | Format-Table -HideTableHeaders

Write-Host "Attente de 22s..."
Start-Sleep -Seconds 22

# --- TEST 4 : EXTINCTION MAÎTRE (FILTRATION) AVEC UVC ---
Write-Host "[TEST 4] Extinction de la Filtration. L'UVC DOIT s'éteindre automatiquement." -ForegroundColor Yellow
Invoke-RestMethod -Uri "http://$LABO_IP/switch/filtration/turn_off" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 3

Write-Host "Vérification des états (UVC et Filtration doivent être OFF) :"
Invoke-RestMethod -Uri "http://$LABO_IP/switch/uvc" | Select-Object id, state | Format-Table -HideTableHeaders
Invoke-RestMethod -Uri "http://$LABO_IP/switch/filtration" | Select-Object id, state | Format-Table -HideTableHeaders

Write-Host "--- FIN DES TESTS ---" -ForegroundColor Cyan
Stop-Process -Id $process.Id -Force
