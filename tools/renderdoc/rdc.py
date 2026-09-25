#!/usr/bin/env python3
"""Capture and inspect RenderDoc frames of the engine, headless.

Capture (a private runtime under `renderdoccmd`, offscreen, never a window):

    rdc.py capture --out DIR --map gi_door --content-root MAPBUILD/content \
        --console-command "ent_fire Door Enable"

forwards to `tools/quality/portal_boot.py --renderdoc --headless
--shader-debug`, which captures the frame after the final screenshot command
with the backend's debug shader variants (GLSL names, source-level debugging;
`--release-shaders` keeps the embedded ones); `gi_runtime.py capture
--renderdoc` does the same for a GI fixture camera. The .rdc paths are printed
and recorded in the boot's evidence.json. Under RenderDoc the backend names its
objects and labels its passes (README.md).

Inspect (qrenderdoc's embedded Python, replayed offscreen; rdc_replay.py):

    rdc.py info CAPTURE
    rdc.py draws CAPTURE [--bindings]
    rdc.py bindings CAPTURE [--event E] [--stage pixel]
    rdc.py pixel CAPTURE --x X --y Y [--event E] [--texture ID]
    rdc.py debug-pixel CAPTURE --event E --x X --y Y [--var NAME ...]
    rdc.py save-texture CAPTURE --texture ID --out FILE.{png,exr,dds} [--event E]
    rdc.py script CAPTURE SCRIPT.py [ARG ...]
    rdc.py gui CAPTURE          (opens the qrenderdoc window, for a person)

Every inspection command takes --json for the raw response. qrenderdoc runs
with a private XDG_DATA_HOME whose UI.config opts out of the first-run
analytics prompt (a modal dialog that otherwise blocks a headless run), so the
user's own RenderDoc settings are neither read nor changed.
"""

import argparse
import glob
import json
import os
import shutil
import signal
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
WORKER = HERE / "rdc_replay.py"
# qrenderdoc's persisted-config format: this magic key and version, then settings.
UI_CONFIG = {"rdocConfigData": 1, "Analytics_TotalOptOut": True,
             "CheckUpdate_AllowChecks": False}
DEFAULT_TIMEOUT = 600


class RdcError(RuntimeError):
    pass


def private_home():
    """qrenderdoc's private data directory, with the prompt-free UI.config."""
    base = Path(os.environ.get("XDG_CACHE_HOME") or Path.home() / ".cache")
    home = base / "source-engine" / "renderdoc-home"
    config = home / "qrenderdoc" / "UI.config"
    config.parent.mkdir(parents=True, exist_ok=True)
    try:
        current = json.loads(config.read_text())
    except (OSError, ValueError):
        current = {}
    if any(current.get(key) != value for key, value in UI_CONFIG.items()):
        current.update(UI_CONFIG)
        config.write_text(json.dumps(current, indent=1) + "\n")
    return home


def headless_environment(home):
    environment = os.environ.copy()
    for variable in ("DISPLAY", "WAYLAND_DISPLAY"):
        environment.pop(variable, None)
    environment["QT_QPA_PLATFORM"] = "offscreen"
    environment["XDG_DATA_HOME"] = str(home)
    return environment


def latest_renderdoc_log():
    logs = sorted(glob.glob("/tmp/RenderDoc/RenderDoc_*.log"), key=os.path.getmtime)
    return logs[-1] if logs else None


def replay(command, capture, args=None, timeout=DEFAULT_TIMEOUT, qrenderdoc=None):
    """Run one worker command on `capture`; returns its result or raises."""
    qrenderdoc = qrenderdoc or shutil.which("qrenderdoc")
    if not qrenderdoc:
        raise RdcError("qrenderdoc is not installed (RenderDoc's UI provides the replay Python)")
    capture = Path(capture).resolve()
    if not capture.is_file():
        raise RdcError("no capture %s" % capture)
    environment = headless_environment(private_home())
    with tempfile.TemporaryDirectory(prefix="rdc-") as scratch:
        request, response = Path(scratch) / "request.json", Path(scratch) / "response.json"
        request.write_text(json.dumps({"command": command, "capture": str(capture),
                                       "args": args or {}}))
        environment["RDC_REQUEST"], environment["RDC_RESPONSE"] = str(request), str(response)
        output = Path(scratch) / "qrenderdoc.log"
        with output.open("wb") as stream:
            process = subprocess.Popen([qrenderdoc, "--python", str(WORKER)], env=environment,
                                       stdout=stream, stderr=subprocess.STDOUT,
                                       stdin=subprocess.DEVNULL, start_new_session=True)
            try:
                process.wait(timeout=timeout)
            except subprocess.TimeoutExpired:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait()
                raise RdcError("qrenderdoc did not answer within %ds (a modal dialog or a "
                               "hung replay; RenderDoc log: %s)" % (timeout, latest_renderdoc_log()))
        if not response.is_file():
            tail = output.read_text(errors="replace")[-2000:]
            raise RdcError("qrenderdoc exited %s without a response (RenderDoc log: %s)\n%s" % (
                process.returncode, latest_renderdoc_log(), tail))
        answer = json.loads(response.read_text())
    if answer.get("error"):
        raise RdcError(answer["error"].rstrip())
    return answer["result"]


# -- human-readable output ---------------------------------------------------

def texture_text(info):
    if not info:
        return "-"
    text = "#%d %s" % (info["id"], info["name"])
    if "width" in info:
        size = "%dx%d" % (info["width"], info["height"])
        if info.get("depth", 1) > 1:
            size += "x%d" % info["depth"]
        text += " %s %s" % (size, info["format"])
        if info.get("mips", 1) > 1:
            text += " %d mips" % info["mips"]
        if info.get("array", 1) > 1:
            text += " [%d]" % info["array"]
    return text


def numbers(values):
    if isinstance(values, dict):
        return "{%s}" % ", ".join("%s: %s" % (k, numbers(v)) for k, v in values.items())
    if isinstance(values, list):
        return "(" + ", ".join(numbers(v) for v in values) + ")" if len(values) != 1 \
            else numbers(values[0])
    if isinstance(values, float):
        return "%.6g" % values
    return str(values)


def print_stage(name, stage):
    if stage is None:
        print("  %s: no shader" % name)
        return
    print("  %s shader #%d %s%s" % (name, stage["shader"], stage["entry"],
                                    "" if stage["debug_info"] else " (no debug info)"))
    for resource in stage["resources"]:
        where = "set %s binding %s" % (resource.get("set", "?"), resource.get("binding", "?"))
        print("    %-22s %-18s %s%s" % (where, resource.get("name", "?"),
                                        texture_text(resource["resource"]),
                                        "  (statically unused)" if resource["statically_unused"]
                                        else ""))
    for block in stage["constants"]:
        print("    %s %s:" % ("push constants" if block["push_constants"] else
                              "set %s binding %s" % (block["set"], block["binding"]),
                              block["name"]))
        for key, value in block["values"].items():
            print("      %s = %s" % (key, numbers(value)))


def show(command, result):
    if command == "info":
        for key, value in result.items():
            print("%s: %s" % (key, value))
    elif command == "draws":
        for row in result:
            markers = " / ".join(row["markers"])
            print("EID %-6d %-26s idx %-7d inst %-3d -> %s%s" % (
                row["event"], row["name"], row["indices"], row["instances"],
                ", ".join(texture_text(o) for o in row["outputs"]) or "-",
                "   [%s]" % markers if markers else ""))
            if "stage" in row:
                print_stage("stage", row["stage"])
            elif row.get("resources"):
                print("           reads: %s" % ", ".join(row["resources"]))
    elif command == "bindings":
        print("EID %d" % result["event"])
        for name, stage in result["stages"].items():
            print_stage(name, stage)
    elif command == "pixel":
        print("(%d, %d) of %s at EID %d = %s" % (result["x"], result["y"],
                                                 texture_text(result["texture"]),
                                                 result["event"],
                                                 numbers(result["value_at_event"])))
        for entry in result["history"]:
            out = entry["shader_out"]
            print("  EID %-6d prim %-5s %-6s shader out %-40s after %s%s" % (
                entry["event"], entry["primitive"], "pass" if entry["passed"] else "FAIL",
                numbers(out["color"]) if out else "-",
                numbers(entry["after"]["color"]) if entry["after"] else "-",
                "  " + ",".join(entry["flags"]) if entry["flags"] else ""))
    elif command == "debug-pixel":
        print("EID %d (%d, %d): %d steps" % (result["event"], result["x"], result["y"],
                                             result["steps"]))
        for name, value in sorted(result["source_variables"].items()):
            print("  %-28s %s" % (name, numbers(value)))
        for name, value in sorted(result.get("registers", {}).items()):
            print("  [reg] %-22s %s" % (name, numbers(value)))
    elif command == "save-texture":
        print("%s -> %s" % (texture_text(result["texture"]), result["out"]))
    else:
        print(json.dumps(result, indent=1))


# -- capture -------------------------------------------------------------------

def capture(forwarded):
    """portal_boot.py --renderdoc --headless with the caller's arguments."""
    if not shutil.which("renderdoccmd"):
        raise RdcError("renderdoccmd is not installed")
    release = "--release-shaders" in forwarded
    forwarded = [arg for arg in forwarded if arg != "--release-shaders"]
    boot = [sys.executable, str(ROOT / "tools/quality/portal_boot.py"), "--renderdoc",
            "--headless"] + ([] if release else ["--shader-debug"]) + forwarded
    if "--renderer" not in forwarded:
        boot += ["--renderer", "native-vulkan"]
    if "--runtime" not in forwarded:
        boot += ["--runtime", str(ROOT / "run/runtime-native")]
    if "--build" not in forwarded:
        boot += ["--build", str(ROOT / "build")]
    code = subprocess.run(boot, cwd=ROOT).returncode
    out = Path(forwarded[forwarded.index("--out") + 1])
    evidence = json.loads((out / "evidence.json").read_text()) \
        if (out / "evidence.json").is_file() else {}
    captures = evidence.get("renderdoc", {}).get("captures", [])
    for path in captures:
        print(path)
    if not captures:
        raise RdcError("no RenderDoc capture was written (portal_boot exit %d; %s)" % (
            code, out / "evidence.json"))
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--timeout", type=int, default=DEFAULT_TIMEOUT)
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("capture", help="boot headless under RenderDoc (portal_boot.py arguments "
                                   "follow; --out is required)", add_help=False)

    def inspect(name, help_text):
        p = sub.add_parser(name, help=help_text)
        p.add_argument("capture", type=Path)
        p.add_argument("--json", action="store_true", help="print the raw response")
        return p

    inspect("info", "API, frame and counts")
    p = inspect("draws", "every draw/dispatch with its targets and reflected inputs")
    p.add_argument("--bindings", action="store_true", help="full bindings and constants")
    p.add_argument("--stage", choices=("vertex", "pixel", "compute"), default="pixel")
    p = inspect("bindings", "a draw's shaders, bound resources and constant values")
    p.add_argument("--event", type=int, help="event id (default: the last draw)")
    p.add_argument("--stage", choices=("vertex", "pixel", "compute", "geometry"))
    p = inspect("pixel", "pixel history and value of a target pixel")
    p.add_argument("--x", type=int, required=True)
    p.add_argument("--y", type=int, required=True)
    p.add_argument("--event", type=int, help="event whose target is read (default: last draw)")
    p.add_argument("--texture", help="texture id or name instead of the event's colour target")
    p.add_argument("--target", type=int, default=0, help="colour target index")
    p = inspect("debug-pixel", "step a pixel shader; last value of each source variable")
    p.add_argument("--event", type=int, required=True)
    p.add_argument("--x", type=int, required=True)
    p.add_argument("--y", type=int, required=True)
    p.add_argument("--primitive", type=int, help="the primitive to debug when several cover it")
    p.add_argument("--var", action="append", dest="variables", help="only these names")
    p.add_argument("--registers", action="store_true", help="also report every register")
    p = inspect("save-texture", "write a texture (as bound at an event) to a file")
    p.add_argument("--texture", required=True, help="texture id or name")
    p.add_argument("--out", type=Path, required=True)
    p.add_argument("--event", type=int)
    p.add_argument("--mip", type=int, default=0)
    p.add_argument("--slice", type=int, default=0)
    p = inspect("script", "run a Python file in the replay (replay, controller, rd, args, "
                          "result)")
    p.add_argument("script", type=Path)
    p.add_argument("script_args", nargs=argparse.REMAINDER)
    p = sub.add_parser("gui", help="open the capture in the qrenderdoc window (for a person)")
    p.add_argument("capture", type=Path)

    argv = list(sys.argv[1:] if argv is None else argv)
    if argv[:1] == ["capture"]:
        if "--out" not in argv:
            parser.error("capture needs --out DIR (and portal_boot.py's --map etc.)")
        try:
            return capture(argv[1:])
        except RdcError as error:
            print("rdc: %s" % error, file=sys.stderr)
            return 1
    args = parser.parse_args(argv)
    if args.command == "gui":
        os.execvp("qrenderdoc", ["qrenderdoc", str(args.capture)])
    request = {key: value for key, value in vars(args).items()
               if key not in ("command", "capture", "json", "timeout") and value is not None}
    for key in ("out", "script"):
        if key in request:
            request[key] = str(Path(request[key]).resolve())
    try:
        result = replay(args.command, args.capture, request, timeout=args.timeout)
    except RdcError as error:
        print("rdc: %s" % error, file=sys.stderr)
        return 1
    if args.json:
        print(json.dumps(result, indent=1))
    else:
        show(args.command, result)
    return 0


if __name__ == "__main__":
    sys.exit(main())
