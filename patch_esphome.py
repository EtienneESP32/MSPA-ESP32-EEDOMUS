import sys

path = r'C:\Users\ebesa\AppData\Local\Programs\Python\Python312\Lib\site-packages\esphome\core\scheduler.h'
with open(path, 'r', encoding='utf-8') as f:
    lines = f.readlines()

if 'protected:' in lines[109]:
    lines[109] = lines[109].replace('protected:', 'public:')
    with open(path, 'w', encoding='utf-8') as f:
        f.writelines(lines)
    print("Patch applied successfully.")
else:
    print("Target line not found or already patched.")
    print(f"Line 110 content: {repr(lines[109])}")
