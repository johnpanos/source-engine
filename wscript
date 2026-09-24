#! /usr/bin/env python
# encoding: utf-8
# nillerusr

from __future__ import print_function
from waflib import Logs, Context, Configure, Options
import sys
import os
import json
import subprocess

VERSION = '1.0'
APPNAME = 'source-engine'
top = '.'

FT2_CHECK='''extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
}

int main() { return FT_Init_FreeType( NULL ); }
'''

FC_CHECK='''extern "C" {
#include <fontconfig/fontconfig.h>
}

int main() { return (int)FcInit(); }
'''

CPP_64BIT_CHECK='''
#define TEST(a) (sizeof(void*) == a ? 1 : -1) 
int g_Test[TEST(8)];

int main () { return 0; }
'''

CPP_32BIT_CHECK='''
#define TEST(a) (sizeof(void*) == a ? 1 : -1) 
int g_Test[TEST(4)];

int main () { return 0; }
'''


Context.Context.line_just = 55 # should fit for everything on 80x26

projects={
	'game': [
		'jobsystem',
		'mapcontainer',
		'appframework',
		'bitmap',
		'choreoobjects',
		'datacache',
		'datamodel',
		'dmxloader',
		'engine',
		'engine/voice_codecs/minimp3',
		'filesystem',
		'game/client',
		'game/server',
		'gameui',
		'inputsystem',
		'ivp/havana',
		'ivp/havana/havok/hk_base',
		'ivp/havana/havok/hk_math',
		'ivp/ivp_compact_builder',
		'ivp/ivp_physics',
		'launcher',
		'launcher_main',
		'materialsystem',
		'materialsystem/shaderapiempty',
		# Each graphics backend is added below only when its product configured
		# the SDK it needs (DXVK Native for shaderapidx9, the Vulkan SDK for
		# shaderapivulkan). The composition root offers whichever were linked.
		'materialsystem/shaderlib',
		'materialsystem/stdshaders',
		'mathlib',
		'particles',
		'scenefilecache',
		'serverbrowser',
		'soundemittersystem',
		'studiorender',
		'stub_steam',
		'tier0',
		'tier1',
		'tier2',
		'tier3',
		'vgui2/matsys_controls',
		'vgui2/src',
		'vgui2/vgui_controls',
		'vgui2/vgui_surfacelib',
		'vguimatsurface',
		'video',
		'vphysics',
		'vphysics_box3d',
		'vpklib',
		'vstdlib',
		'vtf',
		'utils/vtex',
		'unicode',
		'video',
	],
	'tests': [
		'platform',
		'dedicated/composition',
		'mapcontainer',
		'appframework',
		'tier0',
		'tier1',
		'tier2',
		'tier3',
		'unitlib',
		'mathlib',
		'vstdlib',
		'filesystem',
		'vpklib',
		'jobsystem',
		'unittests/tier0test',
		'unittests/legacymoduleclientfixture',
		'unittests/moduleloadfixture',
		'unittests/moduleloadshutdownfixture',
		'unittests/tier1test',
		'unittests/tier2test',
		'unittests/tier3test',
		'unittests/mathlibtest',
		'unittests/jobsystemtest',
		'unittests/audioprovidertest',
		'unittests/appframeworktest',
		'utils/unittest'
	],
	'tools': [
		'fgdlib',
		'jobsystem',
		'mapcontainer',
		'tier0',
		'tier1',
		'mathlib',
		'bitmap',
		'tier2',
		'vstdlib',
		'vtf',
		'utils/vtex',
		'utils/vtexconv',
		'utils/bsp2tool',
	],
	'dedicated': [
		'jobsystem',
		'mapcontainer',
		'platform',
		'dedicated/composition',
		'appframework',
		'bitmap',
		'choreoobjects',
		'datacache',
		'dedicated',
		'dedicated_main',
		'dmxloader',
		'engine',
		'game/server',
		'ivp/havana',
		'ivp/havana/havok/hk_base',
		'ivp/havana/havok/hk_math',
		'ivp/ivp_compact_builder',
		'ivp/ivp_physics',
		'materialsystem',
		'mathlib',
		'particles',
		'scenefilecache',
		'materialsystem/shaderapiempty',
		'materialsystem/shaderlib',
		'soundemittersystem',
		'studiorender',
		'tier0',
		'tier1',
		'tier2',
		'tier3',
		'vgui2/vgui_controls',
		'vphysics',
		'vphysics_box3d',
		'vpklib',
		'vstdlib',
		'vtf',
		'stub_steam'
	]
}

LINUX_COMPILER_TOOL_PROJECTS = [
	'filesystem',
	'ivp/havana',
	'ivp/havana/havok/hk_base',
	'ivp/havana/havok/hk_math',
	'ivp/ivp_compact_builder',
	'ivp/ivp_physics',
	'materialsystem',
	'materialsystem/shaderapiempty',
	'materialsystem/shaderlib',
	'materialsystem/stdshaders',
	'vpklib',
	'vphysics',
	'raytrace',
	'utils/vbsp',
	'utils/vvis',
	'utils/vrad',
]

@Configure.conf
def check_pkg(conf, package, uselib_store, fragment, *k, **kw):
	errormsg = '{0} not available! Install {0} development package. Also you may need to set PKG_CONFIG_PATH environment variable'.format(package)
	confmsg = 'Checking for \'{0}\' sanity'.format(package)
	errormsg2 = '{0} isn\'t installed correctly. Make sure you installed proper development package for target architecture'.format(package)

	try:
		conf.check_cfg(package=package, args='--cflags --libs', uselib_store=uselib_store, *k, **kw )
	except conf.errors.ConfigurationError:
		conf.fatal(errormsg)

	try:
		conf.check_cxx(fragment=fragment, use=uselib_store, msg=confmsg, *k, **kw)
	except conf.errors.ConfigurationError:
		conf.fatal(errormsg2)

@Configure.conf
def get_taskgen_count(self):
	try: idx = self.tg_idx_count
	except: idx = 0 # don't set tg_idx_count to not increase counter
	return idx

@Configure.conf
def run_test(self, fragment, msg):
	result = self.check_cxx(fragment=fragment, msg=msg, mandatory = False)
	return False if result == None else True

def define_platform(conf):
	conf.env.SDL3 = conf.options.PLATFORM_PROVIDER == 'sdl3'
	conf.env.DXVK = conf.options.RENDER_BACKEND == 'vulkan'
	conf.env.NATIVE_VULKAN = conf.options.RENDER_BACKEND == 'native-vulkan'
	if conf.env.SDL3 or conf.env.DXVK or conf.env.NATIVE_VULKAN:
		if conf.env.DEST_OS not in ['linux', 'android'] or conf.options.DEDICATED or conf.options.TESTS or conf.options.TOOLS:
			conf.fatal('The SDL3/Vulkan profiles currently target the Linux and Android clients')
		if conf.env.DEST_OS == 'android' and not conf.env.NATIVE_VULKAN:
			conf.fatal('The Android client requires --render-backend=native-vulkan')
		if not (conf.env.SDL3 and (conf.env.DXVK or conf.env.NATIVE_VULKAN)):
			conf.fatal('Select both --platform-provider=sdl3 and --render-backend=vulkan')
		conf.options.SDL = 1
		conf.options.GL = 0
		conf.define('USE_SDL3', 1)
		conf.define('USE_DXVK', 1)
	# The SDL3/native Vulkan Android client (build-android-apk.sh). Its native
	# dependencies come from the profile's cross-built prefix through
	# pkg-config, not from the legacy prebuilt lib/android tree.
	conf.env.ANDROID_SDL3 = conf.env.DEST_OS == 'android' and conf.env.SDL3
	conf.env.DEDICATED = conf.options.DEDICATED
	conf.env.TESTS = conf.options.TESTS
	conf.env.TOOLS = conf.options.TOOLS
	if sum(bool(value) for value in (conf.env.DEDICATED, conf.env.TESTS, conf.env.TOOLS)) > 1:
		conf.fatal('--dedicated, --tests, and --tools select different products')
	conf.env.TOGLES = conf.options.TOGLES
	conf.env.GL = conf.options.GL and not conf.options.TESTS and not conf.options.DEDICATED and not conf.options.TOOLS
	conf.env.OPUS = conf.options.OPUS
	conf.env.VIDEO_BINK = conf.options.VIDEO_PROVIDER == 'bink'
	if conf.env.VIDEO_BINK and (conf.options.TESTS or conf.options.DEDICATED):
		conf.fatal('The Bink video provider belongs to client products only')

	arch32 = conf.run_test(CPP_32BIT_CHECK, 'Testing 32bit support')
	arch64 = conf.run_test(CPP_64BIT_CHECK, 'Testing 64bit support')

	if not (arch32 ^ arch64):
		conf.fatal('Your compiler sucks')

	if conf.options.DEDICATED:
		conf.options.SDL = False
		conf.define('DEDICATED', 1)
	elif conf.options.TOOLS:
		conf.options.SDL = False

	if conf.options.TESTS:
		conf.define('UNITTESTS', 1)

	if conf.env.GL:
		conf.env.append_unique('DEFINES', [
			'DX_TO_GL_ABSTRACTION',
			'GL_GLEXT_PROTOTYPES',
			'BINK_VIDEO'
		])

	if conf.options.TOGLES:
		conf.env.append_unique('DEFINES', ['TOGLES'])

	if conf.options.TESTS:
		conf.define('UNITTESTS', 1)

	if conf.options.SDL and not conf.options.TESTS:
		conf.env.SDL = 1
		conf.define('USE_SDL', 1)

	if arch64:
		conf.define('PLATFORM_64BITS', 1)

	if conf.env.DEST_OS == 'linux':
		conf.define('_GLIBCXX_USE_CXX11_ABI',0)
		conf.env.append_unique('DEFINES', [
			'LINUX=1', '_LINUX=1',
			'POSIX=1', '_POSIX=1', 'PLATFORM_POSIX=1',
			'GNUC',
			'NO_HOOK_MALLOC',
			'_DLL_EXT=.so'
		])
		conf.env.append_unique('CFLAGS', '-U_FORTIFY_SOURCE')
		conf.env.append_unique('CXXFLAGS', '-U_FORTIFY_SOURCE')
	elif conf.env.DEST_OS == 'android':
		conf.env.append_unique('DEFINES', [
			'ANDROID=1', '_ANDROID=1',
			'LINUX=1', '_LINUX=1',
			'POSIX=1', '_POSIX=1',
			'GNUC',
			'NO_HOOK_MALLOC',
			'_DLL_EXT=.so'
		])
		
	elif conf.env.DEST_OS == 'win32':
		conf.env.append_unique('DEFINES', [
			'WIN32=1', '_WIN32=1',
			'_WINDOWS',
			'_DLL_EXT=.dll',
			'_CRT_SECURE_NO_DEPRECATE',
			'_CRT_NONSTDC_NO_DEPRECATE',
			'_ALLOW_RUNTIME_LIBRARY_MISMATCH',
			'_ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH',
			'_ALLOW_MSC_VER_MISMATCH',
			'NO_X360_XDK'
		])
	elif conf.env.DEST_OS == 'darwin':
		conf.env.append_unique('DEFINES', [
			'OSX=1', '_OSX=1',
			'POSIX=1', '_POSIX=1', 'PLATFORM_POSIX=1',
			'GNUC',
			'NO_HOOK_MALLOC',
			'_DLL_EXT=.dylib'
		])

	elif conf.env.DEST_OS in ['freebsd', 'openbsd', 'netbsd', 'dragonflybsd']: # Tested only in freebsd
		conf.env.append_unique('DEFINES', [
			'POSIX=1', '_POSIX=1', 'PLATFORM_POSIX=1',
			'GNUC', # but uses clang
			'PLATFORM_BSD=1',
			'_DLL_EXT=.so'
		])

	if conf.env.DEST_OS != 'win32':
		conf.define('NO_MEMOVERRIDE_NEW_DELETE', 1)
#		conf.define('NO_MALLOC_OVERRIDE', 1)

	if conf.options.DEBUG_ENGINE:
		conf.env.append_unique('DEFINES', [
			'DEBUG', '_DEBUG'
		])
	else:
		conf.env.append_unique('DEFINES', [
			'NDEBUG'
		])

	conf.define('GIT_COMMIT_HASH', conf.env.GIT_VERSION)


def options(opt):
	grp = opt.add_option_group('Common options')

	grp.add_option('-4', '--32bits', action = 'store_true', dest = 'TARGET32', default = False,
		help = 'allow targetting 32-bit engine(Linux/Windows/OSX x86 only) [default: %default]')

	grp.add_option('-d', '--dedicated', action = 'store_true', dest = 'DEDICATED', default = False,
		help = 'build dedicated server [default: %default]')

	grp.add_option('--tests', action = 'store_true', dest = 'TESTS', default = False,
		help = 'build unit tests [default: %default]')

	grp.add_option('--tools', action = 'store_true', dest = 'TOOLS', default = False,
		help = 'build isolated host content tools [default: %default]')

	grp.add_option('-D', '--debug-engine', action = 'store_true', dest = 'DEBUG_ENGINE', default = False,
		help = 'build with -DDEBUG [default: %default]')

	grp.add_option('--use-sdl', action = 'store', dest = 'SDL', type = 'int', default = sys.platform != 'win32',
		help = 'build engine with SDL [default: %default]')
	grp.add_option('--platform-provider', choices=['sdl2', 'sdl3'], default='sdl2',
		dest='PLATFORM_PROVIDER', help='linked window/input provider')
	grp.add_option('--render-backend', choices=['legacy', 'vulkan', 'native-vulkan'], default='legacy',
		dest='RENDER_BACKEND', help='linked renderer; vulkan uses the DXVK compatibility provider')
	grp.add_option('--physics-backend', choices=['ivp', 'box3d', 'both'], default='ivp',
		dest='PHYSICS_BACKEND', help='production physics provider, or both for tests')
	grp.add_option('--video-provider', choices=['none', 'bink'], default='none',
		dest='VIDEO_PROVIDER', help='linked video decoder (bink requires FFmpeg development libraries)')
	grp.add_option('--debug-api', choices=['enabled', 'disabled'], default='disabled',
		dest='DEBUG_API', help='engine debug API server (JSON-RPC 2.0; Linux desktop development only)')
	grp.add_option('--protobuf-root', default='', dest='PROTOBUF_ROOT',
		help='pinned protobuf prefix for --debug-api [default: build-deps/debugapi/prefix]')
	grp.add_option('--dxvk-root', default='', dest='DXVK_ROOT',
		help='pinned DXVK Native package prefix (contains include/dxvk and lib)')
	grp.add_option('--product-profile', default='quality/product_profiles/portal-linux-wayland.json',
		dest='PRODUCT_PROFILE', help='versioned profile for the Vulkan compatibility product')
	grp.add_option('--ktx-source-root', default='', dest='KTX_SOURCE_ROOT',
		help='explicit checkout of the pinned KTX-Software revision for texture reader tests')
	grp.add_option('--ktx-build-root', default='', dest='KTX_BUILD_ROOT',
		help='isolated KTX-Software build containing libktx_read.a')
	for name in ('openusd-source', 'openusd-build', 'openusd-install',
		'onetbb-source', 'onetbb-build', 'onetbb-install'):
		grp.add_option('--' + name + '-root', default='',
			dest=name.upper().replace('-', '_') + '_ROOT',
			help='isolated pinned World Stage host-tool dependency path')

	grp.add_option('--use-togl', action = 'store', dest = 'GL', type = 'int', default = sys.platform != 'win32',
		help = 'build engine with ToGL [default: %default]')

	grp.add_option('--build-games', action = 'store', dest = 'GAMES', type = 'string', default = 'hl2',
		help = 'build games [default: %default]')

	grp.add_option('--use-ccache', action = 'store_true', dest = 'CCACHE', default = False,
		help = 'build using ccache [default: %default]')

	grp.add_option('--disable-warns', action = 'store_true', dest = 'DISABLE_WARNS', default = False,
		help = 'build using ccache [default: %default]')

	grp.add_option('--togles', action = 'store_true', dest = 'TOGLES', default = False,
		help = 'build engine with ToGLES [default: %default]')

	# TODO(nillerusr): add wscript for opus building
	grp.add_option('--enable-opus', action = 'store_true', dest = 'OPUS', default = False,
		help = 'build engine with Opus voice codec [default: %default]')

	grp.add_option('--sanitize', action = 'store', dest = 'SANITIZE', default = '',
		help = 'build with sanitizers [default: %default]')

	opt.load('compiler_optimizations subproject')

	opt.load('xcompile compiler_cxx compiler_c sdl2 clang_compilation_database strip_on_install_v2 waf_unit_test subproject')
	if sys.platform == 'win32':
		opt.load('msvc msdev msvs')
	opt.load('reconfigure')

def check_deps(conf):
	if conf.env.DEST_OS != 'win32':
		conf.check_cc(lib='dl', mandatory=False)
		if not conf.env.ANDROID_SDL3: # built in-tree (utils/bzip2), as on Windows
			conf.check_cc(lib='bz2', mandatory=True)
		conf.check_cc(lib='rt', mandatory=False)

		if not conf.env.LIB_M: # HACK: already added in xcompile!
			conf.check_cc(lib='m')
	else:
		# Common Win32 libraries
		# Don't check them more than once, to save time
		# Usually, they are always available
		# but we need them in uselib
		a = [
			'user32',
			'shell32',
			'gdi32',
			'advapi32',
			'dbghelp',
			'psapi',
			'ws2_32',
			'rpcrt4',
			'winmm',
			'wininet',
			'ole32',
			'shlwapi',
			'imm32'
		]

		if conf.env.COMPILER_CC == 'msvc':
			for i in a:
				conf.check_lib_msvc(i)
		else:
			for i in a:
				conf.check_cc(lib = i)

	if conf.env.DEST_OS == "darwin":
		conf.check(lib='iconv', uselib_store='ICONV')
		conf.env.FRAMEWORK_APPKIT = "AppKit"
		conf.env.FRAMEWORK_IOKIT = "IOKit"
		conf.env.FRAMEWORK_FOUNDATION = "Foundation"
		conf.env.FRAMEWORK_COREFOUNDATION = "CoreFoundation"
		conf.env.FRAMEWORK_COREGRAPHICS = "CoreGraphics"
		conf.env.FRAMEWORK_OPENGL = "OpenGL"
		conf.env.FRAMEWORK_CARBON = "Carbon"
		conf.env.FRAMEWORK_APPLICATIONSERVICES = "ApplicationServices"
		conf.env.FRAMEWORK_CORESERVICES = "CoreServices"
		conf.env.FRAMEWORK_COREAUDIO = "CoreAudio"
		conf.env.FRAMEWORK_AUDIOTOOLBOX = "AudioToolbox"
		conf.env.FRAMEWORK_SYSTEMCONFIGURATION = "SystemConfiguration"

	if conf.options.TESTS:
		return

	if conf.options.TOOLS:
		if conf.env.DEST_OS == 'android':
			conf.fatal('host tools cannot be built for the Android target')
		if conf.env.DEST_OS != 'win32':
			conf.check_cfg(package='libjpeg', uselib_store='JPEG', args=['--cflags', '--libs'])
			conf.check_cfg(package='libpng', uselib_store='PNG', args=['--cflags', '--libs'])
			conf.check_cfg(package='zlib', uselib_store='ZLIB', args=['--cflags', '--libs'])
		else:
			conf.check(lib='libz', uselib_store='ZLIB', define_name='USE_ZLIB')
			conf.check(lib='libjpeg', uselib_store='JPEG', define_name='HAVE_JPEG')
			conf.check(lib='libpng', uselib_store='PNG', define_name='HAVE_PNG')
		return

	if conf.env.DEST_OS != 'android':
		if conf.env.DEST_OS != 'win32':
			if conf.options.SDL:
				conf.check_cfg(package='sdl3' if conf.env.SDL3 else 'sdl2',
					uselib_store='SDL2', args=['--cflags', '--libs'])
				if conf.env.SDL3:
					conf.check_cfg(package='sdl3', uselib_store='SDL3', args=['--cflags', '--libs'])
					conf.env.INCLUDES_SDL2 += [os.path.abspath('platform/sdl3/legacy_include')]
			if conf.options.DEDICATED:
				conf.check_cfg(package='libedit', uselib_store='EDIT', args=['--cflags', '--libs'])
			else:
				conf.check_pkg('freetype2', 'FT2', FT2_CHECK)
				conf.check_pkg('fontconfig', 'FC', FC_CHECK)
				if conf.env.DEST_OS == "darwin":
					conf.env.FRAMEWORK_OPENAL = "OpenAL"
				else:
					conf.check_cfg(package='openal', uselib_store='OPENAL', args=['--cflags', '--libs'])
				conf.check_cfg(package='libjpeg', uselib_store='JPEG', args=['--cflags', '--libs'])
				conf.check_cfg(package='libpng', uselib_store='PNG', args=['--cflags', '--libs'])
				conf.check_cfg(package='libcurl', uselib_store='CURL', args=['--cflags', '--libs'])
			conf.check_cfg(package='zlib', uselib_store='ZLIB', args=['--cflags', '--libs'])

			if conf.options.OPUS:
				conf.check_cfg(package='opus', uselib_store='OPUS', args=['--cflags', '--libs'])
	elif conf.env.ANDROID_SDL3:
		# PKG_CONFIG_LIBDIR names only the cross-built prefix, so no host
		# package can satisfy a target check. No fontconfig: Android fonts are
		# files the app ships. Audio uses SDL3; zlib is the NDK's system library.
		conf.check_cfg(package='sdl3', uselib_store='SDL2', args=['--cflags', '--libs'])
		conf.check_cfg(package='sdl3', uselib_store='SDL3', args=['--cflags', '--libs'])
		conf.env.INCLUDES_SDL2 += [os.path.abspath('platform/sdl3/legacy_include')]
		conf.check_pkg('freetype2', 'FT2', FT2_CHECK)
		conf.check_cfg(package='libjpeg', uselib_store='JPEG', args=['--cflags', '--libs'])
		conf.check_cfg(package='libpng', uselib_store='PNG', args=['--cflags', '--libs'])
		conf.check_cfg(package='libcurl', uselib_store='CURL', args=['--cflags', '--libs'])
		conf.check_cc(lib='z', uselib_store='ZLIB')
		conf.check_cc(lib='android', uselib_store='ANDROID')
	else:
		conf.check(lib='SDL2', uselib_store='SDL2')
		conf.check(lib='freetype2', uselib_store='FT2')
		conf.check(lib='jpeg', uselib_store='JPEG', define_name='HAVE_JPEG')
		conf.check(lib='png', uselib_store='PNG', define_name='HAVE_PNG')
		conf.check(lib='curl', uselib_store='CURL', define_name='HAVE_CURL')
		conf.check(lib='z', uselib_store='ZLIB', define_name='HAVE_ZLIB')
		if conf.env.DEST_CPU != 'aarch64':
			conf.check(lib='unwind', uselib_store='UNWIND')
			conf.check(lib='crypto', uselib_store='CRYPTO')
			conf.check(lib='ssl', uselib_store='SSL')
		conf.check(lib='android_support', uselib_store='ANDROID_SUPPORT')
		conf.check(lib='opus', uselib_store='OPUS')

	if conf.env.DEST_OS == 'win32':
		conf.check(lib='libz', uselib_store='ZLIB', define_name='USE_ZLIB')
		# conf.check(lib='nvtc', uselib_store='NVTC')
		# conf.check(lib='ati_compress_mt_vc10', uselib_store='ATI_COMPRESS_MT_VC10')
		conf.check(lib='SDL2', uselib_store='SDL2')
		conf.check(lib='libjpeg', uselib_store='JPEG', define_name='HAVE_JPEG')
		conf.check(lib='libpng', uselib_store='PNG', define_name='HAVE_PNG')
		conf.check(lib='d3dx9', uselib_store='D3DX9')
		conf.check(lib='d3d9', uselib_store='D3D9')
		conf.check(lib='dsound', uselib_store='DSOUND')
		conf.check(lib='dxguid', uselib_store='DXGUID')
		if conf.options.OPUS:
			conf.check(lib='opus', uselib_store='OPUS')

		# conf.multicheck(*a, run_all_tests = True, mandatory = True)

def configure(conf):
	conf.load('fwgslib reconfigure compiler_optimizations')
	# --rebuild-cache/--reconfigure replace conf.options with the options stored
	# when the tree was configured; options added since then keep their defaults.
	for name, value in vars(Options.options).items():
		if not hasattr(conf.options, name):
			setattr(conf.options, name, value)

	# Force XP compability, all build targets should add
	# subsystem=bld.env.MSVC_SUBSYSTEM
	# TODO: wrapper around bld.stlib, bld.shlib and so on?
	conf.env.MSVC_SUBSYSTEM = 'WINDOWS,5.01'
	conf.env.MSVC_TARGETS = ['x64'] # explicitly request x86 target for MSVC
	if conf.options.TARGET32:
		conf.env.MSVC_TARGETS = ['x86']

	if sys.platform == 'win32':
		conf.load('msvc_pdb_ext msdev msvs msvcdeps')
	conf.load('subproject xcompile compiler_c compiler_cxx gccdeps gitversion clang_compilation_database strip_on_install_v2 waf_unit_test enforce_pic')
	if conf.env.DEST_OS == 'win32' and conf.env.DEST_CPU == 'amd64':
		conf.load('masm')
	elif conf.env.DEST_OS == 'darwin':
		conf.load('mm_hook')

	conf.env.BIT32_MANDATORY = conf.options.TARGET32
	if conf.env.BIT32_MANDATORY:
		Logs.info('WARNING: will build engine for 32-bit target')
		conf.load('force_32bit')

	define_platform(conf)

	if conf.env.TOGLES:
		projects['game'] += ['togles']
	elif conf.env.GL:
		projects['game'] += ['togl']

	if conf.env.DEST_OS == 'win32':
		projects['game'] += ['utils/bzip2']
		projects['dedicated'] += ['utils/bzip2']
	if conf.options.OPUS or (conf.env.DEST_OS == 'android' and not conf.env.ANDROID_SDL3):
		projects['game'] += ['engine/voice_codecs/opus']
	if conf.env.ANDROID_SDL3:
		projects['game'] += ['utils/bzip2']

	if conf.options.DISABLE_WARNS:
		compiler_optional_flags = ['-w']
	else:
		compiler_optional_flags = [
			'-Wall',
			'-fdiagnostics-color=always',
			'-Wcast-align',
			'-Wuninitialized',
			'-Winit-self',
			'-Wstrict-aliasing',
			'-Wno-reorder',
			'-Wno-unknown-pragmas',
			'-Wno-unused-function',
			'-Wno-unused-but-set-variable',
			'-Wno-unused-value',
			'-Wno-unused-variable',
			'-faligned-new',
		]

	c_compiler_optional_flags = [
		'-fnonconst-initializers' # owcc
	]

	cflags, linkflags = conf.get_optimization_flags()


	flags = []

	if conf.options.SANITIZE:
		flags += ['-fsanitize=%s'%conf.options.SANITIZE, '-fno-sanitize=vptr']

	if conf.env.ANDROID_SDL3:
		flags += ['-pipe', '-fPIC']
	elif conf.env.DEST_OS != 'win32':
		flags += ['-pipe', '-fPIC', '-L'+os.path.abspath('.')+'/lib/'+conf.env.DEST_OS+'/'+conf.env.DEST_CPU+'/']
	if conf.env.COMPILER_CC != 'msvc':
		flags += ['-pthread']

	if conf.env.ANDROID_SDL3:
		flags += ['-llog', '-funwind-tables', '-g']
	elif conf.env.DEST_OS == 'android':
		flags += [
			'-I'+os.path.abspath('.')+'/thirdparty/curl/include',
			'-I'+os.path.abspath('.')+'/thirdparty/SDL',
			'-I'+os.path.abspath('.')+'/thirdparty/openal-soft/include/',
			'-I'+os.path.abspath('.')+'/thirdparty/fontconfig',
			'-I'+os.path.abspath('.')+'/thirdparty/freetype/include',
			'-llog',
			'-lz'
		]

		flags += ['-funwind-tables', '-g']
	elif conf.env.COMPILER_CC != 'msvc' and conf.env.DEST_OS != 'darwin' and conf.env.DEST_CPU in ['x86', 'x86_64']:
		flags += ['-march=core2']

	if conf.env.DEST_CPU in ['x86', 'x86_64']:
		flags += ['-mfpmath=sse']
	elif conf.env.DEST_CPU in ['arm', 'aarch64']:
		flags += ['-fsigned-char']

	if conf.env.DEST_CPU == 'arm':
		flags += ['-march=armv7-a', '-mfpu=neon-vfpv4']

	if conf.env.DEST_OS == 'freebsd':
		linkflags += ['-lexecinfo']

	if conf.env.DEST_OS != 'win32':
		cflags += flags
		linkflags += flags
	else:
		cflags += [
			'/I'+os.path.abspath('.')+'/thirdparty/SDL',
			'/arch:SSE' if conf.env.DEST_CPU == 'x86' else '/arch:AVX',
			'/GF',
			'/Gy',
			'/fp:fast',
			'/Zc:forScope',
			'/Zc:wchar_t',
			'/GR',
			'/TP',
			'/EHsc'
		]

		if conf.options.BUILD_TYPE == 'debug':
			linkflags += [
				'/FORCE:MULTIPLE',
				'/INCREMENTAL:NO',
				'/NODEFAULTLIB:libc',
				'/NODEFAULTLIB:libcd',
				'/NODEFAULTLIB:libcmt',
				'/LARGEADDRESSAWARE'
			]
		else:
			linkflags += [
				'/INCREMENTAL',
				'/NODEFAULTLIB:libc',
				'/NODEFAULTLIB:libcd',
				'/NODEFAULTLIB:libcmtd',
				'/LARGEADDRESSAWARE'
			]

		linkflags += [
			'/LIBPATH:'+os.path.abspath('.')+'/lib/win32/'+conf.env.DEST_CPU+'/',
			'/LIBPATH:'+os.path.abspath('.')+'/dx9sdk/lib/'+conf.env.DEST_CPU+'/'
		]

	# And here C++ flags starts to be treated separately
	cxxflags = list(cflags)
	policy_path = os.path.join(conf.srcnode.abspath(), 'tools', 'quality')
	if policy_path not in sys.path:
		sys.path.insert(0, policy_path)
	import toolchain_policy
	policy = toolchain_policy.load_policy(conf.srcnode.abspath())
	family = toolchain_policy.compiler_family(policy, conf.env.COMPILER_CXX)
	cxxflags += toolchain_policy.dialect_flags(policy, policy['defaults']['c++'], family)

	if conf.env.COMPILER_CC == 'gcc':
		conf.define('COMPILER_GCC', 1)
	elif conf.env.COMPILER_CC == 'msvc':
		conf.define('COMPILER_MSVC', 1)
		conf.define('MSVC', 1)
		if conf.env.DEST_CPU == 'x86':
			conf.define('COMPILER_MSVC32', 1)
		elif conf.env.DEST_CPU in ['x86_64', 'amd64']:
			conf.define('COMPILER_MSVC64', 1)

	if conf.env.COMPILER_CC != 'msvc':
		conf.check_cc(cflags=cflags, linkflags=linkflags, msg='Checking for required C flags')
		conf.check_cxx(cxxflags=cxxflags, linkflags=linkflags, msg='Checking for required C++ flags')

		conf.env.append_unique('CFLAGS', cflags)
		conf.env.append_unique('CXXFLAGS', cxxflags)
		conf.env.append_unique('LINKFLAGS', linkflags)

		cxxflags += conf.filter_cxxflags(compiler_optional_flags, cflags)
		cflags += conf.filter_cflags(compiler_optional_flags + c_compiler_optional_flags, cflags)

	conf.env.append_unique('CFLAGS', cflags)
	conf.env.append_unique('CXXFLAGS', cxxflags)
	conf.env.append_unique('LINKFLAGS', linkflags)
	conf.env.append_unique('INCLUDES', [os.path.abspath('common/')])

	# Per-target dialects (RFC 0006 M0): probes each declared dialect with the
	# flags above; targets select theirs in quality/toolchain/policy.json.
	conf.load('toolchain_dialect')

	check_deps( conf )

	if conf.env.NATIVE_VULKAN and conf.env.DEST_OS == 'android':
		# The NDK sysroot provides the Vulkan headers and loader stub.
		conf.check_cc(lib='vulkan', header_name='vulkan/vulkan.h', uselib_store='VULKAN')
	elif conf.env.NATIVE_VULKAN:
		conf.check_cfg(package='vulkan', uselib_store='VULKAN', args=['--cflags', '--libs'])
	if conf.options.KTX_SOURCE_ROOT or conf.options.KTX_BUILD_ROOT:
		if not (conf.options.KTX_SOURCE_ROOT and conf.options.KTX_BUILD_ROOT):
			conf.fatal('KTX reader tests require both --ktx-source-root and --ktx-build-root')
		if not (conf.env.NATIVE_VULKAN and conf.env.DEST_OS == 'linux'):
			conf.fatal('KTX reader profile requires a Linux native Vulkan client')
		with open('quality/product_profiles/ktx2-linux-tools.json') as profile_file:
			ktx_profile = json.load(profile_file)
		ktx_source = os.path.abspath(conf.options.KTX_SOURCE_ROOT)
		ktx_build = os.path.abspath(conf.options.KTX_BUILD_ROOT)
		ktx_reader_library = os.path.join(ktx_build, ktx_profile['build']['reader_library'])
		try:
			ktx_revision = subprocess.check_output(
				['git', '-C', ktx_source, 'rev-parse', 'HEAD'], text=True).strip()
			ktx_dirty = subprocess.check_output(
				['git', '-C', ktx_source, 'status', '--porcelain', '--untracked-files=no'],
				text=True).strip()
		except (OSError, subprocess.CalledProcessError) as error:
			conf.fatal('Cannot verify KTX-Software source revision: %s' % error)
		if ktx_revision != ktx_profile['dependencies']['ktx_software']['revision']:
			conf.fatal('KTX-Software source revision differs from pinned profile')
		if ktx_dirty:
			conf.fatal('KTX-Software source checkout has tracked changes')
		if not all(os.path.isfile(path) for path in [
			os.path.join(ktx_source, 'lib/include/ktx.h'),
			os.path.join(ktx_source, 'external/dfdutils/KHR/khr_df.h'),
			ktx_reader_library,
			os.path.join(ktx_build, 'CMakeCache.txt')]):
			conf.fatal('Pinned KTX source headers or libktx_read.a are missing')
		with open(os.path.join(ktx_build, 'CMakeCache.txt')) as cache_file:
			ktx_cache = cache_file.read()
		if ('CMAKE_HOME_DIRECTORY:INTERNAL=%s\n' % ktx_source) not in ktx_cache:
			conf.fatal('KTX library build does not belong to the pinned source checkout')
		if ('CMAKE_BUILD_TYPE:STRING=%s\n' % ktx_profile['toolchain']['build_type']) not in ktx_cache:
			conf.fatal('KTX library build type differs from the pinned profile')
		ktx_archive = os.path.basename(ktx_reader_library)
		if not (ktx_archive.startswith('lib') and ktx_archive.endswith('.a')):
			conf.fatal('KTX profile reader library must be a static archive')
		conf.env.KTX_READ_ENABLED = True
		conf.env.INCLUDES_KTXREAD = [os.path.join(ktx_source, 'lib/include'),
			os.path.join(ktx_source, 'external/dfdutils')]
		conf.env.STLIB_KTXREAD = [ktx_archive[3:-2]]
		conf.env.STLIBPATH_KTXREAD = [os.path.dirname(ktx_reader_library)]
		conf.env.LIB_KTXREAD = ['z', 'zstd']
	worldstage_options = [conf.options.OPENUSD_SOURCE_ROOT, conf.options.OPENUSD_BUILD_ROOT,
		conf.options.OPENUSD_INSTALL_ROOT, conf.options.ONETBB_SOURCE_ROOT,
		conf.options.ONETBB_BUILD_ROOT, conf.options.ONETBB_INSTALL_ROOT]
	if any(worldstage_options):
		if not all(worldstage_options) or not (conf.options.TOOLS and conf.env.DEST_OS == 'linux'):
			conf.fatal('World Stage tools require all six OpenUSD/oneTBB roots and Linux --tools')
		worldstage_paths = [os.path.abspath(path) for path in worldstage_options]
		stage_dir = os.path.join(conf.bldnode.abspath(), 'utils', 'worldstage')
		probe = ['python3', 'tools/quality/openusd_host_probe.py',
			'--openusd-source', worldstage_paths[0], '--openusd-build', worldstage_paths[1],
			'--openusd-install', worldstage_paths[2], '--onetbb-source', worldstage_paths[3],
			'--onetbb-build', worldstage_paths[4], '--onetbb-install', worldstage_paths[5],
			'--out', os.path.join(conf.bldnode.abspath(), 'openusd-host-evidence.json')]
		try:
			subprocess.run(probe, check=True)
			subprocess.run(['python3', 'tools/worldstage/generate_schema.py',
				'--openusd-source', worldstage_paths[0],
				'--openusd-install', worldstage_paths[2],
				'--onetbb-install', worldstage_paths[5], '--out', stage_dir], check=True)
		except (OSError, subprocess.CalledProcessError) as error:
			conf.fatal('Pinned World Stage host toolchain failed: %s' % error)
		conf.env.WORLDSTAGE_ENABLED = True
		conf.env.WORLDSTAGE_SCHEMA_DIR = stage_dir
		conf.env.WORLDSTAGE_OPENUSD_SOURCE = worldstage_paths[0]
		conf.env.WORLDSTAGE_OPENUSD_INSTALL = worldstage_paths[2]
		conf.env.WORLDSTAGE_TBB_INSTALL = worldstage_paths[5]
		conf.env.WORLDSTAGE_OPENUSD_LIBDIR = os.path.join(worldstage_paths[2], 'lib')
		conf.env.WORLDSTAGE_TBB_LIBDIR = os.path.join(worldstage_paths[5], 'lib64')
		conf.env.INCLUDES_OPENUSD = [conf.bldnode.abspath(),
			os.path.join(worldstage_paths[2], 'include'),
			os.path.join(worldstage_paths[5], 'include'), '/usr/include/python3.12']
		conf.env.LIBPATH_OPENUSD = [conf.env.WORLDSTAGE_OPENUSD_LIBDIR,
			conf.env.WORLDSTAGE_TBB_LIBDIR]
		conf.env.LIB_OPENUSD = ['usd_usdLux', 'usd_usdGeom', 'usd_usd', 'usd_sdf', 'usd_tf', 'usd_vt',
			'usd_gf', 'usd_plug', 'usd_python', 'tbb', 'python3.12']
	if conf.env.VIDEO_BINK:
		for package, store in [('libavcodec', 'AVCODEC'), ('libavformat', 'AVFORMAT'), ('libavutil', 'AVUTIL')]:
			conf.check_cfg(package=package, uselib_store=store, args=['--cflags', '--libs'])
	if conf.env.DXVK:

		sys.path.insert(0, os.path.abspath('tools/quality'))
		from product_profile import load_profile, check_environment, verify_dependency, ProfileError
		try:
			profile = load_profile(conf.options.PRODUCT_PROFILE)
			for key, value in profile['configure_options'].items():
				actual = {'platform_provider': conf.options.PLATFORM_PROVIDER,
					'render_backend': conf.options.RENDER_BACKEND, 'build_games': conf.options.GAMES}[key]
				if actual != value:
					raise ProfileError('profile requires %s=%s' % (key, value))
			check_environment(profile, cxx=conf.env.CXX)
			verify_dependency(profile, 'dxvk_native', conf.options.DXVK_ROOT)
		except ProfileError as error:
			conf.fatal(str(error))
		dependency = profile['dependencies']['dxvk_native']
		dxvk_root = os.path.abspath(conf.options.DXVK_ROOT)
		if not conf.options.DXVK_ROOT or not os.path.isfile(os.path.join(dxvk_root, 'include/dxvk/d3d9.h')):
			conf.fatal('Vulkan requires --dxvk-root pointing to pinned DXVK Native 2.7.1')
		conf.env.INCLUDES_DXVK = [os.path.join(dxvk_root, dependency['include_directory'])]
		conf.env.LIBPATH_DXVK = [os.path.join(dxvk_root, dependency['library_directory'])]
		conf.env.LIB_DXVK = [dependency['link_library']]
		conf.env.DXVK_ROOT = dxvk_root
		conf.env.PRODUCT_PROFILE = os.path.abspath(conf.options.PRODUCT_PROFILE)
		conf.check_cxx(fragment='#include <d3d9.h>\nint main() { return Direct3DCreate9 ? 0 : 1; }',
			use='DXVK', mandatory=True, msg='Checking DXVK Native D3D9 ABI')

	# The engine debug API links a pinned, ABI-matched protobuf. It is a desktop
	# development feature: store and mobile products must never enable it.
	conf.env.DEBUGAPI = conf.options.DEBUG_API == 'enabled'
	if conf.env.DEBUGAPI:
		if conf.env.DEST_OS != 'linux' or conf.env.ANDROID_SDL3:
			conf.fatal('--debug-api is a Linux desktop development feature')
		if conf.options.TESTS or conf.options.TOOLS:
			conf.fatal('--debug-api belongs to client and dedicated products')
		sys.path.insert(0, os.path.abspath('tools/debugapi'))
		import build_deps
		try:
			pinned = build_deps.verify_consumer_prefix(
				conf.options.PROTOBUF_ROOT or os.path.join('build-deps', 'debugapi', 'prefix'),
				conf.env.CXX[-1])
		except (build_deps.DepsError, OSError, KeyError) as error:
			conf.fatal('debug API: %s' % error)
		conf.env.PROTOC = pinned['protoc']
		conf.env.INCLUDES_PROTOBUF = [pinned['include']]
		conf.env.STLIBPATH_PROTOBUF = [pinned['libdir']]
		conf.env.STLIB_PROTOBUF = pinned['libs']
		# Built with hidden visibility; also keep the archives' symbols out of
		# the engine's dynamic symbol table.
		conf.env.LINKFLAGS_PROTOBUF = ['-Wl,--exclude-libs=' + ':'.join(
			'lib%s.a' % name for name in pinned['libs'])]
		conf.msg('Checking pinned protobuf for the debug API', pinned['prefix'])

	# indicate if we are packaging for Linux/BSD
	if conf.env.DEST_OS != 'android':
		conf.env.LIBDIR = conf.env.PREFIX+'/bin/'
		conf.env.TESTDIR = conf.env.PREFIX+'/tests/'
		conf.env.BINDIR = conf.env.PREFIX
	else:
		conf.env.LIBDIR = conf.env.BINDIR = conf.env.PREFIX

	if conf.options.CCACHE:
		conf.env.CC.insert(0, 'ccache')
		conf.env.CXX.insert(0, 'ccache')

	if conf.options.TESTS:
		conf.add_subproject(projects['tests'])
	elif conf.options.TOOLS:
		tool_projects = projects['tools'] + (LINUX_COMPILER_TOOL_PROJECTS if conf.env.DEST_OS == 'linux' else [])
		conf.add_subproject(tool_projects)
	elif conf.options.DEDICATED:
		if conf.env.DEBUGAPI:
			projects['dedicated'].insert(0, 'debugapi')
		conf.add_subproject(projects['dedicated'])
	else:
		# Desktop conformance harnesses; the Android product packages only runtime modules.
		if conf.env.SDL3 and not conf.env.ANDROID_SDL3:
			projects['game'] += ['unittests/platformtest/sdl3', 'unittests/shaderextensiontest', 'unittests/audioprovidertest',
				'unittests/moduleloadfixture', 'unittests/moduleloadshutdownfixture']
		if conf.env.DXVK:
			projects['game'] += ['materialsystem/shaderapidx9']
		if conf.env.NATIVE_VULKAN:
			projects['game'] += ['materialsystem/shaderapivulkan']
			if not conf.env.ANDROID_SDL3:
				projects['game'] += ['unittests/shaderapivulkantest']
			if conf.env.KTX_READ_ENABLED:
				projects['game'] += ['texturecontainer', 'unittests/texturecontainertest']
		if not conf.env.ANDROID_SDL3:
			projects['game'] += ['unittests/physicstest']
		if conf.env.VIDEO_BINK:
			projects['game'] += ['video/video_bink']
		if conf.env.DEBUGAPI:
			projects['game'].insert(0, 'debugapi')
		conf.add_subproject(projects['game'])

def build(bld):
	os.environ["CCACHE_DIR"] = os.path.abspath('.ccache/'+bld.env.COMPILER_CC+'/'+bld.env.DEST_OS+'/'+bld.env.DEST_CPU)
	if bld.env.DXVK:
		# Ordinary package linkage: install the selected runtime dependency beside
		# the engine, preserving its SONAME. No developer-checkout path is needed.
		from pathlib import Path
		for library in Path(bld.env.LIBPATH_DXVK[0]).glob('libdxvk_d3d9.so*'):
			bld.install_files(bld.env.LIBDIR, [str(library)])

	if bld.env.DEST_OS == 'win32' or (bld.env.DEST_OS == 'android' and not bld.env.ANDROID_SDL3):
		sdl_name = 'SDL2.dll' if bld.env.DEST_OS == 'win32' else 'libSDL2.so'
		sdl_path = os.path.join('lib', bld.env.DEST_OS, bld.env.DEST_CPU, sdl_name)
		bld.install_files(bld.env.LIBDIR, [sdl_path])

	if bld.env.DEST_OS == 'win32':
		projects['game'] += ['utils/bzip2']
		projects['dedicated'] += ['utils/bzip2']

	if bld.env.OPUS or (bld.env.DEST_OS == 'android' and not bld.env.ANDROID_SDL3):
		projects['game'] += ['engine/voice_codecs/opus']
	if bld.env.ANDROID_SDL3:
		projects['game'] += ['utils/bzip2']

	if bld.env.TESTS:
		bld.add_subproject(projects['tests'])
	elif bld.env.TOOLS:
		tool_projects = projects['tools'] + (LINUX_COMPILER_TOOL_PROJECTS if bld.env.DEST_OS == 'linux' else [])
		bld.add_subproject(tool_projects)
	elif bld.env.DEDICATED:
		# First: its protoc rule ends a build group that its consumers follow.
		if bld.env.DEBUGAPI:
			projects['dedicated'].insert(0, 'debugapi')
		bld.add_subproject(projects['dedicated'])
	else:
		# Desktop conformance harnesses; the Android product packages only runtime modules.
		if bld.env.SDL3 and not bld.env.ANDROID_SDL3:
			projects['game'] += ['unittests/platformtest/sdl3', 'unittests/shaderextensiontest', 'unittests/audioprovidertest',
				'unittests/moduleloadfixture', 'unittests/moduleloadshutdownfixture']
		if bld.env.DXVK:
			projects['game'] += ['materialsystem/shaderapidx9']
		if bld.env.NATIVE_VULKAN:
			projects['game'] += ['materialsystem/shaderapivulkan']
			if not bld.env.ANDROID_SDL3:
				projects['game'] += ['unittests/shaderapivulkantest']
			if bld.env.KTX_READ_ENABLED:
				projects['game'] += ['texturecontainer', 'unittests/texturecontainertest']
		if not bld.env.ANDROID_SDL3:
			projects['game'] += ['unittests/physicstest']
		if bld.env.TOGLES:
			projects['game'] += ['togles']
		elif bld.env.GL:
			projects['game'] += ['togl']

		if bld.env.VIDEO_BINK:
			projects['game'] += ['video/video_bink']
		# First: its protoc rule ends a build group that its consumers follow.
		if bld.env.DEBUGAPI:
			projects['game'].insert(0, 'debugapi')
		bld.add_subproject(projects['game'])
