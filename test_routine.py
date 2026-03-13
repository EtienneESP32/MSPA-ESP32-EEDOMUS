import sys
import time
import requests
import subprocess
import threading

# We will start esphome logs in a subprocess to capture full UART output
def run_logger():
    # Use unbuffered output and flush
    with open("C:/Users/ebesa/Documents/MSPA/diagnostic_sniff.txt", "w", encoding="utf-8") as f:
        process = subprocess.Popen(
            ["py", "-m", "esphome", "logs", "esphome/mspa-controller.yaml"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1
        )
        for line in process.stdout:
            f.write(line)
            f.flush()

log_thread = threading.Thread(target=run_logger)
log_thread.daemon = True
log_thread.start()

print("Waiting 15 seconds for ESPHome logs to connect and stabilize...")
time.sleep(15)

def hit_api(entity, action):
    try:
        url = f"http://192.168.1.226/switch/{entity}/turn_{action}"
        print(f"Sending {action.upper()} to {entity}...")
        requests.post(url, timeout=5)
    except Exception as e:
        print(f"HTTP Error: {e}")

print("--- COMMENCING 50 SEC AUTOMATED TEST SEQUENCE ---")

for cycle in range(1, 4):
    print(f"\n[Cycle {cycle}/3]")
    
    # Toggle Filtration
    print("Toggle Filtration...")
    hit_api("filtration", "on") 
    time.sleep(5)
    
    print("Toggle Filtration...")
    hit_api("filtration", "off") # Note: firmwmare 3.5.9 maps off->cmd_f=1
    time.sleep(5)
    
print("\n--- TEST ROUTINE COMPLETE. WAITING 5S FOR FINAL LOGS ---")
time.sleep(5)
print("DONE.")
