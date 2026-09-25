#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Diagnosis corpus: why portal-light's `room` camera regions get 0 pixels
# (RFC 0011 G10 fixture; roadmap R70; RFC 0005 Q0 gap corpus).
#
# `gi_reference.py check` fails with "closed.room region floor has 0 pixels"
# (and seven more lines like it). This suite reproduces that with an oracle that
# shares no code with the renderer or the checker: a numpy pinhole ray caster
# over the meshes authored in the fixture's own .usda layers (parsed as text).
#
#   1. The ray caster is validated against Blender/Cycles: for the `joined`
#      camera in both states its per-object pixel counts and per-pixel object
#      ids must agree with the recorded reference index images.
#   2. Reproduction: the `room` camera (a map-space pose) cast into the
#      reference stage (the rooms joined at x = 4, room B at x 4..8) sees only
#      the back of B's east wall and the void, exactly what the recorded
#      `closed.room`/`open.room` index images hold. Every region is empty.
#   3. Mechanism: `room` is the `joined` camera translated by the portal pair's
#      offset; its eye is inside room B of the map stage and outside room B of
#      the reference stage, and in the map stage it sees every region. The
#      fixture declares `reference_cameras: {room: joined}`, which
#      gi_runtime.view_masks() honors, but the reference render and
#      `gi_reference.py check` enumerate fixture["cameras"] without it.
#
# Seeded faults (sensitivity rows): `--seed-fault mirror` mirrors the caster's
# image (the Cycles cross-check must reject it) and `--seed-fault alias-room`
# applies the alias to the room camera (the reproduction must no longer hold).
#
#   python3 tools/quality/corpus/gi_portal_light.py [--seed-fault ...]
#
# ============================================================================

import argparse
import json
import math
import os
import re
import subprocess
import sys
from pathlib import Path

import numpy as np
from PIL import Image

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))

from conformance_result import Checks  # noqa: E402
from corpus import provenance  # noqa: E402

ROOT = Path(provenance.ROOT)
FIXTURE = ROOT / "quality/fixtures/gi/portal-light"
SUITE = "corpus.gi.portal-light-room-camera"
MIN_REGION_PIXELS = 50          # gi_reference.py's region floor
COUNT_TOLERANCE = 0.02          # per-object pixel-count agreement with Cycles
AGREEMENT = 0.97                # per-pixel object-id agreement with Cycles


# ---------------------------------------------------------------------------
# A minimal .usda reader: Mesh points/faces under Xform translates and the
# `visibility = "invisible"` overs of a state layer. Anything else that could
# move geometry is refused rather than ignored.
# ---------------------------------------------------------------------------

DEF = re.compile(r'^\s*(def|over)\s+(\w+\s+)?"(\w+)"')
TUPLE = re.compile(r"\(([^()]*)\)")


def _floats(text):
    return [tuple(float(v) for v in m.group(1).split(",")) for m in TUPLE.finditer(text)]


def _ints(text):
    return [int(v) for v in text.strip().strip("[]").split(",") if v.strip()]


def read_layer(path):
    """{mesh name: {"points", "counts", "indices", "offset"}} and hidden names."""
    meshes, hidden, stack, lights = {}, set(), [], {}
    depth = 0
    pending = None
    for line in path.read_text().splitlines():
        match = DEF.match(line)
        if match:
            pending = (match.group(2) or "").strip(), match.group(3)
        if "{" in line and pending is not None:
            stack.append((depth, pending[0], pending[1], [0.0, 0.0, 0.0]))
            pending = None
        depth += line.count("{") - line.count("}")
        while stack and depth <= stack[-1][0]:
            stack.pop()
        if not stack:
            continue
        kind, name, offset = stack[-1][1], stack[-1][2], stack[-1][3]
        stripped = line.strip()
        if stripped.startswith("double3 xformOp:translate"):
            offset[:] = _floats(stripped.split("=", 1)[1])[0]
        elif "xformOp:" in stripped and not stripped.startswith("uniform token[] xformOpOrder") \
                and kind != "Camera":
            raise ValueError("%s: unsupported transform on %s: %s" % (path.name, name, stripped))
        elif stripped.startswith('token visibility = "invisible"'):
            hidden.add(name)
        elif kind == "RectLight" and stripped.startswith(("float inputs:width",
                                                           "float inputs:height")):
            # An area light is an emitter quad in the render (the pipeline's
            # documented `LightQuadNN` naming, map_scene.EMITTER_PREFIXES): an
            # unrotated USD RectLight lies in its local XY plane.
            light = lights.setdefault(name, {})
            light["width" if "width" in stripped else "height"] = float(stripped.split("=")[1])
            light["offset"] = offset
        elif kind == "Mesh":
            mesh = meshes.setdefault(name, {"counts": [], "indices": [], "points": []})
            mesh["offset"] = tuple(sum(entry[3][i] for entry in stack) for i in range(3))
            if stripped.startswith("point3f[] points"):
                mesh["points"] = _floats(stripped.split("=", 1)[1])
            elif stripped.startswith("int[] faceVertexCounts"):
                mesh["counts"] = _ints(stripped.split("=", 1)[1])
            elif stripped.startswith("int[] faceVertexIndices"):
                mesh["indices"] = _ints(stripped.split("=", 1)[1])
    for number, (name, light) in enumerate(sorted(lights.items())):
        w, h = light["width"] / 2.0, light["height"] / 2.0
        meshes["LightQuad%02d" % number] = {
            "points": [(-w, -h, 0.0), (w, -h, 0.0), (w, h, 0.0), (-w, h, 0.0)],
            "counts": [4], "indices": [0, 1, 2, 3], "offset": tuple(light["offset"])}
    return meshes, hidden


def triangles(meshes, hidden=()):
    """(names, (T, 3, 3) triangle array, (T,) mesh id) of the visible meshes."""
    names, tris, owner = [], [], []
    for name, mesh in sorted(meshes.items()):
        if name in hidden or not mesh["points"]:
            continue
        points = np.asarray(mesh["points"]) + np.asarray(mesh["offset"])
        at = 0
        for count in mesh["counts"]:
            face = mesh["indices"][at:at + count]
            at += count
            for k in range(1, count - 1):
                tris.append(points[[face[0], face[k], face[k + 1]]])
                owner.append(len(names))
        names.append(name)
    return names, np.asarray(tris), np.asarray(owner)


def rays(pose, film, horizontal_fov, mirror=False):
    """Blender's HORIZONTAL-fit pinhole: (origin, (H*W, 3) unit directions), row 0 on top."""
    forward = np.asarray(pose["forward"], float)
    forward /= np.linalg.norm(forward)
    right = np.cross(forward, pose["up"])
    right /= np.linalg.norm(right)
    up = np.cross(right, forward)
    width, height = film["width"], film["height"]
    half = math.tan(math.radians(horizontal_fov) / 2.0)
    xs = ((np.arange(width) + 0.5) / width * 2.0 - 1.0) * half
    ys = (1.0 - (np.arange(height) + 0.5) / height * 2.0) * half * height / width
    if mirror:
        xs = xs[::-1]
    gx, gy = np.meshgrid(xs, ys)
    directions = forward + gx[..., None] * right + gy[..., None] * up
    directions /= np.linalg.norm(directions, axis=-1, keepdims=True)
    return np.asarray(pose["eye"], float), directions.reshape(-1, 3)


def cast(origin, directions, tris, owner, chunk=4096):
    """Nearest-hit mesh id per ray (-1 for the void), Moller-Trumbore."""
    v0, e1, e2 = tris[:, 0], tris[:, 1] - tris[:, 0], tris[:, 2] - tris[:, 0]
    tvec = origin - v0
    qvec = np.cross(tvec, e1)
    hit = np.full(len(directions), -1)
    for start in range(0, len(directions), chunk):
        d = directions[start:start + chunk]
        p = np.cross(d[:, None, :], e2[None])
        det = np.einsum("tk,rtk->rt", e1, p)
        with np.errstate(divide="ignore", invalid="ignore"):
            inv = 1.0 / det
            u = np.einsum("tk,rtk->rt", tvec, p) * inv
            v = np.einsum("rk,tk->rt", d, qvec) * inv
            t = np.einsum("tk,tk->t", e2, qvec)[None] * inv
        valid = (np.abs(det) > 1e-12) & (u >= 0) & (v >= 0) & (u + v <= 1) & (t > 1e-6)
        t = np.where(valid, t, np.inf)
        nearest = np.argmin(t, axis=1)
        found = np.isfinite(t[np.arange(len(d)), nearest])
        hit[start:start + chunk] = np.where(found, owner[nearest], -1)
    return hit


def object_image(names, hit, object_index, shape):
    """Cast result as Blender object indices (0 for the void)."""
    lookup = np.asarray([object_index.get(n, -2) for n in names] + [0])
    return lookup[hit].reshape(shape)


def histogram(image):
    values, counts = np.unique(image, return_counts=True)
    return dict(zip(values.tolist(), counts.tolist()))


def region_pixels(image, object_index, regions, props):
    """Unfiltered pixels per region (props map a model name to its stand-in mesh)."""
    out = {}
    for region, entries in regions.items():
        numbers = [object_index[props.get(e, e)] for e in entries]
        out[region] = int(np.isin(image, numbers).sum())
    return out


def box_contains(box, point):
    low, high = box
    return all(low[i] < point[i] < high[i] for i in range(3))


def room_box(meshes, prefix):
    points = np.concatenate([np.asarray(m["points"]) + np.asarray(m["offset"])
                             for n, m in meshes.items() if n.startswith(prefix) and m["points"]])
    return points.min(axis=0), points.max(axis=0)


def main(argv=None):
    parser = argparse.ArgumentParser(description="Diagnose portal-light's empty room-camera regions.")
    parser.add_argument("--seed-fault", choices=("mirror", "alias-room"))
    parser.add_argument("--skip-harness", action="store_true",
                        help="do not run gi_reference.py check (check 4)")
    args = parser.parse_args(argv)

    fixture = json.loads((FIXTURE / "fixture.json").read_text())
    record = json.loads((FIXTURE / "references/references.json").read_text())
    film, fov = fixture["film"], fixture["horizontal_fov_degrees"]
    shape = (film["height"], film["width"])
    props = {m["name"]: m["stand_in_mesh"] for m in fixture.get("dynamic_models", [])}
    offset = np.asarray(fixture["portals"]["offset_m"], float)
    mirror = args.seed_fault == "mirror"

    stage, _ = read_layer(FIXTURE / fixture["stage"])
    layers = {"closed": (stage, set())}
    open_overs = read_layer(FIXTURE / fixture["states"]["open"]["layer"])[1]
    layers["open"] = (stage, open_overs)
    map_stage, _ = read_layer(FIXTURE / "map.usda")

    checks = Checks()
    provenance.emit(provenance.collect(
        SUITE, tools={"numpy": np.__version__},
        inputs={"fixture": str(FIXTURE / "fixture.json"),
                "references": str(FIXTURE / "references/references.json"),
                "stage": str(FIXTURE / fixture["stage"]),
                "open_layer": str(FIXTURE / fixture["states"]["open"]["layer"]),
                "map_stage": str(FIXTURE / "map.usda"),
                "renderer": str(ROOT / "tools/quality/gi_reference_blender.py"),
                "checker": str(ROOT / "tools/quality/gi_reference.py")},
        extra={"seed_fault": args.seed_fault}))

    def recorded(view_name):
        view = record["views"][view_name]
        image = np.asarray(Image.open(FIXTURE / "references" / view["files"]["index"]["file"]))
        return view, image.astype(np.int64)

    # 1. The caster agrees with Cycles where the camera is where it belongs.
    for state, (meshes, hidden) in layers.items():
        view, reference = recorded("%s.joined" % state)
        names, tris, owner = triangles(meshes, hidden)
        origin, directions = rays(fixture["cameras"]["joined"], film, fov, mirror)
        image = object_image(names, cast(origin, directions, tris, owner),
                             view["object_index"], shape)
        ours, theirs = histogram(image), histogram(reference)
        for number in sorted(set(ours) | set(theirs)):
            a, b = ours.get(number, 0), theirs.get(number, 0)
            checks.check(abs(a - b) <= max(COUNT_TOLERANCE * b, 60),
                         "raycast.count.%s.joined.%d" % (state, number),
                         "caster %d px, Cycles %d px" % (a, b))
        agreement = float((image == reference).mean())
        checks.check(agreement >= AGREEMENT, "raycast.agreement.%s.joined" % state,
                     "%.4f of pixels agree with Cycles (need %.2f)" % (agreement, AGREEMENT))

    # 2. Reproduction: the room camera in the reference stage.
    room_pose = dict(fixture["cameras"]["room"])
    if args.seed_fault == "alias-room":
        room_pose = dict(fixture["cameras"][fixture["reference_cameras"]["room"]])
    for state, (meshes, hidden) in layers.items():
        view, reference = recorded("%s.room" % state)
        names, tris, owner = triangles(meshes, hidden)
        origin, directions = rays(room_pose, film, fov, mirror)
        image = object_image(names, cast(origin, directions, tris, owner),
                             view["object_index"], shape)
        pixels = region_pixels(image, view["object_index"], fixture["regions"]["room"], props)
        for region, count in sorted(pixels.items()):
            checks.check(count == 0, "reproduce.%s.room.%s-empty" % (state, region),
                         "the room camera sees %d px of %s in the reference stage" %
                         (count, region))
        seen = {n for n, i in view["object_index"].items() if i in histogram(image)}
        checks.equal(sorted(seen), ["B_Xp"], "reproduce.%s.room.only-b-east-wall" % state)
        recorded_seen = {n for n, i in view["object_index"].items() if i in histogram(reference)}
        checks.equal(sorted(recorded_seen), sorted(seen),
                     "reproduce.%s.room.matches-cycles" % state)
        checks.equal({k: v["pixels"] for k, v in view["regions"].items()},
                     {k: 0 for k in fixture["regions"]["room"]},
                     "reproduce.%s.room.recorded-regions-empty" % state)

    # 3. Mechanism: `room` is `joined` in map space.
    room, joined = fixture["cameras"]["room"], fixture["cameras"]["joined"]
    checks.check(np.allclose(np.asarray(room["eye"]) - joined["eye"], offset),
                 "mechanism.room-is-joined-plus-portal-offset",
                 "eye delta %s, portal offset %s" %
                 ((np.asarray(room["eye"]) - joined["eye"]).tolist(), offset.tolist()))
    checks.check(np.allclose(room["forward"], joined["forward"]) and
                 np.allclose(room["up"], joined["up"]), "mechanism.room-orientation-is-joined")
    checks.equal(fixture.get("reference_cameras"), {"room": "joined"},
                 "mechanism.fixture-declares-alias")
    reference_b = room_box(stage, "B_")
    map_b = room_box(map_stage, "B_")
    checks.check(not box_contains(reference_b, room["eye"]),
                 "mechanism.room-eye-outside-reference-room-b",
                 "reference room B %s contains %s" % (
                     [b.tolist() for b in reference_b], room["eye"]))
    checks.check(box_contains(map_b, room["eye"]), "mechanism.room-eye-inside-map-room-b",
                 "map room B %s does not contain %s" % ([b.tolist() for b in map_b], room["eye"]))
    names, tris, owner = triangles(map_stage)
    object_index = {n: i + 1 for i, n in enumerate(names)}
    origin, directions = rays(room, film, fov, mirror)
    image = object_image(names, cast(origin, directions, tris, owner), object_index, shape)
    for region, count in sorted(region_pixels(image, object_index, fixture["regions"]["room"],
                                              props).items()):
        checks.check(count >= MIN_REGION_PIXELS, "mechanism.map-room.%s-visible" % region,
                     "%d px" % count)

    # 4. The harness: gi_reference.py check reports exactly the aliased camera.
    if not args.skip_harness:
        run = subprocess.run([sys.executable, str(ROOT / "tools/quality/gi_reference.py"),
                              "check"], capture_output=True, text=True, cwd=ROOT)
        lines = [ln for ln in run.stdout.splitlines() if ln.startswith("FAIL portal-light:")]
        zero = [ln for ln in lines if "has 0 pixels" in ln]
        expected = sorted("FAIL portal-light: %s.room region %s has 0 pixels" % (s, r)
                          for s in fixture["states"] for r in fixture["regions"]["room"])
        checks.equal(sorted(zero), expected, "harness.zero-pixel-lines-are-the-room-camera")
        checks.equal(sorted(set(lines) - set(zero)), [], "harness.no-other-portal-light-failure")

    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
