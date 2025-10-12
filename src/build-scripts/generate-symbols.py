#!/usr/bin/env python3
import subprocess
import re

binary_path = "./kernel.elf"
output_c_file = "./src/generated/symbols.c"

result = subprocess.run(["nm", "-S", "-n", binary_path], capture_output=True, text=True)

symbol_pattern = re.compile(r"([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+([A-Za-z])\s+(.+)")

symbols = []

for line in result.stdout.splitlines():
    match = symbol_pattern.match(line)
    if match:
        address, size, type_char, name = match.groups()
        if type_char.lower() == 't':
            address = int(address, 16)
            size = int(size, 16)
            if not name.endswith("_isr"):
                symbols.append((name, address, size))

with open(output_c_file, "w") as f:
    f.write("typedef struct {\n")
    f.write("    const char* name;\n")
    f.write("    unsigned long address;\n")
    f.write("    unsigned long size;\n")
    f.write("} symbol;\n\n")

    f.write("symbol symbols[] = {\n")
    for name, address, size in symbols:
        f.write(f'    {{"{name}", 0x{address:x}, 0x{size:x}}},\n')
    f.write("};\n\n")
    f.write(f"unsigned long symbol_count = {len(symbols)};\n")

if len(symbols) != 0:
    print(f"Generated {output_c_file} with {len(symbols)} function symbols.")
else:
    with open(output_c_file, "w") as f:
        f.write("typedef struct {\n")
        f.write("    const char* name;\n")
        f.write("    unsigned long address;\n")
        f.write("    unsigned long size;\n")
        f.write("} symbol;\n\n")

        f.write("symbol symbols[1];\n")

        f.write(f"unsigned long symbol_count = {len(symbols)};\n")
        print(f"Generated dummy {output_c_file}")
