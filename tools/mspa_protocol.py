#!/usr/bin/env python3
"""
Outil de test du protocole MSPA (Série D).
UART 9600 8N1, trames 4 octets : 0xA5 | ID | DATA | Checksum.
Checksum = (0xA5 + ID + DATA) & 0xFF.

Usage:
  python mspa_protocol.py check A5 01 01 A7
  python mspa_protocol.py build 01 01
  python mspa_protocol.py send COM3 01 01
  python mspa_protocol.py listen COM3
"""

import argparse
import sys

HEADER = 0xA5


def checksum(id_byte: int, data_byte: int) -> int:
    return (HEADER + id_byte + data_byte) & 0xFF


def build_frame(id_byte: int, data_byte: int) -> bytes:
    cs = checksum(id_byte, data_byte)
    return bytes([HEADER, id_byte, data_byte, cs])


def parse_frame(raw: bytes) -> tuple[int, int, int] | None:
    """Retourne (id, data, checksum) si trame valide, sinon None."""
    if len(raw) != 4 or raw[0] != HEADER:
        return None
    expected = checksum(raw[1], raw[2])
    if raw[3] != expected:
        return None
    return (raw[1], raw[2], raw[3])


def hex_to_int(s: str) -> int:
    s = s.strip().upper()
    if s.startswith("0X"):
        s = s[2:]
    return int(s, 16)


# Dictionnaire connu (docs/protocol_mspa.md)
IDS_CLAVIER_SPA = {
    0x01: "Chauffage",
    0x02: "Filtration",
    0x03: "Bulles",
    0x04: "Consigne (temp×2)",
    0x15: "Sanitize",
    0x19: "UVC",
}
IDS_SPA_CLAVIER = {
    0x06: "Température (data/2 = °C)",
    0x1A: "Flags (01=Pompe, 02=Chauffage, 08=Repos)",
}


def cmd_check(hex_bytes: list[str]) -> None:
    if len(hex_bytes) != 4:
        print("Il faut 4 octets (ex: A5 01 01 A7)")
        sys.exit(1)
    raw = bytes(hex_to_int(b) for b in hex_bytes)
    parsed = parse_frame(raw)
    if parsed is None:
        print("Trame invalide (header ou checksum)")
        sys.exit(1)
    id_b, data_b, cs = parsed
    print(f"ID=0x{id_b:02X} DATA=0x{data_b:02X} Checksum=0x{cs:02X} OK")
    name = IDS_CLAVIER_SPA.get(id_b) or IDS_SPA_CLAVIER.get(id_b) or "?"
    print(f"  → {name}")
    if id_b == 0x06:
        print(f"  → Température = {data_b / 2:.1f} °C")
    elif id_b == 0x1A:
        flags = []
        if data_b & 0x01:
            flags.append("Pompe")
        if data_b & 0x02:
            flags.append("Chauffage")
        if data_b & 0x08:
            flags.append("Repos")
        print(f"  → Flags: {flags or ['aucun']}")


def cmd_build(hex_bytes: list[str]) -> None:
    if len(hex_bytes) != 2:
        print("Il faut ID et DATA (ex: 01 01)")
        sys.exit(1)
    id_b = hex_to_int(hex_bytes[0])
    data_b = hex_to_int(hex_bytes[1])
    frame = build_frame(id_b, data_b)
    print(" ".join(f"{b:02X}" for b in frame))


def cmd_send(port: str, hex_bytes: list[str]) -> None:
    if len(hex_bytes) != 2:
        print("Il faut ID et DATA (ex: 01 01)")
        sys.exit(1)
    try:
        import serial
    except ImportError:
        print("Installer pyserial: pip install pyserial")
        sys.exit(1)
    id_b = hex_to_int(hex_bytes[0])
    data_b = hex_to_int(hex_bytes[1])
    frame = build_frame(id_b, data_b)
    ser = serial.Serial(port, 9600, timeout=0.5)
    ser.write(frame)
    ser.close()
    print(f"Envoyé: {' '.join(f'{b:02X}' for b in frame)} sur {port}")


def cmd_listen(port: str, duration_sec: float = 30.0) -> None:
    try:
        import serial
    except ImportError:
        print("Installer pyserial: pip install pyserial")
        sys.exit(1)
    import time
    ser = serial.Serial(port, 9600, timeout=0.1)
    buf = bytearray()
    end = time.monotonic() + duration_sec
    print(f"Écoute sur {port} ({duration_sec}s). Trames 4 octets (0xA5...)…")
    while time.monotonic() < end:
        b = ser.read(1)
        if not b:
            continue
        buf.append(b[0])
        if len(buf) > 4:
            buf.pop(0)
        if len(buf) == 4 and buf[0] == HEADER:
            parsed = parse_frame(bytes(buf))
            if parsed:
                id_b, data_b, _ = parsed
                name = IDS_CLAVIER_SPA.get(id_b) or IDS_SPA_CLAVIER.get(id_b) or "?"
                extra = ""
                if id_b == 0x06:
                    extra = f" → {data_b/2:.1f}°C"
                elif id_b == 0x1A:
                    extra = f" → {data_b:02X}"
                print(f"  {' '.join(f'{x:02X}' for x in buf)}  ID=0x{id_b:02X} {name}{extra}")
            buf.clear()
    ser.close()
    print("Fin écoute.")


def main() -> None:
    ap = argparse.ArgumentParser(description="Test protocole MSPA (trames 0xA5)")
    ap.add_argument("cmd", choices=["check", "build", "send", "listen"], help="Commande")
    ap.add_argument("args", nargs="*", help="Octets hex (ex: A5 01 01 A7) ou port COM pour send/listen")
    ap.add_argument("--duration", type=float, default=30.0, help="Durée écoute en secondes (listen)")
    a = ap.parse_args()

    if a.cmd == "check":
        cmd_check(a.args)
    elif a.cmd == "build":
        cmd_build(a.args)
    elif a.cmd == "send":
        if len(a.args) < 3:
            print("Usage: send <PORT> <ID> <DATA>  ex: send COM3 01 01")
            sys.exit(1)
        cmd_send(a.args[0], a.args[1:3])
    elif a.cmd == "listen":
        if not a.args:
            print("Usage: listen <PORT>  ex: listen COM3")
            sys.exit(1)
        cmd_listen(a.args[0], a.duration)


if __name__ == "__main__":
    main()
