#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""End-to-end debug API smoke test against a real product.

Stages a private runtime (portal_boot's staging), boots it offscreen with
-debugapi on a private Unix socket and drives the whole session over
JSON-RPC: no +map/+wait/+screenshot command line and no log scraping.
Writes <out>/evidence.json (debugapi-smoke-evidence/v1) and exits nonzero on
any failed check.

  build-deps/debugapi-venv/bin/python tools/debugapi/smoke.py \\
      --runtime run/runtime --build build-debugapi --out quality-results/debugapi/client-1
"""

import argparse
import base64
import datetime
import json
import os
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import time

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(ROOT / "tools" / "quality"))

import client as api  # noqa: E402
import conformance  # noqa: E402
import portal_boot  # noqa: E402

SCHEMA = "debugapi-smoke-evidence/v1"


class Checks:
    def __init__(self):
        self.results = []

    def check(self, name, condition, detail=None):
        self.results.append({"name": name, "ok": bool(condition), "detail": detail})
        print("[%s] %s%s" % ("ok  " if condition else "FAIL", name,
                             "" if condition or detail is None else ": %s" % (detail,)), flush=True)
        return condition

    @property
    def failures(self):
        return [r for r in self.results if not r["ok"]]


def client_scenario(session, checks, args, output):
    hello = session.call("hello", protocolVersion="PROTOCOL_VERSION_1", clientName="smoke")
    providers = {p.role: p.id for p in hello.providers}
    checks.check("hello reports a client product", hello.product == api.pb.PRODUCT_CLIENT)
    checks.check("hello reports the root's render provider",
                 providers.get("render") == args.renderer, providers)
    checks.check("hello reports newline framing and the socket", hello.framing == "newline"
                 and hello.transport.startswith("unix:"), [hello.framing, hello.transport])

    subscribed = session.call("subscribe", log=True)
    checks.check("subscribe enables log notifications", subscribed.log)

    missing = None
    try:
        session.call("cvarGet", name="no_such_cvar_debugapi")
    except api.RpcError as error:
        missing = error
    checks.check("cvarGet of a missing cvar fails with NOT_FOUND",
                 missing is not None and missing.error_code == "ERROR_CODE_NOT_FOUND")

    cheats = session.call("cvarSet", name="sv_cheats", value="1")
    checks.check("cvarSet applies through the console path", cheats.value == "1", cheats.value)

    started = time.monotonic()
    loaded = session.call("exec", command="map %s" % args.map)
    checks.check("exec queues a map load", not loaded.truncated, loaded.output)
    active = session.call("waitFor", condition="WAIT_CONDITION_CLIENT_ACTIVE",
                          timeoutMs=int(args.load_timeout * 1000), timeout=args.load_timeout + 30)
    load_seconds = time.monotonic() - started
    checks.check("waitFor(CLIENT_ACTIVE) completes after the load",
                 active.status.client_signon == api.pb.SIGNON_STATE_FULL, api.to_json_dict(active))
    checks.check("status names the loaded map", active.status.map == args.map, active.status.map)

    frames = session.call("waitFor", condition="WAIT_CONDITION_FRAMES", frames=60, timeoutMs=60000,
                          timeout=90)
    checks.check("waitFor(FRAMES 60) counts frames", frames.frames_waited == 60, frames.frames_waited)

    status_output = session.call("exec", command="status").output
    checks.check("exec returns the command's own output",
                 ("map     : %s" % args.map) in status_output or args.map in status_output,
                 status_output[:400])

    wire = session.call("cvarSet", name="mat_wireframe", value="0")
    checks.check("cheat cvars are settable once sv_cheats is on", wire.value == "0")

    shot = session.call("screenshot", mode="SCREENSHOT_MODE_PATH", timeout=60)
    info = portal_boot.screenshot_info(Path(shot.path)) if shot.path else None
    checks.check("screenshot(PATH) writes a TGA with scene detail",
                 bool(info) and info.get("has_scene_detail"), info)
    jpeg = session.call("screenshot", mode="SCREENSHOT_MODE_INLINE_JPEG", jpegQuality=85,
                        timeout=60)
    checks.check("screenshot(INLINE_JPEG) returns JPEG bytes",
                 jpeg.jpeg[:2] == b"\xff\xd8" and jpeg.width == shot.width, len(jpeg.jpeg))
    (output / "inline.jpg").write_bytes(jpeg.jpeg)

    malformed = session.raw('{"jsonrpc":"2.0","id":')
    parse_errors = [m for m in malformed if isinstance(m, dict)
                    and m.get("error", {}).get("code") == -32700]
    checks.check("malformed JSON gets PARSE_ERROR", len(parse_errors) == 1, malformed)
    batch = session.raw(json.dumps([{"jsonrpc": "2.0", "id": "a", "method": "status"},
                                    {"jsonrpc": "2.0", "method": "status"},
                                    {"jsonrpc": "2.0", "id": "b", "method": "hello"}]))
    batches = [m for m in batch if isinstance(m, list)]
    checks.check("a batch is answered with one array of its requests",
                 len(batches) == 1 and sorted(m["id"] for m in batches[0]) == ["a", "b"], batch)
    checks.check("the connection survives protocol errors",
                 session.call("status").map == args.map)

    with api.DebugApiClient.connect_unix(args.socket, framing="content-length") as second:
        other = second.call("hello")
        checks.check("a second client can use the other framing", other.framing == "newline"
                     and other.product == api.pb.PRODUCT_CLIENT)

    logs = []
    while (item := session.next_notification(timeout=0.5)) is not None:
        logs.append(item[1].text)
    checks.check("console output arrives as log notifications", len(logs) > 0, len(logs))
    return {"load_seconds": round(load_seconds, 3), "screenshot": info,
            "screenshot_path": shot.path, "jpeg_bytes": len(jpeg.jpeg),
            "log_notifications": len(logs), "providers": providers,
            "build_revision": hello.build_revision}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--runtime", type=Path, required=True)
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--out", type=Path, required=True)
    parser.add_argument("--map", default="testchmb_a_00")
    parser.add_argument("--renderer", default="native-vulkan")
    parser.add_argument("--physics", default="vphysics")
    parser.add_argument("--load-timeout", type=float, default=180)
    parser.add_argument("--timeout", type=float, default=420)
    args = parser.parse_args(argv)

    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    if (output / "evidence.json").exists():
        parser.error("evidence already exists; use a new output directory")
    runtime_dir = os.environ.get("XDG_RUNTIME_DIR") or tempfile.gettempdir()
    args.socket = str(Path(runtime_dir) / ("source-debugapi-smoke-%d.sock" % os.getpid()))
    evidence = {"schema": SCHEMA, "status": "fail",
                "started_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
                "source": conformance.source_identity(conformance.repo_root()),
                "runtime": str(args.runtime.resolve()), "build": str(args.build.resolve()),
                "map": args.map, "renderer": args.renderer, "socket": args.socket}
    checks = Checks()
    process = None
    try:
        stage = output / "runtime"
        evidence["staging"] = portal_boot.stage_runtime(args.runtime, stage)
        evidence["build_overrides"] = portal_boot.install_build(args.build, stage)
        executable = stage / "hl2_launcher"
        evidence["executables"] = {str(path.relative_to(stage)): portal_boot.sha256(path)
                                   for path in [executable] + sorted((stage / "bin").glob("*.so"))}
        command = [str(executable), "-game", "portal", "-windowed", "-w", "1024", "-h", "768",
                   "-multirun", "-novid", "-insecure", "-console", "-condebug", "-dev",
                   "-physics", args.physics, "-renderer", args.renderer,
                   "-debugapi", "unix:" + args.socket, "+volume", "0"]
        evidence["command"] = command
        environment = os.environ.copy()
        environment["LD_LIBRARY_PATH"] = str(stage / "bin") + ":" + environment.get("LD_LIBRARY_PATH", "")
        environment["SteamAppId"] = environment["SteamGameId"] = "400"
        # SDL3 offscreen: real GPU rendering, no window on the user's desktop.
        environment["SDL_VIDEODRIVER"] = environment["SDL_VIDEO_DRIVER"] = "offscreen"
        environment.pop("DISPLAY", None)
        environment.pop("WAYLAND_DISPLAY", None)
        log = (output / "stdout.log").open("wb")
        process = subprocess.Popen(command, cwd=stage, env=environment, stdout=log,
                                   stderr=subprocess.STDOUT, start_new_session=True)
        deadline = time.monotonic() + args.timeout
        with api.DebugApiClient.connect_unix(args.socket, timeout=120) as session:
            evidence["connected_after_seconds"] = round(args.timeout - (deadline - time.monotonic()), 3)
            evidence["session"] = client_scenario(session, checks, args, output)
            quit_result = session.call("quit")
            checks.check("quit is acknowledged", quit_result is not None)
            checks.check("the server closes the connection at teardown",
                         session.wait_closed(timeout=60))
        try:
            returncode = process.wait(timeout=max(1.0, deadline - time.monotonic()))
        except subprocess.TimeoutExpired:
            returncode = None
        evidence["returncode"] = returncode
        checks.check("the engine exits cleanly after quit", returncode == 0, returncode)
        checks.check("the socket file is removed at shutdown", not Path(args.socket).exists())
    except Exception as error:  # evidence must record every failure
        checks.check("session completed", False, "%s: %s" % (type(error).__name__, error))
    finally:
        if process is not None and process.poll() is None:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait()
        evidence["checks"] = checks.results
        evidence["status"] = "pass" if checks.results and not checks.failures else "fail"
        evidence["finished_utc"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
        (output / "evidence.json").write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print("%d checks, %d failed -> %s" % (len(checks.results), len(checks.failures),
                                          evidence["status"].upper()))
    return 0 if evidence["status"] == "pass" else 1


if __name__ == "__main__":
    sys.exit(main())
