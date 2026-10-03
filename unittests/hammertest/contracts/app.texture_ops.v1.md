# Contract: `app.texture_ops.v1`

Module: `hammer.app`
Header: `public/hammer/app/ops/texture_ops.h` · Impl: `hammer/core/app/ops/texture_ops.cpp`
Conformance: `unittests/hammertest/app/test_texture_ops.cpp`
Migration: `R08-DOMAIN`

## 1. Purpose, consumers, scope

Face texture operations: materials, shift/scale/rotation/lightmap scale, legacy justify and fit, world/face alignment, material replacement, applying one face's texture to others, and the texture-lock rule transforms use. Consumers: the face tool, the face inspector, transform operations, the command layer. Required.

## 2. Accepted inputs

Face references (solid id + side VMF id); materials by name; the IMaterialInfo port for mapping sizes (justify, fit, shift normalization).

## 3. Results and guarantees

- Texel space is legacy: u = dot(p, u.axis) / u.scale + u.shift.
- `LockTexture` keeps every point's texel under any invertible affine map (translation, rotation, non-uniform scale, mirror); a singular map leaves the texture unchanged.
- Justify/fit follow legacy `JustifyTextureUsingExtents`; 'treat as one' uses the union extent. Shifts are wrapped by the material size.
- An unknown face, a zero scale, a non-positive lightmap scale, a material without a size or a fit count below 1 refuses the whole operation with nothing staged.
- `ReplaceMaterial` (Replace Textures) is specified by [`app.replace_textures.v1`](app.replace_textures.v1.md): legacy exact, partial and substitute matching under RFC 0015's material identity, hidden objects, rescale through IMaterialInfo, and `MarkMaterialUses` for mark only.

## 4. Ownership, threading

Pure functions over a borrowed `scene::DocumentEdit`; the session commits the
edit as one history unit. Single sequence.

## 5. Invariants

Every successful result passes `scene::ValidateEdit`. A refusal returns
`Rejected` or `Nothing`; the session then discards the staged edit.

## 6. Side effects and performance

None beyond the staged edit. Cost is proportional to the objects touched.

## 7. Conformance suite and providers

`test_texture_ops.cpp` (76 checks): texture lock under five maps, alignment and rotation, shift wrapping, every operation and each refusal. Built with gcc and clang (`-Wall -Wextra -Werror`) on
`linux-headless-core`.
