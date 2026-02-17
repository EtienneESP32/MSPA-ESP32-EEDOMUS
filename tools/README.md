# Outils – Test protocole MSPA

## `mspa_protocol.py`

Script Python pour construire, vérifier et envoyer/écouter des trames du protocole MSPA (4 octets : `0xA5` | ID | DATA | Checksum). UART 9600 8N1.

**Installation :**
```bash
pip install -r requirements.txt
```

**Exemples :**
- Vérifier une trame : `python mspa_protocol.py check A5 01 01 A7`
- Générer une trame : `python mspa_protocol.py build 01 01`
- Envoyer sur le port série : `python mspa_protocol.py send COM3 01 01`
- Écouter le bus (30 s par défaut) : `python mspa_protocol.py listen COM3 --duration 60`

Adapte le port (`COM3`, `COM4`, `/dev/ttyUSB0`, etc.) selon ton adaptateur UART-USB (sniffer ou bus en lecture seule).
