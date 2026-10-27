"""Author-only sample generator and ID log viewer. Never reads a ROM/VRAM dump."""
import argparse
import collections
import hashlib
import json
from pathlib import Path
import struct
import zlib


def example(directory):
    directory.mkdir(parents=True, exist_ok=True)
    # A synthetic red 8x8 tile, not taken from any game.
    planar = [255] * 8 + [0] * 8
    palette = [0, 31] + [0] * 14
    canonical = bytes([ord('T'), 1, 4, 0]) + struct.pack('<32H', *(planar + palette))
    tile_id = hashlib.sha256(canonical).hexdigest()
    # Original geometric cyan/gold emblem, deliberately unrelated to TMNT art.
    rows = bytearray()
    for y in range(64):
        rows.append(0)
        for x in range(64):
            ring = 16 < abs(x - 31.5) + abs(y - 31.5) < 23
            cross = abs(x - 31.5) < 3 or abs(y - 31.5) < 3
            rows.extend((255, 205, 65, 255) if ring else
                        (65, 220, 230, 255) if cross else (12, 40, 60, 255))

    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))

    png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>2I5B', 64, 64, 8, 6, 0, 0, 0))
    png += chunk(b'IDAT', zlib.compress(rows)) + chunk(b'IEND', b'')
    (directory / 'author-emblem.png').write_bytes(png)
    manifest = dict(version=1, author='TMNT IV recomp contributors', license='CC0-1.0',
                    entries=[dict(id=tile_id, file='author-emblem.png')])
    (directory / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    print(tile_id)


def inspect(path):
    counts = collections.Counter()
    first = {}
    with path.open(encoding='utf-8') as stream:
        for n, line in enumerate(stream):
            if n >= 20000:
                break
            item = json.loads(line)
            key = item['id']
            counts[key] += 1
            first.setdefault(key, item)
    for key, count in counts.most_common():
        item = first[key]
        print(f"{key} layer={item['layer']} bpp={item['bpp']} oam={item['oam']} "
              f"at=({item['x']},{item['y']}) observations={count}")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('--example', type=Path)
    group.add_argument('--inspect', type=Path)
    args = parser.parse_args()
    example(args.example) if args.example else inspect(args.inspect)
