#!/usr/bin/env python3
"""RFC 0011 G4 evidence for the precomputed radiosity producer, on baked maps.

    python3 tools/quality/gi_radiosity.py furnace --out quality-results/rfc0011-g4/furnace.json
    python3 tools/quality/gi_radiosity.py convergence --out .../room-states-convergence.json
    python3 tools/quality/gi_radiosity.py budget --out .../budget.json
    python3 tools/quality/gi_radiosity.py states --out quality-results/rfc0011-g4/states

Every command runs the product's solver (public/render/indirect_radiosity.h)
through `rtrntool` on a pipeline build of a fixture
(quality-results/rfc0011-maps/<fixture>, or --map-build) and writes a receipt
with its status; the exit status is nonzero when a gate fails.

`furnace` (G4.2): a cold start (no indirect light) of the furnace map's
transfer must reach the analytic infinite-bounce indirect light (the
fixture's diffuse_light_indirect, 0.45) within 2% in the producer's
declared convergenceFrames, at the probes that gather patches; the
one-bounce defect must never reach it.

`convergence` (G4.3): on room-states, switching the ceiling panel or the
screen's emission off (their light styles' scalar 0) must bring every
probe's change within 2% of its settled value in convergenceFrames.

`budget` (G4.5): the per-update CPU cost on one thread while converging
(update plus composition), and the transfer's memory, judged against the
desktop row of quality/budgets/indirect-light-v1.json.

`states` (G4.3): in-game captures of room-states' indirect-light view with
radiosity selected and each state switched in game (`ent_fire`), compared
with that state's Cycles indirect-only reference, world and model regions
gated; and the negative control, the baked producer after the panel is
switched off, which must fail.
"""

import argparse
import json
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import gi_budgets  # noqa: E402
import pbrt_map_toolchain  # noqa: E402

MAPS = ROOT / "quality-results" / "rfc0011-maps"
FIXTURES = ROOT / "quality" / "fixtures" / "gi"
TOLERANCE = 0.02
# room-states switches: state -> (source, console command).
STATES = {"panel-off": ("CeilingPanel", "ent_fire CeilingPanel TurnOff"),
          "screen-off": ("screen", "ent_fire screen TurnOff")}
DESKTOP = "portal-linux-wayland-native-vulkan"


def rtrntool():
    profile, _ = pbrt_map_toolchain.load_profiles()
    toolchain = pbrt_map_toolchain.load(ROOT / profile["layout"]["toolchain_file"])
    return Path(toolchain["compile_tools"]) / "rtrntool"


def lighting(map_build):
    base = Path(map_build) / "lighting"
    return base / "probe_volume.prbv", base / "radiosity.rtrn"


def run_tool(*arguments):
    result = subprocess.run([str(rtrntool()), *map(str, arguments)], capture_output=True,
                            text=True, timeout=600)
    if result.returncode:
        raise SystemExit("rtrntool failed: %s" % result.stderr.strip())
    return json.loads(result.stdout)


def write(out, record):
    out = Path(out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(record, indent=2, sort_keys=True) + "\n")
    print("%s: %s" % (record["gate"], record["status"]))
    return 0 if record["status"] == "pass" else 1


def furnace(args):
    fixture = json.loads((FIXTURES / "furnace" / "fixture.json").read_text())
    expected = fixture["analytic"]["diffuse_light_indirect"]
    prbv, rtrn = lighting(args.map_build or MAPS / "furnace")
    product = run_tool("converge", prbv, rtrn, "--absolute", "--expect", expected,
                       "--tolerance", TOLERANCE, "--frames", 240)
    defect = run_tool("converge", prbv, rtrn, "--absolute", "--one-bounce", "--expect",
                      expected, "--tolerance", TOLERANCE, "--frames", 240)
    declared = product["declared_convergence_frames"]
    reached = product["frames_to_tolerance"]
    passed = 1 <= reached <= declared and defect["frames_to_tolerance"] == -1
    return write(args.out, {
        "gate": "G4.2 furnace", "status": "pass" if passed else "fail",
        "expected_indirect": expected, "tolerance": TOLERANCE,
        "declared_convergence_frames": declared, "frames_to_tolerance": reached,
        "settled": product["settled"], "probes": product["probes"],
        "one_bounce": {"settled": defect["settled"],
                       "frames_to_tolerance": defect["frames_to_tolerance"]},
        "transfer": run_tool("info", prbv, rtrn), "means": product["means"][:40],
        "reproduce": "python3 tools/quality/gi_radiosity.py furnace --out <file>"})


def convergence(args):
    prbv, rtrn = lighting(args.map_build or MAPS / "room-states")
    results, passed = {}, True
    for state, (source, _) in STATES.items():
        run = run_tool("converge", prbv, rtrn, "--set", "%s=0" % source, "--tolerance",
                       TOLERANCE, "--frames", 240)
        declared = run["declared_convergence_frames"]
        ok = 1 <= run["frames_to_tolerance"] <= declared
        passed &= ok
        results[state] = {"source": source, "frames_to_tolerance": run["frames_to_tolerance"],
                          "declared_convergence_frames": declared,
                          "settled_change": run["settled"], "status": "pass" if ok else "fail",
                          "means": run["means"][:40]}
    return write(args.out, {"gate": "G4.3 convergence", "status": "pass" if passed else "fail",
                            "tolerance": TOLERANCE, "states": results,
                            "reproduce": "python3 tools/quality/gi_radiosity.py convergence "
                                         "--out <file>"})


def budget(args):
    prbv, rtrn = lighting(args.map_build or MAPS / "room-states")
    runs = {state: run_tool("bench", prbv, rtrn, "--set", "%s=0" % source)
            for state, (source, _) in STATES.items()}
    info = run_tool("info", prbv, rtrn)
    worst_median = max(run["median_ms"] for run in runs.values())
    worst = max(run["max_ms"] for run in runs.values())
    # The producer's memory: the transfer plus the solver's arrays and the
    # published volume it composes (one copy of the volume per publication).
    memory_mb = (info["rtrn_bytes"] + 2 * info["prbv_bytes"] +
                 info["patches"] * 6 * 3 * 4 + info["probes"] * 2 * 36 * 3 * 4 * 2) / 2 ** 20
    judged = gi_budgets.report(gi_budgets.load(), args.profile, "radiosity",
                               {"cpu_ms_per_update": worst_median, "memory_mb": memory_mb})
    return write(args.out, {"gate": "G4.5 budget", "status": judged["status"],
                            "profile": args.profile, "median_ms_worst_state": worst_median,
                            "max_ms": worst, "memory_mb": memory_mb, "runs": runs,
                            "budget": judged, "transfer": info,
                            "note": "one thread, the serial executor; median and maximum over "
                                    "the updates that converge each state"})


def states(args):
    map_build = Path(args.map_build or MAPS / "room-states")
    out = Path(args.out)
    captures = {"default": ("default", ["r_indirect_producer radiosity"], True),
                "panel-off": ("panel-off", ["r_indirect_producer radiosity; wait 60; " +
                                            STATES["panel-off"][1]], True),
                "screen-off": ("screen-off", ["r_indirect_producer radiosity; wait 60; " +
                                              STATES["screen-off"][1]], True),
                "baked-panel-off": ("panel-off", [STATES["panel-off"][1]], False)}
    results, passed = {}, True
    for name, (state, commands, should_pass) in captures.items():
        target = out / name
        capture = [sys.executable, HERE / "gi_runtime.py", "capture", "--fixture", "room-states",
                   "--map-build", map_build, "--out", target, "--build", args.build]
        if args.runtime:
            capture += ["--runtime", args.runtime]
        for command in commands:
            capture += ["--console-command", command]
        booted = subprocess.run([str(part) for part in capture], capture_output=True, text=True,
                                timeout=1200).returncode == 0
        compare = subprocess.run(
            [sys.executable, str(HERE / "gi_runtime.py"), "compare", "--fixture", "room-states",
             "--state", state, "--capture", str(target), "--out", str(target / "gate.json"),
             "--gate-models"], capture_output=True, text=True, timeout=600)
        gate = json.loads((target / "gate.json").read_text()) \
            if (target / "gate.json").is_file() else {}
        ok = booted and (gate.get("status") == "pass") == should_pass
        passed &= ok
        results[name] = {"state": state, "commands": commands, "booted": booted,
                         "expected": "pass" if should_pass else "fail",
                         "compare": gate.get("status"), "status": "pass" if ok else "fail",
                         "compare_log": compare.stdout[-2000:]}
    return write(out / "states.json", {"gate": "G4.3 states", "status":
                                       "pass" if passed else "fail", "build": args.build,
                                       "captures": results})


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("furnace", "convergence", "budget", "states"):
        command = commands.add_parser(name)
        command.add_argument("--map-build", type=Path)
        command.add_argument("--out", required=True)
        if name == "budget":
            command.add_argument("--profile", default=DESKTOP)
        if name == "states":
            command.add_argument("--build", default="build",
                                 help="the client build tree carrying the producer")
            command.add_argument("--runtime", help="base runtime (a private copy is booted)")
    args = parser.parse_args()
    return {"furnace": furnace, "convergence": convergence, "budget": budget,
            "states": states}[args.command](args)


if __name__ == "__main__":
    sys.exit(main())
