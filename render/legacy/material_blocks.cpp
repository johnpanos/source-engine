//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RenderMaterialBlocks001 (RFC 0016 K4 "Proxy corpus", the frontend
//			side); see material_blocks.h.
//
//=============================================================================//

#include "frontend_material_blocks.h"

#include "render/material/registry.h"
#include "render/material/vmt_import.h"
#include "render/material/vmt_mapping.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>
#include <vector>

namespace render::legacy
{

using namespace render::material;

namespace
{

void AppendJsonString( std::string &out, std::string_view text )
{
	out += '"';
	for ( const unsigned char c : text )
	{
		if ( c == '"' || c == '\\' )
		{
			out += '\\';
			out += char( c );
		}
		else if ( c < 0x20 || c >= 0x7f )
		{
			char escaped[8];
			std::snprintf( escaped, sizeof( escaped ), "\\u%04x", c );
			out += escaped;
		}
		else
		{
			out += char( c );
		}
	}
	out += '"';
}

void AppendNumber( std::string &out, double value )
{
	char text[32];
	std::snprintf( text, sizeof( text ), "%.9g", value );
	out += text;
}

// The components a parameter type stores (floats), 0 for an integer.
int Components( ParameterType type )
{
	switch ( type )
	{
	case ParameterType::kFloat:
		return 1;
	case ParameterType::kFloat2:
		return 2;
	case ParameterType::kFloat3:
		return 3;
	case ParameterType::kFloat4:
		return 4;
	case ParameterType::kTransform:
		return 8;
	case ParameterType::kInt:
	case ParameterType::kTexture:
		return 0;
	}
	return 0;
}

} // namespace

FrontendMaterialBlocks::FrontendMaterialBlocks()
{
	for ( const FamilyDesc &family : FamiliesFromMapping( BuiltinVmtMapping() ) )
		(void)m_Registry.Register( family );
}

int FrontendMaterialBlocks::FormatBlock( const char *material, const char *pass, const char *shader,
    int count, const char *const *keys, const char *const *values, char *out, int size )
{
	std::vector<VmtPair> variables;
	for ( int i = 0; i < count; ++i )
		variables.push_back( { keys[i] ? keys[i] : "", values[i] ? values[i] : "" } );
	auto mapped = MapVariables( shader ? shader : "", std::move( variables ), {} );
	if ( !mapped )
		return -1;
	const MaterialDesc &desc = mapped.Value();
	const FamilySchema *family = m_Registry.Find( desc.family );
	if ( !family )
		return 0; // the legacy family: its parameters are the variables
	ParameterBlock block( *family );
	const bool applied = ApplyValues( desc, block ).HasValue();

	std::string line = "{\"kind\":\"block\",\"name\":";
	AppendJsonString( line, material ? material : "" );
	line += ",\"pass\":";
	AppendJsonString( line, pass ? pass : "" );
	line += ",\"family\":";
	AppendJsonString( line, desc.family );
	line += applied ? ",\"applied\":true" : ",\"applied\":false";
	line += ",\"params\":[";
	bool first = true;
	for ( const MaterialValue &value : desc.values )
	{
		if ( value.kind == ValueKind::kMaterial )
			continue;
		const std::optional<std::size_t> index = family->IndexOf( value.parameter );
		if ( !index )
			continue;
		line += first ? "{\"key\":" : ",{\"key\":";
		first = false;
		AppendJsonString( line, value.key );
		line += ",\"parameter\":";
		AppendJsonString( line, value.parameter );
		const ParameterLayout &layout = family->layout[*index];
		const std::byte *bytes = block.Bytes().data() + layout.offset;
		if ( layout.type == ParameterType::kTexture )
		{
			line += ",\"texture\":";
			AppendJsonString( line, value.text );
		}
		else if ( layout.type == ParameterType::kInt )
		{
			std::int32_t integer = 0;
			std::memcpy( &integer, bytes, sizeof( integer ) );
			line += ",\"int\":" + std::to_string( integer );
			line += value.kind == ValueKind::kBool   ? ",\"kind\":\"bool\""
			        : value.kind == ValueKind::kEnum ? ",\"kind\":\"enum\""
			                                         : ",\"kind\":\"int\"";
		}
		else
		{
			line += ",\"value\":[";
			for ( int c = 0; c < Components( layout.type ); ++c )
			{
				float component = 0.0f;
				std::memcpy( &component, bytes + c * sizeof( float ), sizeof( component ) );
				if ( c )
					line += ',';
				AppendNumber( line, component );
			}
			line += ']';
		}
		line += '}';
	}
	line += "],\"unmapped\":[";
	for ( std::size_t i = 0; i < desc.unmapped.size(); ++i )
	{
		if ( i )
			line += ',';
		AppendJsonString( line, desc.unmapped[i] );
	}
	line += "]}";
	if ( out && size > 0 )
	{
		const std::size_t copied = std::min<std::size_t>( line.size(), std::size_t( size - 1 ) );
		std::memcpy( out, line.data(), copied );
		out[copied] = 0;
	}
	return int( line.size() );
}

} // namespace render::legacy
