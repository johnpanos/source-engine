#!/bin/sh
# Builds tools/n3ds/pica_lab into build-3ds-lab/pica_lab.3dsx with the
# extracted devkitARM (see build-3ds.sh).
set -e
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
DKP=$ROOT/dependencies/3ds/devkitpro
OUT=$ROOT/build-3ds-lab
API=$ROOT/materialsystem/shaderapipica
mkdir -p "$OUT"
"$DKP/tools/bin/picasso" -o "$OUT/fullbright.shbin" "$API/fullbright.v.pica"
python3 "$ROOT/tools/n3ds/embed_binary.py" "$OUT/fullbright.shbin" "$OUT/fullbright_shbin.c" fullbright_shbin
ARCH="-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft -mword-relocations"
"$DKP/devkitARM/bin/arm-none-eabi-g++" $ARCH -std=gnu++20 -O2 -D__3DS__ \
	-I"$DKP/libctru/include" -I"$OUT" -I"$API" \
	"$ROOT/tools/n3ds/pica_lab/main.cpp" "$API/pica_renderer.cpp" "$API/pica_texture.cpp" -x c "$OUT/fullbright_shbin.c" -x none \
	-specs=3dsx.specs -L"$DKP/libctru/lib" -lcitro3d -lctru -lm -o "$OUT/pica_lab.elf"
"$DKP/tools/bin/3dsxtool" "$OUT/pica_lab.elf" "$OUT/pica_lab.3dsx"
echo "$OUT/pica_lab.3dsx"
