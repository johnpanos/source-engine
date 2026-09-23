# RFC 0008 progress: Canonical world data and runtime formats

Updated: 2026-09-23
Portfolio rows: R53 (F1) active; R54 (F2) partial with a compiled World Stage,
Portal material layer and Cycles preview; R55 (F3) has host-tool, reader, native
GPU and GTK preview evidence; R56 has an initial WMSH container slice; R57–R58
are not started. F1 native-run evidence records the engine
revision and working-tree digest; the F3 probe records the pinned KTX source
revision and profile hash. This work overlaps other in-progress tree changes.

This file is the gate-decision record for
[RFC 0008](0008-canonical-world-data-and-runtime-formats.md). Where it
disagrees with the versioned artifacts, the artifacts win:

- the suite manifest row `world.map-container`;
- the [container contract](../unittests/mapcontainertest/contracts/world.map_container.v1.md);
- the local evidence under `quality-results/rfc0008-f1-20260922/` and
  `quality-results/rfc0008-f1-dedicated-manifest-20260923/`.

The evidence directory is local and not versioned; the reproduction commands
below regenerate it.

[RFC 0009](0009-usd-native-map-authoring.md) now defines USD as the editable
source for new maps, with separate world-solid and prop roles. Its native
compiler and editor gates (R59–R60) are planned. F2's current VMF-derived
World Stage is compiled interchange evidence, not native USD authoring; no
RFC 0009 gate is complete. F2 must also validate a compiled-stage fixture with
stable source surface/material identity and no BSP face IDs before closing.
RFC 0008 F8–F11 (R61–R64) are planned modernization gates for native map
spatial data, model assets, visual parity/scalability and direct USD development
iteration. The current WMSH/LMAP preview does not pass those gates.

## Phase status

| Phase | State | Summary |
| --- | --- | --- |
| F1 BSP2 container and map-reader seam | **active (prototype; gate incomplete)** | Container, seam, both readers, pinned v20/v21 lossless corpora, 64-bit sparse tool export, client boot, a 26-map Portal v20 dedicated comparison and five-map v21 client/dedicated engine-seam comparisons work. A derived v19-header map also loads in both products. Authored v19 content, native Portal 2 gameplay and the CI content lane remain (see [Remaining for the F1 gate](#remaining-for-the-f1-gate)) |
| F2 World Stage | partial (world geometry, light and Portal material preview; gate incomplete) | Opt-in `vbsp2` emits BSP, lossless BSP2 and OpenUSD world faces, a lightmap chart table, ordered entities and a `UsdLuxSphereLight` with its compiled style ID. A private Portal-remaster VMF compiles; a separate `UsdPreviewSurface` layer binds two real PBR materials and renders in Blender Cycles at 2048×1152. A preview bridge places Cycles flat and three RNM lightmap samples in a legacy lighting lump. The BSP2 map carries them and renders the flat page in native Vulkan; a controlled legacy DXVK comparison shows the RNM samples change in-game pixels. Pinned standalone Cycles, a canonical baker, accepted SH L1, render lumps and native consumption of directional lighting remain. |
| F4–F5 BSP2 world path | partial (opt-in playable WMSH PBR slice; gate incomplete) | An independently checked WMSH v1 mesh payload from the Portal World Stage is carried in BSP2 beside byte-identical legacy data. The client validates and retains WMSH for the map lifetime; native Vulkan uploads its vertex and index sections once into device-local buffers. An opt-in draw uses the engine's visible leaves, WMSH meshlet references, and Source material passes. A playable USD-derived staircase now binds VTF base color/MRAO and a Cycles HDR KTX2 atlas through the native WMSH PBR pipeline, with a generated material namespace and a real player frame. The GPU pixel fixture covers tangents, material masks, normal maps, linear irradiance and back-face culling. Malformed optional WMSH is rejected while the legacy map stays playable. Canonical lightmap/probe data, automatic map-selected PBR rendering, feature cohorts, visual parity and load-cost gates remain. |
| F6–F7 | planned | Not started |
| F3 KTX2 textures | **partial (host packer, device selection, owned readers and native/GTK consumers)** | Pinned Linux tool builds; thirteen transcode targets validate, UASTC master decode pixels match fixtures, and the packer publishes validated packages from explicit or product-profile device selections. Strict KTX2 and VTF readers produce the same owned image description for a 2D caller cohort; sRGB base color and linear BC7 MRAO feed native pixel tests. Hammer's GTK material catalog previews packaged RGBA8/BGRA8 KTX2 through that reader. Material-system file selection, Hammer compressed-format preview, installed product device identity and ASTC/ETC2/EAC GPU pixels remain. |

F2 requires the R48 host compile-tool gate. Its [current compiler preparation](0007-progress.md#r48-host-compiler-preparation-2026-09-23)
now builds and installs VBSP, local non-MPI VVIS, and local non-MPI VRAD with
their dependencies in the Linux `--tools` product. A self-contained VMF
compiles to v21 BSP and survives a byte-exact BSP2 convert/export round trip;
two missing-surface-property cases fail before output. A sealed four-cluster
fixture additionally compiles through VVIS, yields identical visibility bytes
with one and two threads, rejects missing portals and unsupported MPI, and
survives the same BSP2 round trip. A corrected texture-axis fixture then bakes
nonzero LDR lighting and world/ambient light data through VRAD, with identical
lighting lumps at one and two threads and a byte-exact BSP2 round trip. This
supplies native compiler consumers for F1 while F2 remains planned. The
legacy-output oracle, baker contract, and full R48 gate are still missing.
An installed Portal 2 Community Edition v25 toolchain under Wine produces
byte-identical entity, visibility, and HDR ambient sample lumps and the same
world-light origin/intensity from the sealed fixture. Its different v25
lightmap and physics payloads leave the required same-revision v21 byte
comparison open; see the [cross-version baseline](0007-progress.md#r48-cross-version-executable-baseline-2026-09-23).
An independent v21 lighting-layout audit now runs in both the LDR VRAD smoke
and HDR cross-version comparator. It checks per-face style/bump luxel spans
and exact lighting-lump coverage; a seeded face-offset mutation fails. It is
structural evidence for R48, not the same-revision or World Stage bake gate.
The same audit passes all 106 SHA-pinned Portal 2 v21 maps (259,620 lit faces;
183,075 bumped) with no unreferenced lighting bytes; reproduction and evidence
are in [R48 progress](0007-progress.md#r48-cross-version-executable-baseline-2026-09-23).

### F2 OpenUSD host and schema preparation (2026-09-23)

The [host profile](../quality/product_profiles/openusd-linux-tools.json) pins
OpenUSD v25.11 and its tested oneTBB 2021.9 dependency, with source revisions,
licenses, compiler, Python, and CMake options. Isolated source, build, and
install directories under `/tmp/rfc0008-*` produced the OpenUSD C++ libraries,
Python bindings, `usdcat`, and `usdchecker`. OpenUSD remains outside engine,
dedicated, and mobile products. Its installed `usdGenSchema` wrapper is a
Windows-style launcher on this Linux CMake 4.3 build; the probe invokes the
pinned `usdGenSchema.py` source through Python 3.12 with the installed bindings.
The [build driver](../tools/worldstage/build_host.py) consumes the profile from
exact-revision local checkouts, builds oneTBB first, then OpenUSD, and installs
both into isolated roots. A repeat invocation completed successfully.

The [schema source](../utils/worldstage/schema.usda) defines `SourceWorldAPI`,
`SourceMeshAPI`, `SourceEntityAPI` and `SourceLightAPI` as the owner of the
version and Source face/chart/entity/light mappings.
The [minimal stage fixture](../quality/fixtures/worldstage/minimal.usda) has a
Z-up, 0.0254-meter-per-unit world, one triangle, material/lightmap UVs, and
Source face, smoothing-group, and chart IDs. The
[host probe](../tools/quality/openusd_host_probe.py) verifies exact checkout
revisions and CMake settings, runs `usdchecker` on the fixture and a `.usdc`
round trip, requires a missing-up-axis mutation to fail, generates 20 schema
artifacts, and compiles the five generated non-Python C++ sources in C++20
syntax mode. It passed locally; ignored evidence is
`quality-results/rfc0008-openusd-host-20260923.json`.

```sh
python3 tools/worldstage/build_host.py \
  --openusd-source /tmp/rfc0008-openusd-25.11 \
  --openusd-build /tmp/rfc0008-openusd-build \
  --openusd-install /tmp/rfc0008-openusd-install \
  --onetbb-source /tmp/rfc0008-onetbb-2021.9.0 \
  --onetbb-build /tmp/rfc0008-onetbb-build \
  --onetbb-install /tmp/rfc0008-onetbb-install --jobs 8
python3 tools/quality/openusd_host_probe.py \
  --openusd-source /tmp/rfc0008-openusd-25.11 \
  --openusd-build /tmp/rfc0008-openusd-build \
  --openusd-install /tmp/rfc0008-openusd-install \
  --onetbb-source /tmp/rfc0008-onetbb-2021.9.0 \
  --onetbb-build /tmp/rfc0008-onetbb-build \
  --onetbb-install /tmp/rfc0008-onetbb-install \
  --out quality-results/rfc0008-openusd-host-20260923.json
```

This validates the dependency and schema toolchain. The emitted geometry slice
below advances F2, but F2 and R54 remain open, and R48 remains their hard
prerequisite.

### F2 compiled world geometry slice (2026-09-23)

An opt-in Linux Waf `--tools` profile now builds `vbsp2` beside legacy `vbsp`.
Its typed `sourceWorld` OpenUSD schema is generated into the isolated Waf build
directory and installed with the product; a strict C++20 `vbspworldstage`
adapter owns the stage writer. Legacy VBSP's libstdc++ ABI stays unchanged and
crosses a C-compatible function boundary into the OpenUSD ABI. No OpenUSD
dependency enters the existing `vbsp`, engine, or dedicated products. Installed
ELF `NEEDED` inspection confirms `vbsp` has no OpenUSD dependency while
`vbsp2` links `libsourceWorld` and OpenUSD. The
compiler writes `<map>.geometry.usda` after the BSP: one `UsdGeomMesh` per
compiled world face, fan-triangulated face topology, Source face/smoothing/chart
IDs, material path and UVs, Z-up Source units, and a single atlas chart table.
`SourceEntityAPI` carries each compiled BSP entity's ordered keys and values,
including duplicate keys, under `/World/Entities`. `SourceLightAPI` identifies
the compiled point light under `/World/Lights`. The emitted schema version is 4;
the minimal reviewed stage fixture exercises those APIs. The entity
lump is the input after VBSP's own serialization, so compiler-added and edited
keys are carried into the stage.
It rejects brush-entity models, displacement faces, malformed references and
an overflowing atlas explicitly. BSP, USD and BSP2 publication are not yet atomic.

The sealed room emitted 16 faces, 32 triangles, and three entities;
`usdchecker` passed. Legacy `vbsp` and `vbsp2` produced byte-identical BSP
files from private copies of the same fixture. The
[independent comparator](../tools/quality/worldstage_geometry_compare.py)
reads the BSP face/edge/vertex/model lumps directly and checks every mesh's
vertices, triangle indices, identity, material path, smoothing group, both UV
sets, chart size and chart overlap. It independently parses the serialized BSP
entity lump and checks record order and values, and checks the point light
against its entity. Deactivating a mesh, entity or light, or reversing a face
normal in memory makes that
comparator fail. The independent BSP2 reader verifies the direct `vbsp2`
output and exports the original BSP byte for byte. Local ignored evidence is
`quality-results/rfc0008-worldstage-room-20260923.json`. A second isolated
Waf output and install prefix also passed a clean configure, full install, and
the same smoke with matching BSP and stage hashes; its ignored evidence is
`quality-results/rfc0008-worldstage-clean-20260923.json`.

```sh
WAFLOCK=.lock-waf-rfc0008-worldstage ./waf configure \
  -o build-rfc0008-worldstage --tools -T release \
  --openusd-source-root=/tmp/rfc0008-openusd-25.11 \
  --openusd-build-root=/tmp/rfc0008-openusd-build \
  --openusd-install-root=/tmp/rfc0008-openusd-install \
  --onetbb-source-root=/tmp/rfc0008-onetbb-2021.9.0 \
  --onetbb-build-root=/tmp/rfc0008-onetbb-build \
  --onetbb-install-root=/tmp/rfc0008-onetbb-install \
  --prefix=/tmp/rfc0008-worldstage-install
WAFLOCK=.lock-waf-rfc0008-worldstage ./waf install -j8
python3 tools/quality/worldstage_host_smoke.py \
  --vbsp /tmp/rfc0008-worldstage-install/vbsp \
  --vbsp2 /tmp/rfc0008-worldstage-install/vbsp2 \
  --usdchecker /tmp/rfc0008-openusd-install/bin/usdchecker \
  --python /usr/bin/python3.12 \
  --openusd-python /tmp/rfc0008-openusd-install/lib/python \
  --plugin-root /tmp/rfc0008-worldstage-install/share/sourceWorld \
  --out quality-results/rfc0008-worldstage-room-20260923.json
```

The [smoke runner](../tools/quality/worldstage_host_smoke.py) copies the game
and VMF into temporary directories, compiles with both tools, compares the BSP
bytes, runs `usdchecker` and the semantic comparator, and records content
hashes. The full F2 gate still needs complete authored content semantics, a
versioned corpus, pinned standalone Cycles, lightmap chart ownership across all
supported face types, failure recovery, and the R48 legacy compiler oracle.

### F2 Portal material and playable BSP2 slice (2026-09-23)

The [private fixture generator](../tools/quality/worldstage_render_fixture.py)
places the point light inside the sealed room and replaces its two development
materials with actual `portal_pbr/tile/observation_tilefloor001a` and
`portal_pbr/metal/metalwall_bts_006b` VMTs from
`/home/john/source-engine-portal-runtime/portal-pbr-remastered`. It copies the
matching fallback VMTs and converts private copies of the 2048-pixel PNG masters
to VTF with the installed VTEX. Licensed texture bytes stay in private/ignored
outputs. The source asset tree is not modified by the generator.

`vbsp2` compiled that VMF to BSP, BSP2 and `.geometry.usda`; VVIS and VRAD
completed. The independent World Stage comparator passed all 16 faces,
32 triangles, three entities and one point light, including four seeded missing
prim/normal controls. The first Cycles bake exposed BSP face loops with winding
opposite their plane normals. The emitter now orients triangles to the BSP
plane, and the comparator checks normals and winding independently. The
[Portal material resolver](../tools/quality/worldstage_portal_materials.py)
reads the geometry layer and VMTs, then authors a separate composed USD layer
with two `UsdPreviewSurface` bindings. It maps Source MRAO red to metalness,
green to roughness and the authored bumpmap to a normal input, matching the
engine PBR shader's texture set. Both geometry and composed
layers pass `usdchecker`; the comparator also passes on the composed stage.

The [Cycles inspection renderer](../tools/quality/worldstage_cycles_preview.py)
imports that composed USD stage directly, including its `UsdLuxSphereLight`
and material bindings. Blender Cycles 5.2.1 rendered 2048×1152 at 128 CPU
samples. A 1024×576 light-disabled control has mean RGB approximately
`[0.08, 0.08, 0.08]` versus `[88.28, 80.88, 65.33]` with the light enabled;
the whole frame differs. The system Blender package pairs an OCIO 2.5 config
with an OCIO 2.4.2 library, so the
[private compatibility helper](../tools/quality/blender_ocio_compat.py) makes a
preview-only OCIO 2.4 config. The generated USD light uses `preview-v1` raw
Source intensity; it has not been calibrated against VRAD. Blender Cycles is
inspection evidence, not the RFC 0007 pinned standalone Cycles gate.

The [Cycles atlas prototype](../tools/quality/worldstage_cycles_bake_preview.py)
bakes direct plus indirect diffuse light into a **4096×256 linear EXR**, four
times the Stage chart resolution in each axis, at 64 samples. All 16 chart
rectangles have nonzero light; the EXR has 73,920 nonzero texels. The old
incorrect-winding stage fails this chart control, and disabling its one light
fails with no lit atlas pixels. This is an unencoded preview
of canonical lighting, with no style layers, SH/RNM, probes or runtime packing.

After VRAD, `bsp2tool` converted the lit BSP; independent export matched its
bytes. The [Portal boot runner](../tools/quality/portal_boot.py) now accepts a
private `--content-root` and copies only map/material files into its staged
runtime. The authored BSP2 map booted and rendered in the native Vulkan Portal
client. `map_container_info` reported `kind=bsp2 legacy_version=21` and 39
lumps; the 1920×1080 screenshot shows the Portal textures in the room. Local
ignored evidence includes
`quality-results/rfc0008-portal-usd-materials-cycles-2048.png` and its JSON,
`quality-results/rfc0008-portal-cycles-lightmap-4096x256.exr` and its JSON,
`quality-results/rfc0008-portal-cycles-lightmap-4096x256-preview.png` for visual
inspection,
`quality-results/rfc0008-portal-room-native-final-20260923/evidence.json` and
its screenshot, and `quality-results/rfc0008-worldstage-clean3-20260923.json`.
An isolated clean `--tools` Waf install and repeated host smoke passed.
The Portal boot fixture suite passes 55 tests, including a preflight rejection
that leaves staged content untouched. Pinned stylelint passes on the local
changed files. The whole-tree architecture check still reports 49 new and
three stale occurrences in other in-progress modules; none names this F2 slice.

Reproduction from the repository root with the installed host profile (use
fresh output paths when repeating the commands):

```sh
python3 tools/quality/worldstage_render_fixture.py \
  --out-dir /tmp/rfc0008-portal-room-oriented \
  --portal-assets /home/john/source-engine-portal-runtime/portal-pbr-remastered \
  --vtex /tmp/rfc0008-worldstage-clean3-install/vtex
PXR_PLUGINPATH_NAME=/tmp/rfc0008-worldstage-clean3-install/share/sourceWorld \
  /tmp/rfc0008-worldstage-clean3-install/vbsp2 \
  -game /tmp/rfc0008-portal-room-oriented/game \
  /tmp/rfc0008-portal-room-oriented/sealed_room.vmf
/tmp/rfc0008-worldstage-clean3-install/vvis -threads 1 \
  -game /tmp/rfc0008-portal-room-oriented/game \
  /tmp/rfc0008-portal-room-oriented/sealed_room.bsp
/tmp/rfc0008-worldstage-clean3-install/vrad -bounce 0 -threads 2 \
  -game /tmp/rfc0008-portal-room-oriented/game \
  /tmp/rfc0008-portal-room-oriented/sealed_room.bsp
PYTHONPATH=/tmp/rfc0008-openusd-install/lib/python \
PXR_PLUGINPATH_NAME=/tmp/rfc0008-worldstage-clean3-install/share/sourceWorld \
  /usr/bin/python3.12 tools/quality/worldstage_portal_materials.py \
  --stage /tmp/rfc0008-portal-room-oriented/sealed_room.geometry.usda \
  --portal-assets /home/john/source-engine-portal-runtime/portal-pbr-remastered \
  --out /tmp/rfc0008-portal-room-oriented/material-manifest.json \
  --composed-stage /tmp/rfc0008-portal-room-oriented/portal-preview-v3.usda
PXR_PLUGINPATH_NAME=/tmp/rfc0008-worldstage-clean3-install/share/sourceWorld \
  /tmp/rfc0008-openusd-install/bin/usdchecker \
  /tmp/rfc0008-portal-room-oriented/portal-preview-v3.usda
python3 tools/quality/blender_ocio_compat.py \
  --source-dir /usr/share/blender/5.2/datafiles/colormanagement \
  --out-dir /tmp/rfc0008-ocio24-clean3
OCIO=/tmp/rfc0008-ocio24-clean3/config.ocio blender -b --factory-startup \
  --python-exit-code 9 \
  --python tools/quality/worldstage_cycles_preview.py -- \
  --stage /tmp/rfc0008-portal-room-oriented/portal-preview-v3.usda \
  --import-usd-materials \
  --out quality-results/rfc0008-portal-usd-materials-cycles-2048.png \
  --width 2048 --height 1152 --samples 128 --exposure -5
OCIO=/tmp/rfc0008-ocio24-clean3/config.ocio blender -b --factory-startup \
  --python-exit-code 9 \
  --python tools/quality/worldstage_cycles_bake_preview.py -- \
  --stage /tmp/rfc0008-portal-room-oriented/portal-preview-v3.usda \
  --manifest /tmp/rfc0008-portal-room-oriented/material-manifest.json \
  --require-all-charts-lit \
  --out quality-results/rfc0008-portal-cycles-lightmap-4096x256.exr \
  --width 4096 --height 256 --samples 64
mkdir -p /tmp/rfc0008-portal-playable/maps /tmp/rfc0008-portal-playable/materials
cp -a /tmp/rfc0008-portal-room-oriented/game/materials/. \
  /tmp/rfc0008-portal-playable/materials/
/tmp/rfc0008-worldstage-clean3-install/bsp2tool convert \
  /tmp/rfc0008-portal-room-oriented/sealed_room.bsp \
  /tmp/rfc0008-portal-playable/maps/rfc0008_portal_room.bsp
python3 tools/quality/portal_boot.py --runtime run/runtime-native \
  --content-root /tmp/rfc0008-portal-playable --renderer native-vulkan \
  --headless --map rfc0008_portal_room \
  --console-command map_container_info \
  --console-command 'cmd setpos -160 -160 92' \
  --console-command 'cmd setang -10 45 0' \
  --out quality-results/rfc0008-portal-room-native-final-20260923
```

That baseline playable map uses VRAD's legacy lighting payload. The next
inspection slice places some Cycles light into the game. The game does not read
USD at runtime. F2/R54, R49 and R56 remain open until the canonical baker,
packer and native world consumer close that path.

### F2 Cycles flat-lightmap in-game preview (2026-09-23)

The [preview bridge](../tools/quality/worldstage_legacy_lightmap_preview.py)
checks the geometry Stage, composed Portal material Stage and Cycles EXR hashes
against their manifests. It samples the 4× atlas in Source's 17×17 luxel
charts, converts linear RGB into the legacy RGBExp32 representation and writes
one compiled style's **flat** lighting samples into an already VRAD-lit v21
BSP. It leaves bump basis samples, other styles, face records and all other
lumps intact. The style comes from an independent BSP/Stage comparison. This
fixture's named light is style 32; style 0 contains zero
flat lighting. The bridge is deliberately labeled a preview. It is not the
RFC 0007 directional RNM/SH baker, and the Source light-to-radiance scale is
not calibrated.

All 16 faces received 4,624 flat luxels. Twelve faces still carry VRAD bump
bases. Only legacy lighting lump 8 changed (18,348 bytes). `bsp2tool` converted
the preview to BSP2; the independent exporter recovered the exact modified
BSP. The native Vulkan Portal client booted the BSP2 map with the same 13
Portal material assets and executables as the VRAD baseline. The
[in-game comparator](../tools/quality/worldstage_in_game_compare.py) checked
those identities, the BSP lump difference and both recorded screenshot hashes.
The 1920×1080 frame changed by more than five RGB levels in 99.65% of pixels;
the mean absolute RGB difference is 69.47. Repeating the comparator on the
same BSP/frame failed, as did requests for a missing style or incorrect chart.
Ignored evidence is
`quality-results/rfc0008-portal-cycles-flat-in-game-compare-20260923.json`,
`quality-results/rfc0008-portal-room-cycles-flat-preview-20260923/evidence.json`
and its screenshot. This demonstrates the path **VMF → vbsp2 World Stage →
Portal material layer → Cycles bake → legacy flat-lightmap bridge → BSP2 →
native Vulkan in-game frame** for this fixture. It does not demonstrate direct
USD runtime loading, canonical SH lightmaps, exact bumped RNM, calibrated
lighting, probes or the full RFC 0008 world mesh path.

An additional `--gain 0.15` preview also booted, with the same identities and
only lump 8 changed. Its frame differs from the baseline by 18.04 mean
absolute RGB levels while avoiding the raw bake's strong overexposure. This
gain is an inspection setting, not a measured `LightingPolicy` conversion;
ignored evidence is
`quality-results/rfc0008-portal-cycles-gain015-in-game-compare-20260923.json`.

### F2 compiled light style in World Stage schema 4 (2026-09-23)

The typed `SourceLightAPI` now carries `source:styleId` from the serialized
VBSP entity (default 0, valid range 0–254). VBSP assigned the fixture's named
light style 32. The independent comparator checks the Stage value against that
compiled entity, rejects a seeded style change, and records both BSP and Stage
hashes. The preview bridge verifies this comparison and infers style 32; a
request for style 0 fails before output. This removes the manual style
assignment from the USD → bake → BSP2 preview path. The clean schema 4 host
smoke (`quality-results/rfc0008-worldstage-schema4-clean4-20260923.json`)
also passed a byte-identical `vbsp`/`vbsp2` BSP comparison and BSP2 export,
`usdchecker`, and the geometry/entity/light negative controls.

A fresh private Portal fixture, VVIS/VRAD run, and 64-sample Cycles bake
produced the same BSP and EXR bytes as the earlier fixture. The style-derived
preview BSP matches the earlier in-game preview byte for byte. A new native
Vulkan boot passed, and the in-game comparison
(`quality-results/rfc0008-portal-schema4-in-game-compare-20260923.json`)
again found only legacy lighting lump 8 changed, with identical executables,
13 materials, and camera commands. These results establish the style mapping
for this fixture; multiple-light and animated-style runtime behavior remain
outside this preview.

Reproduce with new private output directories if these paths already exist:

```sh
python3 tools/quality/worldstage_render_fixture.py \
  --out-dir /tmp/rfc0008-portal-room-schema4 \
  --portal-assets /home/john/source-engine-portal-runtime/portal-pbr-remastered \
  --vtex /tmp/rfc0008-worldstage-clean4-install/vtex
PXR_PLUGINPATH_NAME=/tmp/rfc0008-worldstage-clean4-install/share/sourceWorld \
  /tmp/rfc0008-worldstage-clean4-install/vbsp2 \
  -game /tmp/rfc0008-portal-room-schema4/game \
  /tmp/rfc0008-portal-room-schema4/sealed_room.vmf
/tmp/rfc0008-worldstage-clean4-install/vvis -threads 1 \
  -game /tmp/rfc0008-portal-room-schema4/game \
  /tmp/rfc0008-portal-room-schema4/sealed_room.bsp
/tmp/rfc0008-worldstage-clean4-install/vrad -bounce 0 -threads 2 \
  -game /tmp/rfc0008-portal-room-schema4/game \
  /tmp/rfc0008-portal-room-schema4/sealed_room.bsp
PYTHONPATH=/tmp/rfc0008-openusd-install/lib/python \
PXR_PLUGINPATH_NAME=/tmp/rfc0008-worldstage-clean4-install/share/sourceWorld \
  /usr/bin/python3.12 tools/quality/worldstage_geometry_compare.py \
  --bsp /tmp/rfc0008-portal-room-schema4/sealed_room.bsp \
  --stage /tmp/rfc0008-portal-room-schema4/sealed_room.geometry.usda \
  --negative-self-test \
  --out /tmp/rfc0008-portal-room-schema4/geometry-comparison.json
PYTHONPATH=/tmp/rfc0008-openusd-install/lib/python \
PXR_PLUGINPATH_NAME=/tmp/rfc0008-worldstage-clean4-install/share/sourceWorld \
  /usr/bin/python3.12 tools/quality/worldstage_portal_materials.py \
  --stage /tmp/rfc0008-portal-room-schema4/sealed_room.geometry.usda \
  --portal-assets /home/john/source-engine-portal-runtime/portal-pbr-remastered \
  --out /tmp/rfc0008-portal-room-schema4/material-manifest.json \
  --composed-stage /tmp/rfc0008-portal-room-schema4/portal-preview-schema4.usda
OCIO=/tmp/rfc0008-ocio24-clean3/config.ocio blender -b --factory-startup \
  --python-exit-code 9 \
  --python tools/quality/worldstage_cycles_bake_preview.py -- \
  --stage /tmp/rfc0008-portal-room-schema4/portal-preview-schema4.usda \
  --manifest /tmp/rfc0008-portal-room-schema4/material-manifest.json \
  --require-all-charts-lit \
  --out quality-results/rfc0008-portal-schema4-cycles-lightmap.exr \
  --width 4096 --height 256 --samples 64
python3 tools/quality/worldstage_legacy_lightmap_preview.py \
  --bsp /tmp/rfc0008-portal-room-schema4/sealed_room.bsp \
  --geometry-stage /tmp/rfc0008-portal-room-schema4/sealed_room.geometry.usda \
  --material-stage /tmp/rfc0008-portal-room-schema4/portal-preview-schema4.usda \
  --geometry-comparison /tmp/rfc0008-portal-room-schema4/geometry-comparison.json \
  --manifest /tmp/rfc0008-portal-room-schema4/material-manifest.json \
  --exr quality-results/rfc0008-portal-schema4-cycles-lightmap.exr \
  --out /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-style-derived.bsp
mkdir -p /tmp/rfc0008-portal-schema4-playable/maps \
  /tmp/rfc0008-portal-schema4-playable/materials
cp -a /tmp/rfc0008-portal-room-schema4/game/materials/. \
  /tmp/rfc0008-portal-schema4-playable/materials/
/tmp/rfc0008-worldstage-clean4-install/bsp2tool convert \
  /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-style-derived.bsp \
  /tmp/rfc0008-portal-schema4-playable/maps/rfc0008_portal_room.bsp
python3 tools/quality/portal_boot.py --runtime run/runtime-native \
  --content-root /tmp/rfc0008-portal-schema4-playable --renderer native-vulkan \
  --headless --map rfc0008_portal_room \
  --console-command map_container_info \
  --console-command 'cmd setpos -160 -160 92' \
  --console-command 'cmd setang -10 45 0' \
  --out quality-results/rfc0008-portal-schema4-native-20260923
python3 tools/quality/worldstage_in_game_compare.py \
  --baseline-bsp /tmp/rfc0008-portal-room-schema4/sealed_room.bsp \
  --preview-bsp /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-style-derived.bsp \
  --baseline-evidence quality-results/rfc0008-portal-room-native-final-20260923/evidence.json \
  --preview-evidence quality-results/rfc0008-portal-schema4-native-20260923/evidence.json \
  --out quality-results/rfc0008-portal-schema4-in-game-compare-20260923.json
```

### F2 directional Cycles inspection bake (2026-09-23)

The same compiled Stage was baked four times in Blender Cycles 5.2.1: the
geometric normal and Source's three RNM basis directions derived from each
face's texture tangent frame. An analytic distant-light fixture exposed an
important Cycles behavior: setting an unlinked shader normal produced identical
flat and RNM output, and the default bump-map correction reduced the RNM0/flat
ratio from the expected 1.732 to 1.557. Explicitly linking the normal and
disabling that correction produced a measured ratio of 1.7320512. The
[basis oracle](../tools/quality/worldstage_cycles_basis_oracle.py) records the
passing result in `quality-results/rfc0008-cycles-basis-oracle-uncorrected-20260923.json`;
its earlier failing run is retained as a negative feasibility result.

The 4096×256, 64-sample flat/RNM0/RNM1/RNM2 EXRs have distinct hashes. An
[independent BSP tangent comparator](../tools/quality/worldstage_directional_bake_compare.py)
matched all 16 face directions, required each chart to be lit, and rejected a
seeded wrong direction
(`quality-results/rfc0008-portal-directional-4096-compare-20260923.json`).
The [preview bridge](../tools/quality/worldstage_legacy_lightmap_preview.py)
encoded those passes into style 32 of the original VRAD BSP: 4,624 flat
luxels and 10,404 RNM luxels on 12 bumped faces. A flat-only companion uses
the same Cycles flat samples and style averages. The
[delta comparator](../tools/quality/worldstage_rnm_delta_compare.py) proved
that the two BSPs differ only in the three RNM spans of style 32, with a
negative control for a changed non-RNM lighting byte
(`quality-results/rfc0008-portal-rnm-only-delta-20260923.json`). The RNM BSP
round-tripped through BSP2 and booted as a playable Portal map in native
Vulkan (`quality-results/rfc0008-portal-rnm-native-20260923/evidence.json`).
This native path currently samples the flat lightmap page; it does not yet
consume the three RNM pages.

For the legacy renderer, the source-matched DXVK shader pack needed two new
`LightmappedGeneric` static groups and three `sprite_ps20b` groups for this
room. The first boot correctly failed its missing-permutation check. After
rebuilding the pack from that demand log, a legacy BSP with flat Cycles data
and a legacy BSP with the same flat data plus Cycles RNM both loaded. A first
repeat showed mouse motion changed the camera; the boot harness now exposes
`--no-mouse`, using Source's existing `-nomouse` switch. Two flat-map captures
with that switch were byte identical. The
[DXVK frame comparator](../tools/quality/worldstage_rnm_in_game_compare.py)
then verified identical executables, shaders, 13 material assets, camera and
display settings. The RNM-only BSP delta changed 7.87% of image pixels by
more than five RGB levels (mean absolute RGB difference 1.56); see
`quality-results/rfc0008-portal-rnm-dxvk-in-game-compare-20260923.json`.
This demonstrates directional Cycles lighting reaches the existing
`LightmappedGeneric` DXVK path for the playable map. It does not establish
native Vulkan directional sampling or the new PBR shader family.

An [unencoded SH L1 preview](../tools/quality/worldstage_sh_l1_preview.py)
fits the four measured directions at each atlas pixel. Its four input
directions reconstruct within numerical precision, but that is an interpolation
check only. The [analytic held-out oracle](../tools/quality/worldstage_sh_l1_analytic_oracle.py)
fails a provisional maximum error bound of 0.15: its worst direct-light error
is 0.845 and its 95th percentile is 0.277. Even dense least squares into L1
has worst error 0.385 for this analytic grid. See
`quality-results/rfc0008-sh-l1-four-direction-analytic-20260923.json`.
Additional-direction fitting and a declared acceptance tolerance are needed before
encoding canonical LMAP data. The pinned standalone Cycles source was checked
out at `a456b761034dda42c32eef9f4aae0fa5a5c9f604`, but its pinned LFS
dependency bundle and required development libraries are not yet installed;
these runs use Blender's integrated Cycles and are inspection evidence only.

The directional preview can be reproduced from the schema 4 fixture above
with fresh output paths:

```sh
for basis in flat rnm0 rnm1 rnm2; do
  OCIO=/tmp/rfc0008-ocio24-clean3/config.ocio blender -b --factory-startup \
    --python-exit-code 9 --python tools/quality/worldstage_cycles_bake_preview.py -- \
    --stage /tmp/rfc0008-portal-room-schema4/portal-preview-schema4.usda \
    --manifest /tmp/rfc0008-portal-room-schema4/material-manifest.json \
    --basis "$basis" --require-all-charts-lit \
    --out "quality-results/rfc0008-portal-${basis}-exact-4096.exr" \
    --width 4096 --height 256 --samples 64
done
python3 tools/quality/worldstage_directional_bake_compare.py \
  --bsp /tmp/rfc0008-portal-room-schema4/sealed_room.bsp \
  --stage /tmp/rfc0008-portal-room-schema4/portal-preview-schema4.usda \
  --manifest /tmp/rfc0008-portal-room-schema4/material-manifest.json \
  --flat quality-results/rfc0008-portal-flat-exact-4096.exr \
  --rnm0 quality-results/rfc0008-portal-rnm0-exact-4096.exr \
  --rnm1 quality-results/rfc0008-portal-rnm1-exact-4096.exr \
  --rnm2 quality-results/rfc0008-portal-rnm2-exact-4096.exr \
  --negative-self-test \
  --out quality-results/rfc0008-portal-directional-4096-compare-20260923.json
python3 tools/quality/worldstage_sh_l1_analytic_oracle.py \
  --out quality-results/rfc0008-sh-l1-four-direction-analytic-20260923.json
```

The last command is expected to exit nonzero while the four-direction fit
fails its provisional analytic error bound. The checked DXVK artifact pack is
`build-portal-vulkan/shaders/portal-source-matched-rfc0008/manifest.json`;
its invocation and source hashes are recorded there. The three passing
`--no-mouse` boots and their comparator inputs are named in the comparator
evidence above.

### F2 supplemental Cycles SH fit (2026-09-23)

The [versioned direction plan](../quality/fixtures/rfc0008-sh-fit-directions-v1.json)
adds eight normals across two tangent-frame elevations. The
[Cycles batch runner](../tools/quality/worldstage_cycles_supplemental_bakes.py)
baked all eight through the same Portal World Stage at 4096×256 and 64 samples,
with hashes, inputs and commands in
`quality-results/rfc0008-portal-sh12-4096-20260923/supplemental-bakes.json`.
The SH preview now validates each authored normal against an independent
compiled-BSP tangent frame, rejects a seeded wrong supplemental normal and
fits all 12 irradiances by least squares. Its
unencoded coefficients are in
`quality-results/rfc0008-portal-sh-l1-12-preview-4096.npz`.

For analytic clamped-cosine direct lighting, the 12-direction fit reduced the
maximum error from **0.845 to 0.500** and the 95th percentile from **0.277 to
0.194**. A separate Cycles bake at a direction excluded from both fits tested
the actual stage. The [held-out comparator](../tools/quality/worldstage_sh_l1_heldout_compare.py)
verified its direction against the BSP, rejected a seeded wrong direction,
and found chart-pixel mean absolute error **0.257 → 0.207** (19.5% lower) and
95th-percentile error **0.695 → 0.575**. Evidence is
`quality-results/rfc0008-portal-sh-l1-heldout-20260923.json`. These are
linear irradiance units, before encoding. The provisional analytic maximum
bound of 0.15 still fails; dense least squares in L1 itself has a 0.385 worst
error on this grid. More samples alone cannot certify the chosen SH L1 format.
The pixel oracle, encoding decision, light-style layers and native runtime
evaluation remain open.

```sh
OCIO=/tmp/rfc0008-ocio24-clean3/config.ocio \
  python3 tools/quality/worldstage_cycles_supplemental_bakes.py \
  --plan quality/fixtures/rfc0008-sh-fit-directions-v1.json \
  --stage /tmp/rfc0008-portal-room-schema4/portal-preview-schema4.usda \
  --manifest /tmp/rfc0008-portal-room-schema4/material-manifest.json \
  --out-dir quality-results/rfc0008-portal-sh12-4096-20260923 \
  --width 4096 --height 256 --samples 64
python3 tools/quality/worldstage_sh_l1_analytic_oracle.py \
  --direction-plan quality/fixtures/rfc0008-sh-fit-directions-v1.json \
  --out quality-results/rfc0008-sh-l1-12-direction-analytic-20260923.json
python3 tools/quality/worldstage_sh_l1_preview.py \
  --comparison quality-results/rfc0008-portal-directional-4096-compare-20260923.json \
  --geometry-comparison /tmp/rfc0008-portal-room-schema4/geometry-comparison.json \
  --flat quality-results/rfc0008-portal-flat-exact-4096.exr \
  --rnm0 quality-results/rfc0008-portal-rnm0-exact-4096.exr \
  --rnm1 quality-results/rfc0008-portal-rnm1-exact-4096.exr \
  --rnm2 quality-results/rfc0008-portal-rnm2-exact-4096.exr \
  --out quality-results/rfc0008-portal-sh-l1-four-refit-4096.npz
python3 tools/quality/worldstage_sh_l1_preview.py \
  --comparison quality-results/rfc0008-portal-directional-4096-compare-20260923.json \
  --geometry-comparison /tmp/rfc0008-portal-room-schema4/geometry-comparison.json \
  --flat quality-results/rfc0008-portal-flat-exact-4096.exr \
  --rnm0 quality-results/rfc0008-portal-rnm0-exact-4096.exr \
  --rnm1 quality-results/rfc0008-portal-rnm1-exact-4096.exr \
  --rnm2 quality-results/rfc0008-portal-rnm2-exact-4096.exr \
  --direction-plan quality/fixtures/rfc0008-sh-fit-directions-v1.json \
  --extra-dir quality-results/rfc0008-portal-sh12-4096-20260923 \
  --bsp /tmp/rfc0008-portal-room-schema4/sealed_room.bsp \
  --manifest /tmp/rfc0008-portal-room-schema4/material-manifest.json \
  --negative-self-test \
  --out quality-results/rfc0008-portal-sh-l1-12-preview-4096.npz
OCIO=/tmp/rfc0008-ocio24-clean3/config.ocio blender -b --factory-startup \
  --python-exit-code 9 --python tools/quality/worldstage_cycles_bake_preview.py -- \
  --stage /tmp/rfc0008-portal-room-schema4/portal-preview-schema4.usda \
  --manifest /tmp/rfc0008-portal-room-schema4/material-manifest.json \
  --direction-id heldout0 \
  --normal-local 0.6123724356957945 0.6123724356957945 0.5 \
  --require-all-charts-lit \
  --out quality-results/rfc0008-portal-sh-heldout0-4096.exr \
  --width 4096 --height 256 --samples 64
python3 tools/quality/worldstage_sh_l1_heldout_compare.py \
  --bsp /tmp/rfc0008-portal-room-schema4/sealed_room.bsp \
  --stage /tmp/rfc0008-portal-room-schema4/portal-preview-schema4.usda \
  --manifest /tmp/rfc0008-portal-room-schema4/material-manifest.json \
  --heldout quality-results/rfc0008-portal-sh-heldout0-4096.exr \
  --four quality-results/rfc0008-portal-sh-l1-four-refit-4096.npz \
  --twelve quality-results/rfc0008-portal-sh-l1-12-preview-4096.npz \
  --negative-self-test \
  --out quality-results/rfc0008-portal-sh-l1-heldout-20260923.json
```

Use fresh output paths for reruns. The analytic command exits nonzero because
the provisional bound remains unmet. The adjacent JSON evidence files record
the input hashes and outputs.

## F4 WMSH container slice (2026-09-23)

[WMSH v1](../public/mapcontainer/world_mesh_format.h) defines a little-endian
world mesh payload with upload-ready corners and indices, material batches,
bounded meshlets, BSP face IDs, and leaf-to-meshlet references. The
[Stage packer](../tools/quality/worldstage_mesh_pack.py) requires the existing
independent Stage/BSP geometry comparison with matching input hashes before it
emits bytes. The [independent comparator](../tools/quality/worldstage_mesh_compare.py)
parses those bytes separately and checks every triangle, material, UV, normal,
face ID, batch, meshlet, and BSP leaf reference against the source artifacts. A
seeded wrong face ID is rejected. The 16-face Portal room produced 96 corners,
32 triangles, two material batches, 16 meshlets, 29 leaves and 16 leaf
references; the payload is 5,688 bytes. Local evidence is
`quality-results/rfc0008-portal-worldmesh-v1.json` and
`quality-results/rfc0008-portal-worldmesh-v1-compare.json`. Three
[payload contract tests](../tools/quality/tests/test_worldstage_mesh_payload.py)
cover UTF-8 materials, leaf references, truncation, shifted sections, bad
indices, and invalid leaf references.

`bsp2tool pack-world` carries WMSH as an optional 4,096-byte-aligned lump
beside the unmodified legacy v21 payload. The tool and client share the
[map-scoped WMSH validator](../public/mapcontainer/world_mesh.h), which checks
section bounds, finite vertex data by IEEE-754 bits (the profile uses fast-math),
deindexed indices, material batches and paths, meshlet bounds/cones and leaf
references without retaining or allocating from untrusted bytes. The twelve
synthetic malformed cases fail before output. The
independent reader verified WMSH byte equality inside BSP2, all container
hashes, and byte-identical export of the Cycles RNM legacy BSP. Both readers
validated the resulting container. The native Vulkan Portal client loaded it
headlessly, reached the active map, and captured a detailed frame. Compared
with the prior RNM BSP2 frame at the same camera, mean absolute RGB difference
was 0.026 and 0.024% of pixels differed by more than five levels; this is
compatibility evidence, not evidence of WMSH rendering. Boot evidence is
`quality-results/rfc0008-portal-wmsh-native-20260923/evidence.json`.
The [synthetic tool conformance](../tools/quality/worldstage_mesh_tool_conformance.py)
reproduces the carriage and twelve malformed controls without licensed maps;
evidence is `quality-results/rfc0008-worldmesh-tool-conformance-20260923.json`.

The Portal client reads WMSH through the existing BSP2 map source while it is
open, verifies the container hash and payload, then retains owned bytes in the
world model loader. The world brush borrows them until map unload; the
dedicated build neither loads this render lump nor links the validator object.
Both isolated client and dedicated builds passed. The valid packaged room
reported `WMSH ready (96 vertices, 32 triangles, 16 meshlets, 29 leaves)` in
the playable native Vulkan client. A BSP2 package with only a WMSH vertex
changed to NaN retained valid container hashes and identical legacy lumps;
the client reported `WMSH rejected (ok, invalid-vertices)` and still rendered
the legacy map. The [runtime comparator](../tools/quality/worldstage_mesh_runtime_compare.py)
verified the same executable and material assets, both log outcomes, and a
0.0242 mean absolute RGB difference between the two frames. Evidence is
`quality-results/rfc0008-portal-wmsh-runtime-compare-v3-20260923.json`, with
each boot under `quality-results/rfc0008-portal-wmsh-runtime-{valid,invalid}-v3-20260923/`.
The refreshed client/dedicated toolchain boundary passes: two frozen consumers
link, 24 facades compile and 18 probes run, with zero errors in
`quality-results/rfc0008-worldmesh-toolchain-client-ded-20260923.json`. The
separate host-tool invocation check still reports six LZMA C
dialect records as `None` rather than `c-toolchain-default`; its evidence is
`quality-results/rfc0008-worldmesh-toolchain-boundary-20260923.json`.

```sh
PYTHONPATH=/tmp/rfc0008-openusd-install/lib/python /usr/bin/python3.12 \
  tools/quality/worldstage_mesh_pack.py \
  --stage /tmp/rfc0008-portal-room-schema4/sealed_room.geometry.usda \
  --bsp /tmp/rfc0008-portal-room-schema4/sealed_room.bsp \
  --geometry-comparison /tmp/rfc0008-portal-room-schema4/geometry-comparison.json \
  --out quality-results/rfc0008-portal-worldmesh-v1.wmsh
./waf build --targets=bsp2tool
python3 tools/quality/worldstage_mesh_tool_conformance.py \
  --tool build-rfc0008-tools-vbsp/utils/bsp2tool/bsp2tool \
  --out quality-results/rfc0008-worldmesh-tool-conformance-20260923.json
build-rfc0008-tools-vbsp/utils/bsp2tool/bsp2tool pack-world \
  /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-rnm.bsp \
  quality-results/rfc0008-portal-worldmesh-v1.wmsh \
  /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-rnm-wmsh.bsp2
PYTHONPATH=/tmp/rfc0008-openusd-install/lib/python /usr/bin/python3.12 \
  tools/quality/worldstage_mesh_compare.py \
  --wmsh quality-results/rfc0008-portal-worldmesh-v1.wmsh \
  --stage /tmp/rfc0008-portal-room-schema4/sealed_room.geometry.usda \
  --bsp /tmp/rfc0008-portal-room-schema4/sealed_room.bsp \
  --bsp2 /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-rnm-wmsh.bsp2 \
  --container-legacy /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-rnm.bsp \
  --negative-self-test \
  --out quality-results/rfc0008-portal-worldmesh-v1-compare.json
WAFLOCK=.lock-waf-rfc0008-client-current ./waf build -j8
WAFLOCK=.lock-waf-rfc0008-worldmesh-ded ./waf configure \
  --dedicated --build-games=portal --disable-warns -T release \
  -o build-rfc0008-worldmesh-ded \
  --prefix=/tmp/rfc0008-worldmesh-ded-install
WAFLOCK=.lock-waf-rfc0008-worldmesh-ded ./waf build -j8
python3 tools/quality/toolchain_boundary.py check \
  --artifacts build-rfc0008-client-current \
  build-rfc0008-client-current/toolchain-invocations.json \
  build-rfc0008-worldmesh-ded/toolchain-invocations.json \
  --out quality-results/rfc0008-worldmesh-toolchain-client-ded-20260923.json
mkdir -p /tmp/rfc0008-portal-wmsh-playable/maps \
  /tmp/rfc0008-portal-wmsh-playable/materials \
  /tmp/rfc0008-portal-wmsh-bad-playable/maps \
  /tmp/rfc0008-portal-wmsh-bad-playable/materials
cp -a /tmp/rfc0008-portal-room-schema4/game/materials/. \
  /tmp/rfc0008-portal-wmsh-playable/materials/
cp -a /tmp/rfc0008-portal-room-schema4/game/materials/. \
  /tmp/rfc0008-portal-wmsh-bad-playable/materials/
cp /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-rnm-wmsh.bsp2 \
  /tmp/rfc0008-portal-wmsh-playable/maps/rfc0008_portal_room.bsp
python3 tools/quality/worldstage_mesh_negative_map.py \
  --bsp2 /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-rnm-wmsh.bsp2 \
  --out /tmp/rfc0008-portal-wmsh-bad-playable/maps/rfc0008_portal_room.bsp
python3 tools/quality/portal_boot.py --runtime run/runtime-native \
  --build build-rfc0008-client-current \
  --content-root /tmp/rfc0008-portal-wmsh-playable \
  --renderer native-vulkan --headless --map rfc0008_portal_room \
  --console-command map_container_info \
  --console-command 'cmd setpos -160 -160 92' \
  --console-command 'cmd setang -10 45 0' \
  --out quality-results/rfc0008-portal-wmsh-runtime-valid-v3-20260923
python3 tools/quality/portal_boot.py --runtime run/runtime-native \
  --build build-rfc0008-client-current \
  --content-root /tmp/rfc0008-portal-wmsh-bad-playable \
  --renderer native-vulkan --headless --map rfc0008_portal_room \
  --console-command map_container_info \
  --console-command 'cmd setpos -160 -160 92' \
  --console-command 'cmd setang -10 45 0' \
  --out quality-results/rfc0008-portal-wmsh-runtime-invalid-v3-20260923
python3 tools/quality/worldstage_mesh_runtime_compare.py \
  --valid-bsp2 /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-rnm-wmsh.bsp2 \
  --invalid-bsp2 /tmp/rfc0008-portal-wmsh-bad-playable/maps/rfc0008_portal_room.bsp \
  --valid-boot quality-results/rfc0008-portal-wmsh-runtime-valid-v3-20260923/evidence.json \
  --invalid-boot quality-results/rfc0008-portal-wmsh-runtime-invalid-v3-20260923/evidence.json \
  --out quality-results/rfc0008-portal-wmsh-runtime-compare-v3-20260923.json
```

The current WMSH packer is a bounded world-brush slice. It rejects
displacements and brush-entity meshes through the existing Stage compiler, and
the host tool caps the in-memory WMSH input at 512 MiB. The native upload and
draw experiments below use this payload; F4/F5 still need a map-selected path,
canonical lighting, unsupported-feature decisions, and the full world feature
and performance corpus. The optional flag will be revisited when the runtime
requires WMSH.

### F4 native Vulkan upload slice (2026-09-23)

The client now queries a narrow upload capability from the selected material
provider after validating WMSH. The native Vulkan implementation copies the
vertex and index sections through a temporary staging buffer into two
device-local buffers. A transfer-to-vertex-input barrier precedes use; upload
completion is acknowledged before the staging buffer is retired. Replacement
preserves the old pair on failure, and map unload/device shutdown wait for GPU
work before freeing the pair. The dedicated product has no WMSH upload call.
The temporary material adapter bridge is retired when the native world path
receives the map-owned payload through the render-device contract.

The actual Portal BSP2 boot reported `WMSH GPU buffers ready (96 vertices, 96
indices)` after validation. The hash-valid map whose WMSH vertex is NaN
reported `WMSH rejected (ok, invalid-vertices)` and reached a playable frame
without uploading WMSH. The valid/corrupt runtime comparator passed with
byte-identical legacy lumps and the same 13 material assets; its small frame
difference is diagnostic noise while the legacy mesh still draws. Evidence:
`quality-results/rfc0008-portal-wmsh-device-upload-{valid,invalid}-20260923/`
and `quality-results/rfc0008-portal-wmsh-device-upload-compare-20260923.json`.

The real-device native Vulkan conformance now performs exact vertex and index
readback with a nonzero negative control, then checks replacement, failed
replacement, resize, idempotent release, reupload and shutdown: 43 checks,
zero failures on the AMD RADV offscreen profile. Its log is
`quality-results/rfc0008-portal-wmsh-device-upload-valid-20260923/upload-conformance.log`.
The required `VK_LAYER_KHRONOS_validation` was unavailable on this runner, so
the validation-layer lane remains unverified. Both isolated client and
dedicated Waf builds, stylelint, and stylelint's 38 fixtures passed. Changed
archlint has zero new/stale occurrences but still reports eight existing
CAP002 include violations. Two unchanged legacy `SetMode` signatures were
reviewed and their stale ARCH105 fingerprints reconciled in the baseline;
whole-tree baseline verification still reports 47 new and one stale unrelated
legacy occurrence. This slice proves GPU upload and exact bytes, not WMSH
drawing, PVS/meshlet culling, material binding, feature parity, or the F4 gate.

```sh
WAFLOCK=.lock-waf-rfc0008-client-current ./waf build -j8
WAFLOCK=.lock-waf-rfc0008-worldmesh-ded ./waf build -j8
SDL_VIDEODRIVER=offscreen \
  build-rfc0008-client-current/unittests/shaderapivulkantest/native_vulkan_bringup_conformance
python3 tools/quality/portal_boot.py --runtime run/runtime-native \
  --build build-rfc0008-client-current \
  --content-root /tmp/rfc0008-portal-wmsh-playable \
  --renderer native-vulkan --headless --map rfc0008_portal_room \
  --console-command map_container_info \
  --console-command 'cmd setpos -160 -160 92' \
  --console-command 'cmd setang -10 45 0' \
  --out quality-results/rfc0008-portal-wmsh-device-upload-valid-20260923
python3 tools/quality/portal_boot.py --runtime run/runtime-native \
  --build build-rfc0008-client-current \
  --content-root /tmp/rfc0008-portal-wmsh-bad-playable \
  --renderer native-vulkan --headless --map rfc0008_portal_room \
  --console-command map_container_info \
  --console-command 'cmd setpos -160 -160 92' \
  --console-command 'cmd setang -10 45 0' \
  --out quality-results/rfc0008-portal-wmsh-device-upload-invalid-20260923
```

### F4 native Vulkan visible WMSH draw preview (2026-09-23)

The native Vulkan client can now draw the already uploaded 40-byte WMSH
vertices and uint32 indices through a dedicated vertex shader and the selected
Source material pass. The draw record retains the map upload revision so a
queued draw cannot use a released or replaced buffer. The WMSH pipeline uses
the packed triangles' front-face convention; the legacy dynamic-mesh pipeline
keeps its existing convention. The engine resolves material paths at map load,
keeps owned batch/meshlet/leaf-reference arrays until unload, and intersects
each view's visible leaf list with those references before queuing meshlets.
The `r_worldmesh_draw` cheat setting is an explicit diagnostic control: `0`
keeps the legacy draw, `1` overlays WMSH, and `2` substitutes it for the
strictly-above-water opaque chain. It is not the final per-map render-policy
boundary.

The first Portal capture showed a large texture mismatch because native Vulkan
selected the PBR material's legacy fallback VMT, whose base texture differs
from the upsampled Portal texture used for the World Stage UVs. A controlled
private content override made both modes use `LightmappedGeneric` VMTs with
the actual upsampled `portal_pbr/.../basecolor.vtf` assets. The two playable
1920×1080 captures then differed by 2.6333 mean absolute RGB and 35.26% of
pixels by more than five channel values. The
[draw comparator](../tools/quality/worldstage_mesh_draw_compare.py) requires
the same executable hashes, map and material bytes, camera and launch options;
it requires a nonempty visible WMSH queue and rejects a substituted old
fallback-texture image as a negative control. The client log recorded four
visible leaves and 16 queued meshlets for this room. Evidence is
`quality-results/rfc0008-portal-wmsh-visual-{legacy,draw}-20260923/` and
`quality-results/rfc0008-portal-wmsh-visual-compare-20260923.json`; the
negative-control result is
`quality-results/rfc0008-portal-wmsh-visual-negative-20260923.json`.

Client and dedicated Waf builds passed after the draw change. The dedicated
engine and material-system binaries export no WMSH symbols. The real-device
native Vulkan bring-up test now covers a packed, indexed triangle through the
world pipeline as well as upload/readback/lifetime behavior: 49 checks, zero
failures on the AMD RADV offscreen runner. Changed stylelint passes 15 files
with no failures. Changed archlint has zero new and zero stale occurrences but
still reports eight existing CAP002 include violations; whole-tree archlint
still reports unrelated baseline drift, and the loader inventory reports 12
uninstrumented sites outside this slice. The validation layer was unavailable.

This is a geometry and material-binding preview, not the F4/F5 acceptance
gate. It still builds the legacy world on load, has no canonical `LMAP`/`LSTY`
binding, does not prove directional Cycles lighting on WMSH, and cannot claim
native PBR from the fallback material. Meshlet submission is currently one
draw per visible meshlet; batching, feature cohorts, mobile profiles, and
load-cost targets remain. The full path must select WMSH from the map and
provider contract, fail clearly for unsupported required features, preserve
decals/water/displacements/sky/fog/props, and consume canonical lighting.

```sh
WAFLOCK=.lock-waf-rfc0008-client-current ./waf build -j8
WAFLOCK=.lock-waf-rfc0008-worldmesh-ded ./waf build -j8
SDL_VIDEODRIVER=offscreen \
  build-rfc0008-client-current/unittests/shaderapivulkantest/native_vulkan_bringup_conformance
mkdir -p /tmp/rfc0008-portal-wmsh-upsampled/materials \
  /tmp/rfc0008-portal-wmsh-upsampled/maps
cp -a /tmp/rfc0008-portal-room-schema4/game/materials/. \
  /tmp/rfc0008-portal-wmsh-upsampled/materials/
cp /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-rnm-wmsh.bsp2 \
  /tmp/rfc0008-portal-wmsh-upsampled/maps/rfc0008_portal_room.bsp
python3 - <<'PY'
from pathlib import Path
root = Path('/tmp/rfc0008-portal-wmsh-upsampled/materials/portal_pbr')
for name in ('metal/metalwall_bts_006b', 'tile/observation_tilefloor001a'):
    (root / (name + '.vmt')).write_text(
        '"LightmappedGeneric"\n{\n "$basetexture" "portal_pbr/' +
        name + '/basecolor"\n}\n')
PY
python3 tools/quality/portal_boot.py --runtime run/runtime-native \
  --build build-rfc0008-client-current \
  --content-root /tmp/rfc0008-portal-wmsh-upsampled \
  --renderer native-vulkan --headless --map rfc0008_portal_room \
  --console-command 'r_worldmesh_draw 0' \
  --console-command 'cmd setpos -160 -160 92' \
  --console-command 'cmd setang -10 45 0' \
  --out quality-results/rfc0008-portal-wmsh-visual-legacy-20260923
python3 tools/quality/portal_boot.py --runtime run/runtime-native \
  --build build-rfc0008-client-current \
  --content-root /tmp/rfc0008-portal-wmsh-upsampled \
  --renderer native-vulkan --headless --map rfc0008_portal_room \
  --console-command 'r_worldmesh_draw 2' \
  --console-command 'cmd setpos -160 -160 92' \
  --console-command 'cmd setang -10 45 0' \
  --out quality-results/rfc0008-portal-wmsh-visual-draw-20260923
python3 tools/quality/worldstage_mesh_draw_compare.py \
  --legacy-boot quality-results/rfc0008-portal-wmsh-visual-legacy-20260923/evidence.json \
  --world-boot quality-results/rfc0008-portal-wmsh-visual-draw-20260923/evidence.json \
  --out quality-results/rfc0008-portal-wmsh-visual-compare-20260923.json
```

### F4 World Stage geometry and Cycles light in the playable WMSH view (2026-09-23)

The independent [in-game lighting comparator](../tools/quality/worldstage_mesh_lighting_in_game_compare.py)
ties the checked WMSH payload to the emitted USD geometry Stage, the Portal
material Stage, the real upsampled PNGs and their VTEX-produced VTFs, and the
Blender Cycles flat EXR. It verifies the preview bridge's input and output BSP
hashes. A second BSP2 package carries the same WMSH and unchanged legacy
lumps except `L008`: one map has VRAD lighting and the other has the
Cycles-derived flat and RNM samples for the compiled style 32. The native
Vulkan client booted both in `r_worldmesh_draw 2` with the same executable
hashes, material bytes, camera, display settings, and 16 visible queued
meshlets. The two frames differ by **76.04 mean absolute RGB**, with
**99.68%** of pixels differing by more than five channel levels. Reusing the
same frame fails the comparator's negative control.

A separate hash-valid BSP2 negative map changes only the 96 WMSH vertices'
lightmap UV fields to zero; all other WMSH bytes and every other lump remain
identical to the Cycles map. Its playable capture differs from the original
WMSH/Cycles capture by **71.63 mean absolute RGB** and **99.68%** of pixels.
This proves the packed vertex stage's atlas UVs influence the material
lightmap term. The [receipt](../quality-results/rfc0008-portal-wmsh-cycles-lighting-compare-20260923.json)
records hashes for the stages, bake, WMSH, both map boots, real upsampled
texture sources, and the controlled differences.

For this fixture the path is **VMF → compiled OpenUSD World Stage → WMSH →
native Vulkan playable world**, with **Portal-remaster upsampled base textures
and Cycles lighting**. The Cycles light reaches WMSH through the legacy
`L008`/Source lightmap page bridge. This is a positive integration preview,
not native `LMAP`/`LSTY` consumption or a complete RFC 0008 renderer. The
current material override uses `LightmappedGeneric` to select the upsampled
base textures on native Vulkan; `PBRMetalRough` still selects its fallback.
The next render boundary is an owned KTX2 lightmap asset with explicit
map-scoped binding and style evaluation, so WMSH no longer depends on legacy
lightmap allocation. The acceptance gate still requires RNM/SH quality,
probes, feature cohorts, load cost, and the declared platform profiles.

```sh
build-rfc0008-tools-vbsp/utils/bsp2tool/bsp2tool pack-world \
  /tmp/rfc0008-portal-room-schema4/sealed_room.bsp \
  quality-results/rfc0008-portal-worldmesh-v1.wmsh \
  /tmp/rfc0008-portal-wmsh-vrad.bsp2
python3 tools/quality/worldstage_mesh_lighting_in_game_compare.py \
  --vrad-bsp /tmp/rfc0008-portal-room-schema4/sealed_room.bsp \
  --cycles-bsp /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-rnm.bsp \
  --vrad-bsp2 /tmp/rfc0008-portal-wmsh-vrad.bsp2 \
  --cycles-bsp2 /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-rnm-wmsh.bsp2 \
  --wmsh quality-results/rfc0008-portal-worldmesh-v1.wmsh \
  --bridge-evidence /tmp/rfc0008-portal-room-schema4/sealed_room-cycles-rnm.json \
  --mesh-comparison quality-results/rfc0008-portal-worldmesh-v1-compare.json \
  --geometry-stage /tmp/rfc0008-portal-room-schema4/sealed_room.geometry.usda \
  --material-stage /tmp/rfc0008-portal-room-schema4/portal-preview-schema4.usda \
  --cycles-exr quality-results/rfc0008-portal-flat-exact-4096.exr \
  --cycles-bake-evidence quality-results/rfc0008-portal-flat-exact-4096.json \
  --portal-manifest /tmp/rfc0008-portal-room-schema4/material-manifest.json \
  --portal-variant /tmp/rfc0008-portal-room-schema4/variant.json \
  --portal-assets /home/john/source-engine-portal-runtime/portal-pbr-remastered \
  --vrad-boot quality-results/rfc0008-portal-wmsh-vrad-draw-20260923/evidence.json \
  --cycles-boot quality-results/rfc0008-portal-wmsh-uvcontrol-draw-20260923/evidence.json \
  --uvzero-bsp2 /tmp/rfc0008-portal-wmsh-uvzero.bsp2 \
  --uvzero-boot quality-results/rfc0008-portal-wmsh-uvzero-draw-20260923/evidence.json \
  --negative-self-test \
  --out quality-results/rfc0008-portal-wmsh-cycles-lighting-compare-20260923.json
```

### F5 Cycles flat atlas as a checked KTX2 L0 asset (2026-09-23)

The [packer](../tools/quality/worldstage_flat_l0_ktx2.py) checks the exact
Cycles flat EXR against its bake receipt, material Stage, geometry Stage and
World Stage chart manifest. It converts the 4× baked atlas to linear
`R16G16B16A16_SFLOAT`, with opaque alpha and top-left KTX2 orientation; the
scanline flip preserves the Stage's lightmap UV convention. The pinned KTX
tool at revision `90967979cbb7e9401ee2401ff997f30b4b7507d6` validated
the package and extracted **exactly the input half-float bytes**. The atlas
is 4096×256, one mip, 16 charts and 8,388,888 package bytes. Maximum RGB
half-float quantization error against the source EXR is 0.00390625. The
[local receipt](../quality-results/rfc0008-portal-flat-l0-preview.json) records
all source and output hashes.

The pinned engine `ReadKtx2Image` also accepted this generated atlas in the
extended Waf conformance executable: **20 checks, 0 failures**. It reported
RGBA16F, the exact dimensions and one mip; its owned pixel bytes matched an
independent `ktx extract --raw` output before and after the encoded input was
cleared. A deliberately wrong EXR hash in the bake receipt was rejected before
any package was published. This is an authored flat L0 asset and reader proof.
The current playable WMSH view still takes Cycles light through legacy `L008`;
it does not load this KTX2 or select it from a BSP2 asset table. SH L1 style layers,
map-scoped binding, probes and shader evaluation remain F5 work.

```sh
python3 tools/quality/worldstage_flat_l0_ktx2.py \
  --exr quality-results/rfc0008-portal-flat-exact-4096.exr \
  --bake-evidence quality-results/rfc0008-portal-flat-exact-4096.json \
  --manifest /tmp/rfc0008-portal-room-schema4/material-manifest.json \
  --geometry-stage /tmp/rfc0008-portal-room-schema4/sealed_room.geometry.usda \
  --material-stage /tmp/rfc0008-portal-room-schema4/portal-preview-schema4.usda \
  --ktx-source /tmp/rfc0008-ktx-pin \
  --ktx-build /tmp/rfc0008-ktx-pin/build-rfc0008 \
  --out quality-results/rfc0008-portal-flat-l0-preview.ktx2
/tmp/rfc0008-ktx-pin/build-rfc0008/Release/ktx extract --raw \
  quality-results/rfc0008-portal-flat-l0-preview.ktx2 \
  /tmp/rfc0008-portal-flat-l0-preview.rgba16f
WAFLOCK=.lock-waf-rfc0008-ktx-reader ./waf build \
  --targets=ktx2_reader_conformance -j8
build-rfc0008-ktx-reader/unittests/texturecontainertest/ktx2_reader_conformance \
  quality/fixtures/ktx2 \
  quality-results/rfc0008-portal-flat-l0-preview.ktx2 \
  /tmp/rfc0008-portal-flat-l0-preview.rgba16f
```

### F2/F5 visual reference fixture: staircase2 (2026-09-23)

The user-supplied [`staircase2`](../staircase2/) is a PBRT/PLY scene, separate
from the Portal VMF. Its `LICENSE.txt` credits *Modern Hall* by NewSee2l035
under CC BY 3.0. The [fixture adapter](../tools/quality/staircase2_usd_cycles.py)
checks the PBRT camera, all **19** named-material PLY meshes, three authored
texture files and **13** mesh area emitters. It reconstructs their UVs from
the PLY `u`/`v` properties, exports a USD stage, imports that stage in a fresh
Blender scene, and renders the imported scene with Cycles. The stage passed
`usdchecker`; independent USD text inspection found 19 source meshes, 13
emitter meshes, one camera, and the three expected texture asset references.
The exported texture copies were checked byte-for-byte against the fixture.

Blender's USD preview-surface bridge dropped mesh emission, so the adapter
restores source PBRT emitter radiance after USD import. PBRT `coateddiffuse`,
conductor and dielectric materials are approximated by Cycles Principled
shaders; this benchmark does not prove the production engine's PBR family or
full USD material fidelity. The run used Blender 5.2.1 with the checked-in
[OCIO 2.4 fixture](../quality/fixtures/staircase2-ocio/config.ocio). The
adapter converts PBRT Y-up geometry, emitters and camera to the stage's Z-up
coordinates before export.

The [reference comparator](../tools/quality/staircase2_compare.py) checks
source/stage/render hashes and compares the 1024×1024 image with the supplied
Tungsten PNG and EXR. Blender Standard display produced 15.11 mean absolute
RGB difference and 0.955 luminance SSIM. The supplied reference EXR/PNG pair
reveals a Reinhard-power display curve: fitting it on 1/16 of reference pixels
predicts held-out reference pixels within 0.60 RGB levels. Applying that
reference-only curve to the Cycles EXR gives **9.95 mean RGB difference** and
**0.961 SSIM** against the supplied PNG. Linear EXR mean absolute difference
is 0.061. A black-frame negative control fails both image thresholds. The
[receipt](../quality-results/staircase2-parity-1024.json),
[reference-display render](../quality-results/staircase2-cycles-reference-display-1024.png)
and [USD stage](../quality-results/staircase2-reference-parity.usdc) are local
ignored artifacts. This establishes a measured external visual target for
high-resolution mapper materials. The Portal VMF remains the primary playable
in-game pipeline proof; the staircase2 slice below supplements it.

```sh
OCIO="$PWD/quality/fixtures/staircase2-ocio/config.ocio" \
  blender -b --factory-startup --python-exit-code 9 \
    --python tools/quality/staircase2_usd_cycles.py -- \
    --scene staircase2/scene-v4.pbrt \
    --stage quality-results/staircase2-reference-parity.usdc \
    --out quality-results/staircase2-cycles-1024.png \
    --size 1024 --samples 64
/tmp/rfc0008-openusd-install/bin/usdchecker \
  quality-results/staircase2-reference-parity.usdc
python3 tools/quality/staircase2_compare.py \
  --reference staircase2/TungstenRender.png \
  --candidate quality-results/staircase2-cycles-1024.png \
  --reference-exr staircase2/TungstenRender.exr \
  --candidate-exr quality-results/staircase2-cycles-1024.exr \
  --display-candidate quality-results/staircase2-cycles-reference-display-1024.png \
  --evidence quality-results/staircase2-cycles-1024.json \
  --scene staircase2/scene-v4.pbrt \
  --stage quality-results/staircase2-reference-parity.usdc \
  --out quality-results/staircase2-parity-1024.json \
  --max-mae 10 --min-ssim 0.9 --negative-self-test
```

### F2/F5 USD-to-BSP2 playable staircase slice (2026-09-23)

The [collision producer](../tools/quality/staircase2_collision_vmf.py) derives
a sealed Portal VMF from the Z-up USD geometry: six shell brushes, sixteen
upper-flight tread brushes, a player start and a fallback light. The local
`vbsp2`/`vvis`/`vrad` tool build produced the collision/PVS BSP. The
[USD world packer](../tools/quality/usd_worldmesh_pack.py) reads the lighting
stage with OpenUSD, preserves material and `lightmap_st` coordinates,
triangulates the 13 emitter meshes, and packs WMSH with 30,733 visible
triangles, 1,520 meshlets and nine material batches. Its `--out-bsp2` path
invokes `bsp2tool pack-world-lit` and the independent BSP2 reader; repeating
the integrated pack produced byte-identical WMSH and BSP2 outputs. The first
visibility policy references every imported meshlet from every leaf, pending
spatial association.

The [Cycles lightmap bake](../tools/quality/staircase2_lightmap_bake.py)
authored a shared UV layer in a second USD stage and baked a 2048², 64-sample
diffuse atlas. Large architectural meshes own the atlas space; fixture and
glass meshes use unlit/translucent preview materials. The stage passes
`usdchecker`. The [KTX2 packager](../tools/quality/staircase2_lightmap_ktx2.py)
validates a linear RGBA16F package with exact extracted half-float bytes. Its
0.17 linear preview gain is recorded separately from the unmodified Cycles EXR
as temporary Source display calibration, not canonical irradiance policy.

`bsp2tool` carries that KTX2 page as a versioned `LMAP` lump. The engine
verifies its BSP2 content hash before the native Vulkan provider parses and
uploads it. World batches bind this map-scoped texture and release it with the
world mesh. The [content bridge](../tools/quality/staircase2_playable_content.py)
builds VTF/VMT base textures and an alpha-blended glass approximation from the
original PBRT assets. The installed Portal client boot passed with an active
player and a fresh [in-game frame](../quality-results/staircase2-playable-lit-boot-v6/staircase2-lit.png):
the log reports `WMSH LMAP ready (2048 x 2048, linear RGBA16F)` and nine WMSH
material batches. The pinned KTX reader was built as a PIC static archive for
the Vulkan shared module in a separate Waf output profile. A wrong-format
KTX2 fails packaging, and an altered BSP2 byte fails the independent hash
checker. The native boot harness accepts the live native Vulkan provider
marker and retains its negative fixture.

The in-game capture is 1920×1080 despite a requested square mode, so no
registered pixel parity score is claimed against the 1024² Cycles reference.
The frame still lacks the reference's glass refraction, floor reflections,
metal response and smooth lightmap filtering. Collision covers the sealed
floor and upper tread flight, but an automated `+forward` input experiment did
not move the headless player; traversal is not yet attested. This is a working
USD → WMSH/LMAP → BSP2 → native Vulkan preview path, not F2/F5 acceptance or
the requested high visual parity gate. The Portal VMF remains the primary map.

```sh
OCIO="$PWD/quality/fixtures/staircase2-ocio/config.ocio" \
  blender -b -t 16 --python tools/quality/staircase2_lightmap_bake.py -- \
    --stage quality-results/staircase2-reference-parity.usdc \
    --scene staircase2/scene-v4.pbrt \
    --out-stage quality-results/staircase2-lighting-arch-2048.usdc \
    --out-exr quality-results/staircase2-lighting-arch-2048.exr \
    --size 2048 --samples 64
python3 tools/quality/staircase2_lightmap_ktx2.py \
  --exr quality-results/staircase2-lighting-arch-2048.exr \
  --bake-evidence quality-results/staircase2-lighting-arch-2048.exr.json \
  --lighting-stage quality-results/staircase2-lighting-arch-2048.usdc \
  --ktx-tool /tmp/rfc0008-ktx-pin/build-rfc0008/Release/ktx \
  --preview-gain 0.17 \
  --out quality-results/staircase2-lighting-arch-2048-gain017.ktx2
PYTHONPATH=/tmp/rfc0008-openusd-install/lib/python /usr/bin/python3.12 \
  tools/quality/usd_worldmesh_pack.py \
  --stage quality-results/staircase2-lighting-arch-2048.usdc \
  --bsp quality-results/staircase2-collision-v2/staircase2_collision.bsp \
  --material-prefix staircase2 --require-lightmap-uv --include-emitters \
  --lightmap-ktx2 quality-results/staircase2-lighting-arch-2048-gain017.ktx2 \
  --bsp2tool build-rfc0008-tools-vbsp/utils/bsp2tool/bsp2tool \
  --out quality-results/staircase2-integrated-v1.wmsh \
  --out-bsp2 quality-results/staircase2-integrated-v1.bsp2
```

### F4 native world upload adapter extraction (2026-09-23)

The WMSH service now lives in
[`vulkan_world_mesh_upload.cpp`](../materialsystem/shaderapivulkan/vulkan_world_mesh_upload.cpp)
instead of the 7,000-line shader API implementation. It owns request size
validation, context upload/release, and checked HDR LMAP attachment. A narrow
callback in the shader API still submits each batch through its bound Source
material pass; the adapter has no access to that pass's global state. The
module manifest records the adapter's Vulkan and legacy interface dependencies.

The `shaderapivulkan` Waf target built in the staircase client profile.
The [playable boot receipt](../quality-results/staircase2-playable-world-upload-extract-draw-v1/evidence.json)
reports a native Vulkan player frame, 92,199 WMSH vertices and indices, a
2048² linear RGBA16F LMAP, nine material batches, and 1,520 queued visible
meshlets with `r_worldmesh_draw 2`. Style checking and `git diff --check`
passed. Architecture checking still reports its eight existing CAP002 entries,
with zero new and zero stale occurrences. This extraction preserves the
diagnostic WMSH path; `PBRMetalRough` materials still select their legacy
fallback, so it does not close F4/F5 or establish PBR parity in the playable map.

## F1: what exists

### Format (prototype choices, recorded against open decisions 1 and 2)

The layout is specified once, in
[`map_container_format.h`](../public/mapcontainer/map_container_format.h):

- a 64-byte header, then payloads, then a directory of 64-byte entries at the
  end of the file;
- little-endian throughout;
- 4CC lump ids;
- 64-bit offsets and sizes;
- per-lump version, flags (compression bits and a "required" bit), alignment
  (a power of two, at least 16) and a content hash;
- a hash over the whole directory in the header.

| Decision | Prototype choice | Status |
| --- | --- | --- |
| Magic | `"SRCBSP2\x1A"` (8 bytes) | Chosen because it is not `VBSP` and not Quake's `BSP2`/`2PSB`. Legacy readers reject it at the first 4 bytes (`SRCB` ≠ `VBSP`). RFC open decision 1 stays open until a reader test covers every known legacy version. |
| Hash | BLAKE2b-128 (RFC 7693), hash-algorithm id 1 | No new dependency: an in-tree reference implementation, with Python `hashlib` as an independent oracle. The algorithm id allows BLAKE3/XXH3 later. RFC open decision 2 stays open. |
| Compression | None. The zstd and LZMA flag values are reserved; readers reject them on known or required lumps | Waits for a pinned zstd with license record. |
| Legacy carriage | Legacy lump N is stored as `Lnnn`, byte-identical. `LHDR` holds the legacy file size and the verbatim legacy header. `LGAP` holds any nonzero bytes between lumps | Makes legacy → BSP2 → legacy byte-identical by construction. |

### Code

- **Library.** `mapcontainer/` is a strict C++20 static library with no
  tier0/tier1 dependency.
  - It has two readers behind `IMapContainer`: legacy VBSP and BSP2.
  - It provides a writer, lossless conversion and legacy export.
  - The engine includes only the C++11-compatible
    [`map_container.h`](../public/mapcontainer/map_container.h). Tools use the
    C++20 [`map_container_builder.h`](../public/mapcontainer/map_container_builder.h).
- **Engine seam.** Every engine and filesystem reader of `.bsp` headers now goes
  through the container (inventory: grep for `dheader_t`, `IDBSPHEADER`,
  `MINBSPVERSION`):

  | Location | Change |
  | --- | --- |
  | `CMapLoadHelper` (`engine/modelloader.cpp`) | Lump size, offset and version come from `FindLegacyLump`. Every lump read from a BSP2 container is verified against its hash before use. Lump override files still take precedence. |
  | Game lumps | Dictionary offsets are absolute offsets in the legacy file. They are validated against the new `CMapLoadHelper::LumpLegacyOrigin()` and rebased to the stored offset. |
  | `CModelLoader::Map_IsValid`, `CMapListItem::CheckFSHeaderVersion` | Version checks go through the container. |
  | `CRC_MapFile`/`MD5_MapFile` | Share one lump walker. A BSP2 map checksums the same as its legacy source, because the checksum covers lump bytes, not headers. |
  | Filesystem pak mounting (`CBaseFileSystem`, `filesystem_stdio`) | Locates the pak lump through the container. |

  Bridges: [`engine/map_container_file.{h,cpp}`](../engine/map_container_file.h)
  for the engine; the filesystem has a private stdio byte source.
  `CMapLoadHelper::InitFromMemory` (only called from the removed console preload
  path) now fails explicitly instead of reading a raw header.
- **Module registration.** `world.map-container` (`public/mapcontainer/`,
  `mapcontainer/`) is registered in `architecture/modules.json` with only a
  `foundation` edge. archlint reports no capability errors for it.
- **CLI.** `utils/bsp2tool` (Waf `tools` group) provides `info`, `verify`,
  `convert` and `export`. All four commands use file-backed random access;
  conversion scans uncovered legacy bytes and copies payloads in 64 KiB chunks.
  One layout encoder owns header/directory bytes for memory and file writers.
  Export also verifies content and copies each lump in 64 KiB chunks. The CLI
  publishes either output through a temporary file only after success.
- **LGAP validation.** The C++ container reader and independent Python reader
  reject overlapping or out-of-order gap records, overlaps with the legacy
  header/lumps, and nonzero record padding. The streaming exporter checks all
  records before resetting its sink, so malformed gap metadata cannot mutate
  an output file.
- **Diagnostics.** A new `map_container_info` console command reports the
  container kind and reads every game lump back through the dictionary, with a
  CRC of each.

### Evidence (2026-09-22, Linux x86_64, g++ 16.2.1 / clang++ 22.1.8, Python 3.14.7)

| Check | Result |
| --- | --- |
| `world.map-container` shared suite (checks-v1), g++ and clang++ | PASS, 114 checks. It covers: <ul><li>the RFC 7693 vector;</li><li>the contract suite against both readers, with 3 deliberately bad providers detected;</li><li>3 synthetic round trips;</li><li>29 table-driven negative fixtures, each with its named error, plus legacy-reader and writer negatives;</li><li>20 000 seeded mutations (18 972 rejected, 1 028 accepted, none crashing).</li></ul> |
| LGAP validation follow-up, 2026-09-23 | The shared suite passes **154 checks** with g++ and clang++ release builds and Clang ASan+UBSan; its 20 000 mutation run reports no sanitizer errors. The independent reader passes 8 tests, including malformed gap ordering, overlap and padding. Fresh conversion and byte-exact export pass the SHA-pinned Portal v20 **26/26** and Portal 2 v21 **106/106** inventories. A fresh isolated Waf `--tools` profile builds `bsp2tool`, and that binary passes the sparse-offset export fixture. Evidence: `quality-results/rfc0008-lgap-validated-20260923.json`, `rfc0008-lgap-clang-20260923.json`, `rfc0008-lgap-v20-corpus-20260923.json`, `rfc0008-lgap-v21-corpus-20260923.json`, and `rfc0008-lgap-waf-sparse-export-20260923/evidence.json` (local, ignored). |
| Bounded conversion follow-up, 2026-09-23 | The memory and file converters now share one source/sink implementation and one BSP2 layout encoder. The shared suite passes **173 checks** with GCC and Clang ASan+UBSan, including bounded reads and preflight failure without sink reset. The rebuilt Waf tool matches the independent Python writer and exports byte-identically on the pinned v20 **26/26** and v21 **106/106** corpora; sparse-offset export passes. The Python tool suite checks that a malformed conversion leaves an existing destination untouched. Evidence: `quality-results/rfc0008-stream-unified-20260923.json`, `rfc0008-stream-v20-final-20260923.json`, `rfc0008-stream-v21-final-20260923.json`, and `rfc0008-stream-sparse-final-20260923/evidence.json` (local, ignored). |
| Large-map conversion memory spot check | On the SHA-pinned 65,882,168-byte `sp_a2_bts6.bsp`, `/usr/bin/time -f %M` reports 132,744 KiB peak RSS for the previous whole-file converter and 4,252 KiB for the new Waf-built streaming converter. Their BSP2 outputs compare byte-for-byte and have SHA-256 `3280b73b66e44c1725ff559d72542b78abd332dff8b385a7a9560025c7d41410`. This is one local memory measurement, not a cross-profile budget or speed claim. Temporary converted maps were removed. |
| Same suite under ASan+UBSan (clang++; the local g++ has no libasan) | PASS, no reports |
| Suite sensitivity: 5 seeded library defects | Each of four defects (overlap, directory hash, LGAP overlap, alignment checks removed) fails the suite. The `legacyOrigin` defect aborts under ASan. |
| `tools/quality/tests/test_bsp2_reader.py` | PASS, 4 tests. They cover: <ul><li>independent reader round trips;</li><li>12 negative fixtures with the same error names as C++;</li><li>game-lump origin checks;</li><li>C++ vs Python writers agreeing byte-for-byte;</li><li>shared error vocabulary.</li></ul> |
| Corpus: `bsp2_reader.py corpus` over 54 maps (26 Portal v20, 28 chamber-sdk v25; about 7 s) | **54/54 pass.** For every map: <ul><li>the C++ conversion validates in the Python reader;</li><li>the Python and C++ writers produce identical bytes;</li><li>both exporters return the original file byte-for-byte;</li><li>every legacy lump and the game-lump structure match.</li></ul> A converted Portal map is about 3.4 KB larger than its legacy file. |
| Portal 2 v21 corpus, 2026-09-23 | **106/106 pass** through the same independent-reader corpus command. Both writers agree byte-for-byte, both exporters reproduce each original map exactly, and every legacy lump/game-lump structure matches. Evidence: `quality-results/rfc0008-f1-portal2-v21-corpus-20260923.json` (local, ignored). This establishes real v21 content carriage, not native Portal 2 server compatibility. |
| Versioned v21 content check | `quality/baseline.json` now declares `content.bsp2-portal2-v21` with Portal 2 content as a required input. `baseline.py audit --check content.bsp2-portal2-v21 --strict` passes 1/1 checks; local evidence is in `quality-results/rfc0008-f1-v21-baseline-audit-20260923/`. |
| Pinned v20/v21 inventories, 2026-09-23 | `quality/fixtures/bsp2-corpus-v20.json` and `bsp2-corpus-v21.json` fix every expected filename, VBSP version and SHA-256. The independent corpus runner fails missing, extra, changed or non-VBSP maps, and records failure evidence. Strict baseline audits pass 26/26 v20 and 106/106 v21 maps, with local evidence in `quality-results/rfc0008-f1-v20-pinned-baseline-audit-20260923/` and `quality-results/rfc0008-f1-v21-pinned-baseline-audit-20260923/`. |
| Portal client + listen server, null renderer, headless, BSP2 `testchmb_a_00` | Map active, player active at the same position, 326 edicts. The console log is identical to the legacy map's apart from paths and ping. |
| Game lumps through the rebased dictionary (`map_container_info`) | `sprp` v5 (5994 bytes) and `dprp` v4 (12 bytes) load with the same CRCs for legacy and BSP2. |
| Map pak lump | Mounted as the `(map)` search path for both containers. The zip directory must parse before the path is added. |
| Corrupted BSP2 map (one bit flipped in lump 1) | Rejected with `Host_Error: ... lump 1 failed verification (content-hash-mismatch, 57160 bytes)`. The process exits normally, and the Python reader reports the same error name. |
| DXVK rendering, `testchmb_a_01`, fixed camera and exposure, 2 legacy + 2 BSP2 runs | All pass with scene detail. Cross-container differences (mean 0.24–0.47 per channel) match same-container ones (0.17–0.38), and all fall in the same animated region (about 1300 pixels). |
| Dedicated engine library (`--dedicated` tree) | Builds with the container linked statically. `DT_NEEDED` is identical to the `build-rfc0001-dedicated` baseline, so no render or texture libraries were added. |
| Dedicated Portal server, 2026-09-23 | A clean `--dedicated --build-games=portal` build passes. `testchmb_a_01` loads as both VBSP v20 and BSP2; `sprp`/`dprp` CRCs, 227-edict status, and seven server collision/PVS probes match exactly. Five rays hit world geometry. Both runs exit normally with `+quit` (status 0). Evidence: `quality-results/rfc0008-f1-20260923/server-compare.json` and its two console logs. This is one map, not corpus-wide certification. |
| Portal v20 dedicated corpus, 2026-09-23 | **26/26 pass** using the versioned 11-probe manifest in `quality/fixtures/bsp2-dedicated-cases.json` and the SHA-pinned source inventory: both server runs exit 0 and all 11 collision/PVS probes, game-lump CRCs, container metadata, map status and edict counts match for every map. The pinned run is in `quality-results/rfc0008-f1-v20-dedicated-pinned-20260923/evidence.json` (local, ignored). |
| v21 engine-seam experiment, 2026-09-23 | Five real Portal 2 v21 maps (`sp_a1_intro1`, `sp_a2_triple_laser`, `sp_a3_speed_flings`, `sp_a4_tb_intro`, `mp_coop_doors`) load as VBSP and BSP2 in both the Portal dedicated server and the headless Portal client. All 20 processes exit 0. Every map has 12 executed PVS/collision probes, at least one visible cluster and world hit, identical game-lump CRCs, status and edict counts, and an active player in each client run. The versioned manifest is `quality/fixtures/bsp2-v21-engine-cases.json`; local evidence is in `quality-results/rfc0008-f1-v21-dedicated-five-20260923/` and `quality-results/rfc0008-f1-v21-client-five-20260923/`. This exercises the engine container seam with v21 maps; it does not certify Portal 2 gameplay or its missing server target. |
| Pinned native v21 rerun | The five-map client and dedicated checks pass again with `--source-inventory quality/fixtures/bsp2-corpus-v21.json`, which verifies the source map hashes before staging. Evidence: `quality-results/rfc0008-f1-v21-client-pinned-20260923/` and `quality-results/rfc0008-f1-v21-dedicated-pinned-20260923/`. |
| Synthetic v19-header path | `bsp2_v19_derivative.py` changes only bytes 4–7 of the pinned Portal v20 `testchmb_a_01` map to declare v19, leaving every payload byte intact. The generated map and inventory stay in `quality-results/`. Both the client and dedicated server load its VBSP and BSP2 forms and match 11 PVS/collision probes, game lumps and status; evidence is in `quality-results/rfc0008-f1-v19-derived-{client,dedicated}-pinned-20260923/`. **This is a route test, not evidence that authored v19 content is compatible.** |
| Aggregate F1 Linux content/native slice | `bsp2_f1_gate.py` passed all 8 required steps with zero failures: pinned v20 (26 maps) and v21 (106 maps) lossless corpora, v20 dedicated (26 maps), v21 dedicated and client (5 maps each), and the v19 derivative plus dedicated/client comparisons. It records each step's evidence hash in `quality-results/rfc0008-f1-aggregate-20260923/evidence.json` (local, ignored). The generated v19 map copy was removed after the checks. This is a local content/native slice; the full F1 and RFC 0008 gates remain open. |
| 64-bit sparse file inspection | The conformance suite passed 135 checks under GCC and Clang, including a virtual BSP2 payload above 4 GiB whose byte source rejects reads over 64 KiB and whose corrupted payload fails its hash. The Python/C++ cross-reader test creates a sparse file with all payloads above 4 GiB, checks `bsp2tool info` and `verify`, then checks hash-failure detection. A separately converted Portal `testchmb_a_01` map also passed `info` and `verify` after its 48 payloads were shifted above 4 GiB: logical size 4,305,107,248 bytes, allocated size 10,145,792 bytes. The isolated Waf `--tools` profile built and its `bsp2tool` verified that file; local evidence is `quality-results/rfc0008-f1-sparse-20260923/evidence.json`. The next row extends this tool path to export. |
| Streaming export from a sparse real map | `bsp2_sparse_export.py` verifies the SHA-pinned Portal v20 `testchmb_a_01` source, converts it, shifts all 48 payloads above 4 GiB, and checks `info`, `verify`, and byte-identical legacy export. One corrupted byte makes both verification and export fail with `content-hash-mismatch`; export leaves an existing destination untouched and removes its temporary file. The private sparse and converted copies are removed. Local evidence: `quality-results/rfc0008-f1-sparse-export-20260923/evidence.json`. The shared conformance suite adds rejecting sinks and verifies content failure occurs before output reset; 143 checks pass under GCC, Clang, and ASan+UBSan. Separate local compiler evidence is `quality-results/rfc0008-world-map-container-stream-{gcc,clang}-20260923.json`. |
| Aggregate F1 Linux slice with streaming export | A fresh `bsp2_f1_gate.py` run with the Waf-built streaming `bsp2tool` passes **9/9** required steps with zero failures: both pinned corpora, the sparse export, v20 dedicated, v21 dedicated/client, the v19 derivative and its dedicated/client checks. Every step evidence file exists and matches the aggregate SHA-256. Local evidence is `quality-results/rfc0008-f1-aggregate-stream-20260923/evidence.json`; private generated map copies were removed. F1 remains open for authored v19 content, native Portal 2 gameplay and a licensed-content CI runner. |
| Style: new files in full, edited regions of legacy files | Clean (clang-format 22.1.8) |
| archlint `check --all` | No findings in RFC 0008 files. The 7 new and 3 stale ARCH105 items are in other sessions' uncommitted `shaderapivulkan`, `physicstest` and `vphysics_box3d` work. archlint self-tests: 71 pass. |
| Current static checks, 2026-09-23 | `world.map-container` passes 143 checks under GCC, Clang, and ASan+UBSan; the full `tools/quality/tests` suite passes 411 tests, followed by a passing BSP2 reader rerun with the export I/O failure case; client, dedicated, and tools Waf builds pass. `git diff --check` and `baseline.py validate` pass. Changed-region style has one failure in concurrent `material_pixel_conformance.cpp` work, none in RFC 0008 files. `archlint check --all` and `baseline --verify` report 49 new and 3 stale ARCH105 sites outside RFC 0008; `inventory --verify` reports 12 uninstrumented sites in Box3D samples, external Portal2 assets and physics tests. |

`testchmb_a_00` is not usable for pixel comparison. Its scripted wake-up rolls
the view and its countdown sign animates, so frames vary with timing even
between two legacy runs. Use `testchmb_a_01`.

### Reproduction

```sh
# Unit/contract/fuzz suite (both compilers) and sanitizer run
python3 tools/quality/conformance.py check --suite world.map-container --cxx g++
python3 tools/quality/conformance.py check --suite world.map-container --cxx clang++
clang++ -std=c++20 -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all \
  -Ipublic -I. mapcontainer/blake2b.cpp mapcontainer/map_container.cpp \
  unittests/mapcontainertest/test_map_container.cpp -o /tmp/mctest && /tmp/mctest
python3 -m unittest tools/quality/tests/test_bsp2_reader.py -v

# Host tool, including the sparse-offset info/verify test in test_bsp2_reader.py
WAFLOCK=.lock-waf-rfc0008-tools-stream ./waf configure --tools --disable-warns \
  -T release -o build-rfc0008-tools-stream \
  --prefix=/tmp/rfc0008-tools-stream-install
WAFLOCK=.lock-waf-rfc0008-tools-stream ./waf build -j16
python3 tools/quality/bsp2_sparse_export.py \
  --legacy-map run/runtime/portal/maps/testchmb_a_01.bsp \
  --tool build-rfc0008-tools-stream/utils/bsp2tool/bsp2tool \
  --inventory quality/fixtures/bsp2-corpus-v20.json \
  --out quality-results/rfc0008-f1-sparse-export-repro
# Corpus (the direct build is useful for a self-contained independent-reader check)
g++ -std=c++20 -O2 -Ipublic mapcontainer/*.cpp utils/bsp2tool/bsp2tool.cpp -o /tmp/bsp2tool
python3 tools/quality/bsp2_reader.py corpus --tool /tmp/bsp2tool \
  --out quality-results/bsp2-corpus.json <portal>/portal/maps <chamber-sdk>

# Engine: a separate DXVK tree, so shared build profiles stay untouched
./waf configure --platform-provider=sdl3 --render-backend=vulkan \
  --dxvk-root=dependencies/dxvk-native-2.7.1/usr --build-games=portal \
  --disable-warns -T release -o build-rfc0008
(cd build-rfc0008 && ../waf build -k)
python3 tools/quality/stage_runtime.py --build build-rfc0008 --runtime <rt> --base-runtime <runtime>
rm <rt>/portal/maps/testchmb_a_00.bsp   # a symlink to the shared asset
/tmp/bsp2tool convert <portal>/portal/maps/testchmb_a_00.bsp <rt>/portal/maps/testchmb_a_00.bsp
python3 tools/quality/portal_boot.py --runtime <rt> --map testchmb_a_00 --headless \
  --renderer null --console-command map_container_info --out <out>
DXVK_WSI_DRIVER=SDL3 SDL_VIDEODRIVER=wayland python3 tools/quality/portal_boot.py \
  --runtime <rt> --map testchmb_a_01 --console-command "mat_autoexposure_min 1.5" \
  --console-command "mat_autoexposure_max 1.5" --console-command "setpos -486 -380 150" \
  --console-command "setang 10 45 0" --out <out>
```

A windowed DXVK run through `portal_boot.py` needs `DXVK_WSI_DRIVER=SDL3`, which
`run.sh` exports and `portal_boot.py` does not. Without it, DXVK aborts with
`DxvkError` for legacy maps too. Headless DXVK on the offscreen SDL driver also
aborts.

### Dedicated-server follow-up (2026-09-23)

The launcher build had two actual integration faults. `CSys::LoadModules`
called the protected `LoadModule` outside the app-system group, and the group's
public `AddSystem(IAppSystem*)` hid the module-index overload needed for the
physics provider. `CDedicatedAppSystemGroup` now exposes a named physics-module
load and both `AddSystem` overloads. The dedicated target also compiles the
filesystem directly and therefore links `mapcontainer` itself.
The POSIX launcher no longer calls `dlclose` on the dedicated runtime after its
main loop returns: background workers could still execute code from those
modules, causing a shutdown crash. It returns the dedicated process status and
lets process exit release the modules.

`map_container_probe` queries the loaded server collision BSP: start leaf,
cluster, contents, PVS CRC, and a world ray trace. It rejects malformed or
nonfinite coordinates. The independent
[`bsp2_server_compare.py`](../tools/quality/bsp2_server_compare.py) requires a
loaded-map status, successful game lumps, at least three probes, a visible
cluster and a world hit; it compares the legacy and BSP2 observations exactly.
Three unit tests include changed-PVS, changed-trace, missing-probe and vacuous
no-hit negative controls. CI now runs those tests and the independent Python
BSP2 reader tests alongside the shared C++ suite.
The C++ suite now exercises synthetic legacy v19, v20 and v21 files through
both container readers and checks that BSP2's leading bytes differ from VBSP;
GCC and Clang each pass 130 checks on 2026-09-23. This does not provide native
v19 content evidence; the separate v21 engine-seam runs below use real maps.

The native runtime runner, [`bsp2_dedicated.py`](../tools/quality/bsp2_dedicated.py),
stages content and products privately, converts only the staged map, and
records source, map, tool, manifest and installed-product hashes. It fails on
missing required input, failed or timed-out processes, incomplete probes, or
divergent observations. The client mode also requires an active player in both
captures. Its primary manifest covers all 26 installed Portal v20 maps. The
first broad run exposed a pre-existing dedicated-server crash on
chamber 08 and later: the Portal stats display asked `GameUI` for bonus-map
objectives during entity spawn, then crashed inside that UI library. The game
accessor now skips UI access on dedicated servers and initializes absent
objectives to zero. A GDB trace of the legacy `testchmb_a_08` failure is
recorded in the local diagnostic log; after the fix, the map loads under both
containers without `GameUI`. The first broad run also showed that origin-only
probes miss playable space on some maps; the manifest includes rays from each
map's player start. This did not change the comparator's visible-cluster and
world-hit requirements.

The local native run used `testchmb_a_01` in a private Portal runtime with the
same dedicated binary and game module for both containers. The build and
comparison can be reproduced with:

```sh
WAFLOCK=.lock-waf-rfc0008-ded-current ./waf configure --dedicated \
  --build-games=portal --disable-warns -T release -o build-rfc0008-ded-current \
  --prefix=/tmp/rfc0008-ded-current-install
WAFLOCK=.lock-waf-rfc0008-ded-current ./waf build -j8
python3 -m unittest tools/quality/tests/test_bsp2_server_compare.py -v
python3 tools/quality/bsp2_server_compare.py \
  --legacy-log quality-results/rfc0008-f1-20260923/legacy-server.log \
  --bsp2-log quality-results/rfc0008-f1-20260923/bsp2-server.log \
  --out quality-results/rfc0008-f1-20260923/server-compare.json
python3 tools/quality/bsp2_dedicated.py --runtime run/runtime \
  --build build-rfc0008-ded-current --tool build-rfc0008-tools-stream/utils/bsp2tool/bsp2tool \
  --source-inventory quality/fixtures/bsp2-corpus-v20.json \
  --cases quality/fixtures/bsp2-dedicated-cases.json \
  --out quality-results/rfc0008-f1-v20-dedicated-pinned-20260923
python3 tools/quality/bsp2_reader.py corpus \
  --tool build-rfc0008-tools-stream/utils/bsp2tool/bsp2tool \
  --expect-manifest quality/fixtures/bsp2-corpus-v21.json \
  --out quality-results/rfc0008-f1-v21-pinned-corpus-20260923.json \
  run/runtime-p2/portal2/maps
python3 tools/quality/bsp2_dedicated.py --runtime run/runtime \
  --map-source-root run/runtime-p2/portal2/maps \
  --source-inventory quality/fixtures/bsp2-corpus-v21.json \
  --build build-rfc0008-ded-current --tool build-rfc0008-tools-stream/utils/bsp2tool/bsp2tool \
  --cases quality/fixtures/bsp2-v21-engine-cases.json \
  --out quality-results/rfc0008-f1-v21-dedicated-pinned-20260923
WAFLOCK=.lock-waf-rfc0008-client-current ./waf configure \
  --platform-provider=sdl3 --render-backend=native-vulkan \
  --build-games=portal --disable-warns -T release \
  -o build-rfc0008-client-current --prefix=/tmp/rfc0008-client-current-install
WAFLOCK=.lock-waf-rfc0008-client-current ./waf build -j16
python3 tools/quality/bsp2_dedicated.py --runtime run/runtime \
  --map-source-root run/runtime-p2/portal2/maps --product client \
  --source-inventory quality/fixtures/bsp2-corpus-v21.json \
  --build build-rfc0008-client-current --tool build-rfc0008-tools-stream/utils/bsp2tool/bsp2tool \
  --cases quality/fixtures/bsp2-v21-engine-cases.json \
  --out quality-results/rfc0008-f1-v21-client-pinned-20260923
python3 tools/quality/bsp2_v19_derivative.py \
  --portal-maps run/runtime/portal/maps \
  --inventory quality/fixtures/bsp2-corpus-v20.json \
  --out quality-results/rfc0008-f1-v19-derivative-20260923
python3 tools/quality/bsp2_dedicated.py --runtime run/runtime --product dedicated \
  --map-source-root quality-results/rfc0008-f1-v19-derivative-20260923 \
  --source-inventory quality-results/rfc0008-f1-v19-derivative-20260923/inventory.json \
  --build build-rfc0008-ded-current --tool build-rfc0008-tools-stream/utils/bsp2tool/bsp2tool \
  --cases quality/fixtures/bsp2-v19-derived-cases.json \
  --out quality-results/rfc0008-f1-v19-derived-dedicated-pinned-20260923
python3 tools/quality/bsp2_dedicated.py --runtime run/runtime --product client \
  --map-source-root quality-results/rfc0008-f1-v19-derivative-20260923 \
  --source-inventory quality-results/rfc0008-f1-v19-derivative-20260923/inventory.json \
  --build build-rfc0008-client-current --tool build-rfc0008-tools-stream/utils/bsp2tool/bsp2tool \
  --cases quality/fixtures/bsp2-v19-derived-cases.json \
  --out quality-results/rfc0008-f1-v19-derived-client-pinned-20260923
python3 tools/quality/bsp2_f1_gate.py \
  --portal-runtime run/runtime --portal2-runtime run/runtime-p2 \
  --dedicated-build build-rfc0008-ded-current \
  --client-build build-rfc0008-client-current \
  --tool build-rfc0008-tools-stream/utils/bsp2tool/bsp2tool \
  --out quality-results/rfc0008-f1-aggregate-stream-repro
```

The private runtime was staged with `portal_boot.stage_runtime` from the local
Portal content, then overlaid with the dedicated build's shared libraries,
`dedicated_launcher`, and `game/server/libserver.so`. A private map copy was
converted using `bsp2tool`; the same `+map testchmb_a_01 +wait 120 +exec
rfc0008_probes.cfg +quit` invocation ran against each container. The fixture config
contains seven `map_container_probe` lines (including rays from the origin in
both X/Y directions and downwards), `map_container_info`, and `status`. The
comparison evidence records the source revision and dirty digest, log hashes,
observations and exact count. The logs are local and ignored by Git because
licensed game content and runtime output stay outside source control.

## Remaining for the F1 gate

- **Native v19/Portal 2 gameplay content.** The installed Portal gameplay maps
  are v20. A Portal 2 v21 corpus passes lossless carriage and independent read,
  and five v21 maps pass engine collision/vis comparison through the Portal
  server and client. Native Portal 2 gameplay loading remains unavailable: a fresh isolated
  `--dedicated --build-games=portal2` Waf build stops because 66 selected server
  source files are absent. No authored v19 native content was found; the
  header-only derivative checks the engine route but cannot replace that corpus.
  Portal 2 game-specific behavior remains unverified.
- **CI content lane.** The C++ suite and Python reader/comparator self-tests run
  in CI. The client and dedicated runners now treat licensed content as a required input,
  but no CI runner with that content and a built dedicated product is configured.
  This lane remains unverified rather than skipped or inferred from local runs.
- **Scope decisions still open:**
  - Lump override files (`.lmp`) keep working by legacy index. Keying them by
    4CC for new lumps waits until new lumps exist (F4/F5).
  - `IFileSystem` seeks are 32-bit, so the engine bridge refuses offsets past
    2 GiB. The container and `bsp2tool info/verify/export` support 64-bit
    offsets; `bsp2tool convert` now uses bounded file I/O and 64-bit offsets.
  - Compile tools (`utils/common/bsplib`, `vbsp`, `vvis`, `vrad`, `bspzip`)
    and Hammer still read and write only VBSP. A BSP2 package is produced after
    compile by `bsp2tool convert`, until F2 (`vbsp2`) writes it directly.
  - `utils/hlfaceposer` also reads raw BSP headers. It is Windows-only and out
    of F1 scope.
  - Big-endian console `PSBV` maps are rejected, the same as the removed
    console paths.

## F3 KTX2 host-tool feasibility (2026-09-23)

The [Linux host-tool profile](../quality/product_profiles/ktx2-linux-tools.json)
owns the exact KTX-Software Git revision, GCC/CMake versions, CMake options,
fixture tolerances and target formats. A source checkout at that revision built
the static `libktx.a` and `ktx` CLI in an isolated CMake/Ninja output tree.
`tools/quality/ktx2_probe.py` verifies the checkout and build settings before
it creates 8×8 raw LDR/HDR fixtures, encodes UASTC masters, and transcodes ten
BC, ASTC, ETC2/EAC and HDR targets. Every output passes `ktx validate` and has
the expected Vulkan format in `ktx info`; the LDR and HDR masters decode back
to the authored uniform-block pixels with zero observed channel error. A bad
KTX2 magic fails validation, and a wrong source revision produces failure
evidence. The three comparator/profile negative-control tests pass. Local
evidence is `quality-results/rfc0008-f3-ktx-linux-feasibility-20260923.json`
and `rfc0008-f3-ktx-wrong-pin-negative-20260923.json` (ignored by Git).

Reproduce the pinned host-tool check without changing an engine Waf profile:

```sh
ktx_revision=$(python3 -c 'import json; print(json.load(open("quality/product_profiles/ktx2-linux-tools.json"))["dependencies"]["ktx_software"]["revision"])')
git init /tmp/rfc0008-ktx-pin
git -C /tmp/rfc0008-ktx-pin remote add origin https://github.com/KhronosGroup/KTX-Software.git
git -C /tmp/rfc0008-ktx-pin fetch --depth 1 origin "$ktx_revision"
git -C /tmp/rfc0008-ktx-pin checkout --detach FETCH_HEAD
cmake -S /tmp/rfc0008-ktx-pin -B /tmp/rfc0008-ktx-pin/build-rfc0008 -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DKTX_FEATURE_TOOLS=ON \
  -DKTX_FEATURE_TESTS=OFF -DKTX_FEATURE_LOADTEST_APPS=OFF
cmake --build /tmp/rfc0008-ktx-pin/build-rfc0008 --target ktxtools -j8
python3 tools/quality/ktx2_probe.py --source /tmp/rfc0008-ktx-pin \
  --build /tmp/rfc0008-ktx-pin/build-rfc0008 \
  --out quality-results/rfc0008-f3-ktx-linux-feasibility-20260923.json
python3 -m unittest tools/quality/tests/test_ktx2_probe.py -v
```

The [host packer](../tools/texture/ktx2_pack.py) uses the same profile's
texture-class and target mapping. It accepts a UASTC KTX2 master, requires an
explicit target, validates the source model and transfer function, and
atomically publishes a target KTX2 only after `ktx validate`, Vulkan-format
and image-topology checks. Its JSON receipt includes input and output hashes.
The [real-tool packer conformance](../tools/quality/ktx2_pack_conformance.py)
passes ten target packages across four texture classes and six negative cases:
wrong transfer function,
wrong UASTC codec family, undeclared target, corrupt input, a deliberately
bad transcode provider and a stale product selection. A synthetic device plan
also drives one positive package. Every rejection preserves an existing output. Local
evidence is `quality-results/rfc0008-f3-ktx-pack-20260923.json` (ignored).
Reproduce it with:

```sh
python3 tools/quality/ktx2_pack_conformance.py \
  --source /tmp/rfc0008-ktx-pin \
  --build /tmp/rfc0008-ktx-pin/build-rfc0008 \
  --out quality-results/rfc0008-f3-ktx-pack-20260923.json
```

The [Portal Linux native Vulkan product profile](../quality/product_profiles/portal-linux-wayland-native-vulkan.json)
now owns ordered format choices for all four texture classes. The
[selector](../tools/texture/ktx2_select.py) checks sampled-image,
linear-filter and transfer-destination support from a Vulkan Profiles device
query, records the selected device identity and report hash, and fails a plan
when any required class has no supported target. A live `vulkaninfo` query on
the local AMD Radeon 8060S / RADV selected BC7, BC5, BC4 and BC6H. Removing
all base-color formats from the report produces a failing plan. Five unit
tests cover priority, explicit fallback, missing class, missing transfer
capability and an undeclared target. Local evidence is
`quality-results/rfc0008-f3-device-selection-20260923.json` and
`rfc0008-f3-device-selection-missing-negative-20260923.json` (ignored).

```sh
python3 tools/texture/ktx2_select.py --query-device \
  --out quality-results/rfc0008-f3-device-selection-20260923.json
python3 -m unittest tools/quality/tests/test_ktx2_select.py -v
```

The packer accepts the selection plan and rejects one whose host or product
profile hash has changed. The native Vulkan device core now recognizes the
profile's BC, ASTC, ETC2 and EAC 4×4 block formats when checking upload
alignment and size. It requires sampled-image and transfer-destination support
before creating a managed texture. The
[`ktx2_native_pixel_conformance`](../unittests/shaderapivulkantest/test_ktx2_pixels.cpp)
target uses blocks produced by the pinned UASTC transcoder: BC7 red and blue,
BC4 mask, BC5 two-channel normal and BC6H HDR. It rejects short payloads and
misaligned regions before upload, captures sampled pixels, and compares BC6H
against a separate RGBA16F upload of the same authored values. On the local
AMD Radeon 8060S / RADV, the native run passed **19 checks, 0 failures**;
BC6H and the float reference both captured `(55,13,4)`. The Vulkan validation
layer was requested but unavailable. Existing native bring-up, material-facing
and PBR pixel checks passed (25, 13 and 31 checks respectively). The
material-facing test required its built shared-library directories on
`LD_LIBRARY_PATH`.

The [native evidence runner](../tools/quality/ktx2_native_pixels.py) checks
one complete `CONFORMANCE` record, rejects skips, crashes and incomplete
output, and requires the test Vulkan context's name/vendor/device IDs to match
the live format-query plan. It records binary and plan hashes, source
revision/dirty digest, duration and separate logs. The local gate passes at
`quality-results/rfc0008-f3-native-pixels-20260923.json` (ignored). Five
runner unit tests include mismatched-device and stale-plan negative controls.
The whole-tree architecture check still reports 49 new and 3 stale
occurrences outside this F3 change. Changed-line style checking finds only
the concurrently edited `material_pixel_conformance.cpp`; the edited Vulkan
device lines and new F3 test pass the pinned formatter.

```sh
WAFLOCK=.lock-waf-rfc0008-client-current ./waf build \
  -o build-rfc0008-client-current \
  --targets=ktx2_native_pixel_conformance,native_vulkan_bringup_conformance,material_facing_vulkan_conformance,pbr_native_pixel_conformance -j8
build-rfc0008-client-current/unittests/shaderapivulkantest/ktx2_native_pixel_conformance
python3 tools/quality/ktx2_native_pixels.py \
  --binary build-rfc0008-client-current/unittests/shaderapivulkantest/ktx2_native_pixel_conformance \
  --selection-plan quality-results/rfc0008-f3-device-selection-20260923.json \
  --out quality-results/rfc0008-f3-native-pixels-20260923.json
python3 -m unittest tools/quality/tests/test_ktx2_native_pixels.py -v
```

This establishes pinned encoder/transcoder, explicit host packaging,
device-query selection and a native BC upload/pixel path on Linux. F3 still
needs material-system VTF/KTX2 selection, the Hammer reader, installed texture
assets, proof that the installed product selected the queried device,
ASTC/ETC2/EAC native pixels on supported hardware, and a complete per-format
GPU corpus. The KTX checkout and generated host-tool textures remain outside
the repository.

### F3 packaged-image reader and GPU consumer (2026-09-23)

The new [`texturecontainer` contract](../public/texturecontainer/texture_image.h)
describes format, dimensions and owned mip bytes without Vulkan or KTX types.
The read-only [KTX2 adapter](../texturecontainer/ktx2_reader.cpp) privately
uses the pinned KTX library and maps the declared BC, ASTC, ETC2, EAC and
uncompressed fallback formats. It accepts 2D, single-layer, single-face,
already-transcoded packages within bounded dimensions and bytes. It rejects
malformed files, unexpected orientation, UASTC masters, unsupported topology
and mip sizes before returning data. KTX source revision and CMake build source
are checked during Waf configuration; no sibling checkout is discovered.

The committed [fixture corpus](../quality/fixtures/ktx2/README.md) contains
the exact BC7 red blocks, an untranscoded master, and four RGBA8 sRGB mip
levels. The Waf-built reader suite passed **15 checks, 0 failures**: exact
blocks and mip pixels, ownership after input mutation, malformed magic,
truncation, oversized mip and supercompression declarations, and master
rejection. When the pinned reader is configured, the
native pixel suite loads the BC7 fixture through it before GPU upload. On the
local AMD Radeon 8060S / RADV that suite passed **21 checks, 0 failures**, and
the device-matched runner passed with local ignored evidence at
`quality-results/rfc0008-f3-reader-native-pixels-20260923.json`. The matching
live format-selection plan is
`quality-results/rfc0008-f3-reader-device-selection-20260923.json`.

```sh
cmake --build /tmp/rfc0008-ktx-pin/build-rfc0008 --target ktx_read -j8
WAFLOCK=.lock-waf-rfc0008-ktx-reader ./waf configure \
  --platform-provider=sdl3 --render-backend=native-vulkan --build-games=portal \
  --ktx-source-root=/tmp/rfc0008-ktx-pin \
  --ktx-build-root=/tmp/rfc0008-ktx-pin/build-rfc0008 \
  --disable-warns -T release -o build-rfc0008-ktx-reader \
  --prefix=/tmp/rfc0008-ktx-reader-install
WAFLOCK=.lock-waf-rfc0008-ktx-reader ./waf build \
  --targets=ktx2_reader_conformance,ktx2_native_pixel_conformance -j8
build-rfc0008-ktx-reader/unittests/texturecontainertest/ktx2_reader_conformance \
  quality/fixtures/ktx2
python3 tools/texture/ktx2_select.py --query-device \
  --out quality-results/rfc0008-f3-reader-device-selection-20260923.json
python3 tools/quality/ktx2_native_pixels.py \
  --binary build-rfc0008-ktx-reader/unittests/shaderapivulkantest/ktx2_native_pixel_conformance \
  --selection-plan quality-results/rfc0008-f3-reader-device-selection-20260923.json \
  --out quality-results/rfc0008-f3-reader-native-pixels-20260923.json
```

The whole-tree architecture check remains at 49 new and 3 stale occurrences;
none arise from the new texture modules. Changed-line style checking reports
only the concurrently edited `material_pixel_conformance.cpp`. The KTX reader,
fixture test and native pixel changes pass the pinned formatter.

### F3 VTF/KTX2 shared image cohort (2026-09-23)

The [VTF adapter](../texturecontainer/vtf_reader.cpp) uses the existing VTF
library behind a private legacy bridge. Ordinary 2D, single-frame,
single-face VTFs with RGBA/BGRA8, DXT1/3/5, ATI1N/2N or RGBA16F image data
map into the same owned format and mip description as KTX2. Animation,
cubemaps, volume textures and other formats return explicit unsupported
results and continue to use the existing VTF caller path. The adapter copies
mip bytes before destroying the VTF decoder and does not expose a legacy type
through the portable header.

The Waf-built [shared-data test](../unittests/texturecontainertest/test_vtf_reader.cpp)
serializes a four-mip opaque-red VTF with the legacy writer, reads it through
the adapter, and compares every level's dimensions and bytes against the
committed RGBA8 sRGB KTX2 fixture. It also compares the exact top-level BC1
blocks and format from a one-mip VTF and KTX2 pair. This caught a legacy VTF
decoder behavior: it allocates a full mip chain even when the file authors only
one level. The adapter now exposes only authored levels, avoiding unused bytes.
The suite passed **11 checks, 0 failures**, including input-lifetime,
malformed-header and animated-VTF negatives. The pinned
toolchain boundary check passed for 79 Waf targets with no dialect errors;
local ignored evidence is
`quality-results/rfc0008-f3-vtf-ktx-reader-toolchain-20260923.json`.

```sh
WAFLOCK=.lock-waf-rfc0008-ktx-reader ./waf build \
  --targets=vtf_texture_reader_conformance -j8
LD_LIBRARY_PATH=build-rfc0008-ktx-reader/tier0 \
  build-rfc0008-ktx-reader/unittests/texturecontainertest/vtf_texture_reader_conformance \
  quality/fixtures/ktx2
python3 tools/quality/toolchain_boundary.py check \
  build-rfc0008-ktx-reader/toolchain-invocations.json \
  --out quality-results/rfc0008-f3-vtf-ktx-reader-toolchain-20260923.json
```

This is a bounded reader cohort. The material system still loads only VTF
paths; it has not adopted the common description or KTX2 file selection.

### F3 native Vulkan image upload bridge (2026-09-23)

The strict C++20 [image upload bridge](../materialsystem/shaderapivulkan/vulkan_texture_image.cpp)
maps every declared `TextureImage` format to a Vulkan format and checks the
complete mip chain against the shared [block layout contract](../public/texturecontainer/texture_image.h)
before creating a managed GPU image. The selected device must support sampling
and upload for the requested format. All authored levels are uploaded; a failed
upload destroys the managed image and cancels its pending copies. The native
provider also releases partially created multi-mip images if its initial
layout command fails. This is a provider-side seam; no material-system file
texture caller uses it yet. `CTexture::LoadTextureBitsFromFile` currently
requires an `IVTFTexture` result for dimensions, flags, low-res data, resources,
and upload, while the Vulkan shader API receives decoded VTF objects through
`TexImageFromVTF`. Product KTX2 selection therefore needs a bounded caller
migration with those metadata and upload obligations accounted for.

The native pixel suite now reads the committed BC7 package, uploads it through
the bridge, and captures red pixels. It also copies the four-level RGBA8
package, gives each lower mip a distinct test color, uploads it, then samples
and captures each level with a mip-enabled sampler. It rejects short BC7 bytes
and an unknown required format. On AMD Radeon 8060S / RADV,
the device-matched suite passed **37 checks, 0 failures**; local ignored
evidence is `quality-results/rfc0008-f3-image-upload-native-20260923.json`.
The KTX2 and VTF reader suites passed **15/0** and **11/0**. The pinned
toolchain boundary check passed **80 targets** and **18/18** probes with no
dialect errors; evidence is
`quality-results/rfc0008-f3-image-upload-toolchain-20260923.json`. Vulkan
validation was requested but its layer was unavailable in this run.
Post-creation failure injection, ASTC/ETC2/EAC
native pixels, product texture selection, and installed content remain open.

```sh
WAFLOCK=.lock-waf-rfc0008-ktx-reader ./waf build \
  --targets=ktx2_native_pixel_conformance,ktx2_reader_conformance,vtf_texture_reader_conformance -j8
LD_LIBRARY_PATH=build-rfc0008-ktx-reader/tier0 \
  build-rfc0008-ktx-reader/unittests/texturecontainertest/ktx2_reader_conformance \
  quality/fixtures/ktx2
LD_LIBRARY_PATH=build-rfc0008-ktx-reader/tier0 \
  build-rfc0008-ktx-reader/unittests/texturecontainertest/vtf_texture_reader_conformance \
  quality/fixtures/ktx2
python3 tools/quality/ktx2_native_pixels.py \
  --binary build-rfc0008-ktx-reader/unittests/shaderapivulkantest/ktx2_native_pixel_conformance \
  --selection-plan quality-results/rfc0008-f3-reader-device-selection-20260923.json \
  --out quality-results/rfc0008-f3-image-upload-native-20260923.json
python3 tools/quality/toolchain_boundary.py check \
  build-rfc0008-ktx-reader/toolchain-invocations.json \
  --out quality-results/rfc0008-f3-image-upload-toolchain-20260923.json
```

Whole-tree architecture checking still reports 49 new and 3 stale occurrences,
none from this texture cohort. Changed-line style checking still reports only
the independently edited `material_pixel_conformance.cpp`. Loader inventory
verification separately reports 12 uninstrumented native sites outside this
cohort, including vendored samples and the physics test.

### F3 Hammer KTX2 preview caller (2026-09-23)

`MaterialCatalog` now checks for a packaged KTX2 asset before the legacy VTF
for the same `$basetexture`. Its optional decoder is supplied at composition;
the strict Hammer core keeps no KTX library dependency. A present malformed or
unpreviewable KTX2 asset fails explicitly rather than displaying a VTF from
another path. The [Hammer adapter](../hammer/adapters/source/ktx2_preview.cpp)
calls the same owned KTX2 reader as native Vulkan and presents packaged
RGBA8/BGRA8 top-level pixels to the GTK texture browser and viewport. The
GTK build checks the pinned KTX source revision and build root before enabling
that adapter. Compressed BC/ASTC/ETC2/EAC preview is still missing.

The Waf-built [Hammer KTX2 suite](../unittests/hammertest/formats/test_ktx2_preview.cpp)
passed **10 checks, 0 failures**: KTX selection and pixels, stable cache,
canonical `.ktx2` names, malformed/BC7/absent-decoder failures, a deliberately
bad decoder, and VTF-only fallback. Existing material-catalog suites passed
**13/0** and **8/0**. The strict toolchain boundary passed **81 targets** with
**18/18** probes. An opt-in GTK product built against the pinned KTX reader;
the [product smoke runner](../tools/quality/hammer_ktx2_preview.py) staged VMT
and KTX2 assets in a private VPK, rendered the room sample offscreen, and
observed 45,221 red-dominant texture pixels. Corrupting the KTX2 signatures
yielded zero such pixels; the runner also rejected a GTK build without the
KTX reader. Local ignored evidence is
`quality-results/rfc0008-f3-hammer-ktx-product-20260923.json`,
`rfc0008-f3-hammer-ktx-negative-20260923.json`, and
`rfc0008-f3-hammer-toolchain-20260923.json`.

```sh
KTX_SOURCE_ROOT=/tmp/rfc0008-ktx-pin \
KTX_BUILD_ROOT=/tmp/rfc0008-ktx-pin/build-rfc0008 \
  hammer/gtk/build.sh /tmp/rfc0008-hammer-ktx-preview
WAFLOCK=.lock-waf-rfc0008-ktx-reader ./waf build \
  --targets=hammer_ktx2_preview_conformance -j8
build-rfc0008-ktx-reader/unittests/texturecontainertest/hammer_ktx2_preview_conformance \
  quality/fixtures/ktx2
python3 tools/quality/hammer_ktx2_preview.py \
  --binary /tmp/rfc0008-hammer-ktx-preview \
  --out quality-results/rfc0008-f3-hammer-ktx-product-20260923.json
```

This verifies a GTK file-texture consumer for the uncompressed preview
profile. Native Vulkan gameplay texture selection is still VTF-only. The GTK
asset source currently offers no cross-extension search-path precedence, so
KTX2 presence wins over a VTF of the same name even when they come from
different mounted providers; that policy needs an explicit compatibility
decision before general mod/content rollout.

## Change inventory

| Kind | Files |
| --- | --- |
| New | `public/mapcontainer/`, `mapcontainer/`, `engine/map_container_file.{h,cpp}`, `utils/bsp2tool/`, `unittests/mapcontainertest/` (suite and contract), `tools/quality/bsp2_reader.py`, `tools/quality/tests/test_bsp2_reader.py` |
| Edited | `engine/modelloader.{h,cpp}`, `engine/checksum_engine.cpp`, `engine/host_listmaps.cpp`, `filesystem/basefilesystem.{h,cpp}`, `engine/wscript`, `filesystem/wscript`, root `wscript` (`mapcontainer` in the game, dedicated, tests and tools groups; `bsp2tool` in tools), `quality/conformance.manifest.json`, `architecture/modules.json` |

The 2026-09-23 follow-up adds
`tools/quality/bsp2_server_compare.py`, `tools/quality/bsp2_dedicated.py`,
`tools/quality/bsp2_v19_derivative.py`, `tools/quality/bsp2_f1_gate.py`,
`tools/quality/bsp2_sparse_export.py` and its negative-control tests,
`quality/fixtures/bsp2-dedicated-cases.json`,
`quality/fixtures/bsp2-v21-engine-cases.json`,
`quality/fixtures/bsp2-v19-derived-cases.json`,
`quality/fixtures/bsp2-corpus-v20.json`, and
`quality/fixtures/bsp2-corpus-v21.json` and their negative-control tests; edits
`dedicated/dedicated.h`, `dedicated/sys_{linux,windows}.cpp`,
`dedicated_main/main.cpp`,
`dedicated/wscript`, `engine/modelloader.cpp`,
`game/shared/point_bonusmaps_accessor.cpp`, `tools/quality/portal_boot.py`,
`tools/quality/bsp2_reader.py`,
`mapcontainer/map_container.cpp`, `utils/bsp2tool/bsp2tool.cpp`,
`public/mapcontainer/map_container.h`, `public/mapcontainer/map_container_builder.h`,
`unittests/mapcontainertest/contracts/world.map_container.v1.md`,
`unittests/mapcontainertest/test_map_container.cpp`, and
`.github/workflows/conformance.yml`. The v21 corpus check updates the R53
content input and check in `quality/baseline.json`.

The F3 image cohort adds `public/texturecontainer/`, `texturecontainer/`,
`materialsystem/shaderapivulkan/vulkan_texture_image.{h,cpp}`,
`unittests/texturecontainertest/`,
`unittests/shaderapivulkantest/test_ktx2_pixels.cpp`,
`quality/fixtures/ktx2/`, the texture packer/selector and native pixel runner
under `tools/texture/` and `tools/quality/`, and its pinned product profile.
The bridge is registered in `architecture/modules.json` and
`quality/toolchain/policy.json`. Waf builds the KTX2 and VTF reader targets
when the pinned KTX reader is configured, and builds the Vulkan bridge with
the native Vulkan provider.
The Hammer follow-up edits `public/hammer/formats/material_catalog.h`,
`hammer/core/formats/material_catalog.cpp`, `hammer/gtk/{app.cpp,offscreen.cpp,build.sh}`,
and the catalog contract; it adds `hammer/adapters/source/ktx2_preview.{h,cpp}`,
`unittests/hammertest/formats/test_ktx2_preview.cpp`, and
`tools/quality/hammer_ktx2_preview.py`.

### F3 native texture limits and filtering (2026-09-23)

The native Vulkan material hardware config now reports the selected device's
2D image limit, capped at the KTX2 reader's 16,384-pixel content limit. Managed
texture creation checks both that device limit and the requested format, usage,
extent, array layers, sample count, and mip count before allocating the image.
Anisotropic texture requests now use device-supported Vulkan samplers up to
16×; changing the configured level after a frame refreshes affected descriptors
only after GPU completion. The selected RADV device reports a 16,384-pixel 2D
limit and 16× anisotropy. The native KTX2 pixel suite passes 42 checks, including
oversized-image rejection and an anisotropic sampler change after a submitted
frame; the material-facing suite passes 15 checks, including the hardware
config's live limit reports. Changed-line style and
`git diff --check` pass.

This is a 16K-capable GPU image boundary for supported compressed formats, not
yet a 16K PBR world material. The gameplay material loader still selects VTF,
and the BSP2 lightmap consumer currently requires one RGBA16F page. A 16K BC7
full mip chain fits the reader's 512 MiB container bound; equivalent uncompressed
RGBA8/HDR chains do not. Runtime KTX2 material selection, memory budgeting and
large-texture load/pixel evidence remain F3 work. No numeric Source 2 texture
ceiling has been verified from a Valve source; 16K is this profile's tested
device capability and current content ceiling, not an asserted Source 2 fact.

### F3 linear BC7 MRAO and 16K allocation; F4 WMSH PBR shader (2026-09-23)

The host and Portal product profiles now select a separate linear MRAO class.
The pinned KTX tool produces BC7, ASTC 4×4 and ETC2 RGBA **UNORM** packages
from a linear UASTC master, and the owned KTX2 reader and Vulkan bridge
preserve those formats. Committed 8×8 packages pass `ktx validate` and the
reader suite; BC7 also passes native sampled-pixel checks for separate
metalness, roughness and AO channels. The host probe and packer conformance
pass with thirteen targets across five classes. ASTC/ETC2 native pixel checks
still require a selected device that supports those formats.

The selected RADV device also allocated a 16,384² BC7 sRGB image with all 15
mip levels; the native suite rejected a dimension beyond the device limit. This
proves an image allocation on this profile, while the sampled-pixel fixtures
prove color space and format handling at small sizes. Full-size content upload,
streaming and budget evidence remain open.

An opt-in native WMSH PBR pipeline now reads the packed WMSH normal and tangent,
base color, linear MRAO, optional tangent-space normal and map-scoped RGBA16F
irradiance. Its pixel suite changes metalness, roughness, directional light and
normal direction independently, and rejects an sRGB texture in the linear MRAO
slot. Draw recording and replay require live descriptors and matching image
formats, so a missing required PBR image cannot silently sample the built-in
fallback texture. The renderer's material selection still chooses the legacy
path for gameplay; map-authored lights, style layers, probes, HDR output and
Cycles image parity remain F4/F5 work.
Its pipeline construction, image requirements and teardown live in
`materialsystem/shaderapivulkan/vulkan_world_pbr.cpp`, leaving the device core
to own common allocation, command recording and submission. The BSP2 KTX2
lightmap validation/upload now lives in `vulkan_world_lightmap.cpp`; the large
legacy shader API adapter only invokes it and reports the result. The WMSH PBR
pixel suite uses that bridge with a checked RGBA16F KTX2 fixture and rejects a
corrupt package before the successful upload.

Evidence: `ktx2_reader_conformance` **18/0**,
`ktx2_native_pixel_conformance` **47/0**,
`world_pbr_native_pixel_conformance` **22/0**, and
`material_facing_vulkan_conformance` **15/0** on the selected Radeon 8060S.
The native validation layer was unavailable, so those runs did not verify
validation-clean command recording. The pinned host probe and packer conformance
both pass; the texture selector's ten Python unit tests pass. Changed-line
style and `git diff --check` pass. The architecture checker reports no new or
stale occurrences for this change, but exits nonzero on existing CAP002 include
violations in the current tree.

### F4 playable WMSH PBR material bridge and winding fix (2026-09-23)

The native Vulkan shader API now recognizes the legacy `PBR` shader only for
WMSH batches. It forwards that shader's bound base color, MRAO and optional
normal image, alpha reference and actual camera position to the WMSH PBR
pipeline; missing required images reject the draw. The native device keeps the
map-owned HDR lightmap in its PBR descriptor set. Ordinary dynamic `PBR` meshes
remain outside this bridge, and canonical `PBRMetalRough` material selection
still uses its legacy fallback. This is an explicit preview cohort, not the
final runtime material contract.

The first playable PBR capture culled most of the room. `BuildMaterialPipeline`
classified only the textured WMSH vertex input as world geometry, so the PBR
vertex input received the opposite Vulkan front face. It now classifies both
world inputs consistently and keeps back-face culling enabled. The native GPU
pixel suite added a back-face-culling fixture and passes **25 checks, zero
failures** on the selected Radeon 8060S. Temporary two-sided and fragment
shader diagnostic probes were removed after the source of the culling error
was identified.

The [staircase content bridge](../tools/quality/staircase2_playable_content.py)
accepts an optional `--world-pbr-prefix` matching the USD world packer's
`--material-prefix`. It emits the `PBR` VMT namespace for six opaque WMSH
materials while retaining the `PBRMetalRough` and legacy fallback namespaces;
glass and small emitter fixtures retain their unlit preview materials. The
script checks compiled solid VTF channel values, including metalness and
roughness. Its generated 46 content files are byte-identical to the manually
assembled preview fixture. The reproducible generated fixture and namespaced
BSP2 pass the [native playable boot](../quality-results/staircase2-playable-pbr-generated-v1/evidence.json)
with nine batches and 1,520 queued meshlets. The earlier fixed-winding
[inspection frame](../quality-results/staircase2-playable-pbr-winding-fixed-v1/inspection.png)
shows the complete room and PBR response; the generated boot also captured a
scene-detailed frame. Its requested 1024² mode became 2816×1713 on this
desktop, so no Cycles pixel-parity score is claimed.

The map uses the unscaled Cycles 2048² RGBA16F atlas, but still looks darker
than the Cycles reference. The WMSH PBR scene uses a synthetic directional
source and has no authored reflection probes, transmission or validated output
exposure. Gameplay material loading still uses VTF; KTX2 is consumed here for
the HDR atlas only. These gaps, plus spatial meshlet visibility and feature
cohorts, remain before high visual parity or F4/F5 acceptance. The native GPU
run requested validation, but the validation layer was unavailable. Changed-line
style, Python syntax and `git diff --check` pass. The current full architecture
check reports eight CAP002 include violations and 47 baseline drift entries in
unrelated legacy files; no new entry is in this WMSH material cohort.
