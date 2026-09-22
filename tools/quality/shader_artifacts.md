# Source-matched compatibility shader artifacts

The Vulkan compatibility renderer consumes DX9 bytecode. Its shader selectors
are generated C++ headers in `materialsystem/stdshaders/fxctmp9`; a Steam content
pack can contain a different permutation schema. Matching filenames alone does
not establish compatibility. For example, this repository's
`lightmappedgeneric_ps20b` selector has 96 dynamic permutations, while the tested
installed Portal pack has 288. Using that pack selects incorrect static groups.

`shader_artifacts.py` builds the requested static groups from repository sources.
It consumes the existing `fxc_prep.pl` plan, including shader-model filtering,
ordered macro ranges, compiler arguments, centroid mask, and Perl `SKIP` rules.
It verifies the checked-in selector's ranges and mixed-radix coefficients before
compiling every valid dynamic permutation in a requested static group. Skipped
permutations retain their original indices; they are never renumbered.

The compiler version, exact SHA-256 pin, source and selector locations, and option
authority live in `quality/shader_profiles/source-dx9-fxc-wine.json`. The Portal
product profile references this file; the runner consumes it and validates the
bundled `dx9sdk/utilities/fxc.exe` before compiling through the installed Wine.
No compiler download is performed. Python3,
Perl, and Wine are host tools. Compiler failures, timeouts, missing or malformed
bytecode, source/header mismatch, and empty required groups fail the build.

Capture `mat_spewvertexandpixelshaders` in the target scene. The product boot
harness already records this dump in the staged game's `portal/console.log`.
Then run, from the repository root:

```sh
python3 tools/quality/shader_artifacts.py \
  --log /path/to/boot-evidence/runtime/portal/console.log \
  --out build-portal-vulkan/shaders/portal-source-matched --jobs 4
```

Repeat `--log` to compile the union of multiple scenes or runs. An explicit
`--shader lightmappedgeneric_ps20b:0,149376` also accepts the engine's **static
base** indices, including the dynamic multiplier. The runner uses the source
schema to decode them, not the installed pack's reported dynamic count.
`--cache /path/to/shared/bytecode` reuses immutable compiler outputs across
separate artifact directories; cache keys include the exact compiler, source
closure, selector, profile, and generated plan.

Output contains `shaders/fxc/<name>.vcs`, authoritative plans, compiler logs,
identity-keyed cached bytecode, and `manifest.json`. The manifest has `schema: 1`,
`status: passed` only after all requests compile, and
`coverage: observed-static-sets`. It records the compiler hash, source revision
and dirty digest, input-log hashes, each source/include/selector/helper hash,
each selected static group and valid dynamic ID, and the final artifact hash.
An input modified during compilation fails publication. New requests reuse
bytecode only when the compiler, source closure, and plan identities match.

The pack uses the existing version-6 VCS format with little-endian fields,
sorted static dictionary and sentinel, no aliases, bounded uncompressed dynamic
blocks, and the include-expanded CRC used by `copyshaders.pl`. Compression is
optional in this existing format; no reader or runtime ABI change is required.

The product harness stages manifest-enumerated artifacts in a private runtime:

```sh
python3 tools/quality/portal_boot.py \
  --runtime /path/to/source-engine-portal-runtime \
  --build build-portal-vulkan \
  --shader-artifacts build-portal-vulkan/shaders/portal-source-matched \
  --require-vulkan --require-sdl3 --require-wayland --out /path/to/new-boot-evidence
```

The artifact directory is not an installed game tree. The original runtime and
VPKs remain unchanged. A scene-specific pack is not a complete shipping shader
library: new materials/maps may require additional static groups. Capture their
demands, extend the artifact set, and rerun native image/diagnostic acceptance.
Successfully compiling bytecode alone does not certify visual correctness.

Conformance tests include an independent VCS reader, a handwritten byte-exact
fixture, multiple-block round trips, preserved skipped IDs, mismatched schemas,
corrupt records, and successful compiler exit without output:

```sh
python3 -m unittest discover -s tools/quality/tests -p test_shader_artifacts.py -v
```
