#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Shared conformance runner for RFC 0005 (Q1 / roadmap R02).
#
# One host-side orchestrator that builds, runs, and evidences every headless
# conformance suite declared in the authoritative manifest
# (quality/conformance.manifest.json). It is the connective tissue that lets
# "achieve parity as we go through all our RFCs" scale: a domain adds one
# manifest row when it lands a suite, and this runner guarantees the suite is
# built with the correct sources, run under bounded time and memory, classified
# by outcome, and recorded in immutable evidence with its full logs -- so a
# suite can never silently drift out of the gate (as the hammer geometry suites
# did when aabb.cpp grew a dependency that run_headless.sh did not track).
#
# Dependency-free: Python 3 standard library only, per RFC 0005 "the
# orchestrator may use dependency-free Python".
#
# The runner is itself tested with negative fixtures (tools/quality/tests/) that
# discover no suites, deliberately fail, crash, hang, omit or duplicate their
# result record, fail to compile, go missing, rely on assert(), and request an
# unavailable required provider -- proving outcome handling before it is
# trusted as a gate.
#
# ============================================================================

import argparse
import concurrent.futures
import datetime
import hashlib
import json
import os
import platform
import re
import shutil
import signal
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import toolchain_policy  # noqa: E402

try:
    import resource
except ImportError:  # non-POSIX hosts: limits are recorded as unavailable
    resource = None

MANIFEST_SCHEMA = "conformance-manifest/v1"
PROFILE_SCHEMA = "conformance-profile/v1"
EVIDENCE_SCHEMA = "conformance-evidence/v2"
RESULT_PROTOCOL = "checks-v1"

# Outcomes the runner can observe for a suite. `expect` in the manifest names
# the outcome a suite SHOULD produce; a suite passes the gate only when its
# observed outcome equals its expected outcome on every attempt.
OUTCOME_PASS = "pass"            # built, ran, exit 0, one valid result record
OUTCOME_FAIL = "fail"            # built, ran, exit != 0 or invalid/failing record
OUTCOME_CRASH = "crash"          # terminated by a signal
OUTCOME_TIMEOUT = "timeout"      # exceeded the profile/suite timeout
OUTCOME_COMPILE_ERROR = "compile-error"  # did not build
OUTCOME_MISSING_SOURCE = "missing-source"  # a declared source file is absent
# Runner-level outcomes that no manifest row may expect.
OUTCOME_UNAVAILABLE = "unavailable-provider"  # a required provider is absent
OUTCOME_SKIPPED = "skipped"      # optional suite whose provider is absent
OUTCOME_INVALID_ORACLE = "invalid-oracle"  # suite checks compile out under NDEBUG

VALID_EXPECT = {
    OUTCOME_PASS,
    OUTCOME_FAIL,
    OUTCOME_CRASH,
    OUTCOME_TIMEOUT,
    OUTCOME_COMPILE_ERROR,
    OUTCOME_MISSING_SOURCE,
}
VALID_KIND = {"positive", "sensitivity", "self-test"}
PROVIDER_KINDS = ("executable", "env", "path", "vulkan-device")
# Runner classes a profile declares (`runner`). A plain `check` runs the
# headless class; GPU suites run on a runner that has the device and are
# selected with --runner gpu (or any explicit selector). Every class is a
# required gate on its own runner; none is folded into another's evidence.
RUNNER_CLASSES = ("headless", "gpu")
DEFAULT_RUNNER = "headless"
# `vulkan-device:<type>` accepts these device classes from `vulkaninfo`.
VULKAN_DEVICE_TYPES = {
    "gpu": {"PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU", "PHYSICAL_DEVICE_TYPE_DISCRETE_GPU"},
    "any": {"PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU", "PHYSICAL_DEVICE_TYPE_DISCRETE_GPU",
            "PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU", "PHYSICAL_DEVICE_TYPE_CPU"},
}

# Build configurations. `release` proves test assertions stay effective in an
# optimized NDEBUG build (RFC 0005 runner contract); it is appended after the
# profile and suite flags so it cannot be undone by them.
BUILD_CONFIGS = {
    "default": [],
    "release": ["-O2", "-DNDEBUG"],
}
DEFAULT_SEED = 20260921
DEFAULT_MEMORY_LIMIT_MB = 4096

# assert() is compiled out by NDEBUG; test sources must use counted checks.
ASSERT_CALL = re.compile(r"(?<![\w.:>])assert\s*\(")
RESULT_RECORD = re.compile(r"^CONFORMANCE ([0-9]+) ([0-9]+)$", re.MULTILINE)


class ManifestError(Exception):
    """A structural problem with the manifest or a profile -- always fatal."""


def repo_root():
    # tools/quality/conformance.py -> repo root is two levels up.
    return os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


# ---------------------------------------------------------------------------
# Loading and validation
# ---------------------------------------------------------------------------

def load_json(path):
    try:
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)
    except FileNotFoundError:
        raise ManifestError("file not found: %s" % path)
    except json.JSONDecodeError as e:
        raise ManifestError("invalid JSON in %s: %s" % (path, e))


def parse_requirement(text):
    """`kind:value` provider requirement -> (kind, value)."""
    kind, sep, value = str(text).partition(":")
    if not sep or kind not in PROVIDER_KINDS or not value:
        raise ManifestError(
            "invalid provider requirement %r (expected one of %s followed by ':<name>')"
            % (text, ", ".join(PROVIDER_KINDS)))
    return kind, value


def load_manifest(path):
    data = load_json(path)
    schema = data.get("schema")
    if schema != MANIFEST_SCHEMA:
        raise ManifestError(
            "unsupported manifest schema %r (expected %r)" % (schema, MANIFEST_SCHEMA))
    suites = data.get("suites")
    if not isinstance(suites, list):
        raise ManifestError("manifest 'suites' must be a list")
    seen = set()
    for i, s in enumerate(suites):
        sid = s.get("id")
        if not sid:
            raise ManifestError("suite #%d has no id" % i)
        if sid in seen:
            raise ManifestError("duplicate suite id: %s" % sid)
        seen.add(sid)
        if s.get("units") is not None:
            if s.get("sources"):
                raise ManifestError("suite %s declares both sources and units" % sid)
            units = s["units"]
            if not isinstance(units, list) or not units:
                raise ManifestError("suite %s declares no units" % sid)
            for unit in units:
                if not unit.get("id") or not unit.get("dialect") or not unit.get("sources"):
                    raise ManifestError("suite %s has a unit without id, dialect or sources" % sid)
        elif not s.get("sources"):
            raise ManifestError("suite %s declares no sources" % sid)
        expect = s.get("expect", OUTCOME_PASS)
        if expect not in VALID_EXPECT:
            raise ManifestError(
                "suite %s has invalid expect %r (valid: %s)"
                % (sid, expect, ", ".join(sorted(VALID_EXPECT))))
        kind = s.get("kind", "positive")
        if kind not in VALID_KIND:
            raise ManifestError(
                "suite %s has invalid kind %r (valid: %s)"
                % (sid, kind, ", ".join(sorted(VALID_KIND))))
        if not s.get("profile"):
            raise ManifestError("suite %s declares no profile" % sid)
        protocol = s.get("result_protocol")
        if protocol not in (None, RESULT_PROTOCOL):
            raise ManifestError("suite %s has unknown result protocol %r" % (sid, protocol))
        # An exit status alone cannot distinguish a complete run from one that
        # returned before its checks: every suite expected to pass must report.
        if expect == OUTCOME_PASS and protocol != RESULT_PROTOCOL:
            raise ManifestError(
                "suite %s expects pass but does not declare result_protocol %r "
                "(end main with testing::ReportConformance)" % (sid, RESULT_PROTOCOL))
        min_checks = s.get("min_checks")
        if min_checks is not None and (not isinstance(min_checks, int) or min_checks < 1):
            raise ManifestError("suite %s has invalid min_checks %r" % (sid, min_checks))
        requires = s.get("requires", [])
        if not isinstance(requires, list):
            raise ManifestError("suite %s 'requires' must be a list" % sid)
        for req in requires:
            parse_requirement(req)
        if not isinstance(s.get("optional", False), bool):
            raise ManifestError("suite %s 'optional' must be a boolean" % sid)
        if s.get("optional") and not requires:
            raise ManifestError(
                "suite %s is optional but names no provider it may be skipped for" % sid)
    return data


def load_profile(profiles_dir, profile_id):
    path = os.path.join(profiles_dir, profile_id + ".json")
    data = load_json(path)
    if data.get("schema") != PROFILE_SCHEMA:
        raise ManifestError(
            "profile %s has unsupported schema %r (expected %r)"
            % (profile_id, data.get("schema"), PROFILE_SCHEMA))
    # Repository profiles name a dialect from quality/toolchain/policy.json, the
    # single owner of standard selection; a literal cxx_std is accepted only
    # for isolated self-test profiles and never together with a dialect.
    if data.get("dialect"):
        if data.get("cxx_std"):
            raise ManifestError("profile %s declares both dialect and cxx_std" % profile_id)
        try:
            policy = toolchain_policy.load_policy(repo_root())
            data["cxx_std"] = toolchain_policy.profile_std(policy, data["dialect"])
        except toolchain_policy.PolicyError as e:
            raise ManifestError("profile %s: %s" % (profile_id, e))
    if not data.get("cxx_std"):
        raise ManifestError("profile %s declares no dialect or cxx_std" % profile_id)
    runner = data.setdefault("runner", DEFAULT_RUNNER)
    if runner not in RUNNER_CLASSES:
        raise ManifestError("profile %s has unknown runner class %r (valid: %s)"
                            % (profile_id, runner, ", ".join(RUNNER_CLASSES)))
    for req in data.get("requires", []):
        parse_requirement(req)
    run_env = data.get("run_env", {})
    if not isinstance(run_env, dict) or not all(isinstance(v, str) for v in run_env.values()):
        raise ManifestError("profile %s run_env must map names to strings" % profile_id)
    if not isinstance(data.get("run_env_unset", []), list):
        raise ManifestError("profile %s run_env_unset must be a list" % profile_id)
    return data


def run_environment(profile, base=None):
    """The environment a profile's suites (and its providers) run under."""
    env = dict(os.environ if base is None else base)
    for name in profile.get("run_env_unset", []):
        env.pop(name, None)
    env.update(profile.get("run_env", {}))
    return env


def suite_sources(suite):
    if suite.get("units") is not None:
        return [src for unit in suite["units"] for src in unit["sources"]]
    return suite["sources"]


# ---------------------------------------------------------------------------
# Environment / evidence identity
# ---------------------------------------------------------------------------

def git(root, *args):
    # Capture bytes and decode leniently: git diff/status can contain non-UTF-8
    # bytes from binary files in the working tree, which would otherwise raise
    # UnicodeDecodeError under text=True and crash evidence collection.
    try:
        out = subprocess.run(
            ["git", "-C", root, *args],
            capture_output=True, text=False, check=False)
        if out.returncode != 0:
            return None
        return out.stdout.decode("utf-8", "replace")
    except (OSError, subprocess.SubprocessError):
        return None


def source_identity(root):
    rev = git(root, "rev-parse", "HEAD")
    rev = rev.strip() if rev else None
    porcelain = git(root, "status", "--porcelain")
    dirty_files = len([ln for ln in porcelain.splitlines() if ln.strip()]) if porcelain else 0
    diff = git(root, "diff", "HEAD") or ""
    dirty_digest = None
    if dirty_files:
        digest = hashlib.sha256(diff.encode("utf-8", "replace"))
        # New sources are absent from git diff; include their bytes in evidence.
        untracked = git(root, "ls-files", "--others", "--exclude-standard", "-z") or ""
        for relative in sorted(filter(None, untracked.split("\0"))):
            path = os.path.join(root, relative)
            digest.update(relative.encode("utf-8", "replace"))
            if os.path.isfile(path):
                with open(path, "rb") as stream:
                    for chunk in iter(lambda: stream.read(65536), b""):
                        digest.update(chunk)
        dirty_digest = digest.hexdigest()
    return {
        "source_revision": rev,
        "dirty": dirty_files > 0,
        "dirty_files": dirty_files,
        "dirty_digest": dirty_digest,
    }


def submodule_identity(root):
    """Pinned submodule revisions (`git submodule status`), keyed by path."""
    out = git(root, "submodule", "status", "--recursive") or ""
    modules = {}
    for line in out.splitlines():
        parts = line.strip().split()
        if len(parts) >= 2:
            state = {"-": "uninitialized", "+": "modified", "U": "conflict"}.get(line[:1], "clean")
            modules[parts[1]] = {"revision": parts[0].lstrip("-+U"), "state": state}
    return modules


def compiler_identity(cxx):
    def first_output(*args):
        try:
            out = subprocess.run([cxx, *args], capture_output=True, text=True, check=False)
        except OSError:
            return None
        lines = (out.stdout or "").splitlines()
        return lines[0].strip() if out.returncode == 0 and lines else None
    return {
        "cxx": cxx,
        "cxx_path": shutil.which(cxx),
        "cxx_version": first_output("--version"),
        "cxx_target": first_output("-dumpmachine"),
    }


def host_identity():
    return {
        "system": platform.system(),
        "release": platform.release(),
        "machine": platform.machine(),
        "python": platform.python_version(),
    }


def file_digest(root, relative):
    path = os.path.join(root, relative)
    if not os.path.isfile(path):
        return None
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def suite_input_digest(root, suite):
    """Digest of the declared sources and contract record plus the suite row.

    Includes reached by those sources are covered by the revision/dirty digest,
    not by this value; it identifies which declared inputs a result belongs to.
    """
    digest = hashlib.sha256(json.dumps(suite, sort_keys=True).encode("utf-8"))
    for relative in sorted(set(suite_sources(suite)) | ({suite["contract"]} if suite.get("contract") else set())):
        digest.update(relative.encode("utf-8"))
        digest.update((file_digest(root, relative) or "absent").encode("ascii"))
    return digest.hexdigest()


# ---------------------------------------------------------------------------
# Providers
# ---------------------------------------------------------------------------

_VULKAN_DEVICES = {}


def parse_vulkaninfo_summary(text):
    """Physical devices from `vulkaninfo --summary`: dicts of its key = value
    fields (deviceName, deviceType, apiVersion, driverName, driverInfo...)."""
    devices, current = [], None
    for line in text.splitlines():
        stripped = line.strip()
        if re.match(r"^GPU[0-9]+:$", stripped):
            current = {}
            devices.append(current)
            continue
        if current is None:
            continue
        key, sep, value = stripped.partition("=")
        if sep and key.strip():
            current[key.strip()] = value.strip()
    return [device for device in devices if device.get("deviceType")]


def vulkan_devices(env=None):
    """Physical Vulkan devices visible under `env` (cached per environment)."""
    key = tuple(sorted((env or os.environ).items()))
    if key not in _VULKAN_DEVICES:
        tool = shutil.which("vulkaninfo")
        devices, error = [], None
        if tool is None:
            error = "vulkaninfo not found on PATH"
        else:
            try:
                probe = subprocess.run([tool, "--summary"], capture_output=True, text=True,
                                       timeout=60, env=env, check=False)
                devices = parse_vulkaninfo_summary(probe.stdout)
                if probe.returncode != 0 and not devices:
                    error = "vulkaninfo exited %d" % probe.returncode
            except (OSError, subprocess.TimeoutExpired) as e:
                error = "vulkaninfo failed: %s" % e
        _VULKAN_DEVICES[key] = (devices, error)
    return _VULKAN_DEVICES[key]


def provider_available(root, requirement, env=None):
    """Return (available, detail) for one `kind:value` requirement."""
    kind, value = parse_requirement(requirement)
    if kind == "vulkan-device":
        if value not in VULKAN_DEVICE_TYPES:
            return False, "unknown device class %r (valid: %s)" % (
                value, ", ".join(sorted(VULKAN_DEVICE_TYPES)))
        devices, error = vulkan_devices(env)
        matching = [d for d in devices if d.get("deviceType") in VULKAN_DEVICE_TYPES[value]]
        if not matching:
            return False, error or ("no %s Vulkan device (found: %s)" % (
                value, ", ".join(d.get("deviceType", "?") for d in devices) or "none"))
        return True, "; ".join("%s (%s, %s)" % (d.get("deviceName"), d.get("driverName"),
                                                d.get("driverInfo")) for d in matching)
    if kind == "executable":
        path = shutil.which(value)
        return (path is not None), (path or "not found on PATH")
    if kind == "env":
        present = bool(os.environ.get(value))
        return present, ("set" if present else "environment variable unset or empty")
    path = value if os.path.isabs(value) else os.path.join(root, value)
    exists = os.path.exists(path)
    return exists, (path if exists else "%s does not exist" % path)


def suite_requirements(suite, profile=None):
    """The suite's own providers plus every provider its profile requires."""
    return list((profile or {}).get("requires", [])) + list(suite.get("requires", []))


def missing_providers(root, suite, profile=None):
    missing = []
    env = run_environment(profile) if profile else None
    for req in suite_requirements(suite, profile):
        ok, detail = provider_available(root, req, env)
        if not ok:
            missing.append("%s (%s)" % (req, detail))
    return missing


# ---------------------------------------------------------------------------
# Building and running one suite
# ---------------------------------------------------------------------------

def rooted_flags(root, flags):
    """Relative compiler response files (@path) name repository paths, like
    sources, so they resolve against --root rather than the caller's cwd."""
    return [("@" + os.path.join(root, f[1:])) if f.startswith("@") and not os.path.isabs(f[1:])
            else f for f in flags]


def build_command(root, cxx, profile, suite, out_bin, config="default"):
    flags = ["-std=" + profile["cxx_std"]]
    flags += list(profile.get("base_flags", []))
    flags += rooted_flags(root, suite.get("extra_flags", []))
    flags += BUILD_CONFIGS[config]
    includes = []
    for inc in profile.get("include_roots", []):
        includes += ["-I", os.path.join(root, inc)]
    sources = [os.path.join(root, s) for s in suite["sources"]]
    return [cxx, *flags, *includes, *sources, *rooted_flags(root, suite.get("link_flags", [])),
            *profile.get("link_flags", []), "-o", out_bin]


def separate_build_commands(root, cxx, profile, suite, out_bin, config="default"):
    """build_command() split into one compile per source plus a link, the
    shape a compiler cache (ccache) can cache: it declines commands that
    compile several sources or compile and link at once. Same flags, sources
    and link inputs, so the binary is equivalent."""
    flags = ["-std=" + profile["cxx_std"]]
    flags += list(profile.get("base_flags", []))
    flags += rooted_flags(root, suite.get("extra_flags", []))
    flags += BUILD_CONFIGS[config]
    includes = []
    for inc in profile.get("include_roots", []):
        includes += ["-I", os.path.join(root, inc)]
    commands, objects = [], []
    for index, source in enumerate(suite["sources"]):
        obj = "%s.%d.o" % (out_bin, index)
        objects.append(obj)
        commands.append([cxx, *flags, *includes, "-c", os.path.join(root, source), "-o", obj])
    # The link keeps the whole command's flags (e.g. -pthread, sanitizers) so
    # it links what the single-step build would have.
    commands.append([cxx, *flags, *objects,
                     *rooted_flags(root, suite.get("link_flags", [])),
                     *profile.get("link_flags", []), "-o", out_bin])
    return commands


def unit_build_commands(root, cxx, profile, suite, out_bin, config="default"):
    """Mixed-dialect suites: each unit compiles its sources with its own policy
    dialect (plus the profile's warnings and the suite's shared flags) into
    objects, and one link step joins them. Returns the command list."""
    policy = toolchain_policy.load_policy(root)
    includes = []
    for inc in profile.get("include_roots", []):
        includes += ["-I", os.path.join(root, inc)]
    commands, objects = [], []
    for unit in suite["units"]:
        try:
            dialect = toolchain_policy.dialect_flags(policy, unit["dialect"])
        except (KeyError, toolchain_policy.PolicyError) as e:
            raise ManifestError("suite %s unit %s: unknown dialect %s (%s)"
                                % (suite["id"], unit["id"], unit["dialect"], e))
        for index, source in enumerate(unit["sources"]):
            obj = "%s.%s.%d.o" % (out_bin, unit["id"], index)
            objects.append(obj)
            commands.append([cxx, *dialect, *profile.get("base_flags", []),
                             *rooted_flags(root, suite.get("extra_flags", [])),
                             *rooted_flags(root, unit.get("flags", [])),
                             *BUILD_CONFIGS[config], *includes,
                             "-c", os.path.join(root, source), "-o", obj])
    commands.append([cxx, *objects, *rooted_flags(root, suite.get("link_flags", [])),
                     *profile.get("link_flags", []), "-o", out_bin])
    return commands


def assert_users(root, suite):
    """Test sources (under unittests/) whose checks would vanish under NDEBUG."""
    offenders = []
    for relative in suite_sources(suite):
        if not relative.startswith("unittests/"):
            continue
        path = os.path.join(root, relative)
        with open(path, "r", encoding="utf-8", errors="replace") as stream:
            text = stream.read()
        # Ignore comments so documentation may mention assert().
        text = re.sub(r"//[^\n]*|/\*.*?\*/", "", text, flags=re.DOTALL)
        if ASSERT_CALL.search(text):
            offenders.append(relative)
    return offenders


def memory_limit_mb(profile, suite):
    """The per-process address-space cap, or None when it cannot apply.

    Sanitizer runtimes reserve very large shadow mappings, so an address-space
    cap would make them fail spuriously; those builds rely on the timeout.
    """
    flags = list(profile.get("base_flags", [])) + list(suite.get("extra_flags", []))
    if any(f.startswith("-fsanitize") for f in flags) or resource is None:
        return None
    limit = suite.get("memory_limit_mb", profile.get("memory_limit_mb", DEFAULT_MEMORY_LIMIT_MB))
    return limit or None


def _limit_child(limit_mb):
    def apply():
        resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
        if limit_mb:
            size = int(limit_mb) * 1024 * 1024
            resource.setrlimit(resource.RLIMIT_AS, (size, size))
    return apply if resource is not None else None


def execute(argv, timeout, limit_mb, env):
    """Run one suite process in its own session under time and memory bounds.

    Returns (returncode, stdout, stderr, timed_out). On timeout the whole
    process group is killed and whatever it wrote so far is retained.
    """
    proc = subprocess.Popen(
        argv, stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env,
        start_new_session=True, preexec_fn=_limit_child(limit_mb))
    try:
        out, err = proc.communicate(timeout=timeout)
        return proc.returncode, _as_text(out), _as_text(err), False
    except subprocess.TimeoutExpired:
        try:
            os.killpg(proc.pid, signal.SIGKILL)
        except (ProcessLookupError, PermissionError):
            proc.kill()
        out, err = proc.communicate()
        return proc.returncode, _as_text(out), _as_text(err), True


def classify_run(suite, returncode, stdout):
    """Classify a completed (non-timed-out) process run of `suite`.

    Shared by every runner that executes manifest suites (native, Wine) so the
    result protocol has one implementation. Returns a dict of observed fields.
    """
    observed = {"outcome": None, "exit_code": returncode, "signal": None,
                "checks": None, "failed_checks": None, "first_divergence": None}
    if returncode < 0:
        observed["signal"] = -returncode
        observed["outcome"] = OUTCOME_CRASH
        return observed
    records = RESULT_RECORD.findall(stdout)
    if len(records) == 1:
        observed["checks"], observed["failed_checks"] = map(int, records[0])
    if returncode != 0:
        observed["outcome"] = OUTCOME_FAIL
        observed["first_divergence"] = first_line(stdout, ("FAIL", "fail")) or \
            ("exit status %d" % returncode)
        return observed
    observed["outcome"] = OUTCOME_PASS
    if suite.get("result_protocol") == RESULT_PROTOCOL:
        problem = None
        if len(records) != 1:
            problem = ("missing result record (incomplete run)" if not records
                       else "%d result records (expected exactly one)" % len(records))
        elif observed["checks"] == 0:
            problem = "zero checks executed"
        elif observed["failed_checks"]:
            problem = "%d failed check(s) with exit status 0" % observed["failed_checks"]
        elif suite.get("min_checks") and observed["checks"] < suite["min_checks"]:
            problem = "%d check(s) executed, manifest requires at least %d" % (
                observed["checks"], suite["min_checks"])
        if problem:
            observed["outcome"] = OUTCOME_FAIL
            observed["first_divergence"] = "FAIL result protocol: " + problem
    return observed


def new_result(suite):
    return {
        "id": suite["id"],
        "domain": suite.get("domain"),
        "rfc": suite.get("rfc"),
        "migration": suite.get("migration"),
        "contract": suite.get("contract"),
        "kind": suite.get("kind", "positive"),
        "profile": suite.get("profile"),
        "expect": suite.get("expect", OUTCOME_PASS),
        "optional": bool(suite.get("optional", False)),
        "requires": list(suite.get("requires", [])),
        "outcome": None,
        "matched": False,
        "certified": False,
        "skip_reason": None,
        "build_ok": None,
        "exit_code": None,
        "signal": None,
        "checks": None,
        "failed_checks": None,
        "duration_s": None,
        "first_divergence": None,
        "detail": None,
        "repro": None,
        "input_digest": None,
        "logs": {},
        "attempts": [],
    }


def write_log(log_dir, name, sections):
    if not log_dir:
        return None
    os.makedirs(log_dir, exist_ok=True)
    path = os.path.join(log_dir, name)
    with open(path, "w", encoding="utf-8") as stream:
        for title, body in sections:
            stream.write("==== %s ====\n%s\n" % (title, body or ""))
    return path


def _finish(result, expect, outcome, detail=None):
    result["outcome"] = outcome
    if detail is not None:
        result["detail"] = detail
    result["matched"] = (outcome == expect)
    result["certified"] = result["matched"]
    return result


def build_suite(root, cxx, profile, suite, out_dir, config="default", log_dir=None,
                launcher=None):
    """Preflight and compile one suite without running it.

    Returns {"result", "done", "out_bin", "log_dir"}. When "done" is true the
    result is final (skipped, unavailable, missing source, invalid oracle or a
    compile error) and must not be run. Builds of different suites touch
    disjoint outputs, so any number may run at once; runs stay serial.
    """
    sid = suite["id"]
    expect = suite.get("expect", OUTCOME_PASS)
    result = new_result(suite)
    result["input_digest"] = suite_input_digest(root, suite)
    safe = sid.replace("/", "_")
    suite_log_dir = os.path.join(log_dir, safe) if log_dir else None
    built = {"result": result, "done": True, "out_bin": None, "log_dir": suite_log_dir}

    def finish(outcome, detail=None):
        return _finish(result, expect, outcome, detail)

    # An unavailable provider is never a pass. Required suites fail; optional
    # suites record why they were skipped and certify nothing.
    result["requires"] = suite_requirements(suite, profile)
    missing = missing_providers(root, suite, profile)
    if missing:
        reason = "unavailable provider(s): " + "; ".join(missing)
        if suite.get("optional"):
            result["outcome"] = OUTCOME_SKIPPED
            result["skip_reason"] = reason
            result["detail"] = reason
            result["matched"] = True   # a declared optional skip is not a gate failure
            result["certified"] = False
            return built
        finish(OUTCOME_UNAVAILABLE, reason)
        return built

    # Missing sources are a runner-level failure: a suite that cannot be found
    # can never certify anything (RFC 0005: fail on missing fixtures).
    absent = [s for s in suite_sources(suite) if not os.path.exists(os.path.join(root, s))]
    if absent:
        finish(OUTCOME_MISSING_SOURCE, "missing source(s): " + ", ".join(absent))
        return built

    offenders = assert_users(root, suite)
    if offenders:
        result["first_divergence"] = "FAIL assert() in test source(s): " + ", ".join(offenders)
        finish(OUTCOME_INVALID_ORACLE,
               "assert() is compiled out by NDEBUG; use counted checks reported "
               "through testing::ReportConformance")
        return built

    out_bin = os.path.join(out_dir, "%s.%s" % (safe.replace(".", "_"), config))
    if suite.get("units") is not None:
        commands = unit_build_commands(root, cxx, profile, suite, out_bin, config)
    elif launcher:
        commands = separate_build_commands(root, cxx, profile, suite, out_bin, config)
    else:
        commands = [build_command(root, cxx, profile, suite, out_bin, config)]
    if launcher:
        # A compiler launcher (ccache) wraps every compile and link command.
        commands = [[launcher, *cmd] for cmd in commands]
    result["repro"] = " && ".join(" ".join(cmd) for cmd in commands)

    build_output = []
    build_rc = 0
    for cmd in commands:
        try:
            build = subprocess.run(cmd, capture_output=True, text=True, check=False)
        except OSError as e:
            finish(OUTCOME_UNAVAILABLE, "compiler %s could not be started: %s" % (cmd[0], e))
            return built
        build_output.append(("$ " + " ".join(cmd), build.stdout + build.stderr))
        build_rc = build.returncode
        if build_rc != 0:
            break
    result["logs"]["build"] = write_log(suite_log_dir, "build.log", build_output)
    if build_rc != 0:
        stderr = build_output[-1][1]
        result["build_ok"] = False
        result["first_divergence"] = first_line(stderr, ("error:",)) or \
            next((ln.strip() for ln in stderr.splitlines() if "error:" in ln), None)
        finish(OUTCOME_COMPILE_ERROR, tail(stderr))
        if suite.get("expected_diagnostic"):
            result["matched"] = result["matched"] and suite["expected_diagnostic"] in stderr
            result["certified"] = result["matched"]
        return built
    result["build_ok"] = True
    built.update(done=False, out_bin=out_bin)
    return built


def run_suite(root, cxx, profile, suite, out_dir, config="default", repeat=1,
              seed=DEFAULT_SEED, log_dir=None, launcher=None, built=None):
    """Build (unless `built` is a build_suite() result) and run one suite."""
    if built is None:
        built = build_suite(root, cxx, profile, suite, out_dir, config=config,
                            log_dir=log_dir, launcher=launcher)
    result = built["result"]
    if built["done"]:
        return result
    sid = suite["id"]
    expect = suite.get("expect", OUTCOME_PASS)
    timeout = suite.get("timeout_seconds", profile.get("timeout_seconds", 60))
    out_bin = built["out_bin"]
    suite_log_dir = built["log_dir"]

    limit = memory_limit_mb(profile, suite)
    result["memory_limit_mb"] = limit
    result["timeout_seconds"] = timeout
    env = run_environment(profile)
    env["CONFORMANCE_SEED"] = str(seed)
    env["CONFORMANCE_SUITE"] = sid

    # Every attempt is retained: repeats expose nondeterminism and are never a
    # retry-until-green mechanism. The suite matches only if all attempts do.
    for attempt in range(repeat):
        env["CONFORMANCE_ATTEMPT"] = str(attempt)
        start = datetime.datetime.now()
        rc, stdout, stderr, timed_out = execute([out_bin], timeout, limit, env)
        duration = round((datetime.datetime.now() - start).total_seconds(), 3)
        if timed_out:
            observed = {"outcome": OUTCOME_TIMEOUT, "exit_code": None, "signal": None,
                        "checks": None, "failed_checks": None,
                        "first_divergence": "timed out after %ss" % timeout}
        else:
            observed = classify_run(suite, rc, stdout)
        observed["attempt"] = attempt
        observed["duration_s"] = duration
        observed["log"] = write_log(suite_log_dir, "run.%d.log" % attempt, [
            ("command", out_bin),
            ("seed", str(seed)),
            ("exit", "timeout" if timed_out else str(rc)),
            ("stdout", stdout),
            ("stderr", stderr),
        ])
        observed["detail"] = tail(stdout + stderr)
        observed["matched"] = observed["outcome"] == expect
        if suite.get("expected_divergence"):
            # A negative row names the defect it must be rejected for.
            observed["matched"] = observed["matched"] and \
                suite["expected_divergence"] in (observed["first_divergence"] or "")
        if "expected_signal" in suite:
            observed["matched"] = observed["matched"] and observed["signal"] == suite["expected_signal"]
        result["attempts"].append(observed)

    # Report the first mismatching attempt (or the last one when all matched).
    decisive = next((a for a in result["attempts"] if not a["matched"]), result["attempts"][-1])
    for key in ("exit_code", "signal", "checks", "failed_checks", "first_divergence", "detail"):
        result[key] = decisive[key]
    result["duration_s"] = round(sum(a["duration_s"] for a in result["attempts"]), 3)
    result["outcome"] = decisive["outcome"]
    result["matched"] = all(a["matched"] for a in result["attempts"])
    result["certified"] = result["matched"]
    return result


def _as_text(v):
    if v is None:
        return ""
    if isinstance(v, bytes):
        return v.decode("utf-8", "replace")
    return v


def first_line(text, prefixes):
    for ln in _as_text(text).splitlines():
        stripped = ln.strip()
        for p in prefixes:
            if stripped.startswith(p):
                return stripped
    return None


def tail(text, n=20):
    lines = _as_text(text).splitlines()
    return "\n".join(lines[-n:]) if lines else ""


# ---------------------------------------------------------------------------
# Selection
# ---------------------------------------------------------------------------

def selectors_from(args):
    selectors = []
    for field in ("suite", "domain", "rfc", "profile"):
        values = getattr(args, field, None)
        if values:
            selectors.append(("id" if field == "suite" else field, set(values)))
    return selectors


def select_suites(manifest, args):
    suites = manifest["suites"]
    selectors = selectors_from(args)
    if not selectors:
        return list(suites), None

    # Every requested value must match something: a mistyped id or domain is
    # a required selector that matched nothing, not a smaller passing run.
    for field, wanted in selectors:
        known = {s.get(field) for s in suites}
        unmatched = sorted(wanted - known)
        if unmatched:
            return [], "no suite matches required %s selector(s): %s" % (field, ", ".join(unmatched))
    selected = [s for s in suites if all(s.get(field) in wanted for field, wanted in selectors)]
    if not selected:
        wanted = "; ".join("%s in {%s}" % (f, ", ".join(sorted(w))) for f, w in selectors)
        return [], "no suite matches required selector(s): " + wanted
    return selected, None


# ---------------------------------------------------------------------------
# Commands
# ---------------------------------------------------------------------------

def cmd_list(args):
    root = args.root
    manifest = load_manifest(os.path.join(root, args.manifest))
    for s in manifest["suites"]:
        print("%-34s %-12s rfc:%-5s expect:%-14s %s"
              % (s["id"], s.get("domain", "-"), s.get("rfc", "-"),
                 s.get("expect", OUTCOME_PASS), s.get("kind", "positive")))
    print("\n%d suite(s)" % len(manifest["suites"]))
    return 0


def plan(root, manifest, args):
    """Enumerate the selected suites and provider/profile combinations before
    anything executes. Returns (selected, profiles) or raises/returns an error."""
    if not manifest["suites"]:
        # Zero discovery is a hard failure: an empty required run cannot certify
        # anything (RFC 0005 runner contract).
        return None, None, "manifest declares zero suites (zero discovery)"
    selected, sel_err = select_suites(manifest, args)
    if sel_err:
        return None, None, sel_err
    profiles_dir = os.path.join(root, manifest.get("profiles_dir", "quality/profiles"))
    profiles = {}
    for s in selected:
        pid = s["profile"]
        if pid not in profiles:
            profiles[pid] = load_profile(profiles_dir, pid)
    runner = selected_runner(args)
    if runner != "all":
        selected = [s for s in selected if profiles[s["profile"]]["runner"] == runner]
        if not selected:
            return None, None, "no suite runs on the %s runner class" % runner
        profiles = {pid: p for pid, p in profiles.items()
                    if any(s["profile"] == pid for s in selected)}
    return selected, profiles, None


def selected_runner(args):
    """--runner, else the headless class for an unselected run and every class
    when the caller named suites, domains, RFCs or profiles explicitly."""
    runner = getattr(args, "runner", None)
    if runner:
        return runner
    return "all" if selectors_from(args) else DEFAULT_RUNNER


def cmd_plan(args):
    root = args.root
    manifest = load_manifest(os.path.join(root, args.manifest))
    selected, profiles, err = plan(root, manifest, args)
    if err:
        print("FATAL: " + err, file=sys.stderr)
        return 2
    for s in selected:
        missing = missing_providers(root, s, profiles[s["profile"]])
        status = "ready"
        if missing:
            status = ("skip (optional): " if s.get("optional") else "UNAVAILABLE: ") + "; ".join(missing)
        print("%-40s profile:%-22s %s" % (s["id"], s["profile"], status))
    print("\n%d suite(s) selected across %d profile(s)" % (len(selected), len(profiles)))
    return 0


def evidence_paths(root, out_arg, build_dir, stamp):
    """(evidence json path or None, log directory)."""
    if out_arg == "-":
        return None, os.path.join(build_dir, "logs", stamp)
    if out_arg:
        path = out_arg if os.path.isabs(out_arg) else os.path.join(root, out_arg)
    else:
        path = os.path.join(root, "quality-results", "conformance.%s.json" % stamp)
    return path, os.path.splitext(path)[0] + ".logs"


def cmd_check(args):
    root = args.root
    manifest_path = os.path.join(root, args.manifest)
    manifest = load_manifest(manifest_path)
    selected, profiles, err = plan(root, manifest, args)
    if err:
        print("FATAL: " + err, file=sys.stderr)
        return 2
    if args.repeat < 1:
        print("FATAL: --repeat must be at least 1", file=sys.stderr)
        return 2
    if args.jobs < 1:
        print("FATAL: --jobs must be at least 1", file=sys.stderr)
        return 2
    if args.launcher and shutil.which(args.launcher) is None:
        print("FATAL: compiler launcher %r is unavailable" % args.launcher, file=sys.stderr)
        return 2

    cxx = args.cxx
    if shutil.which(cxx) is None:
        # The compiler is the one provider every suite requires.
        print("FATAL: required compiler provider %r is unavailable" % cxx, file=sys.stderr)
        return 2
    out_dir = args.build_dir
    os.makedirs(out_dir, exist_ok=True)

    stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    evidence_path, log_dir = evidence_paths(root, args.out, out_dir, stamp)

    expected_ids = [s["id"] for s in selected]
    run = {
        "invocation": [sys.executable, os.path.relpath(os.path.abspath(__file__), root)]
                      + list(args.argv),
        "selectors": {f: sorted(v) for f, v in selectors_from(args)},
        "runner": selected_runner(args),
        "config": args.config,
        "config_flags": BUILD_CONFIGS[args.config],
        "repeat": args.repeat,
        "seed": args.seed,
        "jobs": args.jobs,
        "launcher": args.launcher,
    }
    print("conformance: %d suite(s) selected, cxx=%s, config=%s, repeat=%d, seed=%d, jobs=%d%s"
          % (len(selected), cxx, args.config, args.repeat, args.seed, args.jobs,
             ", launcher=" + args.launcher if args.launcher else ""))

    identity = source_identity(root)
    identity.update(compiler_identity(cxx))
    identity["host"] = host_identity()
    identity["submodules"] = submodule_identity(root)
    # GPU evidence names the physical devices the suites could see.
    for pid, profile in profiles.items():
        if any(parse_requirement(r)[0] == "vulkan-device" for r in profile.get("requires", [])):
            devices, error = vulkan_devices(run_environment(profile))
            identity.setdefault("vulkan_devices", {})[pid] = {
                "devices": devices, "error": error}

    # Evidence is written before execution and after every suite, so a runner
    # that is killed leaves an artifact whose decision is "incomplete".
    results = []

    def record(decision):
        if evidence_path:
            write_evidence(evidence_path, build_evidence(
                root, manifest_path, manifest, identity, profiles, run, results,
                expected_ids, decision, log_dir))

    record("incomplete")
    # With --jobs > 1 every suite is compiled first, concurrently; the runs
    # then happen one at a time in manifest order, with no compiler competing
    # for the host, so timeouts, memory limits and evidence order are the same
    # as a serial run.
    prebuilt = {}
    if args.jobs > 1:
        prebuilt = build_all(root, cxx, profiles, selected, out_dir, args, log_dir)
    for s in selected:
        r = run_suite(root, cxx, profiles[s["profile"]], s, out_dir, config=args.config,
                      repeat=args.repeat, seed=args.seed, log_dir=log_dir,
                      launcher=args.launcher, built=prebuilt.get(s["id"]))
        results.append(r)
        record("incomplete")
        if r["outcome"] == OUTCOME_SKIPPED:
            status = "skip"
        else:
            status = "ok  " if r["matched"] else "FAIL"
        line = "  [%s] %-34s expect=%-14s got=%-14s" % (
            status, r["id"], r["expect"], r["outcome"])
        if r["checks"] is not None:
            line += " checks=%d" % r["checks"]
        if r["duration_s"] is not None:
            line += " (%.2fs)" % r["duration_s"]
        print(line)
        if r["skip_reason"]:
            print("        " + r["skip_reason"])
        elif not r["matched"]:
            if r["first_divergence"]:
                print("        first divergence: " + r["first_divergence"])
            if r["detail"]:
                print("        " + r["detail"].replace("\n", "\n        "))
            failing_log = next((a["log"] for a in r["attempts"] if not a["matched"]), None) \
                or r["logs"].get("build")
            if failing_log:
                print("        log: " + os.path.relpath(failing_log, root))

    decision = decide(expected_ids, results)
    record(decision)
    counts = count(results)
    print("\n%d suite(s): %d matched, %d mismatched, %d skipped -> %s"
          % (counts["total"], counts["matched"], counts["mismatched"], counts["skipped"],
             decision.upper()))
    if sorted(expected_ids) != sorted(r["id"] for r in results):
        print("FATAL: reconciliation mismatch (expected != executed)", file=sys.stderr)
    if evidence_path:
        print("evidence: " + os.path.relpath(evidence_path, root))
    return 0 if decision == "pass" else 1


def build_all(root, cxx, profiles, selected, out_dir, args, log_dir):
    """Compile every selected suite on `args.jobs` workers; {suite id: build}."""
    started = datetime.datetime.now()
    builds = {}
    pool = concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs)
    try:
        futures = {pool.submit(build_suite, root, cxx, profiles[s["profile"]], s, out_dir,
                               config=args.config, log_dir=log_dir,
                               launcher=args.launcher): s["id"] for s in selected}
        for future in concurrent.futures.as_completed(futures):
            builds[futures[future]] = future.result()
    except BaseException:
        pool.shutdown(wait=True, cancel_futures=True)
        raise
    pool.shutdown(wait=True)
    print("conformance: built %d suite(s) on %d job(s) in %.1fs"
          % (len(builds), args.jobs, (datetime.datetime.now() - started).total_seconds()))
    return builds


def count(results):
    skipped = sum(1 for r in results if r["outcome"] == OUTCOME_SKIPPED)
    matched = sum(1 for r in results if r["matched"] and r["outcome"] != OUTCOME_SKIPPED)
    return {
        "total": len(results),
        "matched": matched,
        "mismatched": sum(1 for r in results if not r["matched"]),
        "skipped": skipped,
        "certified": sum(1 for r in results if r["certified"]),
        "checks": sum(r["checks"] or 0 for r in results),
    }


def decide(expected_ids, results):
    executed = [r["id"] for r in results]
    if sorted(expected_ids) != sorted(executed) or len(set(executed)) != len(executed):
        return "fail"
    return "pass" if all(r["matched"] for r in results) else "fail"


def build_evidence(root, manifest_path, manifest, identity, profiles, run, results,
                   expected_ids, decision, log_dir):
    executed_ids = [r["id"] for r in results]
    return {
        "schema": EVIDENCE_SCHEMA,
        "generated_at": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "manifest": os.path.relpath(manifest_path, root),
        "manifest_schema": manifest.get("schema"),
        "manifest_digest": file_digest(root, os.path.relpath(manifest_path, root)),
        "result_protocol": RESULT_PROTOCOL,
        "identity": identity,
        "run": run,
        "logs": os.path.relpath(log_dir, root) if log_dir else None,
        "profiles": {
            pid: {
                "dialect": p.get("dialect"),
                "cxx_std": p.get("cxx_std"),
                "base_flags": p.get("base_flags", []),
                "include_roots": p.get("include_roots", []),
                "timeout_seconds": p.get("timeout_seconds"),
                "runner": p.get("runner", DEFAULT_RUNNER),
                "link_flags": p.get("link_flags", []),
                "requires": p.get("requires", []),
                "run_env": p.get("run_env", {}),
                "run_env_unset": p.get("run_env_unset", []),
                "memory_limit_mb": p.get("memory_limit_mb", DEFAULT_MEMORY_LIMIT_MB),
            } for pid, p in profiles.items()
        },
        "expected_ids": sorted(expected_ids),
        "executed_ids": sorted(executed_ids),
        "reconciled": sorted(expected_ids) == sorted(executed_ids),
        "counts": count(results),
        "suites": results,
        "decision": decision,
    }


def write_evidence(path, evidence):
    os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
    tmp = path + ".tmp"
    with open(tmp, "w", encoding="utf-8") as f:
        json.dump(evidence, f, indent=2, sort_keys=False)
        f.write("\n")
    os.replace(tmp, path)
    return path


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def add_selectors(p):
    p.add_argument("--suite", action="append", help="Select by suite id (repeatable).")
    p.add_argument("--domain", action="append", help="Select by domain, e.g. Q-EDITOR.")
    p.add_argument("--rfc", action="append", help="Select by RFC number, e.g. 0002.")
    p.add_argument("--profile", action="append", help="Select by profile id.")
    p.add_argument("--runner", choices=RUNNER_CLASSES + ("all",),
                   help="Runner class to run (default: headless for an unselected run, "
                        "every class when suites, domains, RFCs or profiles are named).")


def build_parser():
    p = argparse.ArgumentParser(
        prog="conformance.py",
        description="Shared conformance runner (RFC 0005 Q1 / roadmap R02).")
    p.add_argument("--root", default=repo_root(),
                   help="Repository root (default: inferred from script location).")
    p.add_argument("--manifest", default="quality/conformance.manifest.json",
                   help="Manifest path, relative to --root.")
    sub = p.add_subparsers(dest="command", required=True)

    lst = sub.add_parser("list", help="List declared conformance suites.")
    lst.set_defaults(func=cmd_list)

    pln = sub.add_parser("plan", help="Enumerate selected suites and provider availability "
                                      "without building or running anything.")
    add_selectors(pln)
    pln.set_defaults(func=cmd_plan)

    chk = sub.add_parser("check", help="Build, run, and evidence conformance suites.")
    chk.add_argument("--cxx", default=os.environ.get("CXX", "g++"),
                     help="C++ compiler to use (default: $CXX or g++).")
    add_selectors(chk)
    chk.add_argument("--config", choices=sorted(BUILD_CONFIGS), default="default",
                     help="Build configuration; 'release' appends %s."
                          % " ".join(BUILD_CONFIGS["release"]))
    chk.add_argument("--repeat", type=int, default=1,
                     help="Run each suite N times; every attempt is retained and all must match.")
    chk.add_argument("--seed", type=int, default=DEFAULT_SEED,
                     help="Deterministic seed exported to suites as CONFORMANCE_SEED.")
    chk.add_argument("--jobs", type=int, default=1,
                     help="Compile suites on N concurrent jobs before running them; runs stay "
                          "serial in manifest order (default: 1, build and run each in turn).")
    chk.add_argument("--launcher", default=os.environ.get("CONFORMANCE_LAUNCHER") or None,
                     help="Compiler launcher prefixed to every build command, e.g. ccache "
                          "(default: $CONFORMANCE_LAUNCHER).")
    chk.add_argument("--build-dir", default=os.path.join(repo_root(), "build", "quality"),
                     help="Directory for compiled suite binaries.")
    chk.add_argument("--out", default=None,
                     help="Evidence JSON path (default: quality-results/conformance.<timestamp>.json, "
                          "logs beside it in <name>.logs/); '-' skips the evidence file and "
                          "keeps logs under --build-dir.")
    chk.set_defaults(func=cmd_check)
    return p


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    args = build_parser().parse_args(argv)
    args.argv = argv
    try:
        return args.func(args)
    except ManifestError as e:
        print("FATAL: " + str(e), file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
