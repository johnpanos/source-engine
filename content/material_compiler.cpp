//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Validate VMT with kvtext, retain bytes, record runtime references.
//
//=============================================================================//

#include "content/material_compiler.h"

#include "kvtext/keyvalues.h"

#include <algorithm>
#include <set>

namespace content
{
namespace
{
bool Same( std::string_view left, std::string_view right )
{
	if ( left.size() != right.size() )
		return false;
	for ( std::size_t i = 0; i < left.size(); ++i )
	{
		const char a = left[i] >= 'A' && left[i] <= 'Z' ? char( left[i] + 32 ) : left[i];
		const char b = right[i] >= 'A' && right[i] <= 'Z' ? char( right[i] + 32 ) : right[i];
		if ( a != b )
			return false;
	}
	return true;
}

bool TextureKey( std::string_view key )
{
	// Known Source material texture slots; the extractor is expanded with
	// fixture evidence for shader families before C0 closure is claimed.
	for ( const std::string_view known : { "$basetexture", "$basetexture2", "$bumpmap", "$bumpmap2",
	          "$normalmap", "$detail", "$envmapmask", "$selfillummask", "$lightwarptexture",
	          "$mraotexture", "$texture2", "$dudvmap", "$refracttinttexture", "$flowmap" } )
		if ( Same( key, known ) )
			return true;
	return false;
}

std::optional<AssetRef> TextureRef( std::string_view value )
{
	auto name = NormalizeAssetName( value );
	if ( !name || name->empty() )
		return std::nullopt;
	if ( name->ends_with( ".vtf" ) )
		name->resize( name->size() - 4 );
	if ( !name->starts_with( "materials/" ) )
		*name = "materials/" + *name;
	return AssetRef::Create( AssetKind::Texture, *name );
}

bool SpecialTexture( std::string_view value )
{
	return value.empty() || value.starts_with( "_rt_" ) || Same( value, "env_cubemap" ) ||
	       value.front() == '[';
}

std::optional<AssetRef> IncludeRef( std::string_view value )
{
	auto name = NormalizeAssetName( value );
	if ( !name )
		return std::nullopt;
	if ( name->ends_with( ".vmt" ) )
		name->resize( name->size() - 4 );
	if ( !name->starts_with( "materials/" ) )
		*name = "materials/" + *name;
	return AssetRef::Create( AssetKind::Material, *name );
}

bool Collect( const kvtext::KeyValueNode &block, const AssetRef &source, BuildInputs &inputs,
    std::set<AssetRef> &visited, std::vector<AssetEdge> &edges, std::string &error, unsigned depth )
{
	if ( depth > 10 )
	{
		error = "VMT patch include depth exceeded: " + source.name;
		return false;
	}
	for ( const kvtext::KeyValue &pair : block.pairs )
	{
		if ( Same( pair.key, "include" ) && Same( block.name, "patch" ) )
		{
			auto included = IncludeRef( pair.value );
			if ( !included )
			{
				error = "invalid VMT patch include: " + pair.value;
				return false;
			}
			edges.push_back( { source, *included, false } );
			if ( !visited.insert( *included ).second )
			{
				error = "VMT patch include cycle: " + included->name;
				return false;
			}
			auto bytes = inputs.Read( included->RuntimePath(), &error );
			if ( !bytes )
				return false;
			kvtext::ParseOptions options;
			options.closeBlocksAtEnd = true;
			const kvtext::ParseResult parsed = kvtext::ParseKeyValues(
			    std::string( reinterpret_cast<const char *>( bytes->data() ), bytes->size() ),
			    options );
			if ( !parsed.ok || parsed.root.children.size() != 1 ||
			     !Collect(
			         parsed.root.children[0], source, inputs, visited, edges, error, depth + 1 ) )
			{
				if ( error.empty() )
					error = "invalid included VMT: " + included->name;
				return false;
			}
			visited.erase( *included );
		}
		else if ( TextureKey( pair.key ) )
		{
			if ( SpecialTexture( pair.value ) )
				continue;
			auto texture = TextureRef( pair.value );
			if ( texture )
				edges.push_back( { source, *texture, false } );
			else
			{
				error = "invalid VMT texture reference: " + pair.value;
				return false;
			}
		}
	}
	for ( const kvtext::KeyValueNode &child : block.children )
		if ( !Collect( child, source, inputs, visited, edges, error, depth ) )
			return false;
	return true;
}
} // namespace

std::optional<std::vector<AssetEdge>> MaterialVmtCompiler::Plan(
    const AssetRef &ref, BuildInputs &inputs, std::string &error ) const
{
	if ( ref.kind != Kind() )
	{
		error = "material compiler received another kind";
		return std::nullopt;
	}
	auto bytes = inputs.Read( ref.RuntimePath(), &error );
	if ( !bytes )
		return std::nullopt;
	kvtext::ParseOptions options;
	options.closeBlocksAtEnd = true;
	const kvtext::ParseResult parsed = kvtext::ParseKeyValues(
	    std::string( reinterpret_cast<const char *>( bytes->data() ), bytes->size() ), options );
	if ( !parsed.ok || parsed.root.children.size() != 1 )
	{
		error = "invalid VMT: " + ref.name + ( parsed.ok ? "" : ": " + parsed.error );
		return std::nullopt;
	}
	std::set<AssetRef> visited{ ref };
	std::vector<AssetEdge> edges;
	if ( !Collect( parsed.root.children[0], ref, inputs, visited, edges, error, 0 ) )
		return std::nullopt;
	std::sort( edges.begin(), edges.end(),
	    []( const auto &a, const auto &b )
	    {
		    return a.target < b.target;
	    } );
	edges.erase( std::unique( edges.begin(), edges.end(),
	                 []( const auto &a, const auto &b )
	                 {
		                 return a.target == b.target;
	                 } ),
	    edges.end() );
	return edges;
}

std::optional<std::vector<std::uint8_t>> MaterialVmtCompiler::Compile(
    const AssetRef &ref, BuildInputs &inputs, std::string &error ) const
{
	auto bytes = inputs.Read( ref.RuntimePath(), &error );
	if ( !bytes )
		return std::nullopt;
	return std::vector<std::uint8_t>( bytes->begin(), bytes->end() );
}

} // namespace content
