#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""The WebGPU adapter's lanes (RFC 0029 decision 11, gate W3): build
render.device.webgpu and a suite, and run it natively through the pinned Dawn
or as WebAssembly in a headless browser.

    python3 tools/render/webgpu_lane.py run [--out DIR] [--compiler CXX]
        native: the render.device.v2 suite (unittests/rendertest/core/
        device/test_device_webgpu.cpp) built with the host compiler against
        the pinned Dawn release (quality/toolchain/webgpu.json) and run on
        the host GPU through Dawn
    python3 tools/render/webgpu_lane.py browser [--out DIR] [--browser EXE]
        [--timeout SECONDS] [--adapter gpu|swiftshader]
        browser: the same suite built with the pinned Emscripten
        (quality/toolchain/emscripten.json) against the pinned emdawnwebgpu
        package, served cross-origin isolated (threads need
        SharedArrayBuffer) and run in the browser, headless, with WebGPU on
        the host GPU (a fallback to SwiftShader fails the run) or, with
        --adapter swiftshader, on Chrome's CPU Vulkan
    python3 tools/render/webgpu_lane.py fetch
        fetch, verify and extract the pinned Dawn release and Emscripten SDK
        (Waf's --render-core-webgpu links the former)
    python3 tools/render/webgpu_lane.py suite --name NAME [--out DIR]
        [--compiler CXX] [--define D]... SOURCE...
        any suite: SOURCE files (repository paths) compiled with the adapter,
        natively

The generated headers (SPIR-V, GLSL, MSL, HLSL and the WGSL artifacts, with
the core artifact store's table) come from tools/render/shader_artifacts.py
headers; the WGSL form and its contract with the adapter are
shader_artifacts' wgsl_compile, translated by the pinned tint.

Native: Dawn picks its backend (Vulkan on Linux); the adapter it chose is
printed. Browser: the WebAssembly build runs on the browser's main thread
with JSPI, so the adapter's waits (pipeline creation, WaitIdle) and the
suite's waits for tokens yield to the browser's event loop, which delivers
WebGPU's callbacks; encoder threads are Web Workers from a preallocated pool.
The browser is host software, not pinned: its version is printed with every
run. Native results prove the adapter against WebGPU as the pinned Dawn
implements it; browser results against that browser's WebGPU.

Each suite's stdout passes through unchanged (the browser's after the page
reports its exit), so a checks-v1 suite's CONFORMANCE record reaches the
conformance runner.
"""

import argparse
import hashlib
import http.server
import json
import os
import shutil
import subprocess
import sys
import tempfile
import threading
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "render"))
import shader_toolchain as st  # noqa: E402

DEFAULT_OUT = Path(os.environ.get("TMPDIR", "/tmp")) / "webgpu-lane"
DEVICE_SUITE = ["unittests/rendertest/core/device/test_device_webgpu.cpp"]
EMSCRIPTEN_PIN = ROOT / "quality" / "toolchain" / "emscripten.json"
# RFC 0029 decisions 6 and 9: a preallocated worker pool and a fixed heap.
BROWSER_LINK = ["-sJSPI", "-sPTHREAD_POOL_SIZE=8", "-sINITIAL_MEMORY=512MB", "-sEXIT_RUNTIME",
                "-sSTACK_SIZE=1MB"]
BROWSER_FLAGS = ["--headless=new", "--enable-unsafe-webgpu", "--enable-features=Vulkan",
                 "--ignore-gpu-blocklist", "--no-first-run", "--no-default-browser-check"]
# The browser's WebGPU adapter: the host GPU (headless Chrome reaches it
# through ANGLE's Vulkan backend; without it, it falls back to SwiftShader),
# or SwiftShader, Chrome's CPU Vulkan, on purpose.
ADAPTER_FLAGS = {"gpu": ["--use-angle=vulkan"],
                 "swiftshader": ["--use-webgpu-adapter=swiftshader"]}


class LaneError(Exception):
    pass


def run(argv, what, **kwargs):
    result = subprocess.run(argv, capture_output=True, text=True, **kwargs)
    if result.returncode != 0:
        raise LaneError("%s failed (%d): %s" % (what, result.returncode,
                                                (result.stdout + result.stderr).strip()[-4000:]))
    return result


def emsdk():
    """The pinned Emscripten SDK's em++ (quality/toolchain/emscripten.json),
    fetched, verified, installed and activated on first use."""
    pin = json.loads(EMSCRIPTEN_PIN.read_text())
    entry = pin["emsdk"]
    archive = ROOT / "dependencies" / "archives" / entry["archive"]
    if not archive.exists():
        archive.parent.mkdir(parents=True, exist_ok=True)
        run(["curl", "-fsSL", "-o", str(archive), entry["url"]], "download " + entry["archive"])
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    if digest != entry["sha256"]:
        raise LaneError("%s: sha256 %s is not the pin %s" % (archive, digest, entry["sha256"]))
    sdk = ROOT / "dependencies" / entry["directory"]
    stamp = ROOT / "dependencies" / (entry["directory"] + ".pin")
    want = "%s %s" % (digest, pin["version"])
    if not stamp.is_file() or stamp.read_text() != want:
        shutil.rmtree(sdk, ignore_errors=True)
        run(["tar", "-xzf", str(archive), "-C", str(sdk.parent)], "extract " + entry["archive"])
        run([str(sdk / "emsdk"), "install", pin["version"]], "emsdk install", cwd=sdk)
        run([str(sdk / "emsdk"), "activate", pin["version"]], "emsdk activate", cwd=sdk)
        stamp.write_text(want)
    compiler = sdk / "upstream" / "emscripten" / "em++"
    identity = run([str(compiler), "--version"], "em++ --version").stdout.splitlines()[0]
    if identity != pin["emcc_identity"]:
        raise LaneError("%s reports %r; the pin is %r" % (compiler, identity, pin["emcc_identity"]))
    return compiler


def adapter_sources():
    return (sorted(str(p.relative_to(ROOT)) for p in (ROOT / "render/device").glob("*.cpp")) +
            sorted(str(p.relative_to(ROOT)) for p in (ROOT / "render/device/webgpu").glob("*.cpp")))


def generated(out):
    """The generated headers (shader_artifacts headers), rebuilt in place."""
    directory = out / "generated"
    run([sys.executable, str(ROOT / "tools/render/shader_artifacts.py"), "headers", "--out",
         str(directory / "spv")], "shader_artifacts headers")
    return directory


def dependency_times(depfile):
    """Modification times of the files a -MMD dependency file lists (a
    missing file counts as new, so the object rebuilds)."""
    if not depfile.is_file():
        return [float("inf")]
    text = depfile.read_text().replace("\\\n", " ")
    _, _, listed = text.partition(":")
    return [Path(name).stat().st_mtime if Path(name).exists() else float("inf")
            for name in listed.split()]


def build(out, name, sources, compiler, defines=(), browser=False):
    """The native executable, or the browser build's .js (with its .wasm)."""
    out.mkdir(parents=True, exist_ok=True)
    include = generated(out)
    files = list(dict.fromkeys(adapter_sources() + list(sources)))
    objects = out / "obj" / name
    objects.mkdir(parents=True, exist_ok=True)
    flags = ["-std=c++20", "-O2", "-g", "-Wall", "-Wextra", "-Werror", "-pthread",
             "-I" + str(ROOT / "public"), "-I" + str(ROOT),
             "-I" + str(ROOT / "unittests/rendertest/core/device"),
             "-I" + str(include)] + ["-D" + d for d in defines]
    if browser:
        port = st.webgpu_release("emscripten") / "emdawnwebgpu.port.py"
        flags.append("--use-port=" + str(port))
        link = ["--use-port=" + str(port), *BROWSER_LINK]
        target = out / (name + ".js")
    else:
        dawn = st.webgpu_release("linux")
        flags += ["-isystem", str(dawn / "include")]
        link = [str(dawn / "lib64" / "libwebgpu_dawn.a"), "-ldl", "-lm"]
        target = out / name
    flag_text = " ".join([str(compiler)] + flags)
    outputs, running = [], []

    def finish(entry):
        source, obj, stamp, process = entry
        output, _ = process.communicate()
        if process.returncode != 0:
            obj.unlink(missing_ok=True)
            raise LaneError("compile of %s failed: %s" % (source, output.strip()[-4000:]))
        stamp.write_text(flag_text)

    for source in files:
        obj = objects / (source.replace("/", "__") + ".o")
        stamp = obj.with_suffix(".flags")
        stale = (not obj.exists() or not stamp.exists() or stamp.read_text() != flag_text or
                 obj.stat().st_mtime < max([(ROOT / source).stat().st_mtime] +
                                           dependency_times(obj.with_suffix(".d"))))
        if stale:
            running.append((source, obj, stamp, subprocess.Popen(
                [str(compiler), *flags, "-MMD", "-MF", str(obj.with_suffix(".d")), "-c",
                 str(ROOT / source), "-o", str(obj)],
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)))
            if len(running) >= (os.cpu_count() or 4):
                finish(running.pop(0))
        outputs.append(str(obj))
    for entry in running:
        finish(entry)
    run([str(compiler), *outputs, "-pthread", "-o", str(target), *link], "link " + name)
    return target


def execute(exe):
    """Runs the native suite with its stdout passed through; its exit status."""
    print("webgpu_lane: Dawn %s" % json.loads(st.WEBGPU_PIN.read_text())["release"],
          file=sys.stderr)
    return subprocess.run([str(exe)], cwd=ROOT).returncode


# The page the browser build runs in: it forwards the suite's output to the
# lane while it runs and its exit status when it ends.
PAGE = """<!doctype html>
<html><head><meta charset="utf-8"><title>render.device.webgpu</title></head><body>
<script>
const lines = [];
let sent = 0;
function flush() {
  if (sent < lines.length) {
    const chunk = lines.slice(sent);
    sent = lines.length;
    fetch('/log', { method: 'POST', body: chunk.join('\\n') + '\\n' });
  }
}
setInterval(flush, 250);
function finish(status) {
  flush();
  fetch('/exit', { method: 'POST', body: String(status) });
}
var Module = {
  print: (text) => lines.push(text),
  printErr: (text) => lines.push('[stderr] ' + text),
  onExit: (code) => finish(code),
  onAbort: (what) => { lines.push('[abort] ' + what); finish(134); },
};
addEventListener('error', (e) => { lines.push('[error] ' + e.message); finish(135); });
addEventListener('unhandledrejection', (e) => { lines.push('[rejection] ' + e.reason); finish(135); });
if (!navigator.gpu)
  { lines.push('[error] this browser has no navigator.gpu'); finish(136); }
</script>
<script src="%s"></script>
</body></html>
"""


class Page(http.server.SimpleHTTPRequestHandler):
    """Serves the build cross-origin isolated (SharedArrayBuffer, RFC 0029
    decision 6) and takes the page's output and exit status."""
    log = None
    status = None
    done = None

    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def log_message(self, *args):
        pass

    def do_POST(self):
        body = self.rfile.read(int(self.headers.get("Content-Length", 0)))
        if self.path == "/log":
            with open(Page.log, "ab") as out:
                out.write(body)
        elif self.path == "/exit":
            Page.status = body.decode(errors="replace").strip()
            Page.done.set()
        self.send_response(200)
        self.end_headers()


def browse(script, browser, timeout, adapter):
    """Serves the build, runs it in the browser headless on the adapter, and
    prints its output; the page's exit status (2 when it never reported one,
    3 when a GPU run fell back to SwiftShader)."""
    directory = script.parent
    (directory / "index.html").write_text(PAGE % script.name)
    Page.log = directory / "page.log"
    Page.log.write_bytes(b"")
    Page.status = None
    Page.done = threading.Event()
    Page.extensions_map = dict(Page.extensions_map, **{".wasm": "application/wasm",
                                                        ".js": "text/javascript"})

    def handler(*args, **kwargs):
        return Page(*args, directory=str(directory), **kwargs)
    server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), handler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    version = subprocess.run([browser, "--version"], capture_output=True, text=True).stdout.strip()
    print("webgpu_lane: %s; Emscripten %s; emdawnwebgpu %s" % (
        version, json.loads(EMSCRIPTEN_PIN.read_text())["version"],
        json.loads(st.WEBGPU_PIN.read_text())["release"]), file=sys.stderr)
    profile = tempfile.mkdtemp(prefix="webgpu-lane-browser-")
    url = "http://127.0.0.1:%d/index.html" % server.server_address[1]
    process = subprocess.Popen([browser, *BROWSER_FLAGS, *ADAPTER_FLAGS[adapter],
                                "--user-data-dir=" + profile, url],
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                               start_new_session=True)
    try:
        # The page's exit, or the browser's (a browser that dies before the
        # page reports fails at once, not at the timeout).
        deadline = time.monotonic() + timeout
        finished = False
        while not finished and time.monotonic() < deadline:
            finished = Page.done.wait(1.0)
            if not finished and process.poll() is not None:
                print("webgpu_lane: the browser exited (%s) before the page finished"
                      % process.returncode, file=sys.stderr)
                break
    finally:
        time.sleep(0.5)  # the page's last /log
        process.terminate()
        try:
            process.wait(10)
        except subprocess.TimeoutExpired:
            process.kill()
        server.shutdown()
        shutil.rmtree(profile, ignore_errors=True)
    output = Page.log.read_text(errors="replace")
    sys.stdout.write(output)
    sys.stdout.flush()
    adapters = [line for line in output.splitlines() if line.startswith("webgpu adapter:")]
    if adapter == "gpu" and any("swiftshader" in line.lower() for line in adapters):
        print("webgpu_lane: the GPU run fell back to SwiftShader: %s" % adapters[0],
              file=sys.stderr)
        return 3
    if not finished:
        print("webgpu_lane: the page did not finish", file=sys.stderr)
        return 2
    try:
        return int(Page.status)
    except (TypeError, ValueError):
        return 2


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("fetch")
    run_parser = commands.add_parser("run")
    run_parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    run_parser.add_argument("--compiler", default=os.environ.get("CXX", "g++"))
    browser_parser = commands.add_parser("browser")
    browser_parser.add_argument("--out", type=Path, default=DEFAULT_OUT / "browser")
    browser_parser.add_argument("--browser", default="google-chrome")
    browser_parser.add_argument("--timeout", type=int, default=600)
    browser_parser.add_argument("--adapter", choices=sorted(ADAPTER_FLAGS), default="gpu")
    suite_parser = commands.add_parser("suite")
    suite_parser.add_argument("--name", required=True)
    suite_parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    suite_parser.add_argument("--compiler", default=os.environ.get("CXX", "g++"))
    suite_parser.add_argument("--define", action="append", default=[])
    suite_parser.add_argument("sources", nargs="+")
    args = parser.parse_args(argv)
    try:
        if args.command == "fetch":
            print(st.webgpu_release("linux"))
            print(st.webgpu_release("emscripten"))
            print(emsdk())
            return 0
        if args.command == "browser":
            script = build(args.out, "test_device_webgpu", DEVICE_SUITE, emsdk(), browser=True)
            return browse(script, args.browser, args.timeout, args.adapter)
        if args.command == "run":
            exe = build(args.out, "test_device_webgpu", DEVICE_SUITE, args.compiler)
        else:
            exe = build(args.out, args.name, args.sources, args.compiler, args.define)
    except (LaneError, st.ToolchainError) as error:
        print("webgpu_lane: %s" % error, file=sys.stderr)
        return 2
    return execute(exe)


if __name__ == "__main__":
    sys.exit(main())
