#!/usr/bin/env python3
"""Build the Linux-hosted iOS cross toolchain from the iOS product profile.

    python3 tools/ios/build_toolchain.py [--profile FILE] [--jobs N] [--fetch-only]

Every input is a pinned archive in the profile's host_toolchain section
(quality/product_profiles/portal-ios-native-vulkan.json): LLVM (clang and
ld64.lld), swift-corelibs-libdispatch and cctools-port (Apple's ld64). Archives
are cached in dependencies/ios/archives and verified by sha256 before use.
Everything installs into one prefix, dependencies/ios/toolchain (gitignored):

    bin/clang, bin/clang++    compile for arm64-apple-ios
    bin/ld64.lld              final app link
    bin/ld64                  Apple's ld64, for 'ld64 -r' module objects
    bin/llvm-*                archive, symbol and Mach-O tools

Each stage is skipped when its stamp matches the stage's inputs. The script
ends with a smoke test that needs no SDK: hidden symbols of a partially linked
arm64-apple-ios module object must come out local, and ld64.lld must link a
program from it. toolchain/manifest.json records the pins and the result.
The iOS SDK is not part of the toolchain; the app build takes it separately.
"""

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile
import tempfile
import urllib.request

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_PROFILE = ROOT / 'quality' / 'product_profiles' / 'portal-ios-native-vulkan.json'
BASE = ROOT / 'dependencies' / 'ios'
ARCHIVES = BASE / 'archives'
SOURCES = BASE / 'src'
# Bump when a stage recipe below changes, to force that stage to rebuild.
RECIPE = 1


class ToolchainError(Exception):
    pass


def log(message):
    print('build_toolchain: ' + message, flush=True)


def sha256(path):
    digest = hashlib.sha256()
    with open(path, 'rb') as stream:
        for block in iter(lambda: stream.read(1 << 20), b''):
            digest.update(block)
    return digest.hexdigest()


def fetch(pin):
    """The verified cached archive of a pin, downloading it when missing."""
    ARCHIVES.mkdir(parents=True, exist_ok=True)
    path = ARCHIVES / pin['cache_archive']
    if not path.is_file():
        log('downloading ' + pin['url'])
        partial = path.with_suffix(path.suffix + '.partial')
        with urllib.request.urlopen(pin['url']) as response, open(partial, 'wb') as out:
            shutil.copyfileobj(response, out)
        partial.rename(path)
    actual = sha256(path)
    if actual != pin['sha256']:
        raise ToolchainError('%s: sha256 %s does not match the pin %s'
                             % (path, actual, pin['sha256']))
    return path


def extract(pin, archive):
    target = SOURCES / pin['extracted_directory']
    if not target.is_dir():
        SOURCES.mkdir(parents=True, exist_ok=True)
        log('extracting ' + archive.name)
        with tarfile.open(archive) as tar:
            tar.extractall(SOURCES, filter='tar')
    if not target.is_dir():
        raise ToolchainError('%s did not contain %s' % (archive, pin['extracted_directory']))
    return target


def run(argv, cwd=None, env=None, logfile=None):
    printable = ' '.join(str(a) for a in argv)
    log('$ ' + printable[:200])
    with open(logfile, 'a') if logfile else open(os.devnull, 'w') as out:
        stream = out if logfile else None
        completed = subprocess.run([str(a) for a in argv], cwd=cwd, env=env,
                                   stdout=stream, stderr=subprocess.STDOUT if logfile else None)
    if completed.returncode:
        raise ToolchainError('command failed (%d): %s%s' % (
            completed.returncode, printable[:200], '; see ' + str(logfile) if logfile else ''))


def stage(name, inputs, prefix, build):
    """Runs build() unless the stage's stamp records these exact inputs."""
    stamp = prefix / '.stamps' / name
    key = json.dumps({'recipe': RECIPE, 'inputs': inputs}, sort_keys=True)
    if stamp.is_file() and stamp.read_text() == key:
        log('%s: up to date' % name)
        return
    build()
    stamp.parent.mkdir(parents=True, exist_ok=True)
    stamp.write_text(key)


def build_llvm(pin, source, prefix, jobs):
    build_dir = BASE / 'llvm-build'
    run(['cmake', '-G', 'Ninja', '-S', source / 'llvm', '-B', build_dir,
         '-DCMAKE_BUILD_TYPE=Release',
         '-DLLVM_ENABLE_PROJECTS=' + ';'.join(pin['projects']),
         '-DLLVM_TARGETS_TO_BUILD=' + ';'.join(pin['targets']),
         '-DLLVM_ENABLE_ASSERTIONS=OFF', '-DLLVM_INCLUDE_TESTS=OFF',
         '-DLLVM_INCLUDE_BENCHMARKS=OFF', '-DLLVM_INCLUDE_EXAMPLES=OFF',
         '-DCLANG_ENABLE_ARCMT=OFF', '-DLLVM_ENABLE_ZSTD=OFF', '-DLLVM_ENABLE_LIBXML2=OFF',
         '-DCMAKE_INSTALL_PREFIX=' + str(prefix),
         '-DLLVM_DISTRIBUTION_COMPONENTS=' + ';'.join(pin['distribution_components'])],
        logfile=BASE / 'llvm-configure.log')
    run(['ninja', '-C', build_dir, '-j', str(jobs), 'install-distribution'],
        logfile=BASE / 'llvm-build.log')


def host_compilers():
    for cc, cxx in (('clang', 'clang++'), ('gcc', 'g++')):
        if shutil.which(cc) and shutil.which(cxx):
            return cc, cxx
    raise ToolchainError('a host C/C++ compiler (clang or gcc) is required')


def build_libdispatch(source, prefix, jobs):
    cc, cxx = host_compilers()
    build_dir = BASE / 'libdispatch-build'
    run(['cmake', '-G', 'Ninja', '-S', source, '-B', build_dir, '-DCMAKE_C_COMPILER=' + cc,
         '-DCMAKE_CXX_COMPILER=' + cxx, '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_TESTING=OFF',
         '-DCMAKE_INSTALL_PREFIX=' + str(prefix), '-DCMAKE_INSTALL_LIBDIR=lib'],
        logfile=BASE / 'libdispatch-build.log')
    run(['ninja', '-C', build_dir, '-j', str(jobs), 'install'], logfile=BASE / 'libdispatch-build.log')


def build_cctools(pin, source, prefix, jobs):
    cc, cxx = host_compilers()
    build_dir = BASE / 'cctools-build'
    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True)
    # configure records a runpath to the prefix's libdispatch in the tools.
    env = dict(os.environ, CC=cc, CXX=cxx)
    run([source / 'cctools' / 'configure', '--prefix=' + str(prefix)] + pin['configure_options']
        + ['--with-libdispatch=' + str(prefix), '--with-libblocksruntime=' + str(prefix)],
        cwd=build_dir, env=env, logfile=BASE / 'cctools-build.log')
    run(['make', '-j', str(jobs)], cwd=build_dir, env=env, logfile=BASE / 'cctools-build.log')
    run(['make', 'install'], cwd=build_dir, env=env, logfile=BASE / 'cctools-build.log')
    ld64 = prefix / 'bin' / 'ld64'
    if ld64.is_symlink() or ld64.exists():
        ld64.unlink()
    ld64.symlink_to('aarch64-apple-darwin-ld')


SMOKE_MODULE = r'''
struct Registry { int count; };
inline Registry &Local() { static Registry r; return r; }
extern "C" void *CreateInterface( const char *, int * ) { return &Local(); }
extern "C" __attribute__(( visibility( "default" ) )) void *StaticModule_smoke_CreateInterface(
	const char *p, int *r ) { return CreateInterface( p, r ); }
'''
SMOKE_PROGRAM = r'''
extern "C" void *StaticModule_smoke_CreateInterface( const char *, int * );
int main() { return StaticModule_smoke_CreateInterface( 0, 0 ) != 0; }
'''


def smoke(prefix, triple):
    """ld64 -r localizes hidden symbols; ld64.lld links the module object."""
    bin_dir = prefix / 'bin'
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        (tmp / 'm.cpp').write_text(SMOKE_MODULE)
        (tmp / 'main.cpp').write_text(SMOKE_PROGRAM)
        for name in ('m', 'main'):
            run([bin_dir / 'clang++', '-target', triple, '-fvisibility=hidden', '-O1', '-c',
                 '%s.cpp' % name, '-o', '%s.o' % name], cwd=tmp)
        run([bin_dir / 'ld64', '-r', '-arch', 'arm64', 'm.o', '-o', 'libsmoke.o'], cwd=tmp)
        symbols = subprocess.run([str(bin_dir / 'llvm-nm'), '-m', 'libsmoke.o'], cwd=tmp,
                                 capture_output=True, text=True, check=True).stdout
        leaked = [line for line in symbols.splitlines()
                  if 'private external' in line and 'was a private external' not in line]
        if leaked:
            raise ToolchainError('ld64 -r left hidden symbols global:\n' + '\n'.join(leaked))
        if ' external _StaticModule_smoke_CreateInterface' not in symbols:
            raise ToolchainError('ld64 -r did not keep the exported entry:\n' + symbols)
        minimum = triple.split('ios')[-1]
        run([bin_dir / 'ld64.lld', '-arch', 'arm64', '-platform_version', 'ios', minimum, minimum,
             '-e', '_main', '-undefined', 'dynamic_lookup', 'main.o', 'libsmoke.o', '-o', 'smoke'],
            cwd=tmp)
        header = subprocess.run([str(bin_dir / 'llvm-otool'), '-l', 'smoke'], cwd=tmp,
                                capture_output=True, text=True, check=True).stdout
        if 'platform 2' not in header and 'platform IOS' not in header.upper():
            raise ToolchainError('the smoke program is not an iOS Mach-O:\n' + header[:400])
    return {'ld64_r_localizes_hidden': True, 'ld64_lld_links_ios': True}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--profile', default=str(DEFAULT_PROFILE))
    parser.add_argument('--jobs', type=int, default=os.cpu_count() or 4)
    parser.add_argument('--fetch-only', action='store_true',
                        help='download and verify the pinned archives, then stop')
    args = parser.parse_args(argv)

    profile = json.loads(Path(args.profile).read_text())
    pins = profile['host_toolchain']
    prefix = ROOT / pins['directory']
    try:
        archives = {name: fetch(pins[name]) for name in ('llvm', 'cctools_port', 'libdispatch')}
        if args.fetch_only:
            log('archives verified')
            return 0
        sources = {name: extract(pins[name], archive) for name, archive in archives.items()}
        prefix.mkdir(parents=True, exist_ok=True)
        stage('llvm', pins['llvm'], prefix,
              lambda: build_llvm(pins['llvm'], sources['llvm'], prefix, args.jobs))
        stage('libdispatch', pins['libdispatch'], prefix,
              lambda: build_libdispatch(sources['libdispatch'], prefix, args.jobs))
        stage('cctools', [pins['cctools_port'], pins['libdispatch']], prefix,
              lambda: build_cctools(pins['cctools_port'], sources['cctools_port'], prefix,
                                    args.jobs))
        result = smoke(prefix, profile['target']['llvm_triple'])
    except (ToolchainError, OSError, subprocess.CalledProcessError) as error:
        print('build_toolchain: error: %s' % error, file=sys.stderr)
        return 1
    manifest = {'profile': profile['id'], 'host_toolchain': pins, 'smoke': result}
    (prefix / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    log('toolchain ready in %s (smoke test passed)' % prefix)
    return 0


if __name__ == '__main__':
    sys.exit(main())
