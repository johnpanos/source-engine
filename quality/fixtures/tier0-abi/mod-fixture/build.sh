#!/bin/sh
# Builds the R103 mod fixture ONCE for a platform from this directory's source
# and the Tier 0 headers of the current checkout. The binary is then kept and
# never rebuilt: later Tier 0 builds must load it unchanged. Re-running this is
# only for a new platform, and records the revision it was built at.
#
#   quality/fixtures/tier0-abi/mod-fixture/build.sh linux-x86_64 <dir with libtier0.so>
set -eu
platform=$1
tier0=$2
here=$(cd "$(dirname "$0")" && pwd)
root=$(cd "$here/../../../.." && pwd)
out="$here/$platform"
if [ -e "$out/libmod_fixture.so" ]; then
	echo "refusing to rebuild $out/libmod_fixture.so: the fixture is kept, not rebuilt" >&2
	exit 1
fi
mkdir -p "$out"
case "$platform" in
linux-x86_64) cxx="g++"; arch="-march=core2 -mfpmath=sse -DPLATFORM_64BITS=1" ;;
linux-i386) cxx="g++ -m32"; arch="-march=pentium4 -mfpmath=sse" ;;
android-arm64-v8a) cxx="$root/dependencies/android/android-ndk-r30/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang++"; arch="-DPLATFORM_64BITS=1 -DANDROID=1" ;;
*) echo "unknown platform $platform" >&2; exit 1 ;;
esac
$cxx -std=c++11 -shared -fPIC -O2 -w $arch -D_GLIBCXX_USE_CXX11_ABI=0 -DLINUX=1 -D_LINUX=1 \
	-DPOSIX=1 -D_POSIX=1 -DPLATFORM_POSIX=1 -DGNUC -DNO_HOOK_MALLOC -D_DLL_EXT=.so \
	-DNO_MEMOVERRIDE_NEW_DELETE=1 -DCOMPILER_GCC=1 -DNDEBUG \
	-I"$root/public" -I"$root/public/tier0" -I"$root/common" "$here/mod_fixture.cpp" \
	-L"$tier0" -ltier0 -Wl,-soname,libmod_fixture.so -o "$out/libmod_fixture.so"
git -C "$root" rev-parse HEAD > "$out/built-at-revision.txt"
echo "built $out/libmod_fixture.so"
