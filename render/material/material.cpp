//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.material.v2 registry, schemas and parameter blocks
//			(RFC 0016).
//
//=============================================================================//

#include "render/device/bind_group.h"
#include "render/material/parameter_block.h"
#include "render/material/registry.h"

#include <cstring>

namespace render::material
{

namespace
{

foundation::Unexpected<MaterialError> Fail( MaterialStatus status )
{
	return foundation::MakeUnexpected( MaterialError{ status } );
}

std::uint32_t SizeOf( ParameterType type )
{
	switch ( type )
	{
	case ParameterType::kFloat:
	case ParameterType::kInt:
		return 4;
	case ParameterType::kFloat2:
		return 8;
	case ParameterType::kFloat3:
		return 12;
	case ParameterType::kFloat4:
		return 16;
	case ParameterType::kTransform:
		return 32;
	case ParameterType::kTexture:
		return 0;
	}
	return 0;
}

std::uint32_t AlignmentOf( ParameterType type )
{
	switch ( type )
	{
	case ParameterType::kFloat2:
		return 8;
	case ParameterType::kFloat3:
	case ParameterType::kFloat4:
	case ParameterType::kTransform:
		return 16;
	default:
		return 4;
	}
}

} // namespace

std::optional<std::size_t> FamilySchema::IndexOf( std::string_view name ) const
{
	for ( std::size_t i = 0; i < desc.parameters.size(); ++i )
	{
		if ( desc.parameters[i].name == name )
			return i;
	}
	return std::nullopt;
}

std::optional<device::Capability> MissingCapability(
    const FamilySchema &family, const device::DeviceFacts &facts )
{
	return device::FirstMissing( facts.capabilities, family.desc.required );
}

foundation::Expected<FamilyId, MaterialError> FamilyRegistry::Register( const FamilyDesc &desc )
{
	if ( desc.name.empty() )
		return Fail( MaterialStatus::kInvalidDescription );
	if ( Find( desc.name ) )
		return Fail( MaterialStatus::kDuplicateFamily );
	if ( desc.bindGroups > device::kMaxBindGroups )
		return Fail( MaterialStatus::kTooManyBindGroups );
	FamilySchema schema;
	schema.desc = desc;
	std::uint32_t offset = 0;
	for ( std::size_t i = 0; i < desc.parameters.size(); ++i )
	{
		const ParameterDesc &parameter = desc.parameters[i];
		if ( parameter.name.empty() )
			return Fail( MaterialStatus::kInvalidDescription );
		for ( std::size_t j = 0; j < i; ++j )
		{
			if ( desc.parameters[j].name == parameter.name )
				return Fail( MaterialStatus::kDuplicateParameter );
		}
		if ( parameter.type == ParameterType::kTexture )
		{
			schema.layout.push_back( { parameter.type, schema.textureSlots++ } );
			continue;
		}
		const std::uint32_t align = AlignmentOf( parameter.type );
		offset = ( offset + align - 1 ) / align * align;
		schema.layout.push_back( { parameter.type, offset } );
		offset += SizeOf( parameter.type );
	}
	schema.blockSize = ( offset + 15u ) / 16u * 16u;
	m_Families.push_back( std::move( schema ) );
	return FamilyId{ static_cast<std::uint32_t>( m_Families.size() ) };
}

const FamilySchema *FamilyRegistry::Find( FamilyId id ) const
{
	return id.IsValid() && id.value <= m_Families.size() ? &m_Families[id.value - 1] : nullptr;
}

const FamilySchema *FamilyRegistry::Find( std::string_view name ) const
{
	for ( const FamilySchema &family : m_Families )
	{
		if ( family.desc.name == name )
			return &family;
	}
	return nullptr;
}

std::optional<FamilyId> FamilyRegistry::IdOf( std::string_view name ) const
{
	for ( std::size_t i = 0; i < m_Families.size(); ++i )
	{
		if ( m_Families[i].desc.name == name )
			return FamilyId{ static_cast<std::uint32_t>( i + 1 ) };
	}
	return std::nullopt;
}

ParameterBlock::ParameterBlock( const FamilySchema &family )
    : m_Family( &family ), m_Bytes( family.blockSize ), m_Textures( family.textureSlots )
{
	for ( std::size_t i = 0; i < family.desc.parameters.size(); ++i )
	{
		const ParameterLayout &layout = family.layout[i];
		if ( layout.type == ParameterType::kTexture )
			continue;
		const ParameterDesc &parameter = family.desc.parameters[i];
		if ( layout.type == ParameterType::kInt )
		{
			const std::int32_t value = static_cast<std::int32_t>( parameter.defaults[0] );
			std::memcpy( m_Bytes.data() + layout.offset, &value, sizeof( value ) );
		}
		else
		{
			std::memcpy(
			    m_Bytes.data() + layout.offset, parameter.defaults, SizeOf( layout.type ) );
		}
	}
}

foundation::Expected<void, MaterialError> ParameterBlock::Write(
    std::string_view name, ParameterType type, const void *data, std::size_t size )
{
	const std::optional<std::size_t> index = m_Family->IndexOf( name );
	if ( !index )
		return Fail( MaterialStatus::kUnknownParameter );
	const ParameterLayout &layout = m_Family->layout[*index];
	if ( layout.type != type )
		return Fail( MaterialStatus::kTypeMismatch );
	std::byte *target = m_Bytes.data() + layout.offset;
	if ( std::memcmp( target, data, size ) != 0 )
	{
		std::memcpy( target, data, size );
		++m_Revision;
	}
	return {};
}

foundation::Expected<void, MaterialError> ParameterBlock::SetFloat(
    std::string_view name, float value )
{
	return Write( name, ParameterType::kFloat, &value, sizeof( value ) );
}

foundation::Expected<void, MaterialError> ParameterBlock::SetFloat2(
    std::string_view name, const float ( &value )[2] )
{
	return Write( name, ParameterType::kFloat2, value, sizeof( value ) );
}

foundation::Expected<void, MaterialError> ParameterBlock::SetFloat3(
    std::string_view name, const float ( &value )[3] )
{
	return Write( name, ParameterType::kFloat3, value, sizeof( value ) );
}

foundation::Expected<void, MaterialError> ParameterBlock::SetFloat4(
    std::string_view name, const float ( &value )[4] )
{
	return Write( name, ParameterType::kFloat4, value, sizeof( value ) );
}

foundation::Expected<void, MaterialError> ParameterBlock::SetTransform(
    std::string_view name, const float ( &rows )[8] )
{
	return Write( name, ParameterType::kTransform, rows, sizeof( rows ) );
}

foundation::Expected<void, MaterialError> ParameterBlock::SetInt(
    std::string_view name, std::int32_t value )
{
	return Write( name, ParameterType::kInt, &value, sizeof( value ) );
}

foundation::Expected<void, MaterialError> ParameterBlock::SetTexture(
    std::string_view name, device::TextureId texture )
{
	const std::optional<std::size_t> index = m_Family->IndexOf( name );
	if ( !index )
		return Fail( MaterialStatus::kUnknownParameter );
	const ParameterLayout &layout = m_Family->layout[*index];
	if ( layout.type != ParameterType::kTexture )
		return Fail( MaterialStatus::kTypeMismatch );
	if ( m_Textures[layout.offset] != texture )
	{
		m_Textures[layout.offset] = texture;
		++m_Revision;
	}
	return {};
}

bool ParameterBlockCopy::Refresh( const ParameterBlock &block )
{
	if ( Current( block ) )
		return false;
	m_Family = &block.Family();
	m_Bytes.assign( block.Bytes().begin(), block.Bytes().end() );
	m_Textures.assign( block.Textures().begin(), block.Textures().end() );
	m_Revision = block.Revision();
	return true;
}

} // namespace render::material
