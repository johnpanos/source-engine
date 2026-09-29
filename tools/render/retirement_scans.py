#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Static scans for the RFC 0016 inversion and retirement gates.

    python3 tools/render/retirement_scans.py side-channels   [--root DIR]
    python3 tools/render/retirement_scans.py record-replay   [--root DIR]
    python3 tools/render/retirement_scans.py legacy-stream   [--root DIR] [--write]
    python3 tools/render/retirement_scans.py dead-code       [--root DIR]
    python3 tools/render/retirement_scans.py legacy-freeze   [--root DIR] [--write]
    python3 tools/render/retirement_scans.py sensitivity

  side-channels  K3 "Side channels gone": no first-party source names the
                 WorldMeshUpload007, RenderLightSetConsumer001 or
                 RenderGpuCompute001 lookups (the strings or the constants
                 that hold them), and materialsystem/render_capability_queue.*
                 is deleted.
  record-replay  K3 "Record replay gone": CVulkanContext's BeginFrame and
                 EndFrame, which replayed the frame's recorded draw stream
                 into the context's own command buffer at present, have no
                 caller and no definition (frames record into the render
                 core's port encoders).
  legacy-stream  K9 "First-party use is zero", static half: every first-party
                 site that acquires the legacy stream (CMatRenderContextPtr,
                 GetRenderContext()) is counted per file against the reviewed
                 ratchet tools/render/legacy_stream_ratchet.json. A new file,
                 a higher count, or a stale entry (fewer sites than recorded)
                 fails, so the list stays exact and only shrinks; --write
                 records the current counts after review. The gate itself
                 passes only at zero, which the check reports separately.
  dead-code      K9 "Dead code removed", static half: the legacy SPIR-V
                 sources in materialsystem/shaderapivulkan/shaders/ are gone.
  legacy-freeze  RFC 0016 binding rule 1, "Freeze ratchet": the frozen legacy
                 render paths gain nothing. Recorded exactly in
                 tools/render/legacy_freeze_ratchet.json and compared:
                 - the files of materialsystem/shaderapivulkan/shaders/ and
                   each one's line count (a new file or more lines fails);
                 - the source files of materialsystem/stdshaders/ (a new
                   shader file fails);
                 - the interface names the frozen backends answer in
                   QueryInterface;
                 - the functions defined in engine/gl_lightmap.cpp and
                   engine/lightcache.cpp (the CPU runtime-lighting path);
                 - the ConVars, console commands and launch switches defined
                   in the frozen paths.
                 Anything recorded that disappeared must be recorded too
                 (the ratchet stays exact). A change that grows a set is a
                 Frozen-path commit (defect fix or explicit user request)
                 that rewrites the ratchet with --write in the same commit.
  sensitivity    seeded faults in private temporary trees must each be
                 rejected by the scan they target, and a clean tree accepted.

Before K3 and K9 close, side-channels, record-replay and dead-code fail on
this tree; the conformance rows record that as their expected outcome, so
a change in either direction is noticed. Comments and string literals are
stripped before matching code identifiers; the side-channel strings are
matched in literals too. Each subcommand ends with one checks-v1 record
(CONFORMANCE n f). Python 3 standard library only.
"""

import argparse
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
from conformance_result import Checks  # noqa: E402

SCHEMA = "render-legacy-stream-ratchet/v1"
RATCHET = "tools/render/legacy_stream_ratchet.json"
EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".inl", ".mm"}
SKIPPED_ROOTS = ("box3d/", "ivp/", "lib/", "thirdparty/", "dependencies/", "build")

SIDE_CHANNEL_STRINGS = ("WorldMeshUpload007", "RenderLightSetConsumer001", "RenderGpuCompute001")
SIDE_CHANNEL_NAMES = re.compile(
    r"\b(kWorldMeshUploadInterface|kLightSetConsumerInterface|kGpuComputeInterface)\b")
CAPABILITY_QUEUE = ("materialsystem/render_capability_queue.cpp",
                    "materialsystem/render_capability_queue.h")
# The replay's old entry points: a caller, or the functions themselves.
RECORD_REPLAY_CALL = re.compile(
    r"\bg_VulkanContext\s*\.\s*BeginFrame\s*\(|\bCVulkanContext::(?:BeginFrame|EndFrame)\b")
LEGACY_ACQUIRE = re.compile(r"\bCMatRenderContextPtr\b|\bGetRenderContext\s*\(")
# First-party callers of the legacy stream (RFC 0016 "Compatibility surface").
# The material system, the shader API and the stdshader DLLs implement the
# stream rather than call it, and legacy tools and Hammer's MFC shell retire
# with their own rows, so they are not in the ratchet.
FIRST_PARTY = ("game/", "engine/", "vgui2/", "vguimatsurface/", "studiorender/", "particles/",
               "datacache/", "launcher/", "gameui/")
LEGACY_SHADERS = "materialsystem/shaderapivulkan/shaders/"
FREEZE_SCHEMA = "render-legacy-freeze-ratchet/v1"
FREEZE_RATCHET = "tools/render/legacy_freeze_ratchet.json"
# RFC 0016 binding rule 1: the frozen legacy render paths.
FROZEN_DIRS = ("materialsystem/shaderapivulkan/", "materialsystem/shaderapidx9/",
               "materialsystem/stdshaders/")
FROZEN_LIGHTING = ("engine/gl_lightmap.cpp", "engine/lightcache.cpp")
STDSHADERS = "materialsystem/stdshaders/"
STDSHADER_EXTENSIONS = {".cpp", ".h", ".fxc", ".vsh", ".psh", ".inc"}
CONVAR_DEF = re.compile(r'\bConVar\s+\w+\s*\(\s*"([^"]+)"')
CONCOMMAND_DEF = re.compile(r'\b(?:CON_COMMAND(?:_F)?|ConCommand\s+\w+)\s*\(\s*"?(\w+)"?')
SWITCH_USE = re.compile(r'\b(?:FindParm|CheckParm|ParmValue|HasParm)\s*\(\s*"(-[\w.-]+)"')
INTERFACE_ANSWER = re.compile(r'\(\s*pInterfaceName\s*,\s*("[^"]+"|[A-Za-z_]\w*)\s*\)')
# A function definition starting at column 0 (Source style): a return type,
# the (possibly qualified) name, the parameter list, then a brace, possibly
# on a later line. Declarations end in ';' and are skipped.
FUNCTION_DEF = re.compile(
    r"^(?:[A-Za-z_][\w:<>,\*&]*[\s\*&]+)+\**([A-Za-z_~][\w:~]*)\s*\(([^;{}]*?)\)\s*(?:const\s*)?\{",
    re.M)


def sources(root):
    """First-party C/C++ files: tracked plus untracked, non-ignored ones."""
    root = Path(root)
    if (root / ".git").exists():
        listed = subprocess.run(
            ["git", "-C", str(root), "ls-files", "--cached", "--others", "--exclude-standard"],
            capture_output=True, text=True, check=True).stdout.splitlines()
    else:
        listed = [p.relative_to(root).as_posix() for p in root.rglob("*") if p.is_file()]
    out = []
    for path in listed:
        if path.startswith(SKIPPED_ROOTS) or Path(path).suffix not in EXTENSIONS:
            continue
        if (root / path).is_file():
            out.append(path)
    return sorted(set(out))


def strip_code(text):
    """Text with comments and string/char literals blanked, lines kept."""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("\n" * text.count("\n", i, j))
            i = j
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == "\\" else 1
            out.append(c + c)
            i = j + 1
        else:
            out.append(c)
            i += 1
    return "".join(out)


def read(root, path):
    return (Path(root) / path).read_text(encoding="utf-8", errors="replace")


def line_of(text, index):
    return text.count("\n", 0, index) + 1


def scan_side_channels(root):
    hits = []
    for path in sources(root):
        text = read(root, path)
        for literal in SIDE_CHANNEL_STRINGS:
            for match in re.finditer(re.escape('"%s"' % literal), text):
                hits.append("%s:%d: the %s lookup string" % (path, line_of(text, match.start()),
                                                              literal))
        code = strip_code(text)
        for match in SIDE_CHANNEL_NAMES.finditer(code):
            hits.append("%s:%d: %s" % (path, line_of(code, match.start()), match.group(1)))
    present = [p for p in CAPABILITY_QUEUE if (Path(root) / p).exists()]
    return hits, present


def scan_record_replay(root):
    hits = []
    for path in sources(root):
        code = strip_code(read(root, path))
        for match in RECORD_REPLAY_CALL.finditer(code):
            hits.append("%s:%d" % (path, line_of(code, match.start())))
    return hits


def legacy_stream_counts(root):
    counts = {}
    for path in sources(root):
        if not path.startswith(FIRST_PARTY):
            continue
        found = len(LEGACY_ACQUIRE.findall(strip_code(read(root, path))))
        if found:
            counts[path] = found
    return counts


def load_ratchet(root):
    path = Path(root) / RATCHET
    if not path.exists():
        return None
    data = json.loads(path.read_text())
    if data.get("schema") != SCHEMA:
        raise ValueError("%s: schema must be %s" % (RATCHET, SCHEMA))
    return data


def check_side_channels(root, checks):
    hits, present = scan_side_channels(root)
    for hit in hits[:40]:
        print("side-channel:", hit)
    checks.check(not hits, "k3.side-channels.no-lookup",
                 "%d side-channel lookup site(s) remain" % len(hits))
    checks.check(not present, "k3.side-channels.capability-queue-deleted",
                 "still present: %s" % ", ".join(present))


def check_record_replay(root, checks):
    hits = scan_record_replay(root)
    for hit in hits:
        print("record-replay caller:", hit)
    checks.check(not hits, "k3.record-replay.no-caller",
                 "%d use(s) of CVulkanContext::BeginFrame/EndFrame remain" % len(hits))


def check_legacy_stream(root, checks, write=False):
    counts = legacy_stream_counts(root)
    if write:
        data = {"schema": SCHEMA,
                "note": "RFC 0016 K9 static ratchet: first-party sites acquiring the legacy stream "
                        "(CMatRenderContextPtr, GetRenderContext()) per file. Only shrinks; "
                        "rewrite with `retirement_scans.py legacy-stream --write` after review.",
                "files": counts}
        (Path(root) / RATCHET).write_text(json.dumps(data, indent=2, sort_keys=True) + "\n")
        print("legacy-stream: recorded %d site(s) in %d file(s)" % (sum(counts.values()), len(counts)))
    ratchet = load_ratchet(root)
    checks.check(ratchet is not None, "k9.legacy-stream.ratchet-exists", "no %s" % RATCHET)
    if ratchet is None:
        return
    recorded = ratchet.get("files", {})
    grew = sorted(p for p in counts if counts[p] > recorded.get(p, 0))
    stale = sorted(p for p in recorded if counts.get(p, 0) < recorded[p])
    for path in grew:
        print("legacy-stream: %s has %d site(s), the ratchet allows %d"
              % (path, counts[path], recorded.get(path, 0)))
    for path in stale:
        print("legacy-stream: %s is down to %d site(s) from %d; record it with --write"
              % (path, counts.get(path, 0), recorded[path]))
    checks.check(not grew, "k9.legacy-stream.no-new-caller",
                 "%d file(s) gained legacy-stream sites" % len(grew))
    checks.check(not stale, "k9.legacy-stream.ratchet-exact",
                 "%d file(s) shrank without a ratchet update" % len(stale))
    total = sum(counts.values())
    print("INFO legacy-stream: %d first-party site(s) in %d file(s); the K9 gate needs 0"
          % (total, len(counts)))


def check_dead_code(root, checks):
    shaders = Path(root) / LEGACY_SHADERS
    remaining = sorted(p.name for p in shaders.iterdir()) if shaders.is_dir() else []
    checks.check(not remaining, "k9.dead-code.legacy-shader-sources-deleted",
                 "%d file(s) remain in %s" % (len(remaining), LEGACY_SHADERS))


def all_files(root):
    """Every tracked plus untracked, non-ignored file (any extension)."""
    root = Path(root)
    if (root / ".git").exists():
        listed = subprocess.run(
            ["git", "-C", str(root), "ls-files", "--cached", "--others", "--exclude-standard"],
            capture_output=True, text=True, check=True).stdout.splitlines()
    else:
        listed = [p.relative_to(root).as_posix() for p in root.rglob("*") if p.is_file()]
    return sorted(set(p for p in listed if (root / p).is_file()))


def freeze_snapshot(root):
    files = all_files(root)
    frozen_sources = [p for p in files
                      if p.startswith(FROZEN_DIRS) and Path(p).suffix in EXTENSIONS]
    frozen_sources += [p for p in FROZEN_LIGHTING if (Path(root) / p).is_file()]
    shaders = {p: read(root, p).count("\n") for p in files if p.startswith(LEGACY_SHADERS)}
    stdshaders = sorted(p for p in files
                        if p.startswith(STDSHADERS) and Path(p).suffix in STDSHADER_EXTENSIONS)
    interfaces, convars, switches = set(), set(), set()
    for path in frozen_sources:
        text = read(root, path)
        code = strip_code(text)
        for match in INTERFACE_ANSWER.finditer(text):
            # a literal is kept; an identifier must survive comment stripping
            if match.group(1).startswith('"') or re.search(
                    r"\b%s\b" % re.escape(match.group(1)), code):
                interfaces.add(match.group(1).strip('"'))
        convars.update(CONVAR_DEF.findall(text))
        convars.update(CONCOMMAND_DEF.findall(text))
        switches.update(SWITCH_USE.findall(text))
    functions = {}
    for path in FROZEN_LIGHTING:
        if (Path(root) / path).is_file():
            code = strip_code(read(root, path))
            names = sorted(set(m.group(1) for m in FUNCTION_DEF.finditer(code)
                               if m.group(1) not in ("if", "for", "while", "switch", "return")))
            functions[path] = names
    return {"shader_sources": shaders, "stdshader_files": stdshaders,
            "interfaces": sorted(interfaces), "functions": functions,
            "convars": sorted(convars), "switches": sorted(switches)}


def check_legacy_freeze(root, checks, write=False):
    current = freeze_snapshot(root)
    path = Path(root) / FREEZE_RATCHET
    if write:
        data = {"schema": FREEZE_SCHEMA,
                "note": "RFC 0016 binding rule 1: the frozen legacy render paths. Exact; a set "
                        "may grow only in a Frozen-path commit (defect fix or explicit user "
                        "request) that rewrites this file with `retirement_scans.py "
                        "legacy-freeze --write`. Deletions are recorded the same way.",
                "frozen": current}
        path.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n")
        print("legacy-freeze: recorded %d shader source(s), %d stdshader file(s), %d interface(s), "
              "%d ConVar/command(s), %d switch(es)"
              % (len(current["shader_sources"]), len(current["stdshader_files"]),
                 len(current["interfaces"]), len(current["convars"]), len(current["switches"])))
    checks.check(path.exists(), "freeze.ratchet-exists", "no %s" % FREEZE_RATCHET)
    if not path.exists():
        return
    data = json.loads(path.read_text())
    checks.check(data.get("schema") == FREEZE_SCHEMA, "freeze.schema",
                 "schema must be %s" % FREEZE_SCHEMA)
    recorded = data.get("frozen", {})

    def compare_set(key, cur, rec, label):
        grew = sorted(set(cur) - set(rec))
        gone = sorted(set(rec) - set(cur))
        for item in grew[:40]:
            print("legacy-freeze: new %s on a frozen path: %s" % (label, item))
        for item in gone[:40]:
            print("legacy-freeze: %s gone, record it with --write: %s" % (label, item))
        checks.check(not grew, "freeze.%s.no-growth" % key, "%d new %s(s)" % (len(grew), label))
        checks.check(not gone, "freeze.%s.exact" % key, "%d unrecorded removal(s)" % len(gone))

    shaders, rec_shaders = current["shader_sources"], recorded.get("shader_sources", {})
    compare_set("shader-files", shaders, rec_shaders, "native backend shader file")
    longer = sorted(p for p in shaders if p in rec_shaders and shaders[p] > rec_shaders[p])
    shorter = sorted(p for p in shaders if p in rec_shaders and shaders[p] < rec_shaders[p])
    for p in longer:
        print("legacy-freeze: %s grew from %d to %d lines" % (p, rec_shaders[p], shaders[p]))
    for p in shorter:
        print("legacy-freeze: %s shrank from %d to %d lines; record it with --write"
              % (p, rec_shaders[p], shaders[p]))
    checks.check(not longer, "freeze.shader-lines.no-growth", "%d shader(s) grew" % len(longer))
    checks.check(not shorter, "freeze.shader-lines.exact", "%d unrecorded shrink(s)" % len(shorter))
    compare_set("stdshader-files", current["stdshader_files"],
                recorded.get("stdshader_files", []), "stdshader file")
    compare_set("interfaces", current["interfaces"], recorded.get("interfaces", []),
                "QueryInterface answer")
    rec_functions = recorded.get("functions", {})
    for path in FROZEN_LIGHTING:
        compare_set("functions.%s" % Path(path).stem, current["functions"].get(path, []),
                    rec_functions.get(path, []), "function in %s" % path)
    compare_set("convars", current["convars"], recorded.get("convars", []), "ConVar or command")
    compare_set("switches", current["switches"], recorded.get("switches", []), "launch switch")


SCANS = {
    "side-channels": check_side_channels,
    "record-replay": check_record_replay,
    "legacy-stream": check_legacy_stream,
    "dead-code": check_dead_code,
    "legacy-freeze": check_legacy_freeze,
}


def run_scan(name, root, **kw):
    checks = Checks()
    SCANS[name](root, checks, **kw)
    return checks


def sensitivity():
    """Seeded trees: each fault fails its scan; the clean tree passes all."""
    checks = Checks()
    with tempfile.TemporaryDirectory() as tmp:
        base = Path(tmp) / "clean"
        (base / "engine").mkdir(parents=True)
        (base / "materialsystem" / "shaderapivulkan").mkdir(parents=True)
        (base / "tools" / "render").mkdir(parents=True)
        (base / "engine" / "view.cpp").write_text(
            "// CMatRenderContextPtr in a comment is not a site\n"
            "void Draw() { CMatRenderContextPtr pRenderContext( materials ); }\n")
        (base / "engine" / "note.cpp").write_text(
            'const char *s = "g_VulkanContext.BeginFrame( x )"; // not a call\n')
        (base / "engine" / "gl_lightmap.cpp").write_text(
            "static ConVar r_projected_lights( \"r_projected_lights\", \"1\" );\n"
            "static void R_AddProjectedLights( int surf )\n{\n}\n"
            "void R_AddDynamicLights( int surf ); // a declaration is not a definition\n")
        (base / "engine" / "lightcache.cpp").write_text("void LightcacheStandIn()\n{\n}\n")
        (base / "materialsystem" / "shaderapivulkan" / "api.cpp").write_text(
            "void *Q( const char *pInterfaceName )\n{\n"
            "\tif ( !Q_stricmp( pInterfaceName, SHADER_DEVICE_MGR_INTERFACE_VERSION ) ) return 0;\n"
            "\tif ( CommandLine()->FindParm( \"-vkvalidate\" ) ) return 0;\n\treturn 0;\n}\n")
        (base / "materialsystem" / "stdshaders").mkdir(parents=True)
        (base / "materialsystem" / "stdshaders" / "lightmappedgeneric_dx9.cpp").write_text("\n")
        clean = run_scan("legacy-stream", base, write=True)
        clean_freeze = run_scan("legacy-freeze", base, write=True)
        for name in SCANS:
            result = run_scan(name, base)
            checks.check(result.failures == 0, "control.%s-passes-a-clean-tree" % name,
                         "%d failure(s)" % result.failures)

        def seeded(label, scan, mutate):
            tree = Path(tmp) / label
            shutil.copytree(base, tree)
            mutate(tree)
            result = run_scan(scan, tree)
            checks.check(result.failures > 0, "detects.%s" % label, "the %s scan passed" % scan)

        seeded("lookup-string", "side-channels", lambda t: (t / "engine" / "a.cpp").write_text(
            'void *p = materials->QueryInterface( "RenderGpuCompute001" );\n'))
        seeded("lookup-constant", "side-channels", lambda t: (t / "engine" / "b.cpp").write_text(
            "void *p = materials->QueryInterface( light_set::kLightSetConsumerInterface );\n"))
        seeded("capability-queue", "side-channels",
               lambda t: (t / "materialsystem" / "render_capability_queue.cpp").write_text("\n"))
        seeded("record-replay-call", "record-replay",
               lambda t: (t / "engine" / "c.cpp").write_text(
                   "void F() { g_VulkanContext.BeginFrame( &skip, &error ); }\n"))
        seeded("new-legacy-caller", "legacy-stream",
               lambda t: (t / "engine" / "d.cpp").write_text(
                   "void G() { IMatRenderContext *c = materials->GetRenderContext(); }\n"))
        seeded("more-legacy-sites", "legacy-stream",
               lambda t: (t / "engine" / "view.cpp").write_text(
                   "void Draw() { CMatRenderContextPtr a( materials ); CMatRenderContextPtr b( materials ); }\n"))
        seeded("stale-ratchet", "legacy-stream",
               lambda t: (t / "engine" / "view.cpp").write_text("void Draw() {}\n"))
        seeded("freeze-new-native-shader", "legacy-freeze",
               lambda t: ((t / LEGACY_SHADERS).mkdir(parents=True, exist_ok=True),
                          (t / LEGACY_SHADERS / "new_effect.frag").write_text("void main() {}\n")))
        seeded("freeze-new-stdshader", "legacy-freeze",
               lambda t: (t / "materialsystem" / "stdshaders" / "newshader_dx9.cpp").write_text("\n"))
        seeded("freeze-new-light-function", "legacy-freeze",
               lambda t: (t / "engine" / "gl_lightmap.cpp").write_text(
                   (t / "engine" / "gl_lightmap.cpp").read_text()
                   + "static void R_AddVolumetricLights( int surf )\n{\n}\n"))
        seeded("freeze-new-convar", "legacy-freeze",
               lambda t: (t / "materialsystem" / "shaderapivulkan" / "fx.cpp").write_text(
                   'static ConVar mat_vk_new_effect( "mat_vk_new_effect", "1" );\n'))
        seeded("freeze-new-switch", "legacy-freeze",
               lambda t: (t / "materialsystem" / "shaderapivulkan" / "sw.cpp").write_text(
                   'bool F() { return CommandLine()->FindParm( "-vknewthing" ); }\n'))
        seeded("freeze-new-side-channel", "legacy-freeze",
               lambda t: (t / "materialsystem" / "shaderapivulkan" / "qi.cpp").write_text(
                   'void *Q2( const char *pInterfaceName )\n{\n'
                   '\tif ( !Q_stricmp( pInterfaceName, "RenderNewFeature001" ) ) return 0;\n'
                   '\treturn 0;\n}\n'))
        seeded("freeze-unrecorded-removal", "legacy-freeze",
               lambda t: (t / "engine" / "lightcache.cpp").write_text("\n"))
        seeded("legacy-shader-left", "dead-code",
               lambda t: ((t / LEGACY_SHADERS).mkdir(parents=True),
                          (t / LEGACY_SHADERS / "x.frag").write_text("\n")))
        checks.check(clean.failures == 0, "control.ratchet-write-passes", "")
        checks.check(clean_freeze.failures == 0, "control.freeze-write-passes", "")
    return checks


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("scan", choices=sorted(SCANS) + ["sensitivity"])
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--write", action="store_true",
                        help="legacy-stream, legacy-freeze: record the current state")
    args = parser.parse_args(argv)
    if args.scan == "sensitivity":
        checks = sensitivity()
    elif args.scan in ("legacy-stream", "legacy-freeze"):
        checks = run_scan(args.scan, args.root, write=args.write)
    else:
        if args.write:
            parser.error("--write applies to legacy-stream and legacy-freeze only")
        checks = run_scan(args.scan, args.root)
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
