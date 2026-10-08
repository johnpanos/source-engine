#!/bin/sh
# Package the 3DS build as a CXI (booted directly by Azahar and the
# harness) and a CIA (installed on a console), both with the exheader from
# tools/n3ds/portal2.rsf: the New 3DS 178 MB memory mode, 804 MHz, L2 cache.
# makerom comes from Project_CTR (dependencies/3ds/src/Project_CTR).
#   package_cia.sh <elf>   (the kiln tree's, tools/n3ds/n3ds_tree.py)
set -e
ELF=${1:?usage: package_cia.sh <hl2_launcher elf>}
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
DKP=$ROOT/dependencies/3ds/devkitpro
MAKEROM=$ROOT/dependencies/3ds/src/Project_CTR/makerom/bin/makerom
OUT=$ROOT/build-3ds
if [ ! -x "$MAKEROM" ]; then
	git clone --depth 1 https://github.com/3DSGuy/Project_CTR.git "$ROOT/dependencies/3ds/src/Project_CTR"
	make -C "$ROOT/dependencies/3ds/src/Project_CTR/makerom" deps
	make -C "$ROOT/dependencies/3ds/src/Project_CTR/makerom"
fi
"$DKP/tools/bin/smdhtool" --create "Portal 2" "Source engine, PICA200 fullbright" "Source Engine" \
	"$DKP/libctru/default_icon.png" "$OUT/Portal2.smdh"
for format in cxi cia; do
	"$MAKEROM" -f $format -o "$OUT/Portal2.$format" -elf "$ELF" \
		-rsf "$ROOT/tools/n3ds/portal2.rsf" -icon "$OUT/Portal2.smdh" -target t -ignoresign
done
ls -la "$OUT/Portal2.cxi" "$OUT/Portal2.cia"
