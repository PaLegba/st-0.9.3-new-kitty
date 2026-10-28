#!/usr/bin/env python3
"""Convert the xpet XPM frame tree to PNG files for kitty_pet.

Usage: xpm2png.py [source_dir] [dest_dir]

The default source is ./pets and the default destination is ./pets-png.
Only the simple XPM subset used by xpet assets is supported: a single-char
pixel encoding, #rrggbb colors, and the None color. Frames are padded with
transparent pixels on the right and bottom so that kitty X/Y cell offsets can
move the visible sprite smoothly without leaving opaque overhang outside the
placement's cell box.
"""

import os
import sys

try:
    from PIL import Image
except ImportError:
    raise SystemExit("Pillow is required: pip install pillow")


def quoted_lines(path):
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if line.startswith('"'):
                line = line.rstrip(",")
                if line.endswith('"'):
                    yield line[1:-1]


PAD_RIGHT = 64
PAD_BOTTOM = 64
OUTLINE_COLOR = (255, 255, 255, 96)


def convert(src, dst):
    lines = list(quoted_lines(src))
    if not lines:
        raise ValueError("empty or invalid XPM")
    width, height, ncolors, cpp = map(int, lines[0].split())
    colors = {}
    for line in lines[1:1 + ncolors]:
        symbol = line[:cpp]
        rest = line[cpp:].strip()
        if not rest.startswith("c "):
            raise ValueError("unsupported XPM color line: %r" % line)
        value = rest[2:].strip()
        if value.lower() == "none":
            colors[symbol] = (0, 0, 0, 0)
        elif value.startswith("#") and len(value) == 7:
            colors[symbol] = (
                int(value[1:3], 16),
                int(value[3:5], 16),
                int(value[5:7], 16),
                255,
            )
        else:
            raise ValueError("unsupported XPM color: %r" % value)
    pixels = lines[1 + ncolors:1 + ncolors + height]
    if len(pixels) != height:
        raise ValueError("wrong number of XPM pixel rows")
    image = Image.new("RGBA", (width + PAD_RIGHT, height + PAD_BOTTOM))
    out = image.load()
    for y, row in enumerate(pixels):
        if len(row) != width * cpp:
            raise ValueError("wrong XPM row length")
        for x in range(width):
            symbol = row[x * cpp:(x + 1) * cpp]
            if symbol not in colors:
                raise ValueError("unknown XPM symbol %r" % symbol)
            out[x, y] = colors[symbol]
    source = image.copy()
    for y in range(height):
        for x in range(width):
            if source.getpixel((x, y))[3] != 0:
                continue
            for dy in (-1, 0, 1):
                for dx in (-1, 0, 1):
                    nx, ny = x + dx, y + dy
                    if not (0 <= nx < width and 0 <= ny < height):
                        continue
                    if source.getpixel((nx, ny))[3] != 0:
                        out[x, y] = OUTLINE_COLOR
                        break
                else:
                    continue
                break
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    image.save(dst)


def main():
    src_root = sys.argv[1] if len(sys.argv) > 1 else "pets"
    dst_root = sys.argv[2] if len(sys.argv) > 2 else "pets-png"
    count = 0
    for root, _dirs, files in os.walk(src_root):
        for name in sorted(files):
            if not name.endswith(".xpm"):
                continue
            src = os.path.join(root, name)
            rel = os.path.relpath(src, src_root)
            dst = os.path.splitext(os.path.join(dst_root, rel))[0] + ".png"
            convert(src, dst)
            count += 1
    print("converted %d files to %s" % (count, dst_root))


if __name__ == "__main__":
    main()
