#!/usr/bin/env python3
"""Runs the WebAssembly product in a real browser (RFC 0029 W2/W5 lane).

    browser_lane.py [--profile portal-wasm32-webgpu] [--flavor dev]
                    [--display private] [--map MAP] [--set SWITCH]...
                    [--timeout S] [--out DIR] [-- engine args]

The lane is the profile's own launch through kiln.api: the `browser-page`
run provider starts the page server (tools/web/serve.py) and the browser on
the GPU, in the display session given (by default kiln's private headless Wayland
compositor, so no window reaches the user's desktop; Firefox on Wayland, no
Xwayland). The page
posts the engine's console, the captures it writes (-core_capture_path) and
its exit status to the server, which writes them to the profile's web
session directory; the lane copies them to --out, prints the console and
exits with the engine's status (2 when the run never reported one).
"""

import argparse
import os
import shutil
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "kiln"))
import sepipe_loader  # noqa: E402


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--profile", default="portal-wasm32-webgpu")
    parser.add_argument("--flavor", default="dev")
    parser.add_argument("--display", default="private")
    parser.add_argument("--map")
    parser.add_argument("--set", dest="switches", action="append", default=[])
    parser.add_argument("--timeout", type=int, default=900)
    parser.add_argument("--out", type=Path)
    parser.add_argument("engine_args", nargs="*")
    args = parser.parse_args(argv)

    # Kiln's private session has no path to the desktop's audio server: the
    # page's audio plays into the browser's null sink (serve.py).
    os.environ.setdefault("SOURCE_WEB_NULL_AUDIO", "1")
    tree = ROOT / "out" / args.profile / args.flavor
    session_dir = tree / "web-session"  # the profile's {web_out}
    shutil.rmtree(session_dir, ignore_errors=True)
    options = dict(flavor=args.flavor, display=args.display, switches=args.switches,
                   arguments=list(args.engine_args), log=str(tree / "web-lane.log"))
    if args.map:
        options["map"] = args.map
    run = sepipe_loader.Run(sepipe_loader.load(), sepipe_loader.session(), "run",
                            args.profile, **options)
    deadline = time.monotonic() + args.timeout
    while run.poll() is None and time.monotonic() < deadline:
        time.sleep(0.5)
    timed_out = run.poll() is None
    if timed_out:
        run.stop()
    log = session_dir / "page.log"
    if log.exists():
        sys.stdout.write(log.read_text(errors="replace"))
    if args.out:
        args.out.mkdir(parents=True, exist_ok=True)
        if session_dir.is_dir():
            for item in session_dir.iterdir():
                shutil.copy2(item, args.out / item.name)
    if run.error:
        print("browser_lane: %s" % run.error, file=sys.stderr)
    if timed_out:
        print("browser_lane: the page did not finish within %d s" % args.timeout,
              file=sys.stderr)
        return 2
    return run.returncode if run.returncode is not None else 2


if __name__ == "__main__":
    sys.exit(main())
