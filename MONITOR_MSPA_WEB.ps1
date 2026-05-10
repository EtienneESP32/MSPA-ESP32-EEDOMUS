$ip = "10.0.6.2"
$ports = @(81)
$interval = 2 # secondes

Write-Host "Surveillance de l'interface Web MSPA sur $ip..." -ForegroundColor Cyan
Write-Host "Appuyez sur Ctrl+C pour arrêter." -ForegroundColor Gray

while ($true) {
    foreach ($port in $ports) {
        try {
            $url = "http://${ip}:${port}"
            $response = Invoke-WebRequest -Uri $url -Method Head -TimeoutSec 1 -ErrorAction Stop
            if ($response.StatusCode -eq 200) {
                Write-Host "[$(Get-Date -Format 'HH:mm:ss')] SUCCÈS : L'interface répond sur le port $port ! ($url)" -ForegroundColor Green
                # Optionnel : Faire un bip sonore
                [System.Console]::Beep(440, 500)
                exit 0
            }
        }
        catch {
            # On ignore les erreurs de connexion
        }
    }
    
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] En attente d'une réponse de $ip..." -ForegroundColor Yellow
    Start-Sleep -Seconds $interval
}
