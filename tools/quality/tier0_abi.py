#!/usr/bin/env python3
"""Tier 0's mod-facing export surface as a fixture (RFC 0001, R103 T0).

`record` writes, for one built libtier0, every exported symbol with its kind
(nm's T/D/B/V/...) and, for C exports, the normalized declaration from the
frozen headers in public/tier0/. C++ exports are frozen by their mangled names,
which encode their types.

`check` fails when the library or the headers drop, re-kind or re-declare any
recorded export. New exports are reported but allowed.

  tools/quality/tier0_abi.py record --lib build/tier0/libtier0.so --platform linux-x86_64
  tools/quality/tier0_abi.py check  --lib build/tier0/libtier0.so --platform linux-x86_64
  tools/quality/tier0_abi.py selftest --lib build/tier0/libtier0.so --platform linux-x86_64
"""

from __future__ import annotations

import argparse
import copy
import json
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
FIXTURES = os.path.join(ROOT, "quality", "fixtures", "tier0-abi")
HEADERS = os.path.join(ROOT, "public", "tier0")
SCHEMA = "tier0-abi/v1"
# std:: members, typeinfo and typeinfo names, and their local statics.
STD_INTERNAL = re.compile(r"^_Z(N|NK|TI|TS|TV|ZN|ZNK)St")


def exported_symbols(lib, nm="nm"):
    out = subprocess.run([nm, "-D", "--defined-only", lib], capture_output=True, text=True,
                         check=True).stdout
    symbols = {}
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3:
            symbols[parts[2]] = parts[1]
    return symbols


def header_statements():
    """Declarations from public/tier0/*.h: comments removed, preprocessor
    lines dropped, split into statements at ';', '{' and '}'."""
    statements = []
    for name in sorted(os.listdir(HEADERS)):
        if not name.endswith(".h"):
            continue
        with open(os.path.join(HEADERS, name), encoding="latin-1") as handle:
            text = handle.read()
        text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
        text = re.sub(r"//[^\n]*", " ", text)
        text = "\n".join(l for l in text.splitlines() if not l.lstrip().startswith("#"))
        for piece in re.split(r"[;{}]", text):
            statement = " ".join(piece.split())
            if statement:
                statements.append((name, statement))
    return statements


EXPORT_MARKERS = re.compile(r"\b(PLATFORM_INTERFACE|PLATFORM_OVERLOAD|DBG_INTERFACE|DBG_OVERLOAD|"
                            r"TIER0_INTERFACE|MEM_INTERFACE|DLL_EXPORT|DLL_CLASS_EXPORT|DLL_GLOBAL_EXPORT|extern)\b")


def declarations(name, statements):
    """Every exported declaration of `name` in the headers (overloads and
    platform variants included), normalized and sorted."""
    pattern = re.compile(r"(?<![\w:])" + re.escape(name) + r"\s*(\(|\[|$|=)")
    found = sorted({"%s: %s" % (header, s) for header, s in statements
                    if EXPORT_MARKERS.search(s) and pattern.search(s)})
    return found


def build_record(lib, platform, nm):
    symbols = exported_symbols(lib, nm)
    statements = header_statements()
    exports = {}
    for name, kind in sorted(symbols.items()):
        entry = {"kind": kind}
        if not name.startswith("_Z"):
            entry["declarations"] = declarations(name, statements)
        exports[name] = entry
    return {"schema": SCHEMA, "platform": platform, "library": os.path.basename(lib),
            "exports": exports}


def compare(recorded, current):
    problems, added = [], []
    for name, entry in recorded["exports"].items():
        now = current["exports"].get(name)
        if now is None:
            problems.append("removed export %s" % name)
            continue
        if now["kind"] != entry["kind"]:
            problems.append("%s changed kind %s -> %s" % (name, entry["kind"], now["kind"]))
        if "declarations" in entry and now.get("declarations") != entry["declarations"]:
            problems.append("%s re-declared: %s -> %s" % (name, entry["declarations"],
                                                         now.get("declarations")))
    for name in current["exports"]:
        if name not in recorded["exports"]:
            if STD_INTERNAL.match(name):
                # A static library leaking std's template instantiations or
                # typeinfo into Tier 0's exports: never part of the mod ABI.
                problems.append("new export %s is a standard-library internal" % name)
            else:
                added.append(name)
    return problems, added


def fixture_path(platform):
    return os.path.join(FIXTURES, platform + ".json")


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    for command in ("record", "check", "selftest"):
        p = sub.add_parser(command)
        p.add_argument("--lib", required=True)
        p.add_argument("--platform", required=True)
        p.add_argument("--nm", default="nm")
    args = parser.parse_args()
    current = build_record(args.lib, args.platform, args.nm)

    if args.command == "record":
        os.makedirs(FIXTURES, exist_ok=True)
        with open(fixture_path(args.platform), "w", encoding="utf-8") as handle:
            json.dump(current, handle, indent=1, sort_keys=True)
            handle.write("\n")
        print("recorded %d exports for %s" % (len(current["exports"]), args.platform))
        return 0

    with open(fixture_path(args.platform), encoding="utf-8") as handle:
        recorded = json.load(handle)

    if args.command == "check":
        problems, added = compare(recorded, current)
        for problem in problems:
            print("FAIL " + problem)
        if added:
            print("note: %d new export(s): %s" % (len(added), ", ".join(sorted(added)[:20])))
        checks = len(recorded["exports"])
        print("CONFORMANCE %d %d" % (checks, len(problems)))
        return 1 if problems else 0

    # selftest: seeded faults must each be caught; the unchanged record passes.
    checks, failures = 0, 0

    def expect(ok, what):
        nonlocal checks, failures
        checks += 1
        if not ok:
            failures += 1
            print("FAIL selftest: " + what)

    expect(not compare(recorded, current)[0], "the current library matches its fixture")
    c_names = [n for n, e in recorded["exports"].items() if e.get("declarations")]
    cpp_names = [n for n in recorded["exports"] if n.startswith("_Z")]
    for name in (c_names[:3] + cpp_names[:2]):
        mutated = copy.deepcopy(current)
        del mutated["exports"][name]
        expect(bool(compare(recorded, mutated)[0]), "removing %s is caught" % name)
    for name in c_names[:3]:
        mutated = copy.deepcopy(current)
        mutated["exports"][name]["declarations"] = [d.replace("(", "( int extra,", 1)
                                                     for d in mutated["exports"][name]["declarations"]]
        expect(bool(compare(recorded, mutated)[0]), "re-declaring %s is caught" % name)
    mutated = copy.deepcopy(current)
    first = next(iter(mutated["exports"]))
    mutated["exports"][first]["kind"] = "W"
    expect(bool(compare(recorded, mutated)[0]), "changing a kind is caught")
    mutated = copy.deepcopy(current)
    mutated["exports"]["Plat_NewThing"] = {"kind": "T", "declarations": []}
    expect(not compare(recorded, mutated)[0], "an added export is allowed")
    mutated = copy.deepcopy(current)
    mutated["exports"]["_ZNSt8_Rb_treeISsSsE4findEv"] = {"kind": "W"}
    expect(bool(compare(recorded, mutated)[0]), "an added std internal is refused")
    print("CONFORMANCE %d %d" % (checks, failures))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
