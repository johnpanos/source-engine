//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.material.v2 clauses shared by the material suite and its
//			sensitivity suite (RFC 0016 K4 "Material suite"): the key-mapping
//			oracle and the derived-copy revision clause. Each returns its
//			violations; the suite requires none for the real mapping and copy,
//			and the sensitivity suite requires them for seeded bad ones.
//
//=============================================================================//

#ifndef RENDERTEST_CORE_MATERIAL_CONFORMANCE_H
#define RENDERTEST_CORE_MATERIAL_CONFORMANCE_H

#include "render/material/registry.h"
#include "render/material/vmt_import.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

namespace rendertest::material
{

using namespace render::material;

// The legacy shader a family's key-mapping probe VMT names.
inline std::string_view ProbeShader( std::string_view family )
{
	for ( const VmtShaderRow &row : BuiltinVmtMapping().shaders )
	{
		if ( row.family == family )
			return row.shader;
	}
	return {};
}

// K1: for every GPU key row of `mapping`, a VMT setting only that key to a
// distinct value imports, applies, and the value lands in the block at the
// parameter the key names (the key without its '$', the families' naming
// rule), in the schema of the built-in families. The expectation is
// independent of the mapping under test, so a row pointing at another
// parameter is caught.
inline std::vector<std::string> KeyMappingViolations( const VmtMappingTable &mapping )
{
	std::vector<std::string> violations;
	FamilyRegistry reference;
	for ( const FamilyDesc &family : FamiliesFromMapping( BuiltinVmtMapping() ) )
	{
		if ( !reference.Register( family ) )
			violations.push_back( "family " + family.name + " does not register" );
	}
	VmtImportContext context;
	context.mapping = &mapping;
	// The PBR probe needs its required keys and a fallback that exists.
	context.resolve = []( std::string_view path ) -> std::optional<std::string>
	{
		if ( path == "materials/probe/fallback.vmt" )
			return std::string( "LightmappedGeneric { $basetexture a }" );
		return std::nullopt;
	};
	float next = 0.125f;
	for ( const VmtKeyRow &row : mapping.keys )
	{
		if ( row.kind == ValueKind::kTexture || row.kind == ValueKind::kMaterial )
			continue;
		const std::string key( row.key );
		const std::string expected = key.substr( 1 );
		const FamilySchema *schema = reference.Find( row.family );
		const std::optional<std::size_t> index =
		    schema ? schema->IndexOf( expected ) : std::nullopt;
		if ( !index )
		{
			violations.push_back( key + ": the " + std::string( row.family ) +
			                      " family has no parameter " + expected );
			continue;
		}
		next += 1.0f;
		std::string value = std::to_string( static_cast<int>( next ) );
		float wanted[4] = { next, next, next, next };
		wanted[0] = static_cast<float>( static_cast<int>( next ) );
		if ( row.kind == ValueKind::kBool || row.kind == ValueKind::kEnum )
		{
			value = "1";
			wanted[0] = 1.0f;
		}
		else if ( row.kind != ValueKind::kInt )
		{
			value = "[" + std::to_string( next ) + " " + std::to_string( next + 0.25f ) + " " +
			        std::to_string( next + 0.5f ) + " " + std::to_string( next + 0.75f ) + "]";
			wanted[0] = next;
			wanted[1] = next + 0.25f;
			wanted[2] = next + 0.5f;
			wanted[3] = next + 0.75f;
		}
		std::string text =
		    std::string( ProbeShader( row.family ) ) + " { \"" + key + "\" \"" + value + "\" ";
		if ( row.family == "pbr" )
			text += "$basetexture a $mraotexture b $fallbackmaterial probe/fallback ";
		text += "}";
		auto imported = ImportVmt( text, context );
		if ( !imported )
		{
			violations.push_back( key + ": does not import (" + imported.Error().detail + ")" );
			continue;
		}
		ParameterBlock block( *schema );
		if ( !ApplyValues( imported.Value(), block ) )
		{
			violations.push_back(
			    key + ": its value does not apply to the " + std::string( row.family ) + " block" );
			continue;
		}
		const ParameterLayout &layout = schema->layout[*index];
		const int components = layout.type == ParameterType::kFloat2   ? 2
		                       : layout.type == ParameterType::kFloat3 ? 3
		                       : layout.type == ParameterType::kFloat4 ? 4
		                                                               : 1;
		for ( int i = 0; i < components; ++i )
		{
			float stored = 0.0f;
			if ( layout.type == ParameterType::kInt )
			{
				std::int32_t integer = 0;
				std::memcpy( &integer, block.Bytes().data() + layout.offset, sizeof( integer ) );
				stored = static_cast<float>( integer );
			}
			else
			{
				std::memcpy(
				    &stored, block.Bytes().data() + layout.offset + 4 * i, sizeof( stored ) );
			}
			if ( stored != wanted[i] )
			{
				violations.push_back( key + ": " + expected + "[" + std::to_string( i ) + "] is " +
				                      std::to_string( stored ) + ", the VMT set " +
				                      std::to_string( wanted[i] ) );
				break;
			}
		}
	}
	return violations;
}

// R1: a derived copy follows its block. After every change the copy is not
// current; after a refresh it is, and its bytes, textures and revision equal
// the block's. `Copy` provides Current(block), Refresh(block), Bytes(),
// Textures() and Revision().
template <typename Copy> std::vector<std::string> DerivedCopyViolations()
{
	std::vector<std::string> violations;
	FamilyRegistry registry;
	FamilyDesc desc;
	desc.name = "copied";
	desc.parameters = { { "alpha", ParameterType::kFloat, { 1.0f } },
	    { "tint", ParameterType::kFloat3, { 1.0f, 1.0f, 1.0f } },
	    { "base", ParameterType::kTexture, {} } };
	auto id = registry.Register( desc );
	if ( !id )
		return { "the probe family does not register" };
	ParameterBlock block( *registry.Find( id.Value() ) );
	Copy copy;
	const auto matches = [&]( const char *when )
	{
		const bool same =
		    copy.Revision() == block.Revision() && copy.Bytes().size() == block.Bytes().size() &&
		    std::memcmp( copy.Bytes().data(), block.Bytes().data(), block.Bytes().size() ) == 0 &&
		    copy.Textures().size() == block.Textures().size() &&
		    std::equal( copy.Textures().begin(), copy.Textures().end(), block.Textures().begin() );
		if ( !same )
			violations.push_back( std::string( "the copy differs from the block " ) + when );
	};
	if ( copy.Current( block ) )
		violations.push_back( "a new copy claims to be current" );
	(void)copy.Refresh( block );
	matches( "after the first refresh" );
	const float tints[3][3] = {
	    { 0.5f, 0.25f, 1.0f }, { 0.5f, 0.25f, 1.0f }, { 0.0f, 1.0f, 0.0f } };
	for ( int step = 0; step < 3; ++step )
	{
		const std::uint64_t before = block.Revision();
		(void)block.SetFloat( "alpha", 0.1f * static_cast<float>( step + 1 ) );
		(void)block.SetFloat3( "tint", tints[step] );
		(void)block.SetTexture( "base", render::device::TextureId{ 3u + step } );
		if ( block.Revision() != before && copy.Current( block ) )
			violations.push_back( "the copy claims to be current after a change" );
		if ( !copy.Current( block ) )
			(void)copy.Refresh( block );
		matches( "after a change and a refresh" );
	}
	const std::uint64_t revision = copy.Revision();
	if ( copy.Refresh( block ) || copy.Revision() != revision )
		violations.push_back( "refreshing a current copy copied again" );
	return violations;
}

} // namespace rendertest::material

#endif // RENDERTEST_CORE_MATERIAL_CONFORMANCE_H
