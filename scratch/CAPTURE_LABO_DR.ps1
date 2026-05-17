$LogFile = "scratch\labo_dr_logs.txt"
Write-Host "Demarrage de l'enregistrement LABO vers $LogFile..."
$process = Start-Process -FilePath "py" -ArgumentList "-m esphome logs esphome/labo/mspa-controller-DR-LABO.yaml --device 192.168.1.226" -RedirectStandardOutput $LogFile -PassThru -NoNewWindow
Write-Host "Enregistrement en cours..."
Start-Sleep -Seconds 120
Stop-Process -Id $process.Id -Force
Write-Host "Enregistrement termine."
