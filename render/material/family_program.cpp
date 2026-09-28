//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: What every material family's program shares (RFC 0016 K4); see
//			family_program.h.
//
//=============================================================================//

#include "family_program.h"

#include <cmath>
#include <cstdint>
#include <cstring>

namespace render::material::detail
{

namespace
{

std::size_t ByteSize( ParameterType type )
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
	case ParameterType::kTexture:
		return 0;
	}
	return 0;
}

// Whether a non-texture parameter holds its schema default, compared as the
// block stores it.
bool AtDefault( const FamilySchema &family, std::span<const std::byte> bytes, std::size_t index )
{
	const ParameterLayout &layout = family.layout[index];
	const ParameterDesc &parameter = family.desc.parameters[index];
	if ( layout.type == ParameterType::kInt )
	{
		const std::int32_t value = static_cast<std::int32_t>( parameter.defaults[0] );
		return std::memcmp( bytes.data() + layout.offset, &value, sizeof( value ) ) == 0;
	}
	return std::memcmp(
	           bytes.data() + layout.offset, parameter.defaults, ByteSize( layout.type ) ) == 0;
}

bool IsClaimed( std::string_view name, std::span<const std::string_view> claimed )
{
	for ( std::string_view entry : claimed )
	{
		if ( entry == name )
			return true;
	}
	return false;
}

} // namespace

std::optional<std::string> UnclaimedParameter(
    const ParameterBlock &block, std::span<const std::string_view> claimed )
{
	const FamilySchema &family = block.Family();
	for ( std::size_t i = 0; i < family.desc.parameters.size(); ++i )
	{
		const std::string &name = family.desc.parameters[i].name;
		if ( IsClaimed( name, claimed ) )
			continue;
		const ParameterLayout &layout = family.layout[i];
		const bool set = layout.type == ParameterType::kTexture
		                     ? block.Textures()[layout.offset].IsValid()
		                     : !AtDefault( family, block.Bytes(), i );
		if ( set )
			return "$" + name;
	}
	return std::nullopt;
}

float ReadParameter( const ParameterBlock &block, std::string_view name, std::size_t component )
{
	const FamilySchema &family = block.Family();
	const std::optional<std::size_t> index = family.IndexOf( name );
	if ( !index )
		return 0.0f;
	const ParameterLayout &layout = family.layout[*index];
	if ( layout.type == ParameterType::kInt )
	{
		std::int32_t value = 0;
		std::memcpy( &value, block.Bytes().data() + layout.offset, sizeof( value ) );
		return static_cast<float>( value );
	}
	if ( layout.type == ParameterType::kTexture ||
	     component * sizeof( float ) >= ByteSize( layout.type ) )
		return 0.0f;
	float value = 0.0f;
	std::memcpy( &value, block.Bytes().data() + layout.offset + component * sizeof( float ),
	    sizeof( value ) );
	return value;
}

bool ReadFlag( const ParameterBlock &block, std::string_view name )
{
	return ReadParameter( block, name ) != 0.0f;
}

float SourceGammaToLinear( float gamma )
{
	if ( gamma > 1.0f )
		return gamma;
	if ( gamma < 0.0f )
		return 0.0f;
	if ( gamma >= 0.95f )
		return 1.0f;
	const int index = static_cast<int>( std::lround( gamma * 255.0f ) );
	return std::pow( static_cast<float>( index ) / 255.0f, 2.2f );
}

} // namespace render::material::detail
