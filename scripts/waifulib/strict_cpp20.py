#! /usr/bin/env python
"""Per-target C++20 environment; never modifies the legacy parent environment."""
import json
import sys
from pathlib import Path
from waflib.Configure import conf
from waflib.TaskGen import feature, before_method


@feature('capability_strict')
@before_method('process_source')
def validate_capability_target(target):
    root = Path(target.bld.srcnode.abspath())
    sys.path.insert(0, str(root / 'tools/archlint'))
    from capabilities import target_errors
    block = json.loads((root / 'architecture/modules.json').read_text())['capabilityModules']
    includes = []
    for inc in target.includes:
        path = Path(inc)
        if not path.is_absolute():
            path = Path(target.path.abspath()) / path
        try:
            includes.append(path.resolve().relative_to(root).as_posix())
        except ValueError:
            includes.append(str(path))
    errors = target_errors(target.name, target.use, includes, block)
    for attribute in ('lib', 'stlib', 'uselib', 'framework', 'libpath'):
        if getattr(target, attribute, None):
            errors.append('CAP003 strict target may not bypass declared use with ' + attribute)
    if errors:
        target.bld.fatal('\n'.join(errors))


@conf
def strict_cpp20_env(ctx):
    """Strict-hygiene environment for capability targets.

    The language standard is not chosen here: every target using this
    environment is declared cxx20 in quality/toolchain/policy.json and the
    toolchain_dialect tool applies it. This environment only removes the
    engine's permissive/fast-math relaxations and global includes/defines and
    adds the linux-headless-core warning flags.
    """
    profile = json.loads((Path(ctx.srcnode.abspath()) /
                          'quality/profiles/linux-headless-core.json').read_text())
    env = ctx.env.derive()
    env.detach()
    removed = {'-fpermissive', '-w', '-ffast-math', '-funsafe-math-optimizations'}
    env.CXXFLAGS = [flag for flag in env.CXXFLAGS if flag not in removed]
    if env.COMPILER_CXX != 'msvc':
        env.CXXFLAGS += profile['base_flags']
    env.INCLUDES = []
    env.DEFINES = []
    return env


def configure(ctx):
    # The strict-hygiene variant of the cxx20 dialect probe: same fragment and
    # policy normalization as toolchain_dialect, plus -Werror and no defines.
    import toolchain_dialect
    root = ctx.srcnode.abspath()
    module, policy = toolchain_dialect.load_policy(root)
    strict = ctx.strict_cpp20_env()
    family = module.compiler_family(policy, strict.COMPILER_CXX)
    env = toolchain_dialect.probe_env(strict, module, policy, 'cxx20', 'c++', family)
    probe = policy['dialects']['cxx20']['probe']
    ctx.check_cxx(
        env=env, mandatory=True, execute=bool(ctx.env.TOOLCHAIN_PROBES_EXECUTED),
        msg='Checking strict-hygiene cxx20 dialect (compile/link/run)',
        fragment=(Path(root) / probe).read_text())
