# KTX2 host packer

`ktx2_pack.py` transcodes a UASTC KTX2 master into one explicitly selected
package format for RFC 0008 F3. The pinned
[`ktx2-linux-tools.json`](../../quality/product_profiles/ktx2-linux-tools.json)
owns the KTX-Software revision and the supported texture-class/target pairs.
The tool checks the CLI revision, validates the master, checks its UASTC model
and transfer function, validates the output's Vulkan format and image topology,
then atomically replaces the destination. It prints a JSON receipt with source
and package hashes.

```sh
python3 tools/texture/ktx2_pack.py \
  --ktx /tmp/rfc0008-ktx-pin/build-rfc0008/Release/ktx \
  --texture-class base-color --target bc7 \
  --master /path/to/base-color-uastc.ktx2 \
  --out /path/to/package/base-color-bc7.ktx2
```

The product profile can select the target from a Vulkan device query:

```sh
python3 tools/texture/ktx2_select.py --query-device \
  --out quality-results/ktx2-device-selection.json
python3 tools/texture/ktx2_pack.py \
  --ktx /tmp/rfc0008-ktx-pin/build-rfc0008/Release/ktx \
  --texture-class base-color \
  --selection-plan quality-results/ktx2-device-selection.json \
  --master /path/to/base-color-uastc.ktx2 \
  --out /path/to/package/base-color.ktx2
```

The selection tool reads the product profile's ordered choices and requires
sample, linear-filter, and transfer-destination support for every texture
class. Missing support fails the plan. `vulkaninfo --json` describes its first
reported device; the runtime must confirm it selected the same device before
using packages. The real-tool positive and negative checks are in
`tools/quality/ktx2_pack_conformance.py`.

The strict C++20 reader in `texturecontainer/` consumes already transcoded
KTX2 packages and returns owned, container-neutral mip data. It rejects UASTC
masters, unsupported image topology, unexpected orientation, malformed
containers, and mismatched image sizes. The local Linux native Vulkan test
configuration links the profile-pinned read-only KTX library:

```sh
cmake --build /tmp/rfc0008-ktx-pin/build-rfc0008 --target ktx_read -j8
WAFLOCK=.lock-waf-rfc0008-ktx-reader ./waf configure \
  --platform-provider=sdl3 --render-backend=native-vulkan --build-games=portal \
  --ktx-source-root=/tmp/rfc0008-ktx-pin \
  --ktx-build-root=/tmp/rfc0008-ktx-pin/build-rfc0008 \
  --disable-warns -T release -o build-rfc0008-ktx-reader \
  --prefix=/tmp/rfc0008-ktx-reader-install
WAFLOCK=.lock-waf-rfc0008-ktx-reader ./waf build \
  --targets=ktx2_reader_conformance,ktx2_native_pixel_conformance -j8
build-rfc0008-ktx-reader/unittests/texturecontainertest/ktx2_reader_conformance \
  quality/fixtures/ktx2
```

The native pixel test reads the committed BC7 KTX2 fixture through this reader
when configured with the pinned KTX source and build. Material-system VTF/KTX2
selection and the Hammer texture reader are still pending F3 integration.

The VTF adapter uses the existing VTF library to produce the same owned image
description for a 2D, one-frame, one-face material cohort. Its shared-data
test serializes a red four-mip VTF and compares all pixels and dimensions with
the red KTX2 fixture. A second pair checks that a one-mip BC1 VTF and KTX2
expose identical blocks without leaking the VTF decoder's extra allocated
levels:

```sh
WAFLOCK=.lock-waf-rfc0008-ktx-reader ./waf build \
  --targets=vtf_texture_reader_conformance -j8
LD_LIBRARY_PATH=build-rfc0008-ktx-reader/tier0 \
  build-rfc0008-ktx-reader/unittests/texturecontainertest/vtf_texture_reader_conformance \
  quality/fixtures/ktx2
```

Animated, cubemap, volume and other unsupported VTF formats still use their
existing path. The shared description is not yet selected by the material
system for file textures.

The native Vulkan image bridge accepts the same description. It validates
every mip's dimensions and byte count, requires the selected device to support
the requested sample/upload format, and destroys a managed image if an upload
fails. The native pixel test captures the committed BC7 package and a copy of
the four-mip RGBA8 package after bridge upload. The copy marks each lower mip
with a distinct color and samples all four levels:

```sh
WAFLOCK=.lock-waf-rfc0008-ktx-reader ./waf build \
  --targets=ktx2_native_pixel_conformance -j8
python3 tools/quality/ktx2_native_pixels.py \
  --binary build-rfc0008-ktx-reader/unittests/shaderapivulkantest/ktx2_native_pixel_conformance \
  --selection-plan quality-results/rfc0008-f3-reader-device-selection-20260923.json \
  --out quality-results/rfc0008-f3-image-upload-native-20260923.json
```

The material-system file texture caller has not adopted the shared
description. The Hammer GTK material catalog selects packaged KTX2 assets
when built with the pinned reader and previews RGBA8/BGRA8 through that same
reader; compressed package preview remains open.
