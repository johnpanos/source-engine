//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 resources (RFC 0016). Handles are strong ids
//			that carry a generation, so a released handle never names a newer
//			resource. Descriptions are immutable after creation.
//
//=============================================================================//

#ifndef RENDER_DEVICE_RESOURCES_H
#define RENDER_DEVICE_RESOURCES_H

#include "foundation/strong_id.h"
#include "render/device/usage.h"

#include <cstdint>
#include <optional>
#include <string_view>

namespace render::device
{

using BufferId = foundation::StrongId<struct BufferTag, std::uint64_t>;
using TextureId = foundation::StrongId<struct TextureTag, std::uint64_t>;
using SamplerId = foundation::StrongId<struct SamplerTag, std::uint64_t>;
using PipelineId = foundation::StrongId<struct PipelineTag, std::uint64_t>;
using BindGroupLayoutId = foundation::StrongId<struct BindGroupLayoutTag, std::uint64_t>;
using BindGroupId = foundation::StrongId<struct BindGroupTag, std::uint64_t>;

enum class ResourceKind : std::uint8_t
{
	kNone,
	kBuffer,
	kTexture,
	kSampler,
	kPipeline,
	kBindGroupLayout,
	kBindGroup
};

// Any releasable resource.
struct ResourceId
{
	ResourceKind kind = ResourceKind::kNone;
	std::uint64_t value = 0;

	constexpr ResourceId() = default;
	constexpr ResourceId( BufferId id ) : kind( ResourceKind::kBuffer ), value( id.value ) {}
	constexpr ResourceId( TextureId id ) : kind( ResourceKind::kTexture ), value( id.value ) {}
	constexpr ResourceId( SamplerId id ) : kind( ResourceKind::kSampler ), value( id.value ) {}
	constexpr ResourceId( PipelineId id ) : kind( ResourceKind::kPipeline ), value( id.value ) {}
	constexpr ResourceId( BindGroupLayoutId id )
	    : kind( ResourceKind::kBindGroupLayout ), value( id.value )
	{
	}
	constexpr ResourceId( BindGroupId id ) : kind( ResourceKind::kBindGroup ), value( id.value ) {}

	friend constexpr bool operator==( const ResourceId &, const ResourceId & ) = default;
};

enum class Format : std::uint8_t
{
	kUnknown,
	kR8Unorm,
	kRGBA8Unorm,
	kRGBA8Srgb,
	kBGRA8Unorm,
	kBGRA8Srgb,
	kRG16Float,
	kRGBA16Float,
	kR32Float,
	kRGBA32Float,
	kD32Float,
	kD24UnormS8,
	kD32FloatS8,  // D32 depth with an 8-bit stencil
	kRGBA16Unorm, // D3D9's A16B16G16R16 (integer-HDR lightmap pages)
	// Block-compressed, 4x4 texels per block (D3D9's DXT1/DXT3/DXT5, ATI1N,
	// ATI2N); sampled and copied only, on a device that claims
	// Capability::kTextureCompressionBC. BC1 keeps its one-bit alpha.
	kBC1Unorm,
	kBC1Srgb,
	kBC2Unorm,
	kBC2Srgb,
	kBC3Unorm,
	kBC3Srgb,
	kBC4Unorm,
	kBC5Unorm,
	// BC6H (unsigned half-float RGB) and BC7 (RGBA), 16-byte 4x4 blocks, under
	// the same capability (Vulkan's textureCompressionBC; GL 4.2's BPTC).
	kBC6HUfloat,
	kBC7Unorm,
	kBC7Srgb,
	kRGB10A2Unorm, // packed R10 G10 B10 A2; HDR10 presentation
	// Packed unsigned floats: R and G 11-bit (5-bit exponent, 6-bit mantissa),
	// B 10-bit (5-bit mantissa); half of kRGBA16Float's bytes, no alpha.
	kRG11B10Float,
	// ETC1 (RGB, 4x4 blocks of 8 bytes, each its specification's 64-bit word
	// in Khronos byte order) and the 3DS's ETC1A4 (16-byte blocks: a 64-bit
	// little-endian word of 4-bit alpha, texel (x, y) at bit 4 * (4x + y),
	// then the ETC1 word little-endian), on a device that claims
	// Capability::kTextureCompressionETC1 (clause D40). Blocks follow each
	// other in raster order, as the kBC* formats' do.
	kETC1Rgb,
	kETC1A4,
	kCount
};

// Bytes per texel of an uncompressed format; 0 for a block-compressed one
// (use BlockOf and RegionBytes).
std::uint32_t BytesPerTexel( Format format );
bool IsDepthFormat( Format format );
bool IsBlockCompressed( Format format );
// One kRG11B10Float texel from linear RGB: round to nearest even; negatives
// and NaN become 0; values past the largest finite clamp to it.
std::uint32_t PackRG11B10Float( float r, float g, float b );

// A format's unit of storage: one texel, or a block of a compressed format.
struct FormatBlock
{
	std::uint32_t width = 1;
	std::uint32_t height = 1;
	std::uint32_t bytes = 0;
};
FormatBlock BlockOf( Format format );
// The bytes of a tightly packed width x height region: whole blocks.
std::uint64_t RegionBytes( Format format, std::uint32_t width, std::uint32_t height );
// Whether a copy of width x height texels at (x, y) of a mip that is
// mipWidth x mipHeight lies inside it, starts on a block, covers whole blocks
// (or reaches the mip's edge), and its buffer offset is a multiple of the
// block's bytes.
bool CopyRegionAligned( Format format, std::uint32_t mipWidth, std::uint32_t mipHeight,
    std::uint32_t x, std::uint32_t y, std::uint32_t width, std::uint32_t height,
    std::uint64_t bufferOffset );
// A depth format with a stencil aspect.
bool HasStencil( Format format );

enum class MemoryKind : std::uint8_t
{
	kDeviceLocal,
	kUpload,  // CPU writes, GPU reads
	kReadback // GPU writes, CPU reads through ReadBuffer
};

struct BufferDesc
{
	std::uint64_t size = 0;
	UsageSet usages;
	MemoryKind memory = MemoryKind::kDeviceLocal;
	std::string_view debugName;
};

enum class TextureDimension : std::uint8_t
{
	k2D,
	kCube,
	k3D
};

struct TextureDesc
{
	TextureDimension dimension = TextureDimension::k2D;
	Format format = Format::kUnknown;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t depthOrLayers = 1;
	std::uint32_t mipLevels = 1;
	std::uint32_t sampleCount = 1;
	UsageSet usages;
	std::string_view debugName;
};

enum class Filter : std::uint8_t
{
	kNearest,
	kLinear
};

enum class AddressMode : std::uint8_t
{
	kRepeat,
	kClampToEdge,
	kMirroredRepeat
};

enum class CompareOp : std::uint8_t
{
	kNever,
	kLess,
	kLessEqual,
	kEqual,
	kGreaterEqual,
	kGreater,
	kAlways,
	kNotEqual
};

struct SamplerDesc
{
	Filter minFilter = Filter::kLinear;
	Filter magFilter = Filter::kLinear;
	Filter mipFilter = Filter::kLinear;
	AddressMode address = AddressMode::kRepeat;
	std::uint32_t maxAnisotropy = 1;
	// Absent: ordinary sampling. Present: compare the reference with each
	// depth texel before filtering (D24). Enable and operation are one value.
	std::optional<CompareOp> comparison;

	friend bool operator==( const SamplerDesc &, const SamplerDesc & ) = default;
};

} // namespace render::device

#endif // RENDER_DEVICE_RESOURCES_H
