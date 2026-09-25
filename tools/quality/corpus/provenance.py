#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Provenance for gap-corpus suites and fixtures (RFC 0005 Q0 gap corpus).
#
# Every corpus suite (quality/conformance.manifest.json, profile
# linux-host-corpus) and every recorded fixture under quality/fixtures/corpus/
# names what produced it: the source revision and dirty digest, the pinned tools
# and their versions, and the exact reproduction command. A suite prints one
# `PROVENANCE <json>` line and writes `provenance.json` into its scratch
# directory ($CONFORMANCE_OUT); fixtures carry the same block in their
# `fixture.json`. Python 3 standard library only.
#
# ============================================================================

import hashlib
import json
import os
import shlex
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
QUALITY = os.path.dirname(HERE)
sys.path.insert(0, QUALITY)

import conformance  # noqa: E402

ROOT = conformance.repo_root()
SCHEMA = "corpus-provenance/v1"


def tool_version(argv, pattern_lines=1):
    """First line(s) of a tool's version output, or None when it cannot run."""
    try:
        out = subprocess.run(argv, capture_output=True, text=True, check=False, timeout=60)
    except (OSError, subprocess.SubprocessError):
        return None
    text = (out.stdout or out.stderr or "").strip().splitlines()
    return " ".join(line.strip() for line in text[:pattern_lines]) or None


def file_sha256(path):
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def collect(suite, tools=None, inputs=None, command=None, extra=None):
    """The provenance block of one run.

    `tools` maps a name to a version string (or None when unavailable),
    `inputs` maps a label to a file path whose SHA-256 is recorded, and
    `command` is the exact reproduction argv (default: this process).
    """
    identity = conformance.source_identity(ROOT)
    block = {
        "schema": SCHEMA,
        "suite": suite,
        "source_revision": identity["source_revision"],
        "dirty": identity["dirty"],
        "dirty_digest": identity["dirty_digest"],
        "submodules": conformance.submodule_identity(ROOT),
        "python": sys.version.split()[0],
        "python_executable": sys.executable,
        "tools": dict(tools or {}),
        "inputs": {label: {"path": os.path.relpath(path, ROOT) if os.path.isabs(path) else path,
                           "sha256": file_sha256(path) if os.path.isfile(path) else None}
                   for label, path in (inputs or {}).items()},
        "reproduce": "cd %s && %s" % (shlex.quote(ROOT), shlex.join(
            command if command is not None else [sys.executable] + sys.argv)),
    }
    if extra:
        block.update(extra)
    return block


def emit(block, out=None):
    """Print the block as one PROVENANCE line and write provenance.json to `out`
    (default: $CONFORMANCE_OUT, when set)."""
    print("PROVENANCE " + json.dumps(block, sort_keys=True), flush=True)
    out = out or os.environ.get("CONFORMANCE_OUT")
    if out:
        os.makedirs(out, exist_ok=True)
        with open(os.path.join(out, "provenance.json"), "w", encoding="utf-8") as stream:
            json.dump(block, stream, indent=2, sort_keys=True)
            stream.write("\n")
    return block
