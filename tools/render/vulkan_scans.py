#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Static scans for RFC 0016 K1 "One Vulkan stack" and "No idle waits on frame
# paths" (render.vulkan.allocation-sites, render.vulkan.idle-waits).
#
#   allocation-sites  vkAllocateMemory, vkCreateBuffer and vkCreateImage appear
#                     only under render/device/vulkan/, except reviewed test
#                     fixtures under unittests/ that create their own raw
#                     Vulkan devices. Product code can never be excepted: an
#                     exception outside unittests/ is itself a failure.
#   idle-waits        every vkDeviceWaitIdle and vkQueueWaitIdle call site
#                     matches a reviewed entry (path, enclosing function,
#                     count, class, reason). Unlisted sites, count overruns
#                     and stale entries (listed but gone) all fail, so the
#                     list stays exact.
#   list              prints every current site with its enclosing function.
#   sensitivity       seeded faults in private temporary trees must each be
#                     rejected, and an unseeded control must pass.
#
# Sources are first-party C/C++ files from `git ls-files` plus untracked,
# non-ignored files, skipping the symlinked submodules and dependencies/.
# Comments and string literals are stripped before matching. The reviewed
# list is tools/render/vulkan_scans.json (render-vulkan-scans/v1).
#
# Each check subcommand ends with one checks-v1 record (CONFORMANCE n f) and
# exits 0 only when every check passed. Python 3 standard library only.
#
# ============================================================================

import argparse
import io
import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools" / "quality"))
from conformance_result import Checks  # noqa: E402

SCHEMA = "render-vulkan-scans/v1"
EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".inl", ".mm"}
SKIPPED_ROOTS = ("box3d/", "ivp/", "lib/", "thirdparty/", "dependencies/")
ADAPTER_ROOT = "render/device/vulkan/"
TEST_ROOT = "unittests/"
ALLOCATION = re.compile(r"\b(vkAllocateMemory|vkCreateBuffer|vkCreateImage)\b")
IDLE_WAIT = re.compile(r"\b(vkDeviceWaitIdle|vkQueueWaitIdle)\b")
IDLE_CLASSES = {"teardown", "mode-change", "loss-recovery", "port-wait-idle", "test-fixture"}
# A reviewed site still on a frame path: listed so the scan names it, and
# counted as a failure until it is replaced (class "frame-path", pending true).
PENDING_CLASS = "frame-path"
DEFAULT_LIST = Path(__file__).resolve().with_name("vulkan_scans.json")

# Words that look like a call at column 0 but never name a function.
NOT_FUNCTIONS = {"if", "for", "while", "switch", "return", "sizeof", "catch", "defined",
                 "decltype", "alignof", "static_assert", "namespace", "template"}


def repo_root():
    return Path(__file__).resolve().parents[2]


# Sources -------------------------------------------------------------------

def source_files(root):
    """First-party C/C++ paths (relative, forward slashes), tracked or untracked."""
    root = Path(root)
    names = set()
    for extra in ([], ["--others", "--exclude-standard"]):
        result = subprocess.run(["git", "-C", str(root), "ls-files", "-z"] + extra,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
        if result.returncode != 0:
            raise RuntimeError("git ls-files failed in %s: %s" %
                               (root, result.stderr.decode(errors="replace").strip()))
        names.update(n for n in result.stdout.decode("utf-8", "replace").split("\0") if n)
    files = []
    for name in sorted(names):
        if name.startswith(SKIPPED_ROOTS) or Path(name).suffix not in EXTENSIONS:
            continue
        path = root / name
        if path.is_file() and not path.is_symlink():
            files.append(name)
    return files


def strip_code(text):
    """Blanks comments, string and character literals; keeps line structure."""
    out = []
    i = 0
    n = len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            while i < n and text[i] != "\n":
                i += 1
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            end = text.find("*/", i + 2)
            end = n if end < 0 else end + 2
            out.append("".join("\n" if ch == "\n" else " " for ch in text[i:end]))
            i = end
            continue
        if c == "R" and i + 1 < n and text[i + 1] == '"' and (i == 0 or not (text[i - 1].isalnum() or text[i - 1] == "_")):
            open_paren = text.find("(", i + 2)
            if open_paren >= 0:
                delimiter = text[i + 2:open_paren]
                end = text.find(")" + delimiter + '"', open_paren)
                end = n if end < 0 else end + len(delimiter) + 2
                out.append('"' + "".join("\n" if ch == "\n" else " " for ch in text[i + 1:end - 1]) + '"')
                i = end
                continue
        if c in "\"'":
            quote = c
            j = i + 1
            while j < n and text[j] != quote and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
            out.append(quote + " " * max(0, j - i - 2) + (quote if j - i >= 2 else ""))
            i = j
            continue
        out.append(c)
        i += 1
    return "".join(out)


# Enclosing functions ------------------------------------------------------

_NAME = r"(~?[A-Za-z_]\w*(?:\s*::\s*~?[A-Za-z_]\w*)*(?:\s*::\s*operator\s*[^\s(]+)?)"
_DEFINITION = re.compile(_NAME + r"\s*\(")
_CLASS = re.compile(r"^(?:template\s*<[^>]*>\s*)?(class|struct|union)\s+(?:\w+\s+)*?([A-Za-z_]\w*)"
                    r"\s*(?:final\b)?\s*(?::[^{]*)?$")


def _definition_name(header):
    """The function name a definition header declares, or None."""
    flat = " ".join(header.split())
    if not flat or flat.startswith("#"):
        return None
    if re.match(r"^(class|struct|union|enum|namespace|extern\s+\"C\")\b", flat):
        return None
    if "=" in flat.split("(")[0]:
        return None  # an initializer, not a declarator
    # The declarator is the last name before the first top-level '('.
    for match in _DEFINITION.finditer(flat):
        name = re.sub(r"\s+", "", match.group(1))
        base = name.split("::")[-1].lstrip("~")
        if base in NOT_FUNCTIONS:
            return None
        return name
    return None


def functions_by_line(code):
    """Maps each 1-based line to the name of the outermost function body it
    is in (class member bodies inside a class count; lambdas and nested
    blocks count as their enclosing function), or '<file>' at file scope."""
    lines = code.split("\n")
    result = [None] * (len(lines) + 2)
    # Scope stack: entries are ('function', name) or ('scope', None).
    stack = []
    header = []  # non-blank text since the last ';', '{' or '}' at a scope level
    current = None
    for number, line in enumerate(lines, 1):
        in_function = next((s[1] for s in reversed(stack) if s[0] == "function"), None)
        result[number] = in_function or "<file>"
        if line.lstrip().startswith("#"):
            continue
        for ch in line:
            if in_function is not None:
                if ch == "{":
                    stack.append(("block", None))
                elif ch == "}" and stack:
                    stack.pop()
                    in_function = next((s[1] for s in reversed(stack) if s[0] == "function"), None)
                    if in_function is None:
                        header = []
                continue
            if ch == "{":
                text = "".join(header)
                name = _definition_name(text)
                if name is not None:
                    owner = next((s[1] for s in reversed(stack) if s[0] == "class"), None)
                    if owner and "::" not in name:
                        name = owner + "::" + name
                    stack.append(("function", name))
                    in_function = name
                    current = number
                else:
                    record = _CLASS.match(" ".join(text.split()))
                    stack.append(("class", record.group(2)) if record else ("scope", None))
                header = []
            elif ch == "}":
                if stack:
                    stack.pop()
                header = []
            elif ch == ";":
                header = []
            else:
                header.append(ch)
        header.append(" ")
        # A call on the same line as the opening brace belongs to the function.
        if in_function is not None and current == number:
            result[number] = in_function
    return result


def short_function(name):
    """Class::Method or Function: drops namespaces, keeps the last class."""
    if name in (None, "<file>"):
        return "<file>"
    parts = name.split("::")
    return "::".join(parts[-2:]) if len(parts) >= 2 else parts[0]


def scan(root, pattern, files=None):
    """Every match: dicts with path, line, function, symbol."""
    root = Path(root)
    sites = []
    for name in (files if files is not None else source_files(root)):
        try:
            text = (root / name).read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        if not pattern.search(text):
            continue
        code = strip_code(text)
        owners = None
        for number, line in enumerate(code.split("\n"), 1):
            for match in pattern.finditer(line):
                if owners is None:
                    owners = functions_by_line(code)
                sites.append({"path": name, "line": number,
                              "function": short_function(owners[number]),
                              "symbol": match.group(1)})
    return sites


# The reviewed list --------------------------------------------------------

def load_list(path):
    with open(path, encoding="utf-8") as handle:
        data = json.load(handle)
    return data


def validate_list(data, checks):
    """Schema checks on the reviewed list itself."""
    ok = checks.check(data.get("schema") == SCHEMA, "list.schema",
                      "expected %r, got %r" % (SCHEMA, data.get("schema")))
    for entry in data.get("allocation_exceptions", []):
        path = entry.get("path", "")
        ok &= checks.check(path.startswith(TEST_ROOT), "list.allocation-exception-in-product",
                           "%s: only test fixtures under %s may be excepted" % (path, TEST_ROOT))
        ok &= checks.check(bool(entry.get("reason", "").strip()), "list.allocation-exception-reason",
                           "%s has no reason" % path)
    seen = set()
    for entry in data.get("idle_waits", []):
        key = (entry.get("path"), entry.get("function"))
        ok &= checks.check(key not in seen, "list.idle-wait-duplicate", "%s %s listed twice" % key)
        seen.add(key)
        if entry.get("pending"):
            ok &= checks.check(entry.get("class") == PENDING_CLASS, "list.idle-wait-class",
                               "%s %s: a pending entry has class %r" % (key[0], key[1], PENDING_CLASS))
            ok &= checks.check(False, "idle-waits.pending-review",
                               "%s %s is on a frame path: %s" % (key[0], key[1],
                                                                  entry.get("reason", "")))
        else:
            ok &= checks.check(entry.get("class") in IDLE_CLASSES, "list.idle-wait-class",
                               "%s %s: class %r not in %s" % (key[0], key[1], entry.get("class"),
                                                              sorted(IDLE_CLASSES)))
        ok &= checks.check(bool(entry.get("reason", "").strip()), "list.idle-wait-reason",
                           "%s %s has no reason" % key)
        count = entry.get("count")
        ok &= checks.check(isinstance(count, int) and count >= 1, "list.idle-wait-count",
                           "%s %s: count %r" % (key[0], key[1], count))
        if entry.get("class") == "test-fixture":
            ok &= checks.check(str(key[0]).startswith(TEST_ROOT), "list.idle-wait-test-class",
                               "%s: class test-fixture outside %s" % (key[0], TEST_ROOT))
    return ok


def check_allocations(root, data, checks, files=None):
    exceptions = {e.get("path") for e in data.get("allocation_exceptions", [])
                  if str(e.get("path", "")).startswith(TEST_ROOT)}
    sites = scan(root, ALLOCATION, files)
    outside = [s for s in sites if not s["path"].startswith(ADAPTER_ROOT)]
    for site in outside:
        checks.check(site["path"] in exceptions, "allocation-sites.outside-adapter",
                     "%s:%d %s in %s" % (site["path"], site["line"], site["symbol"], site["function"]))
    matched_paths = {s["path"] for s in outside}
    for path in sorted(exceptions - matched_paths):
        checks.check(False, "allocation-sites.stale-exception",
                     "%s is excepted but allocates nothing" % path)
    checks.check(True, "allocation-sites.scanned",
                 "%d sites, %d under %s" % (len(sites), len(sites) - len(outside), ADAPTER_ROOT))
    return sites


def check_idle_waits(root, data, checks, files=None):
    sites = scan(root, IDLE_WAIT, files)
    counts = {}
    for site in sites:
        counts.setdefault((site["path"], site["function"]), []).append(site)
    reviewed = {(e.get("path"), e.get("function")): e for e in data.get("idle_waits", [])}
    for key, found in sorted(counts.items()):
        entry = reviewed.get(key)
        where = ", ".join("%s:%d" % (s["path"], s["line"]) for s in found)
        if not checks.check(entry is not None, "idle-waits.unlisted",
                            "%s in %s (%s) is not in the reviewed list" % (found[0]["symbol"], key[1], where)):
            continue
        checks.check(len(found) <= entry.get("count", 0), "idle-waits.count",
                     "%s %s: %d sites, %d reviewed (%s)" % (key[0], key[1], len(found),
                                                          entry.get("count", 0), where))
    for key, entry in sorted(reviewed.items(), key=lambda item: (str(item[0][0]), str(item[0][1]))):
        found = len(counts.get(key, []))
        checks.check(found == entry.get("count"), "idle-waits.stale",
                     "%s %s: reviewed %s, found %d" % (key[0], key[1], entry.get("count"), found))
    checks.check(True, "idle-waits.scanned", "%d sites in %d functions" % (len(sites), len(counts)))
    return sites


# Commands -----------------------------------------------------------------

def _emit_json(args, payload):
    if args.json:
        text = json.dumps(payload, indent=2, sort_keys=True)
        if args.json == "-":
            print(text)
        else:
            Path(args.json).write_text(text + "\n", encoding="utf-8")


def _write_out(args, payload):
    if getattr(args, "out", None):
        out = Path(args.out)
        out.mkdir(parents=True, exist_ok=True)
        (out / "evidence.json").write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n",
                                           encoding="utf-8")


def run_check(args, kind):
    checks = Checks()
    data = load_list(args.list)
    validate_list(data, checks)
    if kind == "allocation":
        sites = check_allocations(args.root, data, checks)
    else:
        sites = check_idle_waits(args.root, data, checks)
    payload = {"schema": SCHEMA + "/evidence", "scan": args.command, "root": str(args.root),
               "list": str(args.list), "sites": sites,
               "checks": checks.checks, "failures": checks.failures}
    _emit_json(args, payload)
    _write_out(args, payload)
    return checks.report()


def run_list(args):
    rows = []
    for label, pattern in (("allocation", ALLOCATION), ("idle-wait", IDLE_WAIT)):
        grouped = {}
        for site in scan(args.root, pattern):
            grouped.setdefault((site["path"], site["function"]), []).append(site)
        for (path, function), found in sorted(grouped.items()):
            rows.append({"scan": label, "path": path, "function": function, "count": len(found),
                         "lines": [s["line"] for s in found],
                         "symbols": sorted({s["symbol"] for s in found})})
    for row in rows:
        print("%-10s %-70s %-45s %2d  lines %s" % (row["scan"], row["path"], row["function"],
                                                   row["count"], ",".join(map(str, row["lines"]))))
    _emit_json(args, {"schema": SCHEMA + "/list", "sites": rows})
    return 0


# Sensitivity --------------------------------------------------------------

def _seed_tree(directory, files):
    directory = Path(directory)
    subprocess.run(["git", "init", "-q", str(directory)], check=True)
    for name, text in files.items():
        path = directory / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
    return directory


def _scan_failures(tree, data, kind):
    """The FAIL lines a scan of tree reports against data."""
    stream = io.StringIO()
    quiet = Checks(stream=stream)
    validate_list(data, quiet)
    if kind == "allocation":
        check_allocations(tree, data, quiet)
    else:
        check_idle_waits(tree, data, quiet)
    return [line for line in stream.getvalue().splitlines() if line.startswith("FAIL ")]


def _run_scan(tree, data, kind):
    return not _scan_failures(tree, data, kind)


def run_sensitivity(args):
    """Seeded faults against private trees; each must be rejected."""
    checks = Checks()
    real = load_list(args.list)
    frame_fn = ("bool CVulkanContext::BeginFrame( bool *outSkip )\n{\n"
                "\tvkDeviceWaitIdle( m_device );\n\treturn true;\n}\n")
    teardown = "void Device::Shutdown()\n{\n\tvkDeviceWaitIdle( m_device );\n}\n"
    base = {"schema": SCHEMA, "allocation_exceptions": [],
            "idle_waits": [{"path": "render/device/vulkan/device.cpp", "function": "Device::Shutdown",
                            "count": 1, "class": "teardown", "reason": "teardown"}]}
    cases = [
        ("control", {"render/device/vulkan/resources.cpp": "void F()\n{\n\tvkCreateBuffer( d, i, 0, b );\n}\n",
                     "render/device/vulkan/device.cpp": teardown}, base, True),
        ("product-allocation", {"materialsystem/shaderapivulkan/vulkan_device.cpp":
                                "void F()\n{\n\tvkAllocateMemory( d, i, 0, m );\n}\n",
                                "render/device/vulkan/device.cpp": teardown}, base, False),
        ("product-exception", {"materialsystem/shaderapivulkan/vulkan_device.cpp":
                               "void F()\n{\n\tvkCreateImage( d, i, 0, m );\n}\n",
                               "render/device/vulkan/device.cpp": teardown},
         dict(base, allocation_exceptions=[{"path": "materialsystem/shaderapivulkan/vulkan_device.cpp",
                                            "reason": "seeded"}]), False),
        ("unlisted-idle-wait", {"render/device/vulkan/device.cpp": teardown,
                                "materialsystem/shaderapivulkan/vulkan_device.cpp": frame_fn}, base, False),
        ("stale-entry", {"render/device/vulkan/device.cpp": "void Device::Shutdown()\n{\n}\n"}, base, False),
    ]
    for name, files, data, expect in cases:
        with tempfile.TemporaryDirectory(prefix="vulkan-scans-") as temp:
            tree = _seed_tree(temp, files)
            kind = "allocation" if "allocation" in name or name == "product-exception" else "idle"
            if name == "control":
                passed = _run_scan(tree, data, "allocation") and _run_scan(tree, data, "idle")
            else:
                passed = _run_scan(tree, data, kind)
            checks.check(passed == expect, "sensitivity." + name,
                         "expected %s, got %s" % ("pass" if expect else "rejection",
                                                  "pass" if passed else "rejection"))
    # The real reviewed list over a copy of the real tree, then with one seeded
    # frame-path wait: the seeded copy must add exactly one failure, naming the
    # seeded file (the control may still carry pending frame-path entries).
    with tempfile.TemporaryDirectory(prefix="vulkan-scans-real-") as temp:
        tree = Path(temp)
        subprocess.run(["git", "init", "-q", str(tree)], check=True)
        root = Path(args.root)
        for name in source_files(root):
            if IDLE_WAIT.search((root / name).read_text(encoding="utf-8", errors="replace")):
                (tree / name).parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(root / name, tree / name)
        control = _scan_failures(tree, real, "idle")
        checks.check(not any("stale" in line or "unlisted" in line for line in control),
                     "sensitivity.real-list-control",
                     "the real list must match a copy of the real tree: %s" % control[:3])
        seeded = tree / "materialsystem/shaderapivulkan/vulkan_seeded_frame.cpp"
        seeded.parent.mkdir(parents=True, exist_ok=True)
        seeded.write_text(frame_fn, encoding="utf-8")
        added = [line for line in _scan_failures(tree, real, "idle") if line not in control]
        checks.check(len(added) == 1 and "vulkan_seeded_frame.cpp" in added[0],
                     "sensitivity.real-list-seeded",
                     "a seeded frame-path vkDeviceWaitIdle must add one failure: %s" % added)
    _write_out(args, {"schema": SCHEMA + "/sensitivity", "checks": checks.checks,
                      "failures": checks.failures})
    return checks.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    for name in ("allocation-sites", "idle-waits", "list", "sensitivity"):
        command = sub.add_parser(name)
        command.add_argument("--root", type=Path, default=repo_root())
        command.add_argument("--list", type=Path, default=DEFAULT_LIST)
        command.add_argument("--json", help="write a JSON summary to this path ('-' for stdout)")
        command.add_argument("--out", help="evidence directory (conformance runner)")
    args = parser.parse_args(argv)
    if args.command == "allocation-sites":
        return run_check(args, "allocation")
    if args.command == "idle-waits":
        return run_check(args, "idle")
    if args.command == "sensitivity":
        return run_sensitivity(args)
    return run_list(args)


if __name__ == "__main__":
    sys.exit(main())
