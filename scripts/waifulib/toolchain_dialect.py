#! /usr/bin/env python
# encoding: utf-8
"""Per-target language dialects (RFC 0006 M0 / roadmap R03).

quality/toolchain/policy.json is the only place that selects a target's C or
C++ standard. This tool:

* probes, at configure time, every dialect this configuration can build with
  the configuration's real flags and defines (compile, link and, for native
  builds, run);
* applies `apply`-mode dialects to each task generator after all inherited and
  uselib flags are merged, so the legacy global -std=c++11 cannot win by
  ordering, and verifies `verify`-mode (legacy) dialects without rewriting
  them, so unmigrated targets keep their exact flags;
* rejects a target that passes its own standard flag instead of declaring its
  dialect in the policy;
* writes <out>/toolchain-invocations.json next to compile_commands.json on each
  build: the final argv of every C/C++ compile task with its target, dialect,
  link uses and toolchain identity, for tools/quality/toolchain_boundary.py.
"""

import hashlib
import json
import os
import sys

from waflib import Context, Utils
from waflib.TaskGen import feature, after_method

INVOCATIONS_SCHEMA = 'toolchain-invocations/v1'
LANGUAGES = (
	# language, flags variable, task-generator attribute, compiler variable, waf feature
	('c++', 'CXXFLAGS', 'cxxflags', 'COMPILER_CXX', 'cxx'),
	('c', 'CFLAGS', 'cflags', 'COMPILER_CC', 'c'),
)


def _module(root):
	path = os.path.join(root, 'tools', 'quality')
	if path not in sys.path:
		sys.path.insert(0, path)
	import toolchain_policy
	return toolchain_policy


_POLICIES = {}


def load_policy(root):
	if root not in _POLICIES:
		module = _module(root)
		_POLICIES[root] = (module, module.load_policy(root))
	return _POLICIES[root]


def _policy_digest(root):
	with open(os.path.join(root, 'quality', 'toolchain', 'policy.json'), 'rb') as stream:
		return hashlib.sha256(stream.read()).hexdigest()


def _target_name(tg):
	return getattr(tg, 'name', None) or str(getattr(tg, 'target', ''))


@feature('c', 'cxx')
@after_method('propagate_uselib_vars')
def apply_toolchain_dialect(self):
	if getattr(self.bld, 'kw', None) is not None:
		# Configuration checks (including the probes below) supply their own env.
		return
	root = Context.top_dir
	module, policy = load_policy(root)
	name = _target_name(self)
	selected = {}
	try:
		for language, variable, attribute, compiler, waf_feature in LANGUAGES:
			if waf_feature not in self.features or not self.env[compiler]:
				continue
			family = module.compiler_family(policy, self.env[compiler])
			own = self.to_list(getattr(self, attribute, []))
			if module.std_flags_in(policy, family, own):
				self.bld.fatal('TOOLCHAIN001 target %s passes %s in %s; declare its dialect in '
					'quality/toolchain/policy.json instead' % (name, ' '.join(
					module.std_flags_in(policy, family, own)), attribute))
			dialect = module.target_dialect(policy, name, language)
			if self.env.R03_LEGACY_CXX11 and dialect == policy['defaults'][language]:
				# TEMPORARY (R03 migration): shared trees stay C++11 until it compiles.
				continue
			flags = list(self.env[variable])
			if policy['dialects'][dialect]['mode'] == 'apply':
				normalized = module.normalize_flags(policy, dialect, family, flags)
				if normalized != flags:
					self.env[variable] = normalized
				flags = normalized
			errors = module.check_flags(policy, dialect, family, flags)
			if errors:
				self.bld.fatal('target %s: %s' % (name, '; '.join(errors)))
			selected[language] = dialect
	except module.PolicyError as error:
		self.bld.fatal('target %s: %s' % (name, error))
	self.toolchain_dialects = selected


def probe_env(base, module, policy, dialect, language, family):
	"""A detached copy of `base` with `dialect` applied as a target would get it."""
	env = base.derive()
	env.detach()
	variable = 'CXXFLAGS' if language == 'c++' else 'CFLAGS'
	# Normalizing is the identity for a conforming verify-mode dialect and
	# selects the fixture-only external-consumer dialect explicitly.
	env[variable] = module.normalize_flags(policy, dialect, family, list(base[variable]))
	return env


def configure(ctx):
	root = ctx.srcnode.abspath()
	module, policy = load_policy(root)
	native = ctx.env.DEST_OS == Utils.unversioned_sys_platform() and not ctx.options.ANDROID_OPTS
	probed = []
	for dialect, entry in sorted(policy['dialects'].items()):
		probe = entry.get('probe')
		if not probe:
			continue
		language = entry['language']
		compiler = ctx.env.COMPILER_CXX if language == 'c++' else ctx.env.COMPILER_CC
		family = module.compiler_family(policy, compiler)
		if family not in entry:
			# No mapping: a target selecting this dialect fails at build time
			# with TOOLCHAIN005 instead of guessing flags.
			ctx.msg('Checking %s dialect' % dialect, 'no %s mapping' % family, color='YELLOW')
			continue
		env = probe_env(ctx.env, module, policy, dialect, language, family)
		check = ctx.check_cxx if language == 'c++' else ctx.check_cc
		check(env=env, fragment=Utils.readf(os.path.join(root, probe)),
			execute=native, mandatory=True,
			msg='Checking %s dialect (%s)' % (dialect, 'compile/link/run' if native else 'compile/link'))
		probed.append(dialect)
	ctx.env.TOOLCHAIN_PROBED_DIALECTS = probed
	ctx.env.TOOLCHAIN_PROBES_EXECUTED = bool(native)


def _write_invocations(bld, tasks):
	root = Context.top_dir
	entries = []
	for task in tasks:
		cmd = getattr(task, 'last_cmd', None)
		if not cmd or not task.inputs:
			continue
		tg = task.generator
		language = 'c++' if task.__class__.__name__ == 'cxx' else 'c'
		entries.append({
			'target': _target_name(tg),
			'source': task.inputs[0].path_from(bld.srcnode),
			'output': task.outputs[0].path_from(bld.bldnode) if task.outputs else None,
			'language': language,
			'dialect': getattr(tg, 'toolchain_dialects', {}).get(language),
			'features': Utils.to_list(getattr(tg, 'features', [])),
			'use': Utils.to_list(getattr(tg, 'use', [])),
			'directory': task.get_cwd().abspath(),
			'arguments': [str(arg) for arg in cmd],
		})
	env = bld.env
	record = {
		'schema': INVOCATIONS_SCHEMA,
		'policy_sha256': _policy_digest(root),
		'toolchain': {
			'dest_os': env.DEST_OS,
			'dest_cpu': env.DEST_CPU,
			'compiler_cc': env.COMPILER_CC,
			'compiler_cxx': env.COMPILER_CXX,
			'cc': env.CC,
			'cxx': env.CXX,
			'cc_version': '.'.join(env.CC_VERSION) if env.CC_VERSION else None,
			'probed_dialects': env.TOOLCHAIN_PROBED_DIALECTS,
			'probes_executed': env.TOOLCHAIN_PROBES_EXECUTED,
		},
		'entries': sorted(entries, key=lambda e: (e['target'], e['source'])),
	}
	node = bld.bldnode.make_node('toolchain-invocations.json')
	node.write(json.dumps(record, indent=1) + '\n')


def _install_invocation_recorder():
	try:
		from waflib.extras import clang_compilation_database as clangdb
	except ImportError:
		return
	context = clangdb.ClangDbContext
	if getattr(context, 'toolchain_invocations_patched', False):
		return
	original = context.write_compilation_database

	def write_compilation_database(self):
		original(self)
		_write_invocations(self, self.clang_compilation_database_tasks)

	context.write_compilation_database = write_compilation_database
	context.toolchain_invocations_patched = True


_install_invocation_recorder()
