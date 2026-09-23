#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Run the same Portal map probes on VBSP and BSP2 products.

Licensed content is a required local input. Each run uses a private staged
runtime; source content and the selected Waf build are never modified.
"""

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

import bsp2_server_compare
import bsp2_reader
import conformance
import portal_boot
import stage_runtime


SCHEMA = "bsp2-dedicated-cases/v1"
EVIDENCE_SCHEMA = "bsp2-dedicated-evidence/v1"
MAP_NAME = re.compile(r"[A-Za-z0-9_-]+")
ACTIVE_PLAYER = re.compile(r"#\s*\d+\s+\"[^\n]*\"[^\n]*\bactive\b")


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def load_cases(path):
    declaration = json.loads(Path(path).read_text())
    if (not isinstance(declaration, dict) or declaration.get("schema") != SCHEMA or
            not isinstance(declaration.get("cases"), list)):
        raise ValueError("invalid dedicated case manifest")
    cases = declaration["cases"]
    if not cases:
        raise ValueError("dedicated case manifest has zero cases")
    names = set()
    for case in cases:
        if not isinstance(case, dict):
            raise ValueError("dedicated case is not an object")
        name = case.get("map")
        probes = case.get("probes")
        if not isinstance(name, str) or not MAP_NAME.fullmatch(name) or name in names:
            raise ValueError("invalid or duplicate map name")
        names.add(name)
        if not isinstance(probes, list) or len(probes) < 3:
            raise ValueError("map %s has fewer than three probes" % name)
        for probe in probes:
            if (not isinstance(probe, list) or len(probe) != 6 or
                    any(isinstance(value, bool) or not isinstance(value, (int, float)) or
                        not math.isfinite(value) or abs(value) > 1000000 for value in probe)):
                raise ValueError("map %s has an invalid probe" % name)
    return cases


def check_case_sources(cases, source_maps, inventory_path):
    expected, inventory = bsp2_reader.load_expected_inventory(inventory_path)
    for case in cases:
        name = case["map"] + ".bsp"
        if name not in expected:
            raise ValueError("case map is absent from source inventory: " + name)
        path = source_maps / name
        if not path.is_file():
            raise ValueError("required inventory map missing: " + str(path))
        with path.open("rb") as stream:
            header = stream.read(8)
        version = int.from_bytes(header[4:8], "little", signed=True) if header[:4] == b"VBSP" else None
        if (sha256(path), version) != expected[name]:
            raise ValueError("source inventory hash or version differs: " + name)
    return inventory


def probe_command(probe):
    return "map_container_probe " + " ".join(format(float(value), ".9g") for value in probe)


def compare_case_observations(probe_count, expected_version, legacy, bsp2):
    if len(legacy["probes"]) != probe_count or len(bsp2["probes"]) != probe_count:
        raise ValueError("server did not execute every declared probe")
    if legacy["legacy_version"] != expected_version or bsp2["legacy_version"] != expected_version:
        raise ValueError("server loaded a different legacy version")
    return bsp2_server_compare.compare(legacy, bsp2)


def client_player_active(log):
    return bool(ACTIVE_PLAYER.search(Path(log).read_text(errors="replace")))


def run_server(stage, name, log, timeout, port):
    command = [str(stage / "dedicated_launcher"), "-game", "portal", "-defaultgamedir", "portal",
               "-console", "-consolelog", str(log), "-insecure", "-port", str(port),
               "+map", name, "+wait", "120", "+exec", "rfc0008_probes.cfg", "+quit"]
    environment = dict(os.environ)
    environment["LD_LIBRARY_PATH"] = str(stage / "bin") + ":" + environment.get("LD_LIBRARY_PATH", "")
    try:
        completed = subprocess.run(command, cwd=stage, env=environment, input=b"",
                                   capture_output=True, timeout=timeout, check=False)
        result = {"argv": command, "exit_code": completed.returncode}
        stdout, stderr = completed.stdout, completed.stderr
    except subprocess.TimeoutExpired as error:
        result = {"argv": command, "exit_code": None, "timeout": True}
        stdout, stderr = error.stdout or b"", error.stderr or b""
    for label, data in (("stdout", stdout), ("stderr", stderr)):
        path = log.with_suffix("." + label)
        path.write_bytes(data)
        result[label] = str(path)
    result["console_log"] = str(log)
    return result


def run_client(stage, name, log, timeout, _port):
    command = [str(stage / "hl2_launcher"), "-renderer", "null", "-game", "portal",
               "-windowed", "-w", "640", "-h", "480", "-multirun", "-novid", "-insecure",
               "-console", "-condebug", "-dev", "-physics", "vphysics", "+sv_cheats", "1",
               "+mat_queue_mode", "0", "+fps_max", "60", "+map", name, "+wait", "180",
               "+exec", "rfc0008_probes.cfg", "+wait", "10", "+quit"]
    environment = dict(os.environ)
    environment.update({"LD_LIBRARY_PATH": str(stage / "bin") + ":" +
                        environment.get("LD_LIBRARY_PATH", ""),
                        "SteamAppId": "400", "SteamGameId": "400",
                        "SDL_VIDEODRIVER": "offscreen", "SDL_VIDEO_DRIVER": "offscreen"})
    environment.pop("DISPLAY", None)
    environment.pop("WAYLAND_DISPLAY", None)
    native_log = stage / "portal/console.log"
    native_log.unlink(missing_ok=True)
    try:
        completed = subprocess.run(command, cwd=stage, env=environment, input=b"",
                                   capture_output=True, timeout=timeout, check=False)
        result = {"argv": command, "exit_code": completed.returncode}
        stdout, stderr = completed.stdout, completed.stderr
    except subprocess.TimeoutExpired as error:
        result = {"argv": command, "exit_code": None, "timeout": True}
        stdout, stderr = error.stdout or b"", error.stderr or b""
    for label, data in (("stdout", stdout), ("stderr", stderr)):
        path = log.with_suffix("." + label)
        path.write_bytes(data)
        result[label] = str(path)
    if native_log.is_file():
        shutil.copyfile(native_log, log)
    result["console_log"] = str(log)
    return result


def run_case(case, source_maps, stage, tool, output, timeout, port, foreign_content,
             run_product, result):
    name = case["map"]
    source_map = source_maps / (name + ".bsp")
    if not source_map.is_file():
        raise ValueError("required map missing: " + str(source_map))
    with source_map.open("rb") as stream:
        header = stream.read(8)
    if header[:4] != b"VBSP" or int.from_bytes(header[4:8], "little", signed=True) not in (19, 20, 21):
        raise ValueError("required map is not a declared VBSP v19-21 file: " + name)
    staged_map = stage / "portal/maps" / (name + ".bsp")
    if foreign_content:
        if staged_map.exists() or staged_map.is_symlink():
            raise ValueError("foreign map would replace staged Portal content: " + name)
        shutil.copyfile(source_map, staged_map)
    elif not staged_map.is_symlink() or staged_map.resolve() != source_map.resolve():
        raise ValueError("staged map is not the expected private asset link: " + name)

    config = stage / "portal/cfg/rfc0008_probes.cfg"
    config.parent.mkdir(parents=True, exist_ok=True)
    config.write_text("\n".join(["map_container_info"] +
                                [probe_command(probe) for probe in case["probes"]] +
                                ["status", ""]))
    legacy_log = output / (name + "-legacy.log")
    bsp2_log = output / (name + "-bsp2.log")
    legacy_log.unlink(missing_ok=True)
    bsp2_log.unlink(missing_ok=True)
    result.update({"source_sha256": sha256(source_map),
                   "config_sha256": sha256(config), "probe_count": len(case["probes"])})
    result["legacy_run"] = run_product(stage, name, legacy_log, timeout, port)
    if result["legacy_run"]["exit_code"] != 0:
        raise ValueError("legacy server failed or timed out: " + name)

    staged_map.unlink()
    converted = subprocess.run([str(tool), "convert", str(source_map), str(staged_map)],
                               capture_output=True, text=True, timeout=120, check=False)
    if converted.returncode != 0:
        raise ValueError("BSP2 conversion failed for %s: %s" % (name, converted.stderr.strip()))
    verified = subprocess.run([str(tool), "verify", str(staged_map)],
                              capture_output=True, text=True, timeout=120, check=False)
    if verified.returncode != 0:
        raise ValueError("BSP2 verification failed for %s: %s" % (name, verified.stderr.strip()))
    result["bsp2_sha256"] = sha256(staged_map)
    result["bsp2_run"] = run_product(stage, name, bsp2_log, timeout, port)
    if result["bsp2_run"]["exit_code"] != 0:
        raise ValueError("BSP2 server failed or timed out: " + name)
    legacy = bsp2_server_compare.read_capture(legacy_log)
    bsp2 = bsp2_server_compare.read_capture(bsp2_log)
    if run_product is run_client:
        if not client_player_active(legacy_log) or not client_player_active(bsp2_log):
            raise ValueError("client player did not become active: " + name)
    expected_version = int.from_bytes(header[4:8], "little", signed=True)
    failures = compare_case_observations(len(case["probes"]), expected_version, legacy, bsp2)
    if failures:
        raise ValueError("server observations differ for %s: %s" % (name, ", ".join(failures)))
    result.update({"legacy": legacy, "bsp2": bsp2, "status": "pass"})
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runtime", required=True, help="private Portal content root")
    parser.add_argument("--map-source-root", help="separate map corpus for engine-only compatibility checks")
    parser.add_argument("--product", choices=("dedicated", "client"), default="dedicated")
    parser.add_argument("--build", required=True, help="Waf --dedicated --build-games=portal output")
    parser.add_argument("--tool", required=True, help="built bsp2tool executable")
    parser.add_argument("--cases", default="quality/fixtures/bsp2-dedicated-cases.json")
    parser.add_argument("--source-inventory", help="versioned map names, versions and hashes")
    parser.add_argument("--out", required=True, help="directory for logs and evidence")
    parser.add_argument("--timeout", type=int, default=30)
    parser.add_argument("--port", type=int, default=28040)
    args = parser.parse_args(argv)
    output = Path(args.out).resolve()
    source_runtime = Path(args.runtime).resolve()
    source_maps = Path(args.map_source_root).resolve() if args.map_source_root else source_runtime / "portal/maps"
    build = Path(args.build).resolve()
    tool = Path(args.tool).resolve()
    schema = EVIDENCE_SCHEMA if args.product == "dedicated" else "bsp2-client-evidence/v1"
    evidence = {"schema": schema, "source": conformance.source_identity(conformance.repo_root()),
                "inputs": {"runtime": str(source_runtime), "build": str(build),
                           "tool": str(tool), "cases": str(Path(args.cases).resolve()),
                           "source_maps": str(source_maps),
                           "foreign_game_content": bool(args.map_source_root),
                           "product": args.product},
                "cases": [], "failures": []}
    try:
        cases = load_cases(args.cases)
        if args.timeout < 1 or not 1 <= args.port <= 65535 or args.port + len(cases) > 65536:
            raise ValueError("timeout or port is out of range")
        if not (source_runtime / "portal/gameinfo.txt").is_file():
            raise ValueError("required Portal runtime is missing")
        if not source_maps.is_dir():
            raise ValueError("required map source root is missing")
        if args.source_inventory:
            evidence["inputs"]["source_inventory"] = check_case_sources(
                cases, source_maps, args.source_inventory)
        if source_runtime == output or source_runtime in output.parents:
            raise ValueError("output must be outside the source runtime")
        if not tool.is_file() or not os.access(tool, os.X_OK):
            raise ValueError("required bsp2tool is missing or not executable")
        evidence["inputs"]["tool_sha256"] = sha256(tool)
        evidence["inputs"]["cases_sha256"] = sha256(args.cases)
        output.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="bsp2-dedicated-") as scratch:
            stage = Path(scratch) / "runtime"
            portal_boot.stage_runtime(source_runtime, stage)
            launcher = "dedicated_launcher" if args.product == "dedicated" else "hl2_launcher"
            installed = portal_boot.install_build(build, stage, launcher_name=launcher)
            stage_runtime.sanitize(stage)
            if "portal/bin/libserver.so" not in installed or "bin/libengine.so" not in installed:
                raise ValueError("build has no Portal server or engine product")
            if args.product == "client" and "portal/bin/libclient.so" not in installed:
                raise ValueError("client build has no Portal client product")
            evidence["installed"] = installed
            run_product = run_server if args.product == "dedicated" else run_client
            for index, case in enumerate(cases):
                try:
                    result = {"map": case["map"]}
                    run_case(case, source_maps, stage, tool, output,
                             args.timeout, args.port + index, bool(args.map_source_root),
                             run_product, result)
                    evidence["cases"].append(result)
                except (OSError, ValueError, subprocess.TimeoutExpired) as error:
                    result.update({"status": "fail", "error": str(error)})
                    evidence["cases"].append(result)
                    evidence["failures"].append("%s: %s" % (case["map"], error))
    except (OSError, ValueError, json.JSONDecodeError) as error:
        evidence["failures"].append(str(error))
    evidence["status"] = "pass" if evidence["cases"] and not evidence["failures"] else "fail"
    output.mkdir(parents=True, exist_ok=True)
    evidence_path = output / "evidence.json"
    evidence_path.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("BSP2 %s: %s (%s)" % (args.product, evidence["status"], evidence_path))
    for failure in evidence["failures"]:
        print("  " + failure)
    return 0 if evidence["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
