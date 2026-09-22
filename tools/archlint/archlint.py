#!/usr/bin/env python3
"""Architecture ratchet for RFC 0001.

Phase A intentionally starts with lexical checks.  Existing loader dependencies
are recorded as exact fingerprints; new occurrences fail without requiring the
legacy tree to be cleaned up in the same change.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
from collections import Counter
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable, Sequence


SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".inl", ".mm"}
EXCLUDED_PARTS = {".git", "build", "thirdparty", "waf3"}
EXCLUDED_PREFIXES = ("tools/archlint/tests/fixtures/",)


@dataclass(frozen=True)
class Rule:
    rule_id: str
    title: str
    pattern: re.Pattern[str]
    correction: str
    inventory: bool = True


RULES = (
    Rule(
        "ARCH101",
        "direct Sys_LoadModule use",
        re.compile(r"\bSys_LoadModule\s*\("),
        "Route loading through a named extension host or application composition.",
    ),
    Rule(
        "ARCH102",
        "direct Sys_GetFactory use",
        re.compile(r"\bSys_GetFactory\s*\("),
        "Bind a typed interface at the extension boundary instead of exporting the factory.",
    ),
    Rule(
        "ARCH103",
        "filesystem-mediated module load",
        re.compile(
            r"\b(?:g_p(?:Full)?FileSystem|m_pFileSystem|pFileSystem|"
            r"m_pMasterVMPIFileSystemPassThru|m_pFileSystemPassThru)\s*->\s*LoadModule\s*\("
        ),
        "Separate path resolution from loading and use a named extension host.",
    ),
    Rule(
        "ARCH104",
        "CDllDemandLoader use",
        re.compile(r"\bCDllDemandLoader\b"),
        "Use an explicitly owned provider or named extension host.",
    ),
    Rule(
        "ARCH105",
        "CreateInterfaceFn boundary expansion",
        re.compile(r"\bCreateInterfaceFn\b"),
        "Keep CreateInterfaceFn inside the legacy ABI package; expose a typed contract elsewhere.",
        inventory=False,
    ),
)

NATIVE_LOAD_PATTERN = re.compile(
    r"(?<![A-Za-z0-9_])(?:dlopen|Sys_LoadLibrary|LoadLibraryHandle|LoadLibrary|"
    r"LoadLibraryA|LoadLibraryW|LoadLibraryExA|LoadLibraryExW)\s*\("
)
LOAD_INTERFACE_PATTERN = re.compile(r"\bSys_LoadInterface\s*\(")


@dataclass(frozen=True)
class Occurrence:
    fingerprint: str
    rule: str
    path: str
    line: int
    excerpt: str


def repo_root_from_script() -> Path:
    return Path(__file__).resolve().parents[2]


def load_manifest(root: Path) -> dict:
    with (root / "architecture/modules.json").open(encoding="utf-8") as stream:
        return json.load(stream)


def is_source(path: Path, root: Path) -> bool:
    try:
        relative = path.relative_to(root)
    except ValueError:
        return False
    rel_text = relative.as_posix()
    if path.suffix.lower() not in SOURCE_SUFFIXES:
        return False
    if any(part in EXCLUDED_PARTS or part.startswith("build") for part in relative.parts[:-1]):
        return False
    return not rel_text.startswith(EXCLUDED_PREFIXES)


def source_files(root: Path, selected: Iterable[str] | None = None) -> list[Path]:
    if selected is not None:
        candidates = (root / name for name in selected)
        return sorted(path for path in candidates if path.is_file() and is_source(path, root))

    try:
        search = subprocess.run(
            [
                "rg",
                "-l",
                "--null",
                "-g",
                "*.c",
                "-g",
                "*.cc",
                "-g",
                "*.cpp",
                "-g",
                "*.cxx",
                "-g",
                "*.h",
                "-g",
                "*.hh",
                "-g",
                "*.hpp",
                "-g",
                "*.inl",
                "-g",
                "*.mm",
                "-g",
                "!thirdparty/**",
                "-g",
                "!build*/**",
                "-g",
                "!tools/archlint/tests/fixtures/**",
                r"Sys_LoadModule|Sys_LoadInterface|Sys_GetFactory|CDllDemandLoader|CreateInterfaceFn|LoadModule|dlopen|LoadLibrary",
                ".",
            ],
            cwd=root,
            text=False,
            capture_output=True,
            check=False,
        )
        if search.returncode in (0, 1):
            names = [name for name in search.stdout.decode("utf-8").split("\0") if name]
            return sorted(root / name.removeprefix("./") for name in names)
    except FileNotFoundError:
        pass

    candidates: list[Path] = []
    for directory, names, files in os.walk(root):
        directory_path = Path(directory)
        names[:] = [
            name
            for name in names
            if name not in EXCLUDED_PARTS and not name.startswith("build")
        ]
        candidates.extend(directory_path / name for name in files)
    return sorted(path for path in candidates if is_source(path, root))


def strip_comments_and_literals(text: str) -> str:
    """Replace comments and literal contents while preserving lines and identifiers."""
    result: list[str] = []
    index = 0
    state = "code"
    quote = ""
    while index < len(text):
        char = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and following == "/":
                result.extend("  ")
                index += 2
                state = "line_comment"
                continue
            if char == "/" and following == "*":
                result.extend("  ")
                index += 2
                state = "block_comment"
                continue
            if char in {'"', "'"}:
                quote = char
                result.append(char)
                index += 1
                state = "literal"
                continue
            result.append(char)
        elif state == "line_comment":
            if char == "\n":
                result.append("\n")
                state = "code"
            else:
                result.append(" ")
        elif state == "block_comment":
            if char == "*" and following == "/":
                result.extend("  ")
                index += 2
                state = "code"
                continue
            result.append("\n" if char == "\n" else " ")
        else:
            if char == "\\" and following:
                result.extend((" ", "\n" if following == "\n" else " "))
                index += 2
                continue
            if char == quote:
                result.append(char)
                state = "code"
            else:
                result.append("\n" if char == "\n" else " ")
        index += 1
    return "".join(result)


def normalized_excerpt(line: str) -> str:
    return " ".join(line.strip().split())


def make_fingerprint(rule_id: str, path: str, excerpt: str, ordinal: int) -> str:
    material = "\0".join((rule_id, path, excerpt, str(ordinal))).encode("utf-8")
    return hashlib.sha256(material).hexdigest()[:20]


def scan_file(root: Path, path: Path, manifest: dict) -> list[Occurrence]:
    relative = path.relative_to(root).as_posix()
    original = path.read_text(encoding="utf-8", errors="replace")
    stripped = strip_comments_and_literals(original)
    original_lines = original.splitlines()
    stripped_lines = stripped.splitlines()
    allowed_legacy = set(manifest["legacyAbi"]["paths"])
    repeated: Counter[tuple[str, str]] = Counter()
    occurrences: list[Occurrence] = []

    for line_number, stripped_line in enumerate(stripped_lines, start=1):
        source_line = original_lines[line_number - 1]
        excerpt = normalized_excerpt(source_line)
        for rule in RULES:
            if rule.rule_id == "ARCH105" and relative in allowed_legacy:
                continue
            matches = tuple(rule.pattern.finditer(stripped_line))
            for _ in matches:
                key = (rule.rule_id, excerpt)
                ordinal = repeated[key]
                repeated[key] += 1
                occurrences.append(
                    Occurrence(
                        make_fingerprint(rule.rule_id, relative, excerpt, ordinal),
                        rule.rule_id,
                        relative,
                        line_number,
                        excerpt,
                    )
                )
    return occurrences


def scan(root: Path, manifest: dict, selected: Iterable[str] | None = None) -> list[Occurrence]:
    found: list[Occurrence] = []
    for path in source_files(root, selected):
        found.extend(scan_file(root, path, manifest))
    return sorted(found, key=lambda item: (item.path, item.line, item.rule, item.fingerprint))


def read_baseline(root: Path) -> dict:
    with (root / "architecture/baseline.json").open(encoding="utf-8") as stream:
        return json.load(stream)


def baseline_document(occurrences: Sequence[Occurrence]) -> dict:
    return {
        "version": 1,
        "description": "Exact legacy loader and CreateInterfaceFn occurrences frozen by RFC 0001 Phase A.",
        "entries": [asdict(item) for item in occurrences],
    }


def changed_paths(root: Path, base: str | None) -> list[str]:
    commands = []
    if base:
        commands.append(["git", "diff", "--name-only", "--diff-filter=ACMR", f"{base}...HEAD"])
    else:
        commands.extend(
            [
                ["git", "diff", "--name-only", "--diff-filter=ACMR", "HEAD"],
                ["git", "ls-files", "--others", "--exclude-standard"],
            ]
        )
    paths: set[str] = set()
    for command in commands:
        result = subprocess.run(command, cwd=root, check=True, text=True, capture_output=True)
        paths.update(line for line in result.stdout.splitlines() if line)
    return sorted(paths)


def compare_baseline(
    current: Sequence[Occurrence], baseline: dict, selected: set[str] | None = None
) -> tuple[list[Occurrence], list[dict]]:
    baseline_entries = baseline["entries"]
    baseline_by_id = {entry["fingerprint"]: entry for entry in baseline_entries}
    current_by_id = {entry.fingerprint: entry for entry in current}
    new = [entry for entry in current if entry.fingerprint not in baseline_by_id]
    stale = [
        entry
        for entry in baseline_entries
        if entry["fingerprint"] not in current_by_id
        and (selected is None or entry["path"] in selected)
    ]
    return new, stale


def print_violation(prefix: str, item: Occurrence | dict) -> None:
    get = (lambda key: getattr(item, key)) if isinstance(item, Occurrence) else item.__getitem__
    rule = next(rule for rule in RULES if rule.rule_id == get("rule"))
    print(f"{get('path')}:{get('line')}: {rule.rule_id} {prefix}: {rule.title}")
    print(f"  {get('excerpt')}")
    print(f"  {rule.correction}")


def check_command(args: argparse.Namespace, root: Path, manifest: dict) -> int:
    selected = None
    if args.changed:
        selected = set(changed_paths(root, args.base))
    current = scan(root, manifest, selected)
    baseline = read_baseline(root)
    new, stale = compare_baseline(current, baseline, selected)
    for item in new:
        print_violation("new dependency", item)
    for item in stale:
        print_violation("stale baseline entry", item)
    if new or stale:
        print(f"archlint: failed with {len(new)} new and {len(stale)} stale occurrence(s)")
        return 1
    mode = "changed files" if args.changed else "repository"
    print(f"archlint: {mode} matches the RFC 0001 Phase A baseline")
    return 0


def verify_baseline(root: Path, manifest: dict) -> int:
    current = scan(root, manifest)
    new, stale = compare_baseline(current, read_baseline(root))
    for item in new:
        print_violation("new dependency", item)
    for item in stale:
        print_violation("stale baseline entry", item)
    if new or stale:
        print(f"archlint: baseline failed with {len(new)} new and {len(stale)} stale occurrence(s)")
        return 1
    print("archlint: baseline is current")
    return 0


def classify(path: str, rule_id: str, manifest: dict) -> tuple[str, str]:
    for classification in manifest["loaderInventory"]["classifications"]:
        applicable_rules = classification.get("rules")
        if applicable_rules is None or rule_id in applicable_rules:
            if any(path.startswith(prefix) for prefix in classification["pathPrefixes"]):
                return classification["name"], classification["reason"]
    return "first-party-composition", "First-party runtime module assembled by legacy application composition."


def supplemental_inventory_occurrences(root: Path, manifest: dict) -> list[dict]:
    results: list[dict] = []
    for path in source_files(root):
        relative = path.relative_to(root).as_posix()
        original = path.read_text(encoding="utf-8", errors="replace")
        stripped = strip_comments_and_literals(original)
        original_lines = original.splitlines()
        repeated: Counter[tuple[str, str]] = Counter()
        for line_number, stripped_line in enumerate(stripped.splitlines(), start=1):
            excerpt = normalized_excerpt(original_lines[line_number - 1])
            patterns = (
                ("INVENTORY_NATIVE_LOAD", "native-loader", NATIVE_LOAD_PATTERN),
                ("INVENTORY_LOAD_INTERFACE", "Sys_LoadInterface", LOAD_INTERFACE_PATTERN),
            )
            for inventory_rule, mechanism, pattern in patterns:
                if not pattern.search(stripped_line) or excerpt.startswith("#define"):
                    continue
                key = (inventory_rule, excerpt)
                ordinal = repeated[key]
                repeated[key] += 1
                fingerprint = make_fingerprint(inventory_rule, relative, excerpt, ordinal)
                category, reason = classify(relative, inventory_rule, manifest)
                results.append(
                    {
                        "fingerprint": fingerprint,
                        "mechanism": mechanism,
                        "path": relative,
                        "line": line_number,
                        "excerpt": excerpt,
                        "classification": category,
                        "reason": reason,
                    }
                )
    return results


def inventory_document(root: Path, manifest: dict) -> dict:
    sites: list[dict] = []
    rules_by_id = {rule.rule_id: rule for rule in RULES}
    for item in scan(root, manifest):
        rule = rules_by_id[item.rule]
        if not rule.inventory:
            continue
        category, reason = classify(item.path, item.rule, manifest)
        record = asdict(item)
        record["mechanism"] = item.rule
        record["classification"] = category
        record["reason"] = reason
        sites.append(record)
    sites.extend(supplemental_inventory_occurrences(root, manifest))
    sites.sort(key=lambda item: (item["path"], item["line"], item["mechanism"]))
    counts = Counter(item["classification"] for item in sites)
    return {
        "version": 1,
        "description": "Static dynamic-loader site inventory for RFC 0001 retirement Phase A.",
        "classificationCounts": dict(sorted(counts.items())),
        "sites": sites,
    }


def verify_or_write(
    path: Path, expected: dict, write: bool, label: str
) -> int:
    serialized = json.dumps(expected, indent=2, sort_keys=False) + "\n"
    if write:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(serialized, encoding="utf-8")
        print(f"archlint: wrote {label} to {path}")
        return 0
    actual = path.read_text(encoding="utf-8") if path.exists() else ""
    if actual != serialized:
        print(f"archlint: {label} is stale; review changes and run the matching --write command")
        return 1
    print(f"archlint: {label} is current")
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    check = subparsers.add_parser("check", help="reject new or stale baseline occurrences")
    mode = check.add_mutually_exclusive_group(required=True)
    mode.add_argument("--all", action="store_true")
    mode.add_argument("--changed", action="store_true")
    check.add_argument("--base", help="git base ref used with --changed")
    baseline = subparsers.add_parser("baseline", help="verify or intentionally rewrite the baseline")
    baseline.add_argument("--verify", action="store_true")
    baseline.add_argument("--write", action="store_true")
    inventory = subparsers.add_parser("inventory", help="verify or intentionally rewrite loader inventory")
    inventory.add_argument("--verify", action="store_true")
    inventory.add_argument("--write", action="store_true")
    return parser


def main(argv: Sequence[str] | None = None, root: Path | None = None) -> int:
    args = build_parser().parse_args(argv)
    root = root or repo_root_from_script()
    manifest = load_manifest(root)
    if args.command == "check":
        return check_command(args, root, manifest)
    if args.command == "baseline":
        if args.write == args.verify:
            raise SystemExit("baseline requires exactly one of --verify or --write")
        if args.verify:
            return verify_baseline(root, manifest)
        return verify_or_write(
            root / "architecture/baseline.json",
            baseline_document(scan(root, manifest)),
            args.write,
            "baseline",
        )
    if args.write == args.verify:
        raise SystemExit("inventory requires exactly one of --verify or --write")
    return verify_or_write(
        root / "architecture/loader_inventory.json",
        inventory_document(root, manifest),
        args.write,
        "loader inventory",
    )


if __name__ == "__main__":
    sys.exit(main())
