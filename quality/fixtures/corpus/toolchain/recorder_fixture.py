#! /usr/bin/env python
# encoding: utf-8
# ==== Copyright Valve Corporation, All rights reserved. ======================
#
# Shared Waf tool of the toolchain-recorder fixture projects (RFC 0005 Q0 gap
# corpus, RFC 0006 M0 / roadmap R03). Driven by
# tools/quality/corpus/toolchain_recorder.py, which copies a project into its
# scratch directory and sets TOOLCHAIN_FIXTURE_REPO to the repository root.
#
# It loads the engine's REAL recorder, scripts/waifulib/toolchain_dialect.py,
# and the real tools/quality/toolchain_policy.py from the repository (the
# fixture project has no tools/ directory, so the recorder's own sys.path
# entry for <top>/tools/quality resolves nothing and the repository's module is
# imported). The recorder reads <top>/quality/toolchain/policy.json, so each
# project carries its own fixture policy there. Like the root wscript, the
# global C++ flags are the policy's default C++ dialect flags at configure time.
#
# ============================================================================

import os
import sys

REPO = os.environ.get('TOOLCHAIN_FIXTURE_REPO')
if not REPO:
	raise RuntimeError('TOOLCHAIN_FIXTURE_REPO is not set (run through toolchain_recorder.py)')
WAIFULIB = os.path.join(REPO, 'scripts', 'waifulib')
POLICY_MODULE_DIR = os.path.join(REPO, 'tools', 'quality')
if POLICY_MODULE_DIR not in sys.path:
	sys.path.append(POLICY_MODULE_DIR)


def options(opt):
	opt.load('compiler_c compiler_cxx clang_compilation_database')
	# Read by toolchain_dialect.configure (the root wscript declares it too).
	opt.add_option('--android', action='store', dest='ANDROID_OPTS', default=None)
	opt.add_option('--fixture-define', action='append', dest='FIXTURE_DEFINES', default=[],
		help='extra define for every fixture target (changes the compile argv)')


def configure(conf):
	conf.load('compiler_c compiler_cxx clang_compilation_database')
	import toolchain_policy
	policy = toolchain_policy.load_policy(conf.srcnode.abspath())
	family = toolchain_policy.compiler_family(policy, conf.env.COMPILER_CXX)
	conf.env.append_unique('CXXFLAGS', toolchain_policy.dialect_flags(
		policy, policy['defaults']['c++'], family))
	conf.load('toolchain_dialect', tooldir=WAIFULIB)
