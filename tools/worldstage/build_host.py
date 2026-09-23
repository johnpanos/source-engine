#!/usr/bin/env python3
"""Build the pinned RFC 0008 OpenUSD host-tool profile from local checkouts."""

import argparse
import json
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROFILE = ROOT / "quality/product_profiles/openusd-linux-tools.json"


def require_revision(source, expected):
    result = subprocess.run(["git", "-C", str(source), "rev-parse", "HEAD"],
                            capture_output=True, text=True, check=True)
    if result.stdout.strip() != expected:
        raise ValueError(f"{source} does not match the profile revision")
    result = subprocess.run(
        ["git", "-C", str(source), "status", "--porcelain", "--untracked-files=no"],
        capture_output=True, text=True, check=True)
    if result.stdout.strip():
        raise ValueError(f"{source} has tracked source changes")


def configure(source, build, install, profile, options, extra=()):
    command = ["cmake", "-S", str(source), "-B", str(build),
               "-G", profile["build"]["generator"],
               f"-DCMAKE_BUILD_TYPE={profile['toolchain']['build_type']}",
               f"-DCMAKE_INSTALL_PREFIX={install}"]
    command.extend(f"-D{key}={value}" for key, value in options.items())
    command.extend(extra)
    subprocess.run(command, check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--openusd-source", type=Path, required=True)
    parser.add_argument("--openusd-build", type=Path, required=True)
    parser.add_argument("--openusd-install", type=Path, required=True)
    parser.add_argument("--onetbb-source", type=Path, required=True)
    parser.add_argument("--onetbb-build", type=Path, required=True)
    parser.add_argument("--onetbb-install", type=Path, required=True)
    parser.add_argument("--python", type=Path, default=Path("/usr/bin/python3.12"))
    parser.add_argument("--jobs", type=int, default=8)
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    profile = json.loads(PROFILE.read_text(encoding="utf-8"))
    if profile.get("schema") != "source-host-tool-profile/v1":
        parser.error("host profile schema is invalid")
    require_revision(args.onetbb_source, profile["dependencies"]["onetbb"]["revision"])
    require_revision(args.openusd_source, profile["dependencies"]["openusd"]["revision"])
    paths = {key: getattr(args, key).resolve() for key in
             ("openusd_source", "openusd_build", "openusd_install",
              "onetbb_source", "onetbb_build", "onetbb_install")}
    configure(paths["onetbb_source"], paths["onetbb_build"], paths["onetbb_install"],
              profile, profile["build"]["onetbb_cmake_options"])
    subprocess.run(["cmake", "--build", str(paths["onetbb_build"]),
                    "--target", "install", f"-j{args.jobs}"], check=True)
    configure(paths["openusd_source"], paths["openusd_build"], paths["openusd_install"],
              profile, profile["build"]["cmake_options"],
              (f"-DCMAKE_PREFIX_PATH={paths['onetbb_install']}",
               f"-DPython3_EXECUTABLE={args.python.resolve()}"))
    subprocess.run(["cmake", "--build", str(paths["openusd_build"]),
                    "--target", "install", f"-j{args.jobs}"], check=True)
    print("Pinned OpenUSD host build installed; run openusd_host_probe.py for acceptance evidence")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
