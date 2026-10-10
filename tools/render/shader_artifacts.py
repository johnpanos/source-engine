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
(shader_toolchain.GENERATED_NAMES): material_spv.h and material_spv_index.h
from the SPIR-V artifacts with the regenerator's own writers,
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
from collections import Counter
import concurrent.futures
import copy
import fnmatch
import hashlib
import importlib.util
import io
import json
import os
from pathlib import Path
import re
import struct
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools" / "render"))
sys.path.insert(0, str(ROOT / "tools" / "quality"))
import shader_toolchain as st  # noqa: E402
from conformance_result import Checks  # noqa: E402

SHADERS = "render/shaders/legacy"
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
# The GL adapter's contract with the GLSL 4.50 artifacts (render/device/gl,
# RFC 0016 K10), besides the flattened slots:
# - a sampled texture of group g, binding b is named rg_t<g>_<b> and a sampler
#   rg_s<g>_<b>, so SPIRV-Cross names each combined sampler it builds
#   SPIRV_Cross_Combinedrg_t<g>_<b>rg_s<g>_<b> (a texel fetch without a sampler:
#   ...SPIRV_Cross_DummySampler) on the texture's slot, and the adapter binds
#   that sampler to the slot;
# - the draw constants (D16) are the uniform block GL_DRAW_CONSTANTS_BLOCK,
#   which the adapter binds at GL_DRAW_CONSTANTS_SLOT;
# - specialization constant n (D20) is SPIRV-Cross's macro
#   SPIRV_CROSS_CONSTANT_ID_n (its default when nothing defines it); the line
#   after #version, GL_SPECIALIZATION_LINE, lists each constant's id and type
#   (int, uint, float, bool), so the adapter can define the macro as a literal
#   of that type (GLSL has no constant-expression bit cast).
GL_SPECIALIZATION_LINE = "// render.device.gl specialization"
GL_SPECIALIZATION_TYPES = ("int", "uint", "float", "bool")
GL_DRAW_CONSTANTS_BLOCK = "RenderDrawConstants"
GL_DRAW_CONSTANTS_SLOT = MAX_GROUPS * GL_SLOTS_PER_GROUP
GL_TEXTURE_PREFIX = "rg_t"
GL_SAMPLER_PREFIX = "rg_s"
OP_NAME = 5
OP_MEMBER_NAME = 6
OP_TYPE_IMAGE = 25
OP_TYPE_SAMPLER = 26
OP_TYPE_ARRAY = 28
OP_TYPE_RUNTIME_ARRAY = 29
OP_TYPE_POINTER = 32
OP_VARIABLE = 59
STORAGE_UNIFORM_CONSTANT = 0
STORAGE_PUSH_CONSTANT = 9
# Module-layout instructions that come before the debug names (SPIR-V 2.4).
PRELUDE_OPS = {17, 10, 11, 14, 15, 16, 331, 7, 2, 3, 4}


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
    return load_module("regen_material_spv", base / "regen_material_spv.py")


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
    material = regenerators(root)
    units = [Unit(name, SHADERS + "/" + source, ["-O"], extra, "material")
             for name, source, extra in material.SHADERS]
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


def instructions(words):
    """(index, word count, opcode) of every instruction after the header."""
    index = 5
    while index < len(words):
        count, opcode = words[index] >> 16, words[index] & 0xFFFF
        if count == 0:
            raise ArtifactError("malformed SPIR-V: zero-length instruction at word %d" % index)
        yield index, count, opcode
        index += count


def string_words(text):
    data = text.encode() + b"\0"
    data += b"\0" * (-len(data) % 4)
    return words_of(data)


def gl_names(spirv):
    """{id: name} the GL adapter reads back (GL_TEXTURE_PREFIX, GL_SAMPLER_PREFIX,
    GL_DRAW_CONSTANTS_BLOCK) for the module's sampled textures, samplers and
    draw-constant block type, from their original (set, binding)."""
    words = words_of(spirv)
    sets, bindings, types, pointers, variables = {}, {}, {}, {}, []
    for index, count, opcode in instructions(words):
        operands = words[index + 1:index + count]
        if opcode == OP_DECORATE and len(operands) >= 3:
            if operands[1] == DECORATION_DESCRIPTOR_SET:
                sets[operands[0]] = operands[2]
            elif operands[1] == DECORATION_BINDING:
                bindings[operands[0]] = operands[2]
        elif opcode == OP_TYPE_IMAGE:
            # Sampled: 1 a sampled image, 2 a storage image.
            types[operands[0]] = ("image", operands[6] if len(operands) > 6 else 0)
        elif opcode == OP_TYPE_SAMPLER:
            types[operands[0]] = ("sampler", 0)
        elif opcode in (OP_TYPE_ARRAY, OP_TYPE_RUNTIME_ARRAY):
            types[operands[0]] = ("array", operands[1])
        elif opcode == OP_TYPE_POINTER:
            pointers[operands[0]] = (operands[1], operands[2])
        elif opcode == OP_VARIABLE:
            variables.append((operands[1], operands[0], operands[2]))

    def element(type_id):
        while types.get(type_id, ("", 0))[0] == "array":
            type_id = types[type_id][1]
        return type_id

    names = {}
    for variable, pointer, storage in variables:
        if pointer not in pointers:
            continue
        pointee = element(pointers[pointer][1])
        if storage == STORAGE_PUSH_CONSTANT:
            names[pointee] = GL_DRAW_CONSTANTS_BLOCK
            continue
        if storage != STORAGE_UNIFORM_CONSTANT or variable not in bindings:
            continue
        kind, sampled = types.get(pointee, ("", 0))
        where = "%d_%d" % (sets.get(variable, 0), bindings[variable])
        if kind == "image" and sampled == 1:
            names[variable] = GL_TEXTURE_PREFIX + where
        elif kind == "sampler":
            names[variable] = GL_SAMPLER_PREFIX + where
    return names


def rename(spirv, names):
    """A copy of the module whose OpName for each id in names is names[id]."""
    words = words_of(spirv)
    out, insert_at = words[:5], None
    for index, count, opcode in instructions(words):
        if insert_at is None and opcode not in PRELUDE_OPS:
            insert_at = len(out)
        if opcode == OP_NAME and words[index + 1] in names:
            continue
        out.extend(words[index:index + count])
    added = []
    for target, name in sorted(names.items()):
        operands = [target] + string_words(name)
        added += [((len(operands) + 1) << 16) | OP_NAME] + operands
    if insert_at is None:
        insert_at = len(out)
    return bytes_of(out[:insert_at] + added + out[insert_at:])


def mark_specialization(text, constants):
    """The GLSL with GL_SPECIALIZATION_LINE after its #version line, listing
    each specialization constant as id:type, when it has any. Every constant
    must be of a GL_SPECIALIZATION_TYPES type and have SPIRV-Cross's macro."""
    if not constants:
        return text
    entries = []
    for constant in sorted(constants, key=lambda c: c["id"]):
        kind = constant.get("type")
        if kind not in GL_SPECIALIZATION_TYPES:
            raise ArtifactError("specialization constant %d has type %s, which the GL "
                                "artifacts do not carry" % (constant["id"], kind))
        if "#define SPIRV_CROSS_CONSTANT_ID_%d " % constant["id"] not in text:
            raise ArtifactError("SPIRV-Cross wrote no macro for specialization constant %d"
                                % constant["id"])
        entries.append("%d:%s" % (constant["id"], kind))
    version, _, rest = text.partition("\n")
    if not version.startswith("#version"):
        raise ArtifactError("SPIRV-Cross's GLSL does not start with #version")
    return "%s\n%s %s\n%s" % (version, GL_SPECIALIZATION_LINE, " ".join(entries), rest)


def cross_compile(spirv, stage, es=False):
    """The GLSL 4.50 artifact (es: GLSL ES 3.10, RFC 0022) of a SPIR-V module
    for the GL adapter: slots flattened, resources named, the draw constants a
    uniform block, combined samplers on their textures' slots, specialization
    constants listed with their types. Both dialects share that form; the ES
    one adds default precisions. It must compile with the pinned
    glslangValidator for its API."""
    version = ["--version", "310", "--es"] if es else ["--version", "450", "--no-es"]
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / "module.spv"
        path.write_bytes(spirv)
        constants = json.loads(run([st.spirv_cross(), str(path), "--reflect"],
                                   "spirv-cross --reflect").stdout).get(
                                       "specialization_constants", [])
        path.write_bytes(flatten_bindings(rename(spirv, gl_names(spirv))))
        result = run([st.spirv_cross(), str(path), *version,
                      "--combined-samplers-inherit-bindings", "--glsl-emit-push-constant-as-ubo"],
                     "spirv-cross " + " ".join(version))
        text = result.stdout
        if es:
            # The port's math is 32-bit: SPIRV-Cross defaults fragment floats to
            # mediump, which ES 3.1 lets a driver run at 16 bits.
            text = text.replace("precision mediump float;", "precision highp float;")
            text = text.replace("precision mediump int;", "precision highp int;")
            if re.search(r"\b[iu]?samplerCubeArray\b", text):
                # Cube arrays (D36) are core from ES 3.2's 320 es only; a 310 es
                # program takes them from the extension the adapter claims
                # kCubeArrays on. Without either it fails to compile, and the
                # device lacks the capability anyway.
                version_line, newline, rest = text.partition("\n")
                text = (version_line + newline +
                        "#if defined(GL_EXT_texture_cube_map_array)\n"
                        "#extension GL_EXT_texture_cube_map_array : enable\n"
                        "#elif defined(GL_OES_texture_cube_map_array)\n"
                        "#extension GL_OES_texture_cube_map_array : enable\n"
                        "#endif\n" + rest)
        text = mark_specialization(text, constants)
        text, bound = re.subn(r"layout\(std140\) uniform %s\b" % GL_DRAW_CONSTANTS_BLOCK,
                              "layout(binding = %d, std140) uniform %s"
                              % (GL_DRAW_CONSTANTS_SLOT, GL_DRAW_CONSTANTS_BLOCK), text)
        if bound != text.count("uniform " + GL_DRAW_CONSTANTS_BLOCK):
            raise ArtifactError("SPIRV-Cross wrote the draw constants in an unexpected form")
        check = Path(tmp) / ("artifact" + {v: k for k, v in STAGES.items()}[stage])
        check.write_text(text)
        run([st.glslang_validator(), str(check)],
            "glslangValidator on the GLSL ES 3.10" if es else
            "glslangValidator (OpenGL) on the GLSL 4.50")
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
    material = regenerators(root)
    by_material = {(u.source[len(SHADERS) + 1:], tuple(u.extra)): words.get(u.name)
                   for u in units if u.header == "material"}

    def material_words(source, extra):
        found = by_material.get((source, tuple(extra)))
        if found is None:
            raise ArtifactError("no artifact for %s %s" % (source, " ".join(extra)))
        return found

    material.compile_words = material_words
    material_text, index_text = material.render()
    return {"material_spv.h": material_text, "material_spv_index.h": index_text}


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
    glsl, es_missing = glsl_headers(words, root)
    written.update(glsl)
    written[st.STORE_HEADER] = store_header(words, es_missing)
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    for name, text in written.items():
        target = out_dir / name
        # Unchanged headers keep their timestamps, so consumers do not rebuild.
        if not target.is_file() or target.read_text() != text:
            target.write_text(text)
    return sorted(written)


PORT_BINDING_KINDS = {
    "uniform-buffer": "kUniformBuffer", "storage-buffer": "kStorageBuffer",
    "sampled-texture": "kSampledTexture", "storage-texture": "kStorageTexture",
    "sampler": "kSampler"}
STAGE_ENUMS = {"vertex": "kVertex", "fragment": "kFragment", "compute": "kCompute"}
SCALAR_BYTES = {"float": 4, "int": 4, "uint": 4, "bool": 4}


def type_bytes(name, member, types):
    """Bytes a push-constant member of reflected type `name` spans."""
    if name in types:
        return block_bytes(types[name]["members"], types)
    base = name.rstrip("0123456789x")
    digits = name[len(base):]
    if base in ("vec", "ivec", "uvec", "bvec") and digits.isdigit():
        size = 4 * int(digits)
    elif base == "mat" and digits:
        columns, _, rows = digits.partition("x")
        columns, rows = int(columns), int(rows or columns)
        size = member.get("matrix_stride", 16) * (rows if member.get("row_major") else columns)
    elif name in SCALAR_BYTES:
        size = SCALAR_BYTES[name]
    else:
        raise ArtifactError("draw constants: no size for reflected type %s" % name)
    if "array" in member:
        count = 1
        for extent in member["array"]:
            count *= extent
        size = member.get("array_stride", size) * count
    return size


def block_bytes(members, types):
    return max((m.get("offset", 0) + type_bytes(m["type"], m, types) for m in members),
               default=0)


def reflect_port(spirv):
    """(bindings as (set, binding, port kind enum), draw-constant bytes) of a
    module, for the artifact store."""
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / "module.spv"
        path.write_bytes(spirv)
        data = json.loads(run([st.spirv_cross(), str(path), "--reflect"],
                              "spirv-cross --reflect").stdout)
    bindings = []
    for binding in reflect(spirv)["bindings"]:
        kind = PORT_BINDING_KINDS.get(binding["kind"])
        if kind is None:
            raise ArtifactError("binding (%d, %d) is a %s, which the port has no kind for"
                                % (binding["set"], binding["binding"], binding["kind"]))
        bindings.append((binding["set"], binding["binding"], kind))
    push = 0
    for block in data.get("push_constants", []):
        push = max(push, block_bytes(data["types"][block["type"]]["members"], data["types"]))
    return bindings, push


def store_header(words, es_missing=None):
    """spv/core_artifact_table.h: the artifact store's table (public/render/
    shaderlib/core_artifacts.h), one entry per core program row and format:
    source, stage, format, code (the generated SPIR-V, GLSL 4.50 and, for the
    rows in es_missing's complement, GLSL ES 3.10 arrays),
    reflected bindings and draw-constant bytes."""
    pin = st.load_pin()
    compiler = compiler_identities(pin)["glsl450"]
    includes, bindings_out, entries = [], [], []
    for header in st.CORE_PROGRAM_HEADERS:
        namespace, _, rows = st.GENERATED[header]
        includes += [header, header.replace("_spv.h", "_glsl.h"),
                     header.replace("_spv.h", "_gles.h")]
        for array, source, _ in rows:
            spirv = bytes_of(words[array])
            reflected, push = reflect_port(spirv)
            table = "kBindings_%s" % array
            if reflected:
                bindings_out.append("inline constexpr device::ReflectedBinding %s[] = { %s };\n"
                                    % (table, ", ".join(
                                        "{ %d, %d, device::BindingKind::%s }" % b
                                        for b in reflected)))
            span = table if reflected else "{}"
            stage = STAGE_ENUMS[STAGES[Path(source).suffix]]
            glsl_namespace = namespace.replace("::spirv", "::glsl")
            entries.append('    { "%s", device::ShaderStage::%s, device::ArtifactFormat::kSpirv,\n'
                           '        std::as_bytes( std::span( %s::%s ) ), %s, %d },\n'
                           % (source, stage, namespace, array, span, push))
            entries.append('    { "%s", device::ShaderStage::%s, device::ArtifactFormat::kGlsl450,\n'
                           '        std::as_bytes( std::span( %s::%s ).first( sizeof( %s::%s ) - 1 ) ),'
                           ' %s, %d },\n'
                           % (source, stage, glsl_namespace, array, glsl_namespace, array, span,
                              push))
            for missing, suffix, enum in ((es_missing, "gles", "kGlslEs310"),):
                if array in (missing or {}):
                    continue
                text_namespace = namespace.replace("::spirv", "::" + suffix)
                entries.append('    { "%s", device::ShaderStage::%s, device::ArtifactFormat::%s,\n'
                               '        std::as_bytes( std::span( %s::%s ).first( sizeof( %s::%s ) - 1 ) ),'
                               ' %s, %d },\n'
                               % (source, stage, enum, text_namespace, array, text_namespace, array,
                                  span, push))
    return "".join(
        ["//========= Copyright Valve Corporation, All rights reserved. ============//\n",
         "//\n",
         "// Purpose: The core artifact store's table (render/shaderlib/core_artifacts.cpp):\n",
         "//          every core program's SPIR-V and GLSL 4.50 with its reflection. GENERATED\n",
         "//          by tools/render/shader_artifacts.py headers; not committed, do not edit.\n",
         "//\n",
         "//=============================================================================//\n\n",
         "#ifndef GENERATED_CORE_ARTIFACTS_H\n#define GENERATED_CORE_ARTIFACTS_H\n\n",
         '#include "render/device/pipeline.h"\n'] +
        ['#include "spv/%s"\n' % name for name in includes] +
        ["\n#include <cstddef>\n#include <cstdint>\n#include <span>\n\n",
         "namespace render::shaderlib::generated\n{\n\n",
         'inline constexpr const char *kCompiler = "%s";\n\n' % compiler,
         "struct CoreArtifact\n{\n",
         "\tconst char *source;\n\tdevice::ShaderStage stage;\n",
         "\tdevice::ArtifactFormat format;\n\tstd::span<const std::byte> code;\n",
         "\tstd::span<const device::ReflectedBinding> bindings;\n",
         "\tstd::uint32_t drawConstantBytes;\n};\n\n"] +
        bindings_out +
        ["\ninline const CoreArtifact kCoreArtifacts[] = {\n"] + entries +
        ["};\n\n} // namespace render::shaderlib::generated\n\n",
         "#endif // GENERATED_CORE_ARTIFACTS_H\n"])


def glsl_headers(words, root=ROOT):
    """({name: text} of the GLSL_GENERATED and GLES_GENERATED headers,
    {array: reason} of the rows with no ES artifact): each row's SPIR-V (its
    artifact when the row is a unit or a GENERATED row, else compiled here)
    through cross_compile in both dialects. A GLSL 4.50 failure fails the
    build; an ES failure leaves the row out of its header and the store."""
    rows = [row for _, _, header_rows in st.GLSL_GENERATED.values() for row in header_rows]
    compiler = st.glslc()

    def spirv_of(row):
        array, source, options = row
        spirv = words.get(array)
        if spirv is None:
            spirv = st.compile_module(compiler, Path(root) / source, list(options))
        return bytes_of(spirv)

    def text_of(row):
        return cross_compile(spirv_of(row), STAGES[Path(row[1]).suffix])

    def es_text_of(row):
        try:
            return cross_compile(spirv_of(row), STAGES[Path(row[1]).suffix], es=True), None
        except ArtifactError as error:
            return None, str(error)

    with concurrent.futures.ThreadPoolExecutor(os.cpu_count() or 4) as pool:
        texts = dict(zip((row[0] for row in rows), pool.map(text_of, rows)))
        es_results = dict(zip((row[0] for row in rows), pool.map(es_text_of, rows)))
    headers = {header: st.render_glsl(header, lambda array: texts[array])
               for header in st.GLSL_GENERATED}
    headers.update({header: st.render_glsl(header, lambda array: es_results[array][0])
                    for header in st.GLES_GENERATED})
    es_missing = {array: reason for array, (text, reason) in es_results.items() if text is None}
    return headers, es_missing


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
        if unit.header == "material":
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
    for header in ["material_spv.h"] + sorted(
            {u.header for u in units if u.header != "material"}):
        array, _ = st.flip_one_byte(Path(generated_dir) / header)
        if header == "material_spv.h":
            expected.add("headers.material.regenerator-agrees")
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
