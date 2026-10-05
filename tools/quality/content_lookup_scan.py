#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""RFC 0015 "One lookup path" ratchet: asset lookups that bypass the resolver.

    python3 tools/quality/content_lookup_scan.py check [--root DIR] [--write]
    python3 tools/quality/content_lookup_scan.py sensitivity

Counts, per file and per category, first-party sites that find assets
without the RFC 0015 runtime resolver
(RFC/0015-asset-identity-content-build-graph.md#one-lookup-path-amended-2026-10-05):

  path-build     a "materials/%s..." or "materials/" literal: a loader
                 building a texture or material path itself (CTexture's
                 materials/%s.vtf among them) instead of asking for an asset;
  vpk-direct     CPackedStore or Hammer's VpkArchive named outside their own
                 implementation: VPK access other than the future VPK
                 package source;
  search-enum    ->FindFirst/FindFirstEx/GetSearchPath member calls: walking
                 search paths to find files (the frozen IFileSystem
                 declarations are not calls and are not counted);
  fs-search-loop m_SearchPaths in filesystem/: the IFileSystem search loop
                 that RFC 0015's last cohort replaces;
  unindexed      the Unindexed lookup status, which the end state deletes.

The counts equal tools/quality/content_lookup_ratchet.json exactly: a new
file, a higher count, or a lower count not yet recorded fails, so the list
only shrinks and stays exact. --write records the current counts after
review; a migration commit records its own decrease in the same change.
A reviewed site that is not an asset lookup (a save or config listing) goes
in the ratchet's "exempt" map with a reason instead of being counted. R83
needs every count at zero, which the check reports separately (INFO).

Comments are stripped before matching identifiers; path-build matches
string literals. Ends with one checks-v1 record. Python 3 standard library
only.
"""

import argparse
import json
import re
import shutil
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "quality"))
sys.path.insert(0, str(ROOT / "tools" / "render"))
from conformance_result import Checks  # noqa: E402
# One owner for first-party source listing and comment stripping.
from retirement_scans import sources, strip_code  # noqa: E402

SCHEMA = "content-lookup-ratchet/v1"
RATCHET = "tools/quality/content_lookup_ratchet.json"
# Tests exercise readers directly; utils/vpk is the VPK authoring tool;
# external/ is vendored (VPC).
SKIPPED = ("unittests/", "utils/vpk/", "external/")
# The readers' own implementations, which the VPK package source will wrap.
VPK_IMPLEMENTATION = ("vpklib/", "public/vpklib/", "hammer/core/formats/vpk_archive.cpp",
                      "public/hammer/formats/vpk_archive.h")

PATH_BUILD = re.compile(r'"materials/(?:"|[^"\n]*%s)', re.I)
VPK_DIRECT = re.compile(r"\b(?:CPackedStore|VpkArchive)\b")
SEARCH_ENUM = re.compile(r"(?:->|\.)\s*(?:FindFirstEx|FindFirst|GetSearchPath)\s*\(")
FS_SEARCH_LOOP = re.compile(r"\bm_SearchPaths\b")
UNINDEXED = re.compile(r"\bUnindexed\b")


def count_file(path, text):
    code = strip_code(text)
    found = {}
    found["path-build"] = len(PATH_BUILD.findall(text)) - len(PATH_BUILD.findall(
        _comments_only(text, code)))
    if not path.startswith(VPK_IMPLEMENTATION):
        found["vpk-direct"] = len(VPK_DIRECT.findall(code))
    found["search-enum"] = len(SEARCH_ENUM.findall(code))
    if path.startswith("filesystem/"):
        found["fs-search-loop"] = len(FS_SEARCH_LOOP.findall(code))
    found["unindexed"] = len(UNINDEXED.findall(code))
    return {k: v for k, v in found.items() if v > 0}


def _comments_only(text, code):
    """Comment text only (literals and code blanked): literals in comments
    are not sites."""
    out = []
    i, n = 0, len(text)
    while i < n:
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(text[i:j])
            i = j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append(text[i:j])
            i = j
        elif text[i] in "\"'":
            q = text[i]
            j = i + 1
            while j < n and text[j] != q and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            i = j + 1
        else:
            i += 1
    return "\n".join(out)


def scan(root):
    counts = {}
    for path in sources(root):
        if path.startswith(SKIPPED):
            continue
        found = count_file(path, (Path(root) / path).read_text(encoding="utf-8",
                                                               errors="replace"))
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


def totals(counts):
    out = {}
    for found in counts.values():
        for category, n in found.items():
            out[category] = out.get(category, 0) + n
    return dict(sorted(out.items()))


def check(root, checks, write=False):
    counts = scan(root)
    previous = load_ratchet(root)
    exempt = previous.get("exempt", {}) if previous else {}
    for path in exempt:
        counts.pop(path, None)
    if write:
        data = {"schema": SCHEMA,
                "note": "RFC 0015 'One lookup path': asset lookups that bypass the resolver, "
                        "per file and category. Only shrinks; rewrite with "
                        "`content_lookup_scan.py check --write` after review. R83 needs zero.",
                "exempt": exempt,
                "totals": totals(counts),
                "files": counts}
        (Path(root) / RATCHET).write_text(json.dumps(data, indent=2, sort_keys=True) + "\n")
        print("content-lookup: recorded %d site(s) in %d file(s)"
              % (sum(totals(counts).values()), len(counts)))
    ratchet = load_ratchet(root)
    checks.check(ratchet is not None, "content-lookup.ratchet-exists", "no %s" % RATCHET)
    if ratchet is None:
        return
    for path, reason in ratchet.get("exempt", {}).items():
        checks.check(bool(str(reason).strip()), "content-lookup.exempt-has-reason",
                     "%s is exempt without a reason" % path)
    recorded = ratchet.get("files", {})
    grew, stale = [], []
    for path in sorted(set(counts) | set(recorded)):
        cur, rec = counts.get(path, {}), recorded.get(path, {})
        for category in sorted(set(cur) | set(rec)):
            c, r = cur.get(category, 0), rec.get(category, 0)
            if c > r:
                grew.append("%s %s: %d, the ratchet allows %d" % (path, category, c, r))
            elif c < r:
                stale.append("%s %s: down to %d from %d; record it with --write"
                             % (path, category, c, r))
    for line in grew:
        print("content-lookup: grew:", line)
    for line in stale:
        print("content-lookup: stale:", line)
    checks.check(not grew, "content-lookup.no-new-bypass", "%d site count(s) grew" % len(grew))
    checks.check(not stale, "content-lookup.ratchet-exact",
                 "%d site count(s) shrank without a ratchet update" % len(stale))
    checks.check(ratchet.get("totals") == totals(recorded), "content-lookup.totals-consistent",
                 "the recorded totals disagree with the recorded files")
    for category, n in totals(counts).items():
        print("INFO content-lookup: %s %d site(s); R83 needs 0" % (category, n))


def sensitivity():
    checks = Checks()
    with tempfile.TemporaryDirectory() as tmp:
        base = Path(tmp) / "clean"
        for d in ("materialsystem", "filesystem", "hammer/gtk", "content", "tools/quality",
                  "vpklib"):
            (base / d).mkdir(parents=True)
        (base / "materialsystem" / "ctexture.cpp").write_text(
            "// \"materials/%s.vtf\" in a comment is not a site\n"
            'void F( char *b, const char *n ) { sprintf( b, "materials/%s.vtf", n ); }\n')
        (base / "filesystem" / "basefilesystem.cpp").write_text(
            "void G() { for ( int i = 0; i < m_SearchPaths.Count(); ++i ) {} }\n")
        (base / "hammer" / "gtk" / "catalog.cpp").write_text("VpkArchive *g_pArchive;\n")
        (base / "vpklib" / "packedstore.cpp").write_text("class CPackedStore {};\n")
        (base / "content" / "resolver.cpp").write_text("enum S { Indexed, Unindexed };\n")
        clean = Checks()
        check(base, clean, write=True)
        checks.check(clean.failures == 0, "control.write-passes", "%d failure(s)" % clean.failures)
        recorded = json.loads((base / RATCHET).read_text())["files"]
        checks.check("vpklib/packedstore.cpp" not in recorded,
                     "control.vpk-implementation-not-counted", "")
        checks.check(recorded.get("materialsystem/ctexture.cpp") == {"path-build": 1},
                     "control.comment-literal-not-counted", str(recorded))
        again = Checks()
        check(base, again)
        checks.check(again.failures == 0, "control.clean-tree-passes", "")

        def seeded(label, mutate):
            tree = Path(tmp) / label
            shutil.copytree(base, tree)
            mutate(tree)
            result = Checks()
            check(tree, result)
            checks.check(result.failures > 0, "detects.%s" % label, "the scan passed")

        seeded("new-path-build", lambda t: (t / "materialsystem" / "cmaterial.cpp").write_text(
            'void H( char *b ) { sprintf( b, "materials/%s.vmt", "x" ); }\n'))
        seeded("more-path-build", lambda t: (t / "materialsystem" / "ctexture.cpp").write_text(
            'void F( char *b ) { sprintf( b, "materials/%s.vtf", "a" ); '
            'strcpy( b, "materials/" ); }\n'))
        seeded("new-packed-store", lambda t: (t / "hammer" / "gtk" / "models.cpp").write_text(
            "CPackedStore *g_pStore;\n"))
        seeded("new-search-enum", lambda t: (t / "hammer" / "gtk" / "browse.cpp").write_text(
            'void B() { g_pFullFileSystem->FindFirstEx( "materials/*", "GAME", 0 ); }\n'))
        seeded("more-fs-loop", lambda t: (t / "filesystem" / "basefilesystem.cpp").write_text(
            "void G() { m_SearchPaths.Count(); m_SearchPaths.Count(); }\n"))
        seeded("new-unindexed", lambda t: (t / "content" / "lab.cpp").write_text(
            "bool U( S s ) { return s == Unindexed; }\n"))
        seeded("stale-ratchet", lambda t: (t / "hammer" / "gtk" / "catalog.cpp").write_text("\n"))
        seeded("exempt-without-reason", lambda t: (t / RATCHET).write_text(json.dumps(
            dict(json.loads((t / RATCHET).read_text()),
                 exempt={"content/resolver.cpp": ""}))))
        seeded("totals-tampered", lambda t: (t / RATCHET).write_text(json.dumps(
            dict(json.loads((t / RATCHET).read_text()), totals={}))))
    return checks


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=["check", "sensitivity"])
    parser.add_argument("--root", type=Path, default=ROOT)
    parser.add_argument("--write", action="store_true", help="check: record the current counts")
    args = parser.parse_args(argv)
    if args.command == "sensitivity":
        if args.write:
            parser.error("--write applies to check only")
        checks = sensitivity()
    else:
        checks = Checks()
        check(args.root, checks, write=args.write)
    return checks.report()


if __name__ == "__main__":
    sys.exit(main())
