#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Static scans for the RFC 0016 inversion and retirement gates.

    python3 tools/render/retirement_scans.py side-channels   [--root DIR]
    python3 tools/render/retirement_scans.py record-replay   [--root DIR]
    python3 tools/render/retirement_scans.py legacy-stream   [--root DIR] [--write]
    python3 tools/render/retirement_scans.py dead-code       [--root DIR]
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


SCANS = {
    "side-channels": check_side_channels,
    "record-replay": check_record_replay,
    "legacy-stream": check_legacy_stream,
    "dead-code": check_dead_code,
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
        clean = run_scan("legacy-stream", base, write=True)
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
        seeded("legacy-shader-left", "dead-code",
               lambda t: ((t / LEGACY_SHADERS).mkdir(parents=True),
                          (t / LEGACY_SHADERS / "x.frag").write_text("\n")))
        checks.check(clean.failures == 0, "control.ratchet-write-passes", "")
    return checks


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("scan", choices=sorted(SCANS) + ["sensitivity"])
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--write", action="store_true", help="legacy-stream: record the counts")
    args = parser.parse_args(argv)
    if args.scan == "sensitivity":
        checks = sensitivity()
    elif args.scan == "legacy-stream":
        checks = run_scan(args.scan, args.root, write=args.write)
    else:
        if args.write:
            parser.error("--write applies to legacy-stream only")
        checks = run_scan(args.scan, args.root)
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
