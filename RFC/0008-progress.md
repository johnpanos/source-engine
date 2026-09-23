# RFC 0008 progress: Canonical world data and runtime formats

Updated: 2026-09-22
Portfolio rows: R53 (F1), with R54–R58 not started.
Source revision at assessment: `87d7acd3` plus working-tree changes.

This file is the gate-decision record for
[RFC 0008](0008-canonical-world-data-and-runtime-formats.md). Where it
disagrees with the versioned artifacts, the artifacts win:

- the suite manifest row `world.map-container`;
- the [container contract](../unittests/mapcontainertest/contracts/world.map_container.v1.md);
- the evidence under `quality-results/rfc0008-f1-20260922/`.

The evidence directory is local and not versioned; the reproduction commands
below regenerate it.

## Phase status

| Phase | State | Summary |
| --- | --- | --- |
| F1 BSP2 container and map-reader seam | **active (prototype; gate incomplete)** | Container, seam, both readers, corpus and client boot work; dedicated-server runtime and CI lane missing (see [Remaining for the F1 gate](#remaining-for-the-f1-gate)) |
| F2–F7 | planned | Not started |

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
  `convert` and `export`.
- **Diagnostics.** A new `map_container_info` console command reports the
  container kind and reads every game lump back through the dictionary, with a
  CRC of each.

### Evidence (2026-09-22, Linux x86_64, g++ 16.2.1 / clang++ 22.1.8, Python 3.14.7)

| Check | Result |
| --- | --- |
| `world.map-container` shared suite (checks-v1), g++ and clang++ | PASS, 114 checks. It covers: <ul><li>the RFC 7693 vector;</li><li>the contract suite against both readers, with 3 deliberately bad providers detected;</li><li>3 synthetic round trips;</li><li>29 table-driven negative fixtures, each with its named error, plus legacy-reader and writer negatives;</li><li>20 000 seeded mutations (18 972 rejected, 1 028 accepted, none crashing).</li></ul> |
| Same suite under ASan+UBSan (clang++; the local g++ has no libasan) | PASS, no reports |
| Suite sensitivity: 5 seeded library defects | Each of four defects (overlap, directory hash, LGAP overlap, alignment checks removed) fails the suite. The `legacyOrigin` defect aborts under ASan. |
| `tools/quality/tests/test_bsp2_reader.py` | PASS, 4 tests. They cover: <ul><li>independent reader round trips;</li><li>12 negative fixtures with the same error names as C++;</li><li>game-lump origin checks;</li><li>C++ vs Python writers agreeing byte-for-byte;</li><li>shared error vocabulary.</li></ul> |
| Corpus: `bsp2_reader.py corpus` over 54 maps (26 Portal v20, 28 chamber-sdk v25; about 7 s) | **54/54 pass.** For every map: <ul><li>the C++ conversion validates in the Python reader;</li><li>the Python and C++ writers produce identical bytes;</li><li>both exporters return the original file byte-for-byte;</li><li>every legacy lump and the game-lump structure match.</li></ul> A converted Portal map is about 3.4 KB larger than its legacy file. |
| Portal client + listen server, null renderer, headless, BSP2 `testchmb_a_00` | Map active, player active at the same position, 326 edicts. The console log is identical to the legacy map's apart from paths and ping. |
| Game lumps through the rebased dictionary (`map_container_info`) | `sprp` v5 (5994 bytes) and `dprp` v4 (12 bytes) load with the same CRCs for legacy and BSP2. |
| Map pak lump | Mounted as the `(map)` search path for both containers. The zip directory must parse before the path is added. |
| Corrupted BSP2 map (one bit flipped in lump 1) | Rejected with `Host_Error: ... lump 1 failed verification (content-hash-mismatch, 57160 bytes)`. The process exits normally, and the Python reader reports the same error name. |
| DXVK rendering, `testchmb_a_01`, fixed camera and exposure, 2 legacy + 2 BSP2 runs | All pass with scene detail. Cross-container differences (mean 0.24–0.47 per channel) match same-container ones (0.17–0.38), and all fall in the same animated region (about 1300 pixels). |
| Dedicated engine library (`--dedicated` tree) | Builds with the container linked statically. `DT_NEEDED` is identical to the `build-rfc0001-dedicated` baseline, so no render or texture libraries were added. |
| Style: new files in full, edited regions of legacy files | Clean (clang-format 22.1.8) |
| archlint `check --all` | No findings in RFC 0008 files. The 7 new and 3 stale ARCH105 items are in other sessions' uncommitted `shaderapivulkan`, `physicstest` and `vphysics_box3d` work. archlint self-tests: 71 pass. |

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

# Corpus (build bsp2tool with Waf --tools, or directly)
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

## Remaining for the F1 gate

- **Dedicated-server runtime load (blocked).** The `dedicated` launcher library
  does not compile at HEAD, independent of this work:
  `dedicated/sys_linux.cpp:277` calls the protected
  `CAppSystemGroup::LoadModule` (file unchanged since `cf8de03c`). The dedicated
  engine library builds and links the container. Once the launcher builds, load
  a BSP2 map on `srcds` and compare its status and collision traces with the
  legacy map. The listen server above already exercises the server-side lump
  paths in a non-`SWDS` engine.
- **Server-side trace and vis equivalence.** Compare traces and vis on both
  containers, not only load and status. This could reuse the Q-PHYSICS query
  cases.
- **CI lane.** Wire the suite, Python tests and corpus into a lane with the
  corpus path declared as a required input.
- **Scope decisions still open:**
  - Lump override files (`.lmp`) keep working by legacy index. Keying them by
    4CC for new lumps waits until new lumps exist (F4/F5).
  - `IFileSystem` seeks are 32-bit, so the engine bridge refuses offsets past
    2 GiB. The container itself is 64-bit.
  - Compile tools (`utils/common/bsplib`, `vbsp`, `vvis`, `vrad`, `bspzip`)
    and Hammer still read and write only VBSP. A BSP2 package is produced after
    compile by `bsp2tool convert`, until F2 (`vbsp2`) writes it directly.
  - `utils/hlfaceposer` also reads raw BSP headers. It is Windows-only and out
    of F1 scope.
  - Big-endian console `PSBV` maps are rejected, the same as the removed
    console paths.

## Change inventory

| Kind | Files |
| --- | --- |
| New | `public/mapcontainer/`, `mapcontainer/`, `engine/map_container_file.{h,cpp}`, `utils/bsp2tool/`, `unittests/mapcontainertest/` (suite and contract), `tools/quality/bsp2_reader.py`, `tools/quality/tests/test_bsp2_reader.py` |
| Edited | `engine/modelloader.{h,cpp}`, `engine/checksum_engine.cpp`, `engine/host_listmaps.cpp`, `filesystem/basefilesystem.{h,cpp}`, `engine/wscript`, `filesystem/wscript`, root `wscript` (`mapcontainer` in the game, dedicated, tests and tools groups; `bsp2tool` in tools), `quality/conformance.manifest.json`, `architecture/modules.json` |
