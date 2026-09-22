//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the VMT material-definition parser. See
//			public/hammer/formats/material.h.
//
//=============================================================================//

#include "hammer/formats/material.h"

#include <cctype>
#include <string>

namespace hammer::formats
{

namespace
{

std::string Lower( std::string s )
{
	for ( char &c : s )
	{
		c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
	}
	return s;
}

// Case-insensitive lookup within one parameter list.
const std::string *FindCI( const std::vector<KeyValue> &params, const std::string &loweredName )
{
	for ( const KeyValue &kv : params )
	{
		if ( Lower( kv.key ) == loweredName )
		{
			return &kv.value;
		}
	}
	return nullptr;
}

} // namespace

const std::string *Material::Param( const std::string &name ) const
{
	return FindCI( parameters, Lower( name ) );
}

const std::string *Material::ResolvedParam( const std::string &name ) const
{
	const std::string target = Lower( name );
	if ( const std::string *v = FindCI( patchReplace, target ) )
	{
		return v;
	}
	if ( const std::string *v = FindCI( patchInsert, target ) )
	{
		return v;
	}
	return FindCI( parameters, target );
}

bool Material::HasParam( const std::string &name ) const
{
	return Param( name ) != nullptr;
}

std::string Material::BaseTexture() const
{
	const std::string *v = ResolvedParam( "$basetexture" );
	return v != nullptr ? *v : std::string();
}

std::string Material::SurfaceProp() const
{
	const std::string *v = ResolvedParam( "$surfaceprop" );
	return v != nullptr ? *v : std::string();
}

bool Material::IsPatch() const
{
	return Lower( shader ) == "patch";
}

std::optional<Material> ParseMaterial( const std::string &vmtText )
{
	ParseResult pr = ParseKeyValues( vmtText );
	if ( !pr.ok || pr.root.children.empty() )
	{
		return std::nullopt;
	}

	// The first top-level block is the shader; its pairs are the parameters.
	const KeyValueNode &shaderBlock = pr.root.children.front();
	Material material;
	material.shader = shaderBlock.name;
	material.parameters = shaderBlock.pairs;
	for ( const KeyValueNode &child : shaderBlock.children )
	{
		const std::string lname = Lower( child.name );
		if ( lname == "proxies" )
		{
			material.hasProxies = true;
		}
		else if ( lname == "replace" )
		{
			material.patchReplace = child.pairs;
		}
		else if ( lname == "insert" )
		{
			material.patchInsert = child.pairs;
		}
	}
	return material;
}

} // namespace hammer::formats
