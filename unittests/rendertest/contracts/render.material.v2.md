# Contract: `render.material.v2` and `render.shader-artifacts.v1` (first slice)

Modules: `render.material` (layer 3), `render.shader-library` and
`render.resources` (layer 2)
Suites: `unittests/rendertest/core/material/test_material.cpp`,
`unittests/rendertest/core/shaderlib/test_shaderlib.cpp`,
`unittests/rendertest/core/resources/test_resources.cpp`
Rows: R88 (RFC 0016 K4; this is its first slice)

| Clause | Obligation |
| --- | --- |
| M1 | A family's parameters take std140-style offsets; textures take slots |
| M2 | Duplicate families or parameters, and families of more than four bind groups, fail |
| M3 | A family's missing capability is named |
| M4 | Parameter blocks start at the defaults; setters check name and type; the revision rises only on a real change |
| S1 | The artifact store rejects duplicate keys and empty artifacts |
| S2 | Permutation keys are a bijection over the declared axes |
| S3 | A recipe resolves only to the requested artifact format, failing by key otherwise, and survives a move |
| R1–R5 | Resource caches upload staged bytes, end in their use usage, keep a replaced resource live until the replacing submission completes, and release everything at teardown |

## K4 clauses (2026-09-28)

Suites: `test_material.cpp` (`render.material.v2`), `test_material_sensitivity.cpp`
(`render.material.v2.sensitivity`), the corpus (`render.material.vmt-corpus`,
`.sensitivity`, `.asan`) and the artifacts (`render.shader-artifacts`,
`.sensitivity`). The shared clauses are `material_conformance.h`.

| Clause | Obligation |
| --- | --- |
| F1–F4 | The VMT mapping defines the core families `lightmapped`, `vertexlit`, `unlit` and `pbr` (in that order); each registers with the four groups; the schema is exactly the GPU key rows, PBR's is RFC 0007's; defaults come from the rows; the legacy shader table is populated and its aliases resolve |
| K1 | Every GPU key row's value lands in the parameter its key names (the key without its `$`) in the built-in schema. A row mapped to another parameter, or read with another kind, is caught (sensitivity S1, S2) |
| R1 | A derived copy (`ParameterBlockCopy`) is not current after any change to its block and equals it (bytes, textures, revision) after a refresh; refreshing a current copy copies nothing. A copy never refreshed after a change, and one whose revision moves without its bytes, are caught (S3, S4) |
| V1–V10 | VMT reading follows the material system: `[$SYMBOL]` tags, `cond?` variables, the profile's fallback block, first definitions (a conditional one replaces), aliases, the legacy family keeping every variable, `subrect`, unknown shaders by name, texture prefixes, enumerations, braced colors, scalars filling vectors, unreadable values reported, proxies, editor and metadata keys, unmapped keys, malformed text, the end of input closing blocks |
| P1–P3 | Patches: includes resolve in order; insert adds and overwrites; replace overwrites only existing keys; missing, absent and more than ten nested includes fail by status |
| B1–B3 | PBRMetalRough: required parameters are named when absent; the fallback reference must be well formed, exist and not name the material itself |
| A1–A2 | `ApplyValues` writes a material's values at their offsets and raises the revision; a block of another family refuses them |
| C1 | Every VMT of the Portal and Portal 2 corpus imports or is reported with its status; the family counts, unsupported counts, unsupported materials and the family digest equal `quality/fixtures/render-material/vmt-corpus-v1.json`; a seeded wrong shader mapping and a missing VMT are caught |
| A-ART | Every shader the backend ships, and every family program, builds a SPIR-V and a GLSL 4.50 artifact (or a declared exclusion); reflection matches `render/shaders/layouts.json`; a family declares at most four groups; the generated headers agree with the regenerators and the compiler (none is committed) |
| F-UNLIT | `render.family.unlit`: the `unlit` family (`unlit_family.h`) claims UnlitGeneric's base texture, `$color`/`$alpha`, vertex color and alpha, alpha test, and translucent or additive blending; it refuses any other parameter set away from its default, by name. Each case of `quality/fixtures/legacy-shaders/families/unlit.vdf`, imported and claimed, drawn with the family into an sRGB target with the D3D9 half-pixel shift a legacy draw carries, matches the legacy port's pixels within 2 levels (`quality/fixtures/render-families/unlit-port-v1.vdf`, recorded from a port run judged against the retail D3D9 bytecode). Source's gamma rules apply: `$color` through the 2.2 table (1 from 0.95), vertex color through pow 2.2; destination alpha is kept for translucent and alpha-tested draws (D17). Seeded: `$vertexcolor` packed off is caught |
| F-LIGHTMAPPED | `render.family.lightmapped`: the `lightmapped` family (`lightmapped_family.h`) claims LightmappedGeneric's and WorldVertexTransition's base texture, `$color`/`$alpha`, vertex color, vertex alpha blending, alpha test and translucency; it refuses any other parameter set away from its default, by name (bump maps, `$basetexture2`, env maps, `$additive`). The lightmap page and its sampler are the draw's bind group (role `kDraw`); the material group holds the constants, the base texture and its sampler. Each case of `quality/fixtures/legacy-shaders/families/lightmapped.vdf`, drawn as `F-UNLIT` draws its cases, matches the legacy port's pixels within 2 levels (`quality/fixtures/render-families/lightmapped-port-v1.vdf`). The port's rules apply: the tint is `$color` times the lightmap scale 2^2.2 with no gamma conversion; vertex color is used unconverted; on the vertex fast path the vertex alpha replaces the modulation alpha, and without `$vertexcolor` `$alpha` applies twice. Seeded: `$color` gamma-converted is caught |
| F-PBR | `render.family.pbr`: the `pbr` family (`pbr_family.h`) claims RFC 0007 PBRMetalRough's `$basetexture`, `$mraotexture`, `$bumpmap`, `$emissiontexture` and `$emissionscale` (`$fallbackmaterial` is accepted); it refuses environment maps, alpha test, translucency, clear coat and glass by name, and a block without both required textures bound. The frame group holds the split-sum table (`SplitSumTable`), the view group Source's model lighting (`PackSourceModelLighting`: ambient cube, up to four lights sorted spot, point, directional, with `SetLight`'s cone), the material group the constants and four texture/sampler pairs; the draw constants are world-to-clip and object-to-world (the `FamilyDrawConstants` prefix). The BRDF is `render/shaders/common/pbr_brdf.glsl`, the one GLSL copy. Each case of the recorded native `pbr-model` run (`quality/fixtures/render-families/pbr-port-v1.vdf`, `family_port_pixels.py record-model`: the harness's own inputs and the port's pixels where its BRDF oracle judges) matches the port within 2 levels. A directional light shines along its direction. Seeded: the normal map ignored is caught |

Open for K4: the `vertexlit` family with its
pixel oracle, running the proxy corpus on the core (the legacy capture
exists), and texture transforms in the family schemas.
