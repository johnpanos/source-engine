#!/bin/sh
# Viewport smoke test for the GTK Hammer shell (RFC 0002, Q-PRESENTATION-style).
#
# Builds the shell, renders the sample VMF headlessly through the offscreen EGL
# path (the SAME hammergtk::Renderer the interactive viewports use), and asserts
# the produced image is real geometry, not a blank frame: a minimum fraction of
# non-background pixels and the expected solid/triangle count reported by the
# renderer. Requires an EGL-capable GPU/driver (Mesa software rasteriser is fine).
#
# Usage: hammer/gtk/tests/viewport_smoke.sh
# Exit:  0 = pass, non-zero = failure (build, render, or content check).
set -eu

ROOT="$( cd "$( dirname "$0" )/../../.." && pwd )"
BIN="${HAMMERGTK_BIN:-}"
SAMPLE="$ROOT/hammer/gtk/samples/room.vmf"
WORK="$( mktemp -d )"
trap 'rm -rf "$WORK"' EXIT

if [ -z "$BIN" ]; then
	BIN="$WORK/hammer_gtk"
	echo "viewport_smoke: building shell..."
	"$ROOT/hammer/gtk/build.sh" "$BIN" >/dev/null
fi

PPM="$WORK/shot.ppm"
echo "viewport_smoke: rendering $SAMPLE offscreen..."
OUT="$( "$BIN" --screenshot "$PPM" "$SAMPLE" --width 640 --height 480 )"
echo "  $OUT"

# The renderer prints "... N solids, M triangles"; require the expected counts.
echo "$OUT" | grep -q "7 solids" || { echo "FAIL: expected 7 solids"; exit 1; }
echo "$OUT" | grep -q "84 triangles" || { echo "FAIL: expected 84 triangles"; exit 1; }

# Content check: at least 8% of pixels must differ from the clear colour, proving
# the 3D geometry actually rasterised.
python3 - "$PPM" <<'PY'
import sys
p = sys.argv[1]
with open(p, "rb") as f:
    assert f.readline().strip() == b"P6", "not a P6 PPM"
    w, h = map(int, f.readline().split())
    f.readline()
    data = f.read(w * h * 3)
bg = (33, 36, 43)  # clear colour 0.13,0.14,0.17 * 255
non = 0
for i in range(0, len(data), 3):
    if abs(data[i]-bg[0]) + abs(data[i+1]-bg[1]) + abs(data[i+2]-bg[2]) > 18:
        non += 1
frac = non / (w * h)
print(f"  non-background pixels: {frac*100:.1f}%")
if frac < 0.08:
    print("FAIL: too few geometry pixels; viewport looks blank")
    sys.exit(1)
PY

echo "viewport_smoke: PASS"
