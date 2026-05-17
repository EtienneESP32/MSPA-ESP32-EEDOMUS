$LogFile = "scratch\spa_timing_logs_v3.txt"
Write-Host "Demarrage de l'enregistrement d'observation (5 minutes) vers $LogFile..."
$process = Start-Process -FilePath "py" -ArgumentList "-m esphome logs esphome/mspa-controller.yaml --device 10.0.6.2" -RedirectStandardOutput $LogFile -PassThru -NoNewWindow
Write-Host "Processus lance (PID: $($process.Id)). Enregistrement silencieux en cours..."
Start-Sleep -Seconds 300
Stop-Process -Id $process.Id -Force
Write-Host "Enregistrement termine."
