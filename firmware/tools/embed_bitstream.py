#!/usr/bin/env python3
from pathlib import Path

root = Path(__file__).resolve().parent.parent
src = root / "fpga" / "top.bin"
dst = root / "src" / "fpga_bitstream.c"
data = src.read_bytes()

with dst.open("w") as f:
    f.write("/* Generated from firmware/fpga/top.bin. Do not edit. */\n")
    f.write("#include <stdint.h>\n\n")
    f.write("const uint32_t fpga_bitstream_len = %d;\n" % len(data))
    f.write("const uint8_t fpga_bitstream[] = {\n")
    for i in range(0, len(data), 16):
        f.write("    " + ", ".join("0x%02x" % b for b in data[i:i + 16]) + ",\n")
    f.write("};\n")

print("embedded %d bytes into %s" % (len(data), dst))
