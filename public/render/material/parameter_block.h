//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.material.v2 parameter blocks (RFC 0016). A block holds one
//			material's parameter values in its family's uniform layout. Setters
//			check name and type against the schema; the revision rises only
//			when a value actually changes, so derived GPU copies know when to
//			update.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_PARAMETER_BLOCK_H
#define RENDER_MATERIAL_PARAMETER_BLOCK_H

#include "foundation/expected.h"
#include "render/device/resources.h"
#include "render/material/registry.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace render::material
{

class ParameterBlock
{
public:
	explicit ParameterBlock( const FamilySchema &family );

	foundation::Expected<void, MaterialError> SetFloat( std::string_view name, float value );
	foundation::Expected<void, MaterialError> SetFloat2(
	    std::string_view name, const float ( &value )[2] );
	foundation::Expected<void, MaterialError> SetFloat3(
	    std::string_view name, const float ( &value )[3] );
	foundation::Expected<void, MaterialError> SetFloat4(
	    std::string_view name, const float ( &value )[4] );
	foundation::Expected<void, MaterialError> SetInt( std::string_view name, std::int32_t value );
	// A texture transform's rows 0 and 1.
	foundation::Expected<void, MaterialError> SetTransform(
	    std::string_view name, const float ( &rows )[8] );
	foundation::Expected<void, MaterialError> SetTexture(
	    std::string_view name, device::TextureId texture );

	std::span<const std::byte> Bytes() const { return m_Bytes; }
	std::span<const device::TextureId> Textures() const { return m_Textures; }
	std::uint64_t Revision() const { return m_Revision; }
	const FamilySchema &Family() const { return *m_Family; }

private:
	foundation::Expected<void, MaterialError> Write(
	    std::string_view name, ParameterType type, const void *data, std::size_t size );

	const FamilySchema *m_Family;
	std::vector<std::byte> m_Bytes;
	std::vector<device::TextureId> m_Textures;
	std::uint64_t m_Revision = 1;
};

// A derived copy of a block (the bytes a GPU upload is made from). It records
// the revision it copied; Current() is false after any change to the block, and
// Refresh() makes it equal again. Consumers upload only from a current copy.
class ParameterBlockCopy
{
public:
	bool Current( const ParameterBlock &block ) const
	{
		return m_Revision == block.Revision() && m_Family == &block.Family();
	}
	// Copies the block when it changed; returns whether it did.
	bool Refresh( const ParameterBlock &block );

	std::span<const std::byte> Bytes() const { return m_Bytes; }
	std::span<const device::TextureId> Textures() const { return m_Textures; }
	std::uint64_t Revision() const { return m_Revision; }

private:
	const FamilySchema *m_Family = nullptr;
	std::vector<std::byte> m_Bytes;
	std::vector<device::TextureId> m_Textures;
	std::uint64_t m_Revision = 0; // blocks start at 1, so a new copy is never current
};

} // namespace render::material

#endif // RENDER_MATERIAL_PARAMETER_BLOCK_H
