#!/usr/bin/env python3
"""Compile a VMF into a playable map: the editor's author -> compile -> play loop.

One tool owns "turn this VMF into a published, bootable map" for every caller:
the Hammer loop suite, scripted map generators (tools/quality/gyro_lab_map.py)
and, later, the editor's Run Map action and an MCP client.

  vmf_map_build.py build --vmf room.vmf --out quality-results/room [--publish] [--boot]

Steps, each recorded in <out>/build.json:
  1. stage a compile game: the pinned vbsp fixture's gameinfo and lights, plus
     every material the VMF references (with base textures) and the surface
     properties, extracted from the staged runtime;
  2. vbsp, then vvis and vrad (fast by default for a quick edit loop;
     --quality full for release lighting). A leak is a failure: vbsp's log
     reports it and writes a pointfile;
  3. package <out>/content/maps/<name>.bsp; --publish copies it to the playable
     map store (./play <name>); --install-game-dir copies it into a staged game
     directory's maps/ instead (a product the store is not mounted in, such as
     the F-Stop runtime's run/runtime-fstop/fstop);
  4. --boot runs the installed Portal product headless on the map
     (tools/quality/portal_boot.py): the map must be active, a player must
     spawn, and the capture must show scene detail.

Exit status is 0 only when every requested step passed.
"""

import argparse
import datetime
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import playable_maps  # noqa: E402
from source_content import ContentResolver  # noqa: E402

ROOT = HERE.parents[1]
TOOLCHAIN = ROOT / "build/toolchains/pbrt-map-toolchain.json"
FIXTURE_GAME = ROOT / "quality/fixtures/vbsp-host/game"
SURFACE_MANIFEST = "scripts/surfaceproperties_manifest.txt"
LEAK_TEXT = re.compile(r"\*+\s*leaked\s*\*+|^\s*leaked\b", re.IGNORECASE | re.MULTILINE)
SCHEMA = "vmf-map-build/v1"


KV_TOKEN = re.compile(r'"([^"]*)"|([{}])|([^\s"{}]+)')


def referenced_materials(vmf_text):
    """Every material a VMF's brush sides name, lower-cased and de-duplicated.
    Only "side" blocks count: entities also have "material" keys (a physbox's
    breakable material type, e.g. "2")."""
    materials, blocks, pending = set(), [], []
    for quoted, brace, bare in KV_TOKEN.findall(vmf_text):
        if brace == "{":
            blocks.append(pending[-1] if pending else "")
            pending = []
        elif brace == "}":
            if blocks:
                blocks.pop()
            pending = []
        else:
            # A token is a key, a value, or the name of the block that follows.
            pending.append(quoted if quoted or not bare else bare)
            if len(pending) == 2:
                if blocks and blocks[-1] == "side" and pending[0].lower() == "material":
                    materials.add(pending[1].lower())
                pending = []
    return sorted(materials)


STATIC_PROP = re.compile(r'"classname"\s+"prop_static"')
MODEL_KEY = re.compile(r'"model"\s+"([^"]+\.mdl)"', re.IGNORECASE)
# The files vbsp (bounds, collision) and vrad (shadows, per-vertex light) read.
MODEL_SUFFIXES = (".mdl", ".phy", ".vvd", ".dx90.vtx", ".dx80.vtx", ".vtx")


def referenced_models(vmf_text):
    """The models of a VMF's prop_static entities, lower-cased: vbsp builds the
    static prop lump from their files, so the compile game needs them."""
    models = set()
    for block in re.split(r"\n\s*entity\s*\n", vmf_text):
        if STATIC_PROP.search(block):
            models.update(m.lower().replace("\\", "/") for m in MODEL_KEY.findall(block))
    return sorted(models)


def stage_compile_game(game, runtime, materials, models=()):
    """A game root for the compilers: the vbsp fixture's gameinfo and lights, and
    the given materials (with base textures, for vrad's reflectivity), static
    prop models and the surface properties, extracted from the staged
    runtime's packs. Returns the materials and models the runtime does not
    have."""
    shutil.rmtree(game, ignore_errors=True)
    game.mkdir(parents=True)
    for name in ("gameinfo.txt", "lights.rad"):
        shutil.copy2(FIXTURE_GAME / name, game / name)
    resolver = ContentResolver(runtime)

    def extract(relative):
        data, _ = resolver.read(relative)
        if data is None:
            return None
        target = game / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)
        return data

    manifest = extract(SURFACE_MANIFEST)
    if manifest is None:
        raise FileNotFoundError("missing from the runtime: " + SURFACE_MANIFEST)
    for name in re.findall(r'"file"\s+"([^"]+)"', manifest.decode("utf-8", "replace")):
        if extract(name) is None:
            raise FileNotFoundError("missing from the runtime: " + name)
    missing = []
    for material in materials:
        vmt = extract("materials/%s.vmt" % material.lower())
        if vmt is None:
            missing.append(material)
            continue
        texture = re.search(r'"?\$basetexture"?\s+"?([^"\s]+)', vmt.decode("utf-8", "replace"),
                            re.IGNORECASE)
        if texture:
            extract("materials/%s.vtf" % texture.group(1).lower().replace("\\", "/"))
    for model in models:
        stem = model[:-len(".mdl")]
        if extract(model) is None:
            missing.append(model)
            continue
        for suffix in MODEL_SUFFIXES[1:]:
            extract(stem + suffix)
        # vrad reads the dx80 strip file; Portal 2 ships only dx90, the same format.
        dx80, dx90 = game / (stem + ".dx80.vtx"), game / (stem + ".dx90.vtx")
        if not dx80.exists() and dx90.exists():
            shutil.copy2(dx90, dx80)
    return missing


def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def run(command, log):
    """Run one compile step, appending its output to the log; returns the exit code."""
    with open(log, "a") as stream:
        stream.write("+ " + " ".join(str(c) for c in command) + "\n")
        stream.flush()
        return subprocess.run([str(c) for c in command], stdout=stream,
                              stderr=subprocess.STDOUT).returncode


def compile_steps(tools, game, vmf, bsp, quality):
    fast = quality == "fast"
    return [
        ("vbsp", [tools / "vbsp", "-game", game, vmf]),
        ("vvis", [tools / "vvis", *(["-fast"] if fast else []), "-threads", "4", "-game", game, bsp]),
        # LDR and HDR lighting, as two passes (the pinned vrad does not run
        # -both in one process). Portal renders HDR; without the HDR lumps a
        # map falls back to flat lighting that auto-exposure evens out.
        *(("vrad-" + mode, [tools / "vrad", "-" + mode,
                            *(["-fast", "-bounce", "1"] if fast else ["-bounce", "2"]),
                            "-threads", "4", "-game", game, bsp])
          for mode in ("ldr", "hdr")),
    ]


def lowercase_scratch_base():
    """A writable temporary directory whose absolute path has no uppercase
    letter. The pinned compile tools open files through the Source filesystem,
    which folds path case on Linux, so any uppercase directory on the way to
    the map (e.g. a timestamped results folder) makes them fail."""
    for base in (tempfile.gettempdir(), "/tmp", "/var/tmp"):
        if base == base.lower() and os.access(base, os.W_OK):
            return base
    raise RuntimeError("no writable lower-case temporary directory for the compile tools")


def build(vmf, out, tools, runtime, quality="fast", name=None):
    """Compile `vmf` under `out`; returns the build record (also written to
    <out>/build.json). status is "pass", "leak", "missing-materials" or
    "fail". The compile runs in a lower-case temporary directory, and its
    files (the staged game excepted) are copied to <out>/compile afterwards."""
    vmf = Path(vmf).resolve()
    # Map names resolve through the same case-folding filesystem.
    name = (name or vmf.stem).lower()
    data = vmf.read_bytes()
    out.mkdir(parents=True, exist_ok=True)
    kept = out / "compile"
    shutil.rmtree(kept, ignore_errors=True)
    work = Path(tempfile.mkdtemp(prefix="vmf-build-", dir=lowercase_scratch_base()))
    try:
        record = compile_in(work, data, name, out, tools, runtime, quality)
    finally:
        shutil.copytree(work, kept, ignore=shutil.ignore_patterns("game"))
        shutil.rmtree(work, ignore_errors=True)
    return finish(out, record)


def compile_in(work, data, name, out, tools, runtime, quality):
    source = work / (name + ".vmf")
    source.write_bytes(data)
    text = source.read_text(errors="replace")
    record = {"schema": SCHEMA, "map": name, "vmf_sha256": sha256(source), "quality": quality,
              "status": "fail", "failed_gates": [], "steps": [],
              "built": datetime.datetime.now(datetime.timezone.utc).isoformat(timespec="seconds")}
    log = out / "compile.log"
    log.write_text("")

    materials = referenced_materials(text)
    models = referenced_models(text)
    missing = stage_compile_game(work / "game", runtime, materials, models)
    record["materials"] = {"referenced": len(materials), "missing": missing}
    record["static_prop_models"] = len(models)
    if missing:
        record.update(status="missing-materials", failed_gates=["materials"])
        return record

    bsp = work / (name + ".bsp")
    for step, command in compile_steps(tools, work / "game", source, bsp, quality):
        code = run(command, log)
        record["steps"].append({"step": step, "returncode": code})
        if step == "vbsp":
            leaked = bool(LEAK_TEXT.search(log.read_text(errors="replace"))) or \
                (work / (name + ".lin")).exists() or (work / (name + ".pts")).exists()
            record["leaked"] = leaked
            if leaked:
                record.update(status="leak", failed_gates=["leak"])
                return record
        if code != 0 or not bsp.is_file():
            record.update(failed_gates=[step])
            return record

    content = out / "content"
    shutil.rmtree(content, ignore_errors=True)
    (content / "maps").mkdir(parents=True)
    target = content / "maps" / (name + ".bsp")
    shutil.copy2(bsp, target)
    record.update(status="pass", content_root=str(content), bsp2_sha256=sha256(target))
    return record


def install(record, game_dir):
    """Copy a passed build's maps/<name>.bsp into <game_dir>/maps; returns the path."""
    if record.get("status") != "pass":
        raise ValueError("only a passed build can be installed")
    source = Path(record["content_root"]) / "maps" / (record["map"] + ".bsp")
    game_dir = Path(game_dir)
    if not game_dir.is_dir():
        raise ValueError("game directory is missing: " + str(game_dir))
    target = game_dir / "maps" / source.name
    target.parent.mkdir(exist_ok=True)
    partial = target.with_name(target.name + ".partial")
    shutil.copy2(source, partial)
    os.replace(partial, target)
    return target


def finish(out, record):
    (out / "build.json").write_text(json.dumps(record, indent=2) + "\n")
    return record


def boot(record, out, runtime, renderer="native-vulkan", timeout=240):
    """Boot the installed product headless on the built map (portal_boot.py)."""
    boot_out = out / "boot"
    command = [sys.executable, str(HERE / "portal_boot.py"), "--runtime", str(runtime),
               "--content-root", record["content_root"], "--map", record["map"],
               "--renderer", renderer, "--headless", "--out", str(boot_out),
               "--timeout", str(timeout)]
    code = subprocess.run(command).returncode
    evidence_path = boot_out / "evidence.json"
    evidence = json.loads(evidence_path.read_text()) if evidence_path.is_file() else {}
    return {"returncode": code, "status": evidence.get("status", "missing"),
            "failures": evidence.get("failures", []), "evidence": str(evidence_path),
            "screenshots": evidence.get("screenshots", [])}


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    b = sub.add_parser("build", help="compile a VMF (and optionally publish and boot it)")
    b.add_argument("--vmf", type=Path, required=True)
    b.add_argument("--out", type=Path, required=True)
    b.add_argument("--name", help="map name (default: the VMF's file name)")
    b.add_argument("--quality", choices=("fast", "full"), default="fast")
    b.add_argument("--toolchain", type=Path, default=TOOLCHAIN)
    b.add_argument("--runtime", type=Path, default=ROOT / "run/runtime",
                   help="staged runtime the map's materials come from")
    b.add_argument("--publish", action="store_true", help="publish to the playable map store")
    b.add_argument("--install-game-dir", type=Path,
                   help="also copy the map into <dir>/maps (a staged game directory)")
    b.add_argument("--boot", action="store_true", help="boot the map headless (portal_boot.py)")
    b.add_argument("--boot-runtime", type=Path, default=ROOT / "run/runtime-native")
    args = parser.parse_args()

    tools = Path(json.loads(args.toolchain.read_text())["compile_tools"])
    out = args.out.resolve()
    record = build(args.vmf, out, tools, args.runtime.resolve(), args.quality, args.name)
    print("vmf_map_build: %s %s (%s)" % (record["map"], record["status"], out / "build.json"))
    if record["status"] != "pass":
        return 1
    if args.publish:
        print("published " + playable_maps.describe(playable_maps.publish(record)))
    if args.install_game_dir:
        print("installed " + str(install(record, args.install_game_dir.resolve())))
    if args.boot:
        result = boot(record, out, args.boot_runtime.resolve())
        record["boot"] = result
        finish(out, record)
        print("vmf_map_build: boot %s %s" % (result["status"], "; ".join(result["failures"])))
        if result["status"] != "pass":
            return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
