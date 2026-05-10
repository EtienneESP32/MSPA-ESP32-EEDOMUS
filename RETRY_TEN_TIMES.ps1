$max_retries = 10
$device = "192.168.1.226"

for ($i = 1; $i -le $max_retries; $i++) {
    Write-Host "--- Tentative $i sur $max_retries ---" -ForegroundColor Cyan
    py -m esphome upload esphome/mspa-controller.yaml --device $device
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Félicitations ! Envoi réussi à la tentative $i." -ForegroundColor Green
        exit 0
    }
    
    if ($i -lt $max_retries) {
        Write-Host "Échec de la tentative $i. Attente de 5 secondes avant la suivante..." -ForegroundColor Yellow
        Start-Sleep -Seconds 5
    }
}

Write-Host "Les 10 tentatives ont échoué." -ForegroundColor Red
exit 1
