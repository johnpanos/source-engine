#!/usr/bin/env python3
"""Recorded vtable tables of frozen C++ interfaces: the shared machinery.

A table lists every vtable slot of a set of frozen interfaces in Itanium
(Linux x86_64) order: each virtual method with its full signature, the
this-adjustment a caller through the interface applies, and its function
slot; destructor pairs; secondary vtables; the slot count; and the interface
version string where the header defines one. A legacy-cxx11 conformance
suite includes the table and derives each slot from the compiler's own
pointer-to-member-function value (unittests/abitable/abi_table_check.h).

One `Spec` describes one table: its interfaces, the manifest row whose
flags build it, its macro prefix and its probe names. The tools that own a
table hold its Spec and nothing else:

    tools/render/render_abi.py   RFC 0016 K0, legacy.render-abi
    tools/vgui/vgui_abi.py       RFC 0010 V0, legacy.vgui-abi

Each offers the same commands, implemented here:

    record [--update]   derive the table from clang's vtable layout dump
                        (-fdump-vtable-layouts); refuses to replace an
                        existing table without --update, because the table is
                        the frozen contract and changing it is a reviewed ABI
                        decision, never a way to make a check pass
    check               rederive the table and fail on any difference (a
                        second oracle, independent of the suite's compiler)
    sensitivity         per interface, seed (1) a swap of two adjacent virtual
                        declarations and (2) an appended virtual into a private
                        copy of the headers; clang's dump must confirm each
                        seed changes that interface's layout only, and the
                        suite rebuilt against the copy must fail naming that
                        interface and no other. An unmutated copy must pass.

Compiler flags come from the Spec's manifest row and its profile through the
conformance runner's own build function, so no tool holds a second copy.
"""
import argparse
import difflib
import os
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
sys.path.insert(0, os.path.join(ROOT, "tools", "quality"))
import conformance  # noqa: E402  (the shared runner: manifest, profiles, build commands)


class AbiError(Exception):
    pass


class Spec:
    """One recorded table.

    interfaces: [(id, header under public/, version macro or None)] or
    [(id, qualified type, header, macro)]. `id` is an identifier that names
    the interface in the table and in failure lines; `type` is the C++ type
    when it differs (a namespace member). The order is the table's order.
    typed: the table's INTERFACE entries carry the type after the id.
    preamble: the table's leading comment lines.
    extra_includes: headers the layout dump also includes (version macros
    defined outside the declaring header).
    qualify_global: write a declaring class at global scope as '::C', so a
    suite that resolves the table inside a namespace (where clang's unqualified
    parameter types are found) cannot bind it to a namesake there.
    """

    def __init__(self, version, table, suite_id, prefix, probe, pmf, seeded_extra,
                 interfaces, preamble, typed=False, extra_includes=(), qualify_global=False):
        self.version = version
        self.table = table
        self.suite_id = suite_id
        self.prefix = prefix
        self.probe_prefix = probe + "RecordProbe_"
        self.probe_tail = probe + "RecordProbeTail"
        self.pmf = pmf
        self.seeded_extra = seeded_extra
        self.interfaces = [normalize(i) for i in interfaces]
        self.preamble = list(preamble)
        self.typed = typed
        self.extra_includes = list(extra_includes)
        self.qualify_global = qualify_global


def normalize(entry):
    """(id, type, header, macro) from a 3- or 4-tuple."""
    if len(entry) == 3:
        name, header, macro = entry
        return name, name, header, macro
    return tuple(entry)


def bare(type_name):
    """The class name without its namespace qualification."""
    return type_name.split("::")[-1]


# ---------------------------------------------------------------------------
# Build flags (owned by the manifest row)
# ---------------------------------------------------------------------------

def load_suite(spec, root=ROOT):
    # Read without the runner's validation: `record` runs before the table
    # the row names as its contract exists.
    manifest = conformance.load_json(os.path.join(root, "quality", "conformance.manifest.json"))
    suite = next((s for s in manifest["suites"] if s["id"] == spec.suite_id), None)
    if suite is None:
        raise AbiError("manifest has no %s row" % spec.suite_id)
    profiles_dir = os.path.join(root, manifest.get("profiles_dir", "quality/profiles"))
    return suite, conformance.load_profile(profiles_dir, suite["profile"])


def compile_command(root, cxx, source, suite, profile, extra=()):
    """The suite's compile command for `source` (its first unit's dialect and
    flags), without the -c/-o tail."""
    unit = dict(suite["units"][0], sources=[source])
    probe = dict(suite, units=[unit], extra_flags=list(extra) + suite.get("extra_flags", []))
    command = conformance.unit_build_commands(root, cxx, profile, probe, "/dev/null")[0]
    cut = command.index("-c")
    return command[:cut]


# ---------------------------------------------------------------------------
# Deriving the table from clang's vtable layout dump
# ---------------------------------------------------------------------------

def dump_source(spec):
    lines = ["// Generated by tools/quality/abi_table.py for clang -fdump-vtable-layouts."]
    for header in sorted(set(h for _, _, h, _ in spec.interfaces) | set(spec.extra_includes)):
        lines.append('#include "%s"' % header)
    for name, type_name, _, _ in spec.interfaces:
        lines.append("struct %s%s : %s { virtual void %s(); };" % (
            spec.probe_prefix, name, type_name, spec.probe_tail))
        lines.append("void %s%s::%s() {}" % (spec.probe_prefix, name, spec.probe_tail))
    return "\n".join(lines) + "\n"


ENTRY = re.compile(r"^\s*(\d+) \| (.*)$")
ADDRESS = re.compile(r"^\s*-- \((\w+), (-?\d+)\) vtable address --$")
SIGNATURE = re.compile(r"^(?P<prefix>.*?)(?P<cls>[A-Za-z_]\w*)::(?P<name>~?[A-Za-z_]\w*)$")
QUALIFIER = re.compile(r"^(?P<ret>.*?)(?P<scope>(?:[A-Za-z_]\w*::)*)$", re.S)


def split_signature(text):
    """'R N::C::M(P) const [pure]' -> (ret, 'N::C', name, params, quals).
    The declaring class keeps its namespace qualification."""
    text = re.sub(r"\s*\[pure\]$", "", text.strip())
    quals = ""
    for q in (" const",):
        if text.endswith(q):
            quals, text = q.strip(), text[: -len(q)]
    if not text.endswith(")"):
        raise AbiError("unparsed signature: %r" % text)
    depth = 0
    for i in range(len(text) - 1, -1, -1):
        depth += {")": 1, "(": -1}.get(text[i], 0)
        if depth == 0:
            break
    head, params = text[:i], text[i + 1: -1]
    m = SIGNATURE.match(head)
    if not m:
        raise AbiError("unparsed signature head: %r" % head)
    scoped = QUALIFIER.match(m.group("prefix").strip())
    # A va_list parameter decays to the builtin tag pointer; spell it portably.
    params = params.replace("__va_list_tag *", "va_list")
    return (scoped.group("ret").strip(), scoped.group("scope") + m.group("cls"), m.group("name"),
            params, quals)


def parse_layouts(dump):
    """{probe class: [section, ...]} where a section is
    {"base": name, "offset": int, "entries": [(index, text), ...]}."""
    layouts, sections, current = {}, None, None
    for line in dump.splitlines():
        head = re.match(r"^Vtable for '(\w+)' \(\d+ entries\)\.$", line)
        if head:
            sections = layouts.setdefault(head.group(1), [])
            current = None
            continue
        if sections is None:
            continue
        if not line.strip():
            sections = None
            continue
        m = ENTRY.match(line)
        if m:
            index, text = int(m.group(1)), m.group(2)
            if text.startswith("offset_to_top"):
                current = {"base": None, "offset": None, "start": index + 2, "entries": []}
                sections.append(current)
            elif text.endswith(" RTTI"):
                pass
            elif text.startswith(("vcall_offset", "vbase_offset")):
                raise AbiError("virtual inheritance is not modelled: %s" % text)
            else:
                current["entries"].append((index - current["start"], text))
            continue
        m = ADDRESS.match(line)
        if m and current is not None and current["base"] is None:
            current["base"], current["offset"] = m.group(1), int(m.group(2))
    return layouts


def table_from_layouts(spec, layouts, versions):
    """The recorded table model: one record per interface."""
    table = []
    for name, type_name, header, macro in spec.interfaces:
        probe = spec.probe_prefix + name
        if probe not in layouts:
            raise AbiError("no vtable layout for %s" % probe)
        sections = layouts[probe]
        record = {"name": name, "type": type_name, "header": "public/" + header, "macro": macro,
                  "version": versions.get(macro) if macro else None,
                  "slots": None, "methods": [], "destructors": [], "secondary": []}
        for number, section in enumerate(sections):
            primary = number == 0
            if not primary:
                if section["offset"] is None or section["offset"] <= 0:
                    raise AbiError("%s: secondary vtable without an offset" % name)
                record["secondary"].append({"base": section["base"], "offset": section["offset"],
                                            "slots": len(section["entries"])})
            for slot, text in section["entries"]:
                if spec.probe_tail in text:
                    if not primary:
                        raise AbiError("%s: probe tail in a secondary vtable" % name)
                    record["slots"] = slot
                    continue
                if "::~" in text:
                    kind = re.search(r"\[(complete|deleting)\]", text)
                    if not kind:
                        raise AbiError("%s: unparsed destructor %r" % (name, text))
                    record["destructors"].append({"adj": 0 if primary else section["offset"],
                                                  "slot": slot, "kind": kind.group(1)})
                    continue
                ret, cls, method, params, quals = split_signature(text)
                if spec.qualify_global and "::" not in cls:
                    cls = "::" + cls
                record["methods"].append({
                    "adj": 0 if primary else section["offset"], "slot": slot,
                    "ret": ret, "cls": cls, "name": method, "params": params, "quals": quals,
                    "signature": re.sub(r"\s*\[pure\]$", "", text.strip()).replace(
                        "__va_list_tag *", "va_list")})
        if record["slots"] is None:
            raise AbiError("%s: probe tail slot not found" % name)
        if macro and record["version"] is None:
            raise AbiError("%s: version macro %s not defined" % (name, macro))
        table.append(record)
    return table


def c_string(text):
    return '"%s"' % text.replace("\\", "\\\\").replace('"', '\\"')


def pmf_type(method, declarator=""):
    """'R ( C::*declarator )( params ) quals' for a recorded method."""
    ret = method["ret"]
    # 'ITexture *' spells as 'ITexture *( C::* )( ... )'.
    spacer = "" if ret.endswith(("*", "&")) else " "
    params = ( " %s " % method["params"]) if method["params"] else ""
    quals = (" " + method["quals"]) if method["quals"] else ""
    return "%s%s( %s::*%s )(%s)%s" % (ret, spacer, method["cls"], declarator, params, quals)


def render_table(spec, table):
    p = spec.prefix
    out = list(spec.preamble) + [
        "",
        "#define %s_TABLE_VERSION %s" % (p, c_string(spec.version)),
        "",
        "// clang-format off",
        "",
    ]
    for record in table:
        name = record["name"]
        out.append("// %s (%s): %d primary slots, %d methods, %d secondary vtable(s)" % (
            name, record["header"], record["slots"], len(record["methods"]), len(record["secondary"])))
        typed = ", %s" % record["type"] if spec.typed else ""
        out.append("%s_INTERFACE( %s%s, %s, %d )" % (p, name, typed, c_string(record["header"]),
                                                     record["slots"]))
        if record["macro"]:
            out.append("%s_VERSION( %s, %s, %s )" % (p, name, record["macro"], record["version"]))
        for sec in record["secondary"]:
            out.append("%s_SECONDARY( %s, %s, %d, %d )" % (p, name, sec["base"], sec["offset"], sec["slots"]))
        rows = [("m", m["adj"], m["slot"], m) for m in record["methods"]] + \
               [("d", d["adj"], d["slot"], d) for d in record["destructors"]]
        for kind, adj, slot, item in sorted(rows, key=lambda r: (r[1], r[2])):
            if kind == "d":
                out.append("%s_DESTRUCTOR( %s, %d, %d, %s )" % (p, name, adj, slot, c_string(item["kind"])))
            else:
                out.append("%s_METHOD( %s, %d, %d, %s,\n\t%s, %s, %s )" % (
                    p, name, adj, slot, c_string(item["signature"]), item["cls"], item["name"],
                    pmf_type(item, spec.pmf)))
        out.append("%s_END( %s )" % (p, name))
        out.append("")
    out.append("// clang-format on")
    return "\n".join(out) + "\n"


def derive_table(spec, root=ROOT, clang="clang++", include_root=None):
    """The table model from clang's dump of the headers (under `include_root`,
    searched first, when given)."""
    suite, profile = load_suite(spec, root)
    with tempfile.TemporaryDirectory(prefix="abi_table_") as tmp:
        source = os.path.join(tmp, "abi_table_dump.cpp")
        with open(source, "w") as stream:
            stream.write(dump_source(spec))
        extra = ["-isystem", include_root] if include_root else []
        base = compile_command(root, clang, source, suite, profile, extra)
        base = [a for a in base if a != "-Werror"]
        layout = subprocess.run(base + ["-S", "-o", os.devnull, "-Xclang", "-fdump-vtable-layouts", source],
                                cwd=root, capture_output=True, text=True)
        if layout.returncode != 0:
            raise AbiError("clang layout dump failed:\n" + layout.stderr[-4000:])
        macros = subprocess.run(base + ["-E", "-dM", source], cwd=root, capture_output=True, text=True)
        if macros.returncode != 0:
            raise AbiError("clang macro dump failed:\n" + macros.stderr[-4000:])
        versions = {}
        for line in macros.stdout.splitlines():
            m = re.match(r'^#define (\w+) ("[^"]*")$', line)
            if m:
                versions[m.group(1)] = m.group(2)
    return table_from_layouts(spec, parse_layouts(layout.stdout), versions)


# ---------------------------------------------------------------------------
# Seeded slot reorders
# ---------------------------------------------------------------------------

def _mask_comments(text):
    """Same-length copy of `text` with comments and literals blanked, so
    structure can be scanned by index."""
    out, i, n = list(text), 0, len(text)
    while i < n:
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
        elif text[i] in "\"'":
            quote, j = text[i], i + 1
            while j < n and text[j] != quote:
                j += 2 if text[j] == "\\" else 1
            j += 1
        else:
            i += 1
            continue
        for k in range(i, j):
            if out[k] != "\n":
                out[k] = " "
        i = j
    return "".join(out)


def class_body(text, name):
    """(start, end) of the body between the braces of class `name`."""
    masked = _mask_comments(text)
    m = re.search(r"\b(?:abstract_class|class|struct)\s+%s\b[^;{]*\{" % re.escape(name), masked)
    if not m:
        raise AbiError("class %s not found" % name)
    depth, start = 1, m.end()
    for i in range(start, len(masked)):
        depth += {"{": 1, "}": -1}.get(masked[i], 0)
        if depth == 0:
            return start, i
    raise AbiError("class %s is not closed" % name)


def member_declarations(text, name):
    """[(start, end, masked text)] of the member statements of class `name`,
    each spanning its first significant character through its ';' (or the
    closing brace of an inline body). Preprocessor lines and access
    specifiers are statements of their own, so they separate neighbours."""
    start, end = class_body(text, name)
    masked = _mask_comments(text)
    statements, depth, paren, first, i = [], 0, 0, None, start
    while i < end:
        c = masked[i]
        if first is None and c == "#":
            j = i
            while True:
                j = masked.find("\n", j)
                j = end if j < 0 or j > end else j
                if j >= end or masked[j - 1] != "\\":
                    break
                j += 1
            statements.append((i, j, masked[i:j]))
            i = j
            continue
        if first is None and not c.isspace():
            first = i
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0 and paren == 0 and first is not None:
                statements.append((first, i + 1, masked[first:i + 1]))
                first = None
        elif c == "(":
            paren += 1
        elif c == ")":
            paren -= 1
        elif c == ";" and depth == 0 and paren == 0 and first is not None:
            statements.append((first, i + 1, masked[first:i + 1]))
            first = None
        elif c == ":" and depth == 0 and paren == 0 and first is not None and \
                re.fullmatch(r"(public|protected|private)\s*", masked[first:i]):
            statements.append((first, i + 1, masked[first:i + 1]))
            first = None
        i += 1
    return statements


def is_plain_virtual(statement):
    body = statement.strip()
    return (body.startswith("virtual ") and "~" not in body and "{" not in body
            and "#" not in body and "\n\n" not in body)


def seed_swaps(text, name):
    """Candidate mutants of `text`: each swaps one pair of adjacent plain
    virtual declarations of class `name`, in declaration order; yields
    (mutated text, first, second). Swapping two overriders of base virtuals
    leaves the vtable unchanged, so callers confirm each candidate is a real
    reorder before using it."""
    statements = member_declarations(text, name)
    for (a0, a1, am), (b0, b1, bm) in zip(statements, statements[1:]):
        if not (is_plain_virtual(am) and is_plain_virtual(bm)):
            continue
        if _mask_comments(text[a1:b0]).strip():
            continue
        first, second = text[a0:a1], text[b0:b1]
        mutated = text[:a0] + second + text[a1:b0] + first + text[b1:]
        yield mutated, " ".join(first.split()), " ".join(second.split())


def seed_append(text, name, added_name):
    """`text` with a new pure virtual after the last plain virtual declaration
    of class `name`: a method the recorded table does not list."""
    last = [end for _, end, member in member_declarations(text, name) if is_plain_virtual(member)]
    if not last:
        raise AbiError("%s: no virtual declaration to append after" % name)
    added = "virtual void %s() = 0;" % added_name
    return text[:last[-1]] + "\n\t" + added + text[last[-1]:], added


def mirror_headers(spec, root, dest):
    """Private include root holding copies of every header the table names,
    laid out as under public/ (whole directories, so relative includes stay
    inside the copy)."""
    for header in sorted(set(h for _, _, h, _ in spec.interfaces) | set(spec.extra_includes)):
        directory = os.path.dirname(header)
        if directory:
            target = os.path.join(dest, directory)
            if not os.path.isdir(target):
                shutil.copytree(os.path.join(root, "public", directory), target)
        else:
            shutil.copy2(os.path.join(root, "public", header), os.path.join(dest, header))


def build_and_run(root, cxx, suite, profile, work, include_root):
    """Build the suite with `include_root` searched first; returns
    (built, exit status, output)."""
    patched = dict(suite, extra_flags=["-isystem", include_root] + list(suite.get("extra_flags", [])))
    out_bin = os.path.join(work, "abi_table_suite")
    for command in conformance.unit_build_commands(root, cxx, profile, patched, out_bin):
        result = subprocess.run(command, cwd=root, capture_output=True, text=True)
        if result.returncode != 0:
            return False, result.returncode, result.stdout + result.stderr
    run = subprocess.run([out_bin], cwd=root, capture_output=True, text=True, timeout=120)
    return True, run.returncode, run.stdout + run.stderr


def inheritors(spec, table):
    """{interface: the interfaces whose vtable holds its declarations}: itself,
    and each table interface derived from it (a record with a method its type
    declares). A change to a base's declarations changes exactly these."""
    def plain(type_name):
        return type_name.lstrip(":")
    declared = {r["name"]: set(plain(m["cls"]) for m in r["methods"]) for r in table}
    return {name: {name} | set(other for other, classes in declared.items() if plain(type_name) in classes)
            for name, type_name, _, _ in spec.interfaces}


def sensitivity(spec, root, cxx, clang, out_dir, derive=None):
    """Per interface: a slot reorder (the first adjacent swap clang confirms
    changes that interface's layout) and an appended virtual. Each must be a
    real layout change of exactly that interface and the table interfaces
    derived from it, and the suite must fail naming that interface and no
    interface outside that set."""
    derive = derive or (lambda include_root: derive_table(spec, root, clang, include_root))
    suite, profile = load_suite(spec, root)
    checks = failures = 0

    def check(ok, label):
        nonlocal checks, failures
        checks += 1
        if not ok:
            failures += 1
        print("%s %s" % ("PASS" if ok else "FAIL", label))

    def seeded(work, recorded, affected, name, type_name, header, label, mutants):
        include_root = os.path.join(work, "%s.%s" % (name, label.split()[0]))
        os.makedirs(include_root)
        mirror_headers(spec, root, include_root)
        path = os.path.join(include_root, header)
        with open(path, "r", encoding="utf-8", errors="surrogateescape", newline="") as stream:
            text = stream.read()
        chosen, neutral = None, 0
        for mutated, description in mutants(text, bare(type_name)):
            with open(path, "w", encoding="utf-8", errors="surrogateescape", newline="") as stream:
                stream.write(mutated)
            derived = table_blocks(render_table(spec, derive(include_root)))
            changed = [n for n in recorded if recorded[n] != derived.get(n)]
            if changed:
                chosen = (description, changed)
                break
            neutral += 1
        expected = affected[name]
        check(chosen is not None and set(chosen[1]) == expected,
              "%s: seeded %s changes the layout of this interface and its derived "
              "interfaces only (%s; expected %s; %d ABI-neutral candidate(s) skipped)" % (
                  name, label, ", ".join(chosen[1]) if chosen else "none",
                  ", ".join(sorted(expected)), neutral))
        if chosen is None:
            return
        built, status, output = build_and_run(root, cxx, suite, profile, work, include_root)
        named = re.search(r"^FAIL %s\b" % re.escape(name), output, re.M) is not None
        check(built and status != 0 and named, "%s: %s %s detected" % (name, label, chosen[0]))
        if not (built and status != 0 and named):
            print("\n".join(output.splitlines()[-15:]))
        others = set(re.findall(r"^FAIL (\w+)", output, re.M)) - expected
        check(built and not others, "%s: %s: no interface outside it and its derived ones "
              "reported (%s)" % (name, label, ", ".join(sorted(others)) or "none"))

    def swaps(text, cls):
        for mutated, first, second in seed_swaps(text, cls):
            yield mutated, "[%s] <-> [%s]" % (first[:60], second[:60])

    def appends(text, cls):
        mutated, added = seed_append(text, cls, spec.seeded_extra)
        yield mutated, "[%s]" % added

    os.makedirs(out_dir, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="abi_table_sens_", dir=out_dir) as work:
        control = os.path.join(work, "control")
        os.makedirs(control)
        mirror_headers(spec, root, control)
        built, status, output = build_and_run(root, cxx, suite, profile, work, control)
        check(built and status == 0 and re.search(r"^CONFORMANCE \d+ 0$", output, re.M) is not None,
              "control: unmutated header copy passes")
        if not (built and status == 0):
            print("\n".join(output.splitlines()[-15:]))
        table = derive(control)
        recorded = table_blocks(render_table(spec, table))
        affected = inheritors(spec, table)
        for name, type_name, header, _ in spec.interfaces:
            seeded(work, recorded, affected, name, type_name, header, "slot reorder", swaps)
            seeded(work, recorded, affected, name, type_name, header, "appended virtual", appends)
    return testing_record(checks, failures)


def table_blocks(text):
    """{interface: its block of the rendered table}, plus the preamble."""
    blocks, name = {"(preamble)": []}, "(preamble)"
    for line in text.splitlines(True):
        m = re.match(r"^// (\w+) \(public/", line)
        if m:
            name = m.group(1)
            blocks[name] = []
        blocks[name].append(line)
    return blocks


def compare_tables(spec, recorded, derived):
    """One check per interface block (and the preamble); prints a bounded diff
    of each differing block and a checks-v1 record."""
    old, new = table_blocks(recorded), table_blocks(derived)
    checks = failures = 0
    for name in ["(preamble)"] + [n for n, _, _, _ in spec.interfaces]:
        checks += 1
        if old.get(name) == new.get(name):
            print("PASS %s matches the current headers" % name)
            continue
        failures += 1
        print("FAIL %s differs from the current headers" % name)
        sys.stdout.writelines(list(difflib.unified_diff(
            old.get(name, []), new.get(name, []), spec.table, "derived"))[:40])
    return testing_record(checks, failures)


def testing_record(checks, failures):
    print("CONFORMANCE %d %d" % (checks, failures))
    sys.stdout.flush()
    return 0 if checks > 0 and failures == 0 else 1


# ---------------------------------------------------------------------------
# Command line
# ---------------------------------------------------------------------------

def main(spec, argv=None, description=None, root=None, derive=None, sense=None):
    """The tools' shared command line. `root`, `derive(root, clang)` and
    `sense(root, cxx, clang, out)` default to this module's; a tool passes its
    own so its tests can replace them."""
    root = root or ROOT
    derive = derive or (lambda r, clang: derive_table(spec, r, clang))
    sense = sense or (lambda r, cxx, clang, out: sensitivity(spec, r, cxx, clang, out))
    parser = argparse.ArgumentParser(description=description)
    sub = parser.add_subparsers(dest="command", required=True)
    rec = sub.add_parser("record", help="derive the table and write it")
    rec.add_argument("--update", action="store_true", help="replace an existing table (reviewed ABI change)")
    rec.add_argument("--clang", default="clang++")
    chk = sub.add_parser("check", help="rederive the table and compare with the recorded one")
    chk.add_argument("--clang", default="clang++")
    sen = sub.add_parser("sensitivity", help="seeded slot reorders and appended virtuals per interface")
    sen.add_argument("--cxx", default=os.environ.get("CXX", "g++"))
    sen.add_argument("--clang", default="clang++", help="confirms each seeded swap is a real reorder")
    sen.add_argument("--out", default=os.environ.get("CONFORMANCE_OUT") or tempfile.gettempdir())
    args = parser.parse_args(argv)

    try:
        if args.command == "sensitivity":
            return sense(root, args.cxx, args.clang, args.out)
        text = render_table(spec, derive(root, args.clang))
        path = os.path.join(root, spec.table)
        if args.command == "record":
            if os.path.exists(path) and not args.update:
                print("%s exists; the recorded table is the frozen contract. Use `check`, "
                      "or --update for a reviewed ABI change." % spec.table, file=sys.stderr)
                return 2
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "w") as stream:
                stream.write(text)
            print("recorded %s" % spec.table)
            return 0
        with open(path) as stream:
            recorded = stream.read()
        return compare_tables(spec, recorded, text)
    except AbiError as error:
        print("error: %s" % error, file=sys.stderr)
        return 2
