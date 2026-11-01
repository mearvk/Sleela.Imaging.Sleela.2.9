#!/usr/bin/env python3
"""Dependency-free binary-PPM (P6) to PNG converter (stdlib zlib only).

Usage: ppm_to_png.py <in.ppm> <out.png>
"""
import struct
import sys
import zlib


def read_ppm(path):
    with open(path, "rb") as f:
        data = f.read()
    if not data.startswith(b"P6"):
        raise SystemExit("not a binary P6 PPM")
    # Parse header: P6 <w> <h> <maxval>, tokens separated by whitespace.
    idx = 2
    tokens = []
    while len(tokens) < 3:
        # skip whitespace and comments
        while idx < len(data) and data[idx:idx + 1].isspace():
            idx += 1
        if data[idx:idx + 1] == b"#":
            while idx < len(data) and data[idx:idx + 1] != b"\n":
                idx += 1
            continue
        start = idx
        while idx < len(data) and not data[idx:idx + 1].isspace():
            idx += 1
        tokens.append(data[start:idx])
    w, h, maxval = (int(t) for t in tokens)
    idx += 1  # single whitespace after maxval
    pixels = data[idx:idx + w * h * 3]
    return w, h, pixels


def write_png(path, w, h, rgb):
    def chunk(tag, payload):
        c = tag + payload
        return struct.pack(">I", len(payload)) + c + struct.pack(">I", zlib.crc32(c) & 0xFFFFFFFF)

    sig = b"\x89PNG\r\n\x1a\n"
    ihdr = struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)  # 8-bit, truecolor RGB
    raw = bytearray()
    stride = w * 3
    for y in range(h):
        raw.append(0)  # filter: none
        raw.extend(rgb[y * stride:(y + 1) * stride])
    idat = zlib.compress(bytes(raw), 9)
    with open(path, "wb") as f:
        f.write(sig)
        f.write(chunk(b"IHDR", ihdr))
        f.write(chunk(b"IDAT", idat))
        f.write(chunk(b"IEND", b""))


def main():
    if len(sys.argv) != 3:
        raise SystemExit("usage: ppm_to_png.py <in.ppm> <out.png>")
    w, h, rgb = read_ppm(sys.argv[1])
    if len(rgb) < w * h * 3:
        raise SystemExit("truncated PPM pixel data")
    write_png(sys.argv[2], w, h, rgb)
    print(f"wrote {sys.argv[2]} ({w}x{h})")


if __name__ == "__main__":
    main()
