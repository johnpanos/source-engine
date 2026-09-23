#!/usr/bin/env python3
"""Generate the Source OpenUSD schema and plugin metadata in an isolated build root."""

import argparse
import os
from pathlib import Path
import subprocess


ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--openusd-source", type=Path, required=True)
    parser.add_argument("--openusd-install", type=Path, required=True)
    parser.add_argument("--onetbb-install", type=Path, required=True)
    parser.add_argument("--python", type=Path, default=Path("/usr/bin/python3.12"))
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env["PYTHONPATH"] = str(args.openusd_install.resolve() / "lib/python")
    env["LD_LIBRARY_PATH"] = os.pathsep.join(
        (str(args.openusd_install.resolve() / "lib"),
         str(args.onetbb_install.resolve() / "lib64"))
    )
    generator = args.openusd_source.resolve() / "pxr/usd/usd/usdGenSchema.py"
    subprocess.run([str(args.python.resolve()), str(generator),
                    str(ROOT / "utils/worldstage/schema.usda"), str(output), "-q"],
                   check=True, env=env)
    plug_info = output / "plugInfo.json"
    template = plug_info.read_text(encoding="utf-8")
    for field, marker, replacement in (
        ("LibraryPath", "@PLUG_INFO_LIBRARY_PATH@", "../../libsourceWorld.so"),
        ("ResourcePath", "@PLUG_INFO_RESOURCE_PATH@", "resources"),
        ("Root", "@PLUG_INFO_ROOT@", ".."),
    ):
        if template.count(marker) == 1:
            template = template.replace(marker, replacement)
        elif f'"{field}": "{replacement}"' not in template:
            raise ValueError(f"generated plugin metadata omitted {marker}")
    plug_info.write_text(template, encoding="utf-8")
    (output / "pluginRoot.json").write_text(
        '{"Includes": ["resources/"]}\n', encoding="utf-8")
    print("Generated SourceWorldAPI plugin metadata in " + str(output))


if __name__ == "__main__":
    main()
