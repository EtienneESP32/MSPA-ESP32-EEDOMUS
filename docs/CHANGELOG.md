# Historique des changements (MSPA ESP32)

### 2026-03-22 – UI Intelligente (v3.9.3) – REFERENCE STABLE
- **Logique UI Croisée Natif** : Remplacement des hacks C++ `publish_state` qui n'animaient pas visuellement l'interface. Utilisation des macros officielles `switch.turn_on/off` d'ESPHome. L'activation de la Chauffe allume désormais le bouton Filtration de manière visuelle et instantanée sur le Web UI.
- **Sécurités d'Extinction (Interlocks)** : Sécurisation absolue. Si on coupe la Pompe, ça coupe automatiquement l'UVC et le Chauffage en cascade par sécurité. Et inversement, l'extinction du Chauffage ne déclenche la chute de la pompe *que si* l'UVC n'en a pas besoin lui-même. La logique d'arbre entier est gérée par ESPHome !
