#!/usr/bin/env python3
"""Runs the WebAssembly product in headless Chrome (RFC 0029 W2/W5 lane).

    browser_lane.py --build <dir> --content <game tree> --out <dir>
                    [--adapter gpu|swiftshader] [--timeout S] [-- engine args]

The page (tools/web/site) mounts the game tree from the server
(tools/web/serve.py), runs the engine with the arguments given, and posts its
console, the captures the engine writes (-pica_capture_path) and its exit
status into --out. Prints the console; exits with the page's status (2 when it
never reported one).
"""

import argparse
import shutil
import subprocess
import sys
import tempfile
import time
import urllib.parse
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "render"))
import serve  # noqa: E402
import webgpu_lane  # noqa: E402

DEFAULT_ARGS = ["-game", "portal", "-novid", "-noip", "-w", "1280", "-h", "720",
                "+map", "testchmb_a_01"]


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--build", required=True)
    parser.add_argument("--content", required=True)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--browser", default="google-chrome")
    parser.add_argument("--adapter", choices=sorted(webgpu_lane.ADAPTER_FLAGS), default="gpu")
    parser.add_argument("--timeout", type=int, default=600)
    parser.add_argument("engine_args", nargs="*")
    args = parser.parse_args(argv)
    engine = args.engine_args or DEFAULT_ARGS
    server, base = serve.start(args.build, args.content, args.out)
    url = base + "?harness=1&args=" + urllib.parse.quote(" ".join(engine))
    profile = tempfile.mkdtemp(prefix="browser-lane-")
    version = subprocess.run([args.browser, "--version"], capture_output=True,
                             text=True).stdout.strip()
    print("browser_lane: %s, %s" % (version, url), file=sys.stderr)
    process = subprocess.Popen([args.browser, *webgpu_lane.BROWSER_FLAGS,
                                *webgpu_lane.ADAPTER_FLAGS[args.adapter],
                                "--user-data-dir=" + profile, url],
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                               start_new_session=True)
    finished = False
    try:
        deadline = time.monotonic() + args.timeout
        while not finished and time.monotonic() < deadline:
            finished = server.done.wait(1.0)
            if not finished and process.poll() is not None:
                print("browser_lane: the browser exited (%s) first" % process.returncode,
                      file=sys.stderr)
                break
    finally:
        time.sleep(1.0)  # the page's last posts
        process.terminate()
        try:
            process.wait(10)
        except subprocess.TimeoutExpired:
            process.kill()
        server.shutdown()
        shutil.rmtree(profile, ignore_errors=True)
    sys.stdout.write((args.out / "page.log").read_text(errors="replace"))
    if not finished:
        print("browser_lane: the page did not finish", file=sys.stderr)
        return 2
    try:
        return int(server.status)
    except (TypeError, ValueError):
        return 2


if __name__ == "__main__":
    sys.exit(main())
