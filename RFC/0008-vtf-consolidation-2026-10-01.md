# Shared VTF container reader — 2026-10-01

User-directed consolidation; R55/F3 remains partial. This is a parsing and
compatibility slice, not acceptance of the external PBR mount or render profile.

The contract and single container owner are
[`texturecontainer::vtf`](../public/texturecontainer/vtf_container.h) and
[`container.cpp`](../texturecontainer/vtf/container.cpp), built as strict C++20
`vtf_container`. The module manifest owns its dependencies. The reader has no
legacy engine, renderer, filesystem or decompression-library dependency.

The game `CVTFTexture` adapter, the lab's `ReadVtfImage`, and Hammer's `DecodeVtf`
now use that owner for headers, resource dictionaries, payload bounds and
mip/frame/face/volume layout. The old Hammer header/mip parser, game header and
resource parser, game cubemap-offset loop and unused byteswap/resource-loading
helpers were deleted. The texture loader no longer casts unvalidated bytes to a
header to seek the file. The lab no longer links VTF, bitmap or tier libraries.

Game interface vtables and object data layout are unchanged. The shared parser
returns owned metadata and static error diagnostics; these results do not carry
`std::string` across the engine/strict-library libstdc++ ABI boundary. Preview
RGBA conversion, GPU-format selection and legacy image storage remain adapters.
Header-only reads, authored versus allocated mip counts, skipped streamed mips,
frames, volume slices, historical cube/sphere layouts, inline resources,
thumbnails and resource payloads have independent fixtures.

The shared reader understands VTF 7.6 AXC runs and Strata BC7 storage. CPU
decompression remains an explicit callback. The lab's image adapter preserves
BC7 blocks; Hammer converts them for preview. This does not add a BC7 GPU format
to the legacy engine or install compression providers in every product. A
missing decompressor fails explicitly; a malformed file is never a fallback.

Frozen-path: `materialsystem/ctexture.cpp` uses the validated header-only reader's
buffer position instead of dereferencing an unchecked on-disk header. No shading
or render-quality policy changes.

## Evidence

Local Linux x86_64, GCC 16.2.1 and Clang 22.1.8, source state spanning
`45ae4289b` through `19b1f35f5` plus the final working changes. Other sessions
committed in the shared checkout during this work. Detailed commands, source
hashes and logs are retained under `quality-results/vtf-consolidation*`.

- Release `content.vtf.shared`: **677 checks**, including every byte-prefix
  truncation of an independent fixture and malformed/overlapping resources.
- Existing Hammer positive/sensitivity suites: **32 and 11 checks**.
- Waf native shared suite, also exercising `IVTFTexture`: **1,077 checks**.
- Existing VTF/KTX2 pixel/block comparison: **13 checks**.
- Clang ASan+UBSan shared suite: **677 checks**, including leak detection.
  The GCC sanitizer link was unavailable because its libasan path was missing;
  the Clang run is the sanitizer evidence.
- Installed Portal 2 `portal2/pak01_dir.vpk` and Workshop packs `3594428478`,
  `3506356222`, `3592898497`: **4,515 VTFs / 48,169 native checks**. The game
  adapter's legacy-supported subresources compare byte-for-byte with their
  authored payloads. AXC/BC7 containers are parsed; the corpus does not claim
  that the legacy renderer supports their pixel formats or decompressors.
  Inputs are piped from the existing VPK reader; only logs/digests are written.
- Corpus runner's five negative-control tests reject empty archives, zero,
  missing or duplicate results, failed checks, incomplete coverage and process
  failure hidden by success text.
- Isolated Waf build of `render_lab`, `hammer_formats`,
  `vtf_shared_conformance` and `vtf_texture_reader_conformance`: passed. The
  Portal 2 `materialsystem` target also builds/links with the shared reader.
- Architecture fixtures **162 passed**, style fixtures **38 passed**. Full
  change style check against `45ae4289b` passed. Loader inventory verified.
  Whole-tree architecture/baseline checks still report unrelated existing
  render include violations and two F-Stop `CreateInterfaceFn` occurrences;
  no ratchet was rewritten to hide those failures.

Earlier shader-artifact failures prevented a full lab build; after concurrent
render work resolved them, the full isolated build passed. Waf `step` was used
in between for diagnostic compilation/linking only, not as a substitute for the
final full target build.

No GPU shader or sampling behavior was changed in this slice. GPU frame/power
budgets, native mobile/Apple runs and the fully core-rendered mounted-PBR Portal
2 scene are **not verified** by these parsing tests and remain open.

## Reproduce

Use the configured profile's own Waf lock/output tree. The isolated tree was
configured with `--tools --disable-warns -T release`, and the already-pinned
KTX source/build at `/tmp/rfc0008-ktx-pin{,/build-rfc0008}`. No dependency was
fetched or an existing product profile overwritten.

```sh
python3 tools/quality/conformance.py check --config release \
  --suite content.vtf.shared --suite hammer.formats.vtf_image \
  --suite hammer.formats.vtf_image.sensitivity \
  --out quality-results/vtf-consolidation-final.json

(cd build-vtf-consolidation && WAFLOCK=.lock-waf-vtf-consolidation \
  python3 ../waf build --targets=vtf_shared_conformance,vtf_texture_reader_conformance,hammer_formats,render_lab -j8)

LD_LIBRARY_PATH=build-vtf-consolidation/tier0 \
  build-vtf-consolidation/unittests/texturecontainertest/vtf_shared_conformance
LD_LIBRARY_PATH=build-vtf-consolidation/tier0 \
  build-vtf-consolidation/unittests/texturecontainertest/vtf_texture_reader_conformance \
  quality/fixtures/ktx2

# Repeat --vpk for each installed archive; the result records archive paths and digests.
LD_LIBRARY_PATH=build-vtf-consolidation/tier0 \
  python3 tools/quality/vtf_container_corpus.py \
  --binary build-vtf-consolidation/unittests/texturecontainertest/vtf_shared_conformance \
  --vpk '/path/to/installed/pak01_dir.vpk' \
  --out quality-results/vtf-consolidation-corpus

python3 -m unittest discover -s tools/quality/tests -p test_vtf_container_corpus.py -v
```
