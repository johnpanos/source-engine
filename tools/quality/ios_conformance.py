#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Run the conformance manifest's compiled suites on the connected iPhone.

The iOS device profile (quality/profiles/ios-arm64-device.json) runs the
suites of its host profiles (the Linux headless core and native Vulkan GPU
profiles) with the same manifest rows, the same checks-v1 protocol and the
same classification as conformance.py:

  build   Each suite is compiled on Linux with the iOS toolchain, using
          conformance.py's own compile commands through a compiler wrapper,
          with main renamed. Its objects are partially linked (ld64 -r) into
          one module object whose only external symbol is its entry,
          ConformanceSuite_<n>, so suites cannot collide. The entries link
          into one test-host app (SDL3 owns main and starts UIKit; MoltenVK
          serves Vulkan) together with the repository files the suites read.
  deploy  ios-deploy.sh signs and installs the app under the profile's
          bundle id.
  run     The app is launched once per suite through devicectl (argv: suite
          id, seed, attempt), so each suite runs in its own process, and the
          console is classified by conformance.classify_run.

    python3 tools/quality/ios_conformance.py check --out quality-results/ios-conformance.json
    python3 tools/quality/ios_conformance.py check --suite foundation.expected --no-deploy
    python3 tools/quality/ios_conformance.py check --app render_lab --device-profile tvos-arm64-device

--app NAME builds one of the device profile's `apps` instead of the
conformance host: the same host with that app's suites, bundle id (taken from
the app's product profile), bundle name and display name, in
<build_root>/apps/NAME. --env KEY=VALUE passes an environment variable to
every launch, and --extra-timeout adds seconds to each suite's timeout.

Suites the profile cannot run (a first-party shared library fixture, a
sanitizer runtime) are reported as skipped with the profile's reason: never
certified.
"""

import argparse
import concurrent.futures
import datetime
import json
import os
from pathlib import Path
import plistlib
import re
import shlex
import shutil
import subprocess
import sys

import conformance
import ios_device
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "kiln"))
import sepipe_loader  # noqa: E402
import toolchain_policy

ROOT = Path(conformance.repo_root())
DEFAULT_DEVICE_PROFILE = "ios-arm64-device"
# The device profile's product build (its `build_root`, which the product's
# build script configures): set by use_profile().
BUILD_DIR = ROOT / "build-ios/conformance"
WAF_CACHE = ROOT / "build-ios/waf/c4che/_cache.py"
DEPS_PREFIX = ROOT / "build-ios/deps/prefix"
MAIN_DEFINITION = re.compile(r"^\s*int\s+main\s*\(([^)]*)\)", re.M)
PATH_LITERAL = re.compile(r'"((?:quality|unittests|materialsystem|public|tools|game|engine|'
                          r'mapcontainer|hammer|vphysics[a-z0-9_]*|box3d|shaders|content)'
                          r'/[A-Za-z0-9_./-]+)"')
# The largest single repository input bundled for suites (a fixture corpus).
MAX_BUNDLED_BYTES = 64 << 20
# Launches per attempt when the device console captured nothing from the app.
CAPTURE_ATTEMPTS = 3


def load_profile(profile_id=DEFAULT_DEVICE_PROFILE):
    return json.loads((ROOT / "quality/profiles" / (profile_id + ".json")).read_text())


def use_profile(profile):
    """Points the build at the profile's product build: build-ios for the
    iOS profile, build-tvos for the tvOS one."""
    global BUILD_DIR, WAF_CACHE, DEPS_PREFIX
    build_root = ROOT / profile.get("build_root", "build-ios")
    BUILD_DIR = build_root / "conformance"
    WAF_CACHE = build_root / "waf/c4che/_cache.py"
    DEPS_PREFIX = build_root / "deps/prefix"


def use_app(profile, name):
    """The device profile as one of its `apps`: a host app with its own
    suites, bundle and product profile (the bundle id comes from the product
    profile, which owns it), built in <build_root>/apps/<name>."""
    global BUILD_DIR
    apps = {k: v for k, v in profile.get("apps", {}).items() if not k.startswith("_")}
    if name not in apps:
        raise SystemExit("%s declares no app %s (apps: %s)" % (
            profile["id"], name, ", ".join(sorted(apps)) or "none"))
    app = apps[name]
    product = sepipe_loader.resolve_profile(ROOT / app["product_profile"])
    merged = dict(profile)
    merged.update({
        "product_profile": app["product_profile"],
        "bundle_id": product[product["target"]["os"]]["bundle_id"],
        "app_bundle": app["app_bundle"],
        "display_name": app["display_name"],
        "app_suites": app["suites"],
        "hosts": profile["hosts"] + [h for h in app.get("hosts", []) if h not in profile["hosts"]],
    })
    BUILD_DIR = ROOT / profile.get("build_root", "build-ios") / "apps" / name
    return merged


def target_os(profile):
    """The product profile's target OS: "ios" or "tvos"."""
    product = sepipe_loader.resolve_profile(ROOT / profile["product_profile"])
    return product["target"]["os"]


def waf_env():
    """The iOS Waf configuration (build-ios-app.sh configures it): the
    compiler, partial linker and dependency flags the product uses."""
    env = {}
    exec(compile(WAF_CACHE.read_text(), str(WAF_CACHE), "exec"), {}, env)
    return env


# ---------------------------------------------------------------------------
# selection
# ---------------------------------------------------------------------------

def unsupported_reason(profile, suite):
    # Suites the profile's OS cannot host at all (a tvOS API that does not
    # exist), each with its reason.
    reason = profile.get("unsupported_suites", {}).get(suite["id"])
    if reason:
        return reason
    if any(unit.get("link") == "shared" for unit in suite.get("units", [])):
        return profile["unsupported"]["shared-unit"]
    flags = suite.get("extra_flags", []) + [f for unit in suite.get("units", [])
                                            for f in unit.get("flags", [])]
    abi_seeds = {f for f in flags if f.startswith("-D_GLIBCXX_USE_CXX11_ABI=")}
    if suite.get("kind") == "sensitivity" and len(abi_seeds) > 1:
        return profile["unsupported"]["libstdcxx-dual-abi"]
    if any(f.startswith("-fsanitize") for f in flags) or any(
            r.startswith("env:CONFORMANCE_TSAN") for r in suite.get("requires", [])):
        return profile["unsupported"]["sanitizer"]
    return None


def select(manifest, profile, args):
    hosts = set(profile["hosts"])
    suites = [s for s in manifest["suites"] if s["profile"] in hosts and not s.get("command")]
    if profile.get("app_suites") and not args.suite:
        suites = [s for s in suites if s["id"] in profile["app_suites"]]
    if args.suite:
        wanted = set(args.suite)
        unknown = wanted - {s["id"] for s in suites}
        if unknown:
            raise SystemExit("not an iOS-runnable suite: %s" % ", ".join(sorted(unknown)))
        suites = [s for s in suites if s["id"] in wanted]
    if args.domain:
        suites = [s for s in suites if s["domain"] in args.domain]
    if args.rfc:
        suites = [s for s in suites if s["rfc"] in args.rfc]
    return suites


# ---------------------------------------------------------------------------
# build
# ---------------------------------------------------------------------------

def write_wrapper(env, profile, out):
    """The compiler conformance.py's commands run: the iOS clang++ with the
    product's target and SDK, the dependency headers, hidden visibility and a
    renamed main."""
    include = str(DEPS_PREFIX / "include")
    flags = env["CXX"][1:] + ["-fvisibility=hidden", "-I", include,
                              "-Dmain=conformance_suite_main", "-DCONFORMANCE_IOS_DEVICE=1"
                              ] + profile.get("extra_flags", [])
    for directory in profile.get("include_after", []):
        flags += ["-idirafter", str(ROOT / directory)]
    path = out / "ios-cxx"
    path.write_text("#!/bin/sh\nexec %s %s \"$@\"\n" % (shlex.quote(env["CXX"][0]),
                                                      " ".join(shlex.quote(f) for f in flags)))
    path.chmod(0o755)
    return path


def translate_platform(profile, suite):
    """The suite row with its Linux platform defines replaced by the iOS
    product's (the profile's platform_defines)."""
    table = {k: v for k, v in profile.get("platform_defines", {}).items()
             if not k.startswith("_")}

    def translate(flags):
        out = []
        for flag in flags:
            out += table.get(flag, [flag])
        return out
    suite = dict(suite)
    suite["extra_flags"] = translate(suite.get("extra_flags", []))
    if suite.get("units"):
        suite["units"] = [dict(unit, flags=translate(unit.get("flags", [])))
                          for unit in suite["units"]]
    return suite


def compile_commands(host_profile, suite, wrapper, out_bin, config):
    """conformance.py's per-source compile commands (its link is replaced)."""
    if suite.get("units"):
        commands = conformance.unit_build_commands(str(ROOT), str(wrapper), host_profile, suite,
                                                   out_bin, config)
    else:
        commands = conformance.separate_build_commands(str(ROOT), str(wrapper), host_profile,
                                                       suite, out_bin, config)
    compiles = [c for c in commands if "-c" in c]
    objects = [c[c.index("-o") + 1] for c in compiles]
    return compiles, objects


def main_takes_arguments(suite):
    for relative in conformance.suite_sources(suite):
        if not relative.endswith((".cpp", ".cc", ".cxx", ".mm")):
            continue
        text = (ROOT / relative).read_text(errors="replace")
        match = MAIN_DEFINITION.search(text)
        if match:
            return match.group(1).strip() not in ("", "void")
    raise conformance.ManifestError("suite %s defines no main()" % suite["id"])


def entry_source(index, suite):
    if main_takes_arguments(suite):
        declaration, call = "int conformance_suite_main( int, char ** );", \
            "conformance_suite_main( argc, argv )"
    else:
        declaration, call = "int conformance_suite_main();", "conformance_suite_main()"
    return ("// Generated by tools/quality/ios_conformance.py: %s\n"
            "%s\n"
            "extern \"C\" __attribute__( ( visibility( \"default\" ) ) ) int ConformanceSuite_%s(\n"
            "    int argc, char **argv )\n{\n\t( void )argc;\n\t( void )argv;\n"
            "\treturn %s;\n}\n" % (suite["id"], declaration, index, call))


def run_logged(command, log):
    proc = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    with open(log, "a", encoding="utf-8") as stream:
        stream.write("$ %s\n%s\n" % (" ".join(shlex.quote(c) for c in command), proc.stdout))
    return proc.returncode == 0, proc.stdout


def build_suite(env, profile, profiles, suite, index, wrapper, launcher, config):
    """Compiles one suite into its module object; returns (state, detail):
    built (the module), build-failed, or compile-error for a suite whose row
    expects one (a negative compile test; detail is the compiler output)."""
    expects_error = suite.get("expect") == conformance.OUTCOME_COMPILE_ERROR
    suite = translate_platform(profile, suite)
    directory = BUILD_DIR / "suites" / ("%03d" % index)
    directory.mkdir(parents=True, exist_ok=True)
    log = directory / "build.log"
    log.write_text("")
    out_bin = str(directory / "suite")
    try:
        compiles, objects = compile_commands(profiles[suite["profile"]], suite, wrapper,
                                             out_bin, config)
        entry = directory / "entry.cpp"
        entry.write_text(entry_source(index, suite))
    except (conformance.ManifestError, OSError) as error:
        return "build-failed", str(error)
    entry_object = str(directory / "entry.o")
    compiles.append([str(wrapper), "-std=c++20", "-c", str(entry), "-o", entry_object])
    for command in compiles:
        ok, output = run_logged(launcher + command, log)
        if not ok:
            if expects_error:
                return "compile-error", output[-4000:]
            first = next((line for line in output.splitlines() if "error" in line), output[:300])
            return "build-failed", "compile failed: " + first.strip()
    if expects_error:
        return "build-failed", "compiled, but the row expects a compile error"
    module = str(BUILD_DIR / "modules" / ("suite_%03d.o" % index))
    ok, output = run_logged(env["LD64"] + ["-r", "-arch", env["LD64_ARCH"], *objects,
                                           entry_object, "-exported_symbol",
                                           "_ConformanceSuite_%d" % index, "-o", module], log)
    if not ok:
        return "build-failed", "partial link failed: " + output.strip()[-300:]
    return "built", module


def repository_inputs(suites):
    """Repository files and directories the suites name as path literals,
    bundled at the same relative paths (the app runs from that root)."""
    found = set()
    for suite in suites:
        for relative in conformance.suite_sources(suite):
            if not relative.endswith((".cpp", ".h", ".cc", ".inl")):
                continue
            for literal in PATH_LITERAL.findall((ROOT / relative).read_text(errors="replace")):
                path = ROOT / literal
                if path.exists():
                    found.add(literal.rstrip("/"))
    return sorted(found)


def bundle_inputs(app, inputs):
    # The host always enters root/, which holds nothing when no suite reads files.
    (app / "root").mkdir(parents=True, exist_ok=True)
    copied, skipped = [], []
    for relative in inputs:
        source, target = ROOT / relative, app / "root" / relative
        size = sum(f.stat().st_size for f in source.rglob("*") if f.is_file()) \
            if source.is_dir() else source.stat().st_size
        if size > MAX_BUNDLED_BYTES:
            skipped.append(relative)
            continue
        target.parent.mkdir(parents=True, exist_ok=True)
        if source.is_dir():
            shutil.copytree(source, target, dirs_exist_ok=True)
        else:
            shutil.copy2(source, target)
        copied.append(relative)
    return copied, skipped


# ---------------------------------------------------------------------------
# programs (benchmark hosts the budget tools drive)
# ---------------------------------------------------------------------------

class _Anything:
    """A permissive stand-in for Waf objects a wscript's build() touches."""

    def __init__(self, **values):
        self.__dict__.update(values)

    def __getattr__(self, name):
        return _Anything()

    def __call__(self, *args, **kwargs):
        return _Anything()

    def __bool__(self):
        return False

    def __iter__(self):
        return iter(())


class _RecordingBuild(_Anything):
    """Records the task generators a wscript declares: Waf stays the one
    owner of a program's sources, includes, defines and uses."""

    def __init__(self):
        super().__init__(env=_Anything(DEST_OS="ios"), generators=[])

    def get_taskgen_count(self):
        return 0

    def _record(self, **kwargs):
        self.generators.append(kwargs)
        return _Anything()

    program = shlib = stlib = objects = _record

    def __call__(self, **kwargs):
        return self._record(**kwargs)


def recorded_program(wscript, target):
    waf_dir = next(ROOT.glob(".waf3-*"), None)
    if waf_dir and str(waf_dir) not in sys.path:
        sys.path.insert(0, str(waf_dir))
    namespace = {"__file__": str(ROOT / wscript)}
    exec(compile((ROOT / wscript).read_text(), str(ROOT / wscript), "exec"), namespace)
    recorder = _RecordingBuild()
    namespace["build"](recorder)
    for generator in recorder.generators:
        if generator.get("target") == target:
            return generator
    raise conformance.ManifestError("%s declares no target %s" % (wscript, target))


def build_program(env, profile, name, spec, launcher):
    """Compiles one benchmark program with the product's release flags into a
    module object exporting ConformanceProgram_<name>. Returns (ok, detail,
    module objects of the product the program links)."""
    directory = BUILD_DIR / "programs" / name
    directory.mkdir(parents=True, exist_ok=True)
    log = directory / "build.log"
    log.write_text("")
    try:
        generator = recorded_program(spec["wscript"], spec["target"])
    except (conformance.ManifestError, OSError, KeyError) as error:
        return False, str(error), []
    base = (ROOT / spec["wscript"]).parent
    sources = [base / f for f in _list(generator.get("source"))]
    includes = [base / f for f in _list(generator.get("includes"))]
    policy = toolchain_policy.load_policy(str(ROOT))
    dialect = toolchain_policy.dialect_flags(
        policy, toolchain_policy.target_dialect(policy, spec["target"], "c++"))
    flags = [f for f in env["CXXFLAGS"] if f not in ("-MMD",) and not f.startswith("-std=")]
    flags += dialect + ["-D" + d for d in env["DEFINES"]]
    flags += ["-D" + d for d in _list(generator.get("defines")) + spec.get("defines", [])]
    flags += ["-Dmain=conformance_suite_main", "-I", str(DEPS_PREFIX / "include")]
    for include in includes:
        flags += ["-I", str(include)]
    for directory_after in profile.get("include_after", []):
        flags += ["-idirafter", str(ROOT / directory_after)]
    objects = []
    for index, source in enumerate(sources):
        obj = str(directory / ("%d.o" % index))
        ok, output = run_logged(launcher + env["CXX"] + flags + ["-c", str(source), "-o", obj], log)
        if not ok:
            first = next((l for l in output.splitlines() if "error" in l), output[:300])
            return False, "compile failed: " + first.strip(), []
        objects.append(obj)
    fake_suite = {"id": name, "sources": [str(Path(s).relative_to(ROOT)) for s in sources]}
    entry = directory / "entry.cpp"
    entry.write_text(entry_source(name, fake_suite).replace(
        "ConformanceSuite_%s" % name, "ConformanceProgram_%s" % name))
    entry_object = str(directory / "entry.o")
    ok, output = run_logged(env["CXX"] + ["-std=c++20", "-c", str(entry), "-o", entry_object], log)
    if not ok:
        return False, "entry compile failed: " + output[-300:], []
    archives, modules = [], []
    for use in _list(generator.get("use")) + spec.get("extra_uses", []):
        output_path = profile["program_uses"].get(use)
        if output_path is None:
            continue  # a host-only uselib (DL, LOG): the iOS system provides it
        (modules if output_path.endswith(".o") else archives).append(str(ROOT / output_path))
    module = str(BUILD_DIR / "modules" / ("program_%s.o" % name))
    ok, output = run_logged(env["LD64"] + ["-r", "-arch", env["LD64_ARCH"], *objects,
                                           entry_object, *archives, "-exported_symbol",
                                           "_ConformanceProgram_%s" % name, "-o", module], log)
    if not ok:
        return False, "partial link failed: " + output.strip()[-300:], []
    return True, module, modules


def _list(value):
    if value is None:
        return []
    if isinstance(value, str):
        return value.split()
    return list(value)


HOST_TEMPLATE = """// Generated by tools/quality/ios_conformance.py: the iOS conformance host.
// SDL3 owns main (it starts UIKit); argv is [program, suite id, seed, attempt].
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

%(declarations)s

namespace
{
struct Entry
{
	const char *id;
	int ( *run )( int, char ** );
};
const Entry kSuites[] = {
%(entries)s
};
// Benchmark programs: argv after the name is theirs; they run from Documents,
// where the harness puts their inputs.
const Entry kPrograms[] = {
%(programs)s
};
} // namespace

int main( int argc, char **argv )
{
	setvbuf( stdout, NULL, _IOLBF, 0 );
	if ( argc < 2 )
	{
		for ( const Entry &entry : kSuites )
			printf( "%%s\\n", entry.id );
		exit( 0 );
	}
	const char kProgramPrefix[] = "program:";
	if ( strncmp( argv[1], kProgramPrefix, sizeof( kProgramPrefix ) - 1 ) == 0 )
	{
		const char *name = argv[1] + sizeof( kProgramPrefix ) - 1;
		const char *documents = SDL_GetUserFolder( SDL_FOLDER_DOCUMENTS );
		if ( !documents || chdir( documents ) != 0 )
		{
			printf( "conformance-host: cannot enter Documents\\n" );
			exit( 2 );
		}
		for ( const Entry &entry : kPrograms )
		{
			if ( entry.id[0] == '\\0' || strcmp( entry.id, name ) != 0 )
				continue;
			printf( "conformance-host: running %%s\\n", argv[1] );
			argv[1] = argv[0];
			const int status = entry.run( argc - 1, argv + 1 );
			fflush( stdout );
			fflush( stderr );
			exit( status );
		}
		printf( "conformance-host: unknown program %%s\\n", name );
		exit( 2 );
	}
	// Suites read repository files by relative path (bundled under root/).
	char root[PATH_MAX];
	snprintf( root, sizeof( root ), "%%sroot", SDL_GetBasePath() );
	if ( chdir( root ) != 0 )
	{
		printf( "conformance-host: cannot enter %%s\\n", root );
		exit( 2 );
	}
	setenv( "CONFORMANCE_SUITE", argv[1], 1 );
	setenv( "CONFORMANCE_SEED", argc > 2 ? argv[2] : "%(seed)s", 1 );
	setenv( "CONFORMANCE_ATTEMPT", argc > 3 ? argv[3] : "0", 1 );
	for ( const Entry &entry : kSuites )
	{
		if ( strcmp( entry.id, argv[1] ) != 0 )
			continue;
		printf( "conformance-host: running %%s\\n", entry.id );
		char *suiteArgv[] = { argv[0], NULL };
		const int status = entry.run( 1, suiteArgv );
		fflush( stdout );
		fflush( stderr );
		exit( status );
	}
	printf( "conformance-host: unknown suite %%s\\n", argv[1] );
	exit( 2 );
}
"""


def link_app(env, profile, built, suites, link_flags, programs=(), product_modules=()):
    """The host app: the dispatcher, every module object, SDL3 and MoltenVK.
    `programs` are (name, module) benchmark hosts; `product_modules` the
    product's module objects they link (tier0, vstdlib, the providers)."""
    app = BUILD_DIR / profile["app_bundle"]
    shutil.rmtree(app, ignore_errors=True)
    app.mkdir(parents=True)
    host = BUILD_DIR / "conformance_host.cpp"
    host.write_text(HOST_TEMPLATE % {
        "declarations": "\n".join(['extern "C" int ConformanceSuite_%d( int, char ** );' % i
                                   for i, _ in built] +
                                  ['extern "C" int ConformanceProgram_%s( int, char ** );' % n
                                   for n, _ in programs]),
        "entries": "\n".join('\t{ "%s", ConformanceSuite_%d },' % (s["id"], i)
                             for i, s in built),
        "programs": "\n".join(['\t{ "%s", ConformanceProgram_%s },' % (n, n) for n, _ in programs]
                              + ['\t{ "", NULL },']),
        "seed": conformance.DEFAULT_SEED})
    log = BUILD_DIR / "link.log"
    log.write_text("")
    host_object = str(BUILD_DIR / "conformance_host.o")
    ok, output = run_logged(env["CXX"] + ["-std=c++20", "-I", str(DEPS_PREFIX / "include"), "-c",
                                          str(host), "-o", host_object], log)
    if not ok:
        raise SystemExit("host compile failed:\n" + output[-2000:])
    frameworks = []
    for flag in env.get("LINKFLAGS_SDL3", []):
        frameworks.append(flag)
    for name in env.get("FRAMEWORK_VULKAN", []) + profile.get("frameworks", []):
        frameworks.append("-Wl,-framework," + name)
    # tier1 (in the benchmark programs) converts text with iconv (wscript's ICONV).
    libs = ["-L" + str(DEPS_PREFIX / "lib"), "-lSDL3", "-lMoltenVK", "-lc++", "-lm", "-liconv"]
    extra = [f for f in sorted(link_flags) if f not in ("-lvulkan", "-lrt")]
    command = env["LINK_CXX"] + env.get("LINKFLAGS", []) + [
        host_object, *[str(BUILD_DIR / "modules" / ("suite_%03d.o" % i)) for i, _ in built],
        *[module for _, module in programs], *product_modules,
        *libs, *extra, *frameworks, "-o", str(app / profile["executable"])]
    ok, output = run_logged(command, log)
    if not ok:
        raise SystemExit("app link failed (%s):\n%s" % (log, output[-3000:]))
    product = sepipe_loader.resolve_profile(ROOT / profile["product_profile"])
    os_name = product["target"]["os"]
    os_keys = product[os_name]
    platform = {"ios": "iPhoneOS", "tvos": "AppleTVOS"}[os_name]
    sdk = Path(env["CXX"][env["CXX"].index("-isysroot") + 1])
    sdk_settings = json.loads((sdk / "SDKSettings.json").read_text())
    plist = {
        "CFBundleDevelopmentRegion": "en",
        "CFBundleDisplayName": profile.get("display_name", "Conformance"),
        "CFBundleExecutable": profile["executable"],
        "CFBundleIdentifier": profile["bundle_id"],
        "CFBundleInfoDictionaryVersion": "6.0",
        "CFBundleName": profile.get("display_name", "Conformance"),
        "CFBundlePackageType": "APPL",
        "CFBundleShortVersionString": "1.0",
        "CFBundleVersion": "1",
        "CFBundleSupportedPlatforms": [platform],
        "DTPlatformName": platform.lower(),
        "DTPlatformVersion": sdk_settings.get("Version"),
        "DTSDKName": platform.lower() + sdk_settings.get("Version", ""),
        "LSRequiresIPhoneOS": True,
        "MinimumOSVersion": product["target"]["deployment_target"],
        "UIDeviceFamily": os_keys["device_family"],
        "UIRequiredDeviceCapabilities": os_keys["required_device_capabilities"],
    }
    if os_name == "ios":
        plist.update({
            "UILaunchScreen": {},
            "UIRequiresFullScreen": True,
            "UIStatusBarHidden": True,
            "CADisableMinimumFrameDurationOnPhone": True,
        })
    with open(app / "Info.plist", "wb") as stream:
        plistlib.dump(plist, stream)
    (app / "PkgInfo").write_text("APPL????")
    return app


def cmd_build(args, manifest, profile, suites):
    env = waf_env()
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    (BUILD_DIR / "modules").mkdir(exist_ok=True)
    wrapper = write_wrapper(env, profile, BUILD_DIR)
    profiles_dir = str(ROOT / manifest.get("profiles_dir", "quality/profiles"))
    profiles = {pid: conformance.load_profile(profiles_dir, pid) for pid in profile["hosts"]}
    launcher = [args.launcher] if args.launcher else []
    # Stable indices: a suite's position among all iOS-runnable suites.
    everything = select(manifest, profile, argparse.Namespace(suite=None, domain=None, rfc=None))
    index_of = {s["id"]: i for i, s in enumerate(everything)}
    status = {}
    todo = []
    for suite in suites:
        reason = unsupported_reason(profile, suite)
        if reason:
            status[suite["id"]] = ("unsupported", reason)
        else:
            todo.append(suite)
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = {pool.submit(build_suite, env, profile, profiles, s, index_of[s["id"]],
                               wrapper, launcher, args.config): s for s in todo}
        for future in concurrent.futures.as_completed(futures):
            suite = futures[future]
            state, detail = future.result()
            status[suite["id"]] = (state, detail)
            ok = state in ("built", "compile-error")
            print("  [%s] %s%s" % ("ok  " if ok else "FAIL", suite["id"],
                                   "" if ok else ": " + detail), flush=True)
    built = [(index_of[s["id"]], s) for s in todo if status[s["id"]][0] == "built"]
    link_flags = {f for _, s in built for f in s.get("link_flags", [])}
    programs, product_modules = [], []
    # An app (--app) hosts its own suites only, not the benchmark programs.
    for name, spec in sorted({} if profile.get("app_suites") else profile.get("programs", {}).items()):
        if name.startswith("_"):
            continue
        ok, detail, modules = build_program(env, profile, name, spec, launcher)
        print("  [%s] program %s%s" % ("ok  " if ok else "FAIL", name, "" if ok else ": " + detail),
              flush=True)
        status["program:" + name] = ("built" if ok else "build-failed", detail)
        if ok:
            programs.append((name, detail))
            product_modules += [m for m in modules if m not in product_modules]
    app = link_app(env, profile, built, suites, link_flags, programs, product_modules)
    inputs = repository_inputs([s for _, s in built])
    copied, skipped = bundle_inputs(app, inputs)
    manifest_out = {"built": [s["id"] for _, s in built],
                    "status": {k: list(v) for k, v in status.items()},
                    "bundled_inputs": copied, "inputs_too_large": skipped}
    (BUILD_DIR / "build.json").write_text(json.dumps(manifest_out, indent=2) + "\n")
    print("ios conformance build: %d built, %d expected compile errors, %d failed to build, "
          "%d unsupported; app %s" % (
        len(built), sum(1 for v in status.values() if v[0] == "compile-error"),
        sum(1 for v in status.values() if v[0] == "build-failed"),
        sum(1 for v in status.values() if v[0] == "unsupported"), app))
    if skipped:
        print("  inputs over %d MB not bundled: %s" % (MAX_BUNDLED_BYTES >> 20, ", ".join(skipped)))
    return app


# ---------------------------------------------------------------------------
# deploy and run
# ---------------------------------------------------------------------------

def deploy(profile, app):
    env = dict(os.environ, BUNDLE_ID=profile["bundle_id"])
    command = [str(ROOT / "ios-deploy.sh"), "--profile", str(ROOT / profile["product_profile"]),
               str(app), "--no-launch"]
    if profile.get("deploy_device"):
        command += ["--device", profile["deploy_device"]]
    proc = subprocess.run(command, env=env,
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if proc.returncode != 0:
        raise SystemExit("ios-deploy.sh failed:\n" + proc.stdout[-3000:])


def run_suite(device, profile, suite, seed, repeat, log_dir, environment=None, extra_timeout=0):
    result = conformance.new_result(suite)
    result["profile"] = profile["id"]
    result["host_profile"] = suite["profile"]
    result["build_ok"] = True
    expect = suite.get("expect", conformance.OUTCOME_PASS)
    timeout = suite.get("timeout_seconds", 60) + profile.get("launch_overhead_seconds", 60)
    timeout += extra_timeout
    for attempt in range(repeat):
        start = datetime.datetime.now()
        marker = "conformance-host: running " + suite["id"]
        # A launch whose console captured nothing from the app (not even the
        # host's start line) is a lost capture, not a suite result: launch
        # again, and record it. A run with output is never repeated.
        for _ in range(CAPTURE_ATTEMPTS):
            run = device.launch(profile["executable"], [suite["id"], str(seed), str(attempt)],
                                timeout=timeout, environment=environment)
            if run["timed_out"] or marker in run["console"]:
                break
            result.setdefault("capture_losses", []).append(conformance.tail(run["console"], 3))
        duration = round((datetime.datetime.now() - start).total_seconds(), 3)
        console = run["console"]
        if run["timed_out"]:
            observed = {"outcome": conformance.OUTCOME_TIMEOUT, "exit_code": None, "signal": None,
                        "checks": None, "failed_checks": None,
                        "first_divergence": "timed out after %ss" % timeout}
        elif "conformance-host: running " + suite["id"] not in console:
            observed = {"outcome": conformance.OUTCOME_CRASH, "exit_code": run["exit_code"],
                        "signal": run["signal"], "checks": None, "failed_checks": None,
                        "first_divergence": "the host did not start the suite"}
        else:
            returncode = run["exit_code"]
            if returncode is None:
                returncode = -(run["signal"] or 9)
            observed = conformance.classify_run(suite, returncode, console)
        observed["attempt"] = attempt
        observed["duration_s"] = duration
        observed["log"] = conformance.write_log(str(log_dir / suite["id"]),
                                                "run.%d.log" % attempt,
                                                [("device", device.identifier),
                                                 ("seed", str(seed)), ("console", console)])
        observed["detail"] = conformance.tail(console)
        observed["matched"] = observed["outcome"] == expect
        if suite.get("expected_divergence"):
            observed["matched"] = observed["matched"] and \
                suite["expected_divergence"] in (observed["first_divergence"] or "")
        if "expected_signal" in suite:
            observed["matched"] = observed["matched"] and \
                observed["signal"] == suite["expected_signal"]
        result["attempts"].append(observed)
    decisive = next((a for a in result["attempts"] if not a["matched"]), result["attempts"][-1])
    for key in ("exit_code", "signal", "checks", "failed_checks", "first_divergence", "detail"):
        result[key] = decisive[key]
    result["duration_s"] = round(sum(a["duration_s"] for a in result["attempts"]), 3)
    result["outcome"] = decisive["outcome"]
    result["matched"] = all(a["matched"] for a in result["attempts"])
    result["certified"] = result["matched"]
    return result


def not_run(profile, suite, outcome, reason):
    """A suite the device did not run: a skip the profile declares (matched,
    never certified, like an optional suite's skip) or a build failure."""
    result = conformance.new_result(suite)
    result["profile"] = profile["id"]
    result["host_profile"] = suite["profile"]
    result["outcome"] = outcome
    result["detail"] = reason
    if outcome == conformance.OUTCOME_SKIPPED:
        result["skip_reason"] = reason
        result["matched"] = True
    else:
        result["build_ok"] = False
        result["first_divergence"] = reason
    result["certified"] = False
    return result


def cmd_check(args, manifest, profile, suites):
    if not args.no_build:
        app = cmd_build(args, manifest, profile, suites)
    else:
        app = BUILD_DIR / profile["app_bundle"]
    status = json.loads((BUILD_DIR / "build.json").read_text())["status"]
    if not args.no_deploy:
        deploy(profile, app)
    device = ios_device.Device(profile["bundle_id"], host=args.host,
                               platform={"ios": "iOS", "tvos": "tvOS"}[target_os(profile)])
    stamp = datetime.datetime.now().strftime("%Y%m%dT%H%M%S")
    out = Path(args.out) if args.out else ROOT / ("quality-results/ios-conformance.%s.json" % stamp)
    log_dir = out.with_suffix(".logs")
    results = []
    for suite in suites:
        state, detail = status.get(suite["id"], ("not-built", "not in the last build"))
        if state == "unsupported":
            result = not_run(profile, suite, conformance.OUTCOME_SKIPPED,
                             "unsupported on %s: %s" % (profile["id"], detail))
        elif state == "compile-error":
            # A negative compile test: the row's expected diagnostic must be
            # the reason, as conformance.py judges it.
            result = not_run(profile, suite, conformance.OUTCOME_COMPILE_ERROR,
                             conformance.tail(detail))
            result["build_ok"] = False
            result["matched"] = suite.get("expect") == conformance.OUTCOME_COMPILE_ERROR and (
                not suite.get("expected_diagnostic") or suite["expected_diagnostic"] in detail)
            result["certified"] = result["matched"]
        elif state != "built":
            result = not_run(profile, suite, conformance.OUTCOME_COMPILE_ERROR, detail)
        else:
            environment = dict(item.split("=", 1) for item in args.env) if args.env else None
            result = run_suite(device, profile, suite, args.seed, args.repeat, log_dir,
                               environment, args.extra_timeout)
        results.append(result)
        print("  [%s] %-50s expect=%-6s got=%-12s checks=%s (%ss)" % (
            "ok  " if result["matched"] else ("skip" if result["outcome"] ==
                                             conformance.OUTCOME_SKIPPED else "FAIL"),
            suite["id"], result["expect"], result["outcome"], result["checks"],
            result["duration_s"]), flush=True)
    expected_ids = [s["id"] for s in suites]
    decision = conformance.decide(expected_ids, results)
    identity = {"source": conformance.source_identity(str(ROOT)),
                "device": device.describe(),
                "toolchain": waf_env()["CXX"],
                "app": str(app)}
    run = {"profile": profile["id"], "seed": args.seed, "repeat": args.repeat,
           "config": args.config, "runner": profile["runner"]}
    evidence = conformance.build_evidence(
        str(ROOT), str(ROOT / "quality/conformance.manifest.json"), manifest, identity,
        {profile["id"]: dict(profile, cxx_std="c++20")}, run, results, expected_ids, decision,
        str(log_dir))
    conformance.write_evidence(str(out), evidence)
    counts = conformance.count(results)
    print("\n%d suite(s): %d matched, %d mismatched, %d skipped -> %s" % (
        counts["total"], counts["matched"], counts["mismatched"], counts["skipped"],
        decision.upper()))
    print("evidence: %s" % out)
    return 0 if decision == "pass" else 1


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command", required=True)
    for name in ("build", "check"):
        p = sub.add_parser(name)
        p.add_argument("--device-profile", default=DEFAULT_DEVICE_PROFILE,
                       help="quality/profiles/<id>.json: ios-arm64-device (default) or "
                            "tvos-arm64-device")
        p.add_argument("--suite", action="append")
        p.add_argument("--app", help="build one of the device profile's apps (its `apps`) "
                                     "instead of the conformance host")
        p.add_argument("--domain", action="append")
        p.add_argument("--rfc", action="append")
        p.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
        p.add_argument("--launcher", default=shutil.which("ccache"),
                       help="compiler launcher (default: ccache when installed)")
        p.add_argument("--config", choices=sorted(conformance.BUILD_CONFIGS), default="default")
        if name == "check":
            p.add_argument("--no-build", action="store_true")
            p.add_argument("--no-deploy", action="store_true")
            p.add_argument("--seed", type=int, default=conformance.DEFAULT_SEED)
            p.add_argument("--repeat", type=int, default=1)
            p.add_argument("--out")
            p.add_argument("--host", default=ios_device.DEFAULT_HOST)
            p.add_argument("--env", action="append", metavar="KEY=VALUE",
                           help="an environment variable for every launch")
            p.add_argument("--extra-timeout", type=float, default=0,
                           help="seconds added to every suite's timeout (a suite kept on "
                                "screen by --env)")
    args = parser.parse_args(argv)
    manifest = conformance.load_manifest(str(ROOT / "quality/conformance.manifest.json"),
                                         root=str(ROOT))
    profile = load_profile(args.device_profile)
    use_profile(profile)
    if args.app:
        profile = use_app(profile, args.app)
    suites = select(manifest, profile, args)
    if not suites:
        raise SystemExit("no suites selected")
    if args.command == "build":
        cmd_build(args, manifest, profile, suites)
        return 0
    return cmd_check(args, manifest, profile, suites)


if __name__ == "__main__":
    sys.exit(main())
