//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose: The CPU decompressor for VTF 7.6 AXC mip runs (P2:CE / Strata).
//=============================================================================//
#ifndef TEXTURECONTAINER_VTF_DECOMPRESS_H
#define TEXTURECONTAINER_VTF_DECOMPRESS_H

#include "texturecontainer/vtf_container.h"

namespace texturecontainer::vtf
{
// Decompresses one run into exactly `outputSize` bytes. Returns null on
// success or a static diagnostic. Raw pointers only, so the legacy engine
// (pre-C++11 std::string ABI) calls it directly.
[[nodiscard]] const char *DecompressInto( CompressionMethod method, const void *input,
    std::size_t inputSize, void *output, std::size_t outputSize ) noexcept;

// A Decompressor for ReadImage: Deflate through zlib and Zstandard through
// libzstd. Fills `output` exactly or fails with a static message in `error`;
// a product built without libzstd refuses Zstandard runs by name.
bool Decompress( CompressionMethod method, std::string_view input, std::span<std::uint8_t> output,
    std::string &error );

// Whether this build decodes the method at all (Zstandard needs libzstd).
[[nodiscard]] bool SupportsDecompression( CompressionMethod method ) noexcept;
} // namespace texturecontainer::vtf
#endif
