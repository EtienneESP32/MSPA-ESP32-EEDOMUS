import socket
import struct
import time

ip = "10.0.6.2"
port = 6053

print(f"--- Diagnostic API MSPA ({ip}:{port}) ---")
for i in range(1, 4):
    print(f"Tentative {i}/3...")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(5)
        start = time.time()
        s.connect((ip, port))
        end = time.time()
        print(f"TCP Connected in {end-start:.3f}s. Envoi du handshake...")
        
        # Envoi d'un message Hello minimaliste (Protocol message 1)
        # Structure ESPHome API : [0x00] [length] [type]
        s.sendall(b"\x00\x02\x01\x00") 
        
        data = s.recv(1024)
        if data:
            print(f"Réponse reçue du Spa ! (Hex: {data.hex()})")
        else:
            print("Connexion fermée par le Spa immédiatement (EOF).")
        s.close()
    except Exception as e:
        print(f"Erreur de connexion : {e}")
    time.sleep(1)

print("--- Fin du diagnostic ---")
