$SIMU_IP = "192.168.1.230"
$LABO_IP = "192.168.1.226"

Write-Host "--- TEST DU FILTRE MACRO (15s) ---" -ForegroundColor Cyan

Write-Host "1. Envoi Ordre: CHAUFFAGE ON (Simule la chauffe 2s/2s)"
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_chauffe/press" -Method Post -ErrorAction SilentlyContinue | Out-Null
Start-Sleep -Seconds 10

Write-Host "2. Envoi Ordre: CHAUFFAGE OFF (Simule le Polling 1s/8s)"
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_chauffe/press" -Method Post -ErrorAction SilentlyContinue | Out-Null # C'est un toggle dans le simulateur? Wait, le bouton est "ACTION: CHAUFFE" qui envoie A5 01 01 A7?

# Lancement de l'enregistrement LABO
$LogFile = "scratch\labo_macro_test.txt"
$process = Start-Process -FilePath "py" -ArgumentList "-m esphome logs esphome/labo/mspa-controller-DR-LABO.yaml --device $LABO_IP" -RedirectStandardOutput $LogFile -PassThru -NoNewWindow

Start-Sleep -Seconds 3

Write-Host "--- INJECTION CHAUFFAGE ON ---"
Invoke-RestMethod -Uri "http://$SIMU_IP/button/action_chauffe/press" -Method Post -ErrorAction SilentlyContinue | Out-Null

Write-Host "Attente 20 secondes (La chauffe clignote 2s/2s sur le simu, l'UI du LABO doit rester 100% stable)"
Start-Sleep -Seconds 20

# Pour éteindre le chauffage, il faut envoyer le bouton "FILTRE" ou un custom button. 
# Ou le script de chauffe du simulateur va le faire lui-même (si on lance script_heat).

Write-Host "--- FIN DU TEST, fermeture des logs ---"
Stop-Process -Id $process.Id -Force
Write-Host "Logs ecrits dans $LogFile"
