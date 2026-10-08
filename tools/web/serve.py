#!/usr/bin/env python3
"""Serves the WebAssembly product to a browser (RFC 0029).

    serve.py --build <dir with hl2_launcher.js/.wasm> --content <game tree> [--port N]

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

The same server serves an interactive session (`kiln play`) and the browser
lane's headless run (tools/web/browser_lane.py).
"""

import argparse
import http.server
import json
import os
import re
import sys
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

    def __init__(self, address, build, content, out):
        super().__init__(address, Handler)
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
        self.headers_common(end - start + 1, kind)
        if not body:
            return
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
        if out and self.path == "/log":
            with open(out / "page.log", "ab") as f:
                f.write(data)
        elif out and self.path.startswith("/file/"):
            (out / Path(self.path[len("/file/"):]).name).write_bytes(data)
        elif self.path == "/exit":
            self.server.status = data.decode(errors="replace").strip()
            self.server.done.set()
        self.send_response(200)
        self.headers_common(0, "text/plain")


def start(build, content, out=None, port=0):
    """A running server (in a daemon thread) and its base URL."""
    server = Server(("127.0.0.1", port), build, content, out)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    return server, "http://127.0.0.1:%d/" % server.server_address[1]


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--build", required=True, help="directory with hl2_launcher.js and .wasm")
    parser.add_argument("--content", required=True, help="the game tree the page mounts")
    parser.add_argument("--port", type=int, default=8080)
    parser.add_argument("--out", help="where the page's log, exit status and files go")
    args = parser.parse_args(argv)
    server, url = start(args.build, args.content, args.out, args.port)
    print("serve: %s (content %s, %d files)" % (url, args.content,
          len(json.loads(server.manifest)["files"])), file=sys.stderr)
    try:
        server.done.wait()
    except KeyboardInterrupt:
        pass
    return 0


if __name__ == "__main__":
    sys.exit(main())
