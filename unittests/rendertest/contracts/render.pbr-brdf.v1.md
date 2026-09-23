# PBR specular BRDF v1 (RFC 0007)

`public/render/pbr_brdf.h` evaluates the finite, single-scatter specular BRDF
for linear radiometric values. Its caller supplies valid dot products in
`[0, 1]` from unit vectors, a normal-incidence reflectance in `[0, 1]`, and a
perceptual roughness in `(0, 1]`. A material clamps zero roughness to its
declared minimum before calling the BRDF; the delta distribution is not a
finite function. The diffuse layer, directional-albedo table, multiple
scattering, and light transport are separate contracts.

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
The same suite runs two seeded bad models: GGX scaled by one half, and a
specular term that omits Fresnel. Each sensitivity run passes only when the
relevant oracle reports the expected failure.

This suite certifies the scalar C++ model only. The GLSL implementation and
Cycles image comparison are R47 gate work that must be separately evidenced.
