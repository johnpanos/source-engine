#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Game collision-filter audit (RFC 0013 P3, R67).
#
# With vphysics.parallel-step.v1, Box3D calls the provider's custom filter
# from worker threads, and the filter calls the game's
# IPhysicsCollisionSolver::ShouldCollide. The provider serializes those calls
# (no two run at once, and the thread that called Simulate runs no game code
# meanwhile), but the game code now runs off the main thread. This check
# decides whether that is acceptable, and keeps deciding it as game code
# changes:
#
#   1. It finds every definition the game's filter can reach: the solver
#      roots and the virtual families they dispatch to (game rules
#      ShouldCollide, ForceVPhysicsCollide, PhysicsSolidMaskForEntity, ...).
#   2. It scans each body for constructs that are unsafe or suspect off the
#      main thread: thread identity, engine and sound calls, entity or physics
#      mutation, random numbers, prediction state, lazy transform caches,
#      member writes, statics, allocation and console output.
#   3. It compares the result with the reviewed declaration in
#      quality/physics_filter_audit.json. Every site must be declared, and its
#      body digest must match the reviewed one. Every finding must be accepted
#      with a reason, and forbidden rule classes cannot be accepted at all.
#
# New, changed, stale or unreviewed sites fail. `scaffold` prints stub
# entries to review; nothing here rewrites the declaration.
#
# Dependency-free: Python 3 standard library only.
#
# ============================================================================

import argparse
import hashlib
import json
import os
import re
import sys

SCHEMA = "physics-filter-audit/v1"
DEFAULT_DECLARATION = os.path.join("quality", "physics_filter_audit.json")

# Rule id -> (pattern, description). Patterns run on bodies with comments and
# string literals removed.
RULES = {
    "thread-identity": (r"\b(ThreadInMainThread|ThreadGetCurrentId|IsMainThread|ThreadIsInMainThread)\b",
                        "depends on which thread runs it"),
    "engine-call": (r"\bengine->|\benginesound\b|\bg_pSoundEmitter\b|\bEmitSound\s*\(|\bUTIL_Remove\w*\s*\(|"
                    r"\bDispatchEffect\s*\(|\bte->|\bUTIL_ScreenShake\s*\(",
                    "calls engine, sound or effect services"),
    "entity-mutation": (r"\bSet(Abs|Local)\w*\s*\(|\bTeleport\s*\(|\bSetCollisionGroup\s*\(|\bCollisionRulesChanged\s*\(|"
                        r"\bSetSolid(Flags)?\s*\(|\b(Add|Remove)SolidFlags\s*\(|\bSetMoveType\s*\(|"
                        r"\b(Add|Remove)Effects\s*\(|\bSetOwnerEntity\s*\(",
                        "changes entity state"),
    "physics-mutation": (r"\bEnableCollisions\s*\(|\bEnableMotion\s*\(|\bWake\s*\(|\bSleep\s*\(|\bRecheckCollisionFilter\s*\(|"
                         r"\bSetGameFlags\s*\(|\bSetCallbackFlags\s*\(|\bAddObjectPair\s*\(|\bRemoveObjectPair\s*\(|"
                         r"\bRemoveAllPairsForObject\s*\(",
                         "changes physics state from inside the step"),
    "random": (r"\bRandom(Int|Float|Vector|Angle)\s*\(|\brandom->|\bSharedRandom\w*\s*\(",
               "consumes random numbers, whose order must be preserved"),
    "prediction-state": (r"\bGetPredictionPlayer\s*\(|\bIsInPrediction\s*\(|\bprediction->|\bGetPredictionRandomSeed\s*\(|"
                         r"\bIsPredicted\s*\(",
                         "reads per-thread prediction globals"),
    "lazy-transform": (r"\bGetAbs(Origin|Angles|Velocity)\s*\(|\bEntityToWorldTransform\s*\(|\bCalcAbsolute\w*\s*\(",
                       "may recompute and write an entity's cached transform"),
    "member-write": (r"(?<![=!<>])\bm_\w+(\.\w+|->\w+|\[[^\]]*\])*\s*(=(?!=)|\+\+|--|\+=|-=|\|=|&=|\^=)|(\+\+|--)\s*m_\w+",
                     "writes a member"),
    "static-local": (r"(^|[;{}]\s*)static\s+(?!_cast)", "function-local static"),
    "allocation": (r"\bnew\b|\bdelete\b|\bmalloc\s*\(|\bfree\s*\(", "allocates or frees"),
    "console-output": (r"\b(Msg|DevMsg|Warning|DevWarning|ConMsg|ConDMsg|Log)\s*\(", "writes console output"),
    "convar-write": (r"\.SetValue\s*\(", "changes a console variable"),
    "callback-context": (r"\bCallbackContext\b", "counts callback depth in the solver's own member"),
}

# Rule classes no reviewed reason can accept under the serialized-worker
# policy: they need a redesign (RFC 0013 policy options b or c) first.
FORBIDDEN = {"thread-identity", "engine-call", "entity-mutation", "physics-mutation", "random", "prediction-state",
             "convar-write"}

# What the filter can reach. Roots are the installed solvers; families are
# the virtual (or overloaded) calls those roots make, found in every game.
ROOTS = [
    {"file": "game/server/physics.cpp", "name": "ShouldCollide", "qualifier": "CCollisionEvent"},
    {"file": "game/server/physics.cpp", "name": "ShouldCollide_2", "qualifier": "CCollisionEvent"},
    {"file": "game/server/physics.cpp", "name": "WheelCollidesWith", "qualifier": None},
    {"file": "game/client/physics.cpp", "name": "ShouldCollide", "qualifier": "CCollisionEvent"},
    {"file": "game/client/physics.cpp", "name": "ShouldCollide_2", "qualifier": "CCollisionEvent"},
]
ENTITY_HEADERS = ["game/server/baseentity.h", "game/client/c_baseentity.h", "game/shared/collisionproperty.h",
                  "game/shared/baseentity_shared.cpp", "game/shared/baseentity_shared.h"]
FAMILIES = [
    # CGameRules::ShouldCollide( int, int ) and every override in a game
    # rules class (CBaseEntity::ShouldCollide( group, mask ) is a trace
    # filter the physics filter never calls).
    {"name": "ShouldCollide", "signature": r"\(\s*int\s+\w+\s*,\s*int\s+\w+\s*\)", "derived_from": "CGameRules"},
    {"name": "ForceVPhysicsCollide"},
    {"name": "PhysicsSolidMaskForEntity"},
    # Non-virtual entity accessors the filters call.
    {"name": "GetRootMoveParent", "files": ENTITY_HEADERS},
    {"name": "GetOwnerEntity", "files": ENTITY_HEADERS},
    {"name": "GetMoveParent", "files": ENTITY_HEADERS},
    {"name": "GetSolid", "files": ENTITY_HEADERS},
    {"name": "GetSolidFlags", "files": ENTITY_HEADERS},
    {"name": "GetMoveType", "files": ENTITY_HEADERS},
    {"name": "GetCollisionGroup", "files": ENTITY_HEADERS},
    {"name": "GetModelIndex", "files": ENTITY_HEADERS},
    {"name": "VPhysicsGetObject", "files": ENTITY_HEADERS},
    {"name": "edict", "files": ENTITY_HEADERS},
]
SCAN_DIRS = ["game"]
SOURCE_EXTENSIONS = (".cpp", ".h")


class AuditError(Exception):
    pass


# ---------------------------------------------------------------------------
# Lexing
# ---------------------------------------------------------------------------

def strip_comments_and_strings(text):
    """Blank comments and string/char literals, keeping offsets and newlines."""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append(re.sub(r"[^\n]", " ", text[i:j]))
            i = j
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
            out.append(c + " " * (j - i - 2) + c if j - i >= 2 else c)
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def match_brace(code, open_index):
    depth = 0
    for i in range(open_index, len(code)):
        if code[i] == "{":
            depth += 1
        elif code[i] == "}":
            depth -= 1
            if depth == 0:
                return i
    raise AuditError("unbalanced braces")


def find_definitions(code, name, qualifier=None, signature=None):
    """(start, body_open, body_close, class) of each definition of name in code.

    A definition is `[Qualifier::]name( ... ) [const] [override] {`, possibly
    with preprocessor lines between the signature and the brace. Calls and
    declarations (ending in ';') are skipped.
    """
    prefix = (r"(" + re.escape(qualifier) + r")\s*::\s*") if qualifier else r"(?:\b(\w+)\s*::\s*)?"
    pattern = re.compile(r"(?<![\w.>:])" + prefix + r"\b" + re.escape(name) + r"\s*\(")
    results = []
    for match in pattern.finditer(code):
        if qualifier is None and re.search(r"(->|\.)\s*$", code[max(0, match.start() - 3):match.start()]):
            continue
        # Balance the parameter list.
        depth, i = 0, match.end() - 1
        while i < len(code):
            if code[i] == "(":
                depth += 1
            elif code[i] == ")":
                depth -= 1
                if depth == 0:
                    break
            i += 1
        params = code[match.end() - 1:i + 1]
        if signature and not re.fullmatch(signature, params):
            continue
        # After the parameters: qualifiers, then '{' (a definition) or anything else.
        j = i + 1
        tail = re.match(r"(\s|const\b|override\b|final\b|OVERRIDE\b|#[^\n]*\n)*", code[j:])
        j += tail.end()
        if j < len(code) and code[j] == "{":
            # Reject call sites in expressions such as `if ( x->Name() ) {`.
            before = code[max(0, match.start() - 200):match.start()]
            statement_start = max(before.rfind(";"), before.rfind("{"), before.rfind("}"))
            lead = before[statement_start + 1:]
            if re.search(r"\b(if|while|for|switch|return)\b\s*\(?[^;]*$", lead):
                continue
            results.append((match.start(), j, match_brace(code, j), match.group(1)))
    return results


def line_of(text, index):
    return text.count("\n", 0, index) + 1


# ---------------------------------------------------------------------------
# Inventory
# ---------------------------------------------------------------------------

def site_key(path, name, qualifier, ordinal):
    return "%s::%s%s#%d" % (path, (qualifier + "::") if qualifier else "", name, ordinal)


def scan_body(body):
    findings = []
    for rule, (pattern, _) in RULES.items():
        for match in re.finditer(pattern, body, re.M):
            findings.append((rule, match.start()))
    return findings


def collect(root):
    """Every reachable site: {key: {file, line, name, digest, findings}}."""
    sites = {}
    files = []
    for directory in SCAN_DIRS:
        for base, _, names in os.walk(os.path.join(root, directory)):
            for name in names:
                if name.endswith(SOURCE_EXTENSIONS):
                    files.append(os.path.join(base, name))
    files.sort()

    def add(path, text, code, name, qualifier, signature, required, classes=None):
        definitions = find_definitions(code, name, qualifier, signature)
        if classes is not None:
            definitions = [d for d in definitions if d[3] in classes]
        if required and not definitions:
            raise AuditError("root %s%s not found in %s" % ((qualifier + "::") if qualifier else "", name, path))
        relative = os.path.relpath(path, root).replace(os.sep, "/")
        for ordinal, (start, body_open, body_close, _) in enumerate(definitions):
            body = code[body_open:body_close + 1]
            raw = text[body_open:body_close + 1]
            key = site_key(relative, name, qualifier, ordinal)
            findings = {}
            for rule, offset in scan_body(body):
                findings.setdefault(rule, []).append(line_of(text, body_open + offset))
            sites[key] = {
                "file": relative, "line": line_of(text, start), "name": name,
                "digest": hashlib.sha256(re.sub(r"\s+", " ", raw).strip().encode()).hexdigest()[:16],
                "findings": {rule: sorted(set(lines)) for rule, lines in findings.items()},
            }

    cache = {}

    def load(path):
        if path not in cache:
            with open(path, encoding="utf-8", errors="replace") as stream:
                text = stream.read()
            cache[path] = (text, strip_comments_and_strings(text))
        return cache[path]

    for spec in ROOTS:
        path = os.path.join(root, spec["file"])
        if not os.path.isfile(path):
            raise AuditError("root file missing: %s" % spec["file"])
        text, code = load(path)
        add(path, text, code, spec["name"], spec["qualifier"], None, True)
    # Lexical inheritance: class -> bases, for families limited to a hierarchy.
    bases = {}
    for path in files:
        _, code = load(path)
        for match in re.finditer(r"\b(?:class|struct)\s+(?:\w+\s+)?(\w+)\s*(?:final\s*)?:\s*([^{;]+)\{", code):
            names = re.findall(r"(?:public|protected|private)?\s*(\w+)\s*(?:,|$)", match.group(2).strip())
            bases.setdefault(match.group(1), set()).update(names)

    def derived(root_class):
        found, changed = {root_class}, True
        while changed:
            changed = False
            for cls, parents in bases.items():
                if cls not in found and parents & found:
                    found.add(cls)
                    changed = True
        return found

    for family in FAMILIES:
        classes = derived(family["derived_from"]) if family.get("derived_from") else None
        candidates = [os.path.join(root, f) for f in family["files"]] if family.get("files") else files
        for path in candidates:
            if not os.path.isfile(path):
                raise AuditError("family %s: file missing: %s" % (family["name"], os.path.relpath(path, root)))
            text, code = load(path)
            if family["name"] not in code:
                continue
            relative = os.path.relpath(path, root).replace(os.sep, "/")
            if any(r["file"] == relative and r["name"] == family["name"] for r in ROOTS):
                continue
            add(path, text, code, family["name"], None, family.get("signature"), False, classes)
    return sites


# ---------------------------------------------------------------------------
# Comparison with the reviewed declaration
# ---------------------------------------------------------------------------

def load_declaration(path):
    with open(path) as stream:
        declaration = json.load(stream)
    if declaration.get("schema") != SCHEMA:
        raise AuditError("%s: schema %r is not %s" % (path, declaration.get("schema"), SCHEMA))
    for key, site in declaration.get("sites", {}).items():
        for field in ("digest", "verdict"):
            if field not in site:
                raise AuditError("site %s: missing %s" % (key, field))
        if site["verdict"] not in ("safe", "accepted"):
            raise AuditError("site %s: verdict must be safe or accepted" % key)
        accepted = site.get("accepted", {})
        for rule, reason in accepted.items():
            if rule not in RULES:
                raise AuditError("site %s: unknown rule %s" % (key, rule))
            if rule in FORBIDDEN:
                raise AuditError("site %s: rule %s is forbidden under the serialized-worker policy" % (key, rule))
            if not reason or not isinstance(reason, str):
                raise AuditError("site %s: rule %s needs a reason" % (key, rule))
        if site["verdict"] == "accepted" and not accepted:
            raise AuditError("site %s: an accepted verdict needs accepted findings" % key)
    return declaration


def compare(sites, declaration):
    problems = []
    declared = declaration.get("sites", {})
    for key in sorted(sites):
        site = sites[key]
        where = "%s:%d" % (site["file"], site["line"])
        forbidden = sorted(set(site["findings"]) & FORBIDDEN)
        if forbidden:
            problems.append(("forbidden", key, "%s uses %s (%s)" % (where, ", ".join(forbidden), "; ".join(
                "%s at lines %s" % (r, ",".join(map(str, site["findings"][r]))) for r in forbidden))))
        entry = declared.get(key)
        if entry is None:
            problems.append(("unreviewed", key, "%s is not in the declaration" % where))
            continue
        if entry["digest"] != site["digest"]:
            problems.append(("changed", key, "%s changed since review (digest %s, reviewed %s)" % (
                where, site["digest"], entry["digest"])))
        unaccepted = sorted(set(site["findings"]) - set(entry.get("accepted", {})) - FORBIDDEN)
        if unaccepted:
            problems.append(("unaccepted", key, "%s has findings without a reviewed reason: %s" % (
                where, ", ".join("%s at lines %s" % (r, ",".join(map(str, site["findings"][r]))) for r in unaccepted))))
        if entry["verdict"] == "safe" and site["findings"]:
            problems.append(("unaccepted", key, "%s is declared safe but has findings" % where))
        stale_accepts = sorted(set(entry.get("accepted", {})) - set(site["findings"]))
        if stale_accepts:
            problems.append(("stale", key, "%s accepts findings that no longer occur: %s" % (where, ", ".join(stale_accepts))))
    for key in sorted(set(declared) - set(sites)):
        problems.append(("stale", key, "declared site no longer exists"))
    return problems


def scaffold(sites, declaration):
    """Stub entries for sites that are new or changed; never written."""
    declared = declaration.get("sites", {}) if declaration else {}
    stubs = {}
    for key, site in sorted(sites.items()):
        entry = declared.get(key)
        if entry and entry["digest"] == site["digest"]:
            continue
        stubs[key] = {
            "digest": site["digest"],
            "verdict": "accepted" if site["findings"] else "safe",
            "accepted": {rule: "REVIEW: %s (lines %s)" % (RULES[rule][1], ",".join(map(str, lines)))
                         for rule, lines in site["findings"].items() if rule not in FORBIDDEN},
        }
        if not stubs[key]["accepted"]:
            del stubs[key]["accepted"]
    return stubs


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", nargs="?", default="check", choices=["check", "inventory", "scaffold"])
    parser.add_argument("--root", default=os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..")))
    parser.add_argument("--declaration", default=None)
    args = parser.parse_args(argv)

    root = os.path.abspath(args.root)
    declaration_path = args.declaration or os.path.join(root, DEFAULT_DECLARATION)
    try:
        sites = collect(root)
        if args.command == "inventory":
            for key, site in sorted(sites.items()):
                print("%-80s %s %s" % (key, site["digest"], " ".join(
                    "%s@%s" % (r, ",".join(map(str, l))) for r, l in sorted(site["findings"].items()))))
            print("physics filter audit: %d reachable sites" % len(sites))
            return 0
        declaration = load_declaration(declaration_path) if os.path.isfile(declaration_path) else None
        if args.command == "scaffold":
            print(json.dumps(scaffold(sites, declaration), indent=2))
            return 0
        if declaration is None:
            raise AuditError("declaration missing: %s" % declaration_path)
        problems = compare(sites, declaration)
    except AuditError as error:
        print("physics filter audit: error: %s" % error)
        return 2
    for kind, key, detail in problems:
        print("physics filter audit: %s: %s: %s" % (kind, key, detail))
    print("physics filter audit: %d sites, %d problems" % (len(sites), len(problems)))
    return 0 if not problems else 1


if __name__ == "__main__":
    sys.exit(main())
