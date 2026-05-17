$ErrorActionPreference = "SilentlyContinue"

Write-Host "=============================================" -ForegroundColor Cyan
Write-Host " EXÉCUTION DE LA SUITE DE TESTS COMPLÈTE DR " -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan

# 1. RUN_DR_TESTS (A, C, H)
Write-Host "`n---> LANCEMENT SUITE DE BASE (A-C-H) <---" -ForegroundColor Magenta
. .\scratch\RUN_DR_TESTS.ps1

# 2. RUN_TESTS_EXTREMES (Tsunami, Coupure)
Write-Host "`n---> LANCEMENT SUITE EXTREME <---" -ForegroundColor Magenta
. .\scratch\RUN_TESTS_EXTREMES.ps1

# 3. STRESS_DESTRUCTION (X-Group)
Write-Host "`n---> LANCEMENT SUITE KAMIKAZE (X) <---" -ForegroundColor Magenta
. .\scratch\STRESS_DESTRUCTION.ps1

Test-X1-HeapAttack
Start-Sleep -Seconds 10

Test-X2-UartOverload
Start-Sleep -Seconds 5

Test-X4-SniperFlood
Start-Sleep -Seconds 5

Write-Host "`n=============================================" -ForegroundColor Green
Write-Host " TOUS LES TESTS SONT TERMINÉS " -ForegroundColor Green
Write-Host "=============================================" -ForegroundColor Green
