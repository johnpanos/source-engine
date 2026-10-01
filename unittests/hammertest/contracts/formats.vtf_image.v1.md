# Contract: `formats.vtf_image.v1`

Module: `hammer.formats` · Header: `public/hammer/formats/vtf_image.h`
Impl: `hammer/core/formats/vtf_image.cpp`
Conformance: `unittests/hammertest/formats/test_vtf_image.cpp` (+ `_negative`)
Migration: `HAM-ASSET-001`

A strict-core decoder for Valve Texture Format (VTF) images. It decodes mip 0 /
frame 0 / face 0 to top-to-bottom RGBA8 — the preview an editor needs. It does
not reproduce the engine VTF pipeline (cubemaps, volume slices, animation frames
beyond the first, HDR float formats).

## 1. Purpose, consumers

Turn VTF bytes into an RGBA image for a material thumbnail, textured viewport,
or offline content import. Consumers: the material catalog and Portal PBR import
tool. No tier0, `bitmap`/`ImageLoader`, or `CVTFTexture`. BC7 decoding uses the
pinned, MIT-licensed, header-only `external/bcdec`; CPU decompression is a narrow
caller-supplied capability so the strict module does not link zlib or Zstandard.

## 2. Accepted inputs

- `ReadVtfInfo( bytes, error )`: header facts (dimensions, version, format, mip
  and frame counts, flags) without decoding pixels.
- `DecodeVtf( bytes, error[, decompressor] )`: a whole VTF file (major version 7,
  minor 1–6). The original two-argument interface remains available and rejects
  payloads that need a decompressor.
  Header fields are read by absolute byte offset, little-endian. The high-res
  image offset comes from the 7.3+ resource dictionary when present, else from
  `headerSize + thumbnail size`. Mips are stored smallest-first; the decoder
  skips to mip 0. VTF 7.6 `AXC` metadata supplies a compression level, method,
  and one stored length per mip/frame/face run. Nonzero Deflate (8) and Zstandard
  (93) runs require a synchronous `VtfMipDecompressor` callback.
- Supported pixel formats: RGBA8888, ABGR8888, RGB888, BGR888, BGRA8888,
  BGRX8888, ARGB8888, I8, IA88, A8, DXT1, DXT3, DXT5, and Strata BC7 (70).

## 3. Results, error taxonomy

- `DecodeVtf` returns a `VtfImage` (`width*height*4` RGBA bytes), or `nullopt`
  with a diagnostic on: bad signature, unsupported major version, unsupported
  minor version, unsupported pixel or AXC format, zero dimension, volume depth,
  a header shorter than 7.1, malformed resource/AXC metadata, a missing
  decompressor, decompression failure, or truncated mip-0 data. An unsupported
  format is a failure, never a wrong image.
- DXT1 honors 1-bit punch-through alpha (`c0 <= c1` mode); DXT3 explicit and DXT5
  interpolated alpha are decoded into the alpha channel.
- BC7 decoding is checked with bytes from the independent KTX2 fixture. AXC
  fixtures serialize the real resource/table layout and use a fake decompressor,
  keeping container parsing independent from the compression implementation.

## 4. Ownership, threading

- Pure transform over an owned byte string to an owned image. The optional
  decompressor and all borrowed spans are valid only for the synchronous call.
  No globals, no I/O, not internally synchronized.
