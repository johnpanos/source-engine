#!/usr/bin/env python3
"""Product profiles with their "extends" chain resolved.

A derived product (another game in the same app shell) names the profile that
owns the shared pins instead of copying them. This module owns the merge rule
for every platform's product profiles (build-android-apk.sh, build-ios-app.sh,
ios-deploy.sh, android_apk.py).

    profile_extends.py resolve PROFILE     # print the resolved profile
"""

import argparse
import json
import sys
from pathlib import Path


def merge_profile(base, derived):
    """derived over base: objects merge key by key, any other value replaces."""
    merged = dict(base)
    for key, value in derived.items():
        if isinstance(value, dict) and isinstance(merged.get(key), dict):
            merged[key] = merge_profile(merged[key], value)
        else:
            merged[key] = value
    return merged


def load_profile(path):
    """A product profile, with its "extends" chain (paths relative to it) resolved."""
    path = Path(path).resolve()
    profile = json.loads(path.read_text())
    parent = profile.pop("extends", None)
    if parent is None:
        return profile
    return merge_profile(load_profile(path.parent / parent), profile)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    sub = parser.add_subparsers(dest="command", required=True)
    resolve = sub.add_parser("resolve", help="print a profile with its \"extends\" chain resolved")
    resolve.add_argument("profile", type=Path)
    args = parser.parse_args(argv)
    print(json.dumps(load_profile(args.profile), indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
