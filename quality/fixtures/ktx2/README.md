# RFC 0008 F3 KTX2 reader fixtures

All four files were produced by the KTX-Software revision pinned in
[`ktx2-linux-tools.json`](../../product_profiles/ktx2-linux-tools.json).
`red-8x8-bc7.ktx2` is the packaged UASTC LDR → BC7 sRGB result and contains
four identical 4×4 blocks of solid red. `red-8x8-uastc-master.ktx2` is the
untranscoded master, which the runtime reader must reject. The RGBA8 sRGB
file has four mip levels (8×8, 4×4, 2×2, 1×1); every pixel is opaque red.
`red-8x8-bc1.ktx2` has four exact BC1 blocks and matches the synthetic VTF
BC1 top level.

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `red-8x8-bc7.ktx2` | 288 | `1efc12f50e9d91da85240865e9fcf91dde2f61bd1cbbb488eed8eccfe882273c` |
| `red-8x8-bc1.ktx2` | 240 | `499d6086cfc7c0e0262855ee8908d8033c935c8334101ab85f2e5100355ac7e9` |
| `red-8x8-uastc-master.ktx2` | 272 | `2ca97747539216a4680c2f8d163493263cfa595eec8850e11e59b4a494f826fe` |
| `red-8x8-rgba8-mips.ktx2` | 668 | `76ea33e0ce3a74822a9e4449ab8102c747eb9f8a0d5060d56df76950b2d21df6` |

The reader suite checks the BC7 block bytes, owned storage, all RGBA8 mip
offsets and pixels, corrupt magic, truncation, oversized mip and
supercompression declarations, and master rejection. The native
pixel suite loads the committed BC7 package through the reader before GPU
upload. The VTF/KTX2 suite also compares both formats' BC1 block bytes and
only exposes mips actually authored in the VTF. All suites require their
nonzero `CONFORMANCE` record to pass.

The Hammer GTK preview adapter uses `red-8x8-rgba8-mips.ktx2` through the same
reader. The catalog suite verifies its top-level RGBA pixels and the product
smoke stages it inside a private VPK; a corrupt-signature VPK is the negative
control. The BC7 file is an explicit unsupported-preview fixture until Hammer
can decode or sample packaged BC7 images.
