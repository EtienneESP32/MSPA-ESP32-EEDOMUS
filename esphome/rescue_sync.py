import os
import subprocess
import time
import platform

IP_TARGET = "192.168.1.226"
YAML_FILE = "esphome/mspa-controller.yaml"

def ping(ip):
    param = "-n" if platform.system().lower() == "windows" else "-c"
    command = ["ping", param, "1", ip]
    return subprocess.run(command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode == 0

def run_upload():
    print(f"\n[!] REVEIL DETECTE ! Lancement de l'upload ESPHome...")
    # On utilise py -m esphome pour etre sur de l'environnement
    cmd = ["py", "-m", "esphome", "upload", YAML_FILE, "--device", IP_TARGET]
    subprocess.run(cmd)

def main():
    print(f"--- SCRIPT DE SAUVETAGE MSPA ---")
    print(f"Cible : {IP_TARGET}")
    
    # PHASE 1 : Attendre que ca coupe
    print("\n[*] PHASE 1 : Attends que tu coupes l'alimentation (OFF)...")
    while ping(IP_TARGET):
        time.sleep(0.5)
    
    print("[V] COUPURE DETECTEE. L'ESP est hors ligne.")
    
    # PHASE 2 : Attendre que ca revienne
    print("\n[*] PHASE 2 : Attends que tu rallumes (ON)... Soyez prêt !")
    while not ping(IP_TARGET):
        time.sleep(0.1) # Check tres rapide
        
    # PHASE 3 : Upload immédiat
    run_upload()

if __name__ == "__main__":
    main()
