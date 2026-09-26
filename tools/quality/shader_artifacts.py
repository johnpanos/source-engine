#!/usr/bin/env python3
"""Build a bounded, source-matched DX9 shader pack for the Vulkan compatibility profile.

The existing fxc_prep.pl owns permutation declarations, platform filtering, SKIP
expressions, and compiler options. This runner consumes its plan, verifies the
checked-in C++ selector schema, and packs version-6 VCS records as specified by
utils/shadercompile/shadercompile.cpp. It never changes installed game content.
"""

import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import re
import shlex
import struct
import subprocess
import sys
import tempfile
import time
import zlib

import conformance
import product_profile


ROOT = Path(__file__).resolve().parents[2]
MAX_BLOCK = 128 * 1024
NAME = re.compile(r"[a-zA-Z0-9_]+\Z")


def digest(data):
    return hashlib.sha256(data).hexdigest()


def load_compiler_profile(path=None):
    if path is None:
        path = ROOT / product_profile.load_profile()["intent"]["shader_pipeline"]["compiler_profile"]
    path = Path(path).resolve()
    profile = json.loads(path.read_text())
    if profile["schema"] != "source-shader-compiler/v1":
        raise ValueError("unsupported shader compiler profile")
    if profile["compiler"]["host_command"] != "wine":
        raise ValueError("this artifact runner requires the Wine FXC host")
    if not re.fullmatch(r"[0-9a-f]{64}", profile["compiler"]["sha256"]):
        raise ValueError("shader compiler profile requires an exact SHA-256 pin")
    for relative in (profile["compiler"]["path"], profile["plan_generator"],
                     profile["source_directory"], profile["selector_directory"]):
        if not (ROOT / relative).resolve().is_relative_to(ROOT):
            raise ValueError("shader compiler profile path escapes repository")
    if digest((ROOT / profile["compiler"]["path"]).read_bytes()) != profile["compiler"]["sha256"]:
        raise ValueError("bundled FXC compiler differs from pinned shader artifact compiler")
    return path, profile


def demands_from_log(text):
    """Use requested static bases, never the potentially incompatible VCS count."""
    found = {}
    for match in re.finditer(
        r'(?:vsh|psh) 0x[0-9a-fA-F ]+: static combo:\s*(\d+) '
        r'dynamic combos:\s*\d+ refcount:\s*\d+ "([\w]+)"', text
    ):
        found.setdefault(match[2], set()).add(int(match[1]))
    for match in re.finditer(
        r"Shader 'shaders[\\/]fxc[\\/]([\w]+)\.vcs' - Couldn't load combo (\d+)", text
    ):
        found.setdefault(match[1], set()).add(int(match[2]))
    return found


def parse_workload(document):
    if not isinstance(document, dict) or document.get("schema") != "source-shader-workload/v1":
        raise ValueError("unsupported shader workload schema")
    if not isinstance(document.get("id"), str) or not document["id"]:
        raise ValueError("shader workload requires an id")
    if document.get("coverage") != "observed-static-sets":
        raise ValueError("shader workload must declare observed-only coverage")
    rows = document.get("shaders")
    if not isinstance(rows, list) or not rows:
        raise ValueError("shader workload contains no shader requests")
    observed = document.get("observations")
    if not isinstance(observed, list) or not observed:
        raise ValueError("shader workload requires captured-input provenance")
    for observation in observed:
        if (not isinstance(observation, dict) or not isinstance(observation.get("map"), str)
                or not observation["map"] or not isinstance(observation.get("console_sha256"), str)
                or not re.fullmatch(r"[0-9a-f]{64}", observation["console_sha256"])):
            raise ValueError("invalid shader workload captured-input provenance")
    demands, selectors, names = {}, {}, set()
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError("shader workload request must be an object")
        name = row.get("name")
        if not isinstance(name, str) or not NAME.fullmatch(name):
            raise ValueError("invalid shader workload shader name")
        if name.lower() in names:
            raise ValueError("duplicate shader workload shader name")
        names.add(name.lower())
        indices = row.get("static_bases")
        if (not isinstance(indices, list) or not indices
                or any(type(index) is not int or index < 0 or index > 0x7FFFFFFF for index in indices)):
            raise ValueError("shader workload requires nonnegative integer static bases")
        if len(set(indices)) != len(indices):
            raise ValueError("duplicate shader workload static base")
        selector = row.get("selector_sha256")
        if not isinstance(selector, str) or not re.fullmatch(r"[0-9a-f]{64}", selector):
            raise ValueError("shader workload requires exact selector schema hash")
        demands[name] = set(indices)
        selectors[name] = selector
    return demands, selectors


def load_workload(path):
    def unique_fields(pairs):
        result = {}
        for name, value in pairs:
            if name in result:
                raise ValueError("duplicate shader workload JSON field: " + name)
            result[name] = value
        return result
    document = json.loads(Path(path).read_text(), object_pairs_hook=unique_fields)
    demands, selectors = parse_workload(document)
    return document, demands, selectors


def resolve_source(source_dir, name):
    if not NAME.fullmatch(name):
        raise ValueError(f"invalid shader name: {name}")
    files = {p.name.lower(): p for p in source_dir.glob("*.fxc")}
    direct = files.get(name.lower() + ".fxc")
    if direct:
        return direct
    # These aliases are declared by valve_perl_helpers.pl (ReadShaderList).
    alias = re.sub(r"_ps(?:20b|20|30)$", "_ps2x", name, flags=re.I)
    alias = re.sub(r"_vs(?:11|20|30)$", "_vsxx", alias, flags=re.I)
    if alias.lower() + ".fxc" not in files:
        raise ValueError(f"no repository FXC source for demanded shader {name}")
    return files[alias.lower() + ".fxc"]


def parse_plan(text):
    def definitions(begin, end):
        rows = text.split(begin + "\n", 1)[1].split(end, 1)[0].strip().splitlines()
        result = []
        for row in rows:
            match = re.fullmatch(r"(\w+)=(\d+)\.\.(\d+)", row)
            if not match or int(match[2]) > int(match[3]):
                raise ValueError(f"invalid permutation definition: {row}")
            result.append((match[1], int(match[2]), int(match[3])))
        return result

    plan = {
        "dynamic": definitions("#DEFINES-D:", "#DEFINES-S:"),
        "static": definitions("#DEFINES-S:", "#SKIP:"),
        "skip": text.split("#SKIP:\n", 1)[1].split("#COMMAND:", 1)[0].strip(),
    }
    command = text.split("#COMMAND:\n", 1)[1].split("#END", 1)[0]
    command = command.split(">output.txt", 1)[0]
    tokens = shlex.split(command)
    if tokens[0] != "fxc.exe" or "/Foshader.o" not in tokens:
        raise ValueError("unsupported compiler command in fxc_prep plan")
    plan["arguments"] = [t for t in tokens[1:-1] if t != "/Foshader.o"]
    for key, macro in (("total", "TOTALSHADERCOMBOS"), ("dynamic_count", "NUMDYNAMICCOMBOS"),
                       ("centroid", "CENTROIDMASK"), ("flags", "FLAGS")):
        plan[key] = int(re.search(r"/D" + macro + r"=(\w+)", command)[1], 0)
    if plan["dynamic_count"] != count(plan["dynamic"]):
        raise ValueError("compiler plan dynamic count disagrees with its definitions")
    if plan["total"] != count(plan["static"]) * plan["dynamic_count"]:
        raise ValueError("compiler plan total count disagrees with its definitions")
    return plan


def count(definitions):
    return math.prod(high - low + 1 for _, low, high in definitions)


def decode_index(index, definitions):
    if index < 0 or index >= count(definitions):
        raise ValueError(f"permutation index {index} outside declared schema")
    values = {}
    for name, low, high in definitions:
        values[name] = index % (high - low + 1) + low
        index //= high - low + 1
    return values


def verify_selector(header, name, plan):
    """Reject source/header drift before compiling any shader bytecode."""
    for kind, definitions, scale in (("Static", plan["static"], plan["dynamic_count"]),
                                      ("Dynamic", plan["dynamic"], 1)):
        marker = re.search(r"class " + re.escape(name) + "_" + kind + "_Index", header, flags=re.I)
        if not marker:
            raise ValueError(f"missing {kind} selector for {name}")
        body = header[marker.end():].split("\n};", 1)[0]
        expected = []
        for define, low, high in definitions:
            setter = re.search(r"void Set" + define + r"\( int i \).*?Assert\( i >= (\d+) && i <= (\d+) \)",
                               body, flags=re.S)
            if not setter or (int(setter[1]), int(setter[2])) != (low, high):
                raise ValueError(f"{name} {define} range differs from checked-in selector")
            expected.append((scale, define))
            scale *= high - low + 1
        getter = body.split("int GetIndex()", 1)[1]
        actual = [(int(n), define) for n, define in re.findall(r"\( (\d+) \* m_n(\w+) \)", getter)]
        if actual != expected:
            raise ValueError(f"{name} {kind} permutation ordering differs from checked-in selector")


def flatten_source(source, source_dir, sources, active=()):
    """Match copyshaders.pl's include-expanded CRC; record every input hash too."""
    source = source.resolve()
    if source in active or not source.is_relative_to(source_dir.resolve()):
        raise ValueError(f"invalid recursive/outside shader include: {source}")
    data = source.read_bytes()
    sources[str(source.relative_to(ROOT))] = digest(data)
    output = []
    for line in data.splitlines(keepends=True):
        match = re.search(rb'#include\s+"([^"]+)"', line, flags=re.I)
        if match:
            included = source_dir / match[1].decode().replace("\\", "/")
            output.append(flatten_source(included, source_dir, sources, (*active, source)))
        else:
            output.append(line)
    return b"".join(output)


def valid_combos(plan, static_bases):
    combinations = []
    dynamic_count = plan["dynamic_count"]
    for base in sorted(set(static_bases)):
        if base % dynamic_count:
            raise ValueError(f"static base {base} is not aligned to source dynamic count {dynamic_count}")
        static_id = base // dynamic_count
        static = decode_index(static_id, plan["static"])
        for dynamic_id in range(dynamic_count):
            combinations.append((static_id, dynamic_id,
                                 static | decode_index(dynamic_id, plan["dynamic"])))
    if not combinations:
        raise ValueError("no requested shader permutations")
    # Evaluate the authoritative Perl SKIP expression with Perl itself. Undefined
    # variables deliberately keep Perl's defined/false behavior from fxc_prep.
    names = list(combinations[0][2])
    program = ("while (<STDIN>) { chomp; (" + ",".join("$" + n for n in names)
               + ") = split(/ /); print ((" + plan["skip"] + ") ? \"1\\n\" : \"0\\n\"); }")
    rows = "".join(" ".join(str(values[n]) for n in names) + "\n" for _, _, values in combinations)
    result = subprocess.run(["perl", "-e", program], input=rows, capture_output=True,
                            text=True, check=True, timeout=60)
    skipped = result.stdout.splitlines()
    if len(skipped) != len(combinations) or any(s not in ("0", "1") for s in skipped):
        raise ValueError("incomplete SKIP evaluation")
    valid = [combo for combo, skip in zip(combinations, skipped) if skip == "0"]
    if {s for s, _, _ in valid} != {b // dynamic_count for b in static_bases}:
        raise ValueError("a demanded static permutation has no valid dynamic combinations")
    return valid


def validate_bytecode(data):
    if len(data) < 8 or len(data) % 4:
        raise ValueError("missing/truncated shader bytecode")
    version, = struct.unpack_from("<I", data)
    if version >> 16 not in (0xFFFF, 0xFFFE) or data[-4:] != b"\xff\xff\x00\x00":
        raise ValueError("invalid Direct3D shader bytecode header/terminator")


def pack_vcs(plan, shaders, crc):
    if not shaders:
        raise ValueError("cannot produce empty shader pack")
    static_ids = sorted(shaders)
    header_size = 28 + 8 * (len(static_ids) + 1) + 4
    payload = bytearray()
    dictionary = []
    for static_id in static_ids:
        if not 0 <= static_id < count(plan["static"]) or not shaders[static_id]:
            raise ValueError("invalid/empty static permutation")
        dictionary.append((static_id, header_size + len(payload)))
        block = bytearray()
        for dynamic_id, bytecode in sorted(shaders[static_id].items()):
            if not 0 <= dynamic_id < plan["dynamic_count"]:
                raise ValueError("dynamic permutation outside schema")
            validate_bytecode(bytecode)
            if len(bytecode) + 16 >= MAX_BLOCK:
                raise ValueError("individual shader exceeds VCS unpacked block limit")
            if len(block) + len(bytecode) + 16 >= MAX_BLOCK:
                payload += struct.pack("<I", 0x80000000 | len(block)) + block
                block = bytearray()
            block += struct.pack("<II", dynamic_id, len(bytecode)) + bytecode
        payload += struct.pack("<I", 0x80000000 | len(block)) + block
        payload += struct.pack("<I", 0xFFFFFFFF)
    dictionary.append((0xFFFFFFFF, header_size + len(payload)))
    header = struct.pack("<7I", 6, plan["total"] & 0xFFFFFFFF, plan["dynamic_count"],
                         plan["flags"], plan["centroid"], len(dictionary), crc)
    return header + b"".join(struct.pack("<II", *r) for r in dictionary) + struct.pack("<I", 0) + payload


def compile_shader(compiler, source, arguments, values, output, timeout):
    if output.is_file():
        bytecode = output.read_bytes()
        validate_bytecode(bytecode)
        return bytecode
    output.parent.mkdir(parents=True, exist_ok=True)
    temporary = output.with_suffix(".tmp")
    command = ["wine", str(compiler), *arguments]
    command += [f"/D{name}={value}" for name, value in values.items()]
    command += ["/Fo" + "Z:" + str(temporary).replace("/", "\\"), source.name]
    result = subprocess.run(command, cwd=source.parent, env=dict(os.environ, WINEDEBUG="-all"),
                            capture_output=True, text=True, timeout=timeout)
    output.with_suffix(".log").write_text(result.stdout + result.stderr)
    if result.returncode or not temporary.is_file():
        raise RuntimeError(f"FXC failed for {source.name}: {output.with_suffix('.log')}")
    bytecode = temporary.read_bytes()
    validate_bytecode(bytecode)
    temporary.replace(output)
    return bytecode


def build_one(name, bases, source_dir, output, compiler, jobs, timeout, profile_path, profile,
              cache_root=None, expected_selector=None):
    source = resolve_source(source_dir, name)
    with tempfile.TemporaryDirectory(prefix="source-shader-plan-") as temp:
        work = Path(temp)
        for path in source_dir.iterdir():
            if path.is_file():
                (work / path.name).symlink_to(path)
        subprocess.run(["perl", str(ROOT / profile["plan_generator"]), "-nocpp",
                                 source.name + "-----" + name], cwd=work, capture_output=True,
                                text=True, timeout=30, check=True)
        plan_text = (work / "filelistgen.txt").read_text()
    plan = parse_plan(plan_text)
    selectors = {p.name.lower(): p for p in (ROOT / profile["selector_directory"]).glob("*.inc")}
    selector = selectors[name.lower() + ".inc"]
    if expected_selector is not None and digest(selector.read_bytes()) != expected_selector:
        raise ValueError(f"{name} workload selector schema changed; recapture and review the workload")
    verify_selector(selector.read_text(), name, plan)
    sources = {}
    expanded = flatten_source(source, source_dir, sources)
    for path in (selector, profile_path, ROOT / profile["plan_generator"],
                 ROOT / "devtools/bin/valve_perl_helpers.pl"):
        sources[str(path.relative_to(ROOT))] = digest(path.read_bytes())
    identity = digest(json.dumps({"compiler": profile["compiler"]["sha256"], "plan": plan,
                                  "sources": sources}, sort_keys=True).encode())
    combos = valid_combos(plan, bases)
    cache = (cache_root or output / "bytecode") / identity / name
    shaders = {}
    print(f"{name}: {len(set(bases))} static sets, {len(combos)} valid dynamic permutations "
          f"(schema {plan['dynamic_count']})", flush=True)
    with ThreadPoolExecutor(max_workers=jobs) as pool:
        futures = {pool.submit(compile_shader, compiler, source, plan["arguments"], values,
                               cache / f"{static_id}-{dynamic_id}.o", timeout): (static_id, dynamic_id)
                   for static_id, dynamic_id, values in combos}
        for future in as_completed(futures):
            static_id, dynamic_id = futures[future]
            shaders.setdefault(static_id, {})[dynamic_id] = future.result()
    for path, expected in sources.items():
        if digest((ROOT / path).read_bytes()) != expected:
            raise ValueError(f"shader input changed during compilation: {path}")
    vcs = pack_vcs(plan, shaders, zlib.crc32(expanded))
    target = output / "shaders/fxc" / (name + ".vcs")
    target.parent.mkdir(parents=True, exist_ok=True)
    temporary = target.with_suffix(".tmp")
    temporary.write_bytes(vcs)
    temporary.replace(target)
    plan_file = output / "plans" / (name + ".txt")
    plan_file.parent.mkdir(exist_ok=True)
    plan_file.write_text(plan_text)
    return {"name": name, "source": str(source.relative_to(ROOT)), "identity": identity,
            "sources": sources, "static_bases": sorted(set(bases)), "dynamic_count": plan["dynamic_count"],
            "compiled_count": len(combos), "crc32": zlib.crc32(expanded),
            "path": str(target.relative_to(output)), "sha256": digest(vcs),
            "dynamic_ids_by_static_id": {str(s): sorted(rows) for s, rows in sorted(shaders.items())}}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--log", type=Path, action="append", default=[], help="engine shader demand dump")
    parser.add_argument("--workload", type=Path, action="append", default=[],
                        help="versioned shader demand fixture; defaults to the product profile workload")
    parser.add_argument("--shader", action="append", default=[], help="name:static_base[,static_base...]")
    parser.add_argument("--out", type=Path, required=True, help="build artifact directory (never game content)")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--compile-timeout", type=int, default=60)
    parser.add_argument("--cache", type=Path, help="optional shared compiler output cache; keys include compiler/plan/source hashes")
    parser.add_argument("--compiler-profile", type=Path, help="defaults to the Portal product profile's shader compiler")
    args = parser.parse_args(argv)
    output = args.out.resolve()
    cache_root = args.cache.resolve() if args.cache else None
    if args.jobs < 1 or args.compile_timeout < 1:
        parser.error("jobs and compile timeout must be positive")
    if (output / "gameinfo.txt").exists() or (output / "portal/gameinfo.txt").exists():
        parser.error("output must be a build artifact directory, not installed content")
    if not args.workload and not args.log and not args.shader:
        args.workload = [ROOT / product_profile.load_profile()["intent"]["shader_pipeline"]["default_workload"]]
    demands, expected_selectors, workloads = {}, {}, []
    for workload in args.workload:
        document, requests, selectors = load_workload(workload)
        for name, bases in requests.items():
            demands.setdefault(name, set()).update(bases)
            if name in expected_selectors and expected_selectors[name] != selectors[name]:
                raise ValueError("workloads disagree on selector schema: " + name)
            expected_selectors[name] = selectors[name]
        workloads.append({"path": str(workload.resolve()), "sha256": digest(workload.read_bytes()),
                          "id": document["id"], "coverage": document["coverage"],
                          "observations": document["observations"]})
    for log in args.log:
        for name, bases in demands_from_log(log.read_text(errors="replace")).items():
            demands.setdefault(name, set()).update(bases)
    for entry in args.shader:
        name, indices = entry.split(":", 1)
        demands.setdefault(name, set()).update(int(i) for i in indices.split(","))
    if not demands:
        parser.error("no demanded shaders; capture mat_spewvertexandpixelshaders first")
    profile_path, profile = load_compiler_profile(args.compiler_profile)
    compiler = ROOT / profile["compiler"]["path"]
    wine_version = subprocess.check_output(["wine", "--version"], text=True, timeout=30).strip()
    if wine_version != profile["compiler"]["host_version"]:
        raise ValueError("Wine host differs from pinned shader compiler profile: " + wine_version)
    output.mkdir(parents=True, exist_ok=True)
    evidence = {"schema": 1, "status": "incomplete", "coverage": "observed-static-sets",
                "compiler": profile["compiler"],
                "compiler_profile": {"path": str(profile_path.relative_to(ROOT)),
                                     "sha256": digest(profile_path.read_bytes())},
                **conformance.source_identity(str(ROOT)), "invocation": sys.argv, "shaders": [],
                "producer_sha256": digest(Path(__file__).read_bytes())}
    evidence["host_tools"] = {"wine": wine_version, "python": platform.python_version(),
                              "perl": subprocess.check_output(["perl", "-e", "print $^V"],
                                                               text=True, timeout=30)}
    evidence["input_workloads"] = workloads
    evidence["input_logs"] = {str(p.resolve()): digest(p.read_bytes()) for p in args.log}
    started = time.monotonic()
    (output / "manifest.json").write_text(json.dumps(evidence, indent=2) + "\n")
    try:
        for name, bases in sorted(demands.items()):
            evidence["shaders"].append(build_one(name, bases, ROOT / profile["source_directory"],
                                                 output, compiler, args.jobs, args.compile_timeout,
                                                 profile_path, profile, cache_root,
                                                 expected_selectors.get(name)))
            (output / "manifest.json").write_text(json.dumps(evidence, indent=2) + "\n")
        if any(digest(Path(row["path"]).read_bytes()) != row["sha256"] for row in workloads):
            raise ValueError("shader workload changed during compilation")
        evidence["status"] = "passed"
    except Exception as error:
        evidence["failure"] = str(error)
        raise
    finally:
        evidence["elapsed_seconds"] = time.monotonic() - started
        (output / "manifest.json").write_text(json.dumps(evidence, indent=2) + "\n")
    print(f"Built {len(evidence['shaders'])} source-matched shaders: {output / 'manifest.json'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
