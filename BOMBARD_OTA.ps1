while ($true) {
    Write-Host "--- Tentative de Flash OTA ---" -ForegroundColor Cyan
    py -m esphome upload esphome/mspa-controller.yaml --device 192.168.1.226
    if ($LASTEXITCODE -eq 0) {
        Write-Host "FLASH RÉUSSI !" -ForegroundColor Green
        exit 0
    }
    Write-Host "Echec. Attente d'une seconde..." -ForegroundColor Gray
    Start-Sleep -Seconds 1
}
