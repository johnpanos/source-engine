#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Test chamber signs: the sign lights up, and its light reaches the room at runtime.

Portal 2's chamber sign (the primary case, sp_a1_intro6's info_panel) is a
vgui_screen drawing the sp_progress_sign panel (CVGUI_Base_ProgressSignScreen).
A logic_auto sets it inactive at map spawn; the player crossing a trigger_once
fires a func_instance_io_proxy, whose relay sends SetActive, and the panel
flickers on. Portal's sign is a prop_dynamic of models/props_animsigns/
signage_numNN.mdl spawned at skin 1 (backing not self-illuminated); a trigger
fires a relay whose second relay flickers the skin (2, 3, 4, 3, 4, 5, 4, 5)
and leaves it at skin 6, whose backing (newsignage_back02) is $selfillum.
Retail lights the room around either sign only through baked lightmaps; the
graphics goal (Source 2 parity) is that the lit sign lights its surroundings
at runtime, which RFC 0011's emissive area lights (light set v2,
render.area-light.v1) do: the panel publishes one area light as an
IEmissiveAreaLightSource, the model through its $selfillum emitters.

This harness boots the installed product headless on native Vulkan
(portal_boot.py), frames one sign per scenario with a fixed noclip camera and
shoots it unlit, lit by what the map's trigger sends, switched off at runtime
(SetInactive, or its initial skin), lit again (SetActive, or its lit skin) and
the receivers' albedo (mat_fullbright 1). It judges:

  sign-on           the sign's pixels go from dark to lit (luma rise, lit fraction)
  sign-off          back to the unlit pixels when switched off; sign-relight: lit again
  emitter-at-sign   the published area lights (r_area_lights_report) include a
                    light inside the sign's world box while lit, none while unlit
  casts-light.near  a receiver in front of the sign brightens by a measured amount
  casts-light.falloff  the near receiver brightens more than the far one
  casts-light.hue   the brightening has the chromaticity of the published light
                    reflected by the receiver's albedo
  casts-light.returns  every receiver returns to its unlit pixels when switched off
  casts-light.relight  the near receiver brightens again when relit at runtime
  casts-light.behind   a surface behind the one-sided emitter does not change
                    (and holds any global exposure change)

Baked lighting cannot satisfy the casts-light checks: every one compares shots
of one boot at one camera, with the sign's state changed at runtime.

Each path is run with two live negative controls the judge must reject: every
sign input withheld (no-trigger) and the emission switched off (no-emission,
r_area_lights 0). The core-world path also shoots r_core_world 3 (neither
legacy nor the core draws the core's surfaces) to prove the render core drew
the receivers it was judged on.

    portal_sign_light.py suite --path legacy --out <dir>      # checks-v1
    portal_sign_light.py suite --path core-world --out <dir>
    portal_sign_light.py selftest                             # synthetic oracle
    portal_sign_light.py facts --runtime run/runtime          # what the maps say

Needs numpy and Pillow, a staged Portal runtime (run/runtime), a native
Vulkan Portal build (build/, SOURCE_SIGN_LIGHT_BUILD selects another tree), a
native Vulkan Portal 2 build (build-p2/, SOURCE_PORTAL2_BUILD) and a licensed
Portal 2 install (SOURCE_PORTAL2_STEAM_ROOT, default the Steam library), from
which a content runtime is staged under --out (or pass --p2-runtime).
Keep --out short: the engine refuses command lines over 512 characters.
"""

import argparse
import datetime
import json
import math
import os
from pathlib import Path
import re
import struct
import subprocess
import sys

import numpy
from PIL import Image

QUALITY = Path(__file__).resolve().parent
sys.path.insert(0, str(QUALITY))
import conformance_result  # noqa: E402
from legacy_bsp import LegacyBsp  # noqa: E402
from source_content import ContentResolver  # noqa: E402

ROOT = QUALITY.parents[1]
PORTAL_BOOT = QUALITY / "portal_boot.py"
WORKLOAD = ROOT / "quality/workloads/portal-sign-light-v1.json"
SCHEMA = "portal-sign-light-evidence/v1"
DEFAULT_STEAM_P2 = Path.home() / ".local/share/Steam/steamapps/common/Portal 2"
MARK = "SIGNLIGHT"
SHOTS = ("off", "on", "off2", "on2", "albedo")
STATES = SHOTS[:4]


# ---------------------------------------------------------------------------
# Facts: the sign, its map logic and its lit elements, read from the map and
# the content (recorded facts, no hypotheses).

def outputs(entity, key=None):
    """An entity's outputs (all On* keys, or one) as (key, target, input, parameter, delay).

    Portal 2's compiler separates the fields with ESC, Portal's with commas."""
    result = []
    for name, values in entity.items():
        if (key and name != key) or (not key and not name.startswith("On")):
            continue
        for value in values if isinstance(values, list) else [values]:
            parts = value.split("\x1b") if "\x1b" in value else value.split(",")
            if len(parts) >= 4:
                try:
                    delay = float(parts[3] or 0)
                except ValueError:
                    delay = 0.0
                result.append((name, parts[0], parts[1], parts[2], delay))
    return result


def rotation(angles):
    """Source's (pitch, yaw, roll) as a 3x3 matrix whose columns are the local axes."""
    pitch, yaw, roll = (math.radians(a) for a in angles)
    sp, cp = math.sin(pitch), math.cos(pitch)
    sy, cy = math.sin(yaw), math.cos(yaw)
    sr, cr = math.sin(roll), math.cos(roll)
    forward = (cp * cy, cp * sy, -sp)
    left = (sr * sp * cy - cr * sy, sr * sp * sy + cr * cy, sr * cp)
    up = (cr * sp * cy + sr * sy, cr * sp * sy - sr * cy, cr * cp)
    return numpy.array([forward, left, up]).T


def studio_skins(data):
    """(texture names, material directories, skin table) of a studio model header."""
    offset = 4 + 4 + 4 + 64 + 4 + 12 * 6 + 4
    fields = struct.unpack_from("<19i", data, offset)
    textures, texture_index = fields[12], fields[13]
    cd_count, cd_index = fields[14], fields[15]
    skin_refs, families, skin_index = fields[16], fields[17], fields[18]
    names = []
    for i in range(textures):
        base = texture_index + 64 * i
        name_offset = struct.unpack_from("<i", data, base)[0]
        start = base + name_offset
        names.append(data[start:data.index(b"\0", start)].decode("latin-1"))
    directories = []
    for i in range(cd_count):
        start = struct.unpack_from("<i", data, cd_index + 4 * i)[0]
        directories.append(data[start:data.index(b"\0", start)].decode("latin-1")
                           .replace("\\", "/"))
    table = [list(struct.unpack_from("<%dh" % skin_refs, data, skin_index + 2 * skin_refs * f))
             for f in range(families)]
    return names, directories, table


def studio_hull(data):
    values = struct.unpack_from("<6f", data, 4 + 4 + 4 + 64 + 4 + 24)
    return numpy.array(values[:3]), numpy.array(values[3:])


def vmt_selfillum(text):
    """True when an uncommented `$selfillum 1` is set."""
    for line in text.splitlines():
        line = line.split("//", 1)[0]
        match = re.search(r'"?\$selfillum"?\s+"?([0-9]+)"?', line, re.IGNORECASE)
        if match and int(match.group(1)) != 0:
            return True
    return False


RELAYS = ("logic_relay", "func_instance_io_proxy")


def activation(entities, setter):
    """Walk the outputs up from the entity that lights the sign to the map's trigger.

    Returns (chain, command): each step's (classname, targetname, output), and
    the ent_fire the trigger's own output sends, which is how the map lights it."""
    chain = []
    current = setter
    for _ in range(8):
        sources = [(entity, output) for entity in entities for output in outputs(entity)
                   if output[1] == current.get("targetname")
                   and entity.get("classname") != "logic_auto"]
        if not sources:
            return chain, None
        entity, output = sources[0]
        chain.append({"classname": entity.get("classname"),
                      "targetname": entity.get("targetname", ""), "output": output[0],
                      "fires": "%s %s" % (output[1], output[2])})
        if entity.get("classname") not in RELAYS:
            return chain, "ent_fire %s %s" % (output[1], output[2])
        current = entity
    return chain, None


def world_box(origin, axes, low, high):
    corners = [origin + axes @ numpy.array([x, y, z]) for x in (low[0], high[0])
               for y in (low[1], high[1]) for z in (low[2], high[2])]
    return [numpy.min(corners, axis=0).round(3).tolist(),
            numpy.max(corners, axis=0).round(3).tolist()]


def sign_facts(runtime, map_name, sign_name):
    """What the map and the content say about one sign."""
    content = ContentResolver(str(runtime))
    data, source = content.read("maps/%s.bsp" % map_name)
    if data is None:
        raise ValueError("maps/%s.bsp not found under %s" % (map_name, runtime))
    entities = LegacyBsp(data).entities()
    sign = next((e for e in entities if e.get("targetname") == sign_name), None)
    if sign is None:
        raise ValueError("%s has no entity %s" % (map_name, sign_name))
    origin = numpy.array([float(v) for v in sign["origin"].split()])
    angles = [float(v) for v in sign.get("angles", "0 0 0").split()]
    axes = rotation(angles)
    facts = {"map": map_name, "map_source": source, "sign": sign_name,
             "kind": sign.get("classname"), "origin": origin.tolist(), "angles": angles,
             "front": axes[:, 0].round(6).tolist()}
    if sign.get("classname") == "prop_dynamic":
        # Portal: a model whose skin the map's relay flickers to a lit skin.
        setter = next((e for e in entities if e.get("classname") == "logic_relay" and any(
            o[1] == sign_name and o[2].lower() == "skin" for o in outputs(e))), None)
        if setter is None:
            raise ValueError("%s: no logic_relay sets %s's skin" % (map_name, sign_name))
        sequence = sorted((o[4], int(o[3])) for o in outputs(setter)
                          if o[1] == sign_name and o[2].lower() == "skin")
        model, _ = content.read(sign["model"])
        if model is None:
            raise ValueError("model %s not found" % sign["model"])
        names, directories, table = studio_skins(model)
        hull_min, hull_max = studio_hull(model)

        def skin_materials(skin):
            result = []
            for ref in table[min(skin, len(table) - 1)]:
                text = None
                for directory in directories or [""]:
                    vmt, _ = content.read("materials/%s%s.vmt" % (directory, names[ref]))
                    if vmt is not None:
                        text = vmt.decode("latin-1")
                        break
                result.append({"material": names[ref], "found": text is not None,
                               "selfillum": bool(text and vmt_selfillum(text))})
            return result

        initial, lit = int(sign.get("skin", 0)), sequence[-1][1]
        facts.update(model=sign["model"], box=world_box(origin, axes, hull_min, hull_max),
                     initial_skin=initial, lit_skin=lit,
                     skin_sequence=[[delay, skin] for delay, skin in sequence],
                     starts_off=True,
                     switch_off="ent_fire %s skin %d" % (sign_name, initial),
                     switch_on="ent_fire %s skin %d" % (sign_name, lit),
                     lit_elements={"initial": skin_materials(initial), "lit": skin_materials(lit)})
    elif sign.get("classname") == "vgui_screen":
        # Portal 2: a VGUI screen (sp_progress_sign) the map activates.
        setter = next((e for e in entities if any(
            o[1] == sign_name and o[2].lower() == "setactive" for o in outputs(e))), None)
        if setter is None:
            raise ValueError("%s: nothing sends SetActive to %s" % (map_name, sign_name))
        spawn_off = any(e.get("classname") == "logic_auto" and any(
            o[0] == "OnMapSpawn" and o[1] == sign_name and o[2].lower() == "setinactive"
            for o in outputs(e)) for e in entities)
        width, height = float(sign.get("width", 0)), float(sign.get("height", 0))
        # A screen spans its local +Y (width) and +Z (height) from its origin, facing +X.
        facts.update(panel=sign.get("panelname"), size=[width, height],
                     box=world_box(origin, axes, (-1.0, 0.0, 0.0), (1.0, width, height)),
                     starts_off=spawn_off,
                     switch_off="ent_fire %s SetInactive" % sign_name,
                     switch_on="ent_fire %s SetActive" % sign_name,
                     lit_elements={"panel": sign.get("panelname")})
    else:
        raise ValueError("%s: %s is a %s, not a known sign" % (map_name, sign_name,
                                                               sign.get("classname")))
    chain, command = activation(entities, setter)
    facts.update(setter=setter.get("targetname"), chain=chain, activate=command)
    return facts


# ---------------------------------------------------------------------------
# The run: one boot, four shots, area-light reports between markers.

def console_line(scenario, facts, workload, path, control):
    frames = workload["frames"]
    withhold = bool(control and control.get("withhold_inputs"))
    camera = scenario["camera"]
    steps = list(workload["paths"][path]["console"])
    # Echo the path's console variables so the judge sees they exist and took effect.
    steps += [command.split()[0] for command in workload["paths"][path]["console"]]
    steps += ["noclip", "cmd setpos %g %g %g" % tuple(camera["setpos"]),
              "cmd setang %g %g %g" % tuple(camera["setang"])]

    def shot(label, wait):
        return ["wait %d" % wait, "echo %s %s" % (MARK, label), "r_area_lights_report 1",
                "wait %d" % frames["report"], "screenshot", "wait %d" % frames["after_shot"]]

    def send(command):
        return ["echo %s withheld" % MARK] if withhold else [command]

    steps += shot("off", frames["settle"])
    # The map's own logic: what its trigger sends when the player reaches it.
    steps += send(facts["activate"])
    steps += shot("on", frames["after_trigger"])
    steps += send(facts["switch_off"])
    steps += shot("off2", frames["after_toggle"])
    steps += send(facts["switch_on"])
    steps += shot("on2", frames["after_toggle"])
    steps += ["echo %s diagnostics" % MARK] + list(workload["paths"][path].get("diagnostics", []))
    # The receivers' albedo: lightmaps and model lighting at 1 (mat_fullbright 1).
    steps += ["mat_fullbright 1"] + shot("albedo", frames["after_toggle"]) + ["mat_fullbright 0"]
    coverage = workload["paths"][path].get("coverage_console")
    if coverage:
        steps += [coverage] + shot("coverage", frames["after_toggle"])
    steps += ["echo %s end" % MARK]
    return "; ".join(steps), len(SHOTS) + (1 if coverage else 0)


def capture_frames(workload, shots):
    frames = workload["frames"]
    per_shot = frames["report"] + frames["after_shot"]
    return (frames["settle"] + frames["after_trigger"] + frames["after_toggle"] * (shots - 2)
            + per_shot * shots + 120)


def boot(args, workload, scenario, facts, path, control_name, out):
    control = workload["controls"].get(control_name) if control_name else None
    line, shots = console_line(scenario, facts, workload, path, control)
    game = scenario["game"]
    runtime, build = game_runtime(args, game), (args.p2_build if game == "portal2" else args.build)
    command = [sys.executable, str(PORTAL_BOOT), "--game", game, "--runtime", str(runtime),
               "--build", str(build), "--out", str(out), "--headless",
               "--map", scenario["map"], "--renderer", "native-vulkan", "--require-vulkan",
               "--physics", "vphysics_box3d",
               "--width", str(workload["width"]), "--height", str(workload["height"]),
               "--capture-wait", str(capture_frames(workload, shots)),
               "--timeout", str(args.timeout), "--console-command", line]
    for startup in workload["startup_commands"] + list((control or {}).get("startup_commands", [])):
        command += ["--startup-command", startup]
    completed = subprocess.run(command, capture_output=True, text=True)
    screenshots = sorted((out / "runtime" / game / "screenshots").glob("*.tga"))
    console = out / "runtime" / game / "console.log"
    text = console.read_text(errors="replace") if console.is_file() else ""
    return {"path": path, "control": control_name, "returncode": completed.returncode,
            "boot_tail": (completed.stdout[-600:] + completed.stderr[-600:]).strip(),
            "console_line": line, "expected_shots": shots,
            "screenshots": [str(p) for p in screenshots], "reports": parse_reports(text),
            "markers": re.findall(r"^%s (\S+)" % MARK, text, re.MULTILINE),
            "diagnostics": diagnostics(text),
            "cvars": {name: path_cvar(text, name)
                      for name in (c.split()[0] for c in workload["paths"][path]["console"])}}


def game_runtime(args, game):
    """Portal's runtime, or a Portal 2 content runtime staged once from the Steam install."""
    if game == "portal":
        return args.runtime
    if args.p2_runtime is None:
        args.p2_runtime = args.out.resolve() / "p2content"
        if not args.p2_runtime.exists():
            import stage_portal2_runtime  # noqa: E402 (needs the Steam install only here)
            stage_portal2_runtime.stage_content(args.steam_root, args.p2_runtime)
    return args.p2_runtime


def diagnostics(text):
    """The console lines between the diagnostics marker and the next marker."""
    match = re.search(r"^%s diagnostics[ \t]*\n(.*?)^%s " % (MARK, MARK), text,
                      re.MULTILINE | re.DOTALL)
    return match.group(1).splitlines()[:200] if match else []


def path_cvar(text, name):
    """The value the console printed for `name`, or None (unknown command, never printed)."""
    match = re.search(r'^"%s" = "([^"]*)"' % re.escape(name), text, re.MULTILINE)
    return match.group(1) if match else None


AREA_LINE = re.compile(
    r"^\s+area \d+ key (-?\d+) v\d+ slot \d+ at (\S+) (\S+) (\S+) facing (\S+) (\S+) (\S+) "
    r"area (\S+) (one|two)-sided radiance (\S+) (\S+) (\S+) reach (\S+)")


def parse_reports(text):
    """{label: [light, ...] or None}: the first area-light report after each marker.

    The engine prints a requested report at its next publish; when nothing is
    published (no light at all) none is printed, which reads as no lights."""
    result = {}
    label = None
    in_block = False
    for line in text.splitlines():
        marker = re.match(r"^%s (\S+)" % MARK, line)
        if marker:
            label = marker.group(1)
            result.setdefault(label, None)
            in_block = False
            continue
        if label is None:
            continue
        if line.startswith("area lights generation"):
            in_block = result.get(label) is None
            if in_block:
                result[label] = []
            continue
        match = AREA_LINE.match(line)
        if match and in_block:
            values = match.groups()
            result[label].append({
                "key": int(values[0]), "center": [float(v) for v in values[1:4]],
                "normal": [float(v) for v in values[4:7]], "area": float(values[7]),
                "two_sided": values[8] == "two",
                "radiance": [float(v) for v in values[9:12]], "reach": float(values[12])})
        elif in_block and not line.startswith("  "):
            in_block = False
    return result


# ---------------------------------------------------------------------------
# The oracle: pure functions over shots, regions and reports.

LUMA = numpy.array([0.2126, 0.7152, 0.0722])


def to_linear(values):
    values = numpy.asarray(values, dtype=numpy.float64) / 255.0
    return numpy.where(values <= 0.04045, values / 12.92, ((values + 0.055) / 1.055) ** 2.4)


def crop(image, region):
    height, width = image.shape[:2]
    x0, y0, x1, y1 = region
    return image[int(round(y0 * height)):int(round(y1 * height)),
                 int(round(x0 * width)):int(round(x1 * width))]


def measure(image, region, lit_luma):
    pixels = crop(image, region).reshape(-1, 3).astype(numpy.float64)
    luma = pixels @ LUMA
    return {"mean": pixels.mean(axis=0).tolist(), "luma": float(luma.mean()),
            "lit_fraction": float((luma >= lit_luma).mean()),
            "linear": to_linear(pixels).mean(axis=0).tolist()}


def inside(point, box, margin):
    return all(box[0][k] - margin <= point[k] <= box[1][k] + margin for k in range(3))


def chroma(rgb):
    rgb = numpy.maximum(numpy.asarray(rgb, dtype=numpy.float64), 0.0)
    total = rgb.sum()
    return rgb / total if total > 0 else None


def core_gap(scenario, lines):
    """Why the core did not draw the receivers, from r_core_world_stats."""
    drawn = [line.split()[1].lower() for line in lines
             if re.match(r"^\d+ \S+/\S+ \(", line)]
    receivers = scenario.get("receiver_materials", [])
    missing = [name for name in receivers if name.lower() not in drawn]
    summary = next((line for line in lines if line.startswith("r_core_world_stats: materials")), "")
    gaps = [line.strip() for line in lines if re.match(r"^\d+ \w+: ", line)]
    return ("receiver materials outside the core's material model: %s; %s; model gaps: %s"
            % (missing or receivers, summary.replace("r_core_world_stats: ", "stats ")
               or "no r_core_world_stats output", "; ".join(gaps) or "none listed"))


def judge(images, reports, facts, scenario, thresholds, coverage=None, core_lines=()):
    """[(check, passed, detail)] for one run's shots {label: HxWx3 array}."""
    regions = scenario["regions"]
    t = thresholds
    m = {label: {name: measure(image, region, t["sign_lit_luma"])
                 for name, region in regions.items()}
         for label, image in images.items()}
    results = []

    def add(name, passed, detail):
        results.append((name, bool(passed), detail))

    off, on, off2, on2 = (m[label] for label in STATES)
    rise = on["sign"]["luma"] - off["sign"]["luma"]
    add("sign-on", rise >= t["sign_on_min_luma"]
        and on["sign"]["lit_fraction"] >= t["sign_lit_fraction_on"]
        and off["sign"]["lit_fraction"] <= t["sign_lit_fraction_off"],
        "sign luma %.1f -> %.1f (rise %.1f, need %.1f); lit fraction %.3f -> %.3f "
        "(need <= %.2f then >= %.2f)" % (off["sign"]["luma"], on["sign"]["luma"], rise,
                                         t["sign_on_min_luma"], off["sign"]["lit_fraction"],
                                         on["sign"]["lit_fraction"], t["sign_lit_fraction_off"],
                                         t["sign_lit_fraction_on"]))

    def region_difference(a, b, region):
        return float(numpy.abs(crop(a, region).astype(numpy.float64)
                               - crop(b, region).astype(numpy.float64)).mean())

    back = region_difference(images["off2"], images["off"], regions["sign"])
    add("sign-off", back <= t["sign_return_max"],
        "switched off, the sign differs from its unlit pixels by %.2f (limit %.2f)"
        % (back, t["sign_return_max"]))
    again = region_difference(images["on2"], images["on"], regions["sign"])
    add("sign-relight", again <= t["sign_return_max"]
        and on["sign"]["luma"] - off["sign"]["luma"] >= t["sign_on_min_luma"],
        "relit, the sign differs from its lit pixels by %.2f (limit %.2f)"
        % (again, t["sign_return_max"]))

    margin = t["emitter_box_margin"]
    at_sign = {label: [light for light in (reports.get(label) or [])
                       if inside(light["center"], facts["box"], margin)]
               for label in STATES}
    facing = [light for light in at_sign["on"]
              if numpy.dot(light["normal"], facts["front"]) > 0.9 or light["two_sided"]]
    add("emitter-at-sign", bool(facing) and not at_sign["off"] and not at_sign["off2"]
        and bool(at_sign["on2"]),
        "published lights inside the sign's box %s (+%g): off %d, on %d (%d facing %s), "
        "off2 %d, on2 %d; reports %s" % (facts["box"], margin, len(at_sign["off"]),
                                          len(at_sign["on"]), len(facing), facts["front"],
                                          len(at_sign["off2"]), len(at_sign["on2"]),
                                          {k: (None if v is None else len(v))
                                           for k, v in reports.items()}))

    def delta(a, b, name):
        return a[name]["luma"] - b[name]["luma"]

    near, far = delta(on, off, "near"), delta(on, off, "far")
    add("casts-light.near", near >= t["near_min_delta"],
        "near receiver luma rises %.2f when the sign lights (need >= %.2f)"
        % (near, t["near_min_delta"]))
    add("casts-light.falloff", near >= t["falloff_ratio"] * far and far >= t["far_min_delta"]
        and near >= t["near_min_delta"],
        "near %.2f against far %.2f (need near >= %.2f x far, far >= %.2f)"
        % (near, far, t["falloff_ratio"], t["far_min_delta"]))
    lights = facing or at_sign["on"]
    weights = sum(light["area"] for light in lights)
    emitted = (numpy.sum([numpy.array(light["radiance"]) * light["area"] for light in lights],
                         axis=0) / weights) if weights > 0 else None
    received = numpy.array(on["near"]["linear"]) - numpy.array(off["near"]["linear"])
    # What the receiver reflects of that light: its albedo (the mat_fullbright shot) times it.
    albedo = numpy.array(m["albedo"]["near"]["linear"])
    c_emitted = chroma(emitted * albedo) if emitted is not None else None
    c_received = chroma(received)
    distance = (float(numpy.linalg.norm(c_emitted - c_received))
                if c_emitted is not None and c_received is not None else None)
    add("casts-light.hue", distance is not None and distance <= t["hue_max_distance"]
        and near >= t["near_min_delta"],
        "near receiver's linear change %s has chromaticity %s; the sign's published light %s "
        "reflected by the receiver's albedo %s has %s: distance %s (limit %.3f)" % (
            numpy.round(received, 5).tolist(),
            None if c_received is None else numpy.round(c_received, 4).tolist(),
            None if emitted is None else numpy.round(emitted, 4).tolist(),
            numpy.round(albedo, 4).tolist(),
            None if c_emitted is None else numpy.round(c_emitted, 4).tolist(),
            None if distance is None else round(distance, 4), t["hue_max_distance"]))
    returns = {name: region_difference(images["off2"], images["off"], regions[name])
               for name in ("near", "far", "behind")}
    add("casts-light.returns", max(returns.values()) <= t["return_max"],
        "switched off, receivers differ from their unlit pixels by %s (limit %.2f)"
        % ({k: round(v, 3) for k, v in returns.items()}, t["return_max"]))
    relight = delta(on2, off2, "near")
    add("casts-light.relight", relight >= t["near_min_delta"],
        "relit at runtime, the near receiver rises %.2f (need >= %.2f)"
        % (relight, t["near_min_delta"]))
    behind = max(abs(delta(on, off, "behind")), abs(delta(on2, off2, "behind")))
    add("casts-light.behind", behind <= t["behind_max"],
        "the surface behind the emitter changes by %.2f (limit %.2f)" % (behind, t["behind_max"]))
    if coverage is not None:
        drawn = {name: region_difference(coverage, images["on2"], regions[name])
                 for name in ("near", "far")}
        passed = min(drawn.values()) >= t["coverage_min_delta"]
        add("core-draws-receivers", passed,
            "with r_core_world 3 the receivers change by %s (need >= %.1f each)%s" % (
                {k: round(v, 2) for k, v in drawn.items()}, t["coverage_min_delta"],
                "" if passed else ": the render core did not draw them, so this path's "
                "casts-light verdicts are the legacy world's (RFC 0016 K5 step 4, row R89): "
                + core_gap(scenario, core_lines)))
    return results, m


def load(path):
    return numpy.asarray(Image.open(path).convert("RGB"), dtype=numpy.int16)


# ---------------------------------------------------------------------------
# Commands.

def add_content_arguments(parser):
    parser.add_argument("--runtime", type=Path, default=ROOT / "run/runtime",
                        help="staged Portal runtime (immutable assets are shared)")
    parser.add_argument("--build", type=Path,
                        default=Path(os.environ.get("SOURCE_SIGN_LIGHT_BUILD", ROOT / "build")),
                        help="Portal client Waf tree (native Vulkan, SDL3)")
    parser.add_argument("--p2-build", type=Path,
                        default=Path(os.environ.get("SOURCE_PORTAL2_BUILD", ROOT / "build-p2")),
                        help="Portal 2 client Waf tree (native Vulkan, SDL3)")
    parser.add_argument("--steam-root", type=Path,
                        default=Path(os.environ.get("SOURCE_PORTAL2_STEAM_ROOT", DEFAULT_STEAM_P2)),
                        help="licensed Portal 2 install the content runtime is staged from")
    parser.add_argument("--p2-runtime", type=Path,
                        help="an already staged Portal 2 content runtime (else staged under --out)")


def scenario_runtime(args, scenario):
    return game_runtime(args, scenario["game"])


def cmd_facts(args):
    workload = json.loads(WORKLOAD.read_text())
    for scenario in workload["scenarios"]:
        print(json.dumps(sign_facts(scenario_runtime(args, scenario), scenario["map"],
                                    scenario["sign"]), indent=2))
    return 0


def static_checks(checks, facts, name):
    """What the content and the map logic must say before any pixel is judged."""
    checks.check(facts["starts_off"] and bool(facts["activate"]), name + ".content.map-logic",
                 "starts off %s; the map lights it through %s (%s)" % (
                     facts["starts_off"], facts["chain"], facts["activate"]))
    if facts["kind"] == "prop_dynamic":
        initial, lit = facts["lit_elements"]["initial"], facts["lit_elements"]["lit"]
        checks.check(all(m["found"] for m in initial + lit), name + ".content.materials",
                     "materials not found: %s" % [m["material"] for m in initial + lit
                                                  if not m["found"]])
        checks.check(any(m["selfillum"] for m in lit)
                     and not any(m["selfillum"] for m in initial),
                     name + ".content.lit-skin-selfillum",
                     "lit skin %d materials %s; initial skin %d materials %s" % (
                         facts["lit_skin"], lit, facts["initial_skin"], initial))
    else:
        checks.check(facts["panel"] == "sp_progress_sign" and min(facts["size"]) > 0,
                     name + ".content.panel",
                     "panel %s size %s" % (facts["panel"], facts["size"]))


def run_and_judge(args, workload, scenario, facts, path, control, out):
    result = boot(args, workload, scenario, facts, path, control, out)
    shots = result["screenshots"]
    if result["returncode"] != 0 or len(shots) < result["expected_shots"]:
        return result, None, "boot exit %s with %d of %d screenshots: %s" % (
            result["returncode"], len(shots), result["expected_shots"], result["boot_tail"][-300:])
    wanted = {c.split()[0]: c.split()[1] for c in workload["paths"][path]["console"]}
    if result["cvars"] != wanted:
        return result, None, "the %s path did not take effect: console variables %s, wanted %s" % (
            path, result["cvars"], wanted)
    images = {label: load(shots[i]) for i, label in enumerate(SHOTS)}
    coverage = load(shots[len(SHOTS)]) if result["expected_shots"] > len(SHOTS) else None
    thresholds = dict(workload["thresholds"], **scenario.get("thresholds", {}))
    verdicts, measured = judge(images, result["reports"], facts, scenario,
                               thresholds, coverage, result["diagnostics"])
    result["measurements"] = measured
    result["verdicts"] = [{"check": c, "pass": p, "detail": d} for c, p, d in verdicts]
    preview = out / "preview"
    preview.mkdir(exist_ok=True)
    for label, image in list(images.items()) + ([("coverage", coverage)] if coverage is not None
                                                 else []):
        Image.fromarray(image.astype(numpy.uint8)).save(preview / (label + ".png"))
    return result, verdicts, None


def cmd_suite(args):
    workload = json.loads(WORKLOAD.read_text())
    out = args.out.resolve()
    out.mkdir(parents=True, exist_ok=True)
    checks = conformance_result.Checks()
    evidence = {"schema": SCHEMA, "path": args.path, "workload": str(WORKLOAD.relative_to(ROOT)),
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "build": str(args.build.resolve()), "p2_build": str(args.p2_build.resolve()),
                "runtime": str(args.runtime.resolve()), "scenarios": []}
    for index, scenario in enumerate(workload["scenarios"]):
        if args.scenario and scenario["name"] not in args.scenario:
            continue
        name = "%s.%s" % (scenario["name"], args.path)
        facts = sign_facts(scenario_runtime(args, scenario), scenario["map"], scenario["sign"])
        record = {"scenario": scenario["name"], "facts": facts, "runs": []}
        evidence["scenarios"].append(record)
        static_checks(checks, facts, name)
        run_dir = out / ("s%d" % index)
        result, verdicts, problem = run_and_judge(args, workload, scenario, facts, args.path,
                                                  None, run_dir)
        record["runs"].append(result)
        checks.check(problem is None, name + ".boot", problem or "")
        for check, passed, detail in verdicts or []:
            checks.check(passed, "%s.%s" % (name, check), detail)
        if args.no_controls or not scenario.get("controls"):
            continue
        for control_name, control in workload["controls"].items():
            result, verdicts, problem = run_and_judge(args, workload, scenario, facts, args.path,
                                                      control_name,
                                                      out / ("s%d-%s" % (index, control_name)))
            record["runs"].append(result)
            label = "%s.control.%s" % (name, control_name)
            checks.check(problem is None, label + ".boot", problem or "")
            failed = {check for check, passed, _ in verdicts or [] if not passed}
            for must in control["must_fail"]:
                checks.check(must in failed, "%s.rejects.%s" % (label, must),
                             "the oracle accepted the control on %s: %s" % (
                                 must, next((d for c, _, d in verdicts or [] if c == must), "")))
    (out / "evidence.json").write_text(json.dumps(evidence, indent=2, default=str) + "\n")
    print("evidence: %s" % (out / "evidence.json"))
    return checks.report()


# ---------------------------------------------------------------------------
# Self-test: synthetic shots and reports; every negative control is rejected.

def synthetic(scenario, sign_on=True, near=12.0, far=6.0, behind=0.0, tint=(0.95, 1.0, 1.05),
              returns=True, relight=True, width=128, height=96, base=(90, 100, 110),
              albedo=(0.5, 0.5, 0.5)):
    """Shots {label: image}: the sign lit to white, receivers brightened by light of
    chromaticity `tint` reflected by `albedo` (linear), and the albedo shot."""
    regions = scenario["regions"]
    images = {}
    for label in STATES:
        image = numpy.zeros((height, width, 3), dtype=numpy.float64)
        image[:, :] = base
        lit = sign_on and (label == "on" or (label == "on2" and relight)
                           or (label == "off2" and not returns))
        if lit:
            crop(image, regions["sign"])[:] = (235, 238, 240)
        else:
            crop(image, regions["sign"])[:] = (20, 25, 30)
        glow = label == "on" or (label == "on2" and relight) or (label == "off2" and not returns)
        if glow:
            for name, amount in (("near", near), ("far", far), ("behind", behind)):
                if amount:
                    pixels = crop(image, regions[name])
                    pixels[:] = brightened(pixels[0, 0], amount,
                                           numpy.array(tint) * numpy.array(albedo))
        images[label] = numpy.clip(numpy.round(image), 0, 255).astype(numpy.int16)
    images["albedo"] = numpy.zeros((height, width, 3), dtype=numpy.int16)
    images["albedo"][:, :] = numpy.round(to_srgb(albedo)).astype(numpy.int16)
    return images


def brightened(srgb, amount, tint):
    """`srgb` plus light of chromaticity `tint` (linear) that raises its luma by `amount`."""
    start = to_linear(srgb)
    direction = numpy.array(tint, dtype=numpy.float64) * math.copysign(1.0, amount)
    target = float(numpy.dot(srgb, LUMA)) + amount
    low, high = 0.0, 1.0
    for _ in range(50):
        middle = (low + high) / 2
        if (float(numpy.dot(to_srgb(start + middle * direction), LUMA)) - target) * \
                math.copysign(1.0, amount) < 0:
            low = middle
        else:
            high = middle
    return to_srgb(start + low * direction)


def to_srgb(linear):
    linear = numpy.clip(numpy.asarray(linear, dtype=numpy.float64), 0.0, 1.0)
    return 255.0 * numpy.where(linear <= 0.0031308, linear * 12.92,
                               1.055 * linear ** (1 / 2.4) - 0.055)


def synthetic_reports(facts, lit=True, tint=(0.95, 1.0, 1.05), offset=(0, 0, 0), stays=False):
    center = (numpy.array(facts["box"][0]) + numpy.array(facts["box"][1])) / 2 + offset
    light = {"key": 1, "center": center.tolist(), "normal": facts["front"], "area": 5000.0,
             "two_sided": False, "radiance": (0.36 * numpy.array(tint)).tolist(), "reach": 400.0}
    return {"off": [dict(light)] if stays else [], "on": [light] if lit else [],
            "off2": [dict(light)] if stays else [], "on2": [light] if lit else []}


def cmd_selftest(args):
    workload = json.loads(WORKLOAD.read_text())
    scenario = workload["scenarios"][0]
    thresholds = workload["thresholds"]
    facts = {"box": [[400.0, -0.3, 24.0], [496.0, 4.3, 216.0]], "front": [0.0, 1.0, 0.0]}
    checks = conformance_result.Checks()

    def verdicts(images, reports, coverage=None):
        return {c: (p, d) for c, p, d in judge(images, reports, facts, scenario, thresholds,
                                               coverage)[0]}

    good = verdicts(synthetic(scenario), synthetic_reports(facts))
    for check, (passed, detail) in sorted(good.items()):
        checks.check(passed, "selftest.good." + check, detail)
    # A blue receiver: the light it reflects is bluer than the sign's, as its albedo says.
    blue = verdicts(synthetic(scenario, albedo=(0.25, 0.4, 0.7)), synthetic_reports(facts))
    for check, (passed, detail) in sorted(blue.items()):
        checks.check(passed, "selftest.blue-albedo." + check, detail)
    orange = (1.6, 0.9, 0.3)
    cases = {
        # (images, reports, the checks that must fail)
        "no-trigger": (synthetic(scenario, sign_on=False, near=0, far=0),
                       synthetic_reports(facts, lit=False), ("sign-on", "casts-light.near")),
        "no-emission": (synthetic(scenario, near=0, far=0), synthetic_reports(facts, lit=False),
                        ("casts-light.near", "emitter-at-sign")),
        "baked-only": (synthetic(scenario, near=0, far=0, base=(140, 150, 160)),
                       synthetic_reports(facts), ("casts-light.near", "casts-light.hue")),
        "no-return": (synthetic(scenario, returns=False), synthetic_reports(facts),
                      ("casts-light.returns", "sign-off")),
        "no-relight": (synthetic(scenario, relight=False), synthetic_reports(facts),
                       ("casts-light.relight", "sign-relight")),
        "wrong-hue": (synthetic(scenario, tint=orange), synthetic_reports(facts),
                      ("casts-light.hue",)),
        # A blue receiver lit by the sign's light, judged as if its albedo were neutral.
        "albedo-ignored": (dict(synthetic(scenario, albedo=(0.25, 0.4, 0.7)),
                                albedo=synthetic(scenario)["albedo"]),
                           synthetic_reports(facts), ("casts-light.hue",)),
        "no-falloff": (synthetic(scenario, near=10, far=10), synthetic_reports(facts),
                       ("casts-light.falloff",)),
        "exposure": (synthetic(scenario, behind=8), synthetic_reports(facts),
                     ("casts-light.behind",)),
        "detached-light": (synthetic(scenario),
                           synthetic_reports(facts, offset=(0, 300, 0)), ("emitter-at-sign",)),
        "light-stays-on": (synthetic(scenario), synthetic_reports(facts, stays=True),
                           ("emitter-at-sign",)),
        "wrong-facing": (synthetic(scenario),
                         {k: [dict(v, normal=[0.0, -1.0, 0.0]) for v in (lights or [])]
                          for k, lights in synthetic_reports(facts).items()},
                         ("emitter-at-sign",)),
    }
    for case, (images, reports, must) in cases.items():
        result = verdicts(images, reports)
        for check in must:
            checks.check(not result[check][0], "selftest.%s.rejects.%s" % (case, check),
                         "accepted: " + result[check][1])
    # The coverage check: a core that did not draw the receivers is rejected.
    images = synthetic(scenario)
    drawn = images["on2"].copy()
    for name in ("near", "far"):
        crop(drawn, scenario["regions"][name])[:] = 0
    checks.check(verdicts(images, synthetic_reports(facts), drawn)["core-draws-receivers"][0],
                 "selftest.coverage.accepts-drawn")
    checks.check(not verdicts(images, synthetic_reports(facts),
                              images["on2"].copy())["core-draws-receivers"][0],
                 "selftest.coverage.rejects-undrawn")
    # The report parser: markers, blocks, a missing report.
    text = "\n".join([
        "%s off" % MARK,
        "area lights generation 5: 1 lit, 0 dropped; 3 surface(s) hold area light, 0 dirty",
        "  area 0 key 7 v1 slot 2 at 900.0 1.0 2.0 facing 1.00 0.00 0.00 area 10.0 one-sided "
        "radiance 0.100 0.200 0.300 reach 50",
        "%s on" % MARK,
        "area lights generation 6: 2 lit, 0 dropped; 3 surface(s) hold area light, 0 dirty",
        "  area 0 key 8 v2 slot 1 at 448.0 3.1 56.5 facing -0.00 1.00 -0.00 area 5214.2 "
        "one-sided radiance 0.341 0.358 0.375 reach 399",
        "  area 1 key 7 v1 slot 2 at 900.0 1.0 2.0 facing 1.00 0.00 0.00 area 10.0 two-sided "
        "radiance 0.100 0.200 0.300 reach 50",
        "unrelated line",
        "area lights generation 7: 0 lit",
        "%s off2" % MARK])
    parsed = parse_reports(text)
    checks.equal([len(parsed["off"]), len(parsed["on"])], [1, 2], "selftest.parser.blocks")
    checks.equal(parsed["off2"], None, "selftest.parser.missing-report")
    checks.equal(parsed["on"][0]["center"], [448.0, 3.1, 56.5], "selftest.parser.center")
    checks.equal(parsed["on"][1]["two_sided"], True, "selftest.parser.sidedness")
    # The core gap message names the receivers the core's model left out.
    lines = ["r_core_world_stats: materials 133 claimed 1 surfaces 9 claimed 2 views queued 4 "
             "drawn 4 failed 0 surfaces drawn 8 last failure ''", "r_core_world_stats: gaps:",
             "17 lightmapped: the family does not draw $detail", "r_core_world_stats: drawn by "
             "the core:", "12 signage/signage_border (22 unread keys at neutral)"]
    receivers = ["concrete/floor", "signage/signage_border"]
    message = core_gap(dict(scenario, receiver_materials=receivers), lines)
    checks.check("['concrete/floor']" in message and "$detail" in message
                 and "drawn 4" in message, "selftest.core-gap.message", message)
    # The facts helpers.
    checks.check(vmt_selfillum('"VertexLitGeneric"\n{\n "$selfillum" "1"\n}'),
                 "selftest.vmt.selfillum")
    checks.check(not vmt_selfillum('"VertexLitGeneric"\n{\n//\t"$selfillum" "1"\n}'),
                 "selftest.vmt.commented")
    front = rotation([0, 90, 0])[:, 0]
    checks.check(numpy.allclose(front, [0, 1, 0], atol=1e-9), "selftest.rotation.yaw90",
                 str(front))
    return checks.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    suite = commands.add_parser("suite", help="boot, shoot and judge one render path")
    suite.add_argument("--path", choices=("legacy", "core-world"), required=True)
    add_content_arguments(suite)
    suite.add_argument("--scenario", action="append",
                       help="run only these scenarios (repeatable; a quick look, not a gate)")
    suite.add_argument("--out", type=Path, required=True)
    suite.add_argument("--timeout", type=int, default=420)
    suite.add_argument("--no-controls", action="store_true",
                       help="skip the live negative controls (a quick look, not a gate)")
    facts = commands.add_parser("facts", help="print what the maps and content say")
    add_content_arguments(facts)
    facts.add_argument("--out", type=Path, default=Path("/tmp/portal-sign-light-facts"),
                       help="where a Portal 2 content runtime is staged when needed")
    commands.add_parser("selftest", help="the oracle against synthetic shots")
    args = parser.parse_args(argv)
    return {"suite": cmd_suite, "facts": cmd_facts, "selftest": cmd_selftest}[args.command](args)


if __name__ == "__main__":
    sys.exit(main())
