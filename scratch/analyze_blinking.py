import os
import re
import glob

log_dir = r"c:\Users\ebesa\Documents\MSPA\archive\logs"
files = glob.glob(os.path.join(log_dir, "*.txt"))

# Pattern to match: [timestamp] ... A5 1A xx Cxx Flags 0xXX (Pump=X Heat=X
pattern = re.compile(r"\[(.*?)\]\[I\]\[mspa.*?\]: (?:\[.*?\] )?\[SPA->CLAV\] A5 1A .*? Flags 0x.*? \(Pump=(\d) Heat=(\d)")

for file in files:
    try:
        with open(file, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except UnicodeDecodeError:
        try:
            with open(file, 'r', encoding='utf-16le') as f:
                lines = f.readlines()
        except:
            continue
            
    from datetime import datetime
    
    last_pump = None
    last_heat = None
    last_time = None
    changes = []
    
    for line in lines:
        match = pattern.search(line)
        if match:
            timestamp_str, pump, heat = match.groups()
            try:
                # Format: HH:MM:SS.mmm
                t = datetime.strptime(timestamp_str, "%H:%M:%S.%f")
            except ValueError:
                continue
                
            if last_pump != pump or last_heat != heat:
                if last_time:
                    delta = (t - last_time).total_seconds()
                    # Handle day wrapping (unlikely but possible)
                    if delta < 0: delta += 86400
                    changes.append(f"{timestamp_str} (+{delta:.1f}s) -> P={pump} H={heat}")
                else:
                    changes.append(f"{timestamp_str} -> P={pump} H={heat}")
                
                last_pump = pump
                last_heat = heat
                last_time = t
                
    # Report for every file processed
    if len(changes) > 5:
        print(f"\n[{len(changes):03d} clignotements] {os.path.basename(file)}")
        for c in changes:
            print(f"    {c}")
    else:
        print(f"[000 clignotements] {os.path.basename(file)}")
