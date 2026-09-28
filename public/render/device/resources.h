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
	kCount
};

std::uint32_t BytesPerTexel( Format format );
bool IsDepthFormat( Format format );

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

struct SamplerDesc
{
	Filter minFilter = Filter::kLinear;
	Filter magFilter = Filter::kLinear;
	Filter mipFilter = Filter::kLinear;
	AddressMode address = AddressMode::kRepeat;
	std::uint32_t maxAnisotropy = 1;
};

} // namespace render::device

#endif // RENDER_DEVICE_RESOURCES_H
