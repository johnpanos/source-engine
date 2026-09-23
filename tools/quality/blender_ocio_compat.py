#!/usr/bin/env python3
"""Make a private OCIO 2.4 preview config from Blender's 2.5 config.

This is a local packaging workaround for Blender 5.2.1 with system OpenColorIO
2.4.2. It only drops 2.5 interop metadata and keeps the color transforms and
referenced LUT files. The result is preview evidence, not a pinned color gate.
"""

import argparse
import hashlib
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-dir", type=Path, required=True)
    parser.add_argument("--out-dir", type=Path, required=True)
    args = parser.parse_args()
    source = args.source_dir.resolve()
    output = args.out_dir.resolve()
    if output.exists():
        parser.error("output directory already exists: " + str(output))
    original = (source / "config.ocio").read_text(encoding="utf-8")
    if original.count("ocio_profile_version: 2.5\n") != 1:
        raise ValueError("Blender OCIO source is not the expected 2.5 config")
    adapted = original.replace("ocio_profile_version: 2.5\n",
                               "ocio_profile_version: 2.4\n", 1)
    lines = adapted.splitlines(keepends=True)
    adapted_lines = []
    removed = 0
    interchange_indent = None
    for line in lines:
        indent = len(line) - len(line.lstrip(" \t"))
        if interchange_indent is not None:
            if line.strip() and indent > interchange_indent:
                removed += 1
                continue
            interchange_indent = None
        if line.lstrip().startswith("interop_id:"):
            removed += 1
            continue
        if line.lstrip().startswith("interchange:"):
            interchange_indent = indent
            removed += 1
            continue
        adapted_lines.append(line)
    adapted = "".join(adapted_lines)
    if not removed:
        raise ValueError("Blender OCIO interop metadata changed; review compatibility")
    output.mkdir(parents=True)
    (output / "config.ocio").write_text(adapted, encoding="utf-8")
    for name in ("icc", "luts", "filmic"):
        path = source / name
        if not path.is_dir():
            raise ValueError("Blender OCIO resource is missing: " + str(path))
        (output / name).symlink_to(path, target_is_directory=True)
    evidence = {
        "source_sha256": hashlib.sha256(original.encode()).hexdigest(),
        "adapted_sha256": hashlib.sha256(adapted.encode()).hexdigest(),
        "removed_interop_metadata_lines": removed,
        "preview_only": True,
    }
    (output / "adaptation.json").write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n")
    print(json.dumps(evidence, sort_keys=True))


if __name__ == "__main__":
    main()
