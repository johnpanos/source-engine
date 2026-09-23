#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Compare dedicated-server map, collision and visibility results for VBSP/BSP2."""

import argparse
import hashlib
import json
from pathlib import Path
import re
import sys

import conformance


INFO = re.compile(
    r"map_container_info: (\S+) kind=(legacy-vbsp|bsp2) "
    r"legacy_version=(\d+) revision=(\d+) lumps=(\d+)$")
GAME_LUMP = re.compile(r"map_container_info: gamelump .+ loaded=1 crc=[0-9a-f]{8}$")
PROBE = re.compile(
    r"map_container_probe: leaf=(\d+) cluster=(-?\d+) contents=([0-9a-f]+) "
    r"pvs_crc=([0-9a-f]{8}) fraction=([0-9.eE+-]+) "
    r"startsolid=([01]) allsolid=([01]) hit_contents=([0-9a-f]+)$")
STATUS = re.compile(r"map\s+: (\S+) at: .+$")
EDICTS = re.compile(r"edicts\s+: (\d+) used of (\d+) max$")


def read_capture(path):
    data = Path(path).read_bytes()
    lines = [line.strip() for line in data.decode("utf-8").splitlines()]
    info_lines = [line for line in lines if line.startswith("map_container_info: ")
                  and " kind=" in line]
    game_lumps = [line for line in lines if line.startswith("map_container_info: gamelump ")]
    probe_lines = [line for line in lines if line.startswith("map_container_probe: ")]
    statuses = [match for line in lines if (match := STATUS.fullmatch(line))]
    edicts = [match for line in lines if (match := EDICTS.fullmatch(line))]
    if len(info_lines) != 1 or len(statuses) != 1 or len(edicts) != 1:
        raise ValueError("expected one container info, map status and edict count")
    info = INFO.fullmatch(info_lines[0])
    if not info:
        raise ValueError("invalid container info")
    if Path(info[1]).stem != statuses[0][1]:
        raise ValueError("container info and server status name different maps")
    if not game_lumps or any(not GAME_LUMP.fullmatch(line) for line in game_lumps):
        raise ValueError("missing or failed game lump")
    if len(probe_lines) < 3:
        raise ValueError("fewer than three server probes")
    probes = [PROBE.fullmatch(line) for line in probe_lines]
    if any(match is None for match in probes):
        raise ValueError("invalid or failed server probe")
    if not any(0.0 < float(match[5]) < 1.0 and int(match[8], 16) != 0
               for match in probes):
        raise ValueError("no probe hit world geometry")
    if not any(int(match[2]) >= 0 for match in probes):
        raise ValueError("no probe has a visible cluster")
    return {
        "path": str(Path(path).resolve()),
        "sha256": hashlib.sha256(data).hexdigest(),
        "map_file": info[1],
        "kind": info[2],
        "legacy_version": int(info[3]),
        "revision": int(info[4]),
        "game_lumps": game_lumps,
        "probes": probe_lines,
        "status_map": statuses[0][1],
        "edicts": tuple(int(value) for value in edicts[0].groups()),
    }


def compare(legacy, bsp2):
    failures = []
    if legacy["kind"] != "legacy-vbsp" or bsp2["kind"] != "bsp2":
        failures.append("wrong container kinds")
    for key in ("map_file", "legacy_version", "revision", "game_lumps",
                "probes", "status_map", "edicts"):
        if legacy[key] != bsp2[key]:
            failures.append(key + " differs")
    return failures


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--legacy-log", required=True)
    parser.add_argument("--bsp2-log", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args(argv)
    evidence = {"schema": "bsp2-server-evidence/v1",
                "source": conformance.source_identity(conformance.repo_root()),
                "inputs": {"legacy": args.legacy_log, "bsp2": args.bsp2_log}}
    try:
        legacy = read_capture(args.legacy_log)
        bsp2 = read_capture(args.bsp2_log)
        failures = compare(legacy, bsp2)
        evidence.update({"legacy": legacy, "bsp2": bsp2,
                         "probe_count": len(legacy["probes"])})
    except (OSError, UnicodeError, ValueError) as error:
        failures = [str(error)]
    evidence["status"] = "fail" if failures else "pass"
    evidence["failures"] = failures
    output = Path(args.out)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("BSP2 server comparison: %s (%s)" % (evidence["status"], output))
    for failure in failures:
        print("  " + failure)
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
