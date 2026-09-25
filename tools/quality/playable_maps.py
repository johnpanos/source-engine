#!/usr/bin/env python3
"""Published PBRT maps for the dev launcher (./play, ./run.sh).

`pbrt_map_build.py` publishes each map it builds to run/maps/<map>/ (a copy of
its content root plus published.json; untracked). Every ./play launch mounts
all published maps into its runtime as portal/custom/pbrt-<map>, a symlink that
the gameinfo's `portal/custom/*` search path loads at boot, so

    ./play living_room

loads the newest build. The engine draws the map's WMSH/LMAP world by default,
so a map needs no launch settings and also draws when loaded from the console.
Publishing replaces the old copy atomically; `remove` unpublishes a map and the
next launch drops its mount. Existing builds can be
published without rebuilding:

    python3 tools/quality/playable_maps.py publish quality-results/living-room-map
    python3 tools/quality/playable_maps.py list
"""

import argparse
import datetime
import json
import os
import shutil
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import portal_boot  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
STORE = ROOT / "run" / "maps"
RECORD = "published.json"
SCHEMA = "pbrt-published-map/v1"
MOUNT_PREFIX = "pbrt-"


def publish(build_summary, store=STORE, sidecars=None):
    """Copy a pipeline build's content root into the store; returns the record.

    `sidecars` ({file name: text}) are build records published beside
    published.json in the same rename, such as the USD compiler's provenance.
    They are not game content and are never mounted as such."""
    name = build_summary["map"]
    files = portal_boot.content_files(build_summary["content_root"])
    if not any(relative == Path("maps", name + ".bsp") for _, relative in files):
        raise ValueError("content root has no maps/%s.bsp" % name)
    sidecars = dict(sidecars or {})
    for sidecar in sidecars:
        if (Path(sidecar).name != sidecar or sidecar == RECORD or
                not sidecar.endswith(".json")):
            raise ValueError("invalid published-map sidecar name: " + sidecar)
    store = Path(store)
    store.mkdir(parents=True, exist_ok=True)
    staging = store / (".publish-%s-%d" % (name, os.getpid()))
    shutil.rmtree(staging, ignore_errors=True)
    for source, relative in files:
        (staging / relative).parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, staging / relative)
    for sidecar, text in sidecars.items():
        (staging / sidecar).write_text(text)
    record = {"schema": SCHEMA, "map": name, "status": build_summary["status"],
              "failed_gates": build_summary.get("failed_gates", []),
              "bsp2_sha256": build_summary.get("bsp2_sha256"),
              "content_root": str(build_summary["content_root"]),
              "files": len(files),
              "published": datetime.datetime.now(datetime.timezone.utc).isoformat(
                  timespec="seconds")}
    (staging / RECORD).write_text(json.dumps(record, indent=2) + "\n")
    target, previous = store / name, store / (".previous-%s-%d" % (name, os.getpid()))
    if target.exists():
        target.rename(previous)
    staging.rename(target)
    shutil.rmtree(previous, ignore_errors=True)
    return record


def published(store=STORE):
    """Records of every published map, by map name."""
    records = {}
    for path in sorted(Path(store).glob("*/" + RECORD)):
        record = json.loads(path.read_text())
        if (record.get("schema") != SCHEMA or record.get("map") != path.parent.name or
                not (path.parent / "maps" / (path.parent.name + ".bsp")).is_file()):
            raise ValueError("invalid published map: " + str(path.parent))
        records[record["map"]] = record
    return records


def remove(name, store=STORE):
    target = Path(store) / name
    if not (target / RECORD).is_file():
        raise ValueError("map is not published: " + name)
    shutil.rmtree(target)


def mount(runtime, store=STORE, game="portal"):
    """Point runtime/<game>/custom/pbrt-<map> at every published map.

    Only symlinks with the pbrt- prefix are managed; a stale mount is removed.
    A published map whose name matches a base map is not mounted, because the
    custom search path would silently replace the shipped map.
    """
    custom = Path(runtime) / game / "custom"
    custom.mkdir(parents=True, exist_ok=True)
    records = published(store)
    for entry in sorted(custom.glob(MOUNT_PREFIX + "*")):
        if not entry.is_symlink():
            raise ValueError("unmanaged directory uses the published-map prefix: " + str(entry))
        if entry.name[len(MOUNT_PREFIX):] not in records:
            entry.unlink()
    mounted, skipped = {}, {}
    for name, record in records.items():
        link = custom / (MOUNT_PREFIX + name)
        if (Path(runtime) / game / "maps" / (name + ".bsp")).exists():
            skipped[name] = "a base map has the same name"
            if link.is_symlink():
                link.unlink()
            continue
        target = os.path.relpath(Path(store).resolve() / name, custom.resolve())
        if not link.is_symlink() or os.readlink(link) != target:
            if link.is_symlink():
                link.unlink()
            link.symlink_to(target, target_is_directory=True)
        mounted[name] = record
    return mounted, skipped


def describe(record):
    status = record["status"]
    if record.get("failed_gates"):
        status += " (" + ", ".join(record["failed_gates"]) + ")"
    return "%-16s %-40s %s" % (record["map"], status, record["published"])


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    publish_command = commands.add_parser("publish", help="publish finished pipeline builds")
    publish_command.add_argument("builds", type=Path, nargs="+",
                                 help="pbrt_map_build.py --out directories (with build.json)")
    commands.add_parser("list", help="list published maps")
    remove_command = commands.add_parser("remove", help="unpublish maps")
    remove_command.add_argument("maps", nargs="+")
    args = parser.parse_args()
    if args.command == "publish":
        for build in args.builds:
            summary = json.loads((build / "build.json").read_text())
            print("published " + describe(publish(summary)))
    elif args.command == "list":
        for record in published().values():
            print(describe(record))
    else:
        for name in args.maps:
            remove(name)
            print("removed " + name)


if __name__ == "__main__":
    main()
