#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Per-region means of a game screenshot and a render_lab frame of the same camera.

    python3 tools/quality/game_lab_regions.py --lab frame.pfm --game shot.tga \\
        --region name=x0,y0,x1,y1 [--region ...] [--png-out DIR]

A diagnostic for matched game/lab captures (RFC 0016 K12): no pass/fail
limit is applied. The lab frame holds linear scene values; each sample is
taken to display values by Reinhard (x / (1 + x)) and the sRGB encoding, then
to bytes. The game's screenshot already holds display bytes. Regions are
pixel rectangles (x1, y1 exclusive) in the images' common size. Each region
prints its mean RGB in both and the difference (game - lab), in bytes.
"""
import argparse
import json
from pathlib import Path

import numpy as np
from PIL import Image


def read_pfm(path):
    with open(path, "rb") as f:
        if f.readline().strip() != b"PF":
            raise ValueError("%s is not an RGB PFM" % path)
        width, height = map(int, f.readline().split())
        scale = float(f.readline())
        data = np.frombuffer(f.read(), dtype="<f4" if scale < 0 else ">f4")
    return np.flipud(data.reshape(height, width, 3))  # row 0 at the top


def display_bytes(linear):
    if not np.isfinite(linear).all():
        raise ValueError("the lab frame contains nonfinite pixels")
    value = np.maximum(linear, 0.0)
    value = value / (1.0 + value)
    srgb = np.where(value <= 0.0031308, 12.92 * value,
                    1.055 * np.power(value, 1.0 / 2.4) - 0.055)
    return np.rint(np.clip(srgb, 0.0, 1.0) * 255.0).astype(np.uint8)


def parse_region(text):
    name, _, box = text.partition("=")
    x0, y0, x1, y1 = (int(v) for v in box.split(","))
    if not name or x1 <= x0 or y1 <= y0:
        raise argparse.ArgumentTypeError("bad region %r" % text)
    return name, (x0, y0, x1, y1)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--lab", type=Path, required=True)
    parser.add_argument("--game", type=Path, required=True)
    parser.add_argument("--region", type=parse_region, action="append", required=True)
    parser.add_argument("--png-out", type=Path, help="write both display images here")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()
    lab = display_bytes(read_pfm(args.lab))
    game = np.asarray(Image.open(args.game).convert("RGB"))
    if lab.shape != game.shape:
        raise SystemExit("sizes differ: lab %s, game %s" % (lab.shape, game.shape))
    if args.png_out:
        args.png_out.mkdir(parents=True, exist_ok=True)
        Image.fromarray(lab).save(args.png_out / (args.lab.stem + "-lab.png"))
        Image.fromarray(game).save(args.png_out / (args.lab.stem + "-game.png"))
    rows = []
    for name, (x0, y0, x1, y1) in args.region:
        lab_mean = lab[y0:y1, x0:x1].reshape(-1, 3).mean(0)
        game_mean = game[y0:y1, x0:x1].reshape(-1, 3).mean(0)
        rows.append({"region": name, "box": [x0, y0, x1, y1],
                     "game": [round(float(v), 2) for v in game_mean],
                     "lab": [round(float(v), 2) for v in lab_mean],
                     "difference": [round(float(v), 2) for v in game_mean - lab_mean]})
    if args.json:
        print(json.dumps(rows, indent=2))
    else:
        for row in rows:
            print("%-14s game %-24s lab %-24s game-lab %s" % (
                row["region"], row["game"], row["lab"], row["difference"]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
