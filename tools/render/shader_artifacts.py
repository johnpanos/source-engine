#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
"""Per-target shader artifacts (RFC 0016 "Shader artifacts", K4 check
render.shader-artifacts).

    python3 tools/render/shader_artifacts.py build --out DIR [--headers DIR]
    python3 tools/render/shader_artifacts.py check [--out DIR] [--seed-fault FAULT]
    python3 tools/render/shader_artifacts.py sensitivity [--out DIR]
    python3 tools/render/shader_artifacts.py derive-layouts

GLSL is the one source language. Every shader the native backend ships (the
units below) becomes one artifact per target format:

  spirv    the pinned glslc (quality/toolchain/shader-compiler.json), with the
           unit's options, exactly as the backend embeds it today;
  glsl450  the pinned SPIRV-Cross (cross_compiler in the same pin) from that
           SPIR-V, for the OpenGL 4.5 adapter (RFC 0016 K10). Bind groups are
           flattened first: binding (group, b) becomes GL slot
           group * GL_SLOTS_PER_GROUP + b, so no two bindings share a slot.
           The text must compile as OpenGL GLSL with the pinned
           glslangValidator.

Each artifact is keyed by ArtifactKey (public/render/shaderlib/artifact.h):
its source, the compiler identity, the target format and the permutation. A
source's permutation axes are the options that vary between its units (one
bit each, in sorted order, as shaderlib::PermutationKey numbers them).

Reflection (spirv-cross --reflect) of each SPIR-V artifact is checked against
the declared layouts, render/shaders/layouts.json:

  passes    the native backend's pipelines today: every reflected binding
            (descriptor set, binding, kind, count) must be declared with the
            unit's stage, and every declared binding must be used by some
            unit of the pass. Passes predate the four-group port; their set
            counts are reported as the switch-over's debt.
  families  render.material.v2 families on the port: at most four groups, in
            the port's role order (frame, view, material, draw), the
            descriptor set being the role's index. A family declaring a fifth
            group fails ("Bind-group ceiling").

Every unit must belong to exactly one pass or family, so a new shader without
a declared layout fails the build.

`build` writes DIR/<format>/<artifact files> and DIR/index.json (the artifact
index: keys, files, sha256, reflected bindings, GL slots, specialization
constants), and fails (exit 1) on any layout or compile failure; the Waf task
(render/shaders/wscript) runs it, so products build artifacts at build time.
With --headers DIR it also writes every generated header into DIR
(shader_toolchain.GENERATED_NAMES): material_spv.h, material_spv_index.h and
legacy_spv.h from the SPIR-V artifacts with the regenerators' own writers,
the GENERATED headers from their rows. None is committed (RFC 0016 K4):
consumers include "spv/<name>" from the build's generated directory.
`headers --out DIR` writes the same headers without the GLSL 4.50 targets
(the conformance runner's generated include root uses it).

`check` is the gate: it builds every artifact, writes the headers as the
build does, and requires every generated header to be written, the backend's
to agree with the regenerators (independent writers that compile for
themselves, --check --compare-dir) and each unit's array to equal its
artifact. It prints one checks-v1 record.

--seed-fault runs the check against a seeded defect:
  layout-mismatch   one declared binding's kind is changed;
  fifth-group       a fixture family declares five groups;
  flip-word         one word of one array in each backend header the build
                    wrote is changed;
`sensitivity` requires the unseeded check to pass and each fault to fail for
exactly its seeded defect.

`derive-layouts` prints the pass bindings the current reflections imply, for
review when a shader's interface changes on purpose; it never writes the
declaration.
"""

import argparse
import concurrent.futures
import copy
import fnmatch
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "render"))
sys.path.insert(0, str(ROOT / "tools" / "quality"))
import shader_toolchain as st  # noqa: E402
from conformance_result import Checks  # noqa: E402

SHADERS = "materialsystem/shaderapivulkan/shaders"
LEGACY = SHADERS + "/legacy"
BACKEND = "materialsystem/shaderapivulkan"
LAYOUTS = ROOT / "render" / "shaders" / "layouts.json"
LAYOUT_SCHEMA = "render-shader-layouts/v1"
INDEX_SCHEMA = "render-shader-artifacts/v1"
FORMATS = ("spirv", "glsl450")
EXTENSIONS = {"spirv": ".spv", "glsl450": ".glsl"}
STAGES = {".vert": "vertex", ".frag": "fragment", ".comp": "compute"}
ROLES = ("frame", "view", "material", "draw")  # render::device::BindGroupRole
MAX_GROUPS = 4  # render::device::kMaxBindGroups
MAX_DRAW_CONSTANT_BYTES = 128  # render::device::kMaxDrawConstantBytes (D16)
# OpenGL has flat binding slots per resource kind; each group owns this many.
GL_SLOTS_PER_GROUP = 16
# spirv-cross --reflect resource lists and their binding kinds. The port
# (render::device::BindingKind) has no combined image-sampler, no input
# attachment and no acceleration structure: shaders using them are backend
# passes until they move onto the port.
KINDS = {
    "ubos": "uniform-buffer",
    "ssbos": "storage-buffer",
    "textures": "combined-image-sampler",
    "separate_images": "sampled-texture",
    "separate_samplers": "sampler",
    "images": "storage-texture",
    "acceleration_structures": "acceleration-structure",
    "subpass_inputs": "input-attachment",
}
PORT_KINDS = {"uniform-buffer", "storage-buffer", "sampled-texture", "sampler",
              "storage-texture"}
OP_DECORATE = 71
DECORATION_BINDING = 33
DECORATION_DESCRIPTOR_SET = 34


class ArtifactError(Exception):
    pass


def load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def regenerators(root=ROOT):
    """The regenerators own the unit lists and header layouts until the
    switch-over; this tool reads their tables instead of copying them."""
    base = Path(root) / SHADERS
    return (load_module("regen_material_spv", base / "regen_material_spv.py"),
            load_module("regen_legacy_spv", base / "regen_legacy_spv.py"))


# ---------------------------------------------------------------------------
# Units: every shader module the backend ships


class Unit:
    """One SPIR-V module the backend embeds: an array name, its GLSL source
    (repository-relative), glslc options and the header that embeds it."""

    def __init__(self, name, source, base, extra, header):
        self.name = name
        self.source = source
        self.base = list(base)
        self.extra = list(extra)
        self.header = header
        self.stage = STAGES[Path(source).suffix]
        self.axes = []
        self.permutation = 0

    @property
    def options(self):
        return self.base + self.extra


def inventory(root=ROOT):
    material, legacy = regenerators(root)
    units = [Unit(name, SHADERS + "/" + source, ["-O"], extra, "material")
             for name, source, extra in material.SHADERS]
    legacy_dir = Path(root) / LEGACY
    for path in sorted(list(legacy_dir.glob("*.vert")) + list(legacy_dir.glob("*.frag")),
                       key=lambda p: p.name):
        units.append(Unit(legacy.array_name(path.stem, path.suffix[1:]),
                          LEGACY + "/" + path.name, ["-O", "-Werror", "-I", LEGACY], [],
                          "legacy"))
    # The backend's file-static generated arrays (demo_triangle_spv.h) and the
    # material families' programs (families_spv.h, RFC 0016 K4), whose
    # layouts.json family entries the bind-group ceiling judges.
    for header, (namespace, _, rows) in sorted(st.GENERATED.items()):
        if namespace is None or header in st.FAMILY_HEADERS:
            for array, source, options in rows:
                units.append(Unit(array, source, list(options), [], header))
    # Axes: the options that vary between a source's units.
    by_source = {}
    for unit in units:
        by_source.setdefault(unit.source, []).append(unit)
    for group in by_source.values():
        axes = sorted({option for unit in group for option in unit.extra})
        for unit in group:
            unit.axes = axes
            unit.permutation = sum(1 << axes.index(option) for option in unit.extra)
    return units


def compiler_identities(pin):
    glslc = "glslc-" + pin["components"]["shaderc"]["version"]
    cross = "spirv-cross-" + pin["cross_compiler"]["component"]["version"]
    return {"spirv": glslc, "glsl450": glslc + "+" + cross}


def artifact_file(unit, fmt):
    return "%s/%s.%016x%s" % (fmt, unit.source.replace("/", "."), unit.permutation,
                              EXTENSIONS[fmt])


# ---------------------------------------------------------------------------
# Compiling, reflecting, cross-compiling


def run(argv, what, cwd=ROOT, binary=False):
    result = subprocess.run(argv, capture_output=True, cwd=cwd, text=not binary)
    if result.returncode != 0:
        output = result.stdout + result.stderr
        if binary:
            output = output.decode(errors="replace")
        raise ArtifactError("%s failed (%d): %s" % (what, result.returncode,
                                                    output.strip()[-2000:]))
    return result


def words_of(data):
    return [int.from_bytes(data[i:i + 4], "little") for i in range(0, len(data), 4)]


def bytes_of(words):
    return b"".join(word.to_bytes(4, "little") for word in words)


def compile_spirv(unit, root=ROOT):
    with tempfile.TemporaryDirectory() as tmp:
        out = Path(tmp) / "module.spv"
        run([st.glslc(), *unit.options, unit.source, "-o", str(out)],
            "glslc %s %s" % (" ".join(unit.options), unit.source), cwd=root)
        return out.read_bytes()


def reflect(spirv):
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / "module.spv"
        path.write_bytes(spirv)
        result = run([st.spirv_cross(), str(path), "--reflect"], "spirv-cross --reflect")
    data = json.loads(result.stdout)
    bindings = []
    for key, kind in KINDS.items():
        for resource in data.get(key, []):
            count = 1
            array = resource.get("array")
            if array:
                count = 1
                for size in array:
                    count *= size
            bindings.append({"set": resource.get("set", 0), "binding": resource.get("binding", 0),
                             "kind": kind, "count": count})
    bindings.sort(key=lambda b: (b["set"], b["binding"], b["kind"]))
    push = None
    for block in data.get("push_constants", []):
        members = data["types"].get(block["type"], {}).get("members", [])
        push = max((member.get("offset", 0) for member in members), default=0)
    specialization = sorted(item["id"] for item in data.get("specialization_constants", []))
    modes = [entry["mode"] for entry in data.get("entryPoints", [])]
    return {"bindings": bindings, "push_constants": push is not None,
            "specialization_constants": specialization, "modes": modes}


def flatten_bindings(spirv):
    """A copy of the module with every (set, binding) decorated as set 0,
    binding set * GL_SLOTS_PER_GROUP + binding."""
    words = words_of(spirv)
    sets, bindings = {}, {}
    index = 5
    while index < len(words):
        count, opcode = words[index] >> 16, words[index] & 0xFFFF
        if count == 0:
            raise ArtifactError("malformed SPIR-V: zero-length instruction at word %d" % index)
        if opcode == OP_DECORATE and count >= 4:
            target, decoration = words[index + 1], words[index + 2]
            if decoration == DECORATION_DESCRIPTOR_SET:
                sets[target] = index + 3
            elif decoration == DECORATION_BINDING:
                bindings[target] = index + 3
        index += count
    for target, where in bindings.items():
        group = words[sets[target]] if target in sets else 0
        binding = words[where]
        if binding >= GL_SLOTS_PER_GROUP:
            raise ArtifactError("binding %d of set %d exceeds the %d GL slots of a group"
                                % (binding, group, GL_SLOTS_PER_GROUP))
        words[where] = group * GL_SLOTS_PER_GROUP + binding
    for where in sets.values():
        words[where] = 0
    return bytes_of(words)


def cross_compile(spirv, stage):
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / "module.spv"
        path.write_bytes(flatten_bindings(spirv))
        result = run([st.spirv_cross(), str(path), "--version", "450", "--no-es"],
                     "spirv-cross --version 450")
        text = result.stdout
        # The artifact must compile as OpenGL GLSL.
        check = Path(tmp) / ("artifact" + {v: k for k, v in STAGES.items()}[stage])
        check.write_text(text)
        run([st.glslang_validator(), str(check)], "glslangValidator (OpenGL) on the GLSL 4.50")
    return text


# ---------------------------------------------------------------------------
# Declared layouts


def load_layouts(path=LAYOUTS):
    layouts = json.loads(Path(path).read_text())
    if layouts.get("schema") != LAYOUT_SCHEMA:
        raise ArtifactError("%s: schema is not %s" % (path, LAYOUT_SCHEMA))
    return layouts


def owner_of(unit, layouts):
    """[(kind, name)] of every pass or family whose sources (path globs) or
    units (array-name globs) match the unit."""
    owners = []
    for kind in ("passes", "families"):
        for name, entry in layouts.get(kind, {}).items():
            if any(fnmatch.fnmatchcase(unit.source, pattern)
                   for pattern in entry.get("sources", [])) or \
                    any(fnmatch.fnmatchcase(unit.name, pattern)
                        for pattern in entry.get("units", [])):
                owners.append((kind, name))
    return owners


def declared_bindings(kind, entry):
    """{(set, binding): declaration} of a pass or family."""
    declared = {}
    if kind == "passes":
        for item in entry["bindings"]:
            declared[(item["set"], item["binding"])] = item
    else:
        for role, items in entry["groups"].items():
            for item in items:
                declared[(ROLES.index(role) if role in ROLES else -1,
                          item["binding"])] = dict(item, set=ROLES.index(role)
                                                   if role in ROLES else -1)
    return declared


def check_family_shape(checks, name, entry):
    """The bind-group ceiling and role names of a family."""
    groups = list(entry.get("groups", {}))
    checks.check(len(groups) <= MAX_GROUPS, "family.%s.bind-group-ceiling" % name,
                 "declares %d bind groups (%s); the port allows %d"
                 % (len(groups), ", ".join(groups), MAX_GROUPS))
    unknown = [group for group in groups if group not in ROLES]
    checks.check(not unknown, "family.%s.roles" % name,
                 "groups %s are not port roles (%s)" % (", ".join(unknown), ", ".join(ROLES)))
    for role, items in entry.get("groups", {}).items():
        for item in items:
            checks.check(item["kind"] in PORT_KINDS, "family.%s.%s.%d.port-kind"
                         % (name, role, item["binding"]),
                         "kind %s is not a render.device.v2 binding kind" % item["kind"])


def check_layouts(checks, units, reflections, layouts):
    """Every unit's reflection against its pass or family; every declaration
    used. Returns {pass or family: [set indices used]} for the debt report."""
    members = {}
    for unit in units:
        owners = owner_of(unit, layouts)
        if not checks.check(len(owners) == 1, "layout.owner.%s" % unit.name,
                            "%s belongs to %s; exactly one pass or family must declare it"
                            % (unit.source, owners or "no pass or family")):
            continue
        members.setdefault(owners[0], []).append(unit)
    for name, entry in sorted(layouts.get("families", {}).items()):
        check_family_shape(checks, name, entry)
    used_sets = {}
    for (kind, name), group in sorted(members.items()):
        entry = layouts[kind][name]
        declared = declared_bindings(kind, entry)
        used = set()
        for unit in group:
            problems = []
            for binding in reflections[unit.name]["bindings"]:
                key = (binding["set"], binding["binding"])
                item = declared.get(key)
                if item is None:
                    problems.append("set %d binding %d (%s) is not declared" % (
                        key[0], key[1], binding["kind"]))
                    continue
                used.add(key)
                if item["kind"] != binding["kind"]:
                    problems.append("set %d binding %d is %s, declared %s" % (
                        key[0], key[1], binding["kind"], item["kind"]))
                if item.get("count", 1) != binding["count"]:
                    problems.append("set %d binding %d has %d elements, declared %d" % (
                        key[0], key[1], binding["count"], item.get("count", 1)))
                if unit.stage not in item["stages"]:
                    problems.append("set %d binding %d is used by the %s stage, declared for %s"
                                    % (key[0], key[1], unit.stage, "/".join(item["stages"])))
            push = reflections[unit.name]["push_constants"]
            if push and unit.stage not in entry.get("push_constants", []):
                problems.append("uses push constants, declared for %s"
                                % ("/".join(entry.get("push_constants", [])) or "no stage"))
            checks.check(not problems, "layout.%s.%s" % (name, unit.name), "; ".join(problems))
        unused = sorted(set(declared) - used)
        checks.check(not unused, "layout.%s.declarations-used" % name,
                     "declared but used by no unit: %s" % ", ".join(
                         "set %d binding %d" % key for key in unused))
        used_sets[name] = sorted({key[0] for key in used})
    return used_sets


# ---------------------------------------------------------------------------
# Building


def build_unit(unit, root=ROOT):
    spirv = compile_spirv(unit, root)
    reflection = reflect(spirv)
    glsl, glsl_error = None, None
    try:
        glsl = cross_compile(spirv, unit.stage)
    except ArtifactError as error:
        glsl_error = str(error)
    return spirv, reflection, glsl, glsl_error


def build_all(units, root=ROOT):
    st.glslc()
    st.spirv_cross()
    st.glslang_validator()
    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 4) as pool:
        futures = {unit.name: pool.submit(build_unit, unit, root) for unit in units}
    results, errors = {}, {}
    for unit in units:
        try:
            results[unit.name] = futures[unit.name].result()
        except ArtifactError as error:
            errors[unit.name] = str(error)
    return results, errors


def excluded_targets(unit, layouts):
    """{format: reason} of the targets a unit is declared not to have."""
    for kind in ("passes", "families"):
        for entry in layouts.get(kind, {}).values():
            for rule in entry.get("excluded_targets", []):
                if any(fnmatch.fnmatchcase(unit.name, pattern) for pattern in rule["units"]):
                    return {rule["format"]: rule["reason"]}
    return {}


def build_artifacts(checks, units, layouts, out=None, root=ROOT):
    """Compile every unit, check every artifact; write them when `out` is set.
    Returns ({unit name: SPIR-V words}, index)."""
    pin = st.load_pin()
    identities = compiler_identities(pin)
    results, errors = build_all(units, root)
    keys = {}
    for unit in units:
        key = (unit.source, unit.permutation)
        checks.check(key not in keys, "key.unique.%s" % unit.name,
                     "same source and permutation as %s" % keys.get(key))
        keys.setdefault(key, unit.name)
    index = {"schema": INDEX_SCHEMA, "compilers": identities,
             "identity": {"glslc": pin["compiler"]["identity"],
                          "spirv-cross": pin["cross_compiler"]["identity"]},
             "gl_slots_per_group": GL_SLOTS_PER_GROUP, "artifacts": []}
    words, reflections = {}, {}
    for unit in units:
        if not checks.check(unit.name not in errors, "spirv.%s" % unit.name,
                            errors.get(unit.name, "")):
            continue
        spirv, reflection, glsl, glsl_error = results[unit.name]
        words[unit.name] = words_of(spirv)
        reflections[unit.name] = reflection
        checks.check(len(spirv) % 4 == 0 and words[unit.name][0] == st.SPIRV_MAGIC,
                     "spirv.valid.%s" % unit.name, "not a SPIR-V module")
        excluded = excluded_targets(unit, layouts)
        outputs = {"spirv": spirv}
        if "glsl450" in excluded:
            checks.check(glsl_error is not None, "glsl450.excluded-needed.%s" % unit.name,
                         "declared without a GLSL 4.50 artifact (%s) but it cross-compiles; "
                         "remove the exclusion" % excluded["glsl450"])
        elif checks.check(glsl_error is None, "glsl450.%s" % unit.name, glsl_error or ""):
            outputs["glsl450"] = glsl.encode()
        for fmt, data in outputs.items():
            record = {
                "id": unit.name, "source": unit.source, "stage": unit.stage,
                "options": unit.options, "axes": unit.axes, "permutation": unit.permutation,
                "format": fmt, "compiler": identities[fmt], "file": artifact_file(unit, fmt),
                "sha256": hashlib.sha256(data).hexdigest(), "bytes": len(data),
                "header": unit.header,
                "bindings": [dict(b, gl_slot=b["set"] * GL_SLOTS_PER_GROUP + b["binding"])
                             if fmt == "glsl450" else b for b in reflection["bindings"]],
                "push_constants": reflection["push_constants"],
                "specialization_constants": reflection["specialization_constants"],
            }
            if fmt == "glsl450":
                record["spirv_source"] = artifact_file(unit, "spirv")
            index["artifacts"].append(record)
            if out:
                target = Path(out) / record["file"]
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(data)
        for fmt, reason in excluded.items():
            index["artifacts"].append({"id": unit.name, "source": unit.source, "format": fmt,
                                       "excluded": reason})
    built = [unit for unit in units if unit.name in reflections]
    index["sets_used"] = check_layouts(checks, built, reflections, layouts)
    index["over_ceiling"] = {name: sets for name, sets in index["sets_used"].items()
                             if len(sets) > MAX_GROUPS or any(s >= MAX_GROUPS for s in sets)}
    # What the switch-over onto render.device.v2 must change: binding kinds
    # the port does not have. Push constants are the port's draw constants
    # (clause D16) up to kMaxDrawConstantBytes, so only a larger block owes.
    debt = {}
    for kind in ("passes", "families"):
        for name, entry in layouts.get(kind, {}).items():
            items = entry.get("bindings", []) + [b for group in entry.get("groups", {}).values()
                                                 for b in group]
            foreign = sorted({b["kind"] for b in items} - PORT_KINDS)
            if entry.get("push_constants") and \
                    entry.get("push_constant_bytes", 0) > MAX_DRAW_CONSTANT_BYTES:
                foreign.append("push-constants over %d bytes" % MAX_DRAW_CONSTANT_BYTES)
            if foreign:
                debt[name] = foreign
    index["non_port_interface"] = debt
    if out:
        Path(out).mkdir(parents=True, exist_ok=True)
        (Path(out) / "index.json").write_text(json.dumps(index, indent=1) + "\n")
    return words, index


# ---------------------------------------------------------------------------
# Headers from artifacts


def headers_from_artifacts(units, words, root=ROOT):
    """{header file name: text} written by the regenerators' own writers, fed
    the artifact SPIR-V instead of compiling."""
    material, legacy = regenerators(root)
    by_material = {(u.source[len(SHADERS) + 1:], tuple(u.extra)): words.get(u.name)
                   for u in units if u.header == "material"}
    by_legacy = {Path(u.source).name: words.get(u.name) for u in units if u.header == "legacy"}

    def material_words(source, extra):
        found = by_material.get((source, tuple(extra)))
        if found is None:
            raise ArtifactError("no artifact for %s %s" % (source, " ".join(extra)))
        return found

    def legacy_words(source):
        found = by_legacy.get(Path(source).name)
        if found is None:
            raise ArtifactError("no artifact for legacy/%s" % Path(source).name)
        return found

    material.compile_words = material_words
    legacy.compile_words = legacy_words
    material_text, index_text = material.render()
    return {"material_spv.h": material_text, "material_spv_index.h": index_text,
            "legacy_spv.h": legacy.render()}


def first_difference(actual, expected):
    for number, (a, b) in enumerate(zip(actual.splitlines(), expected.splitlines()), 1):
        if a != b:
            return "line %d is %r, the artifacts write %r" % (number, a[:80], b[:80])
    return "%d lines, the artifacts write %d" % (len(actual.splitlines()),
                                                 len(expected.splitlines()))


def write_headers(units, words, out_dir, root=ROOT):
    """Every generated header (shader_toolchain.GENERATED_NAMES) into out_dir,
    flat: the backend's with the regenerators' writers fed the artifacts'
    SPIR-V, the GENERATED headers from their rows (a row that is a unit takes
    its artifact; the others are compiled here). Returns the names written."""
    written = headers_from_artifacts(units, words, root)
    words = dict(words)
    missing = [(array, source, options) for _, array, source, options in st.generated_rows()
               if array not in words]
    compiler = st.glslc()
    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 4) as pool:
        futures = [(array, pool.submit(st.compile_module, compiler, Path(root) / source,
                                       list(options)))
                   for array, source, options in missing]
    for array, future in futures:
        words[array] = future.result()
    for header in st.GENERATED:
        written[header] = st.render_generated(header, lambda array: words[array])
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    for name, text in written.items():
        target = out_dir / name
        # Unchanged headers keep their timestamps, so consumers do not rebuild.
        if not target.is_file() or target.read_text() != text:
            target.write_text(text)
    return sorted(written)


def generate_headers(out_dir, root=ROOT):
    """The build's generated headers without the other targets' artifacts
    (the conformance runner and the toolchain check use it)."""
    units = inventory(root)
    st.glslc()
    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 4) as pool:
        futures = {unit.name: pool.submit(compile_spirv, unit, root) for unit in units}
    words = {name: words_of(future.result()) for name, future in futures.items()}
    return write_headers(units, words, out_dir, root)


def check_headers(checks, units, words, generated_dir, root=ROOT):
    """The generated headers in generated_dir: every one written; the
    backend's agree with the regenerators, independent writers that compile
    for themselves; each array of a unit equals its artifact."""
    generated_dir = Path(generated_dir)
    for name in st.GENERATED_NAMES:
        checks.check((generated_dir / name).is_file(), "headers.%s.written" % name,
                     "the build did not write it")
    for name, script, _ in st.REGENERATORS:
        result = subprocess.run([sys.executable, str(Path(root) / script), "--check",
                                 "--compare-dir", str(generated_dir)],
                                capture_output=True, text=True)
        detail = (result.stdout + result.stderr).strip().splitlines()
        checks.check(result.returncode == 0, "headers.%s.regenerator-agrees" % name,
                     "%s --check exited %d: %s" % (Path(script).name, result.returncode,
                                                   " | ".join(detail[-3:])))
    for unit in units:
        if unit.header in ("material", "legacy"):
            continue
        path = generated_dir / unit.header
        arrays = st.embedded_arrays(path.read_text()) if path.is_file() else {}
        checks.check(arrays.get(unit.name) == words.get(unit.name),
                     "headers.%s:%s.identical" % (unit.header, unit.name),
                     "the generated array differs from its SPIR-V artifact")


# ---------------------------------------------------------------------------
# Seeded faults


def seed_layout_mismatch(layouts):
    """Change the kind of the first declared binding of the first pass;
    returns the pass name."""
    name = sorted(layouts["passes"])[0]
    binding = layouts["passes"][name]["bindings"][0]
    binding["kind"] = "storage-buffer" if binding["kind"] != "storage-buffer" \
        else "uniform-buffer"
    return name


FIFTH_GROUP_FAMILY = {
    "sources": [],
    "groups": {"frame": [], "view": [], "material": [], "draw": [], "extra": []},
}


def seed_generated(generated_dir, units):
    """One word of one array changed in each backend header written in
    generated_dir; returns the checks that must fail."""
    expected = set()
    for header in ["material_spv.h", "legacy_spv.h"] + sorted(
            {u.header for u in units if u.header not in ("material", "legacy")}):
        array, _ = st.flip_one_byte(Path(generated_dir) / header)
        if header == "material_spv.h":
            expected.add("headers.material.regenerator-agrees")
        elif header == "legacy_spv.h":
            expected.add("headers.legacy.regenerator-agrees")
        else:
            expected.add("headers.%s:%s.identical" % (header, array))
    return expected


# ---------------------------------------------------------------------------
# Commands


class RecordingChecks(st.RecordingChecks):
    pass


def run_check(out=None, seed_fault=None, root=ROOT, stream=None):
    checks = RecordingChecks(stream)
    evidence = {"schema": "shader-artifacts-evidence/v1", "revision": st.git_revision(root),
                "seed_fault": seed_fault}
    scratch = Path(out) if out else Path(tempfile.mkdtemp(prefix="shader-artifacts-"))
    scratch.mkdir(parents=True, exist_ok=True)
    try:
        pin = st.load_pin()
        checks.check("cross_compiler" in pin, "toolchain.cross-pin",
                     "quality/toolchain/shader-compiler.json has no cross_compiler")
        for what, resolver in (("glslc", st.glslc), ("spirv-cross", st.spirv_cross),
                               ("glslangValidator", st.glslang_validator)):
            try:
                resolver()
                checks.check(True, "toolchain.%s" % what)
            except st.ToolchainError as error:
                checks.check(False, "toolchain.%s" % what, str(error))
                return checks, evidence
        layouts = load_layouts()
        units = inventory(root)
    except (OSError, ValueError, ArtifactError, st.ToolchainError) as error:
        checks.check(False, "setup", str(error))
        return checks, evidence
    expected = set()
    if seed_fault == "layout-mismatch":
        name = seed_layout_mismatch(layouts)
        evidence["seeded"] = "kind of the first binding of pass %s" % name
        # The first pass has one unit, which uses its first binding.
        expected = {"layout.%s.%s" % (name, unit.name) for unit in units
                    if owner_of(unit, layouts) == [("passes", name)]}
    elif seed_fault == "fifth-group":
        layouts.setdefault("families", {})["seeded-five-groups"] = FIFTH_GROUP_FAMILY
        evidence["seeded"] = "family seeded-five-groups with five groups"
        expected = {"family.seeded-five-groups.bind-group-ceiling",
                    "family.seeded-five-groups.roles"}
    evidence["units"] = len(units)
    words, index = build_artifacts(checks, units, layouts, scratch / "artifacts", root)
    generated = scratch / "generated"
    try:
        write_headers(units, words, generated, root)
    except (ArtifactError, st.ToolchainError) as error:
        checks.check(False, "headers.written", str(error))
    if seed_fault == "flip-word":
        expected = seed_generated(generated, units)
        evidence["seeded"] = sorted(expected)
    check_headers(checks, units, words, generated, root)
    evidence["artifacts"] = sum(1 for a in index["artifacts"] if "file" in a)
    evidence["excluded"] = [a for a in index["artifacts"] if "excluded" in a]
    evidence["sets_used"] = index["sets_used"]
    evidence["over_ceiling"] = index["over_ceiling"]
    evidence["non_port_interface"] = index["non_port_interface"]
    evidence["expected_failures"] = sorted(expected)
    return checks, evidence


def command_check(args):
    checks, evidence = run_check(args.out, args.seed_fault)
    evidence.update(checks=checks.checks, failures=checks.failures)
    if args.out:
        Path(args.out).mkdir(parents=True, exist_ok=True)
        (Path(args.out) / "shader-artifacts-evidence.json").write_text(
            json.dumps(dict(evidence, results=checks.results), indent=1) + "\n")
    print("units %d, artifacts %d (%d excluded targets); passes over the four-group "
          "ceiling: %s" % (evidence.get("units", 0), evidence.get("artifacts", 0),
                           len(evidence.get("excluded", [])),
                           ", ".join("%s %s" % (k, v) for k, v in
                                     sorted(evidence.get("over_ceiling", {}).items()))
                           or "none"))
    return checks.report()


def failed_names(checks):
    return {result["name"] for result in checks.results if not result["ok"]}


def command_sensitivity(args):
    checks = Checks()
    out = Path(args.out) if args.out else Path(tempfile.mkdtemp(prefix="shader-artifacts-"))
    for fault in (None, "layout-mismatch", "fifth-group", "flip-word"):
        stream = io.StringIO()
        run, evidence = run_check(out / (fault or "control"), fault, stream=stream)
        (out / ((fault or "control") + ".log")).write_text(stream.getvalue())
        failed = failed_names(run)
        if fault is None:
            checks.check(run.checks > 0 and not failed, "control.passes",
                         "the unseeded check failed: %s" % ", ".join(sorted(failed)[:8]))
            continue
        expected = set(evidence.get("expected_failures", []))
        checks.check(bool(expected), "%s.seeded" % fault, "nothing was seeded")
        for name in sorted(expected):
            checks.check(name in failed, "%s.detected.%s" % (fault, name),
                         "the seeded defect passed")
        checks.check(failed <= expected, "%s.only-seeded" % fault,
                     "unseeded checks failed: %s" % ", ".join(sorted(failed - expected)[:8]))
    return checks.report()


def command_build(args):
    checks = Checks(sys.stderr)
    layouts = load_layouts()
    units = inventory()
    out = Path(args.out)
    if out.exists():
        for fmt in FORMATS:
            shutil.rmtree(out / fmt, ignore_errors=True)
    words, index = build_artifacts(checks, units, layouts, out)
    if args.headers and not checks.failures:
        write_headers(units, words, args.headers)
    print("shader_artifacts: %d units, %d artifacts, %d failures" % (
        len(units), sum(1 for a in index["artifacts"] if "file" in a), checks.failures),
        file=sys.stderr)
    return 1 if checks.failures else 0


def command_headers(args):
    try:
        names = generate_headers(args.out)
    except (ArtifactError, st.ToolchainError) as error:
        print("shader_artifacts: %s" % error, file=sys.stderr)
        return 1
    print("shader_artifacts: %d generated headers in %s" % (len(names), args.out),
          file=sys.stderr)
    return 0


def command_derive_layouts(_args):
    """The bindings each source's reflections imply, grouped by the current
    declaration's owners (for review; never written)."""
    layouts = load_layouts()
    units = inventory()
    results, errors = build_all(units)
    derived = {}
    for unit in units:
        if unit.name in errors:
            print("error %s: %s" % (unit.name, errors[unit.name]), file=sys.stderr)
            continue
        owners = owner_of(unit, layouts)
        name = owners[0][1] if len(owners) == 1 else "unowned:" + unit.source
        entry = derived.setdefault(name, {})
        if results[unit.name][1]["push_constants"]:
            push = entry.setdefault("push_constants", [])
            if unit.stage not in push:
                push.append(unit.stage)
        for binding in results[unit.name][1]["bindings"]:
            key = (binding["set"], binding["binding"])
            item = entry.setdefault(key, {"set": key[0], "binding": key[1],
                                          "kind": binding["kind"], "count": binding["count"],
                                          "stages": []})
            if unit.stage not in item["stages"]:
                item["stages"].append(unit.stage)
                item["stages"].sort(key=["vertex", "fragment", "compute"].index)
    print(json.dumps({name: {"bindings": [entry[key] for key in sorted(k for k in entry
                                                                    if isinstance(k, tuple))],
                             "push_constants": entry.get("push_constants", [])}
                      for name, entry in sorted(derived.items())}, indent=1))
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    commands = parser.add_subparsers(dest="command", required=True)
    build_parser = commands.add_parser("build", help="build every artifact and its index")
    build_parser.add_argument("--out", required=True)
    build_parser.add_argument("--headers", help="also write every generated header here")
    build_parser.set_defaults(run=command_build)
    headers_parser = commands.add_parser(
        "headers", help="write only the generated headers (no GLSL 4.50 artifacts)")
    headers_parser.add_argument("--out", required=True)
    headers_parser.set_defaults(run=command_headers)
    check_parser = commands.add_parser("check", help="the render.shader-artifacts gate")
    check_parser.add_argument("--out", help="evidence and scratch directory")
    check_parser.add_argument("--seed-fault", choices=("layout-mismatch", "fifth-group",
                                                       "flip-word"))
    check_parser.set_defaults(run=command_check)
    sensitivity_parser = commands.add_parser("sensitivity", help="the check against its faults")
    sensitivity_parser.add_argument("--out", help="scratch directory")
    sensitivity_parser.set_defaults(run=command_sensitivity)
    commands.add_parser("derive-layouts", help="print the reflected pass bindings") \
        .set_defaults(run=command_derive_layouts)
    args = parser.parse_args(argv)
    try:
        return args.run(args)
    except (ArtifactError, st.ToolchainError) as error:
        print("shader_artifacts: %s" % error, file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
