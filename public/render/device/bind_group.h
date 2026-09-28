//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.device.v2 bind groups (RFC 0016 decision 3). At most four
//			groups, in a fixed role order: frame, view, material, draw. Vulkan
//			guarantees four bound descriptor sets; OpenGL maps each group to a
//			fixed range of its flat binding slots.
//
//=============================================================================//

#ifndef RENDER_DEVICE_BIND_GROUP_H
#define RENDER_DEVICE_BIND_GROUP_H

#include "render/device/resources.h"

#include <cstdint>
#include <initializer_list>
#include <span>

namespace render::device
{

enum class BindGroupRole : std::uint8_t
{
	kFrame = 0,
	kView = 1,
	kMaterial = 2,
	kDraw = 3
};

inline constexpr std::uint32_t kMaxBindGroups = 4;

enum class BindingKind : std::uint8_t
{
	kUniformBuffer,
	kStorageBuffer,
	kSampledTexture,
	kStorageTexture,
	kSampler
};

enum class ShaderStage : std::uint8_t
{
	kVertex,
	kFragment,
	kCompute
};

class ShaderStageSet
{
public:
	constexpr ShaderStageSet() = default;
	constexpr ShaderStageSet( std::initializer_list<ShaderStage> stages )
	{
		for ( ShaderStage stage : stages )
			m_Bits |= static_cast<std::uint8_t>( 1u << static_cast<std::uint32_t>( stage ) );
	}

	constexpr bool Has( ShaderStage stage ) const
	{
		return ( m_Bits & ( 1u << static_cast<std::uint32_t>( stage ) ) ) != 0;
	}
	constexpr std::uint8_t Bits() const { return m_Bits; }

	friend constexpr bool operator==( ShaderStageSet, ShaderStageSet ) = default;

private:
	std::uint8_t m_Bits = 0;
};

struct BindingDesc
{
	std::uint32_t binding = 0;
	BindingKind kind = BindingKind::kUniformBuffer;
	std::uint32_t count = 1;
	ShaderStageSet stages;
};

struct BindGroupLayoutDesc
{
	BindGroupRole role = BindGroupRole::kFrame;
	std::span<const BindingDesc> bindings;
};

struct BindGroupEntry
{
	std::uint32_t binding = 0;
	BufferId buffer;
	std::uint64_t offset = 0;
	std::uint64_t size = 0; // 0: to the end of the buffer
	TextureId texture;
	SamplerId sampler;
};

struct BindGroupDesc
{
	BindGroupLayoutId layout;
	std::span<const BindGroupEntry> entries;
};

} // namespace render::device

#endif // RENDER_DEVICE_BIND_GROUP_H
