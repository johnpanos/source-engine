# encoding: utf-8
# compiler_optimizations.py -- main entry point for configuring C/C++ compilers
# Copyright (C) 2021 a1batross
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

try: from fwgslib import get_flags_by_type, get_flags_by_compiler
except: from waflib.extras.fwgslib import get_flags_by_type, get_flags_by_compiler
from waflib.Configure import conf
from waflib import Logs

'''
Flags can be overriden and new types can be added
by importing this as normal Python module

Example:
#!/usr/bin/env python
from waflib.extras import compiler_optimizations

compiler_optimizations.VALID_BUILD_TYPES += 'gottagofast'
compiler_optimizations.CFLAGS['gottagofast'] = {
	'gcc': ['-Ogentoo']
}
'''

VALID_BUILD_TYPES = ['fastnative', 'fast', 'release', 'debug', 'nooptimize', 'sanitize', 'none']

LINKFLAGS = {
	'common': {
		'msvc':  ['/DEBUG'], # always create PDB, doesn't affect result binaries
		'clang': ['-fvisibility=hidden'],
		'gcc':   ['-Wl,--no-undefined'],
		'owcc':  ['-Wl,option stack=512k', '-fvisibility=hidden']
	},
	'sanitize': {
		'clang': ['-fsanitize=undefined', '-fsanitize=address'],
		'gcc':   ['-fsanitize=undefined', '-fsanitize=address'],
	}
}

CFLAGS = {
	'common': {
		# disable thread-safe local static initialization for C++11 code, as it cause crashes on Windows XP
		'msvc':    ['/D_USING_V110_SDK71_', '/Zi', '/FS', '/Zc:threadSafeInit-'],
		'clang':   ['-fno-strict-aliasing', '-fvisibility=hidden'],
		'gcc':     ['-fno-strict-aliasing', '-fvisibility=hidden'],
		'owcc':	   ['-fno-short-enum', '-ffloat-store', '-g0']
	},
	'fast': {
		'msvc':	   ['/O2', '/Oy', '/MT'],
		'gcc':	   ['-Ofast'],
		'clang':   ['-Ofast'],
		'default': ['-O3']
	},
	'fastnative': {
		'msvc':    ['/O2', '/Oy', '/MT'],
		'gcc':     ['-O2', '-march=native', '-funsafe-math-optimizations', '-funsafe-loop-optimizations', '-fomit-frame-pointer'],
		'clang':   ['-O2', '-march=native'],
		'default': ['-O3']
	},
	'release': {
		'msvc':    ['/O2', '/MT'],
		'owcc':    ['-O3', '-fomit-leaf-frame-pointer', '-fomit-frame-pointer', '-finline-functions', '-finline-limit=512'],
		'default': ['-O2', '-funsafe-math-optimizations', '-ftree-vectorize', '-ffast-math']
	},
	'debug': {
		'msvc':    ['/Od', '/MTd'],
		'owcc':    ['-g', '-O0', '-fno-omit-frame-pointer', '-funwind-tables', '-fno-omit-leaf-frame-pointer'],
		'default': ['-g', '-O0'] #, '-ftree-vectorize', '-ffast-math']
	},
	'sanitize': {
		'msvc':    ['/Od', '/RTC1', '/MT'],
		'gcc':     ['-Og', '-fsanitize=undefined', '-fsanitize=address'],
		'clang':   ['-O0', '-fsanitize=undefined', '-fsanitize=address'],
		'default': ['-O0']
	},
	'nooptimize': {
		'msvc':    ['/Od', '/MT'],
		'default': ['-O0']
	}
}

LTO_CFLAGS = {
	'msvc':  ['/GL'],
	'gcc':   ['-flto=auto'],
	'clang': ['-flto']
}

LTO_LINKFLAGS = {
	'msvc':  ['/LTCG'],
	'gcc':   ['-flto=auto'],
	'clang': ['-flto']
}

POLLY_CFLAGS = {
	'gcc':   ['-fgraphite-identity'],
	'clang': ['-mllvm', '-polly']
	# msvc sosat :(
}

# RFC 0023 release flavor: appended after the build type's flags, so -O3 wins over
# release's -O2. Floating-point flags are not touched (AGENTS.md). Each entry stays
# only while an interleaved A/B shows a gain with identical images (R1/R2).
# -fno-lifetime-dse: Source's operator new overloads (C_BaseEntity, CHudElement)
# memset the object and constructors rely on the zeroes; once LTO makes the
# allocator visible, gcc deletes that memset as a dead store before the object's
# lifetime begins, and members read garbage (crashed the first release build).
RELEASE_FLAVOR_CFLAGS = {
	'gcc':   ['-O3', '-fno-semantic-interposition', '-fno-lifetime-dse'],
	'clang': ['-O3', '-fno-semantic-interposition'],
}

RELEASE_FLAVOR_LINKFLAGS = {
	'gcc':   ['-Wl,-O1'],
	'clang': ['-Wl,-O1'],
}

def options(opt):
	grp = opt.add_option_group('Compiler optimization options')

	grp.add_option('-T', '--build-type', action='store', dest='BUILD_TYPE', default=None,
		help = 'build type: debug, release or none(custom flags)')

	grp.add_option('--enable-lto', action = 'store_true', dest = 'LTO', default = False,
		help = 'enable Link Time Optimization if possible [default: %default]')

	grp.add_option('--product-flavor', action = 'store', dest = 'PRODUCT_FLAVOR', default = 'dev',
		choices = ['dev', 'release'],
		help = 'dev keeps development instrumentation; release (RFC 0023) adds RELEASE_FLAVOR_CFLAGS, the '
		'--release-march ISA and SOURCE_RELEASE_BUILD [default: %default]')

	grp.add_option('--release-march', action = 'store', dest = 'RELEASE_MARCH', default = 'native',
		help = 'x86 ISA for --product-flavor=release: native (this host only) or x86-64-v3 (portable) [default: %default]')

	grp.add_option('--release-skip', action = 'store', dest = 'RELEASE_SKIP', default = '',
		help = 'comma list of release-flavor parts to leave out, for leave-one-out timing (RFC 0023 '
		'R1): o3, march, instrumentation (LTO has --enable-lto) [default: none]')

	grp.add_option('--enable-poly-opt', action = 'store_true', dest = 'POLLY', default = False,
		help = 'enable polyhedral optimization if possible [default: %default]')

def configure(conf):
	conf.start_msg('Build type')
	if conf.options.BUILD_TYPE == None:
		conf.end_msg('not set', color='RED')
		conf.fatal('Set a build type, for example "-T release"')
	elif not conf.options.BUILD_TYPE in VALID_BUILD_TYPES:
		conf.end_msg(conf.options.BUILD_TYPE, color='RED')
		conf.fatal('Invalid build type. Valid are: %s' % ', '.join(VALID_BUILD_TYPES))
	conf.end_msg(conf.options.BUILD_TYPE)

	conf.msg('LTO build', 'yes' if conf.options.LTO else 'no')
	conf.msg('Product flavor', conf.options.PRODUCT_FLAVOR)
	conf.msg('PolyOpt build', 'yes' if conf.options.POLLY else 'no')

	# -march=native should not be used
	if conf.options.BUILD_TYPE.startswith('fast'):
		Logs.warn('WARNING: \'%s\' build type should not be used in release builds', conf.options.BUILD_TYPE)

	try:
		conf.env.CC_VERSION[0]
	except IndexError:
		conf.env.CC_VERSION = (0,)

def release_skip(conf):
	skip = set(part for part in conf.options.RELEASE_SKIP.split(',') if part)
	unknown = skip - set(['o3', 'march', 'instrumentation'])
	if unknown:
		conf.fatal('--release-skip: unknown part(s) %s' % ', '.join(sorted(unknown)))
	return skip

@conf
def get_optimization_flags(conf):
	'''Returns a list of compile flags,
	depending on build type and options set by user

	NOTE: it doesn't filter out unsupported flags

	:returns: tuple of cflags and linkflags
	'''
	linkflags = conf.get_flags_by_type(LINKFLAGS, conf.options.BUILD_TYPE, conf.env.COMPILER_CC, conf.env.CC_VERSION[0])

	cflags = conf.get_flags_by_type(CFLAGS, conf.options.BUILD_TYPE, conf.env.COMPILER_CC, conf.env.CC_VERSION[0])

	if conf.env.MSVC_WINE:
		# MSVC under Wine: the shared compiler PDB (/Zi, /FS) needs mspdbsrv,
		# which parallel cl runs under Wine cannot share; /Z7 keeps the debug
		# information in each object.
		cflags = ['/Z7' if flag == '/Zi' else flag for flag in cflags if flag != '/FS']

	if conf.options.LTO:
		linkflags+= conf.get_flags_by_compiler(LTO_LINKFLAGS, conf.env.COMPILER_CC)
		cflags   += conf.get_flags_by_compiler(LTO_CFLAGS, conf.env.COMPILER_CC)

	if conf.options.POLLY:
		cflags   += conf.get_flags_by_compiler(POLLY_CFLAGS, conf.env.COMPILER_CC)

	if conf.options.PRODUCT_FLAVOR == 'release':
		flavor = conf.get_flags_by_compiler(RELEASE_FLAVOR_CFLAGS, conf.env.COMPILER_CC)
		if 'o3' in release_skip(conf):
			flavor = [flag for flag in flavor if flag != '-O3']
		cflags    += flavor
		linkflags += conf.get_flags_by_compiler(RELEASE_FLAVOR_LINKFLAGS, conf.env.COMPILER_CC)

	return cflags, linkflags
