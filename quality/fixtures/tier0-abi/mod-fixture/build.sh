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
if [ -e "$out/libmod_fixture.so" ] || [ -e "$out/mod_fixture.dll" ]; then
	echo "refusing to rebuild the fixture in $out: it is kept, not rebuilt" >&2
	exit 1
fi
mkdir -p "$out"
if [ "$platform" = windows-x86_64 ]; then
	# MSVC through the pinned msvc-wine toolchain, as a mod's Visual Studio build
	# would: C++14, static CRT, the dedicated tree's defines. Links tier0.lib.
	msvc="$root/dependencies/windows-msvc-wine/14.44-10.0.26100/bin/x64"
	cd "$out"
	"$msvc/cl" /nologo /std:c++14 /O2 /MT /EHsc /GR /LD /w /DWIN32=1 /D_WIN32=1 /D_WINDOWS \
		/DPLATFORM_64BITS=1 /DCOMPILER_MSVC=1 /DCOMPILER_MSVC64=1 /DMSVC=1 /DNDEBUG \
		/D_CRT_SECURE_NO_DEPRECATE /D_CRT_NONSTDC_NO_DEPRECATE /DNO_X360_XDK /D_DLL_EXT=.dll \
		/DNO_MEMOVERRIDE_NEW_DELETE=1 /I"$root/public" /I"$root/public/tier0" /I"$root/common" \
		"$here/mod_fixture.cpp" /Fe:mod_fixture.dll /link "$tier0/tier0.lib"
	rm -f mod_fixture.obj mod_fixture.exp mod_fixture.lib
	git -C "$root" rev-parse HEAD > "$out/built-at-revision.txt"
	echo "built $out/mod_fixture.dll"
	exit 0
fi
case "$platform" in
linux-x86_64) cxx="g++"; arch="-march=core2 -mfpmath=sse -DPLATFORM_64BITS=1" ;;
linux-i386) cxx="g++ -m32"; arch="-march=pentium4 -mfpmath=sse" ;;
android-arm64-v8a) cxx="$root/dependencies/android-ndk/android-ndk-r30/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang++"; arch="-DPLATFORM_64BITS=1 -DANDROID=1" ;;
*) echo "unknown platform $platform" >&2; exit 1 ;;
esac
$cxx -std=c++11 -shared -fPIC -O2 -w $arch -D_GLIBCXX_USE_CXX11_ABI=0 -DLINUX=1 -D_LINUX=1 \
	-DPOSIX=1 -D_POSIX=1 -DPLATFORM_POSIX=1 -DGNUC -DNO_HOOK_MALLOC -D_DLL_EXT=.so \
	-DNO_MEMOVERRIDE_NEW_DELETE=1 -DCOMPILER_GCC=1 -DNDEBUG \
	-I"$root/public" -I"$root/public/tier0" -I"$root/common" "$here/mod_fixture.cpp" \
	-L"$tier0" -ltier0 -Wl,-soname,libmod_fixture.so -o "$out/libmod_fixture.so"
git -C "$root" rev-parse HEAD > "$out/built-at-revision.txt"
echo "built $out/libmod_fixture.so"
