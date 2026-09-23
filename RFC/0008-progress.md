# RFC 0008 progress: Canonical world data and runtime formats

Updated: 2026-09-23
Portfolio rows: R53 (F1) active; R55 (F3) has host-tool, reader, native GPU and
GTK preview evidence;
R54 has host-tool/schema preparation but no stage emitter; R56–R58 are not started. F1 native-run evidence records the engine
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

## Phase status

| Phase | State | Summary |
| --- | --- | --- |
| F1 BSP2 container and map-reader seam | **active (prototype; gate incomplete)** | Container, seam, both readers, pinned v20/v21 lossless corpora, 64-bit sparse tool export, client boot, a 26-map Portal v20 dedicated comparison and five-map v21 client/dedicated engine-seam comparisons work. A derived v19-header map also loads in both products. Authored v19 content, native Portal 2 gameplay and the CI content lane remain (see [Remaining for the F1 gate](#remaining-for-the-f1-gate)) |
| F2 World Stage | partial (world geometry and entity slices; gate incomplete) | Pinned OpenUSD/oneTBB host build and typed schema; opt-in `vbsp2` emits compiled world faces, one shared lightmap chart table and ordered BSP entity records. A sealed room passes `usdchecker` and an independent BSP comparator with missing-face and missing-entity controls. Brush entities, displacements, props, materials, UsdLux, Cycles and the full semantic corpus remain. |
| F4–F7 | planned | Not started |
| F3 KTX2 textures | **partial (host packer, device selection, owned readers and native/GTK consumers)** | Pinned Linux tool builds; ten transcode targets validate, UASTC master decode pixels match fixtures, and the packer publishes validated packages from explicit or product-profile device selections. Strict KTX2 and VTF readers produce the same owned image description for a 2D caller cohort; BC7 feeds the native pixel test. Hammer's GTK material catalog previews packaged RGBA8/BGRA8 KTX2 through that reader. Material-system file selection, Hammer compressed-format preview, installed product device identity and ASTC/ETC2/EAC GPU pixels remain. |

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

The [schema source](../utils/worldstage/schema.usda) defines `SourceWorldAPI`
and `SourceMeshAPI` as the owner of the version and Source face/chart mappings.
The [minimal stage fixture](../quality/fixtures/worldstage/minimal.usda) has a
Z-up, 0.0254-meter-per-unit world, one triangle, material/lightmap UVs, and
Source face, smoothing-group, and chart IDs. The
[host probe](../tools/quality/openusd_host_probe.py) verifies exact checkout
revisions and CMake settings, runs `usdchecker` on the fixture and a `.usdc`
round trip, requires a missing-up-axis mutation to fail, generates 14 schema
artifacts, and compiles the three generated non-Python C++ sources in C++20
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
dependency enters the existing `vbsp`, engine, or dedicated products. The
compiler writes `<map>.geometry.usda` after the BSP: one `UsdGeomMesh` per
compiled world face, fan-triangulated face topology, Source face/smoothing/chart
IDs, material path and UVs, Z-up Source units, and a single atlas chart table.
`SourceEntityAPI` carries each compiled BSP entity's ordered keys and values,
including duplicate keys, under `/World/Entities`. The emitted schema version
is 2; the minimal reviewed stage fixture also exercises that API. The entity
lump is the input after VBSP's own serialization, so compiler-added and edited
keys are carried into the stage.
It rejects brush-entity models, displacement faces, malformed references and
an overflowing atlas explicitly. BSP and USD publication are not yet atomic.

The sealed room emitted 16 faces, 32 triangles, and three entities;
`usdchecker` passed. The
[independent comparator](../tools/quality/worldstage_geometry_compare.py)
reads the BSP face/edge/vertex/model lumps directly and checks every mesh's
vertices, triangle indices, identity, material path, smoothing group, both UV
sets, chart size and chart overlap. It independently parses the serialized BSP
entity lump and checks record order and values. Removing a mesh or entity in
memory makes that comparator fail. Local ignored evidence is
`quality-results/rfc0008-worldstage-room-20260923.json`.

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
  --vbsp2 /tmp/rfc0008-worldstage-install/vbsp2 \
  --usdchecker /tmp/rfc0008-openusd-install/bin/usdchecker \
  --python /usr/bin/python3.12 \
  --openusd-python /tmp/rfc0008-openusd-install/lib/python \
  --plugin-root /tmp/rfc0008-worldstage-install/share/sourceWorld \
  --out quality-results/rfc0008-worldstage-room-20260923.json
```

The [smoke runner](../tools/quality/worldstage_host_smoke.py) copies the game
and VMF into a temporary directory, compiles them, runs `usdchecker` and the
semantic comparator, and records content hashes. The full F2 gate still needs
complete authored content semantics,
versioned corpus, Cycles open/render, lightmap chart ownership across all
supported face types, failure recovery, and the R48 legacy compiler oracle.

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
