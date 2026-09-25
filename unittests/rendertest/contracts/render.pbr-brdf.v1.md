# PBR layered direct BRDF v1 (RFC 0007)

`public/render/pbr_brdf.h` evaluates the finite GGX specular BRDF for linear
radiometric values, single-scatter (`EvaluateSpecular`) and with
multiple-scattering energy compensation (`EvaluateSpecularMultiScatter`). Its caller supplies valid dot products in
`[0, 1]` from unit vectors, a normal-incidence reflectance in `[0, 1]`, and a
perceptual roughness in `(0, 1]`. A material clamps zero roughness to its
declared minimum before calling the BRDF; the delta distribution is not a
finite function. `EvaluateLayeredDirect` clamps the material roughness to
0.02, compensates the lobe, and adds Lambertian diffuse weighted by
`1 − E_ms(N·V)` for the nonmetal fraction. Light transport is a separate
contract.

Energy compensation follows Kulla and Conty in the form Filament uses:
`1 + F0·(1/(A + B) − 1)`, where `A + B` is the table's single-scatter albedo
for `F0 = 1`. The multiple-scattering directional albedo
`E_ms = min(1, (F0·A + B)·compensation)` weights image light and leaves
`1 − E_ms` to the diffuse layer. A white conductor's compensated albedo is
exactly one; colored conductors are brightened proportionally and stay below
one. `EvaluateClearCoat` is the dielectric coat of Filament's standard model:
IOR 1.5, GGX with Kelemen visibility `1/(4 (L·H)²)`, returning the coat's
specular BRDF and the Fresnel weight that attenuates the layer beneath.

The GGX denominator is evaluated as `(1 − (N·H)²) + (N·H)²·α²`, which is
algebraically `1 + (N·H)²(α² − 1)` but keeps its float precision at `N·H = 1`
and roughness 0.02, where the textbook form lost a fifth of the value.

The generated split-sum table stores coefficients `A` and `B` such that
`E_spec = F0·A + B`. Its generator uses deterministic GGX importance sampling
with the same distribution and correlated Smith visibility as the BRDF. Both
axes include their end points: texel `i` holds N·V and roughness `i/(size−1)`
(N·V below `1/(2·size)` and roughness below 0.02 are evaluated at those
minima), so roughness one is a real row rather than an extrapolation of
roughness 0.984, whose albedo is higher. The CPU lookup interpolates the
generated C++ table bilinearly at `c·(size−1)`. The Vulkan provider uploads
those coefficients as a float texture, and `PbrSplitSum` samples it linearly
at `(c·(size−1) + 0.5)/size`. The table is sampled in linear space, and the direct shader uses it to weight
the diffuse layer before multiplying by incident radiance and `N·L`.

The distribution is GGX with `alpha = roughness²`; visibility is the
height-correlated Smith term; Fresnel is Schlick with the supplied F0. The
dielectric default F0 is 0.04 (IOR 1.5) and metallic F0 is the linear base
color. A light below the geometric horizon contributes zero. The result is a
BRDF value, before multiplication by `N·L` and incident radiance.

The `render.pbr-brdf` headless suite checks closed-form anchors, GGX
normalization, and a numerical white-furnace integral. Its normal-incidence,
roughness-one furnace value is independently `1 − ln(2)` for F0 = 1. The
unit-energy ceiling detects an oversized BRDF, while the midpoint Fresnel
anchor and roughness-half distribution anchor detect omitted Fresnel and the
wrong roughness mapping. These checks remain active under `NDEBUG`.
The suite also compares split-sum coefficients with an independently sampled
hemisphere integral and checks that a white dielectric's layered furnace
energy closes to one within the table's declared 0.03 tolerance, that a rough
white metal (roughness 0.5 to 1) keeps its energy in a white furnace within
0.03 once compensated, and that compensation brightens a colored metal without
creating energy. Its seeded bad models scale GGX by one half, omit Fresnel,
swap the split-sum coefficients, and leave out energy compensation. Each
sensitivity run passes only when the relevant oracle reports the expected
failure.

`materialsystem/shaderapivulkan/shaders/pbr_brdf.glsl` is the one GPU copy of
these functions; every native PBR stage (`pbr_direct.frag`, `world_pbr.frag`,
`world_pbr_glass.frag`, `model_pbr.frag`) includes it and defines no BRDF of
its own. The `render.pbr-brdf.glsl` GPU suite evaluates GGX, Smith, Schlick,
the lobe, the compensation, the directional albedo and the clear coat in a
compute program (`pbr_brdf_check.comp`) on 1,080 cases and requires each value
within `1e-6 + 3e-4·|C++|` of `pbr_brdf.h`. Its seeded C++ models (no
compensation, `α = roughness`, a coat without its Fresnel weight) must each
disagree.

The native Vulkan pixel suite checks the corresponding GLSL direct shader
against the C++ model and rejects omitted diffuse. Cycles image comparison,
scene lights, baked lighting, and material binding remain R47 gate work.
