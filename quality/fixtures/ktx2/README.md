# RFC 0008 F3 KTX2 reader fixtures

All eight files were produced by the KTX-Software revision pinned in
[`ktx2-linux-tools.json`](../../product_profiles/ktx2-linux-tools.json).
`red-8x8-bc7.ktx2` is the packaged UASTC LDR → BC7 sRGB result and contains
four identical 4×4 blocks of solid red. `red-8x8-uastc-master.ktx2` is the
untranscoded master, which the runtime reader must reject. The RGBA8 sRGB
file has four mip levels (8×8, 4×4, 2×2, 1×1); every pixel is opaque red.
`red-8x8-bc1.ktx2` has four exact BC1 blocks and matches the synthetic VTF
BC1 top level.

The three `mrao-8x8` files come from one linear UASTC master of opaque
RGBA pixels `(255, 128, 255, 255)`: R is metalness, G roughness and B AO.
The packer emitted BC7, ASTC 4×4 and ETC2 RGBA UNORM packages from that master.
`white-1x1-rgba16f.ktx2` contains four linear half-float 1.0 channels and
checks the BSP2 WMSH lightmap upload boundary.

| File | Bytes | SHA-256 |
| --- | ---: | --- |
| `red-8x8-bc7.ktx2` | 288 | `1efc12f50e9d91da85240865e9fcf91dde2f61bd1cbbb488eed8eccfe882273c` |
| `red-8x8-bc1.ktx2` | 240 | `499d6086cfc7c0e0262855ee8908d8033c935c8334101ab85f2e5100355ac7e9` |
| `red-8x8-uastc-master.ktx2` | 272 | `2ca97747539216a4680c2f8d163493263cfa595eec8850e11e59b4a494f826fe` |
| `red-8x8-rgba8-mips.ktx2` | 668 | `76ea33e0ce3a74822a9e4449ab8102c747eb9f8a0d5060d56df76950b2d21df6` |
| `mrao-8x8-bc7.ktx2` | 288 | `38bc7a3e453f340a1569fa8f15848ab78a9c12277ddbfa4f478ea351a8fd402c` |
| `mrao-8x8-astc.ktx2` | 288 | `1de4e0d4dca21481f80baf0bc1cf067e62a282dba353f58ca24ce75ff98ec487` |
| `mrao-8x8-etc-rgba.ktx2` | 304 | `875a708ed8230d45844c137b45d7ed04aabce59c2e20f37a45d386ae1b0152e9` |
| `white-1x1-rgba16f.ktx2` | 264 | `1a247228520bad11fcb72163f68bb77d218beed506eb4f36df1b3c23663a50d3` |

The reader suite checks the BC7 block bytes, owned storage, all RGBA8 mip
offsets and pixels, corrupt magic, truncation, oversized mip and
supercompression declarations, and master rejection. The native
pixel suite loads the committed sRGB and linear BC7 packages through the reader
before GPU upload. The VTF/KTX2 suite also compares both formats' BC1 block bytes and
only exposes mips actually authored in the VTF. All suites require their
nonzero `CONFORMANCE` record to pass.

The Hammer GTK preview adapter uses `red-8x8-rgba8-mips.ktx2` through the same
reader. The catalog suite verifies its top-level RGBA pixels and the product
smoke stages it inside a private VPK; a corrupt-signature VPK is the negative
control. The BC7 file is an explicit unsupported-preview fixture until Hammer
can decode or sample packaged BC7 images.
