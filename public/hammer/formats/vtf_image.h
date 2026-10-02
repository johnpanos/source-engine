//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dependency-free decoder for Valve Texture Format (VTF) images (RFC
//			0002, hammer.formats). Decodes the largest mip level (mip 0, frame 0,
//			face 0) of a VTF into a top-to-bottom, tightly-packed RGBA8 image --
//			exactly what an editor material browser thumbnail and a textured
//			viewport need. It does NOT reproduce the engine's full VTF pipeline
//			(cubemaps, volume slices, animation frames beyond the first, sphere
//			maps, HDR float formats); those are out of scope for a 2D preview.
//
//			Container parsing is owned by texturecontainer::vtf. This adapter
//			only converts the selected image's pixels into the editor preview.
//
//			Supported pixel formats (the ones shipped content uses for base
//			textures): RGBA8888, ABGR8888, RGB888, BGR888, BGRA8888, BGRX8888,
//			ARGB8888, I8, IA88, A8, and the block-compressed DXT1/DXT3/DXT5 and
//			Strata BC7. VTF 7.6 CPU-compressed mip payloads are decoded through an
//			explicit callback supplied by the composition root, keeping this codec
//			independent of zlib and Zstandard.
//			An unsupported format is a decode failure with a diagnostic, never a
//			silent wrong image.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_VTF_IMAGE_H
#define HAMMER_FORMATS_VTF_IMAGE_H

#include "texturecontainer/vtf_container.h"

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::formats
{

// A decoded 32-bit image. 'rgba' is width*height*4 bytes, row-major, top row
// first, each pixel R,G,B,A. Empty width/height is never returned on success.
struct VtfImage
{
	int width = 0;
	int height = 0;
	std::vector<std::uint8_t> rgba;
};

// Lightweight header facts, available without decoding pixels.
struct VtfInfo
{
	int width = 0;
	int height = 0;
	int majorVersion = 0;
	int minorVersion = 0;
	int imageFormat = 0; // raw ImageFormat enum value from the file
	int mipCount = 0;
	int frameCount = 0;
	int depth = 1;
	std::uint32_t flags = 0;
};

// Strata Source VTF 7.6 stores each mip/frame/face payload in a separate CPU
// compression frame. The strict codec owns the container parsing while the
// composition root supplies the selected decompressor. 'decoded' is exactly the
// expected output size and must be filled completely on success.
using VtfCompressionMethod = texturecontainer::vtf::CompressionMethod;
using VtfMipDecompressor = texturecontainer::vtf::Decompressor;

// Parses just the VTF header of 'bytes'. Returns nullopt with 'error' set when the
// signature/version is bad or the header is truncated.
std::optional<VtfInfo> ReadVtfInfo( const std::string &bytes, std::string &error );

// Decodes mip 0 / frame 0 / face 0 of the VTF in 'bytes' to RGBA8. The two-argument
// overload preserves the original interface and rejects CPU-compressed payloads.
// A VTF 7.6 file with a nonzero AXC compression level requires the overload with
// 'decompressor'. Returns nullopt with 'error' set on malformed data, missing
// decompression capability, or an unsupported pixel/compression format.
std::optional<VtfImage> DecodeVtf( const std::string &bytes, std::string &error );
std::optional<VtfImage> DecodeVtf(
    const std::string &bytes, std::string &error, VtfMipDecompressor decompressor );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_VTF_IMAGE_H
