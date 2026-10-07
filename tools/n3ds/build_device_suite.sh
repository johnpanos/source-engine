#!/bin/sh
# Builds render.device.v2.pica (RFC 0026): the shared render.device.v2 suite
# against render.device.pica, as build-3ds-device/pica_device.3dsx, with the
# extracted devkitARM (see build-3ds.sh). Run it in Azahar or on a 3DS; it
# writes sdmc:/pica_device.txt and sdmc:/pica_device.done.
#   build_device_suite.sh [N]   N (1-8): a sensitivity build with one seeded
#                               defect (test_device_pica.cpp), pica_device_N.3dsx
set -e
VARIANT=${1:-}
NAME=${VARIANT:+_$VARIANT}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
DKP=$ROOT/dependencies/3ds/devkitpro
OUT=$ROOT/build-3ds-device
FIXTURES=$ROOT/unittests/rendertest/core/device/shaders/pica
mkdir -p "$OUT"
EMBEDDED=
for shader in fullscreen tophalf position shifted colored; do
	"$DKP/tools/bin/picasso" -o "$OUT/$shader.shbin" "$FIXTURES/$shader.v.pica"
	python3 "$ROOT/tools/n3ds/embed_binary.py" "$OUT/$shader.shbin" "$OUT/${shader}_shbin.c" "${shader}_shbin"
done
# The adapter's own presenter program.
"$DKP/tools/bin/picasso" -o "$OUT/present.shbin" "$ROOT/render/device/pica/present.v.pica"
python3 "$ROOT/tools/n3ds/embed_binary.py" "$OUT/present.shbin" "$OUT/present_shbin.c" present_shbin
ARCH="-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft -mword-relocations"
"$DKP/devkitARM/bin/arm-none-eabi-gcc" $ARCH -c "$OUT/present_shbin.c" -o "$OUT/present_shbin.o"
EMBEDDED="$OUT/present_shbin.o"
for shader in fullscreen tophalf position shifted colored; do
	"$DKP/devkitARM/bin/arm-none-eabi-gcc" $ARCH -c "$OUT/${shader}_shbin.c" -o "$OUT/${shader}_shbin.o"
	EMBEDDED="$EMBEDDED $OUT/${shader}_shbin.o"
done
CXX="$DKP/devkitARM/bin/arm-none-eabi-g++"
SOURCES="$ROOT/unittests/rendertest/core/device/test_device_pica.cpp"
for f in port encoder validation recording pica_format pica_codes; do
	SOURCES="$SOURCES $ROOT/render/device/$f.cpp"
done
SOURCES="$SOURCES $(ls "$ROOT"/render/device/pica/*.cpp)"
"$CXX" $ARCH -std=gnu++20 -O2 -g -D__3DS__ -Wall -Wextra \
	-I"$DKP/libctru/include" -I"$ROOT/public" -I"$ROOT/unittests/rendertest/core/device" \
	$SOURCES $EMBEDDED \
	${VARIANT:+-DPICA_SENSITIVITY=$VARIANT} \
	-specs=3dsx.specs -L"$DKP/libctru/lib" -lcitro3d -lctru -lm -o "$OUT/pica_device$NAME.elf" \
	> "$OUT/build.log" 2>&1 || { grep -m40 -E 'error|Error' "$OUT/build.log"; exit 1; }
"$DKP/tools/bin/3dsxtool" "$OUT/pica_device$NAME.elf" "$OUT/pica_device$NAME.3dsx"
echo "$OUT/pica_device$NAME.3dsx"
