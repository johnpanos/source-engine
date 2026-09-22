//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of VMT material parsing and the material catalog. See
//			public/hammer/formats/material.h for the contract.
//
//=============================================================================//

#include "hammer/formats/material.h"

#include "hammer/formats/keyvalues.h"

#include <algorithm>
#include <cctype>

namespace hammer::formats
{

namespace
{

std::string ToLower( std::string s )
{
	std::transform( s.begin(), s.end(), s.begin(), []( unsigned char c ) {
		return static_cast<char>( std::tolower( c ) );
	} );
	return s;
}

bool IEquals( const std::string &a, const std::string &b )
{
	if ( a.size() != b.size() )
		return false;
	for ( std::size_t i = 0; i < a.size(); ++i )
	{
		if ( std::tolower( static_cast<unsigned char>( a[i] ) )
		    != std::tolower( static_cast<unsigned char>( b[i] ) ) )
			return false;
	}
	return true;
}

// Finds the first value for 'key' (case-insensitive) in a block's own pairs and,
// recursively, in its child blocks (so a patch's insert/replace sub-block is
// covered). Returns nullptr when not present.
const std::string *FindCI( const KeyValueNode &node, const std::string &key )
{
	for ( const KeyValue &kv : node.pairs )
	{
		if ( IEquals( kv.key, key ) )
			return &kv.value;
	}
	for ( const KeyValueNode &child : node.children )
	{
		if ( const std::string *v = FindCI( child, key ) )
			return v;
	}
	return nullptr;
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
	// Strip a leading slash.
	std::size_t start = 0;
	while ( start < s.size() && s[start] == '/' )
		++start;
	s.erase( 0, start );
	// Strip an optional "materials/" prefix (authored names sometimes carry it).
	const std::string kPrefix = "materials/";
	if ( s.size() >= kPrefix.size() && s.compare( 0, kPrefix.size(), kPrefix ) == 0 )
		s.erase( 0, kPrefix.size() );
	// Strip a trailing ".vmt" or ".vtf".
	auto ends_with = []( const std::string &str, const std::string &suffix ) {
		return str.size() >= suffix.size()
		    && str.compare( str.size() - suffix.size(), suffix.size(), suffix ) == 0;
	};
	if ( ends_with( s, ".vmt" ) || ends_with( s, ".vtf" ) )
		s.erase( s.size() - 4 );
	return s;
}

bool ParseMaterial( const std::string &vmtText, MaterialInfo &out, std::string &includeOut, std::string &error )
{
	ParseResult parsed = ParseKeyValues( vmtText );
	if ( !parsed.ok )
	{
		error = "vmt: " + parsed.error;
		return false;
	}
	if ( parsed.root.children.empty() )
	{
		error = "vmt: no top-level material block";
		return false;
	}

	const KeyValueNode &block = parsed.root.children.front();
	out.shader = ToLower( block.name );
	out.baseTexture.clear();
	includeOut.clear();

	if ( const std::string *bt = FindCI( block, "$basetexture" ) )
		out.baseTexture = CanonicalizeMaterialName( *bt );

	if ( out.shader == "patch" )
	{
		if ( const std::string *inc = FindCI( block, "include" ) )
			includeOut = CanonicalizeMaterialName( *inc );
	}
	return true;
}

MaterialCatalog::MaterialCatalog( const hammer::ports::IAssetSource &source ) : m_source( source ) {}

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

const MaterialInfo *MaterialCatalog::Material( const std::string &name )
{
	const std::string canonical = CanonicalizeMaterialName( name );

	auto hit = m_materials.find( canonical );
	if ( hit != m_materials.end() )
		return &hit->second;
	if ( m_materialMisses.count( canonical ) )
		return nullptr;

	std::string vmt;
	if ( !m_source.ReadAsset( "materials/" + canonical + ".vmt", vmt ) )
	{
		m_materialMisses.insert( canonical );
		return nullptr;
	}

	MaterialInfo info;
	std::string include;
	std::string error;
	if ( !ParseMaterial( vmt, info, include, error ) )
	{
		m_materialMisses.insert( canonical );
		return nullptr;
	}
	info.name = canonical;

	// Resolve one level of patch: pull shader/base texture from the include when the
	// patch itself does not override them.
	if ( info.shader == "patch" && !include.empty() )
	{
		std::string baseVmt;
		if ( m_source.ReadAsset( "materials/" + include + ".vmt", baseVmt ) )
		{
			MaterialInfo baseInfo;
			std::string baseInclude;
			std::string baseError;
			if ( ParseMaterial( baseVmt, baseInfo, baseInclude, baseError ) )
			{
				info.shader = baseInfo.shader;
				if ( info.baseTexture.empty() )
					info.baseTexture = baseInfo.baseTexture;
			}
		}
	}

	auto inserted = m_materials.emplace( canonical, std::move( info ) );
	return &inserted.first->second;
}

const VtfImage *MaterialCatalog::BaseTextureImage( const std::string &name )
{
	const std::string canonical = CanonicalizeMaterialName( name );

	auto cached = m_images.find( canonical );
	if ( cached != m_images.end() )
		return cached->second.get();

	const MaterialInfo *info = Material( canonical );
	if ( !info || info->baseTexture.empty() )
	{
		m_images.emplace( canonical, nullptr );
		return nullptr;
	}

	std::string vtf;
	if ( !m_source.ReadAsset( "materials/" + info->baseTexture + ".vtf", vtf ) )
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

	auto inserted = m_images.emplace( canonical, std::make_unique<VtfImage>( std::move( *image ) ) );
	return inserted.first->second.get();
}

} // namespace hammer::formats
