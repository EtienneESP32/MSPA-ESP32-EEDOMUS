$LogFile = "scratch\spa_timing_logs.txt"
Write-Host "Demarrage de l'enregistrement des logs vers $LogFile..."
$process = Start-Process -FilePath "py" -ArgumentList "-m esphome logs esphome/mspa-controller.yaml --device 10.0.6.2" -RedirectStandardOutput $LogFile -PassThru -NoNewWindow
Write-Host "Processus lance (PID: $($process.Id)). Enregistrement pendant 120 secondes..."
Start-Sleep -Seconds 120
Stop-Process -Id $process.Id -Force
Write-Host "Enregistrement termine."
