path = r'esphome/components/mspa_uart/mspa_uart.h'
with open(path, 'r', encoding='utf-8') as f:
    text = f.read()

open_count = text.count('{')
close_count = text.count('}')
print(f"Open: {open_count}, Close: {close_count}")
