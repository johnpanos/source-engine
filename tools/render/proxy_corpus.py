#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""The material proxy corpus (RFC 0016 K4 "Proxy corpus", render.material.proxies).

    python3 tools/render/proxy_corpus.py inventory [--check]
    python3 tools/render/proxy_corpus.py materials --out DIR
    python3 tools/render/proxy_corpus.py capture --game portal|portal2 --out DIR [--record]
    python3 tools/render/proxy_corpus.py compare A.jsonl B.jsonl
    python3 tools/render/proxy_corpus.py selftest

inventory scans the Portal and Portal 2 client sources (the files their VPC
projects select for Linux, through scripts/waifulib/vpc_parser.py) for material
proxy registrations (EXPOSE_MATERIAL_PROXY and EXPOSE_INTERFACE of an
IMaterialProxy) and writes quality/fixtures/render-material/proxy-inventory-v1.json:
per game, each proxy's name, class and source location. --check fails when the
sources changed without it.

materials writes one fixture material per proxy (RECIPES below: a VMT whose
Proxies block runs that proxy on variables the recipe seeds) as a material
root for tools/quality/portal_boot.py --material-root.

capture boots the game headless (portal_boot.py, host_framerate 0.015,
-deterministicrender) with that content and runs the client's
mat_proxy_capture command twice ("wait 40" apart: 20 frames, 0.3 s). Each capture holds the
proxies the client registers and, for every fixture material, every material
variable after its proxies bind with no proxy data and with the local player
(game/client/material_proxy_capture.cpp). The check requires: the registered
proxies equal the inventory; every registered proxy has a recipe; every
fixture material loads and has its proxy; and both captures equal the recorded
legacy capture (quality/fixtures/render-material/proxy-capture-v1/<game>/)
under compare(). --record writes that fixture instead.

compare is the comparator the core side will use at K3: headers (time, frame,
player), the proxy list, and per material and pass every variable's type and
value, exactly for integers, strings and textures and within 1e-5 (relative
above 1) for floats, vectors and matrices. It prints the first divergence.
selftest runs it against seeded differences.
"""

import argparse
import copy
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
sys.path.insert(0, str(ROOT / "scripts" / "waifulib"))
from conformance_result import Checks  # noqa: E402

FIXTURES = ROOT / "quality" / "fixtures" / "render-material"
INVENTORY = FIXTURES / "proxy-inventory-v1.json"
CAPTURES = FIXTURES / "proxy-capture-v1"
INVENTORY_SCHEMA = "render-proxy-inventory/v1"
CLIENT_PROJECTS = {
    "portal": ["client_base.vpc", "client_portal.vpc"],
    "portal2": ["client_base.vpc", "client_portal_base.vpc", "client_portal2.vpc"],
}
LINUX_DEFINES = ["LINUX=1", "_LINUX=1", "POSIX=1"]
MAPS = {"portal": "testchmb_a_00", "portal2": "sp_a1_wakeup"}
PATTERN = "materials/proxy_corpus/*.vmt"
TOLERANCE = 1e-5
REGISTRATION = re.compile(
    r"EXPOSE_(?:MATERIAL_PROXY\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)|"
    r"INTERFACE\s*\(\s*(\w+)\s*,\s*IMaterialProxy\s*,\s*\"(\w+)\"\s*"
    r"IMATERIAL_PROXY_INTERFACE_VERSION\s*\))", re.S)


class CorpusError(Exception):
    pass


# ---------------------------------------------------------------------------
# Inventory


def client_sources(game):
    """Repository-relative client sources the game's VPC projects select."""
    import vpc_parser

    class Env:
        DEFINES = LINUX_DEFINES
        SUBPROJECT_PATH = [str(ROOT / "game" / "client")]

    cwd = os.getcwd()
    try:
        project = vpc_parser.parse_vpcs(Env, CLIENT_PROJECTS[game], "../..")
    finally:
        os.chdir(cwd)
    return sorted({os.path.relpath(os.path.normpath(ROOT / "game" / "client" / source), ROOT)
                   for source in project["sources"]})


def uncommented(text):
    """C++ text with comments and #if 0 blocks blanked, keeping line numbers."""
    out, i, n = [], 0, len(text)
    while i < n:
        if text.startswith("//", i):
            end = text.find("\n", i)
            end = n if end < 0 else end
            out.append(" " * (end - i))
            i = end
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            end = n if end < 0 else end + 2
            out.append("".join(c if c == "\n" else " " for c in text[i:end]))
            i = end
        elif text[i] == '"':
            end = i + 1
            while end < n and text[end] != '"' and text[end] != "\n":
                end += 2 if text[end] == "\\" else 1
            out.append(text[i:end + 1])
            i = end + 1
        else:
            out.append(text[i])
            i += 1
    lines, depth = "".join(out).split("\n"), 0
    for index, line in enumerate(lines):
        stripped = line.strip()
        if depth:
            if re.match(r"#\s*if", stripped):
                depth += 1
            elif re.match(r"#\s*endif", stripped):
                depth -= 1
            elif depth == 1 and re.match(r"#\s*(else|elif)", stripped):
                depth = 0
            lines[index] = ""
        elif re.match(r"#\s*if\s+0\b", stripped):
            depth = 1
            lines[index] = ""
    return "\n".join(lines)


def scan(game):
    proxies = []
    for relative in client_sources(game):
        path = ROOT / relative
        if not path.is_file():
            continue
        text = uncommented(path.read_text(errors="replace"))
        for match in REGISTRATION.finditer(text):
            proxies.append({"name": match.group(2) or match.group(4),
                            "class": match.group(1) or match.group(3),
                            "file": relative.replace(os.sep, "/"),
                            "line": text.count("\n", 0, match.start()) + 1})
    return sorted(proxies, key=lambda p: (p["name"].lower(), p["file"]))


def inventory():
    return {"schema": INVENTORY_SCHEMA,
            "description": "Material proxies the Portal and Portal 2 clients register (RFC 0016 K4 "
                           "proxy corpus). Written by tools/render/proxy_corpus.py inventory from "
                           "the client sources their VPC projects select on Linux; the capture "
                           "checks it against the proxies each client registers at run time.",
            "games": {game: scan(game) for game in CLIENT_PROJECTS}}


def command_inventory(args):
    current = inventory()
    if args.check:
        checks = Checks()
        recorded = json.loads(INVENTORY.read_text()) if INVENTORY.is_file() else {}
        for game, proxies in current["games"].items():
            checks.check(len(proxies) > 0, "%s.proxies-found" % game)
            names = [p["name"] for p in proxies]
            checks.check(len(names) == len(set(names)), "%s.one-registration-per-name" % game,
                         "registered twice: %s" % sorted({n for n in names if names.count(n) > 1}))
            checks.equal(proxies, recorded.get("games", {}).get(game),
                         "%s.inventory-current" % game)
            checks.check(all(p["name"] in RECIPES for p in proxies),
                         "%s.every-proxy-has-a-recipe" % game,
                         "no recipe: %s" % sorted(p["name"] for p in proxies
                                                  if p["name"] not in RECIPES))
        return checks.report()
    INVENTORY.parent.mkdir(parents=True, exist_ok=True)
    INVENTORY.write_text(json.dumps(current, indent=1) + "\n")
    for game, proxies in current["games"].items():
        print("%s: %d registrations, %d names" % (game, len(proxies),
                                                  len({p["name"] for p in proxies})))
    return 0


# ---------------------------------------------------------------------------
# Recipes: one fixture material per proxy

BASE = {"$basetexture": "vgui/white"}
ANIMATED = {"$basetexture": "sprites/zerogxplode", "$frame": "0"}
MATH_VARS = {"$src1": "[0.5 -0.25 1.75]", "$src2": "[2 4 0.5]", "$out": "[0 0 0]",
             "$one": "0.6", "$two": "0.4", "$out1": "0"}
RESULT = {"$out1": "0", "$out": "[0 0 0]"}


# The proxy data a capture binds with: "none" (world surfaces), "player" (a
# model's renderable), or "unbound" (not bound: the proxy's data is a
# structure of its own).
ANY = "none player"
NONE_ONLY = "none"  # the proxy casts its data to one entity class


def recipe(params, variables=None, shader="UnlitGeneric", passes=ANY):
    merged = dict(BASE)
    merged.update(variables or {})
    merged["$proxycapturepasses"] = passes
    return {"shader": shader, "vars": merged, "params": params, "passes": passes}


def math(params):
    return recipe(dict(params), MATH_VARS)


RECIPES = {
    # mathproxy.cpp: the function proxies.
    "Add": math({"srcVar1": "$src1", "srcVar2": "$src2", "resultVar": "$out"}),
    "Subtract": math({"srcVar1": "$src1", "srcVar2": "$src2", "resultVar": "$out"}),
    "Multiply": math({"srcVar1": "$src1", "srcVar2": "$src2", "resultVar": "$out"}),
    "Divide": math({"srcVar1": "$src1", "srcVar2": "$src2", "resultVar": "$out"}),
    "Clamp": math({"srcVar1": "$src1", "min": "0.1", "max": "0.9", "resultVar": "$out"}),
    "Equals": math({"srcVar1": "$src1", "resultVar": "$out"}),
    "Frac": math({"srcVar1": "$src1", "resultVar": "$out"}),
    "Int": math({"srcVar1": "$src1", "resultVar": "$out"}),
    "Abs": math({"srcVar1": "$src1", "resultVar": "$out"}),
    "Exponential": math({"srcVar1": "$one", "scale": "2", "offset": "0.5", "minVal": "0",
                         "maxVal": "10", "resultVar": "$out1"}),
    "LessOrEqual": math({"srcVar1": "$one", "srcVar2": "$two", "lessEqualVar": "$src1",
                         "greaterVar": "$src2", "resultVar": "$out"}),
    "WrapMinMax": math({"srcVar1": "$src1", "minVal": "0", "maxVal": "1", "resultVar": "$out"}),
    "SelectFirstIfNonZero": math({"srcVar1": "$src1", "srcVar2": "$src2", "resultVar": "$out"}),
    "Sine": math({"sinePeriod": "2", "sineMin": "0.2", "sineMax": "0.9", "timeOffset": "0.5",
                  "resultVar": "$alpha"}),
    "LinearRamp": math({"rate": "0.5", "initialValue": "0.25", "resultVar": "$out1"}),
    "UniformNoise": math({"minVal": "0", "maxVal": "1", "resultVar": "$out1"}),
    "GaussianNoise": math({"mean": "0.5", "halfwidth": "0.2", "resultVar": "$out1"}),
    "Empty": recipe({}),
    "Dummy": recipe({}),
    "CurrentTime": recipe({"resultVar": "$out1"}, RESULT),
    # matrixproxy.cpp and scrolling.
    "TextureTransform": recipe({"centerVar": "$center", "scaleVar": "$scale", "rotateVar": "$rot",
                                "translateVar": "$trans", "resultVar": "$basetexturetransform"},
                               {"$center": "[0.5 0.5]", "$scale": "[2 3]", "$rot": "30",
                                "$trans": "[0.1 0.2]"}),
    "MatrixRotate": recipe({"axisVar": "$axis", "angle": "45", "resultVar": "$basetexturetransform"},
                           {"$axis": "[0 0 1]"}),
    "TextureScroll": recipe({"textureScrollVar": "$basetexturetransform",
                             "textureScrollRate": "0.5", "textureScrollAngle": "30",
                             "textureScale": "2"}),
    "ConveyorScroll": recipe({"textureScrollVar": "$basetexturetransform"}),
    # Animated textures.
    "AnimatedTexture": recipe({"animatedTextureVar": "$basetexture",
                               "animatedTextureFrameNumVar": "$frame",
                               "animatedTextureFrameRate": "10"}, ANIMATED),
    "AnimatedOffsetTexture": recipe({"animatedTextureVar": "$basetexture",
                                     "animatedTextureFrameNumVar": "$frame",
                                     "animatedTextureFrameRate": "10"}, ANIMATED,
                                    passes=NONE_ONLY),
    "AnimatedEntityTexture": recipe({"animatedTextureVar": "$basetexture",
                                     "animatedTextureFrameNumVar": "$frame",
                                     "animatedTextureFrameRate": "10"}, ANIMATED),
    "AnimateSpecificTexture": recipe({"animatedTextureVar": "$basetexture",
                                      "animatedTextureFrameNumVar": "$frame",
                                      "animatedTextureFrameRate": "10",
                                      "onlyAnimateOnTexture": "sprites/zerogxplode"}, ANIMATED),
    "ToggleTexture": recipe({"toggleTextureVar": "$basetexture",
                             "toggleTextureFrameNumVar": "$frame", "toggleShouldWrap": "1"},
                            ANIMATED),
    "MaterialModify": recipe({}, {"$basetexturetransform": "center .5 .5 scale 1 1 rotate 0 "
                                                           "translate 0 0"}),
    "MaterialModifyAnimated": recipe({"animatedTextureVar": "$basetexture",
                                      "animatedTextureFrameNumVar": "$frame",
                                      "animatedTextureFrameRate": "10"}, ANIMATED),
    "Pupil": recipe({"TextureVar": "$basetexture", "TextureFrameNumVar": "$frame",
                     "PupilCloseRate": "0.1", "PupilOpenRate": "0.03"},
                    dict(ANIMATED, **{"$lighting": "0.5"})),
    # Entity and player state (CResultProxy: resultVar, scale).
    "IsNPC": recipe({"scale": "0.5", "resultVar": "$out1"}, RESULT),
    "Health": recipe({"scale": "0.5", "resultVar": "$out1"}, RESULT),
    "PlayerProximity": recipe({"scale": "0.01", "resultVar": "$out1"}, RESULT),
    "PlayerTeamMatch": recipe({"scale": "1", "resultVar": "$out1"}, RESULT),
    "PlayerView": recipe({"scale": "1", "resultVar": "$out1"}, RESULT),
    "PlayerSpeed": recipe({"scale": "0.01", "resultVar": "$out1"}, RESULT),
    "PlayerPosition": recipe({"scale": "0.001", "resultVar": "$out"}, RESULT),
    "EntitySpeed": recipe({"scale": "0.01", "resultVar": "$out1"}, RESULT),
    "EntityRandom": recipe({"scale": "1", "resultVar": "$out1"}, RESULT),
    "PlayerLogo": recipe({}),
    "PlayerLogoOnModel": recipe({}),
    "Alpha": recipe({}),
    "lampbeam": recipe({}),
    "lamphalo": recipe({}),
    "EntityOrigin": recipe({}, {"$entityorigin": "[0 0 0]"}),
    "EntityOriginAlyx": recipe({}, {"$entityorigin": "[0 0 0]"}),
    "Ep1IntroVortRefract": recipe({}, {"$refractamount": "0.1"}, "Refract"),
    "VortEmissive": recipe({}, {"$emissiveblendstrength": "0.5"}, "VertexLitGeneric"),
    "HeliBlade": recipe({}),
    "FleshInterior": recipe({}, {"$fleshinteriorenabled": "1"}, "VertexLitGeneric",
                            passes=NONE_ONLY),
    "Camo": recipe({}, {"$camopatterntexture": "vgui/white"}),
    "BreakableSurface": recipe({}),
    "Shield": recipe({"textureScrollVar": "$basetexturetransform", "textureScrollRate": "0.5",
                      "textureScrollAngle": "30"}, {"$translucency": "1"}),
    "WaterLOD": recipe({}, {"$cheapwaterstartdistance": "500", "$cheapwaterenddistance": "1000"}),
    "WorldDims": recipe({}, {"$world_mins": "[0 0 0]", "$world_maxs": "[0 0 0]"}),
    # The data is a client shadow handle.
    "Shadow": recipe({}, passes="unbound"),
    "ShadowModel": recipe({}, {"$basetextureoffset": "[0 0]", "$basetexturescale": "[1 1]",
                               "$falloffoffset": "0", "$falloffdistance": "0",
                               "$falloffamount": "0"}, passes="unbound"),
    # The data is the particle manager.
    "ParticleSphereProxy": recipe({}, {"$light_position": "[0 0 0]", "$light_color": "[1 1 1]"},
                                  passes=NONE_ONLY),
    "engine_post": recipe({}),
    "BloomAdd": recipe({}),
    "MotionBlur": recipe({}, {"$motionblurinternal": "[0 0 0 0]"}),
    # Portal and Portal 2.
    "PortalStatic": recipe({"resultVar": "$out1"}, RESULT, passes=NONE_ONLY),
    "PortalStaticModel": recipe({"resultVar": "$out1"}, RESULT, passes=NONE_ONLY),
    "PortalOpenAmount": recipe({"resultVar": "$out1"}, RESULT, passes=NONE_ONLY),
    "PortalPickAlphaMask": recipe({"maskTextureVar": "$basetexture", "maskFrameVar": "$frame",
                                   "idleTexture": "vgui/white",
                                   "openingTexture": "sprites/zerogxplode"}, ANIMATED),
    "FizzlerVortex": recipe({}, {"$flow_vortex1": "0", "$flow_vortex2": "0",
                                 "$flow_vortex_pos1": "[0 0 0]", "$flow_vortex_pos2": "[0 0 0]",
                                 "$flow_color_intensity": "1", "$powerup": "0"}),
    "PhotoMaterial": recipe({}),
    "PlacementPhoto": recipe({}),
    "LightedMouth": recipe({"resultVar": "$out1"}, RESULT),
    "LightedFloorButton": recipe({"resultVar": "$out1"}, RESULT),
    "TractorBeam": recipe({"resultVar": "$out1"}, RESULT),
}


# Registered proxies whose fixture material runs without them in the legacy
# material system, per game, and why. The check requires exactly these.
CAMO = ("CCamoMaterialProxy::Init returns false (a Valve hack: the TGA loader may lack a file "
        "system)")
SYMBOL_CASE = ("KeyValues keeps a symbol's first spelling: a key spelled '%s' earlier in the "
               "boot names the Proxies block '%s', and the interface factory is case-sensitive")
WITHOUT_PROXY = {
    "portal": {"Camo": CAMO, "Dummy": SYMBOL_CASE % ("dummy", "dummy"),
               "Empty": SYMBOL_CASE % ("empty", "empty"),
               "Health": SYMBOL_CASE % ("health", "health")},
    "portal2": {"Camo": CAMO, "Health": SYMBOL_CASE % ("health", "health")},
}


def fixture_name(proxy):
    return "proxy_corpus/%s" % proxy.lower()


def vmt_text(proxy, item):
    lines = ['"%s"' % item["shader"], "{"]
    lines += ['\t"%s" "%s"' % (key, value) for key, value in item["vars"].items()]
    lines += ['\t"Proxies"', "\t{", '\t\t"%s"' % proxy, "\t\t{"]
    lines += ['\t\t\t"%s" "%s"' % (key, value) for key, value in item["params"].items()]
    lines += ["\t\t}", "\t}", "}", ""]
    return "\n".join(lines)


def write_materials(out):
    """A material root (portal_boot --material-root) with every recipe's material."""
    out = Path(out)
    folder = out / "materials" / "proxy_corpus"
    folder.mkdir(parents=True, exist_ok=True)
    names = []
    for proxy, item in sorted(RECIPES.items(), key=lambda kv: kv[0].lower()):
        (folder / ("%s.vmt" % proxy.lower())).write_text(vmt_text(proxy, item))
        names.append(fixture_name(proxy))
    return names


def command_materials(args):
    names = write_materials(args.out)
    print("%d fixture materials in %s" % (len(names), args.out))
    return 0


# ---------------------------------------------------------------------------
# Captures and the comparator


def read_capture(path):
    header, proxies, materials, missing = None, [], {}, []
    for line in Path(path).read_text().splitlines():
        if not line.strip():
            continue
        record = json.loads(line)
        if record["kind"] == "header":
            header = record
        elif record["kind"] == "proxy":
            proxies.append(record["name"])
        elif record["kind"] == "missing":
            missing.append(record["name"])
        elif record["kind"] == "material":
            materials[(record["name"], record["pass"])] = record
    return {"header": header, "proxies": sorted(proxies), "materials": materials,
            "missing": sorted(missing)}


def close(a, b):
    return abs(a - b) <= TOLERANCE * max(1.0, abs(a), abs(b))


def value_difference(name, a, b):
    """None when two captured variables agree, else a description."""
    if a.get("type") != b.get("type"):
        return "%s: type %s, expected %s" % (name, a.get("type"), b.get("type"))
    kind = a.get("type")
    if kind in ("float",):
        if not close(a["value"], b["value"]):
            return "%s: %r, expected %r" % (name, a["value"], b["value"])
    elif kind in ("vector", "matrix"):
        if len(a["value"]) != len(b["value"]) or \
                not all(close(x, y) for x, y in zip(a["value"], b["value"])):
            return "%s: %r, expected %r" % (name, a["value"], b["value"])
    elif kind != "undefined":
        if a.get("value") != b.get("value") or a.get("frame") != b.get("frame"):
            return "%s: %r (frame %r), expected %r (frame %r)" % (
                name, a.get("value"), a.get("frame"), b.get("value"), b.get("frame"))
    return None


def compare(actual, expected):
    """[differences] between two captures; empty when they agree."""
    differences = []
    ha, he = actual["header"] or {}, expected["header"] or {}
    for key in ("schema", "tickcount", "framecount", "player"):
        if ha.get(key) != he.get(key):
            differences.append("header %s: %r, expected %r" % (key, ha.get(key), he.get(key)))
    for key in ("curtime", "frametime"):
        if not close(ha.get(key, 0.0), he.get(key, 0.0)):
            differences.append("header %s: %r, expected %r" % (key, ha.get(key), he.get(key)))
    if actual["proxies"] != expected["proxies"]:
        differences.append("proxies: missing %s, extra %s" % (
            sorted(set(expected["proxies"]) - set(actual["proxies"])),
            sorted(set(actual["proxies"]) - set(expected["proxies"]))))
    if actual["missing"] != expected["missing"]:
        differences.append("missing materials %s, expected %s" % (actual["missing"],
                                                                  expected["missing"]))
    for key in sorted(set(actual["materials"]) | set(expected["materials"])):
        a, e = actual["materials"].get(key), expected["materials"].get(key)
        if a is None or e is None:
            differences.append("%s (%s): %s" % (key[0], key[1],
                                                "not captured" if a is None else "not expected"))
            continue
        if a["has_proxy"] != e["has_proxy"]:
            differences.append("%s (%s): has_proxy %s, expected %s" % (key[0], key[1],
                                                                      a["has_proxy"], e["has_proxy"]))
        vars_a = {v["name"].lower(): v for v in a["vars"]}
        vars_e = {v["name"].lower(): v for v in e["vars"]}
        for name in sorted(set(vars_a) | set(vars_e)):
            # A capture leaves undefined variables out.
            if name not in vars_a or name not in vars_e:
                differences.append("%s (%s): %s %s" % (key[0], key[1], name,
                                                       "undefined" if name not in vars_a
                                                       else "defined, expected undefined"))
                continue
            difference = value_difference(name, vars_a[name], vars_e[name])
            if difference:
                differences.append("%s (%s): %s" % (key[0], key[1], difference))
    return differences


def command_compare(args):
    differences = compare(read_capture(args.actual), read_capture(args.expected))
    for difference in differences[:20]:
        print(difference)
    print("%d difference(s)" % len(differences))
    return 1 if differences else 0


# ---------------------------------------------------------------------------
# Capture through portal_boot


def portal_runtime():
    return Path(os.environ.get("SOURCE_PORTAL_RUNTIME") or
                ROOT.parent / "source-engine" / "run" / "runtime")


def default_build(game):
    """The render-core trees' installs ($SOURCE_PORTAL_BUILD, $SOURCE_PORTAL2_BUILD)."""
    if game == "portal":
        return Path(os.environ.get("SOURCE_PORTAL_BUILD") or ROOT / "build-rc-client" / "install")
    return Path(os.environ.get("SOURCE_PORTAL2_BUILD") or ROOT / "build-rc-p2" / "install")


def default_runtime(game, build):
    if game == "portal":
        return portal_runtime()
    return Path(os.environ.get("SOURCE_PORTAL2_RUNTIME") or ROOT / "build-rc-p2" / "p2content")


def run_capture(game, out, build=None, runtime=None, timeout=900):
    """Boot the game once; returns the two captures' paths and the evidence."""
    out = Path(out)
    content = out / "content"
    write_materials(content)
    build = Path(build or default_build(game))
    runtime = Path(runtime or default_runtime(game, build))
    boot = out / "boot"
    # One line: separate cfg lines run at once, and the wait must hold the
    # second capture back.
    commands = ["mat_proxy_capture proxy_capture_0.jsonl %s; wait 40; "
                "mat_proxy_capture proxy_capture_1.jsonl %s" % (PATTERN, PATTERN)]
    argv = [sys.executable, str(ROOT / "tools" / "quality" / "portal_boot.py"),
            "--runtime", str(runtime), "--build", str(build), "--game", game,
            "--map", MAPS[game], "--headless", "--renderer", "native-vulkan",
            "--material-root", str(content), "--out", str(boot), "--capture-wait", "120",
            "--timeout", str(timeout), "--startup-command", "host_framerate 0.015",
            "--engine-arg=-deterministicrender", "--engine-arg=-nosound"]
    for command in commands:
        argv += ["--console-command", command]
    result = subprocess.run(argv, capture_output=True, text=True, cwd=ROOT, timeout=timeout + 120)
    (out / "portal_boot.log").write_text(result.stdout + result.stderr)
    game_dir = boot / "runtime" / game
    captures = [game_dir / "proxy_capture_0.jsonl", game_dir / "proxy_capture_1.jsonl"]
    return captures, result.returncode


def command_capture(args):
    checks = Checks()
    out = Path(args.out)
    captures, status = run_capture(args.game, out, args.build, args.runtime)
    present = [path for path in captures if path.is_file()]
    if not checks.check(len(present) == 2, "%s.captured" % args.game,
                        "portal_boot exited %d; captures %s (see %s)" % (
                            status, [str(p) for p in present], out / "portal_boot.log")):
        return checks.report()
    first, second = (read_capture(path) for path in captures)
    registered = set(first["proxies"])
    inventoried = {p["name"] for p in json.loads(INVENTORY.read_text())["games"][args.game]}
    checks.check(registered == inventoried, "%s.registered-equals-inventory" % args.game,
                 "registered only: %s; inventoried only: %s" % (
                     sorted(registered - inventoried), sorted(inventoried - registered)))
    unrecipe = sorted(name for name in registered if name not in RECIPES and name != "replace_proxy")
    checks.check(not unrecipe, "%s.every-proxy-has-a-recipe" % args.game,
                 "no recipe: %s" % unrecipe)
    fixtures = {fixture_name(proxy) for proxy in RECIPES if proxy in registered}
    loaded = {name for name, _ in first["materials"]}
    checks.check(fixtures <= loaded, "%s.every-fixture-material-loads" % args.game,
                 "not loaded: %s" % sorted(fixtures - loaded))
    without = sorted({name for (name, _), record in first["materials"].items()
                      if name in fixtures and not record["has_proxy"]})
    expected = sorted(fixture_name(proxy) for proxy in WITHOUT_PROXY[args.game]
                      if proxy in registered)
    checks.check(without == expected, "%s.every-fixture-material-has-its-proxy" % args.game,
                 "without their proxy: %s; expected exactly %s (WITHOUT_PROXY)" % (without,
                                                                                  expected))
    checks.check(first["header"]["curtime"] < second["header"]["curtime"],
                 "%s.the-second-capture-is-later" % args.game,
                 "curtime %r then %r" % (first["header"]["curtime"], second["header"]["curtime"]))
    recorded_dir = CAPTURES / args.game
    if args.record:
        recorded_dir.mkdir(parents=True, exist_ok=True)
        for index, path in enumerate(captures):
            shutil.copyfile(path, recorded_dir / ("capture_%d.jsonl" % index))
        print("recorded %s" % recorded_dir.relative_to(ROOT))
    for index, path in enumerate(captures):
        expected = recorded_dir / ("capture_%d.jsonl" % index)
        if not checks.check(expected.is_file(), "%s.capture-%d.recorded" % (args.game, index),
                            "no recorded capture %s" % expected):
            continue
        differences = compare(read_capture(path), read_capture(expected))
        checks.check(not differences, "%s.capture-%d.equals-the-recorded-legacy-values"
                     % (args.game, index), "; ".join(differences[:5]))
    changed = sum(1 for key, record in first["materials"].items()
                  if key in second["materials"] and
                  compare({"header": None, "proxies": [], "missing": [], "materials": {key: record}},
                          {"header": None, "proxies": [], "missing": [],
                           "materials": {key: second["materials"][key]}}))
    print("%s: %d proxies registered, %d fixture materials, %d material passes change "
          "between the captures" % (args.game, len(registered), len(fixtures), changed))
    return checks.report()


# ---------------------------------------------------------------------------
# Comparator self-test


def sample_capture():
    return {
        "header": {"schema": "source-proxy-capture/v1", "curtime": 3.6, "frametime": 0.015,
                   "tickcount": 240, "framecount": 400, "player": True},
        "proxies": ["Add", "Sine"],
        "missing": [],
        "materials": {
            ("proxy_corpus/sine", "none"): {"has_proxy": True, "vars": [
                {"name": "$alpha", "type": "float", "value": 0.55},
                {"name": "$basetexture", "type": "texture", "value": "vgui/white", "frame": 0},
                {"name": "$basetexturetransform", "type": "matrix", "value": [1.0] + [0.0] * 15},
                {"name": "$color", "type": "vector", "value": [1.0, 1.0, 1.0]},
                {"name": "$frame", "type": "int", "value": 3}]},
        },
    }


def seeded(change):
    capture = copy.deepcopy(sample_capture())
    change(capture)
    return capture


def var(capture, name):
    return next(v for v in capture["materials"][("proxy_corpus/sine", "none")]["vars"]
                if v["name"] == name)


def command_selftest(_args):
    checks = Checks()
    base = sample_capture()
    checks.check(not compare(base, copy.deepcopy(base)), "control.identical-captures-agree")
    within = seeded(lambda c: var(c, "$alpha").update(value=0.55 + 1e-7))
    checks.check(not compare(within, base), "control.a-change-within-tolerance-agrees")
    faults = {
        "float": lambda c: var(c, "$alpha").update(value=0.56),
        "vector": lambda c: var(c, "$color").update(value=[1.0, 0.9, 1.0]),
        "matrix": lambda c: var(c, "$basetexturetransform")["value"].__setitem__(12, 0.25),
        "int": lambda c: var(c, "$frame").update(value=4),
        "texture-frame": lambda c: var(c, "$basetexture").update(frame=1),
        "texture-name": lambda c: var(c, "$basetexture").update(value="vgui/black"),
        "type": lambda c: var(c, "$alpha").update(type="int", value=0),
        "missing-var": lambda c: c["materials"][("proxy_corpus/sine", "none")]["vars"].pop(0),
        "missing-material": lambda c: c["materials"].clear(),
        "missing-proxy": lambda c: c["proxies"].pop(),
        "no-proxy": lambda c: c["materials"][("proxy_corpus/sine", "none")].update(
            has_proxy=False),
        "time": lambda c: c["header"].update(curtime=3.615),
        "frame": lambda c: c["header"].update(framecount=401),
    }
    for name, change in faults.items():
        checks.check(bool(compare(seeded(change), base)), "fault.%s.detected" % name)
    return checks.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    inv = commands.add_parser("inventory", help="write the proxy inventory")
    inv.add_argument("--check", action="store_true")
    inv.set_defaults(run=command_inventory)
    mats = commands.add_parser("materials", help="write the fixture content root")
    mats.add_argument("--out", required=True)
    mats.set_defaults(run=command_materials)
    cap = commands.add_parser("capture", help="capture and check the legacy proxy values")
    cap.add_argument("--game", choices=sorted(CLIENT_PROJECTS), required=True)
    cap.add_argument("--out", required=True)
    cap.add_argument("--build", help="the game's install (default build-rc-client/install or "
                                     "build-rc-p2/install)")
    cap.add_argument("--runtime", help="the staged runtime (default ../source-engine/run/runtime "
                                       "or build-rc-p2/p2content)")
    cap.add_argument("--record", action="store_true", help="record the captures as the fixture")
    cap.set_defaults(run=command_capture)
    cmp_ = commands.add_parser("compare", help="compare two captures")
    cmp_.add_argument("actual")
    cmp_.add_argument("expected")
    cmp_.set_defaults(run=command_compare)
    commands.add_parser("selftest", help="the comparator against seeded differences") \
        .set_defaults(run=command_selftest)
    args = parser.parse_args(argv)
    try:
        return args.run(args)
    except CorpusError as error:
        print("proxy_corpus: %s" % error, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
