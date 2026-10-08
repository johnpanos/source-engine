# encoding: utf-8
# xcompile.py -- crosscompiling utils
# Copyright (C) 2018 a1batross
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

try: from fwgslib import get_flags_by_compiler
except: from waflib.extras.fwgslib import get_flags_by_compiler
from waflib import Logs, TaskGen
from waflib.Tools import c_config
from collections import OrderedDict
import os
import sys

ANDROID_NDK_ENVVARS = ['ANDROID_NDK_HOME', 'ANDROID_NDK']
ANDROID_NDK_SUPPORTED = [10, 19, 20]
ANDROID_NDK_HARDFP_MAX = 11 # latest version that supports hardfp
ANDROID_NDK_GCC_MAX = 17 # latest NDK that ships with GCC
ANDROID_NDK_UNIFIED_SYSROOT_MIN = 15
ANDROID_NDK_SYSROOT_FLAG_MAX = 19 # latest NDK that need --sysroot flag
ANDROID_NDK_API_MIN = { 10: 3, 19: 16, 20: 16 } # minimal API level ndk revision supports
ANDROID_64BIT_API_MIN = 21 # minimal API level that supports 64-bit targets
# r23+ ship one LLVM toolchain with a unified sysroot and libc++ only
# (no GCC, no gnustl, no platforms/ directory). Any such revision is accepted;
# the product profile pins the exact one.
ANDROID_NDK_LLVM_MIN = 23

# Legacy r10e and r19c/r20 NDKs, and LLVM-only r23+ NDKs
class Android:
	ctx            = None # waf context
	arch           = None
	toolchain      = None
	api            = None
	ndk_home       = None
	ndk_rev        = 0
	is_hardfloat   = False
	clang          = False

	def __init__(self, ctx, arch, toolchain, api):
		self.ctx = ctx
		self.api = api
		self.toolchain = toolchain
		self.arch = arch

		for i in ANDROID_NDK_ENVVARS:
			self.ndk_home = os.getenv(i)
			if self.ndk_home != None:
				break
		else:
			ctx.fatal('Set %s environment variable pointing to the root of Android NDK!' %
				' or '.join(ANDROID_NDK_ENVVARS))

		# TODO: this were added at some point of NDK development
		# but I don't know at which version
		# r10e don't have it
		source_prop = os.path.join(self.ndk_home, 'source.properties')
		if os.path.exists(source_prop):
			with open(source_prop) as ndk_props_file:
				for line in ndk_props_file.readlines():
					tokens = line.split('=')
					trimed_tokens = [token.strip() for token in tokens]

					if 'Pkg.Revision' in trimed_tokens:
						self.ndk_rev = int(trimed_tokens[1].split('.')[0])

			if self.ndk_rev not in ANDROID_NDK_SUPPORTED and not self.is_llvm_ndk():
				ctx.fatal('Unknown NDK revision: %d' % (self.ndk_rev))
		else:
			self.ndk_rev = ANDROID_NDK_SUPPORTED[0]

		if 'clang' in self.toolchain or self.ndk_rev > ANDROID_NDK_GCC_MAX:
			self.clang = True

		if self.arch == 'armeabi-v7a-hard':
			if self.ndk_rev <= ANDROID_NDK_HARDFP_MAX:
				self.arch = 'armeabi-v7a' # Only armeabi-v7a have hard float ABI
				self.is_hardfloat = True
			else:
				ctx.fatal('NDK does not support hardfloat ABI')

		if not self.is_llvm_ndk() and self.api < ANDROID_NDK_API_MIN[self.ndk_rev]:
			self.api = ANDROID_NDK_API_MIN[self.ndk_rev]
			Logs.warn('API level automatically was set to %d due to NDK support' % self.api)

		if (self.is_arm64() or self.is_amd64()) and self.api < ANDROID_64BIT_API_MIN:
			self.api = ANDROID_64BIT_API_MIN
			Logs.warn('API level for 64-bit target automatically was set to %d' % self.api)

	def is_llvm_ndk(self):
		'''
		Checks if the NDK is LLVM-only with a unified sysroot and libc++ (r23+)
		'''
		return self.ndk_rev >= ANDROID_NDK_LLVM_MIN

	def is_host(self):
		'''
		Checks if we using host compiler(implies clang)
		'''
		return self.toolchain == 'host'

	def is_arm(self):
		'''
		Checks if selected architecture is **32-bit** ARM
		'''
		return self.arch.startswith('armeabi')

	def is_x86(self):
		'''
		Checks if selected architecture is **32-bit** or **64-bit** x86
		'''
		return self.arch == 'x86'

	def is_amd64(self):
		'''
		Checks if selected architecture is **64-bit** x86
		'''
		return self.arch == 'x86_64'

	def is_arm64(self):
		'''
		Checks if selected architecture is AArch64
		'''
		return self.arch == 'aarch64'

	def is_clang(self):
		'''
		Checks if selected toolchain is Clang (TODO)
		'''
		return self.clang

	def is_hardfp(self):
		return self.is_hardfloat

	def ndk_triplet(self, llvm_toolchain = False, toolchain_folder = False):
		if self.is_x86():
			if toolchain_folder:
				return 'x86'
			else:
				return 'i686-linux-android'
		elif self.is_arm():
			if llvm_toolchain:
				return 'armv7a-linux-androideabi'
			else:
				return 'arm-linux-androideabi'
		elif self.is_amd64() and toolchain_folder:
			return 'x86_64'
		else:
			return self.arch + '-linux-android'

	def apk_arch(self):
		if self.is_arm64():
			return 'arm64-v8a'
		return self.arch

	def gen_host_toolchain(self):
		# With host toolchain we don't care about OS
		# so just download NDK for Linux x86_64
		if self.is_host():
			return 'linux-x86_64'

		if sys.platform.startswith('win32') or sys.platform.startswith('cygwin'):
			osname = 'windows'
		elif sys.platform.startswith('darwin'):
			osname = 'darwin'
		elif sys.platform.startswith('linux'):
			osname = 'linux'
		else:
			self.ctx.fatal('Unsupported by NDK host platform')

		if sys.maxsize > 2**32:
			arch = 'x86_64'
		else: arch = 'x86'

		return '%s-%s' % (osname, arch)

	def gen_gcc_toolchain_path(self):
		path = 'toolchains'
		toolchain_host = self.gen_host_toolchain()

		if self.is_clang():
			toolchain_folder = 'llvm'
		else:
			if self.is_host():
				toolchain = '4.9'
			else:
				toolchain = self.toolchain

			toolchain_folder = '%s-%s' % (self.ndk_triplet(toolchain_folder = True), toolchain)

		return os.path.abspath(os.path.join(self.ndk_home, path, toolchain_folder, 'prebuilt', toolchain_host))

	def gen_toolchain_path(self):
		if self.is_clang():
			triplet = '%s%d-' % (self.ndk_triplet(llvm_toolchain = True), self.api)
		else:
			triplet = self.ndk_triplet() + '-'
		return os.path.join(self.gen_gcc_toolchain_path(), 'bin', triplet)

	def gen_binutils_path(self):
		return os.path.join(self.gen_gcc_toolchain_path(), self.ndk_triplet(), 'bin')

	def cc(self):
		if self.is_host():
			return 'clang --target=%s%d' % (self.ndk_triplet(), self.api)
		return self.gen_toolchain_path() + ('clang' if self.is_clang() else 'gcc')

	def cxx(self):
		if self.is_host():
			return 'clang++ --target=%s%d' % (self.ndk_triplet(), self.api)
		return self.gen_toolchain_path() + ('clang++' if self.is_clang() else 'g++')

	def strip(self):
		if self.is_host():
			return 'llvm-strip'
		if self.is_llvm_ndk():
			return os.path.join(self.gen_gcc_toolchain_path(), 'bin', 'llvm-strip')
		return os.path.join(self.gen_binutils_path(), 'strip')

	def system_stl(self):
		# TODO: proper STL support
		return [
			#os.path.abspath(os.path.join(self.ndk_home, 'sources', 'cxx-stl', 'system', 'include')),
			os.path.abspath(os.path.join(self.ndk_home, 'sources', 'android', 'support', 'include'))
		]

	def libsysroot(self):
		arch = self.arch
		if self.is_arm():
			arch = 'arm'
		elif self.is_arm64():
			arch = 'arm64'
		path = 'platforms/android-%s/arch-%s' % (self.api, arch)

		return os.path.abspath(os.path.join(self.ndk_home, path))

	def sysroot(self):
		if self.ndk_rev >= ANDROID_NDK_UNIFIED_SYSROOT_MIN:
			return os.path.abspath(os.path.join(self.ndk_home, 'sysroot'))
		else:
			return self.libsysroot()

	def cflags(self, cxx = False):
		cflags = []

		if self.ndk_rev <= ANDROID_NDK_SYSROOT_FLAG_MAX:
			cflags += ['--sysroot=%s' % (self.sysroot())]
		else:
			if self.is_host():
				cflags += [
					'--sysroot=%s/sysroot' % (self.gen_gcc_toolchain_path()),
					'-isystem', '%s/usr/include/' % (self.sysroot())
				]

		if not self.is_llvm_ndk():
			cflags += ['-I%s'%i for i in self.system_stl()]
		cflags += ['-DANDROID', '-D__ANDROID__']

		if cxx and not self.is_clang() and self.toolchain not in ['4.8','4.9']:
			cflags += ['-fno-sized-deallocation']

		if self.is_arm():
			if self.arch == 'armeabi-v7a':
				# ARMv7 support
				cflags += ['-mfpu=neon-vfpv4', '-mcpu=cortex-a7', '-mtune=cortex-a7', '-DHAVE_EFFICIENT_UNALIGNED_ACCESS', '-DVECTORIZE_SINCOS']

				if not self.is_clang() and not self.is_host():
					cflags += [ '-mvectorize-with-neon-quad' ]

				if self.is_hardfp():
					cflags += ['-D_NDK_MATH_NO_SOFTFP=1', '-mfloat-abi=hard', '-DLOAD_HARDFP', '-DSOFTFP_LINK']

					if self.is_host():
					# Clang builtin redefine w/ different calling convention bug
					# NOTE: I did not added complex.h functions here, despite
					# that NDK devs forgot to put __NDK_FPABI_MATH__ for complex
					# math functions
					# I personally don't need complex numbers support, but if you want it
					# just run sed to patch header
						for f in ['strtod', 'strtof', 'strtold']:
							cflags += ['-fno-builtin-%s' % f]
				else:
					cflags += ['-mfloat-abi=softfp']
			else:
				# ARMv5 support
				cflags += ['-march=armv5te', '-mtune=xscale', '-msoft-float']
		elif self.is_x86():
			cflags += ['-mtune=atom', '-march=atom', '-mssse3', '-mfpmath=sse', '-DVECTORIZE_SINCOS', '-DHAVE_EFFICIENT_UNALIGNED_ACCESS']
		return cflags

	# they go before object list
	def linkflags(self):
		linkflags = []
		if self.is_host():
			linkflags += ['--gcc-toolchain=%s' % self.gen_gcc_toolchain_path()]

		if self.ndk_rev <= ANDROID_NDK_SYSROOT_FLAG_MAX:
			linkflags += ['--sysroot=%s' % (self.sysroot())]
		elif self.is_host():
			linkflags += ['--sysroot=%s/sysroot' % (self.gen_gcc_toolchain_path())]

		if self.is_clang() or self.is_host():
			linkflags += ['-fuse-ld=lld']

		linkflags += ['-Wl,--hash-style=both','-Wl,--no-undefined']
		return linkflags

	def ldflags(self):
		ldflags = ['-no-canonical-prefixes']
		if not self.is_clang():
			ldflags += ['-lgcc']

		# LLVM NDKs link libc++_shared: one C++ runtime for every packaged module.
		if (self.is_clang() or self.is_host()) and not self.is_llvm_ndk():
			ldflags += ['-stdlib=libstdc++']
		if self.is_arm():
			if self.arch == 'armeabi-v7a':
				ldflags += ['-march=armv7-a']

				if not self.is_clang() and not self.is_host(): # lld only
					ldflags += ['-Wl,--fix-cortex-a8']

				if self.is_hardfp():
					ldflags += ['-Wl,--no-warn-mismatch', '-lm_hard']
			else:
				ldflags += ['-march=armv5te']
		return ldflags

# The Apple SDKs the build targets: SDK canonical-name prefix -> (target
# platform, -target triple OS, Waf DEST_OS). macOS is DEST_OS darwin; iOS and
# tvOS are the ios (UIKit) family, told apart by APPLE_PLATFORM.
APPLE_SDKS = {
	'macosx' : ('macos', 'macos', 'darwin'),
	'iphoneos' : ('ios', 'ios', 'ios'),
	'appletvos' : ('tvos', 'tvos', 'ios'),
}
APPLE_PLATFORM_NAMES = { 'macos' : 'macOS', 'ios' : 'iOS', 'tvos' : 'tvOS' }

def configure_apple(conf):
	"""--apple-sdk: cross-compile for an Apple platform with the pinned host
	toolchain (tools/ios/build_toolchain.py) and the user's SDK: MacOSX for
	macOS, iPhoneOS for iOS or AppleTVOS for tvOS. The SDK is never
	downloaded: it comes from Xcode on the user's Mac. The SDK selects the
	triple; Waf's compiler probe then sets DEST_OS (darwin or ios)."""
	sdk = os.path.abspath(conf.options.APPLE_SDK)
	settings = os.path.join(sdk, 'SDKSettings.json')
	if not os.path.isfile(settings):
		conf.fatal('--apple-sdk=%s is not an Apple SDK (no SDKSettings.json); copy it from the '
			'Mac: xcrun --sdk macosx|iphoneos|appletvos --show-sdk-path' % sdk)
	import json
	with open(settings) as stream:
		sdk_info = json.load(stream)
	entry = APPLE_SDKS.get(sdk_info.get('CanonicalName', '').rstrip('0123456789.'))
	if not entry:
		conf.fatal('%s is %s, not a MacOSX, iPhoneOS or AppleTVOS SDK' % (sdk, sdk_info.get('CanonicalName')))
	platform, triple_os, _ = entry
	toolchain = os.path.abspath(conf.options.APPLE_TOOLCHAIN)
	bindir = os.path.join(toolchain, 'bin')
	for tool in ('clang', 'clang++', 'ld64.lld', 'ld64', 'llvm-ar'):
		if not os.path.isfile(os.path.join(bindir, tool)):
			conf.fatal('%s has no %s; run python3 tools/ios/build_toolchain.py' % (toolchain, tool))
	if not conf.options.APPLE_DEPLOYMENT_TARGET:
		conf.fatal('--apple-sdk needs --apple-deployment-target (the product profile pins it)')
	triple = 'arm64-apple-%s%s' % (triple_os, conf.options.APPLE_DEPLOYMENT_TARGET)
	target = ['-target', triple, '-isysroot', sdk]
	conf.environ['PATH'] = bindir + os.pathsep + conf.environ.get('PATH', '')
	# The target is part of the compiler command, so Waf's compiler probe
	# (clang -dM -E) sees the platform's macros and sets DEST_OS.
	conf.environ['CC'] = ' '.join([os.path.join(bindir, 'clang')] + target)
	conf.environ['CXX'] = ' '.join([os.path.join(bindir, 'clang++')] + target)
	conf.environ['AR'] = os.path.join(bindir, 'llvm-ar')
	conf.env.LINKFLAGS += ['-fuse-ld=lld']
	conf.env.APPLE_SDK = sdk
	conf.env.APPLE_SDK_VERSION = sdk_info.get('Version', '')
	conf.env.APPLE_DEPLOYMENT_TARGET = conf.options.APPLE_DEPLOYMENT_TARGET
	conf.env.APPLE_TRIPLE = triple
	conf.env.APPLE_PLATFORM = platform
	conf.msg('Selected %s SDK' % APPLE_PLATFORM_NAMES[platform], '%s (%s)' % (sdk, conf.env.APPLE_SDK_VERSION))
	conf.msg('... target', triple)

def configure_msvc_wine(conf):
	"""--msvc-wine: cross-compile for Windows with the pinned MSVC under Wine
	(tools/windows/msvc_wine.py provisions it from msvc-wine and the pinned
	Visual Studio manifest). Waf's msvc tool detects Visual Studio from the
	Windows registry; here the toolchain is given instead: its wrapper
	scripts (cl, link, lib, mt, ml64) run the real MSVC binaries under Wine
	and set INCLUDE and LIB themselves."""
	root = os.path.abspath(conf.options.MSVC_WINE)
	arch = 'x86' if conf.options.TARGET32 else 'x64'
	bindir = os.path.join(root, 'bin', arch)
	for tool in ('cl', 'link', 'lib', 'mt'):
		if not os.path.isfile(os.path.join(bindir, tool)):
			conf.fatal('%s has no %s; run python3 tools/windows/msvc_wine.py provision' % (bindir, tool))
	conf.env.DEST_OS = 'win32'
	conf.env.DEST_CPU = 'x86' if arch == 'x86' else 'amd64'
	# Selected tools and versions for msvc.find_msvc (no registry autodetect).
	conf.env.NO_MSVC_DETECT = 1
	conf.env.MSVC_COMPILER = 'msvc'
	conf.env.MSVC_VERSION = float(conf.options.MSVC_WINE_VERSION)
	# Empty: the msvc tool would replace the host PATH with these (';'-joined,
	# for Windows), and the wrappers need the host's to find wine.
	conf.env.PATH = []
	conf.env.MSVC_WINE = root
	for var, tool in (('CC', 'cl'), ('CXX', 'cl'), ('LINK_CC', 'link'), ('LINK_CXX', 'link'),
		('AR', 'lib'), ('MT', 'mt'), ('WINRC', 'rc'),
		('AS', 'ml64' if arch == 'x64' else 'ml')):
		conf.environ[var] = os.path.join(bindir, tool)
	conf.options.check_c_compiler = conf.options.check_cxx_compiler = 'msvc'
	conf.msg('Selected MSVC under Wine', '%s (%s)' % (root, arch))

def options(opt):
	apple = opt.add_option_group('Apple options')
	apple.add_option('--apple-sdk', action='store', dest='APPLE_SDK', default=None,
		help='cross-compile for macOS, iOS or tvOS with this MacOSX.sdk, iPhoneOS.sdk or AppleTVOS.sdk (copied from Xcode on a Mac)')
	apple.add_option('--apple-toolchain', action='store', dest='APPLE_TOOLCHAIN',
		default=os.path.join('dependencies', 'ios', 'toolchain'),
		help='host toolchain built by tools/ios/build_toolchain.py [default: %default]')
	apple.add_option('--apple-deployment-target', action='store', dest='APPLE_DEPLOYMENT_TARGET',
		default=None, help='minimum OS version of the target platform (from the product profile)')
	windows = opt.add_option_group('Windows (MSVC under Wine) options')
	windows.add_option('--msvc-wine', action='store', dest='MSVC_WINE', default=None,
		help='cross-compile for Windows with this msvc-wine install (tools/windows/msvc_wine.py)')
	windows.add_option('--msvc-wine-version', action='store', dest='MSVC_WINE_VERSION', default='14.4',
		help='the installed MSVC toolset as Waf compares it [default: %default]')
	if not sys.platform.startswith('win32'): # the root wscript loads it on Windows
		opt.load('msvc')
	android = opt.add_option_group('Android options')
	android.add_option('--android', action='store', dest='ANDROID_OPTS', default=None,
		help='enable building for android, format: --android=<arch>,<toolchain>,<api>, example: --android=armeabi-v7a-hard,4.9,21')

def configure(conf):
	if getattr(conf.options, 'MSVC_WINE', None):
		if conf.options.ANDROID_OPTS or getattr(conf.options, 'APPLE_SDK', None):
			conf.fatal('--msvc-wine selects a Windows target')
		configure_msvc_wine(conf)
	if getattr(conf.options, 'APPLE_SDK', None):
		if conf.options.ANDROID_OPTS:
			conf.fatal('--apple-sdk and --android select different targets')
		configure_apple(conf)
	if conf.options.ANDROID_OPTS:
		values = conf.options.ANDROID_OPTS.split(',')
		if len(values) != 3:
			conf.fatal('Invalid --android paramater value!')

		valid_archs = ['x86', 'x86_64', 'armeabi', 'armeabi-v7a', 'armeabi-v7a-hard', 'aarch64']

		if values[0] not in valid_archs:
			conf.fatal('Unknown arch: %s. Supported: %r' % (values[0], ', '.join(valid_archs)))


		stlarch = values[0]
		if values[0] == 'aarch64': stlarch = 'arm64-v8a'

		conf.android = android = Android(conf, values[0], values[1], int(values[2]))
		conf.environ['CC'] = android.cc()
		conf.environ['CXX'] = android.cxx()
		conf.environ['STRIP'] = android.strip()
		conf.env.CFLAGS += android.cflags()
		conf.env.CXXFLAGS += android.cflags(True)
		conf.env.LINKFLAGS += android.linkflags()
		conf.env.LDFLAGS += android.ldflags()
		if not android.is_llvm_ndk():
			conf.env.INCLUDES += [
				os.path.abspath(os.path.join(android.ndk_home, 'sources', 'cxx-stl', 'gnu-libstdc++', '4.9', 'include')),
				os.path.abspath(os.path.join(android.ndk_home, 'sources', 'cxx-stl', 'gnu-libstdc++', '4.9', 'libs', stlarch, 'include'))
			]
			conf.env.STLIBPATH += [os.path.abspath(os.path.join(android.ndk_home, 'sources','cxx-stl','gnu-libstdc++','4.9','libs',stlarch))]
			conf.env.LDFLAGS += ['-lgnustl_static']
		conf.env.ANDROID_NDK_HOME = android.ndk_home
		conf.env.ANDROID_API = android.api
		conf.env.ANDROID_APK_ARCH = android.apk_arch()

		conf.env.HAVE_M = True
		if android.is_hardfp():
			conf.env.LIB_M = ['m_hard']
		else: conf.env.LIB_M = ['m']

		conf.env.PREFIX += '/lib/%s' % android.apk_arch()

		conf.msg('Selected Android NDK', '%s, version: %d' % (android.ndk_home, android.ndk_rev))
		# no need to print C/C++ compiler, as it would be printed by compiler_c/cxx
		conf.msg('... C/C++ flags', ' '.join(android.cflags()).replace(android.ndk_home, '$NDK/'))
		conf.msg('... link flags', ' '.join(android.linkflags()).replace(android.ndk_home, '$NDK/'))
		conf.msg('... ld flags', ' '.join(android.ldflags()).replace(android.ndk_home, '$NDK/'))

		# conf.env.ANDROID_OPTS = android
		conf.env.DEST_OS2 = 'android'

	# iOS and tvOS before the generic __APPLE__ (darwin) mapping; clang defines
	# these macros for their device and simulator targets only. tvOS is part
	# of the ios (UIKit) family; APPLE_PLATFORM tells the two apart.
	MACRO_TO_DESTOS = OrderedDict({ '__ANDROID__' : 'android',
		'__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__' : 'ios',
		'__ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__' : 'ios' })
	for k in c_config.MACRO_TO_DESTOS:
		# ordering is important; Waf's own table maps the iOS macro to darwin
		MACRO_TO_DESTOS.setdefault(k, c_config.MACRO_TO_DESTOS[k])
	c_config.MACRO_TO_DESTOS  = MACRO_TO_DESTOS

def post_compiler_cxx_configure(conf):
	if conf.env.DEST_OS == 'ios':
		# Waf applies its Apple settings (no -Bstatic markers, frameworks,
		# .dylib patterns) only for DEST_OS darwin.
		conf.gxx_modifier_darwin()
	conf.msg('Target OS', conf.env.DEST_OS)
	conf.msg('Target CPU', conf.env.DEST_CPU)
	conf.msg('Target binfmt', conf.env.DEST_BINFMT)

	if conf.options.ANDROID_OPTS:
		if conf.android.ndk_rev == 19:
			conf.env.CXXFLAGS_cxxshlib += ['-static-libstdc++']
			conf.env.LDFLAGS_cxxshlib += ['-static-libstdc++']
	return

def post_compiler_c_configure(conf):
	if conf.env.DEST_OS == 'ios':
		conf.gcc_modifier_darwin()
	conf.msg('Target OS', conf.env.DEST_OS)
	conf.msg('Target CPU', conf.env.DEST_CPU)
	conf.msg('Target binfmt', conf.env.DEST_BINFMT)

	return

from waflib.Tools import compiler_cxx, compiler_c

compiler_cxx_configure = getattr(compiler_cxx, 'configure')
compiler_c_configure = getattr(compiler_c, 'configure')

def patch_compiler_cxx_configure(conf):
	compiler_cxx_configure(conf)
	post_compiler_cxx_configure(conf)

def patch_compiler_c_configure(conf):
	compiler_c_configure(conf)
	post_compiler_c_configure(conf)

setattr(compiler_cxx, 'configure', patch_compiler_cxx_configure)
setattr(compiler_c, 'configure', patch_compiler_c_configure)

@TaskGen.feature('cshlib', 'cxxshlib', 'dshlib', 'fcshlib', 'vnum')
@TaskGen.after_method('apply_link', 'propagate_uselib_vars')
@TaskGen.before_method('apply_vnum')
def apply_android_soname(self):
	"""
	Enforce SONAME on Android
	"""
	if self.env.DEST_OS != 'android':
		return

	setattr(self, 'vnum', None) # remove vnum, so SONAME would not be overwritten
	link = self.link_task
	node = link.outputs[0]
	libname = node.name
	v = self.env.SONAME_ST % libname
	self.env.append_value('LINKFLAGS', v.split())

@TaskGen.feature('c', 'cxx')
@TaskGen.before_method('process_source')
def apply_apple_ivp_alloca(self):
	"""IVP (the ivp submodule, upstream source-physics) includes <alloca.h>
	only for LINUX/SUN; no Apple SDK header supplies alloca transitively.
	Give only the IVP targets the header."""
	if self.env.DEST_OS not in ('darwin', 'ios'):
		return
	ivp = self.bld.srcnode.find_node('ivp')
	if ivp and self.path.is_child_of(ivp):
		self.env.append_value('CXXFLAGS', ['-include', 'alloca.h'])
