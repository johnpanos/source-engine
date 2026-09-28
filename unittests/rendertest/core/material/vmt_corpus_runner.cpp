//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The VMT corpus runner (RFC 0016 K4, render.material.vmt-corpus).
//			Reads a bundle of one game's VMTs that tools/render/vmt_corpus.py
//			wrote (materials and the patch includes they name), imports every
//			material with ImportVmt for the native Vulkan profile, resolving
//			includes within the bundle, and prints one JSON line per material.
//			The tool compares the counts with the recorded fixture.
//
//=============================================================================//

#include "render/material/vmt_import.h"

#include <cstdio>
#include <fstream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace
{

using render::material::ImportStatusName;
using render::material::ImportVmt;
using render::material::MaterialDesc;
using render::material::VmtImportContext;

struct Bundle
{
	std::map<std::string, std::string, std::less<>> files;
	std::vector<std::string> materials;
};

// "VMTBUNDLE1\n" then per file: "<M|I> <path bytes> <data bytes>\n<path>\n<data>\n".
bool ReadBundle( const char *path, Bundle &bundle )
{
	std::ifstream stream( path, std::ios::binary );
	const std::string data(
	    ( std::istreambuf_iterator<char>( stream ) ), std::istreambuf_iterator<char>() );
	const std::string magic = "VMTBUNDLE1\n";
	if ( data.compare( 0, magic.size(), magic ) != 0 )
		return false;
	std::size_t at = magic.size();
	while ( at < data.size() )
	{
		const std::size_t line = data.find( '\n', at );
		if ( line == std::string::npos )
			return false;
		char kind = 0;
		std::size_t pathBytes = 0;
		std::size_t dataBytes = 0;
		std::istringstream header( data.substr( at, line - at ) );
		header >> kind >> pathBytes >> dataBytes;
		if ( !header || ( kind != 'M' && kind != 'I' ) ||
		     line + 1 + pathBytes + 1 + dataBytes + 1 > data.size() )
			return false;
		std::string file = data.substr( line + 1, pathBytes );
		bundle.files[file] = data.substr( line + 1 + pathBytes + 1, dataBytes );
		if ( kind == 'M' )
			bundle.materials.push_back( std::move( file ) );
		at = line + 1 + pathBytes + 1 + dataBytes + 1;
	}
	return true;
}

std::string Json( std::string_view text )
{
	std::string out = "\"";
	for ( const char c : text )
	{
		const unsigned char byte = static_cast<unsigned char>( c );
		if ( c == '"' || c == '\\' )
		{
			out += '\\';
			out += c;
		}
		else if ( byte < 0x20 || byte >= 0x7f )
		{
			char escaped[8];
			std::snprintf( escaped, sizeof( escaped ), "\\u%04x", byte );
			out += escaped;
		}
		else
		{
			out += c;
		}
	}
	return out + "\"";
}

std::string Record( const std::string &path, const MaterialDesc &material )
{
	std::string out =
	    "{\"path\":" + Json( path ) + ",\"status\":\"ok\",\"family\":" + Json( material.family ) +
	    ",\"shader\":" + Json( material.legacyShader ) +
	    ",\"fallback\":" + Json( material.fallbackBlock ) +
	    ",\"values\":" + std::to_string( material.values.size() ) +
	    ",\"proxies\":" + std::to_string( material.proxies.size() ) +
	    ",\"includes\":" + std::to_string( material.includes.size() ) +
	    ",\"diagnostics\":" + std::to_string( material.diagnostics.size() ) + ",\"unmapped\":[";
	for ( std::size_t i = 0; i < material.unmapped.size(); ++i )
		out += ( i ? "," : "" ) + Json( material.unmapped[i] );
	out += "],\"proxy_names\":[";
	for ( std::size_t i = 0; i < material.proxies.size(); ++i )
		out += ( i ? "," : "" ) + Json( material.proxies[i].name );
	return out + "]}";
}

} // namespace

int main( int argc, char **argv )
{
	if ( argc != 2 )
	{
		std::fprintf( stderr, "usage: vmt_corpus_runner <bundle>\n" );
		return 2;
	}
	Bundle bundle;
	if ( !ReadBundle( argv[1], bundle ) )
	{
		std::fprintf( stderr, "vmt_corpus_runner: %s is not a VMT bundle\n", argv[1] );
		return 2;
	}
	VmtImportContext context;
	context.resolve = [&bundle]( std::string_view path ) -> std::optional<std::string>
	{
		const auto found = bundle.files.find( path );
		if ( found == bundle.files.end() )
			return std::nullopt;
		return found->second;
	};
	for ( const std::string &path : bundle.materials )
	{
		context.path = path;
		auto imported = ImportVmt( bundle.files[path], context );
		if ( imported )
		{
			std::printf( "%s\n", Record( path, imported.Value() ).c_str() );
			continue;
		}
		std::printf( "{\"path\":%s,\"status\":%s,\"detail\":%s}\n", Json( path ).c_str(),
		    Json( ImportStatusName( imported.Error().status ) ).c_str(),
		    Json( imported.Error().detail ).c_str() );
	}
	return 0;
}
