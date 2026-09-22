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

#include <algorithm>
#include <cctype>

namespace hammer::formats
{

namespace
{

std::string ToLower( std::string s )
{
	std::transform( s.begin(), s.end(), s.begin(),
	    []( unsigned char c )
	    {
		    return static_cast<char>( std::tolower( c ) );
	    } );
	return s;
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

} // namespace hammer::formats
