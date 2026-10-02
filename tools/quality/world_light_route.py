#!/usr/bin/env python3
"""A* camera traversal over BSP-checked anchors, with live lighting-layer stops.

This diagnoses view continuity, not player locomotion: cameras move in noclip
through segments whose swept clearance box never enters solid BSP.
Static props and gameplay gates are not navigation evidence. Each completed
capture is checked while the product runs; the first excessive layer change
stops that private product and retains the two images and their positions.
"""

import argparse
import hashlib
import heapq
import json
import math
import os
from pathlib import Path
import signal
import struct
import subprocess
import sys
import time

import bsp2_reader

LAYERS = (("albedo", 1), ("baked", 8), ("direct", 9), ("specular", 10))


class Collision:
    def __init__(self, path):
        bsp = bsp2_reader.Bsp2File(Path(path).read_bytes())
        self.planes, self.nodes, self.leaves = (bsp.legacy_lump(i) for i in (1, 5, 10))
        if not all((self.planes, self.nodes, self.leaves)):
            raise ValueError("map has no BSP collision tree")

    def clear(self, start, end, radius=0):
        """Clip a swept box against each BSP halfspace; thin walls cannot be skipped."""
        stack = [(0, tuple(x / .0254 for x in start), tuple(x / .0254 for x in end))]
        while stack:
            node, a, b = stack.pop()
            if node < 0:
                if struct.unpack_from("<i", self.leaves, (-1 - node) * 32)[0] & 1:
                    return False
                continue
            plane, left, right = struct.unpack_from("<iii", self.nodes, node * 32)
            normal = struct.unpack_from("<4f", self.planes, plane * 20)
            da, db = (sum(p[i] * normal[i] for i in range(3)) - normal[3] for p in (a, b))
            extent = radius / .0254 * sum(abs(normal[i]) for i in range(3))
            for child, side in ((left, 1), (right, -1)):
                d0, d1 = da * side + extent, db * side + extent
                if d0 < 0 and d1 < 0:
                    continue
                if d0 >= 0 and d1 >= 0:
                    stack.append((child, a, b))
                else:
                    t = d0 / (d0 - d1)
                    mid = tuple(a[i] + t * (b[i] - a[i]) for i in range(3))
                    stack.append((child, a, mid) if d0 >= 0 else (child, mid, b))
        return True

    def sweep(self, start, end, radius):
        return self.clear(start, end, radius)


def route(collision, anchors, start, goal, radius=.2, edge=12):
    points = {item["name"]: item["capture"] for item in anchors}
    if start not in points or goal not in points:
        raise ValueError("start and goal must name anchors")
    graph = {name: [] for name in points}
    for a, pa in points.items():
        for b, pb in points.items():
            distance = math.dist(pa, pb)
            if a < b and distance <= edge and collision.sweep(pa, pb, radius):
                graph[a].append((b, distance))
                graph[b].append((a, distance))
    pending, costs, previous = [(0, start)], {start: 0}, {}
    while pending:
        _, current = heapq.heappop(pending)
        if current == goal:
            result = [goal]
            while result[-1] != start:
                result.append(previous[result[-1]])
            return [dict(name=name, eye_m=points[name]) for name in reversed(result)]
        for neighbour, distance in graph[current]:
            cost = costs[current] + distance
            if cost < costs.get(neighbour, math.inf):
                costs[neighbour] = cost
                previous[neighbour] = current
                heapq.heappush(pending, (cost + math.dist(points[neighbour], points[goal]), neighbour))
    raise ValueError("no BSP-clear route between the selected anchors")


def samples(path, step):
    points = [path[0]["eye_m"]]
    for a, b in zip(path, path[1:]):
        count = math.ceil(math.dist(a["eye_m"], b["eye_m"]) / step)
        points += [[a["eye_m"][i] + (b["eye_m"][i] - a["eye_m"][i]) * n / count
                    for i in range(3)] for n in range(1, count + 1)]
    return points


def metric(path):
    import numpy as np
    from PIL import Image
    with Image.open(path) as image:
        pixels = np.asarray(image.convert("RGB"), dtype=float) / 255
    h, w, _ = pixels.shape
    pixels = pixels[h // 5:3 * h // 5, w // 5:4 * w // 5]
    # The catalog's not-applicable hatch is 25%/50% linear grey (137/188
    # in an sRGB capture), in alternating four-pixel runs. Report its coverage
    # separately: it says that a material has no such lighting term.
    low = np.all(np.abs(pixels * 255 - 137) <= 1, axis=2)
    high = np.all(np.abs(pixels * 255 - 188) <= 1, axis=2)
    hatch = ((low & (np.roll(high, 4, axis=1) | np.roll(high, -4, axis=1))) |
             (high & (np.roll(low, 4, axis=1) | np.roll(low, -4, axis=1))))
    linear = np.where(pixels <= .04045, pixels / 12.92, ((pixels + .055) / 1.055) ** 2.4)
    luminance = linear @ np.array([.2126, .7152, .0722])
    field = luminance[~hatch]
    if not field.size:
        raise ValueError("lighting layer has no applicable pixels")
    return {"mean": float(field.mean()), "p95": float(np.percentile(field, 95)),
            "hatch_fraction": float(hatch.mean())}


def layer_change(before, after, statistic):
    return abs(after[statistic] - before[statistic]) / max(before[statistic], after[statistic], .002)


def stop_product(runtime):
    for proc in Path("/proc").iterdir():
        if not proc.name.isdigit():
            continue
        try:
            if (proc / "cwd").resolve() == runtime.resolve():
                os.killpg(int(proc.name), signal.SIGTERM)
        except (OSError, ProcessLookupError):
            pass


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("map", "anchors", "out"):
        parser.add_argument("--" + name, type=Path, required=True)
    parser.add_argument("--start", required=True)
    parser.add_argument("--goal", required=True)
    parser.add_argument("--step", type=float, default=.5)
    parser.add_argument("--clearance", type=float, default=.2)
    parser.add_argument("--max-layer-change", type=float, default=.75)
    parser.add_argument("--statistic", choices=("mean", "p95"), default="p95",
                        help="p95 judges the layer's bright field without isolated hatch pixels")
    parser.add_argument("--yaw", type=float, default=20)
    parser.add_argument("--pitch", type=float, default=8)
    parser.add_argument("--runtime", type=Path)
    parser.add_argument("--build", type=Path)
    parser.add_argument("--timeout", type=float, default=300)
    args = parser.parse_args(argv)
    if args.step <= 0 or args.clearance <= 0 or args.max_layer_change <= 0:
        parser.error("step, clearance and maximum layer change must be positive")
    collision = Collision(args.map)
    path = route(collision, json.loads(args.anchors.read_text()), args.start, args.goal, args.clearance)
    points = samples(path, args.step)
    args.out.mkdir(parents=True, exist_ok=False)
    evidence = {"schema": "world-light-route/v1", "status": "planned", "route": path,
                "points": points, "map_sha256": hashlib.sha256(args.map.read_bytes()).hexdigest(),
                "clearance_m": args.clearance, "max_layer_change": args.max_layer_change,
                "statistic": args.statistic,
                "coverage": __doc__, "captures": []}
    receipt = args.out / "route.json"
    receipt.write_text(json.dumps(evidence, indent=2) + "\n")
    if not args.runtime or not args.build:
        print("A* route:", " -> ".join(p["name"] for p in path), "(%d samples)" % len(points))
        return 0
    commands = ["host_framerate .015", "cmd noclip", "r_drawviewmodel 0",
                "mat_force_tonemap_scale 1", "cmd setang %g %g 0" % (args.pitch, args.yaw)]
    # Source's setpos names the player's feet; the standing eye is 64 units up.
    for point in points:
        feet = [point[0] / .0254, point[1] / .0254, point[2] / .0254 - 64]
        commands += ["cmd setpos %.6f %.6f %.6f" % tuple(feet), "wait 12"]
        for _, view in LAYERS:
            commands += ["cl_render_debug_view %d" % view, "wait 12", "screenshot", "wait 8"]
    commands += ["cl_render_debug_view 0", "wait 12"]
    boot = args.out / "boot"
    command = [sys.executable, str(Path(__file__).with_name("portal_boot.py")),
               "--runtime", str(args.runtime), "--build", str(args.build),
               "--content-root", str(args.map.parent.parent), "--game", "portal2",
               "--renderer", "native-vulkan", "--headless", "--require-vulkan",
               "--map", args.map.stem, "--out", str(boot), "--timeout", str(args.timeout),
               "--startup-command", "r_core_world 1", "--startup-command", "r_core_world_strict 1",
               "--capture-wait", "30"]
    for item in commands:
        command += ["--console-command", item]
    previous, processed, stopped = {}, set(), False
    with (args.out / "runner.log").open("w") as log:
        process = subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT)
        while True:
            frames = sorted((boot / "runtime/portal2/screenshots").glob("*.tga"))
            for index, frame in enumerate(frames[:len(points) * len(LAYERS)]):
                if frame in processed:
                    continue
                try:
                    measured = metric(frame)
                except (OSError, ValueError):
                    break  # the readback is still being written
                point, layer = index // len(LAYERS), LAYERS[index % len(LAYERS)][0]
                record = {"point": point, "eye_m": points[point], "layer": layer,
                          "image": str(frame.resolve()), **measured}
                old = previous.get(layer)
                if old:
                    delta = layer_change(old, measured, args.statistic)
                    record["relative_change"] = delta
                    if delta > args.max_layer_change:
                        evidence["first_divergence"] = {"before": old, "after": record}
                        stopped = True
                evidence["captures"].append(record)
                processed.add(frame)
                previous[layer] = record
                if stopped:
                    stop_product(boot / "runtime")
                    break
            receipt.write_text(json.dumps(evidence, indent=2) + "\n")
            if stopped:
                process.wait(timeout=15)
                break
            if process.poll() is not None:
                break
            time.sleep(.1)
    boot_receipt = boot / "evidence.json"
    passed = boot_receipt.is_file() and json.loads(boot_receipt.read_text())["status"] == "pass"
    complete = len(processed) == len(points) * len(LAYERS)
    evidence["status"] = "halted" if stopped else "pass" if passed and complete else "fail"
    receipt.write_text(json.dumps(evidence, indent=2) + "\n")
    print("Lighting route:", evidence["status"], "(%s)" % receipt)
    return 0 if evidence["status"] == "pass" else 1


if __name__ == "__main__":
    raise SystemExit(main())
