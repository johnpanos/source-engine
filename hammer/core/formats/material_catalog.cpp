//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the material catalog. See
//			public/hammer/formats/material_catalog.h for the contract. This composes
//			the VMT parser (hammer::formats::ParseMaterial), the VTF decoder, and an
//			IAssetSource; it owns no format knowledge of its own beyond name
//			canonicalization and patch-include following.
//
//=============================================================================//

#include "hammer/formats/material_catalog.h"
#include "render/pbr_material_schema.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <utility>
#include <vector>

namespace hammer::formats
{

namespace
{

constexpr int kMaxPatchDepth = 10; // CMaterial::AccumulateRecursiveVmtPatches limit.

std::string ToLower( std::string s )
{
	std::transform( s.begin(), s.end(), s.begin(),
	    []( unsigned char c )
	    {
		    return static_cast<char>( std::tolower( c ) );
	    } );
	return s;
}

bool SafeMaterialPath( const std::string &name )
{
	if ( name.empty() )
		return false;
	std::size_t start = 0;
	while ( start < name.size() )
	{
		const std::size_t end = name.find( '/', start );
		const std::size_t length = ( end == std::string::npos ? name.size() : end ) - start;
		if ( length == 0 || ( length == 1 && name[start] == '.' ) ||
		     ( length == 2 && name[start] == '.' && name[start + 1] == '.' ) )
			return false;
		for ( std::size_t i = start; i < start + length; ++i )
		{
			if ( static_cast<unsigned char>( name[i] ) < 32 || name[i] == ':' )
				return false;
		}
		if ( end == std::string::npos )
			return true;
		start = end + 1;
	}
	return false;
}

} // namespace

std::string CanonicalizeMaterialName( const std::string &name )
{
	std::string s = ToLower( name );
	for ( char &c : s )
	{
		if ( c == '\\' )
			c = '/';
	}
	std::size_t start = 0;
	while ( start < s.size() && s[start] == '/' )
		++start;
	s.erase( 0, start );
	const std::string kPrefix = "materials/";
	if ( s.size() >= kPrefix.size() && s.compare( 0, kPrefix.size(), kPrefix ) == 0 )
		s.erase( 0, kPrefix.size() );
	auto ends_with = []( const std::string &str, const std::string &suffix )
	{
		return str.size() >= suffix.size() &&
		       str.compare( str.size() - suffix.size(), suffix.size(), suffix ) == 0;
	};
	if ( ends_with( s, ".vmt" ) || ends_with( s, ".vtf" ) )
		s.erase( s.size() - 4 );
	return s;
}

MaterialCatalog::MaterialCatalog( const hammer::ports::IAssetSource &source ) : m_source( source )
{
}

const std::vector<std::string> &MaterialCatalog::MaterialNames()
{
	if ( m_names )
		return *m_names;

	std::vector<std::string> paths;
	m_source.ListAssets( "materials/", ".vmt", paths );

	std::vector<std::string> names;
	names.reserve( paths.size() );
	for ( const std::string &path : paths )
		names.push_back( CanonicalizeMaterialName( path ) );

	std::sort( names.begin(), names.end() );
	names.erase( std::unique( names.begin(), names.end() ), names.end() );
	m_names = std::move( names );
	return *m_names;
}

std::string MaterialCatalog::ResolveBaseTexture( const std::string &name )
{
	const std::string canonical = CanonicalizeMaterialName( name );

	auto hit = m_baseTextures.find( canonical );
	if ( hit != m_baseTextures.end() )
		return hit->second;
	if ( m_baseTextureMisses.count( canonical ) )
		return std::string();

	std::string vmt;
	if ( !m_source.ReadAsset( "materials/" + canonical + ".vmt", vmt ) )
	{
		m_baseTextureMisses.insert( canonical );
		return std::string();
	}

	std::optional<Material> material = ParseMaterial( vmt );
	if ( !material )
	{
		m_baseTextureMisses.insert( canonical );
		return std::string();
	}

	std::string base = material->BaseTexture();

	// A `patch` material takes its base texture from the included material when it
	// declares none itself (the VMT parser leaves the include unresolved by design).
	if ( base.empty() && material->IsPatch() )
	{
		if ( const std::string *include = material->Param( "include" ) )
		{
			const std::string includeCanonical = CanonicalizeMaterialName( *include );
			std::string includeVmt;
			if ( m_source.ReadAsset( "materials/" + includeCanonical + ".vmt", includeVmt ) )
			{
				if ( std::optional<Material> included = ParseMaterial( includeVmt ) )
					base = included->BaseTexture();
			}
		}
	}

	if ( base.empty() )
	{
		m_baseTextureMisses.insert( canonical );
		return std::string();
	}

	const std::string canonicalBase = CanonicalizeMaterialName( base );
	m_baseTextures.emplace( canonical, canonicalBase );
	return canonicalBase;
}

const VtfImage *MaterialCatalog::BaseTextureImage( const std::string &name )
{
	const std::string canonical = CanonicalizeMaterialName( name );

	auto cached = m_images.find( canonical );
	if ( cached != m_images.end() )
		return cached->second.get();

	const std::string base = ResolveBaseTexture( canonical );
	if ( base.empty() )
	{
		m_images.emplace( canonical, nullptr );
		return nullptr;
	}

	std::string vtf;
	if ( !m_source.ReadAsset( "materials/" + base + ".vtf", vtf ) )
	{
		m_images.emplace( canonical, nullptr );
		return nullptr;
	}

	std::string error;
	std::optional<VtfImage> image = DecodeVtf( vtf, error );
	if ( !image )
	{
		m_images.emplace( canonical, nullptr );
		return nullptr;
	}

	auto inserted =
	    m_images.emplace( canonical, std::make_unique<VtfImage>( std::move( *image ) ) );
	return inserted.first->second.get();
}

PbrMaterialCheck MaterialCatalog::ValidatePbrMaterial(
    const std::string &name, SupportsPbrFallbackShader supportsShader, void *context ) const
{
	const std::string canonical = CanonicalizeMaterialName( name );
	if ( !SafeMaterialPath( canonical ) )
		return { PbrMaterialStatus::kInvalidMaterialPath };
	std::string vmt;
	std::set<std::string> materialVisited;
	std::vector<Material> definitions;
	std::string materialPath = canonical;
	for ( int depth = 0; depth < kMaxPatchDepth; ++depth )
	{
		if ( !materialVisited.insert( materialPath ).second )
			return { PbrMaterialStatus::kMaterialCycle };
		if ( !m_source.ReadAsset( "materials/" + materialPath + ".vmt", vmt ) )
			return { PbrMaterialStatus::kMaterialMissing };
		std::optional<Material> parsed = ParseMaterial( vmt );
		if ( !parsed )
			return { PbrMaterialStatus::kMalformedMaterial };
		const bool patch = parsed->IsPatch();
		const std::string *include = patch ? parsed->Param( "include" ) : nullptr;
		const std::string includeName = include ? *include : std::string();
		definitions.push_back( std::move( *parsed ) );
		if ( !patch )
			break;
		if ( !include )
			return { PbrMaterialStatus::kMalformedMaterial };
		materialPath = CanonicalizeMaterialName( includeName );
		if ( !SafeMaterialPath( materialPath ) )
			return { PbrMaterialStatus::kInvalidMaterialPath };
	}
	if ( definitions.empty() || definitions.back().IsPatch() )
		return { PbrMaterialStatus::kMaterialChainTooDeep };
	if ( !render::pbr::IsMetalRoughShader( definitions.back().shader.c_str() ) )
		return { PbrMaterialStatus::kNotPbr };
	// Match CMaterial::ApplyPatchKeyValues for scalar VMT parameters: nested
	// patches accumulate outer to inner, with inner values winning; inserts
	// overwrite, then replaces affect only keys that exist after insertion.
	std::map<std::string, std::string> values;
	for ( const KeyValue &parameter : definitions.back().parameters )
		values[ToLower( parameter.key )] = parameter.value;
	std::map<std::string, std::string> inserts;
	std::map<std::string, std::string> replaces;
	for ( std::size_t i = 0; i + 1 < definitions.size(); ++i )
	{
		for ( const KeyValue &parameter : definitions[i].patchInsert )
			inserts[ToLower( parameter.key )] = parameter.value;
		for ( const KeyValue &parameter : definitions[i].patchReplace )
			replaces[ToLower( parameter.key )] = parameter.value;
	}
	for ( const auto &entry : inserts )
		values[entry.first] = entry.second;
	for ( const auto &entry : replaces )
	{
		auto found = values.find( entry.first );
		if ( found != values.end() )
			found->second = entry.second;
	}

	const auto lookup = []( const char *parameter, void *source ) -> const char *
	{
		const auto *parameters = static_cast<const std::map<std::string, std::string> *>( source );
		const auto found = parameters->find( ToLower( parameter ) );
		return found == parameters->end() ? nullptr : found->second.c_str();
	};
	const render::pbr::DefinitionResult definition =
	    render::pbr::ValidateDefinition( definitions.back().shader.c_str(), lookup, &values );
	if ( definition.status != render::pbr::DefinitionStatus::kValid )
		return { PbrMaterialStatus::kMissingParameter, definition.parameter };

	const char *fallbackReference = lookup(
	    render::pbr::Parameter( render::pbr::MaterialParameter::kFallbackMaterial ).name, &values );
	if ( !render::pbr::IsValidFallbackReference( fallbackReference ) )
		return { PbrMaterialStatus::kInvalidFallbackPath };
	const std::string fallback = CanonicalizeMaterialName( fallbackReference );
	if ( !SafeMaterialPath( fallback ) )
		return { PbrMaterialStatus::kInvalidFallbackPath };
	if ( fallback == canonical )
		return { PbrMaterialStatus::kSelfFallback };

	std::set<std::string> visited;
	visited.insert( canonical );
	std::string current = fallback;
	for ( int depth = 0; depth < kMaxPatchDepth; ++depth )
	{
		if ( !visited.insert( current ).second )
			return { PbrMaterialStatus::kFallbackCycle };
		if ( !m_source.ReadAsset( "materials/" + current + ".vmt", vmt ) )
			return { PbrMaterialStatus::kFallbackMissing };
		std::optional<Material> target = ParseMaterial( vmt );
		if ( !target )
			return { PbrMaterialStatus::kFallbackMalformed };
		if ( target->IsPatch() )
		{
			const std::string *include = target->Param( "include" );
			if ( !include )
				return { PbrMaterialStatus::kFallbackMalformed };
			current = CanonicalizeMaterialName( *include );
			if ( !SafeMaterialPath( current ) )
				return { PbrMaterialStatus::kInvalidFallbackPath };
			continue;
		}
		if ( !supportsShader || !supportsShader( target->shader, context ) )
			return { PbrMaterialStatus::kUnsupportedFallbackShader, nullptr, target->shader };
		return { PbrMaterialStatus::kValid, nullptr, target->shader };
	}
	return { PbrMaterialStatus::kFallbackChainTooDeep };
}

} // namespace hammer::formats
