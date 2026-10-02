//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose: The shared VTF 7.x container reader. No pixel conversion or engine ABI.
//=============================================================================//
#ifndef TEXTURECONTAINER_VTF_CONTAINER_H
#define TEXTURECONTAINER_VTF_CONTAINER_H

#include "foundation/expected.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace texturecontainer::vtf
{
inline constexpr std::size_t kMaxImageBytes = 512U * 1024U * 1024U;
inline constexpr std::uint32_t kImage = 0x30, kThumbnail = 1, kAuxCompression = 0x435841;
inline constexpr std::uint32_t kInline = 0x02000000, kEnvmap = 0x4000;

enum class CompressionMethod : std::uint16_t
{
	Deflate = 8,
	Zstandard = 93,
};
using Decompressor = bool ( * )(
    CompressionMethod, std::string_view, std::span<std::uint8_t>, std::string & );

struct Resource
{
	std::uint32_t type = 0;
	std::uint32_t value = 0;
};
struct Header
{
	std::uint32_t minor = 0, headerBytes = 0, flags = 0;
	std::uint16_t width = 0, height = 0, depth = 1, frames = 0, startFrame = 0;
	std::uint8_t mips = 0, thumbnailWidth = 0, thumbnailHeight = 0;
	std::int32_t format = -1, thumbnailFormat = -1;
	std::array<float, 3> reflectivity{};
	float bumpScale = 0;
	std::vector<Resource> resources;
};
struct Range
{
	std::size_t offset = 0, size = 0;
};
struct Subresource
{
	std::uint32_t mip = 0, frame = 0, face = 0;
	std::uint32_t width = 0, height = 0, depth = 0;
	Range stored;
	std::size_t decodedBytes = 0;
};
struct Layout
{
	std::uint32_t faces = 1;
	std::optional<CompressionMethod> compression;
	// File order: smallest mip first, then frame, then face. Depth slices are
	// contiguous within a run. Offsets borrow the input; no payload is retained.
	std::vector<Subresource> images;
};

// Header-only callers need the header and dictionary, not image/resource payloads.
// Malformed data is never repaired into a successful parse. The returned values
// own their metadata. Errors are static diagnostics, independent of the engine
// and strict libraries' different std::string ABIs. File offsets are absolute within the supplied VTF bytes.
[[nodiscard]] foundation::Expected<Header, const char *> ReadHeader( std::span<const std::byte> );
// Pass a Header returned by ReadHeader for these same encoded bytes.
// finestMip permits the game's truncated smallest-mips-only read. It does not
// change the authored mip numbering. A full read uses zero.
[[nodiscard]] foundation::Expected<Layout, const char *> ReadLayout(
    std::span<const std::byte>, const Header &, std::uint32_t finestMip = 0 );
[[nodiscard]] foundation::Expected<Range, const char *> ResourceData(
    std::span<const std::byte>, const Header &, const Resource & );
[[nodiscard]] foundation::Expected<std::vector<std::byte>, const char *> ReadImage(
    std::span<const std::byte>, const Layout &, const Subresource &, Decompressor = nullptr );
// Wire-format storage size, including block rounding and volume slices. Zero
// means unsupported format, invalid dimensions, or the bounded size was exceeded.
[[nodiscard]] std::size_t ImageBytes(
    int format, std::uint32_t width, std::uint32_t height, std::uint32_t depth = 1 );
} // namespace texturecontainer::vtf
#endif
