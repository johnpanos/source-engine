//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Abstract resource states of render.device.v2 (RFC 0016). The port
//			speaks in usages; an adapter turns a usage change into its API's
//			barriers, layout transitions or nothing. No stage mask, access mask
//			or image layout appears above the port.
//
//=============================================================================//

#ifndef RENDER_DEVICE_USAGE_H
#define RENDER_DEVICE_USAGE_H

#include <cstdint>
#include <initializer_list>

namespace render::device
{

enum class ResourceUsage : std::uint8_t
{
	kUndefined, // contents may be discarded
	kSampled,
	kStorageRead,
	kStorageWrite,
	kColorAttachment,
	kDepthRead,
	kDepthWrite,
	kResolveDestination,
	kCopySource,
	kCopyDestination,
	kPresent,
	kVertex,
	kIndex,
	kIndirect,
	kUniform,
	// Handed to another API (an exported image, external_images.h): the
	// writes are available and the image is in the layout its memory
	// description holds. The next use in the port starts from kUndefined.
	kExternal,
	kCount
};

// The usages a resource may ever take, fixed when it is created.
class UsageSet
{
public:
	constexpr UsageSet() = default;
	constexpr UsageSet( std::initializer_list<ResourceUsage> usages )
	{
		for ( ResourceUsage usage : usages )
			m_Bits |= Bit( usage );
	}

	constexpr bool Has( ResourceUsage usage ) const { return ( m_Bits & Bit( usage ) ) != 0; }
	constexpr bool Empty() const { return m_Bits == 0; }
	constexpr std::uint32_t Bits() const { return m_Bits; }
	constexpr UsageSet &Add( ResourceUsage usage )
	{
		m_Bits |= Bit( usage );
		return *this;
	}

	friend constexpr bool operator==( UsageSet, UsageSet ) = default;

private:
	static constexpr std::uint32_t Bit( ResourceUsage usage )
	{
		return 1u << static_cast<std::uint32_t>( usage );
	}

	std::uint32_t m_Bits = 0;
};

// True for usages that write the resource.
constexpr bool IsWrite( ResourceUsage usage )
{
	switch ( usage )
	{
	case ResourceUsage::kStorageWrite:
	case ResourceUsage::kColorAttachment:
	case ResourceUsage::kDepthWrite:
	case ResourceUsage::kResolveDestination:
	case ResourceUsage::kCopyDestination:
		return true;
	default:
		return false;
	}
}

struct SubresourceRange
{
	std::uint32_t baseMip = 0;
	std::uint32_t mipCount = 1;
	std::uint32_t baseLayer = 0;
	std::uint32_t layerCount = 1;
};

} // namespace render::device

#endif // RENDER_DEVICE_USAGE_H
