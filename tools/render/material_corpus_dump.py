#!/usr/bin/env python3
"""The whole-corpus material-state oracle (RFC 0016 K9, R91): every Portal and
Portal 2 material the VMT corpus finds is loaded and precached headless
(`mat_load_material_list`), then `mat_dump_material_state` writes each
material's load-time state (flags, lightmap and bumped lightmap, vertex
format, translucency, alpha test, vertex lighting, two-sidedness, tangent
space, full-frame use, env cubemap, every parameter's value).

    python3 tools/render/material_corpus_dump.py dump --out DIR
    python3 tools/render/material_corpus_dump.py compare BASELINE_DIR DIR

`dump` runs tools/render/vmt_corpus.py check into DIR for the material lists,
then one headless boot per game through kiln (the portal and portal2
profiles). `compare` fails when a material's structural fields differ or a
material is missing; parameter values that animate at runtime ($time,
proxies) and the runtime-created font pages are reported, not judged.
"""

import argparse
import json
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
GAMES = {"portal": "testchmb_a_00", "portal2": "sp_a1_intro4"}
STRUCTURAL = ("shader", "flags", "lightmap", "bumped", "vertex_format", "translucent",
              "alpha_tested", "vertex_lit", "two_sided", "tangent_space", "full_frame",
              "env_cubemap")
RUNTIME_ONLY = ("__fontpage", "__fontpage_additive")


def load(path):
    rows = {}
    with open(path, errors="replace") as handle:
        for line in handle:
            if line.strip():
                row = json.loads(line.replace("\\", "/"), strict=False)
                rows[row["name"]] = row
    return rows


def dump(out):
    os.makedirs(out, exist_ok=True)
    corpus = os.path.join(out, "vmt-corpus")
    subprocess.run([sys.executable, os.path.join(ROOT, "tools/render/vmt_corpus.py"), "check",
                    "--out", corpus], check=True, cwd=ROOT)
    for game, level in GAMES.items():
        names = set()
        with open(os.path.join(corpus, game + ".records.jsonl")) as handle:
            for line in handle:
                record = json.loads(line)
                path = record.get("path", "")
                if record.get("status") == "ok" and path.startswith("materials/") and \
                        path.endswith(".vmt"):
                    names.add(path[len("materials/"):-len(".vmt")])
        listing = os.path.abspath(os.path.join(out, game + "_materials.txt"))
        with open(listing, "w") as handle:
            handle.write("\n".join(sorted(names)) + "\n")
        result = os.path.abspath(os.path.join(out, game + ".materials.jsonl"))
        boot = os.path.abspath(os.path.join(out, "boot-" + game))
        subprocess.run([sys.executable, os.path.join(ROOT, "tools/quality/portal_boot.py"),
                        "--profile", game, "--map", level, "--headless", "--out", boot,
                        "--console-command", "mat_load_material_list " + listing,
                        "--console-command", "mat_dump_material_state " + result],
                       check=True, cwd=ROOT)
        print("%s: %d materials listed, %d dumped" % (game, len(names), len(load(result))))


def compare(baseline, current):
    failed = False
    for game in GAMES:
        a = load(os.path.join(baseline, game + ".materials.jsonl"))
        b = load(os.path.join(current, game + ".materials.jsonl"))
        missing = sorted(set(a) - set(b) - set(RUNTIME_ONLY))
        structural = [(name, field, a[name].get(field), b[name].get(field))
                      for name in sorted(set(a) & set(b)) for field in STRUCTURAL
                      if a[name].get(field) != b[name].get(field)]
        params = sum(1 for name in set(a) & set(b) if a[name]["params"] != b[name]["params"])
        print("%s: %d common, %d missing, %d structural differences, %d with parameter "
              "differences (reported)" % (game, len(set(a) & set(b)), len(missing),
                                          len(structural), params))
        for name in missing[:20]:
            print("  missing: " + name)
        for name, field, before, after in structural[:20]:
            print("  %s %s: %r -> %r" % (name, field, before, after))
        failed = failed or bool(missing) or bool(structural)
    return 1 if failed else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = parser.add_subparsers(dest="command", required=True)
    d = sub.add_parser("dump")
    d.add_argument("--out", required=True)
    c = sub.add_parser("compare")
    c.add_argument("baseline")
    c.add_argument("current")
    args = parser.parse_args()
    if args.command == "dump":
        dump(args.out)
        return 0
    return compare(args.baseline, args.current)


if __name__ == "__main__":
    sys.exit(main())
