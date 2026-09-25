#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Clang/64-bit legacy sweep (RFC 0005 Q0 gap corpus, RFC 0006 M0 / roadmap R03).
#
# Legacy code compiles as `cxx20-permissive` (quality/toolchain/policy.json):
# C++20 with -fpermissive, and the R03 product trees also pass --disable-warns
# (-w). Under GCC that combination turns "cast from 'T*' to 'int' loses
# precision" into a warning and then hides it, so a stack pointer truncated to
# 32 bits (vrad's `(int)&ctx`, fixed in ad7120f3) builds silently on x86_64.
# Clang rejects that cast outright, and has warnings for the neighbouring
# 64-bit hazards; this sweep re-runs every recorded `cxx20-permissive` compile
# of the declared Waf configurations with Clang in -fsyntax-only mode, the
# hazard diagnostics enabled and nothing else, and lists every site in one pass
# (a Waf build would stop at the first failing translation unit).
#
# The inventory quality/fixtures/corpus/clang64/sites.json is an exact ratchet:
# a site not in it fails as new, a recorded site that no longer appears fails
# as stale, and a permissive target entering or leaving coverage fails. Sites
# are keyed by file, diagnostic, message and the text of the source line (not
# the line number), so unrelated edits above a site do not churn the record.
# Every site starts `"classification": "unreviewed"`; re-recording keeps the
# classifications already reviewed.
#
# The oracle is Clang's own semantic analysis over the recorded argv: it does
# not use the engine's code, its Waf dialect hook or GCC.
#
# Subcommands:
#   setup       configure every declared tree (private WAFLOCK/out dir) and
#               record its compile commands with `./waf clangdb` (no build).
#   check       sweep and compare with the inventory (suite corpus.clang64.sweep).
#   record      sweep and rewrite the inventory and its fixture.json.
#   seeded      sweep the seeded fixture TU against the inventory (negative row).
#   historical  sweep a file as of an older revision with its recorded argv
#               (negative row: vraddisps.cpp before ad7120f3).
#
# Python 3 standard library only. No assert statements (checks-v1).
#
# ============================================================================

import argparse
import collections
import concurrent.futures
import json
import os
import re
import shlex
import shutil
import subprocess
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
QUALITY = os.path.dirname(HERE)
sys.path.insert(0, QUALITY)
sys.path.insert(0, HERE)

import conformance  # noqa: E402
import provenance  # noqa: E402
from conformance_result import Checks  # noqa: E402

ROOT = conformance.repo_root()
SUITE = "corpus.clang64.sweep"
FIXTURE_DIR = os.path.join("quality", "fixtures", "corpus", "clang64")
INVENTORY = os.path.join(FIXTURE_DIR, "sites.json")
FIXTURE_RECORD = os.path.join(FIXTURE_DIR, "fixture.json")
SEEDED_TU = os.path.join(FIXTURE_DIR, "seeded", "vrad_enum_context.cpp")
INVENTORY_SCHEMA = "corpus-clang64-sites/v1"
INVENTORY_VERSION = 1
PERMISSIVE = "cxx20-permissive"


def real_compiler(name):
    """`name` on PATH, skipping compiler-cache shims (ccache would replay
    cached diagnostics instead of running the oracle)."""
    if os.path.isabs(name):
        return name
    for directory in os.environ.get("PATH", "").split(os.pathsep):
        candidate = os.path.join(directory, name)
        if os.path.isfile(candidate) and os.access(candidate, os.X_OK) and \
                os.path.basename(os.path.realpath(candidate)) not in ("ccache", "sccache"):
            return candidate
    return name


CLANG = real_compiler(os.environ.get("CLANG64_CXX", "clang++"))

# ---------------------------------------------------------------------------
# Declared configurations: the R03 product trees (quality/baseline.json
# `build.*` setup lines) replayed with Clang under private locks and output
# directories, plus Portal 2 (./play_p2's configure line). The DXVK product
# profile (tools/quality/product_profile.py) requires GCC, so that tree is
# configured with GCC and its recorded argv is compiled by Clang: both record
# the same flag set (no GCC-only options), which `translate_argv` checks.
# ---------------------------------------------------------------------------

DXVK_ROOT_DEFAULT = "dependencies/dxvk-native-2.7.1/usr"

CONFIGS = [
    {"name": "tests", "compiler": "clang",
     "options": ["--tests", "--use-sdl=0", "--use-togl=0"]},
    {"name": "tools", "compiler": "clang",
     "options": ["--tools"]},
    {"name": "dedicated", "compiler": "clang",
     "options": ["--dedicated", "--use-sdl=0", "--use-togl=0", "--build-games=portal",
                 "--physics-backend=both"]},
    {"name": "portal-native", "compiler": "clang",
     "options": ["--platform-provider=sdl3", "--render-backend=native-vulkan",
                 "--build-games=portal", "--physics-backend=both"]},
    {"name": "portal-dxvk", "compiler": "gcc",
     "options": ["--platform-provider=sdl3", "--render-backend=vulkan",
                 "--dxvk-root={dxvk_root}", "--build-games=portal"]},
    {"name": "hl2", "compiler": "clang",
     "options": ["--build-games=hl2"]},
    {"name": "portal2-native", "compiler": "clang",
     "options": ["--platform-provider=sdl3", "--render-backend=native-vulkan",
                 "--build-games=portal2", "--physics-backend=both", "--video-provider=bink"]},
]
COMMON_OPTIONS = ["--disable-warns", "-T", "release"]

# Waf configurations that exist but no declared tree builds: reported gaps.
DECLARED_GAPS = [
    {"scope": "game/client, game/server for --build-games=hl2mp, hl1, episodic, hl1mp, cstrike, dod",
     "reason": "each game compiles the shared client/server sources with its own defines "
               "(~1,500 TUs per game); only portal, hl2 and portal2 are swept"},
    {"scope": "togles", "reason": "GLES translation layer; only Android (non-SDL3) selects it"},
    {"scope": "unicode", "reason": "its wscript builds nothing unless DEST_OS is win32"},
    {"scope": "engine/voice_codecs/celt, engine/voice_codecs/speex",
     "reason": "have wscripts but no root project list reaches them"},
    {"scope": "debugapi, texturecontainer (+ unittests/texturecontainertest)",
     "reason": "strict cxx20 targets (not permissive); also unconfigured here because protoc "
               "and the KTX read dependency are absent, so any permissive code behind "
               "DEBUGAPI/KTX_READ_ENABLED guards is not swept"},
    {"scope": "engine/voice_codecs/opus", "reason": "only --enable-opus or Android (non-SDL3)"},
    {"scope": "utils/bzip2", "reason": "only win32 and Android SDL3 products"},
    {"scope": "Android arm64 (portal-android-native-vulkan, portal2-android-native-vulkan)",
     "reason": "NDK cross-build profiles; LP64 like x86_64 but with different platform #ifdefs; "
               "not replayed here"},
    {"scope": "MSVC / Windows (LLP64: long stays 32-bit)",
     "reason": "no MSVC toolchain on this host; LLP64 truncations such as (long)ptr are not "
               "hazards on LP64 Linux and are not detected by this sweep"},
]

# ---------------------------------------------------------------------------
# Diagnostic set
# ---------------------------------------------------------------------------

# Gated warnings. Clang 22 names; each is verified to exist by `-Werror=<flag>
# -Werror=unknown-warning-option` in `check_flags`.
HAZARD_WARNINGS = [
    "int-to-pointer-cast",        # (T*)i from a smaller integer: sign-extends, loses high bits
    "int-to-void-pointer-cast",   # (void*)i from a smaller integer (context/userdata round trips)
    "pointer-to-int-cast",        # C only in Clang: C++ makes the same cast a hard error (below)
    "void-pointer-to-int-cast",   # C only in Clang
    "pointer-to-enum-cast",       # C only in Clang
    "void-pointer-to-enum-cast",  # C only in Clang
    "format",                     # printf/scanf width mismatch; filtered by `format_width`
]
# Implicit 64->32 narrowing. Split by the spelled source type: a
# pointer-carrying integer (POINTER_CARRIERS) narrowed to 32 bits is the
# implicit twin of `(int)&ctx` and is gated as a site
# (`-Wshorten-64-to-32:pointer-sized`); every other source type (size_t,
# ptrdiff_t, long, handles, SQInteger...) is mostly a small count and is
# recorded as per-directory/per-type counts only, never gated.
ADVISORY_WARNINGS = ["shorten-64-to-32"]
POINTER_CARRIERS = {"intp", "uintp", "intptr_t", "uintptr_t", "std::intptr_t", "std::uintptr_t",
                    "INT_PTR", "UINT_PTR", "LONG_PTR", "ULONG_PTR", "DWORD_PTR",
                    "VPANEL", "vgui::VPANEL"}
POINTER_SIZED_ID = "-Wshorten-64-to-32:pointer-sized"
SHORTEN = re.compile(r"^implicit conversion loses integer precision: '(?:const )?(?P<from>[^']+)'")

# Clang's err_bad_reinterpret_cast_small_int: a C-style or reinterpret_cast
# from a pointer to a smaller integer. An error in C++ in every mode except
# -fms-extensions; GCC's equivalent is a -fpermissive diagnostic.
SMALL_INT_ERROR = re.compile(r"^cast from pointer to smaller type '.*' loses information$")
SMALL_INT_ID = "error:pointer-to-smaller-int"

SWEEP_FLAGS = ["-fsyntax-only", "-ferror-limit=0", "-fno-color-diagnostics",
               "-fdiagnostics-show-option", "-fno-caret-diagnostics",
               "-fno-diagnostics-fixit-info", "-Wno-everything"] + \
    ["-W" + flag for flag in HAZARD_WARNINGS + ADVISORY_WARNINGS]

# Recorded compile-only flags removed before the syntax-only run. -w and every
# -W option are dropped so only the sweep's diagnostic set is active.
DROP_WITH_VALUE = {"-o", "-MF", "-MT", "-MQ"}
DROP_EXACT = {"-c", "-MMD", "-MD", "-MP", "-w", "-pipe"}

DIAGNOSTIC = re.compile(
    r"^(?P<file>[^:\n]+):(?P<line>\d+):(?P<col>\d+): (?P<severity>warning|error|fatal error): "
    r"(?P<message>.*?)(?: \[(?P<options>-W[^\]]*)\])?$")

# ---------------------------------------------------------------------------
# printf width classification (LP64)
# ---------------------------------------------------------------------------

FORMAT_MISMATCH = re.compile(
    r"^format specifies type '(?P<expected>[^']+)' but the argument has type "
    r"'(?P<actual>[^']+)'(?: \(aka '(?P<aka>[^']+)'\))?")

LP64_SIZES = {
    "char": 1, "signed char": 1, "unsigned char": 1, "bool": 1, "char8_t": 1,
    "short": 2, "unsigned short": 2, "wchar_t": 4, "char16_t": 2, "char32_t": 4,
    "int": 4, "unsigned int": 4,
    "long": 8, "unsigned long": 8, "long long": 8, "unsigned long long": 8,
    "__int128": 16, "unsigned __int128": 16,
    "std::nullptr_t": 8, "nullptr_t": 8,
}
FLOATING = {"float", "double", "long double"}


def _canonical(name):
    name = re.sub(r"\b(const|volatile|struct|class|enum|union)\b", "", name)
    name = re.sub(r"\s+", " ", name).strip()
    return {"unsigned": "unsigned int", "signed": "int", "short int": "short",
            "long int": "long", "long long int": "long long",
            "unsigned long int": "unsigned long", "short unsigned int": "unsigned short",
            "__ptrdiff_t": "long", "__size_t": "unsigned long"}.get(name, name)


def _is_pointer(name):
    return name.endswith("*") or "(*)" in name or name.endswith("]")


def _pointee(name):
    return _canonical(re.sub(r"\s*\*$", "", name))


def type_width(name):
    """LP64 byte width of a printed Clang type, or None when unknown."""
    name = _canonical(name)
    if _is_pointer(name):
        return 8
    return LP64_SIZES.get(name)


def format_width(message):
    """(is_width_hazard, detail) for a -Wformat message.

    A hazard is an integer/pointer argument whose LP64 width differs from the
    conversion's (%d with size_t, long or a pointer; %x with a pointer), or a
    scanf-style pointer whose pointee width differs (%d into a size_t*: writes
    4 of 8 bytes). Same-width signedness mismatches, float/int confusion and
    other -Wformat diagnostics are not 64-bit hazards and are not recorded."""
    match = FORMAT_MISMATCH.match(message)
    if not match:
        return False, "not a type mismatch"
    expected = _canonical(match.group("expected"))
    actual = _canonical(match.group("aka") or match.group("actual"))
    if expected in FLOATING or actual in FLOATING:
        return False, "floating-point mismatch"
    if _is_pointer(expected) and _is_pointer(actual):
        a, b = type_width(_pointee(expected)), type_width(_pointee(actual))
        if a is None or b is None or a == b:
            return False, "pointee width equal or unknown"
        return True, "pointee %d vs %d bytes" % (a, b)
    a, b = type_width(expected), type_width(actual)
    if a is None or b is None or a == b:
        return False, "width equal or unknown"
    return True, "%d vs %d bytes" % (a, b)


# ---------------------------------------------------------------------------
# Invocations
# ---------------------------------------------------------------------------

def tree_dir(config):
    return "build-corpus-clang64-" + config["name"]


def lock_name(config):
    return ".lock-waf-corpus-clang64-" + config["name"]


def invocations_path(config):
    return os.path.join(tree_dir(config), "toolchain-invocations.json")


def configure_argv(config, dxvk_root):
    out = os.path.join(ROOT, tree_dir(config))
    options = [o.replace("{dxvk_root}", dxvk_root) for o in config["options"]]
    return [sys.executable, "./waf", "configure", *options, *COMMON_OPTIONS,
            "-o", tree_dir(config), "--prefix=" + os.path.join(out, "install")]


def waf_env(config):
    env = dict(os.environ)
    env["WAFLOCK"] = lock_name(config)
    if config["compiler"] == "clang":
        env["CC"], env["CXX"] = "clang", "clang++"
    else:
        env["CC"], env["CXX"] = "gcc", "g++"
    return env


def translate_argv(arguments):
    """Recorded argv -> Clang syntax-only argv (compiler swapped, compile-only
    and warning options removed, the sweep's flags appended). The recorded
    input file stays where it was."""
    out = [CLANG]
    skip = False
    for arg in arguments[1:]:
        if skip:
            skip = False
            continue
        if arg in DROP_WITH_VALUE:
            skip = True
            continue
        if arg in DROP_EXACT or (arg.startswith("-o") and len(arg) > 2) or \
                arg.startswith("-W") or arg.startswith("-fdiagnostics-color"):
            continue
        out.append(arg)
    return out + SWEEP_FLAGS


def load_invocations(configs, only_sources=None):
    """Every recorded cxx20-permissive C++ compile of the declared trees."""
    tus, problems, coverage = [], [], {}
    for config in configs:
        path = os.path.join(ROOT, invocations_path(config))
        try:
            with open(path, "r", encoding="utf-8") as stream:
                record = json.load(stream)
        except (OSError, ValueError) as error:
            problems.append("%s: %s" % (config["name"], error))
            continue
        cwd = os.path.join(ROOT, tree_dir(config))
        targets = set()
        for entry in record["entries"]:
            if entry.get("dialect") != PERMISSIVE or entry.get("language") != "c++":
                continue
            if only_sources is not None and entry["source"] not in only_sources:
                continue
            targets.add(entry["target"])
            argv = translate_argv(entry["arguments"])
            tus.append({"config": config["name"], "target": entry["target"],
                        "source": entry["source"], "cwd": cwd, "argv": argv})
        coverage[config["name"]] = {
            "recorded_compiler": record["toolchain"].get("compiler_cxx"),
            "recorded_compiler_version": record["toolchain"].get("cc_version"),
            "permissive_tus": sum(1 for t in tus if t["config"] == config["name"]),
            "permissive_targets": sorted(targets),
        }
    return tus, problems, coverage


# ---------------------------------------------------------------------------
# Running and parsing
# ---------------------------------------------------------------------------

def normalize_path(cwd, path, remap=None):
    absolute = os.path.realpath(os.path.join(cwd, path))
    if remap:
        for prefix, replacement in remap.items():
            if absolute.startswith(prefix + os.sep):
                return replacement + absolute[len(prefix):]
    rel = os.path.relpath(absolute, ROOT)
    return absolute if rel.startswith("..") else rel


_line_cache = {}


def source_line(path, line, remap_back=None):
    absolute = remap_back.get(path, path) if remap_back else path
    if not os.path.isabs(absolute):
        absolute = os.path.join(ROOT, absolute)
    lines = _line_cache.get(absolute)
    if lines is None:
        try:
            with open(absolute, "r", encoding="utf-8", errors="replace") as stream:
                lines = stream.read().splitlines()
        except OSError:
            lines = []
        _line_cache[absolute] = lines
    return lines[line - 1].strip() if 0 < line <= len(lines) else ""


def classify(severity, message, options):
    """-> ('site'|'advisory'|'blocking'|None, diagnostic id, detail)."""
    flags = [o[2:] for o in (options or "").split(",") if o.startswith("-W") and o != "-Werror"]
    flag = flags[0] if flags else None
    if severity in ("error", "fatal error"):
        if SMALL_INT_ERROR.match(message):
            return "site", SMALL_INT_ID, None
        if flag in HAZARD_WARNINGS:
            return classify("warning", message, "-W" + flag)
        return "blocking", "error:" + (flag or "other"), None
    if flag in ADVISORY_WARNINGS:
        match = SHORTEN.match(message)
        if match and match.group("from").strip() in POINTER_CARRIERS:
            return "site", POINTER_SIZED_ID, "from %s" % match.group("from").strip()
        return "advisory", "-W" + flag, None
    if flag == "format":
        hazard, detail = format_width(message)
        return ("site" if hazard else None), "-Wformat", detail
    if flag in HAZARD_WARNINGS:
        return "site", "-W" + flag, None
    return None, None, None


def run_tu(tu, timeout):
    start = time.monotonic()
    try:
        env = dict(os.environ, CCACHE_DISABLE="1")
        proc = subprocess.run(tu["argv"], cwd=tu["cwd"], env=env,
                              capture_output=True, text=True, errors="replace",
                              timeout=timeout, check=False)
        status, output = proc.returncode, proc.stdout + proc.stderr
    except subprocess.TimeoutExpired:
        status, output = "timeout", ""
    except OSError as error:
        status, output = "oserror", str(error)
    return tu, status, output, time.monotonic() - start


def sweep(tus, jobs, timeout=600, remap=None):
    """Run every TU; return (sites, advisory, blocking, incomplete, stats)."""
    sites, blocking = {}, {}
    advisory = {}
    incomplete = []
    remap_back = {v: k for k, v in (remap or {}).items()}
    slowest = []
    started = time.monotonic()

    def add(bucket, record, tu, key_extra=None):
        key = (record["file"], record["line"], record["column"], record["diagnostic"],
               record["message"])
        entry = bucket.get(key)
        if entry is None:
            entry = dict(record, targets=set(), configs=set(), tus=0)
            bucket[key] = entry
        entry["targets"].add("%s:%s" % (tu["config"], tu["target"]))
        entry["configs"].add(tu["config"])
        entry["tus"] += 1

    with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as pool:
        futures = [pool.submit(run_tu, tu, timeout) for tu in tus]
        for future in concurrent.futures.as_completed(futures):
            tu, status, output, seconds = future.result()
            slowest.append((seconds, tu["config"], tu["source"]))
            if status in ("timeout", "oserror") or (isinstance(status, int) and status < 0):
                incomplete.append({"config": tu["config"], "target": tu["target"],
                                   "source": tu["source"], "status": str(status)})
                continue
            for line in output.splitlines():
                match = DIAGNOSTIC.match(line)
                if not match:
                    continue
                kind, diag, detail = classify(match.group("severity"), match.group("message"),
                                              match.group("options"))
                if kind is None:
                    continue
                path = normalize_path(tu["cwd"], match.group("file"), remap)
                lineno, col = int(match.group("line")), int(match.group("col"))
                if kind == "advisory":
                    advisory[(path, lineno, col)] = match.group("message")
                    continue
                record = {"file": path, "line": lineno, "column": col, "diagnostic": diag,
                          "message": match.group("message"),
                          "line_text": source_line(path, lineno, remap_back)}
                if detail:
                    record["width"] = detail
                add(sites if kind == "site" else blocking, record, tu)
            if isinstance(status, int) and status != 0 and not any(
                    DIAGNOSTIC.match(l) and DIAGNOSTIC.match(l).group("severity") != "warning"
                    for l in output.splitlines()):
                incomplete.append({"config": tu["config"], "target": tu["target"],
                                   "source": tu["source"],
                                   "status": "exit %s without an error diagnostic" % status})
    advisory = [{"file": p, "line": l, "column": c, "message": m}
                for (p, l, c), m in sorted(advisory.items())]
    stats = {"tus": len(tus), "wall_seconds": round(time.monotonic() - started, 1),
             "jobs": jobs,
             "slowest": [{"seconds": round(s, 1), "config": c, "source": f}
                         for s, c, f in sorted(slowest, reverse=True)[:5]]}
    return finalize(sites), advisory, finalize(blocking), incomplete, stats


def finalize(bucket):
    out = []
    for entry in bucket.values():
        entry = dict(entry)
        entry["targets"] = sorted(entry["targets"])
        entry["configs"] = sorted(entry["configs"])
        out.append(entry)
    return sorted(out, key=lambda e: (e["file"], e["line"], e["column"], e["diagnostic"]))


# ---------------------------------------------------------------------------
# Ratchet
# ---------------------------------------------------------------------------

def site_key(site):
    return (site["file"], site["diagnostic"], site["message"], site["line_text"])


def compare(observed, recorded, scope_new_only=False):
    """-> (new, stale): lists of sites. Keys carry multiplicity: two identical
    lines with the same diagnostic in one file are two sites."""
    rec = collections.Counter(site_key(s) for s in recorded)
    obs = collections.Counter(site_key(s) for s in observed)
    new, stale = [], []
    budget = dict(rec)
    for site in observed:
        key = site_key(site)
        if budget.get(key, 0) > 0:
            budget[key] -= 1
        else:
            new.append(site)
    if not scope_new_only:
        budget = dict(obs)
        for site in recorded:
            key = site_key(site)
            if budget.get(key, 0) > 0:
                budget[key] -= 1
            else:
                stale.append(site)
    return new, stale


def describe(site):
    return "%s:%d:%d %s %s [%s]" % (site["file"], site["line"], site["column"],
                                    site["diagnostic"], site["message"],
                                    ", ".join(site.get("targets", [])[:3]) +
                                    (" +%d" % (len(site["targets"]) - 3)
                                     if len(site.get("targets", [])) > 3 else ""))


# ---------------------------------------------------------------------------
# Summaries and provenance
# ---------------------------------------------------------------------------

def top_dir(path, depth=2):
    parts = path.split("/")
    return "/".join(parts[:depth]) if len(parts) > depth else os.path.dirname(path) or path


def summarize(sites, blocking, advisory):
    by_diag = collections.Counter(s["diagnostic"] for s in sites)
    by_dir = collections.Counter(top_dir(s["file"]) for s in sites)
    by_target = collections.Counter()
    for s in sites:
        for t in s["targets"]:
            by_target[t] += 1
    adv_dir = collections.Counter(top_dir(a["file"]) for a in advisory)
    adv_type = collections.Counter()
    for a in advisory:
        match = SHORTEN.match(a["message"])
        adv_type[match.group("from").strip() if match else "other"] += 1
    return {
        "sites": len(sites),
        "blocking_errors": len(blocking),
        "by_diagnostic": dict(sorted(by_diag.items())),
        "by_directory": dict(by_dir.most_common()),
        "by_target": dict(sorted(by_target.items())),
        "advisory": {"-W" + ADVISORY_WARNINGS[0]: {
            "note": "unique file:line:column occurrences; recorded, not gated",
            "total": len(advisory),
            "by_directory": dict(adv_dir.most_common()),
            "by_source_type": dict(adv_type.most_common())}},
    }


def clang_version():
    return provenance.tool_version([CLANG, "--version"])


def check_flags():
    """Flags that the installed Clang does not know (must be empty)."""
    unknown = []
    for flag in HAZARD_WARNINGS + ADVISORY_WARNINGS:
        proc = subprocess.run([CLANG, "-x", "c++", "-fsyntax-only", "-Werror=unknown-warning-option",
                               "-W" + flag, "-"], input="int x;\n", capture_output=True,
                              text=True, check=False)
        if proc.returncode != 0:
            unknown.append(flag)
    return unknown


def sweep_description():
    return {"clang": clang_version(), "hazard_warnings": ["-W" + f for f in HAZARD_WARNINGS],
            "hazard_errors": [SMALL_INT_ID + " (err_bad_reinterpret_cast_small_int)"],
            "pointer_sized_narrowing": {"id": POINTER_SIZED_ID,
                                        "source_types": sorted(POINTER_CARRIERS)},
            "advisory_warnings": ["-W" + f for f in ADVISORY_WARNINGS],
            "sweep_flags": SWEEP_FLAGS,
            "dropped_recorded_flags": sorted(DROP_EXACT | DROP_WITH_VALUE) + ["-W*", "-o<path>"],
            "format_filter": "LP64 width mismatches only (see format_width)",
            "werror_equivalent": ["-Werror=" + f for f in HAZARD_WARNINGS]}


def provenance_block(suite, command, inputs=None):
    return provenance.collect(
        suite,
        tools={"clang++": clang_version(), "python3": sys.version.split()[0]},
        inputs=inputs or {},
        command=command)


# ---------------------------------------------------------------------------
# Subcommands
# ---------------------------------------------------------------------------

def selected_configs(names):
    if not names:
        return list(CONFIGS)
    return [c for c in CONFIGS if c["name"] in names]


def cmd_setup(args):
    status = 0
    for config in selected_configs(args.config):
        env = waf_env(config)
        argv = configure_argv(config, args.dxvk_root)
        print("setup %s: %s" % (config["name"], shlex.join(argv)), flush=True)
        for step in (argv, [sys.executable, "./waf", "clangdb"]):
            proc = subprocess.run(step, cwd=ROOT, env=env, capture_output=True, text=True,
                                  check=False)
            if proc.returncode != 0:
                print("setup %s failed:\n%s" % (config["name"], (proc.stdout + proc.stderr)[-3000:]))
                status = 1
                break
    return status


def refresh(configs):
    """Re-record compile commands (`./waf clangdb`, no build) so newly added
    sources are swept. Returns a list of failures."""
    failures = []
    for config in configs:
        proc = subprocess.run([sys.executable, "./waf", "clangdb"], cwd=ROOT, env=waf_env(config),
                              capture_output=True, text=True, check=False)
        if proc.returncode != 0:
            failures.append("%s: %s" % (config["name"], (proc.stdout + proc.stderr)[-500:]))
    return failures


def load_inventory(path=INVENTORY):
    with open(os.path.join(ROOT, path), "r", encoding="utf-8") as stream:
        return json.load(stream)


def run_full(args, checks):
    configs = selected_configs(None)
    if not args.no_refresh:
        for failure in refresh(configs):
            checks.check(False, "clang64.refresh", failure)
    unknown = check_flags()
    checks.equal(unknown, [], "clang64.flags-known")
    tus, problems, coverage = load_invocations(configs)
    for problem in problems:
        checks.check(False, "clang64.invocations", problem)
    checks.check(len(tus) > 0, "clang64.tus-discovered", "no cxx20-permissive TU recorded")
    sites, advisory, blocking, incomplete, stats = sweep(tus, args.jobs)
    print("swept %d TUs in %.1fs with %d jobs: %d sites, %d blocking errors, %d incomplete"
          % (stats["tus"], stats["wall_seconds"], stats["jobs"], len(sites), len(blocking),
             len(incomplete)), flush=True)
    return configs, tus, coverage, sites, advisory, blocking, incomplete, stats


def cmd_check(args):
    checks = Checks()
    inventory = load_inventory()
    checks.equal(inventory.get("schema"), INVENTORY_SCHEMA, "clang64.inventory-schema")
    checks.equal(inventory.get("version"), INVENTORY_VERSION, "clang64.inventory-version")
    with open(os.path.join(ROOT, FIXTURE_RECORD), "r", encoding="utf-8") as stream:
        fixture = json.load(stream)
    checks.equal(fixture.get("files", {}).get("sites.json"),
                 provenance.file_sha256(os.path.join(ROOT, INVENTORY)), "clang64.fixture-digest")
    configs, tus, coverage, sites, advisory, blocking, incomplete, stats = run_full(args, checks)
    for tu in incomplete:
        checks.check(False, "clang64.incomplete-tu", json.dumps(tu, sort_keys=True))
    new, stale = compare(sites, inventory["sites"])
    for site in new:
        checks.check(False, "clang64.new-site", describe(site))
    for site in stale:
        checks.check(False, "clang64.stale-site", describe(site))
    checks.equal(len(sites), len(inventory["sites"]), "clang64.site-count")
    new_b, stale_b = compare(blocking, inventory["blocking_errors"])
    for site in new_b:
        checks.check(False, "clang64.new-blocking-error", describe(site))
    for site in stale_b:
        checks.check(False, "clang64.stale-blocking-error", describe(site))
    recorded_cov = inventory["coverage"]["configs"]
    for config in configs:
        name = config["name"]
        now = set(coverage.get(name, {}).get("permissive_targets", []))
        then = set(recorded_cov.get(name, {}).get("permissive_targets", []))
        checks.equal(sorted(now - then), [], "clang64.coverage.%s.unrecorded-targets" % name)
        checks.equal(sorted(then - now), [], "clang64.coverage.%s.dropped-targets" % name)
    unreviewed = sum(1 for s in inventory["sites"] if s.get("classification") == "unreviewed")
    print("inventory: %d recorded sites (%d unreviewed), %d blocking errors"
          % (len(inventory["sites"]), unreviewed, len(inventory["blocking_errors"])))
    provenance.emit(provenance_block(SUITE, [sys.executable] + sys.argv,
                                     {"inventory": os.path.join(ROOT, INVENTORY)}))
    write_observed(args, sites, blocking, advisory, coverage, stats)
    return checks.report()


def write_observed(args, sites, blocking, advisory, coverage, stats):
    out = os.environ.get("CONFORMANCE_OUT") or args.out
    if not out:
        return
    os.makedirs(out, exist_ok=True)
    with open(os.path.join(out, "observed-sites.json"), "w", encoding="utf-8") as stream:
        json.dump({"sites": sites, "blocking_errors": blocking,
                   "summary": summarize(sites, blocking, advisory),
                   "coverage": coverage, "stats": stats}, stream, indent=1, sort_keys=True)
        stream.write("\n")


def cmd_record(args):
    checks = Checks()
    configs, tus, coverage, sites, advisory, blocking, incomplete, stats = run_full(args, checks)
    if incomplete:
        for tu in incomplete:
            print("incomplete TU: %s" % json.dumps(tu))
        print("refusing to record an inventory with incomplete TUs")
        return 1
    if checks.failures:
        return checks.report()
    previous = {}
    try:
        for site in load_inventory()["sites"]:
            previous.setdefault(site_key(site), []).append(site.get("classification", "unreviewed"))
    except (OSError, ValueError, KeyError):
        pass
    for site in sites:
        kept = previous.get(site_key(site))
        site["classification"] = kept.pop(0) if kept else "unreviewed"
    for entry in blocking:
        entry["classification"] = "unreviewed"
    command = [sys.executable, "tools/quality/corpus/clang64_sweep.py", "record"]
    block = provenance_block(SUITE, command)
    block["setup"] = "python3 tools/quality/corpus/clang64_sweep.py setup"
    block["check"] = "python3 tools/quality/conformance.py check --suite %s --out -" % SUITE
    inventory = {
        "schema": INVENTORY_SCHEMA,
        "version": INVENTORY_VERSION,
        "description": "Clang/64-bit hazard sites in every recorded cxx20-permissive compile of "
                       "the declared Waf trees (tools/quality/corpus/clang64_sweep.py). Exact "
                       "ratchet: keyed by file, diagnostic, message and source-line text. "
                       "`classification` is for review (unreviewed / intentional / defect); "
                       "the sweep never edits legacy code.",
        "provenance": block,
        "sweep": sweep_description(),
        "coverage": {
            "configs": coverage,
            "config_options": {c["name"]: {"compiler": c["compiler"],
                                           "configure": c["options"] + COMMON_OPTIONS}
                               for c in configs},
            "not_covered": DECLARED_GAPS,
        },
        "stats": stats,
        "summary": summarize(sites, blocking, advisory),
        "sites": sites,
        "blocking_errors": blocking,
    }
    path = os.path.join(ROOT, INVENTORY)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", encoding="utf-8") as stream:
        json.dump(inventory, stream, indent=1, sort_keys=False)
        stream.write("\n")
    fixture = {
        "schema": "corpus-fixture/v1",
        "version": INVENTORY_VERSION,
        "area": "clang64",
        "description": "Recorded Clang/64-bit hazard inventory (sites.json) and the seeded "
                       "negative-control TU (seeded/). Re-record with the provenance command "
                       "after reviewing a ratchet failure.",
        "files": {"sites.json": provenance.file_sha256(path),
                  "seeded/vrad_enum_context.cpp":
                      provenance.file_sha256(os.path.join(ROOT, SEEDED_TU))},
        "provenance": block,
    }
    with open(os.path.join(ROOT, FIXTURE_RECORD), "w", encoding="utf-8") as stream:
        json.dump(fixture, stream, indent=2, sort_keys=True)
        stream.write("\n")
    print("recorded %d sites, %d blocking errors -> %s" % (len(sites), len(blocking), INVENTORY))
    return 0


SEEDED_ARGV = [CLANG, "-std=c++20", "-fpermissive", "-w", "-DPLATFORM_64BITS=1", "-DPOSIX=1",
               "-DLINUX=1", "-DGNUC", SEEDED_TU, "-c", "-o", "seeded.o"]


def seeded_tus():
    """The seeded TU compiled with the permissive product flags, through the
    same translate/sweep/compare path as the recorded trees."""
    argv = translate_argv(SEEDED_ARGV)
    return [{"config": "seeded", "target": "seeded_fixture", "source": SEEDED_TU,
             "cwd": ROOT, "argv": argv}]


def report_new(checks, inventory, sites, name):
    new, _ = compare(sites, inventory["sites"], scope_new_only=True)
    for site in new:
        checks.check(False, name, describe(site))
    return new


def cmd_seeded(args):
    checks = Checks()
    inventory = load_inventory()
    tus = seeded_tus()
    sites, advisory, blocking, incomplete, stats = sweep(tus, 1)
    checks.equal(incomplete, [], "clang64.seeded.complete")
    new = report_new(checks, inventory, sites, "clang64.new-site")
    print("seeded: %d sites observed, %d not in the inventory" % (len(sites), len(new)))
    provenance.emit(provenance_block("corpus.clang64.seeded-truncation",
                                     [sys.executable] + sys.argv,
                                     {"seeded": os.path.join(ROOT, SEEDED_TU)}))
    return checks.report()


def cmd_historical(args):
    """Sweep `args.source` as of `args.rev` with its recorded argv from the
    `args.config` tree. The old bytes go to the scratch directory at the same
    relative path; diagnostics are mapped back to the repository path."""
    checks = Checks()
    inventory = load_inventory()
    out = os.environ.get("CONFORMANCE_OUT") or args.out
    if not out:
        print("historical needs --out or $CONFORMANCE_OUT")
        return 2
    config = selected_configs([args.config])
    tus, problems, _ = load_invocations(config, only_sources={args.source})
    for problem in problems:
        checks.check(False, "clang64.historical.invocations", problem)
    checks.check(len(tus) > 0, "clang64.historical.recorded",
                 "%s has no cxx20-permissive invocation in %s" % (args.source, args.config))
    if args.rev == "WORKTREE":
        # The checked-out bytes, through the same swap path (paired control).
        with open(os.path.join(ROOT, args.source), "rb") as stream:
            shown = subprocess.CompletedProcess([], 0, stream.read(), b"")
    else:
        shown = subprocess.run(["git", "show", "%s:%s" % (args.rev, args.source)], cwd=ROOT,
                               capture_output=True, check=False)
    checks.equal(shown.returncode, 0, "clang64.historical.git-show")
    old = os.path.realpath(os.path.join(out, "historical", args.source))
    os.makedirs(os.path.dirname(old), exist_ok=True)
    with open(old, "wb") as stream:
        stream.write(shown.stdout)
    for tu in tus:
        # Swap the recorded input for the old bytes. Quote includes resolve
        # from the file's own directory first; add the original directory.
        target = os.path.realpath(os.path.join(ROOT, args.source))
        tu["argv"] = [old if os.path.realpath(os.path.join(tu["cwd"], a)) == target else a
                      for a in tu["argv"]] + ["-I" + os.path.dirname(target)]
        checks.check(old in tu["argv"], "clang64.historical.input-swapped", tu["source"])
    remap = {os.path.dirname(old): os.path.dirname(args.source)}
    sites, advisory, blocking, incomplete, stats = sweep(tus[:1], 1, remap=remap)
    checks.equal(incomplete, [], "clang64.historical.complete")
    # Source-line text comes from the old bytes, not HEAD.
    _line_cache[os.path.join(ROOT, args.source)] = shown.stdout.decode(
        "utf-8", "replace").splitlines()
    for site in sites:
        if site["file"] == args.source:
            site["line_text"] = source_line(args.source, site["line"])
    new = report_new(checks, inventory, sites, "clang64.new-site")
    print("historical %s@%s: %d sites observed, %d not in the inventory"
          % (args.source, args.rev, len(sites), len(new)))
    provenance.emit(provenance_block("corpus.clang64.historical-vraddisps",
                                     [sys.executable] + sys.argv))
    return checks.report()


def main(argv=None):
    parser = argparse.ArgumentParser(description="Clang/64-bit legacy hazard sweep")
    sub = parser.add_subparsers(dest="command", required=True)
    p = sub.add_parser("setup", help="configure the declared trees and record compile commands")
    p.add_argument("--config", action="append")
    p.add_argument("--dxvk-root", default=DXVK_ROOT_DEFAULT,
                   help="DXVK Native SDK prefix (default %(default)s)")
    for name in ("check", "record"):
        p = sub.add_parser(name)
        p.add_argument("--jobs", type=int, default=min(16, os.cpu_count() or 1))
        p.add_argument("--no-refresh", action="store_true",
                       help="do not re-run `./waf clangdb` before sweeping")
        p.add_argument("--out")
    sub.add_parser("seeded")
    p = sub.add_parser("historical")
    p.add_argument("--rev", required=True, help="git revision, or WORKTREE for the checked-out file")
    p.add_argument("--source", required=True)
    p.add_argument("--config", required=True)
    p.add_argument("--out")
    args = parser.parse_args(argv)
    return {"setup": cmd_setup, "check": cmd_check, "record": cmd_record,
            "seeded": cmd_seeded, "historical": cmd_historical}[args.command](args)


if __name__ == "__main__":
    sys.exit(main())
