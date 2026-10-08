#!/usr/bin/env python3
"""Serves the WebAssembly product to a browser (RFC 0029).

    serve.py --build <dir with hl2_launcher.js/.wasm> --content <game tree> [--port N]
             [--out DIR] [--firefox-profile DIR] [-- engine args]

The page (tools/web/site) runs the engine on the browser's main thread with
JSPI and WebGPU. The server sends what the page needs and nothing else:

- every response cross-origin isolated (COOP same-origin, COEP require-corp),
  which SharedArrayBuffer and so the engine's threads need;
- /engine/<file>: the build's loader and module;
- /content/<path>: the game tree (symlinks followed), with byte ranges, which
  the page's lazy files read in chunks; /content/manifest.json lists every
  file and its size (ELF programs and libraries left out: the browser runs
  none of them);
- POST /log, /exit and /file/<name>: the page's console, exit status and
  files it hands back (the harness's captures), into --out.

- /: the page itself, redirected to carry the engine's arguments when they
  were given here.

--firefox-profile makes DIR a fresh Firefox profile with WebGPU and
JavaScript promise integration (JSPI) on, before the server listens: the
browser-page run provider starts the browser only once it does.

It ends when the page reports the engine's exit, with that status. `kiln play`
runs it and the browser as the `browser-page` run provider's two peers, in the
display session the launch selects (the user's, or kiln's private headless
compositor); the browser lane (tools/web/browser_lane.py) is that launch.
"""

import argparse
import http.server
import json
import os
import re
import shutil
import sys
import tempfile
import urllib.parse
import threading
from pathlib import Path

SITE = Path(__file__).resolve().parent / "site"
TYPES = {".wasm": "application/wasm", ".js": "text/javascript", ".html": "text/html",
         ".json": "application/json", ".css": "text/css"}
# Loose files at most this size are fetched whole at startup (they are also the
# ones the engine writes: configs, logs); larger ones are read lazily.
EAGER_BYTES = 256 * 1024


def is_elf(path):
    try:
        with open(path, "rb") as f:
            return f.read(4) == b"\x7fELF"
    except OSError:
        return True


def manifest(content):
    """{"files": [[path, size], ...], "eager": bytes}: the tree's regular files."""
    files = []
    for root, dirs, names in os.walk(content, followlinks=True):
        dirs[:] = sorted(d for d in dirs if d not in ("screenshots", "save", "logs", "dumps"))
        for name in sorted(names):
            path = Path(root) / name
            try:
                size = path.stat().st_size
            except OSError:
                continue
            if not path.is_file() or is_elf(path) or name.endswith((".so", ".dmp")):
                continue
            files.append([str(path.relative_to(content)), size])
    return {"files": files, "eager": EAGER_BYTES}


class Server(http.server.ThreadingHTTPServer):
    daemon_threads = True

    def __init__(self, address, build, content, out, engine_args=None):
        super().__init__(address, Handler)
        self.engine_args = engine_args or []
        self.build = Path(build)
        self.content = Path(content)
        self.out = Path(out) if out else None
        self.manifest = json.dumps(manifest(self.content)).encode()
        self.status = None
        self.done = threading.Event()
        if self.out:
            self.out.mkdir(parents=True, exist_ok=True)
            (self.out / "page.log").write_bytes(b"")


class Handler(http.server.BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"

    def log_message(self, *args):
        pass

    def headers_common(self, length, kind):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cross-Origin-Resource-Policy", "same-origin")
        self.send_header("Cache-Control", "no-store")
        self.send_header("Accept-Ranges", "bytes")
        self.send_header("Content-Type", kind)
        self.send_header("Content-Length", str(length))
        self.end_headers()

    def fail(self, code):
        self.send_response(code)
        self.headers_common(0, "text/plain")

    def resolve(self):
        path = self.path.split("?", 1)[0]
        if path in ("/", "/index.html"):
            return SITE / "index.html"
        if path.startswith("/site/"):
            return SITE / path[len("/site/"):]
        if path.startswith("/engine/"):
            return self.server.build / Path(path[len("/engine/"):]).name
        if path.startswith("/content/"):
            relative = Path(path[len("/content/"):])
            if ".." in relative.parts:
                return None
            return self.server.content / relative
        return None

    def do_HEAD(self):
        self.do_GET(body=False)

    def do_GET(self, body=True):
        if self.path == "/" and self.server.engine_args:
            # The page reads its arguments from the query; the harness flag
            # makes it post its console, captures and exit status here.
            self.send_response(302)
            # SOURCE_WEB_PAGE_INPUT: the page's scripted input (site/engine.js
            # `input`), for the harnesses' input checks.
            script = os.environ.get("SOURCE_WEB_PAGE_INPUT", "")
            self.send_header("Location", "/?harness=1&args=" +
                             urllib.parse.quote(" ".join(self.server.engine_args)) +
                             ("&input=" + urllib.parse.quote(script) if script else ""))
            self.headers_common(0, "text/plain")
            return
        if self.path.split("?", 1)[0] == "/content/manifest.json":
            data = self.server.manifest
            self.send_response(200)
            self.headers_common(len(data), "application/json")
            if body:
                self.wfile.write(data)
            return
        path = self.resolve()
        if path is None or not path.is_file():
            return self.fail(404)
        size = path.stat().st_size
        kind = TYPES.get(path.suffix, "application/octet-stream")
        # pad=1: one zero byte before the body, so the page's synchronous text
        # read never starts with a byte-order mark (site/engine.js fetchRange).
        query = urllib.parse.parse_qs(urllib.parse.urlsplit(self.path).query)
        pad = b"\0" if query.get("pad") == ["1"] else b""
        ranged = re.match(r"bytes=(\d+)-(\d*)$", self.headers.get("Range", ""))
        start, end = 0, size - 1
        if ranged:
            start = int(ranged.group(1))
            end = min(int(ranged.group(2)) if ranged.group(2) else size - 1, size - 1)
            if start > end:
                return self.fail(416)
            self.send_response(206)
            self.send_header("Content-Range", "bytes %d-%d/%d" % (start, end, size))
        else:
            self.send_response(200)
        self.headers_common(end - start + 1 + len(pad), kind)
        if not body:
            return
        self.wfile.write(pad)
        with open(path, "rb") as f:
            f.seek(start)
            left = end - start + 1
            while left:
                chunk = f.read(min(left, 1 << 20))
                if not chunk:
                    break
                self.wfile.write(chunk)
                left -= len(chunk)

    def do_POST(self):
        data = self.rfile.read(int(self.headers.get("Content-Length", 0)))
        out = self.server.out
        if self.path == "/log":
            sys.stderr.buffer.write(data)  # the engine's console, in kiln's log
            sys.stderr.flush()
            if out:
                with open(out / "page.log", "ab") as f:
                    f.write(data)
        elif out and self.path.startswith("/file/"):
            (out / Path(self.path[len("/file/"):]).name).write_bytes(data)
        elif self.path == "/exit":
            self.server.status = data.decode(errors="replace").strip()
            self.server.done.set()
        self.send_response(200)
        self.headers_common(0, "text/plain")


def start(build, content, out=None, port=0, engine_args=None):
    """A running server (in a daemon thread) and its base URL."""
    server = Server(("127.0.0.1", port), build, content, out, engine_args)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    return server, "http://127.0.0.1:%d/" % server.server_address[1]


FIREFOX_PREFS = {
    "dom.webgpu.enabled": True,
    "gfx.webgpu.ignore-blocklist": True,
    "javascript.options.wasm_js_promise_integration": True,
    "browser.shell.checkDefaultBrowser": False,
    "browser.aboutwelcome.enabled": False,
    "datareporting.policy.dataSubmissionEnabled": False,
    "toolkit.telemetry.reportingpolicy.firstRun": False,
    # No xdg-desktop-portal: kiln's private session blocks the document
    # portal, and Firefox would retry the others in a tight loop.
    "widget.use-xdg-desktop-portal.settings": 0,
    "widget.use-xdg-desktop-portal.file-picker": 0,
    "widget.use-xdg-desktop-portal.mime-handler": 0,
    "widget.use-xdg-desktop-portal.location": 0,
    "widget.use-xdg-desktop-portal.open-uri": 0,
    "widget.use-xdg-desktop-portal.native-messaging": 0,
    # The page's console (WebGPU validation messages among it) in the
    # browser's output, which kiln logs.
    "devtools.console.stdout.content": True,
}


def fresh_firefox_profile(directory):
    """A new, empty Firefox profile holding only the page's preferences."""
    directory = Path(directory)
    shutil.rmtree(directory, ignore_errors=True)
    directory.mkdir(parents=True)
    with open(directory / "user.js", "w") as f:
        for name, value in FIREFOX_PREFS.items():
            f.write("user_pref(%s, %s);\n" % (json.dumps(name), json.dumps(value)))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--build", required=True, help="directory with hl2_launcher.js and .wasm")
    parser.add_argument("--content", required=True, help="the game tree the page mounts")
    parser.add_argument("--port", type=int, default=8080)
    parser.add_argument("--out", help="where the page's log, exit status and files go")
    parser.add_argument("--firefox-profile", help="make this a fresh Firefox profile first")
    parser.add_argument("engine_args", nargs="*", help="the engine's arguments (after --)")
    args = parser.parse_args(argv)
    if args.firefox_profile:
        fresh_firefox_profile(args.firefox_profile)
    out = args.out or tempfile.mkdtemp(prefix="source-web-")
    server, url = start(args.build, args.content, out, args.port, args.engine_args)
    print("serve: %s (content %s, %d files; page output in %s)" % (url, args.content,
          len(json.loads(server.manifest)["files"]), out), file=sys.stderr)
    try:
        server.done.wait()
    except KeyboardInterrupt:
        return 130
    server.shutdown()
    try:
        return int(server.status)
    except (TypeError, ValueError):
        return 2


if __name__ == "__main__":
    sys.exit(main())
