#!/usr/bin/env python3
"""RFC 0001 R12: the dedicated server's installed product checks.

  facts --runtime DIR --out FILE [--maps a,b]
      Runs every map of the runtime's portal/maps on the dedicated server and
      records what the server reads from material definitions: each collision
      surface's and displacement's surface properties (cm_dump_surfaceprops) and
      each loaded model's sprite facts and materials (cm_dump_models).
  facts --listen --profile portal --out FILE [--maps a,b]
      The same recording from a listen server (the Portal client, headless on
      native Vulkan, through portal_boot.py), the reference.
  absence --install DIR [--runtime DIR]
      Fails if the installed product holds or links a render or desktop-UI
      module (file names, DT_NEEDED, defined code), or, with --runtime, if the
      running server loads one (LD_DEBUG=files over a map load).
  lifecycle --runtime DIR
      Installed startup and shutdown: a map loads and the server quits cleanly;
      a level change unloads and reloads; partial failures (a missing physics
      provider, a missing game, a missing map) fail by name without a crash.
  join --runtime DIR --client-runtime DIR [--map M] [--next-map M] [--seconds N]
      A Portal client (null renderer, offscreen) connects to the dedicated
      server and walks forward; the server changes level with it connected;
      status must list the player active on each map, and the server must
      quit cleanly.
  selftest
      Seeded violations each fail the absence checks; a clean tree passes.
  facts-check --runtime DIR --reference FILE [--deviations FILE]
      Records the dedicated runtime's facts and compares them with the
      reference (the conformance row's single command).
  compare REFERENCE CANDIDATE
      Fails unless both name the same maps and every surface, displacement and
      model both loaded has the same facts.

Licensed Portal content is a required local input; a runtime without maps is
unavailable, never passed.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCHEMA = "dedicated-material-facts/v1"
SURFPROP = re.compile(r"^SURFPROP (\S+) (\S+)\s*$", re.M)
SURFPROP_END = re.compile(r"^SURFPROP_END (\d+)\s*$", re.M)
DISPPROP = re.compile(r"^DISPPROP (\d+) (\S+) (\S+)\s*$", re.M)
MODEL = re.compile(r"^MODEL (\S+) (\S+)(.*)$", re.M)
MODELMAT = re.compile(r"^MODELMAT (\S+) (\d+) (\S+)\s*$", re.M)
MODEL_END = re.compile(r"^MODEL_END (\d+)\s*$", re.M)
COMMANDS = ["cm_dump_surfaceprops", "cm_dump_models"]


def run_dedicated(runtime: Path, args: list[str], timeout: int = 180, port: int = 27715):
    """Runs the runtime's dedicated_launcher with `args`; returns (exit code or
    None on timeout, console log text, process output)."""
    runtime = runtime.resolve()
    with tempfile.TemporaryDirectory(prefix="r12-ded-") as scratch:
        console = Path(scratch) / "console.log"
        command = [str(runtime / "dedicated_launcher"), "-game", "portal", "-defaultgamedir", "portal",
                   "-console", "-consolelog", str(console), "-insecure", "-port", str(port), *args]
        environment = dict(os.environ)
        environment["LD_LIBRARY_PATH"] = str(runtime / "bin") + ":" + environment.get("LD_LIBRARY_PATH", "")
        try:
            completed = subprocess.run(command, cwd=runtime, env=environment, input=b"",
                                       capture_output=True, timeout=timeout, check=False)
            code = completed.returncode
            output = completed.stdout.decode(errors="replace") + completed.stderr.decode(errors="replace")
        except subprocess.TimeoutExpired as error:
            code = None
            output = (error.stdout or b"").decode(errors="replace")
        text = console.read_text(errors="replace") if console.is_file() else ""
        return code, text, output


def run_listen(profile: str, name: str):
    """Runs the map on a listen server through portal_boot.py; returns (exit
    code, console log text)."""
    out = Path(tempfile.mkdtemp(prefix="r12-lis-"))
    shutil.rmtree(out)
    command = [sys.executable, str(ROOT / "tools/quality/portal_boot.py"), "--profile", profile, "--headless",
               "--map", name, "--out", str(out), "--timeout", "300"]
    for item in COMMANDS:
        command += ["--console-command", item]
    completed = subprocess.run(command, capture_output=True, text=True, check=False)
    log = out / "runtime/portal/console.log"
    text = log.read_text(errors="replace") if log.is_file() else ""
    shutil.rmtree(out, ignore_errors=True)
    return completed.returncode, text


def parse(text: str) -> dict:
    surfaces = [list(s) for s in SURFPROP.findall(text)]
    end = SURFPROP_END.search(text)
    models = {}
    for name, kind, rest in MODEL.findall(text):
        models[name.lower()] = {"kind": kind, "facts": rest.split(), "materials": []}
    for name, index, material in MODELMAT.findall(text):
        entry = models.get(name.lower())
        if entry is not None:
            entry["materials"].append(material.lower())
    entry = {"surfaces": surfaces, "displacements": [list(d) for d in DISPPROP.findall(text)],
             "models": models}
    model_end = MODEL_END.search(text)
    if not end or int(end.group(1)) != len(surfaces) or not model_end or \
            int(model_end.group(1)) != len(models):
        entry["incomplete"] = True
    return entry


def map_names(runtime: Path) -> list[str]:
    return sorted(p.stem for p in (runtime / "portal" / "maps").glob("*.bsp"))


def record(args) -> int:
    if args.listen:
        source = "listen server (%s, native Vulkan headless)" % args.profile
        maps = args.maps.split(",") if args.maps else None
        if maps is None:
            print("FATAL: --listen needs --maps (or a --runtime to list them)")
            return 2
    else:
        source = "dedicated server (%s)" % args.runtime
        maps = args.maps.split(",") if args.maps else map_names(args.runtime)
    if not maps:
        print("UNAVAILABLE: no maps")
        return 3
    result = {"schema": SCHEMA, "source": source, "maps": {}}
    bad = 0
    for name in maps:
        if args.listen:
            code, text = run_listen(args.profile, name)
        else:
            dump = []
            for item in COMMANDS:
                dump += ["+" + item]
            code, text, _ = run_dedicated(args.runtime, ["+map", name, "+wait", "30", *dump, "+wait", "5",
                                                         "+quit"])
        entry = parse(text)
        entry["exit_code"] = code
        result["maps"][name] = entry
        ok = code == 0 and not entry.get("incomplete")
        bad += not ok
        print("%s %s: %d surfaces, %d displacements, %d models, exit %s" % (
            "ok" if ok else "FAIL", name, len(entry["surfaces"]), len(entry["displacements"]),
            len(entry["models"]), code), flush=True)
    args.out.write_text(json.dumps(result, indent=1) + "\n")
    return 1 if bad else 0


def compare(reference: dict, candidate: dict, deviations: list | None = None) -> int:
    allowed = {(d["map"], d["model"].lower()) for d in deviations or []}
    used = set()
    checks = failures = 0
    for name in sorted(set(reference["maps"]) | set(candidate["maps"])):
        checks += 1
        left, right = reference["maps"].get(name), candidate["maps"].get(name)
        if left is None or right is None or left.get("incomplete") or right.get("incomplete"):
            failures += 1
            print("FAIL %s: missing or incomplete in one recording" % name)
            continue
        problems = []
        if left["surfaces"] != right["surfaces"]:
            diffs = [(l, r) for l, r in zip(left["surfaces"], right["surfaces"]) if l != r]
            problems.append("%d of %d surfaces differ, first %s" % (len(diffs), len(left["surfaces"]),
                                                                   diffs[:1] or "(count differs)"))
        if left["displacements"] != right["displacements"]:
            diffs = [(l, r) for l, r in zip(left["displacements"], right["displacements"]) if l != r]
            problems.append("%d displacements differ, first %s" % (len(diffs), diffs[:1] or "(count differs)"))
        shared = sorted(set(left["models"]) & set(right["models"]))
        for model in shared:
            if left["models"][model] != right["models"][model]:
                if (name, model) in allowed:
                    used.add((name, model))
                    continue
                problems.append("model %s: %s vs %s" % (model, left["models"][model], right["models"][model]))
                break
        if not shared:
            problems.append("no model loaded by both")
        if problems:
            failures += 1
            print("FAIL %s: %s" % (name, "; ".join(problems)))
        else:
            print("ok %s: %d surfaces, %d displacements, %d shared models" % (
                name, len(left["surfaces"]), len(left["displacements"]), len(shared)))
    for stale in sorted(allowed - used):
        checks += 1
        failures += 1
        print("FAIL recorded deviation %s %s no longer occurs: remove it" % stale)
    print("CONFORMANCE %d %d" % (checks, failures))
    return 1 if failures else 0


# RFC 0001 R12: the dedicated product's render and desktop-UI absence.
FORBIDDEN_MODULE = re.compile(
    r"^(?:lib)?(materialsystem|shaderapi\w*|stdshader\w*|studiorender|vgui2|vguimatsurface|inputsystem|"
    r"GameUI|launcher|client|togl\w*|dxvk\w*|SDL\d?\w*|vulkan|GLX?|GLESv\d|EGL|OpenGL|X11\w*|xcb\w*|"
    r"wayland-\w+|gtk-\w+|gdk-\w+|render_\w+|d3d\w*|dxgi|opengl32|vulkan-1)(\.so.*|\.dll)?$", re.I)
FORBIDDEN_CODE = re.compile(
    r"^(CMaterialSystem::|CShaderAPI\w*::|CShaderDevice\w*::|CStudioRender\w*::|CMatSystemSurface::|"
    r"vgui::Panel::|vgui::Frame::|CVGui::|render::(device|graph|frame|renderer|pass|scene)::|RenderCore_\w+$|"
    # The engine's own render units (gl_rsurf, l_studio, r_decal, Overlay, disp*, OcclusionSystem,
    # matsys_interface, gl_shader), which the dedicated product does not build.
    r"(R_DrawWorldLists|R_DecalShoot|R_StudioDrawPoses|Shader_\w+|DispInfo_\w+|InitMaterialSystem|"
    r"MaterialSystem_\w+|R_BrushBatchInit|R_LoadSkys|R_DrawLineFile)\(|CModelRender::|COverlayMgr::|"
    r"COcclusionSystem::|CDispInfo::)")


def elf_files(tree: Path) -> list[Path]:
    """ELF and PE binaries of a tree."""
    found = []
    for path in sorted(tree.rglob("*")):
        if path.is_file() and not path.is_symlink():
            with path.open("rb") as stream:
                magic = stream.read(4)
            if magic == b"\x7fELF" or magic[:2] == b"MZ":
                found.append(path)
    return found


def needed(path: Path) -> list[str]:
    with path.open("rb") as stream:
        pe = stream.read(2) == b"MZ"
    if pe:
        out = subprocess.run(["x86_64-w64-mingw32-objdump", "-p", str(path)], capture_output=True,
                             text=True).stdout
        return re.findall(r"DLL Name: (\S+)", out)
    out = subprocess.run(["readelf", "-d", str(path)], capture_output=True, text=True).stdout
    return re.findall(r"\(NEEDED\)\s+Shared library: \[([^\]]+)\]", out)


def defined_code(path: Path) -> list[str]:
    with path.open("rb") as stream:
        if stream.read(2) == b"MZ":
            return []  # MSVC binaries carry their symbols in PDBs; imports and files are checked
    out = subprocess.run(["nm", "-C", "--defined-only", str(path)], capture_output=True, text=True).stdout
    names = []
    for line in out.splitlines():
        parts = line.split(" ", 2)
        if len(parts) == 3 and parts[1] in "TtDdBbRr" and FORBIDDEN_CODE.match(parts[2]):
            names.append(parts[2])
    return names


def install_violations(install: Path) -> tuple[int, list[str]]:
    checks, errors = 0, []
    for path in sorted(install.rglob("*")):
        if path.is_file():
            checks += 1
            if FORBIDDEN_MODULE.match(path.name):
                errors.append("installs %s" % path.relative_to(install))
    for path in elf_files(install):
        checks += 2
        for lib in needed(path):
            if FORBIDDEN_MODULE.match(lib):
                errors.append("%s links %s" % (path.relative_to(install), lib))
        code = defined_code(path)
        if code:
            errors.append("%s defines %d render/UI symbols, e.g. %s" % (path.relative_to(install), len(code), code[0]))
    return checks, errors


def loaded_objects(trace: str) -> list[str]:
    return sorted(set(re.findall(r"file=(\S+?)\s*\[", trace)) | set(re.findall(r"calling init: (\S+)", trace)))


def runtime_violations(runtime: Path, name: str) -> tuple[int, list[str]]:
    runtime = runtime.resolve()
    environment = dict(os.environ, LD_DEBUG="files")
    environment["LD_LIBRARY_PATH"] = str(runtime / "bin") + ":" + environment.get("LD_LIBRARY_PATH", "")
    command = [str(runtime / "dedicated_launcher"), "-game", "portal", "-defaultgamedir", "portal", "-console",
               "-insecure", "-port", "27716", "+map", name, "+wait", "30", "+quit"]
    completed = subprocess.run(command, cwd=runtime, env=environment, input=b"", capture_output=True, timeout=300)
    trace = completed.stderr.decode(errors="replace") + completed.stdout.decode(errors="replace")
    objects = loaded_objects(trace)
    errors = [] if completed.returncode == 0 else ["server exited %d" % completed.returncode]
    if not objects:
        errors.append("no loaded objects recorded")
    errors += ["loads %s" % o for o in objects if FORBIDDEN_MODULE.match(Path(o).name)]
    return len(objects) + 1, errors


def wine_runtime_violations(runtime: Path, name: str, wineprefix: Path) -> tuple[int, list[str]]:
    runtime = runtime.resolve()
    environment = dict(os.environ, WINEDEBUG="+loaddll", WINEPREFIX=str(wineprefix.resolve()))
    command = ["wine", "dedicated_launcher.exe", "-game", "portal", "-console", "+map", name, "+quit"]
    completed = subprocess.run(command, cwd=runtime, env=environment, input=b"", capture_output=True, timeout=300)
    trace = completed.stderr.decode(errors="replace")
    # Only the product's own modules (loaded from the runtime) are judged.
    root = str(runtime).replace("/", "\\\\").lower()
    objects = sorted({m.group(1) for m in re.finditer(r'Loaded L"([^"]+)"', trace) if root in m.group(1).lower()})
    errors = [] if completed.returncode == 0 else ["server exited %d" % completed.returncode]
    if not objects:
        errors.append("no product modules recorded")
    errors += ["loads %s" % o for o in objects if FORBIDDEN_MODULE.match(o.replace("\\", "/").split("/")[-1])]
    return len(objects) + 1, errors


def absence(args) -> int:
    checks, errors = install_violations(args.install)
    if checks == 0:
        errors.append("empty install tree")
    if args.runtime and args.wineprefix:
        c, e = wine_runtime_violations(args.runtime, args.map, args.wineprefix)
        checks += c
        errors += e
    elif args.runtime:
        c, e = runtime_violations(args.runtime, args.map)
        checks += c
        errors += e
    for error in errors:
        print("FAIL " + error)
    print("CONFORMANCE %d %d" % (checks, len(errors)))
    return 1 if errors else 0


def selftest() -> int:
    checks = failures = 0

    def expect(ok, what):
        nonlocal checks, failures
        checks += 1
        if not ok:
            failures += 1
            print("FAIL selftest: " + what)

    with tempfile.TemporaryDirectory(prefix="r12-self-") as scratch:
        root = Path(scratch)
        stub = root / "stubs"
        stub.mkdir()
        source = root / "a.c"
        source.write_text("int f(void){return 1;}\n")
        for lib in ("libSDL3.so", "libmaterialsystem.so"):
            subprocess.run(["gcc", "-shared", "-fPIC", "-o", str(stub / lib), str(source)], check=True)
        clean = root / "clean"
        (clean / "bin").mkdir(parents=True)
        subprocess.run(["gcc", "-shared", "-fPIC", "-o", str(clean / "bin/libengine.so"), str(source)], check=True)
        expect(not install_violations(clean)[1], "a clean tree passes")
        bad = root / "bad-file"
        shutil.copytree(clean, bad)
        shutil.copy(stub / "libmaterialsystem.so", bad / "bin")
        expect(install_violations(bad)[1], "an installed libmaterialsystem.so fails")
        linked = root / "bad-link"
        shutil.copytree(clean, linked)
        subprocess.run(["gcc", "-shared", "-fPIC", "-o", str(linked / "bin/libdedicated.so"), str(source),
                        "-Wl,--no-as-needed", "-L" + str(stub), "-lSDL3"], check=True)
        expect(any("links libSDL3.so" in e for e in install_violations(linked)[1]), "a DT_NEEDED libSDL3 fails")
        code = root / "bad-code"
        shutil.copytree(clean, code)
        cpp = root / "b.cpp"
        cpp.write_text("struct CMaterialSystem { int Init(); }; int CMaterialSystem::Init(){ return 0; }\n")
        subprocess.run(["g++", "-shared", "-fPIC", "-o", str(code / "bin/libengine.so"), str(cpp)], check=True)
        expect(any("render/UI symbols" in e for e in install_violations(code)[1]), "defined CMaterialSystem code fails")
        engine = root / "bad-engine"
        shutil.copytree(clean, engine)
        cpp.write_text("void R_DrawWorldLists(void *, int, float) {}\n")
        subprocess.run(["g++", "-shared", "-fPIC", "-o", str(engine / "bin/libengine.so"), str(cpp)], check=True)
        expect(any("render/UI symbols" in e for e in install_violations(engine)[1]),
               "a defined engine world renderer fails")
        trace = "      1234:\tfile=/x/bin/libstudiorender.so [0];  needed by /x/bin/libengine.so [0]\n"
        expect(any(FORBIDDEN_MODULE.match(Path(o).name) for o in loaded_objects(trace)), "a loaded studiorender is seen")
        expect(not FORBIDDEN_MODULE.match("libvphysics_box3d.so") and not FORBIDDEN_MODULE.match("libserver.so"),
               "physics and the server game are allowed")
    print("CONFORMANCE %d %d" % (checks, failures))
    return 1 if failures else 0


STATUS_MAP = re.compile(r"^map\s+:\s+(\S+)", re.M)


def lifecycle(args) -> int:
    checks = failures = 0

    def expect(ok, what, detail=""):
        nonlocal checks, failures
        checks += 1
        if not ok:
            failures += 1
            print("FAIL %s %s" % (what, detail))
        else:
            print("ok %s" % what)

    runtime = args.runtime
    code, text, out = run_dedicated(runtime, ["+map", "testchmb_a_00", "+wait", "30", "+status", "+quit"])
    expect(code == 0, "startup, map and quit exit 0", "(exit %s)" % code)
    expect(STATUS_MAP.findall(text) == ["testchmb_a_00"], "status reports the map", STATUS_MAP.findall(text))

    code, text, out = run_dedicated(runtime, ["+map", "testchmb_a_00", "+wait", "30", "+changelevel", "testchmb_a_01",
                                              "+wait", "30", "+status", "+quit"])
    expect(code == 0, "a level change and quit exit 0", "(exit %s)" % code)
    expect(STATUS_MAP.findall(text)[-1:] == ["testchmb_a_01"], "status reports the new map", STATUS_MAP.findall(text))

    code, text, out = run_dedicated(runtime, ["-physics", "vphysics", "+map", "testchmb_a_00", "+wait", "30",
                                              "+status", "+quit"])
    expect(code == 0 and STATUS_MAP.findall(text) == ["testchmb_a_00"], "boots on the IVP fallback provider",
           "(exit %s)" % code)

    code, text, out = run_dedicated(runtime, ["-physics", "vphysics_missing", "+map", "testchmb_a_00", "+quit"])
    expect(code is not None and code > 0, "a missing physics provider fails startup", "(exit %s)" % code)
    expect("Failed to load physics provider" in out + text, "and names the provider")

    code, text, out = run_dedicated(runtime, ["+map", "no_such_map_r12", "+wait", "10", "+status", "+quit"])
    expect(code == 0 and not STATUS_MAP.findall(text), "a missing map leaves the server running, then quits",
           "(exit %s)" % code)

    environment_runtime = runtime.resolve()
    command = [str(environment_runtime / "dedicated_launcher"), "-game", "no_such_game_r12", "-console",
               "-insecure", "-port", "27717", "+quit"]
    env = dict(os.environ)
    env["LD_LIBRARY_PATH"] = str(environment_runtime / "bin")
    try:
        completed = subprocess.run(command, cwd=environment_runtime, env=env, input=b"", capture_output=True,
                                   timeout=120)
        code = completed.returncode
    except subprocess.TimeoutExpired:
        code = None
    expect(code is not None and code >= 0, "a missing game fails without a crash signal", "(exit %s)" % code)
    print("CONFORMANCE %d %d" % (checks, failures))
    return 1 if failures else 0


ACTIVE_PLAYER = re.compile(r"#\s*\d+\s+\"[^\n]*\"[^\n]*\bactive\b")


class Rcon:
    """A minimal Source RCON client (TCP: size, id, type, body, two NULs)."""

    def __init__(self, port: int, password: str):
        import socket
        self.sock = socket.create_connection(("127.0.0.1", port), timeout=10)
        self.next_id = 1
        self.send(3, password)
        # The server answers an auth with an empty response then the result.
        for _ in range(2):
            rid, rtype, _ = self.read()
            if rtype == 2:
                if rid == -1:
                    raise RuntimeError("rcon authentication refused")
                return

    def send(self, kind: int, body: str) -> int:
        import struct
        rid = self.next_id
        self.next_id += 1
        payload = struct.pack("<ii", rid, kind) + body.encode() + b"\0\0"
        self.sock.sendall(struct.pack("<i", len(payload)) + payload)
        return rid

    def read(self):
        import struct

        def exact(n):
            data = b""
            while len(data) < n:
                chunk = self.sock.recv(n - len(data))
                if not chunk:
                    raise ConnectionError("rcon closed")
                data += chunk
            return data
        size = struct.unpack("<i", exact(4))[0]
        data = exact(size)
        rid, rtype = struct.unpack("<ii", data[:8])
        return rid, rtype, data[8:-2].decode(errors="replace")

    def command(self, text: str) -> str:
        import socket
        self.send(2, text)
        out = ""
        self.sock.settimeout(2)
        try:
            while True:
                _, _, body = self.read()
                out += body
        except (socket.timeout, ConnectionError):
            pass
        self.sock.settimeout(10)
        return out


def join(args) -> int:
    import time
    runtime = args.runtime.resolve()
    client = args.client_runtime.resolve()
    checks = failures = 0

    def expect(ok, what, detail=""):
        nonlocal checks, failures
        checks += 1
        failures += 0 if ok else 1
        print("%s %s %s" % ("ok" if ok else "FAIL", what, detail))

    with tempfile.TemporaryDirectory(prefix="r12-join-") as scratch:
        scratch = os.environ.get("R12_JOIN_KEEP", scratch)
        console = Path(scratch) / "server.log"
        port = 27735
        env = dict(os.environ)
        env["LD_LIBRARY_PATH"] = str(runtime / "bin")
        # The dedicated console does not read a pipe; RCON drives the server.
        password = "r12-join"
        server = subprocess.Popen(
            [str(runtime / "dedicated_launcher"), "-game", "portal", "-defaultgamedir", "portal", "-console",
             "-consolelog", str(console), "-insecure", "-ip", "127.0.0.1", "-port", str(port), "-usercon", "+maxplayers", "2",
             # Two processes on one host: without it the engine treats 127.0.0.1
             # as its in-process loopback and never sends the replies.
             "+net_usesocketsforloopback", "1",
             "+rcon_password", password, "+map", args.map],
            cwd=runtime, env=env, stdin=subprocess.DEVNULL, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        player = None
        code = None
        try:
            deadline = time.time() + 120
            while time.time() < deadline and server.poll() is None and not (
                    console.is_file() and "SV_ActivateServer" in console.read_text(errors="replace")):
                time.sleep(1)
            expect(server.poll() is None and console.is_file(), "the server is up")
            cenv = dict(os.environ, SDL_VIDEODRIVER="offscreen", SDL_VIDEO_DRIVER="offscreen", SteamAppId="400",
                        SteamGameId="400")
            cenv.pop("DISPLAY", None)
            cenv.pop("WAYLAND_DISPLAY", None)
            cenv["LD_LIBRARY_PATH"] = str(client / "bin")
            client_log = open(Path(scratch) / "client.out", "wb")
            player = subprocess.Popen(
                [str(client / "hl2_launcher"), "-renderer", "null", "-game", "portal", "-windowed", "-w", "640",
                 "-h", "480", "-multirun", "-novid", "-insecure", "-console", "+net_usesocketsforloopback", "1",
                 "+forward", "+connect", "127.0.0.1:%d" % port],
                cwd=client, env=cenv, stdin=subprocess.DEVNULL, stdout=client_log, stderr=subprocess.STDOUT)
            rcon = Rcon(port, password)

            def wait_active(map_name):
                deadline = time.time() + args.seconds
                while time.time() < deadline:
                    time.sleep(5)
                    text = rcon.command("status")
                    (Path(scratch) / "status.txt").write_text(text)
                    if ACTIVE_PLAYER.search(text) and STATUS_MAP.findall(text) == [map_name]:
                        return text
                return None

            first = wait_active(args.map)
            expect(first is not None, "the client joins and is active on " + args.map)
            # The player walks (+forward): movement, world collision and its
            # surface properties run on the server with no material system.
            time.sleep(15)
            expect(server.poll() is None and player.poll() is None, "server and client still run")
            rcon.command("changelevel " + args.next_map)
            second = wait_active(args.next_map)
            expect(second is not None, "after changelevel the client is active on " + args.next_map)
            rcon.command("quit")
            code = server.wait(timeout=60)
        except Exception as error:  # noqa: BLE001 - reported as a failed check
            expect(False, "the join sequence completes", repr(error))
        finally:
            if server.poll() is None:
                server.kill()
            if player is not None:
                player.kill()
                player.wait()
        expect(code == 0, "the server quits with exit 0", "(exit %s)" % code)
    print("CONFORMANCE %d %d" % (checks, failures))
    return 1 if failures else 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    p = sub.add_parser("facts")
    p.add_argument("--runtime", type=Path)
    p.add_argument("--listen", action="store_true")
    p.add_argument("--profile", default="portal")
    p.add_argument("--out", required=True, type=Path)
    p.add_argument("--maps", default=None)
    p = sub.add_parser("absence")
    p.add_argument("--install", required=True, type=Path)
    p.add_argument("--runtime", type=Path)
    p.add_argument("--map", default="testchmb_a_00")
    p.add_argument("--wineprefix", type=Path, help="a Windows PE runtime: run it under Wine")
    p = sub.add_parser("lifecycle")
    p.add_argument("--runtime", required=True, type=Path)
    p = sub.add_parser("join")
    p.add_argument("--runtime", required=True, type=Path)
    p.add_argument("--client-runtime", required=True, type=Path)
    p.add_argument("--map", default="testchmb_a_01")
    p.add_argument("--next-map", default="testchmb_a_02")
    p.add_argument("--seconds", type=int, default=240)
    sub.add_parser("selftest")
    p = sub.add_parser("facts-check")
    p.add_argument("--runtime", required=True, type=Path)
    p.add_argument("--reference", required=True, type=Path)
    p.add_argument("--deviations", type=Path)
    p = sub.add_parser("compare")
    p.add_argument("reference", type=Path)
    p.add_argument("candidate", type=Path)
    p.add_argument("--deviations", type=Path)
    args = parser.parse_args()
    if args.command == "facts":
        if args.listen and not args.maps and args.runtime:
            args.maps = ",".join(map_names(args.runtime))
        if not args.listen and not args.runtime:
            parser.error("facts needs --runtime or --listen")
        return record(args)
    if args.command == "facts-check":
        with tempfile.TemporaryDirectory(prefix="r12-facts-") as scratch:
            args.out = Path(scratch) / "facts.json"
            args.listen = False
            args.maps = None
            record(args)
            deviations = json.loads(args.deviations.read_text())["deviations"] if args.deviations else None
            return compare(json.loads(args.reference.read_text()), json.loads(args.out.read_text()), deviations)
    if args.command == "absence":
        return absence(args)
    if args.command == "lifecycle":
        return lifecycle(args)
    if args.command == "join":
        return join(args)
    if args.command == "selftest":
        return selftest()
    deviations = json.loads(args.deviations.read_text())["deviations"] if args.deviations else None
    return compare(json.loads(args.reference.read_text()), json.loads(args.candidate.read_text()), deviations)


if __name__ == "__main__":
    sys.exit(main())
