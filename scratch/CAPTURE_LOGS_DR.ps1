$LogFile = "scratch\spa_timing_logs_dr.txt"
Write-Host "Demarrage de l'enregistrement des logs DR vers $LogFile..."
$process = Start-Process -FilePath "py" -ArgumentList "-m esphome logs esphome/mspa-controller-DR-PROD.yaml --device 10.0.6.2" -RedirectStandardOutput $LogFile -PassThru -NoNewWindow
Write-Host "Processus lance (PID: $($process.Id)). Enregistrement pendant 300 secondes..."
Start-Sleep -Seconds 300
Stop-Process -Id $process.Id -Force
Write-Host "Enregistrement termine."
