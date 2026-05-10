@echo off
echo Tentative de poussée OTA vers MSPA (10.0.6.2) via VPN...
py -m esphome upload esphome/mspa-controller.yaml --device 10.0.6.2
pause
