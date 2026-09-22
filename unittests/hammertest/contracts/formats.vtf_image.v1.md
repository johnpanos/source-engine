# Contract: `formats.vtf_image.v1`

Module: `hammer.formats` · Header: `public/hammer/formats/vtf_image.h`
Impl: `hammer/core/formats/vtf_image.cpp`
Conformance: `unittests/hammertest/formats/test_vtf_image.cpp` (+ `_negative`)
Migration: `HAM-ASSET-001`

A dependency-free decoder for Valve Texture Format (VTF) images. It decodes mip 0
/ frame 0 / face 0 to top-to-bottom RGBA8 — the preview an editor needs. It does
not reproduce the engine VTF pipeline (cubemaps, volume slices, animation frames
beyond the first, HDR float formats).

## 1. Purpose, consumers

Turn VTF bytes into an RGBA image for a material thumbnail or a textured
viewport. Consumers: the material catalog. C++ stdlib only — no tier0, no
`bitmap`/`ImageLoader`, no `CVTFTexture`. Required for texture preview.

## 2. Accepted inputs

- `ReadVtfInfo( bytes, error )`: header facts (dimensions, version, format, mip
  and frame counts, flags) without decoding pixels.
- `DecodeVtf( bytes, error )`: a whole VTF file (major version 7, minor 1–5).
  Header fields are read by absolute byte offset, little-endian. The high-res
  image offset comes from the 7.3+ resource dictionary when present, else from
  `headerSize + thumbnail size`. Mips are stored smallest-first; the decoder
  skips to mip 0.
- Supported pixel formats: RGBA8888, ABGR8888, RGB888, BGR888, BGRA8888,
  BGRX8888, ARGB8888, I8, IA88, A8, DXT1, DXT3, DXT5.

## 3. Results, error taxonomy

- `DecodeVtf` returns a `VtfImage` (`width*height*4` RGBA bytes), or `nullopt`
  with a diagnostic on: bad signature, unsupported major version, unsupported
  pixel format, zero dimension, a header shorter than 7.1, or image data
  truncated for mip 0. An unsupported format is a failure, never a wrong image.
- DXT1 honors 1-bit punch-through alpha (`c0 <= c1` mode); DXT3 explicit and DXT5
  interpolated alpha are decoded into the alpha channel.

## 4. Ownership, threading

- Pure transform over an owned byte string to an owned image. No globals, no I/O,
  not internally synchronized.
