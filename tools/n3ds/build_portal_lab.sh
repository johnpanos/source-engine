#!/bin/sh
# Builds pica_portals (render/lab/pica_portals): the 3DS portal lab on
# render.device.pica (RFC 0026), as build-3ds-portals/portal_lab.3dsx, with the
# extracted devkitARM (see build-3ds.sh). First builds and runs the host oracle
# of its geometry (portal_math_test.cpp) and its six seeded defects. Run the
# program with tools/n3ds/run_portal_lab.py (Azahar) or on a 3DS; it writes
# sdmc:/portal_lab/.
#   build_portal_lab.sh [FRAMES]   frames per camera-path segment (default 24)
set -e
FRAMES=${1:-24}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
DKP=$ROOT/dependencies/3ds/devkitpro
OUT=$ROOT/build-3ds-portals
SRC=$ROOT/render/lab/pica_portals
mkdir -p "$OUT"

# The host oracle: clean must pass, every seed must fail.
g++ -std=c++20 -O1 -Wall -Wextra -I"$SRC" "$SRC/portal_math_test.cpp" -o "$OUT/portal_math_test"
"$OUT/portal_math_test" > "$OUT/portal_math_test.txt" || { cat "$OUT/portal_math_test.txt"; exit 1; }
grep CONFORMANCE "$OUT/portal_math_test.txt"
for seed in 1 2 3 4 5 6; do
	g++ -std=c++20 -O1 -DPORTAL_MATH_SENSITIVITY=$seed -I"$SRC" "$SRC/portal_math_test.cpp" \
		-o "$OUT/portal_math_test_$seed"
	if "$OUT/portal_math_test_$seed" > /dev/null; then
		echo "seed $seed was not caught by portal_math_test"
		exit 1
	fi
done
echo "portal_math_test: 6 of 6 seeded defects caught"

ARCH="-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft -mword-relocations"
"$DKP/tools/bin/picasso" -o "$OUT/portal_scene.shbin" "$SRC/portal_scene.v.pica"
python3 "$ROOT/tools/n3ds/embed_binary.py" "$OUT/portal_scene.shbin" "$OUT/portal_scene_shbin.c" \
	portal_scene_shbin
"$DKP/devkitARM/bin/arm-none-eabi-gcc" $ARCH -c "$OUT/portal_scene_shbin.c" -o "$OUT/portal_scene_shbin.o"
# The adapter's own presenter program.
"$DKP/tools/bin/picasso" -o "$OUT/present.shbin" "$ROOT/render/device/pica/present.v.pica"
python3 "$ROOT/tools/n3ds/embed_binary.py" "$OUT/present.shbin" "$OUT/present_shbin.c" present_shbin
"$DKP/devkitARM/bin/arm-none-eabi-gcc" $ARCH -c "$OUT/present_shbin.c" -o "$OUT/present_shbin.o"

CXX="$DKP/devkitARM/bin/arm-none-eabi-g++"
SOURCES="$SRC/portal_lab.cpp"
for f in port encoder validation recording pica_format pica_codes; do
	SOURCES="$SOURCES $ROOT/render/device/$f.cpp"
done
SOURCES="$SOURCES $(ls "$ROOT"/render/device/pica/*.cpp)"
"$CXX" $ARCH -std=gnu++20 -O2 -g -D__3DS__ -Wall -Wextra -DPORTAL_LAB_FRAMES=$FRAMES \
	-I"$DKP/libctru/include" -I"$ROOT/public" -I"$SRC" \
	$SOURCES "$OUT/portal_scene_shbin.o" "$OUT/present_shbin.o" \
	-specs=3dsx.specs -L"$DKP/libctru/lib" -lcitro3d -lctru -lm -o "$OUT/portal_lab.elf" \
	> "$OUT/build.log" 2>&1 || { grep -m40 -E 'error|Error' "$OUT/build.log"; exit 1; }
"$DKP/tools/bin/3dsxtool" "$OUT/portal_lab.elf" "$OUT/portal_lab.3dsx"
echo "$OUT/portal_lab.3dsx"
