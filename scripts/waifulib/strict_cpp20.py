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
    profile = json.loads((Path(ctx.srcnode.abspath()) /
                          'quality/profiles/linux-headless-core.json').read_text())
    env = ctx.env.derive()
    env.detach()
    # Preserve toolchain/architecture/optimization settings, remove legacy
    # dialect, permissive compilation and global floating-point relaxations.
    removed = {'-fpermissive', '-w', '-ffast-math', '-funsafe-math-optimizations'}
    env.CXXFLAGS = [flag for flag in env.CXXFLAGS
                    if flag not in removed and not flag.startswith(('-std=', '/std:'))]
    if env.COMPILER_CXX == 'msvc':
        env.CXXFLAGS += ['/std:c++20', '/permissive-']
    else:
        env.CXXFLAGS += ['-std=' + profile['cxx_std']] + profile['base_flags']
    env.INCLUDES = []
    env.DEFINES = []
    return env


def configure(ctx):
    strict = ctx.strict_cpp20_env()
    ctx.check_cxx(
        env=strict, mandatory=True, msg='Checking strict C++20 compiler and library',
        fragment='''#include <concepts>
#include <span>
#include <memory>
template<class T> requires std::integral<T> int value(T x) { return x; }
int main() { int a[] = { 4 }; std::span<int> s(a);
auto p = std::make_unique<int>(value(s[0])); return *p == 4 ? 0 : 1; }
''')
