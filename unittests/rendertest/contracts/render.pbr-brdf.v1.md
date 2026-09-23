# PBR layered direct BRDF v1 (RFC 0007)

`public/render/pbr_brdf.h` evaluates the finite, single-scatter specular BRDF
for linear radiometric values. Its caller supplies valid dot products in
`[0, 1]` from unit vectors, a normal-incidence reflectance in `[0, 1]`, and a
perceptual roughness in `(0, 1]`. A material clamps zero roughness to its
declared minimum before calling the BRDF; the delta distribution is not a
finite function. `EvaluateLayeredDirect` clamps the material roughness to
0.02 and adds Lambertian diffuse weighted by `1 − E_spec(N·V)` for the
nonmetal fraction. Multiple scattering and light transport are separate
contracts.

The generated split-sum table stores coefficients `A` and `B` such that
`E_spec = F0·A + B`. Its generator uses deterministic GGX importance sampling
with the same distribution and correlated Smith visibility as the BRDF. The
CPU lookup uses bilinear interpolation of the generated C++ table. The Vulkan
provider uploads those coefficients as a float texture with clamped linear
sampling. The table is sampled in linear space, and the direct shader uses it to weight
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
energy closes to one within the table's declared 0.03 tolerance. Its seeded
bad models scale GGX by one half, omit Fresnel, and swap the split-sum
coefficients. Each sensitivity run passes only when the relevant oracle
reports the expected failure.

The native Vulkan pixel suite checks the corresponding GLSL direct shader
against the C++ model and rejects omitted diffuse. Cycles image comparison,
scene lights, baked lighting, and material binding remain R47 gate work.
