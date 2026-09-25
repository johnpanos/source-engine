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
import sys
from collections import Counter
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable, Sequence
sys.path.insert(0, str(Path(__file__).resolve().parent))
import capabilities


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
        re.compile(r"\bSys_LoadModule(?:WithContext)?\s*\("),
        "Route loading through a named extension host or application composition.",
    ),
    Rule(
        "ARCH102",
        "direct Sys_GetFactory use",
        re.compile(r"\bSys_GetFactory(?:WithContext)?\s*\("),
        "Bind a typed interface at the extension boundary instead of exporting the factory.",
    ),
    Rule(
        "ARCH103",
        "filesystem-mediated module load",
        re.compile(
            r"(?:\bSys_LoadModuleFromFileSystem(?:WithContext)?\s*\(|"
            r"\b(?:g_p(?:Full)?FileSystem|m_pFileSystem|pFileSystem|"
            r"m_pMasterVMPIFileSystemPassThru|m_pFileSystemPassThru)\s*->\s*LoadModule\s*\()"
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
    Rule(
        "ARCH106",
        "factory treated as an app-system module",
        re.compile(
            r"\bLoadModule\s*\(\s*(?:CreateInterfaceFn\s+\w+|Sys_GetFactoryThis\s*\(\s*\)|"
            r"VStdLib_GetICVarFactory\s*\(\s*\)|(?:[A-Za-z_]\w*)?[Ff]actory\s*(?:\(\s*\))?)\s*\)"
        ),
        "Inject the linked IAppSystem instance directly or use AddLegacyFactory at a retained ABI boundary.",
        inventory=False,
    ),
)

TOOL_MIGRATION_SCHEMA = "rfc0001-tool-migrations/v1"
TOOL_MIGRATION_STATUSES = [
    "inventoried",
    "planned",
    "active",
    "blocked",
    "verified",
    "retired",
]
TOOL_MIGRATION_DISPOSITIONS = [
    "normal-executable",
    "linked-tool-library",
    "child-process",
    "tool-extension-host",
    "tool-only-provider",
    "preserved-external-abi",
]
TOOL_MIGRATION_REQUIRED_COHORTS = {
    "phase-e-vrad",
    "phase-e-vvis",
    "phase-e-shadercompile",
    "phase-e-texturecompile",
    "phase-e-vtex-vtexconv",
    "phase-e-vmpi-worker",
    "phase-e-studiomdl",
    "phase-e-tool-dictionaries",
    "phase-e-tool-integrations",
}
TOOL_WRAPPER_SYMBOLS = ("ILaunchableDLL", "LAUNCHABLE_DLL_INTERFACE_VERSION")
TOOL_WRAPPER_PATTERN = re.compile(
    r"\b(?:" + "|".join(re.escape(symbol) for symbol in TOOL_WRAPPER_SYMBOLS) + r")\b"
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
                r"Sys_LoadModule|Sys_LoadInterface|Sys_GetFactory|CDllDemandLoader|CreateInterfaceFn|"
                r"LoadModule|dlopen|LoadLibrary|ILaunchableDLL|LAUNCHABLE_DLL_INTERFACE_VERSION",
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
        if stripped_line.lstrip().startswith("#"):
            continue
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


EXCEPTION_ROW = re.compile(r"^R[0-9]{2,3}$")
EXCEPTION_FIELDS = ("path", "rule", "count", "reason", "owner", "tracking", "removal")


def loader_exceptions(manifest: dict) -> list[dict]:
    return manifest.get("loaderExceptions", {}).get("entries", [])


def validate_loader_exceptions(root: Path, manifest: dict) -> list[str]:
    """RFC 0001 "Baseline and exceptions": one file, one rule, a reason, an
    owner row, a tracking record and a removal condition; never a glob."""
    errors: list[str] = []
    rule_ids = {rule.rule_id for rule in RULES}
    seen: set[tuple[str, str]] = set()
    for index, entry in enumerate(loader_exceptions(manifest)):
        where = f"loaderExceptions[{index}]"
        missing = [field for field in EXCEPTION_FIELDS if field not in entry]
        if missing:
            errors.append(f"{where}: missing {', '.join(missing)}")
            continue
        path, rule = entry["path"], entry["rule"]
        where = f"loaderExceptions {path} {rule}"
        if not isinstance(path, str) or any(ch in path for ch in "*?[") or path.endswith("/"):
            errors.append(f"{where}: path must name one file, not a glob or directory")
        elif not (root / path).is_file():
            errors.append(f"{where}: path does not exist")
        if rule not in rule_ids:
            errors.append(f"{where}: unknown rule")
        if (path, rule) in seen:
            errors.append(f"{where}: duplicate exception")
        seen.add((path, rule))
        count = entry["count"]
        if not isinstance(count, int) or isinstance(count, bool) or count < 1:
            errors.append(f"{where}: count must be a positive integer")
        if not isinstance(entry["owner"], str) or not EXCEPTION_ROW.match(entry["owner"]):
            errors.append(f"{where}: owner must be a roadmap row id such as R39")
        for field in ("reason", "removal"):
            if not isinstance(entry[field], str) or not entry[field].strip():
                errors.append(f"{where}: {field} must be a non-empty string")
        tracking = entry["tracking"]
        if not isinstance(tracking, str) or not (root / tracking.split("#", 1)[0]).is_file():
            errors.append(f"{where}: tracking must name an existing record")
    return errors


def apply_loader_exceptions(
    new: Sequence[Occurrence], manifest: dict, selected: set[str] | None = None
) -> tuple[list[Occurrence], list[str]]:
    """Remove the exact excepted occurrences from `new`. An exception covers
    exactly `count` unbaselined occurrences of its rule in its file: more or
    fewer is an error, and one that covers nothing is stale."""
    grouped: dict[tuple[str, str], list[Occurrence]] = {}
    for item in new:
        grouped.setdefault((item.path, item.rule), []).append(item)
    remaining: list[Occurrence] = []
    errors: list[str] = []
    exceptions = {(entry["path"], entry["rule"]): entry for entry in loader_exceptions(manifest)}
    for key, items in grouped.items():
        entry = exceptions.get(key)
        if entry is None:
            remaining.extend(items)
        elif len(items) != entry["count"]:
            errors.append(
                f"{key[0]}: {key[1]} exception allows exactly {entry['count']} occurrence(s), "
                f"found {len(items)}; review the change and update the exception or the code"
            )
            remaining.extend(items)
    for key, entry in exceptions.items():
        if key not in grouped and (selected is None or key[0] in selected):
            errors.append(
                f"{key[0]}: stale {key[1]} exception (no unbaselined occurrence); remove it"
            )
    return remaining, errors


def excepted_keys(manifest: dict) -> set[tuple[str, str]]:
    return {(entry["path"], entry["rule"]) for entry in loader_exceptions(manifest)}


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


def classify_drift(
    new: Sequence[Occurrence], stale: Sequence[dict]
) -> tuple[list[tuple[Occurrence, dict]], list[Occurrence], list[dict]]:
    """Separate baseline drift into relocations, genuinely new sites, and removals.

    A frozen loader occurrence carries its path in its fingerprint, so moving a
    file that contains a loader call surfaces as a (new, stale) pair with the same
    rule and normalized excerpt but a different path.  That is a *relocation* -- it
    needs only a reviewed baseline path update, not an architectural decision --
    and it should not be confused with a genuinely new loader call.  This makes the
    R01/R07 reconciliation legible instead of an opaque "N new / M stale" count.
    """
    stale_by_call: dict[tuple[str, str], list[dict]] = {}
    for entry in stale:
        stale_by_call.setdefault((entry["rule"], entry["excerpt"]), []).append(entry)
    relocations: list[tuple[Occurrence, dict]] = []
    genuinely_new: list[Occurrence] = []
    matched: set[int] = set()
    for occurrence in new:
        candidates = stale_by_call.get((occurrence.rule, occurrence.excerpt), [])
        match = next(
            (entry for entry in candidates if id(entry) not in matched and entry["path"] != occurrence.path),
            None,
        )
        if match is not None:
            matched.add(id(match))
            relocations.append((occurrence, match))
        else:
            genuinely_new.append(occurrence)
    removed = [entry for entry in stale if id(entry) not in matched]
    return relocations, genuinely_new, removed


def print_drift_triage(new: Sequence[Occurrence], stale: Sequence[dict]) -> None:
    relocations, genuinely_new, removed = classify_drift(new, stale)
    if not (relocations or genuinely_new or removed):
        return
    print(
        f"archlint: drift triage -> {len(relocations)} relocated, "
        f"{len(genuinely_new)} genuinely new, {len(removed)} removed"
    )
    for occurrence, entry in relocations:
        print(f"  relocated {occurrence.rule}: {entry['path']}:{entry['line']} -> {occurrence.path}:{occurrence.line}")
        print(f"    {occurrence.excerpt}")
    if relocations:
        print("  (relocations are the same frozen call in a moved file: reconcile with a reviewed baseline path update)")


def print_violation(prefix: str, item: Occurrence | dict) -> None:
    get = (lambda key: getattr(item, key)) if isinstance(item, Occurrence) else item.__getitem__
    rule = next(rule for rule in RULES if rule.rule_id == get("rule"))
    print(f"{get('path')}:{get('line')}: {rule.rule_id} {prefix}: {rule.title}")
    print(f"  {get('excerpt')}")
    print(f"  {rule.correction}")


def check_command(args: argparse.Namespace, root: Path, manifest: dict) -> int:
    strict_errors = capabilities.check(root, manifest.get("capabilityModules"), strip_comments_and_literals)
    tool_errors = validate_tool_migrations(root, read_tool_migrations(root))
    for tree in getattr(args, "compile_deps", []):
        tree_path = (root / tree).resolve()
        depfiles = [(tree_path, path) for path in sorted(tree_path.rglob("*.d"))]
        if not depfiles:
            strict_errors.append(f"CAP005 {tree}: no compiler dependency files; build the tree first")
            continue
        dep_errors, checked = capabilities.compile_dep_errors(
            root, manifest.get("capabilityModules"), depfiles, strip_comments_and_literals
        )
        if checked == 0:
            dep_errors.append(f"CAP005 {tree}: no strict translation units found in the dependency files")
        print(f"archlint: compile-deps {tree}: {len(depfiles)} dependency files, {checked} strict units")
        strict_errors.extend(dep_errors)
        invocations = tree_path / "toolchain-invocations.json"
        if invocations.is_file():
            link_errors, judged, skipped = capabilities.link_graph_errors(
                manifest.get("capabilityModules"), json.loads(invocations.read_text(encoding="utf-8"))
            )
            print(f"archlint: link-graph {tree}: {judged} portable strict targets judged, "
                  f"{skipped} mixed/native targets not judged")
            strict_errors.extend(link_errors)
    for error in strict_errors:
        print(error)
    for error in tool_errors:
        print(f"archlint: [tools] {error}")
    selected = None
    if args.changed:
        selected = set(changed_paths(root, args.base))
    current = scan(root, manifest, selected)
    baseline = read_baseline(root)
    new, stale = compare_baseline(current, baseline, selected)
    exception_errors = validate_loader_exceptions(root, manifest)
    new, applied_errors = apply_loader_exceptions(new, manifest, selected)
    exception_errors += applied_errors
    for error in exception_errors:
        print(f"archlint: [exceptions] {error}")
    for item in new:
        print_violation("new dependency", item)
    for item in stale:
        print_violation("stale baseline entry", item)
    if new or stale or strict_errors or tool_errors or exception_errors:
        print(f"archlint: failed with {len(new)} new and {len(stale)} stale occurrence(s)")
        print_drift_triage(new, stale)
        return 1
    mode = "changed files" if args.changed else "repository"
    print(f"archlint: {mode} matches the RFC 0001 Phase A baseline")
    return 0


def verify_baseline(root: Path, manifest: dict) -> int:
    current = scan(root, manifest)
    new, stale = compare_baseline(current, read_baseline(root))
    exception_errors = validate_loader_exceptions(root, manifest)
    new, applied_errors = apply_loader_exceptions(new, manifest)
    exception_errors += applied_errors
    for error in exception_errors:
        print(f"archlint: [exceptions] {error}")
    for item in new:
        print_violation("new dependency", item)
    for item in stale:
        print_violation("stale baseline entry", item)
    if new or stale or exception_errors:
        print(f"archlint: baseline failed with {len(new)} new and {len(stale)} stale occurrence(s)")
        print_drift_triage(new, stale)
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


NATIVE_TELEMETRY_DELEGATIONS = {
    "dedicated/isys.h": "delegates-to-dedicated-native-adapter",
    "dedicated/sys_common.cpp": "delegates-to-dedicated-native-adapter",
    "engine/audio/voice_record_dsound.cpp": "shared-audio-pch-native-adapter",
    "tier1/interface.cpp": "tier1-shared-tracker",
}
NATIVE_TELEMETRY_PROVIDERS = {
    "public/tier0/bootstrap_module_load_telemetry.h": "bootstrap-provider-boundary",
    "public/tier0/native_module_load_telemetry.h": "tier0-provider-boundary",
    "tier0/module_load_telemetry.cpp": "tier0-provider-boundary",
    "external/vpc/public/tier0/native_module_load_telemetry.h": "vpc-provider-boundary",
}


def native_telemetry_coverage(
    root: Path, path: str, original: str, excerpt: str
) -> str:
    """Return the concrete runtime adapter covering one raw native loader site."""
    if path in NATIVE_TELEMETRY_PROVIDERS:
        return NATIVE_TELEMETRY_PROVIDERS[path]
    if path in NATIVE_TELEMETRY_DELEGATIONS:
        return NATIVE_TELEMETRY_DELEGATIONS[path]
    if path.startswith("external/vpc/"):
        wide_loader = re.search(r"\b(LoadLibraryW|LoadLibraryExW)\s*\(", excerpt)
        if wide_loader:
            provider = (
                root / "external/vpc/public/tier0/native_module_load_telemetry.h"
            ).read_text(encoding="utf-8", errors="replace")
            if f"#define {wide_loader.group(1)}(" not in provider:
                return "missing"
        return "vpc-platform-native-adapter"
    if "tier0/bootstrap_module_load_telemetry.h" in original:
        return "bootstrap-native-adapter"
    if "tier0/native_module_load_telemetry.h" in original:
        return "tier0-native-adapter"
    return "missing"


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
                record = {
                        "fingerprint": fingerprint,
                        "mechanism": mechanism,
                        "path": relative,
                        "line": line_number,
                        "excerpt": excerpt,
                        "classification": category,
                        "reason": reason,
                    }
                if mechanism == "native-loader":
                    record["telemetry"] = native_telemetry_coverage(
                        root, relative, original, excerpt
                    )
                results.append(record)
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
    native_sites = [item for item in sites if item["mechanism"] == "native-loader"]
    covered_native_sites = [
        item for item in native_sites if item.get("telemetry") != "missing"
    ]
    return {
        "version": 1,
        "description": "Static dynamic-loader site inventory for RFC 0001 retirement Phase A.",
        "classificationCounts": dict(sorted(counts.items())),
        "nativeTelemetryCoverage": {
            "covered": len(covered_native_sites),
            "total": len(native_sites),
            "status": "complete"
            if len(covered_native_sites) == len(native_sites)
            else "partial",
        },
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


# ---------------------------------------------------------------------------
# RFC 0001 retirement Phase E: tool executable/process migration.
#
# The authored ledger is the single owner of cohort disposition and progress.
# ARCH107 is an exact occurrence ratchet over the old ILaunchableDLL ABI: the
# two public ABI declarations and every still-live wrapper use must be claimed
# explicitly.  A directory allowlist would let a new wrapper hide beside an old
# one, so claims include per-symbol occurrence counts.
# ---------------------------------------------------------------------------


@dataclass(frozen=True)
class ToolWrapperOccurrence:
    symbol: str
    path: str
    line: int
    excerpt: str


def read_tool_migrations(root: Path) -> dict:
    path = root / "architecture/tool_migrations.json"
    if not path.is_file():
        return {}
    try:
        with path.open(encoding="utf-8") as stream:
            document = json.load(stream)
    except (OSError, json.JSONDecodeError) as error:
        return {"_loadError": str(error)}
    return document if isinstance(document, dict) else {"_loadError": "root must be an object"}


def scan_tool_wrapper_occurrences(root: Path) -> list[ToolWrapperOccurrence]:
    occurrences: list[ToolWrapperOccurrence] = []
    for path in source_files(root):
        relative = path.relative_to(root).as_posix()
        original = path.read_text(encoding="utf-8", errors="replace")
        stripped = strip_comments_and_literals(original)
        original_lines = original.splitlines()
        for line_number, stripped_line in enumerate(stripped.splitlines(), start=1):
            for match in TOOL_WRAPPER_PATTERN.finditer(stripped_line):
                occurrences.append(
                    ToolWrapperOccurrence(
                        symbol=match.group(0),
                        path=relative,
                        line=line_number,
                        excerpt=normalized_excerpt(original_lines[line_number - 1]),
                    )
                )
    return sorted(
        occurrences,
        key=lambda item: (item.path, item.line, item.symbol, item.excerpt),
    )


def _string_list(value: object) -> bool:
    return isinstance(value, list) and bool(value) and all(
        isinstance(item, str) and bool(item.strip()) for item in value
    )


def validate_tool_migrations(root: Path, ledger: dict) -> list[str]:
    errors: list[str] = []
    if ledger.get("_loadError"):
        return [f"tool migration ledger cannot be read: {ledger['_loadError']}"]
    if ledger.get("version") != 1:
        errors.append("tool migration ledger version must be 1")
    if ledger.get("schema") != TOOL_MIGRATION_SCHEMA:
        errors.append(f"tool migration ledger schema must be {TOOL_MIGRATION_SCHEMA!r}")
    if ledger.get("statuses") != TOOL_MIGRATION_STATUSES:
        errors.append("tool migration statuses do not match the canonical state machine")
    if ledger.get("dispositions") != TOOL_MIGRATION_DISPOSITIONS:
        errors.append("tool migration dispositions do not match the canonical set")

    declarations = ledger.get("abiDeclarations")
    if not isinstance(declarations, list) or not declarations:
        errors.append("tool migration ledger must declare preserved launchable-DLL ABI headers")
        declarations = []
    cohorts = ledger.get("cohorts")
    if not isinstance(cohorts, list):
        errors.append("tool migration ledger 'cohorts' must be a list")
        cohorts = []

    cohort_ids = [
        record.get("id") for record in cohorts if isinstance(record, dict) and record.get("id")
    ]
    for cohort_id, count in Counter(cohort_ids).items():
        if count > 1:
            errors.append(f"tool migration cohort {cohort_id!r} declared {count} times")
    missing_cohorts = sorted(TOOL_MIGRATION_REQUIRED_COHORTS - set(cohort_ids))
    for cohort_id in missing_cohorts:
        errors.append(f"required tool migration cohort {cohort_id!r} is missing")

    claimed_counts: dict[tuple[str, str], int] = {}
    claimed_owners: dict[tuple[str, str], str] = {}

    def add_claims(site: object, owner: str, require_delete_when: bool) -> None:
        if not isinstance(site, dict):
            errors.append(f"{owner}: wrapper-site record must be an object")
            return
        path = site.get("path")
        if not isinstance(path, str) or not path:
            errors.append(f"{owner}: wrapper-site record is missing a path")
            return
        source = root / path
        if not source.is_file():
            errors.append(f"{owner}: wrapper-site path {path!r} is not a source file")
        if not isinstance(site.get("reason"), str) or not site["reason"].strip():
            errors.append(f"{owner}: wrapper-site {path!r} is missing a reason")
        if require_delete_when and not _string_list(site.get("deleteWhen")):
            errors.append(f"{owner}: ABI declaration {path!r} needs non-empty deleteWhen")
        counts = site.get("symbolCounts")
        if not isinstance(counts, dict) or not counts:
            errors.append(f"{owner}: wrapper-site {path!r} needs symbolCounts")
            return
        for symbol, count in counts.items():
            if symbol not in TOOL_WRAPPER_SYMBOLS:
                errors.append(f"{owner}: wrapper-site {path!r} names unknown symbol {symbol!r}")
                continue
            if type(count) is not int or count <= 0:
                errors.append(
                    f"{owner}: wrapper-site {path!r} count for {symbol} must be a positive integer"
                )
                continue
            key = (path, symbol)
            if key in claimed_counts:
                errors.append(
                    f"ARCH107 {path}: {symbol} is claimed by both "
                    f"{claimed_owners[key]} and {owner}"
                )
                continue
            claimed_counts[key] = count
            claimed_owners[key] = owner

    for index, declaration in enumerate(declarations):
        add_claims(declaration, f"abiDeclarations[{index}]", True)

    for index, cohort in enumerate(cohorts):
        if not isinstance(cohort, dict):
            errors.append(f"cohorts[{index}] must be an object")
            continue
        cohort_id = cohort.get("id", f"cohorts[{index}]")
        context = f"cohort {cohort_id!r}"
        for field in ("title", "scope", "disposition", "status"):
            if not isinstance(cohort.get(field), str) or not cohort[field].strip():
                errors.append(f"{context}: missing non-empty {field!r}")
        disposition = cohort.get("disposition")
        if disposition not in TOOL_MIGRATION_DISPOSITIONS:
            errors.append(f"{context}: invalid disposition {disposition!r}")
        status = cohort.get("status")
        if status not in TOOL_MIGRATION_STATUSES:
            errors.append(f"{context}: invalid status {status!r}")

        source_paths = cohort.get("sourcePaths")
        if not _string_list(source_paths):
            errors.append(f"{context}: sourcePaths must be a non-empty string list")
            source_paths = []
        elif len(source_paths) != len(set(source_paths)):
            errors.append(f"{context}: sourcePaths contains duplicates")
        for path in source_paths:
            if not (root / path).exists():
                errors.append(f"{context}: source path {path!r} does not exist")

        deletion_conditions = cohort.get("deletionConditions")
        if not _string_list(deletion_conditions):
            errors.append(f"{context}: deletionConditions must be a non-empty string list")
        acceptance = cohort.get("acceptanceEvidence")
        if not _string_list(acceptance):
            errors.append(f"{context}: acceptanceEvidence must be a non-empty string list")
        evidence = cohort.get("evidence")
        valid_evidence = 0
        if not isinstance(evidence, list):
            errors.append(f"{context}: evidence must be a list")
            evidence = []
        for evidence_index, record in enumerate(evidence):
            evidence_context = f"{context} evidence[{evidence_index}]"
            if not isinstance(record, dict):
                errors.append(f"{evidence_context}: record must be an object")
                continue
            valid = True
            for field in ("path", "sourceRevision", "profile"):
                if not isinstance(record.get(field), str) or not record[field].strip():
                    errors.append(f"{evidence_context}: missing non-empty {field!r}")
                    valid = False
            if not _string_list(record.get("testSelectors")):
                errors.append(f"{evidence_context}: testSelectors must be a non-empty string list")
                valid = False
            evidence_path = record.get("path")
            if isinstance(evidence_path, str) and evidence_path:
                if not (root / evidence_path).is_file():
                    errors.append(
                        f"{evidence_context}: evidence path {evidence_path!r} does not exist"
                    )
                    valid = False
            if valid:
                valid_evidence += 1
        remaining = cohort.get("remainingWork")
        if not isinstance(remaining, list) or not all(
            isinstance(item, str) and bool(item.strip()) for item in remaining
        ):
            errors.append(f"{context}: remainingWork must be a string list")
            remaining = []

        wrapper_sites = cohort.get("wrapperSites")
        if not isinstance(wrapper_sites, list):
            errors.append(f"{context}: wrapperSites must be a list")
            wrapper_sites = []
        for site in wrapper_sites:
            add_claims(site, context, False)
            if isinstance(site, dict) and site.get("path") not in source_paths:
                errors.append(
                    f"{context}: wrapper-site {site.get('path')!r} is absent from sourcePaths"
                )

        if status == "retired":
            if wrapper_sites:
                errors.append(f"{context}: retired cohort still declares wrapperSites")
            if remaining:
                errors.append(f"{context}: retired cohort still declares remainingWork")
            if valid_evidence == 0:
                errors.append(f"{context}: retired cohort requires current acceptance evidence")
        elif not remaining:
            errors.append(f"{context}: non-retired cohort must state remainingWork")
        if status == "verified" and valid_evidence == 0:
            errors.append(f"{context}: verified cohort requires current acceptance evidence")

    actual_occurrences = scan_tool_wrapper_occurrences(root)
    actual_counts = Counter((item.path, item.symbol) for item in actual_occurrences)
    first_occurrence = {
        (item.path, item.symbol): item for item in actual_occurrences
    }
    for key, count in sorted(actual_counts.items()):
        if key not in claimed_counts:
            occurrence = first_occurrence[key]
            errors.append(
                f"ARCH107 {occurrence.path}:{occurrence.line}: unregistered "
                f"{occurrence.symbol} use ({count} occurrence(s)): {occurrence.excerpt}"
            )
    for key, expected in sorted(claimed_counts.items()):
        actual = actual_counts.get(key, 0)
        if actual != expected:
            path, symbol = key
            errors.append(
                f"ARCH107 {path}: ledger expects {expected} {symbol} occurrence(s), "
                f"found {actual}"
            )
    return errors


def tool_migrations_command(root: Path) -> int:
    ledger = read_tool_migrations(root)
    errors = validate_tool_migrations(root, ledger)
    if errors:
        for message in errors:
            print(f"archlint: [tools] {message}")
        print(f"archlint: tool migration verification failed with {len(errors)} problem(s)")
        return 1
    print(
        "archlint: RFC 0001 Phase E tool cohort ledger and launchable-DLL "
        f"ratchet are valid ({len(ledger['cohorts'])} cohorts; "
        f"{len(scan_tool_wrapper_occurrences(root))} preserved occurrences)"
    )
    return 0


# ---------------------------------------------------------------------------
# RFC 0002 editor migration enforcement (HAM rules).
#
# These checks validate the authored editor inventory, migration ledger, module
# graph, and compatibility declarations, and run the HAM003 lexical scan for
# native/toolkit tokens inside strict include roots.  They do not replace the
# loader freeze above; they extend the same tool as required by RFC 0002.
# ---------------------------------------------------------------------------

HAM_RESPONSIBILITIES = {
    "geometry", "scene", "editing", "interaction", "presentation", "persistence",
    "asset-service", "platform-adapter", "composition", "selection-policy",
}
HAM_EFFECTS = {
    "pure-computation", "document-mutation", "file-io", "process-execution",
    "gpu-work", "native-ui", "async-delivery",
}
HAM_STATES = {
    "authoritative-document-data", "per-document-session-state", "per-view-state",
    "derived-cache", "external-resource",
}
HAM_FACTORIZATIONS = {"keep", "extract", "split", "adapt-legacy", "replace-shell", "retire"}
HAM_EVIDENCE = {"observed", "hypothesis", "decision"}
HAM_STATUSES = [
    "inventoried", "characterized", "isolated", "extracted", "substitutable",
    "cutover", "retired",
]


def _load_json(root: Path, relative: str) -> dict:
    with (root / relative).open(encoding="utf-8") as stream:
        return json.load(stream)


def hammer_modules(manifest: dict) -> dict:
    return manifest["hammerModules"]


def _detect_cycle(edges: dict[str, list[str]]) -> list[str]:
    """Return a cyclic path if the directed graph has one, else []."""
    color: dict[str, int] = {}
    order: list[str] = []

    def visit(node: str) -> list[str]:
        color[node] = 1
        order.append(node)
        for nxt in edges.get(node, []):
            if nxt not in edges:
                continue
            if color.get(nxt) == 1:
                start = order.index(nxt)
                return order[start:] + [nxt]
            if color.get(nxt, 0) == 0:
                found = visit(nxt)
                if found:
                    return found
        color[node] = 2
        order.pop()
        return []

    for node in edges:
        if color.get(node, 0) == 0:
            found = visit(node)
            if found:
                return found
    return []


def validate_module_graph(module_block: dict, external: frozenset[str] = frozenset()) -> list[str]:
    """Validate a module block. `external` names modules registered elsewhere in
    the manifest (capabilityModules) that this block's modules may depend on."""
    errors: list[str] = []
    modules = module_block.get("modules", [])
    ids = [module["id"] for module in modules]
    known = set(ids)
    for module_id, count in Counter(ids).items():
        if count > 1:
            errors.append(f"module {module_id} declared {count} times")
        if module_id in external:
            errors.append(f"module {module_id} is also a registered capability module")
    edges: dict[str, list[str]] = {}
    for module in modules:
        edges[module["id"]] = list(module.get("allowedEdges", []))
        for target in module.get("allowedEdges", []):
            if target not in known and target not in external:
                errors.append(f"module {module['id']} allows edge to unknown module {target}")
            if target == module["id"]:
                errors.append(f"module {module['id']} declares a self edge")
    cycle = _detect_cycle(edges)
    if cycle:
        errors.append("module dependency cycle: " + " -> ".join(cycle))
    return errors


def validate_inventory(root: Path, module_block: dict, inventory: dict) -> list[str]:
    errors: list[str] = []
    known = {module["id"] for module in module_block.get("modules", [])}
    files = inventory.get("files", [])
    seen: set[str] = set()

    def check_owner(context: str, owner: str) -> None:
        if owner not in known:
            errors.append(f"{context}: unknown module {owner!r}")

    for record in files:
        path = record.get("path", "<missing path>")
        if path in seen:
            errors.append(f"{path}: duplicate inventory record (each file resolves to one owner)")
        seen.add(path)
        if not (root / path).is_file():
            errors.append(f"{path}: inventory record does not correspond to a source file")
        check_owner(f"{path} currentOwner", record.get("currentOwner", ""))
        check_owner(f"{path} destinationModule", record.get("destinationModule", ""))
        if record.get("responsibility") not in HAM_RESPONSIBILITIES:
            errors.append(f"{path}: invalid responsibility {record.get('responsibility')!r}")
        for effect in record.get("effects", []):
            if effect not in HAM_EFFECTS:
                errors.append(f"{path}: invalid effect {effect!r}")
        if record.get("state") not in HAM_STATES:
            errors.append(f"{path}: invalid state {record.get('state')!r}")
        if record.get("factorization") not in HAM_FACTORIZATIONS:
            errors.append(f"{path}: invalid factorization {record.get('factorization')!r}")
        if record.get("evidence") not in HAM_EVIDENCE:
            errors.append(f"{path}: invalid evidence {record.get('evidence')!r}")
        for split in record.get("symbolSplits", []):
            if "symbol" not in split:
                errors.append(f"{path}: symbol split missing 'symbol'")
            check_owner(f"{path} split {split.get('symbol')}", split.get("destinationModule", ""))

    coverage = inventory.get("coverage", {})
    status = coverage.get("status")
    if status not in {"partial", "complete"}:
        errors.append(f"inventory coverage status must be 'partial' or 'complete', got {status!r}")
    if coverage.get("filesClassified") != len(files):
        errors.append(
            f"inventory coverage.filesClassified={coverage.get('filesClassified')} "
            f"does not match {len(files)} file records"
        )
    if status == "complete":
        total = coverage.get("totalHammerSourceFiles")
        if len(files) != total:
            errors.append(
                f"inventory claims complete coverage but classifies {len(files)} of {total} files"
            )
    return errors


def validate_migrations(root: Path, module_block: dict, migrations: dict) -> list[str]:
    errors: list[str] = []
    known_modules = {module["id"] for module in module_block.get("modules", [])}
    records = migrations.get("migrations", [])
    if migrations.get("statuses") != HAM_STATUSES:
        errors.append("migration ledger statuses do not match the canonical state machine")
    ids = [record["id"] for record in records]
    known_ids = set(ids)
    for migration_id, count in Counter(ids).items():
        if count > 1:
            errors.append(f"migration {migration_id} declared {count} times")
    edges: dict[str, list[str]] = {}
    for record in records:
        rid = record.get("id", "<missing id>")
        for field in ("title", "phase", "responsibility", "evidenceKind", "status", "authority"):
            if not record.get(field):
                errors.append(f"migration {rid}: missing required field {field!r}")
        if record.get("status") not in HAM_STATUSES:
            errors.append(f"migration {rid}: invalid status {record.get('status')!r}")
        if record.get("evidenceKind") not in HAM_EVIDENCE:
            errors.append(f"migration {rid}: invalid evidenceKind {record.get('evidenceKind')!r}")
        destination = record.get("destinationModule")
        if destination is not None and destination not in known_modules:
            errors.append(f"migration {rid}: unknown destinationModule {destination!r}")
        for dependency in record.get("dependsOn", []):
            if dependency not in known_ids:
                errors.append(f"migration {rid}: dependsOn unknown migration {dependency!r}")
        edges[rid] = list(record.get("dependsOn", []))
        for source in record.get("sources", []):
            spath = source.get("path")
            if spath and not (root / spath).exists():
                errors.append(f"migration {rid}: source path {spath!r} does not exist")
        if "retireWhen" in record and not record["retireWhen"]:
            errors.append(f"migration {rid}: retireWhen is present but empty")
    cycle = _detect_cycle(edges)
    if cycle:
        errors.append("migration dependency cycle: " + " -> ".join(cycle))
    return errors


def validate_compatibility(compatibility: dict) -> list[str]:
    errors: list[str] = []
    if not compatibility.get("profiles"):
        errors.append("compatibility declares no profiles")
    valid_status = {"target", "planned", "unverified", "verified", "supported", "retired"}
    for profile in compatibility.get("profiles", []):
        if "id" not in profile:
            errors.append("compatibility profile missing 'id'")
        if profile.get("status") not in valid_status:
            errors.append(f"compatibility profile {profile.get('id')}: invalid status {profile.get('status')!r}")
    for gate in compatibility.get("gates", []):
        if "phase" not in gate:
            errors.append("compatibility gate missing 'phase'")
    return errors


def strict_root_files(root: Path, strict_roots: Sequence[str]) -> list[Path]:
    """Every source file under a strict include root, independent of loader filters."""
    found: list[Path] = []
    for prefix in strict_roots:
        base = root / prefix
        if not base.is_dir():
            continue
        for path in base.rglob("*"):
            if path.is_file() and is_source(path, root):
                found.append(path)
    return sorted(set(found))


def scan_native_tokens(root: Path, strict_roots: Sequence[str], tokens: Sequence[str]) -> list[Occurrence]:
    """HAM003: reject native/toolkit tokens inside strict editor include roots."""
    token_pattern = re.compile(
        "|".join(sorted((re.escape(token) for token in tokens), key=len, reverse=True))
    )
    occurrences: list[Occurrence] = []
    for path in strict_root_files(root, strict_roots):
        relative = path.relative_to(root).as_posix()
        original_text = path.read_text(encoding="utf-8", errors="replace")
        stripped = strip_comments_and_literals(original_text)
        original_lines = original_text.splitlines()
        repeated: Counter[tuple[str, str]] = Counter()
        for line_number, stripped_line in enumerate(stripped.splitlines(), start=1):
            if not token_pattern.search(stripped_line):
                continue
            excerpt = normalized_excerpt(original_lines[line_number - 1])
            key = ("HAM003", excerpt)
            ordinal = repeated[key]
            repeated[key] += 1
            occurrences.append(
                Occurrence(
                    make_fingerprint("HAM003", relative, excerpt, ordinal),
                    "HAM003",
                    relative,
                    line_number,
                    excerpt,
                )
            )
    return sorted(occurrences, key=lambda item: (item.path, item.line, item.fingerprint))


def hammer_native_token_report(
    root: Path, module_block: dict, baseline: dict
) -> tuple[list[Occurrence], list[dict]]:
    occurrences = scan_native_tokens(
        root, module_block["strictIncludeRoots"], module_block["nativeTokens"]
    )
    baseline_ids = {entry["fingerprint"] for entry in baseline.get("entries", [])}
    current_ids = {occurrence.fingerprint for occurrence in occurrences}
    new = [occurrence for occurrence in occurrences if occurrence.fingerprint not in baseline_ids]
    stale = [entry for entry in baseline.get("entries", []) if entry["fingerprint"] not in current_ids]
    return new, stale


# ---------------------------------------------------------------------------
# RFC 0002 inventory coverage assistance (HAM-INVENTORY-001).
#
# The `hammer --verify` path proves the *authored* inventory is internally
# consistent.  It does not tell an author which of the ~452 hammer source files
# are still unclassified, nor help characterize them.  That gap is the H0 exit
# bottleneck.  These helpers enumerate the source universe, diff it against the
# inventory, and emit *reviewable* stubs.  Every suggested field is a hypothesis
# (evidence "hypothesis") grounded only in a lexical scan; the author remains the
# authority and must confirm each record before coverage flips to "complete".
# ---------------------------------------------------------------------------

HAMMER_UNIVERSE_ROOTS = ("hammer/",)

# Lexically grounded effect signals, scanned over comment/literal-stripped source
# so a token inside a string or comment cannot create a false positive.
EFFECT_SIGNALS: tuple[tuple[str, re.Pattern[str]], ...] = (
    (
        "native-ui",
        re.compile(
            r"\b(?:afxwin\.h|afxext\.h|afx\.h|windows\.h|CWnd|CDialog|CDocument|CView|CDC|"
            r"CPalette|HWND|HDC|__declspec|GtkWidget|GdkGLContext|GtkGLArea|CVGuiWnd)\b"
            r"|gtk/gtk\.h|vgui::"
        ),
    ),
    (
        "file-io",
        re.compile(
            r"\b(?:fopen|fread|fwrite|fclose|ifstream|ofstream|fstream|CUtlBuffer|"
            r"ReadFile|WriteFile|LoadFile|SaveFile|g_pFileSystem|m_pFileSystem)\b"
        ),
    ),
    (
        "process-execution",
        re.compile(
            r"\b(?:system|CreateProcess[AW]?|ShellExecute[AW]?|_spawn\w*|_popen|popen|"
            r"fork|execv[pe]?|execl[pe]?|RunCommand)\b"
        ),
    ),
    (
        "gpu-work",
        re.compile(
            r"\b(?:IMaterialSystem|IMatRenderContext|IMesh|CMeshBuilder|IShaderAPI|"
            r"BeginRender\w*|glBegin|glDraw\w*)\b"
        ),
    ),
)

# Filename hints for the semantic fields a lexical scan cannot reliably infer.
# First match wins.  These are guesses and are always flagged in reviewTODO.
RESPONSIBILITY_HINTS: tuple[tuple[re.Pattern[str], str], ...] = (
    (re.compile(r"(?i)(boundbox|brush|solid|disp|face|vertex|winding|convex|polygon|geom)"), "geometry"),
    (re.compile(r"(?i)(dlg|dialog|sheet|statusbar|toolbar|\bwnd\b|mainfrm|control|options|propert)"), "presentation"),
    (re.compile(r"(?i)(tool|select|drag|handle|morph|\bclip\b|manip|gizmo)"), "interaction"),
    (re.compile(r"(?i)(mapdoc|mapworld|mapentity|mapgroup|mapatom|mapclass|world|entity|\btree\b|scene)"), "scene"),
    (re.compile(r"(?i)(save|load|serial|\bvmf\b|mapfile|\brmf\b|chunk|export|import|reader|writer)"), "persistence"),
    (re.compile(r"(?i)(view|render|camera|\bdraw\b|paint|\bgrid\b)"), "presentation"),
    (re.compile(r"(?i)(hammer\.cpp|\bmain\b|\bapp\b|manifest|factory|\binit\b|options)"), "composition"),
)
STATE_HINTS: tuple[tuple[re.Pattern[str], str], ...] = (
    (re.compile(r"(?i)(mapdoc|mapworld|mapentity|mapatom|mapclass|world|entity)"), "authoritative-document-data"),
    (re.compile(r"(?i)(view|camera|render2d|render3d)"), "per-view-state"),
    (re.compile(r"(?i)cache"), "derived-cache"),
)

# Fallback values used when no filename hint matches.  Always listed in
# reviewTODO so nothing is mistaken for a confirmed classification.
DEFAULT_RESPONSIBILITY = "editing"
DEFAULT_STATE = "per-document-session-state"


def hammer_source_universe(root: Path, module_block: dict) -> list[str]:
    """Every source file that belongs to the editor, as repo-relative posix paths."""
    roots = list(HAMMER_UNIVERSE_ROOTS) + list(module_block.get("strictIncludeRoots", []))
    found: set[str] = set()
    for prefix in roots:
        base = root / prefix
        if not base.is_dir():
            continue
        for path in base.rglob("*"):
            if path.is_file() and is_source(path, root):
                found.add(path.relative_to(root).as_posix())
    return sorted(found)


def detect_effects(root: Path, relative: str) -> list[str]:
    text = strip_comments_and_literals((root / relative).read_text(encoding="utf-8", errors="replace"))
    effects = [effect for effect, pattern in EFFECT_SIGNALS if pattern.search(text)]
    return effects or ["pure-computation"]


def _first_hint(relative: str, hints: Sequence[tuple[re.Pattern[str], str]]) -> str | None:
    stem = relative.rsplit("/", 1)[-1]
    for pattern, value in hints:
        if pattern.search(stem):
            return value
    return None


def suggest_record(root: Path, module_block: dict, relative: str) -> dict:
    """A reviewable inventory stub.  Every semantic field is a hypothesis."""
    effects = detect_effects(root, relative)
    responsibility = _first_hint(relative, RESPONSIBILITY_HINTS) or DEFAULT_RESPONSIBILITY
    state = _first_hint(relative, STATE_HINTS) or DEFAULT_STATE
    legacy_host = module_block.get("legacyHost", "hammer.adapters.mfc")
    if "native-ui" in effects:
        factorization, owner = "adapt-legacy", legacy_host
    else:
        # pure-computation, or effectful-but-portable (file-io/gpu without native UI):
        # the default hypothesis is extraction into a strict module.
        factorization, owner = "extract", legacy_host
    return {
        "path": relative,
        "currentOwner": owner,
        "destinationModule": owner,
        "responsibility": responsibility,
        "effects": effects,
        "state": state,
        "factorization": factorization,
        "evidence": "hypothesis",
        "reviewTODO": [
            "confirm currentOwner/destinationModule by reading the file",
            "confirm responsibility (filename heuristic)",
            "confirm state (filename heuristic)",
            "confirm factorization and re-evidence as 'observed' or 'decision'",
        ],
    }


def hammer_coverage(root: Path, module_block: dict, inventory: dict) -> dict:
    universe = hammer_source_universe(root, module_block)
    universe_set = set(universe)
    classified = [record.get("path", "") for record in inventory.get("files", [])]
    classified_set = set(classified)
    unclassified = [path for path in universe if path not in classified_set]
    classified_outside = sorted(classified_set - universe_set)
    return {
        "universeCount": len(universe),
        "classifiedCount": len(classified_set & universe_set),
        "unclassified": unclassified,
        "classifiedOutsideUniverse": classified_outside,
        "authoredTotal": inventory.get("coverage", {}).get("totalHammerSourceFiles"),
    }


def _filter_paths(paths: Sequence[str], prefix: str | None) -> list[str]:
    if not prefix:
        return list(paths)
    return [path for path in paths if path.startswith(prefix)]


def hammer_coverage_command(root: Path, manifest: dict, args: argparse.Namespace) -> int:
    module_block = hammer_modules(manifest)
    inventory = _load_json(root, "architecture/hammer_inventory.json")
    coverage = hammer_coverage(root, module_block, inventory)
    unclassified = _filter_paths(coverage["unclassified"], args.path)

    if args.json:
        grouped: dict[str, list[str]] = {}
        for path in unclassified:
            key = _first_hint(path, RESPONSIBILITY_HINTS) or DEFAULT_RESPONSIBILITY
            grouped.setdefault(key, []).append(path)
        print(json.dumps({**coverage, "unclassified": unclassified, "byResponsibilityHint": grouped}, indent=2))
        return 0

    total = coverage["universeCount"]
    done = coverage["classifiedCount"]
    print(f"hammer inventory coverage: {done}/{total} files classified ({total - done} remaining)")
    if coverage["authoredTotal"] not in (None, total):
        print(
            f"  warning: coverage.totalHammerSourceFiles={coverage['authoredTotal']} "
            f"but the source universe now has {total} files; update the authored total"
        )
    for path in coverage["classifiedOutsideUniverse"]:
        print(f"  warning: classified file is outside the hammer source universe: {path}")

    groups: Counter[str] = Counter(
        _first_hint(path, RESPONSIBILITY_HINTS) or DEFAULT_RESPONSIBILITY for path in unclassified
    )
    if groups:
        print("\nremaining by responsibility hint (heuristic):")
        for name, count in sorted(groups.items(), key=lambda item: (-item[1], item[0])):
            print(f"  {count:>4}  {name}")

    scope = f" under {args.path!r}" if args.path else ""
    shown = unclassified if args.limit == 0 else unclassified[: args.limit]
    print(f"\nunclassified files{scope} ({len(shown)} of {len(unclassified)} shown):")
    for path in shown:
        effects = ",".join(detect_effects(root, path))
        print(f"  {path}  [effects: {effects}]")
    if len(shown) < len(unclassified):
        print(f"  ... {len(unclassified) - len(shown)} more (raise --limit or use --json)")
    print("\nNext: `archlint.py hammer --scaffold --path <dir>` emits reviewable stubs to classify.")
    return 0


def hammer_scaffold_command(root: Path, manifest: dict, args: argparse.Namespace) -> int:
    module_block = hammer_modules(manifest)
    inventory = _load_json(root, "architecture/hammer_inventory.json")
    coverage = hammer_coverage(root, module_block, inventory)
    unclassified = _filter_paths(coverage["unclassified"], args.path)
    batch = unclassified if args.limit == 0 else unclassified[: args.limit]
    records = [suggest_record(root, module_block, path) for path in batch]

    if not args.json:
        print(
            f"// Reviewable stubs for {len(records)} of {len(unclassified)} unclassified file(s).\n"
            "// evidence is 'hypothesis': read each file, correct the fields, drop reviewTODO,\n"
            "// re-evidence as 'observed'/'decision', then merge into hammer_inventory.json.\n"
            "// Coverage stays 'partial' until every file is confirmed.",
            file=sys.stderr,
        )
    print(json.dumps({"files": records}, indent=2))
    return 0


def hammer_command(root: Path, manifest: dict) -> int:
    module_block = hammer_modules(manifest)
    capabilities = frozenset(module["id"] for module in manifest["capabilityModules"]["modules"])
    errors: list[str] = []
    errors.extend(
        f"[modules] {message}" for message in validate_module_graph(module_block, capabilities)
    )

    inventory = _load_json(root, "architecture/hammer_inventory.json")
    errors.extend(f"[inventory] {message}" for message in validate_inventory(root, module_block, inventory))

    migrations = _load_json(root, "architecture/hammer_migrations.json")
    errors.extend(f"[migrations] {message}" for message in validate_migrations(root, module_block, migrations))

    compatibility = _load_json(root, "architecture/hammer_compatibility.json")
    errors.extend(f"[compatibility] {message}" for message in validate_compatibility(compatibility))

    baseline = _load_json(root, "architecture/hammer_baseline.json")
    new, stale = hammer_native_token_report(root, module_block, baseline)
    for occurrence in new:
        errors.append(
            f"[HAM003] {occurrence.path}:{occurrence.line}: native/toolkit token in strict "
            f"module include root: {occurrence.excerpt}"
        )
    for entry in stale:
        errors.append(f"[HAM003] stale baseline entry no longer present: {entry['path']}:{entry.get('line')}")

    if errors:
        for message in errors:
            print(f"archlint: {message}")
        print(f"archlint: hammer verification failed with {len(errors)} problem(s)")
        return 1
    print(
        "archlint: hammer inventory, migration ledger, module graph, and compatibility "
        f"declarations are valid ({len(inventory['files'])} files, "
        f"{len(migrations['migrations'])} migrations; coverage {inventory['coverage']['status']})"
    )
    return 0


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    check = subparsers.add_parser("check", help="reject new or stale baseline occurrences")
    mode = check.add_mutually_exclusive_group(required=True)
    mode.add_argument("--all", action="store_true")
    mode.add_argument("--changed", action="store_true")
    check.add_argument("--base", help="git base ref used with --changed")
    check.add_argument(
        "--compile-deps",
        action="append",
        default=[],
        metavar="TREE",
        help="also check the transitive includes recorded in TREE's -MMD .d files (repeatable)",
    )
    baseline = subparsers.add_parser("baseline", help="verify or intentionally rewrite the baseline")
    baseline.add_argument("--verify", action="store_true")
    baseline.add_argument("--write", action="store_true")
    inventory = subparsers.add_parser("inventory", help="verify or intentionally rewrite loader inventory")
    inventory.add_argument("--verify", action="store_true")
    inventory.add_argument("--write", action="store_true")
    tools = subparsers.add_parser(
        "tools", help="verify the RFC 0001 Phase E tool cohort ledger and wrapper ratchet"
    )
    tools.add_argument("--verify", action="store_true", help="validate the authored ledger")
    hermetic = subparsers.add_parser(
        "hermetic", help="compile each portable contract header alone and check its full include closure"
    )
    hermetic.add_argument("--cxx", action="append", default=[], help="compiler (repeatable; default g++)")
    hammer = subparsers.add_parser("hammer", help="verify the RFC 0002 editor inventory, migrations, and ratchet")
    mode = hammer.add_mutually_exclusive_group()
    mode.add_argument("--verify", action="store_true", help="validate authored editor artifacts (default)")
    mode.add_argument(
        "--coverage",
        action="store_true",
        help="report which hammer source files are still unclassified (HAM-INVENTORY-001)",
    )
    mode.add_argument(
        "--scaffold",
        action="store_true",
        help="emit reviewable inventory stubs (evidence 'hypothesis') for unclassified files",
    )
    hammer.add_argument("--path", help="restrict --coverage/--scaffold to files under this prefix")
    hammer.add_argument(
        "--limit", type=int, default=25, help="max files for --coverage/--scaffold (0 = no limit)"
    )
    hammer.add_argument("--json", action="store_true", help="machine-readable output")
    return parser


def main(argv: Sequence[str] | None = None, root: Path | None = None) -> int:
    args = build_parser().parse_args(argv)
    root = root or repo_root_from_script()
    manifest = load_manifest(root)
    if args.command == "check":
        return check_command(args, root, manifest)
    if args.command == "tools":
        return tool_migrations_command(root)
    if args.command == "hermetic":
        failures = 0
        for cxx in args.cxx or ["g++"]:
            errors, count = capabilities.hermetic_errors(
                root, manifest["capabilityModules"], capabilities.compiler_deps(root, cxx)
            )
            for error in errors:
                print(error)
            if count == 0:
                print(f"archlint: hermetic {cxx}: no portable public headers found")
                failures += 1
            print(f"archlint: hermetic {cxx}: {count} portable public headers, {len(errors)} error(s)")
            failures += len(errors)
        return 1 if failures else 0
    if args.command == "hammer":
        if args.coverage:
            return hammer_coverage_command(root, manifest, args)
        if args.scaffold:
            return hammer_scaffold_command(root, manifest, args)
        return hammer_command(root, manifest)
    if args.command == "baseline":
        if args.write == args.verify:
            raise SystemExit("baseline requires exactly one of --verify or --write")
        if args.verify:
            return verify_baseline(root, manifest)
        # Manifest exceptions own their occurrences; the baseline never absorbs them.
        excepted = excepted_keys(manifest)
        baselined = {entry["fingerprint"] for entry in read_baseline(root)["entries"]}
        return verify_or_write(
            root / "architecture/baseline.json",
            baseline_document(
                [
                    item
                    for item in scan(root, manifest)
                    if item.fingerprint in baselined or (item.path, item.rule) not in excepted
                ]
            ),
            args.write,
            "baseline",
        )
    if args.write == args.verify:
        raise SystemExit("inventory requires exactly one of --verify or --write")
    inventory_expected = inventory_document(root, manifest)
    missing_native_telemetry = [
        site
        for site in inventory_expected["sites"]
        if site["mechanism"] == "native-loader"
        and site.get("telemetry") == "missing"
    ]
    if missing_native_telemetry:
        for site in missing_native_telemetry:
            print(
                f"{site['path']}:{site['line']}: native loader site has no "
                "runtime telemetry adapter"
            )
        print(
            "archlint: loader inventory rejected; "
            f"{len(missing_native_telemetry)} native site(s) are uninstrumented"
        )
        return 1
    return verify_or_write(
        root / "architecture/loader_inventory.json",
        inventory_expected,
        args.write,
        "loader inventory",
    )


if __name__ == "__main__":
    sys.exit(main())
