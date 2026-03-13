import time
import requests
import threading
import subprocess
import argparse
import sys
from datetime import datetime

# Configuration
target_ip = "192.168.1.226"
log_file = "c:/Users/ebesa/Documents/MSPA/diagnostic_sniff.txt"
entities = ["Filtration", "Chauffage", "UVC"]

class StabilityTester:
    def __init__(self, duration):
        self.duration = duration
        self.start_time = time.time()
        self.running = True
        self.stats = {
            "total_requests": 0,
            "success": 0,
            "failure": 0,
            "errno_23": 0,
            "reboots": 0
        }
        self.lock = threading.Lock()

    def log_capture(self):
        print(f"[*] Starting log capture to {log_file}...")
        try:
            # We use esphome logs to catch safe_mode and reboots
            process = subprocess.Popen(
                ["py", "-m", "esphome", "logs", "esphome/mspa-controller-s3.yaml"],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
                cwd="c:/Users/ebesa/Documents/MSPA"
            )
            
            with open(log_file, "a", encoding="utf-8") as f:
                for line in process.stdout:
                    if not self.running:
                        process.terminate()
                        break
                    
                    timestamp = datetime.now().strftime("%H:%M:%S")
                    f.write(f"[{timestamp}] {line}")
                    f.flush()

                    if "error in accept (23)" in line:
                        with self.lock: self.stats["errno_23"] += 1
                    if "safe_mode" in line or "Booting..." in line:
                        with self.lock: self.stats["reboots"] += 1
                        print(f"[!] REBOOT DETECTED: {line.strip()}")

        except Exception as e:
            print(f"[!] Log capture error: {e}")

    def api_stress(self):
        print("[*] Starting API stress loop...")
        idx = 0
        while time.time() - self.start_time < self.duration and self.running:
            entity = entities[idx % len(entities)]
            action = "on" if (idx // len(entities)) % 2 == 0 else "off"
            url = f"http://{target_ip}/switch/{entity}/turn_{action}"
            
            try:
                with self.lock: self.stats["total_requests"] += 1
                r = requests.post(url, timeout=2)
                if r.status_code == 200:
                    with self.lock: self.stats["success"] += 1
                else:
                    with self.lock: self.stats["failure"] += 1
            except Exception:
                with self.lock: self.stats["failure"] += 1
            
            idx += 1
            time.sleep(1.0) # 1 request per second to maintain pressure without immediate lockout

    def web_hammer(self):
        print("[*] Starting Web hammer loop (simulating multiple UI tabs)...")
        while time.time() - self.start_time < self.duration and self.running:
            try:
                # Hammer the status JSON or root to simulate UI activity
                r = requests.get(f"http://{target_ip}/", timeout=2)
                time.sleep(0.5)
            except:
                pass

    def run(self):
        threads = [
            threading.Thread(target=self.log_capture),
            threading.Thread(target=self.api_stress),
            threading.Thread(target=self.web_hammer)
        ]
        
        for t in threads: t.start()
        
        try:
            while time.time() - self.start_time < self.duration:
                elapsed = int(time.time() - self.start_time)
                with self.lock:
                    print(f"\r[T+{elapsed}s] Req: {self.stats['total_requests']} | OK: {self.stats['success']} | Err: {self.stats['failure']} | Errno23: {self.stats['errno_23']} | Reboots: {self.stats['reboots']}", end="")
                time.sleep(1)
        except KeyboardInterrupt:
            print("\n[!] Aborted by user.")
        
        self.running = False
        print("\n[*] Stopping tests...")
        for t in threads: t.join(timeout=5)
        
        print("\n=== FINAL STABILITY REPORT ===")
        print(f"Duration: {int(time.time() - self.start_time)}s")
        print(f"Total API Requests: {self.stats['total_requests']}")
        print(f"Success Rate: {(self.stats['success']/self.stats['total_requests']*100 if self.stats['total_requests'] > 0 else 0):.1f}%")
        print(f"Socket Errors (Errno 23): {self.stats['errno_23']}")
        print(f"Unexpected Reboots: {self.stats['reboots']}")
        if self.stats['reboots'] == 0:
            print("STATUS: STABLE")
        else:
            print("STATUS: UNSTABLE")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--duration", type=int, default=60, help="Test duration in seconds")
    args = parser.parse_args()
    
    tester = StabilityTester(args.duration)
    tester.run()
