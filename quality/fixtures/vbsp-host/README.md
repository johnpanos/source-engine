# Linux VBSP host-tool smoke fixture

`game/` is a small authored game root with one surface-property manifest and
two materials used by `hammer/gtk/samples/room.vmf`. The host smoke runner
copies both into a temporary directory, invokes the installed `vbsp`, checks
that it emits a v21 BSP with physics, and verifies a byte-exact BSP2
convert/export round trip. It also removes the manifest or its required
`default` property in private copies and requires both failures before output.

The fixture has no texture pixels or skybox, so VBSP logs missing optional
preview assets. This is a compiler/container smoke, not a material-fidelity or
legacy-output oracle. The full R48 and RFC 0008 gates remain open.

`sealed_room.vmf` is the Hammer sample with its center pillar removed and
axis-aligned face texture coordinates. The
sample's player start is inside that pillar, preventing VBSP from producing a
portal file. The original shared texture axes also produce invalid VRAD face
vectors on some walls. The sealed version produces four clusters and four portals. The
VVIS runner checks the independent visibility lump, requires identical bytes
with one and two worker threads, verifies failures for a missing portal file
and unsupported MPI, then performs a byte-exact BSP2 convert/export round trip.
`game/lights.rad` is an empty texlight file for the direct-light fixture.
The VRAD runner requires nonzero LDR lightmap data, world and ambient light
lumps, identical output with one and two threads, preserved visibility and
physics, explicit MPI rejection, and a byte-exact BSP2 round trip. The
independent lighting audit also checks each v21 face's style, bump basis,
average-color prefix, and luxel span against the complete lighting lump. It
does not establish lighting quality or legacy executable parity.
The same fixture also supports a local cross-version comparison with installed
Portal 2 Community Edition tools. Those emit BSP v25; this fork emits v21.
`tools/quality/compiler_cross_version_compare.py` checks the explicitly shared
entity, visibility, ambient, and world-light fields without treating different
version-specific lightmap and physics payloads as equal.

Run after building and installing the Linux `--tools` product:

```sh
python3 tools/quality/vbsp_host_smoke.py --vbsp /path/to/install/vbsp \
  --bsp2tool /path/to/install/bsp2tool \
  --out quality-results/rfc0008-vbsp-host/evidence.json

python3 tools/quality/vvis_host_smoke.py --vbsp /path/to/install/vbsp \
  --vvis /path/to/install/vvis --bsp2tool /path/to/install/bsp2tool \
  --out quality-results/rfc0008-vvis-host/evidence.json

python3 tools/quality/vrad_host_smoke.py --vbsp /path/to/install/vbsp \
  --vvis /path/to/install/vvis --vrad /path/to/install/vrad \
  --bsp2tool /path/to/install/bsp2tool \
  --out quality-results/rfc0008-vrad-host/evidence.json
```
