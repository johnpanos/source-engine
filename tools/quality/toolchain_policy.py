#!/usr/bin/env python3
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Per-target language-dialect policy (RFC 0006 M0 / roadmap R03).
#
# Pure functions over quality/toolchain/policy.json, shared by the Waf tool
# (scripts/waifulib/toolchain_dialect.py), the conformance runner and the
# boundary verifier (tools/quality/toolchain_boundary.py) so the dialect rules
# have one implementation. Python 3 standard library only.
#
# ============================================================================

import json
import os

POLICY_SCHEMA = "toolchain-policy/v1"
POLICY_PATH = os.path.join("quality", "toolchain", "policy.json")
VALID_MODES = {"apply", "verify", "unconstrained"}
VALID_LANGUAGES = {"c", "c++"}


class PolicyError(Exception):
    """The policy file is malformed or cannot serve a request. Always fatal."""


def load_policy(root):
    path = os.path.join(root, POLICY_PATH)
    try:
        with open(path, "r", encoding="utf-8") as stream:
            policy = json.load(stream)
    except FileNotFoundError:
        raise PolicyError("toolchain policy not found: %s" % path)
    except json.JSONDecodeError as error:
        raise PolicyError("invalid JSON in %s: %s" % (path, error))
    validate_policy(policy)
    return policy


def validate_policy(policy):
    if policy.get("schema") != POLICY_SCHEMA:
        raise PolicyError("unsupported toolchain policy schema %r (expected %r)"
                          % (policy.get("schema"), POLICY_SCHEMA))
    dialects = policy.get("dialects")
    if not isinstance(dialects, dict) or not dialects:
        raise PolicyError("policy declares no dialects")
    families = set(policy.get("std_flag_prefixes", {}))
    for name, dialect in dialects.items():
        if dialect.get("language") not in VALID_LANGUAGES:
            raise PolicyError("dialect %s has invalid language %r" % (name, dialect.get("language")))
        if dialect.get("mode") not in VALID_MODES:
            raise PolicyError("dialect %s has invalid mode %r" % (name, dialect.get("mode")))
        mapped = [family for family in families if family in dialect]
        if not mapped:
            raise PolicyError("dialect %s maps no compiler family" % name)
        for family in mapped:
            entry = dialect[family]
            for key in ("required", "forbidden"):
                if not isinstance(entry.get(key, []), list):
                    raise PolicyError("dialect %s/%s %s must be a list" % (name, family, key))
            overlap = set(entry.get("required", [])) & set(entry.get("forbidden", []))
            if overlap:
                raise PolicyError("dialect %s/%s both requires and forbids %s"
                                  % (name, family, ", ".join(sorted(overlap))))
    for language, name in policy.get("defaults", {}).items():
        if name not in dialects or dialects[name]["language"] != language:
            raise PolicyError("default %s dialect %r is not a %s dialect" % (language, name, language))
    for target, entry in policy.get("targets", {}).items():
        if entry.get("dialect") not in dialects:
            raise PolicyError("target %s names unknown dialect %r" % (target, entry.get("dialect")))
        if dialects[entry["dialect"]].get("fixture_only"):
            raise PolicyError("target %s selects fixture-only dialect %s" % (target, entry["dialect"]))
    for language, name in policy.get("defaults", {}).items():
        if dialects[name].get("fixture_only"):
            raise PolicyError("default %s dialect %s is fixture-only" % (language, name))
    for header_set in policy.get("cxx20_header_sets", []):
        paths = header_set.get("paths", [])
        for facade in header_set.get("legacy_facades", []):
            if not any(facade.startswith(p) for p in paths):
                raise PolicyError("facade %s is outside header set %s"
                                  % (facade, header_set.get("id")))


def compiler_family(policy, compiler):
    """Maps a Waf COMPILER_CC/COMPILER_CXX name to a policy compiler family."""
    family = policy["compiler_families"].get(compiler)
    if not family:
        raise PolicyError("TOOLCHAIN005 compiler %r has no declared family" % compiler)
    return family


def target_dialect(policy, target, language):
    """Returns the dialect id for a target's sources of the given language.

    A target's declared dialect applies only to sources of that dialect's
    language; its other-language sources use the language default.
    """
    entry = policy["targets"].get(target)
    if entry:
        name = entry["dialect"]
        if policy["dialects"][name]["language"] == language:
            return name
    return policy["defaults"][language]


def dialect_mapping(policy, dialect_name, family):
    dialect = policy["dialects"][dialect_name]
    mapping = dialect.get(family)
    if mapping is None:
        raise PolicyError("TOOLCHAIN005 dialect %s declares no mapping for the %s compiler family"
                          % (dialect_name, family))
    return mapping


def std_flag(policy, dialect_name, family):
    std = dialect_mapping(policy, dialect_name, family).get("std")
    if std is None:
        return None
    return ("-std=" + std) if family == "gnu" else std


def is_std_flag(policy, family, flag):
    return any(flag.startswith(prefix) for prefix in policy["std_flag_prefixes"][family])


def std_flags_in(policy, family, flags):
    return [flag for flag in flags if is_std_flag(policy, family, flag)]


def normalize_flags(policy, dialect_name, family, flags):
    """Applies an `apply`-mode dialect: exactly one standard flag, required
    flags present, forbidden flags absent. Other flags keep their order."""
    mapping = dialect_mapping(policy, dialect_name, family)
    forbidden = set(mapping.get("forbidden", []))
    result = [flag for flag in flags
              if not is_std_flag(policy, family, flag) and flag not in forbidden]
    wanted = std_flag(policy, dialect_name, family)
    if wanted:
        result.append(wanted)
    for flag in mapping.get("required", []):
        if flag not in result:
            result.append(flag)
    return result


def check_flags(policy, dialect_name, family, flags):
    """Returns the policy violations of one final flag list (empty if none)."""
    dialect = policy["dialects"][dialect_name]
    if dialect["mode"] == "unconstrained":
        return []
    mapping = dialect_mapping(policy, dialect_name, family)
    errors = []
    found = std_flags_in(policy, family, flags)
    wanted = std_flag(policy, dialect_name, family)
    if wanted is None:
        if found:
            errors.append("TOOLCHAIN002 %s expects no standard flag, found %s"
                          % (dialect_name, " ".join(found)))
    elif found != [wanted]:
        errors.append("TOOLCHAIN002 %s expects exactly [%s], found [%s]"
                      % (dialect_name, wanted, " ".join(found)))
    missing = [flag for flag in mapping.get("required", []) if flag not in flags]
    if missing:
        errors.append("TOOLCHAIN003 %s is missing required %s" % (dialect_name, " ".join(missing)))
    present = [flag for flag in mapping.get("forbidden", []) if flag in flags]
    if present:
        errors.append("TOOLCHAIN004 %s has forbidden %s" % (dialect_name, " ".join(present)))
    return errors


def profile_std(policy, dialect_name):
    """The bare standard name (e.g. c++20) a conformance profile compiles with."""
    if dialect_name not in policy["dialects"]:
        raise PolicyError("profile names unknown dialect %r" % dialect_name)
    std = dialect_mapping(policy, dialect_name, "gnu").get("std")
    if not std:
        raise PolicyError("dialect %s has no gnu standard for a conformance profile" % dialect_name)
    return std


def dialect_flags(policy, dialect_name, family="gnu"):
    """Standard plus required flags for compiling a fixture unit in a dialect."""
    flags = []
    wanted = std_flag(policy, dialect_name, family)
    if wanted:
        flags.append(wanted)
    flags += dialect_mapping(policy, dialect_name, family).get("required", [])
    return flags


def header_owner(policy, relative_path):
    """Returns the cxx20 header set owning a repository-relative path, or None."""
    for header_set in policy.get("cxx20_header_sets", []):
        if any(relative_path.startswith(p) for p in header_set.get("paths", [])):
            return header_set
    return None
