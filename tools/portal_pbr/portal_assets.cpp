//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Export Portal material facts and decoded VTF pixels for the PBR
//          staging workflow. Format knowledge stays in hammer::formats.
//
//=============================================================================//

#include "hammer/adapters/platform/disk_byte_store.h"
#include "hammer/formats/keyvalues.h"
#include "hammer/formats/material.h"
#include "hammer/formats/vpk_archive.h"
#include "hammer/formats/vtf_image.h"
#include "render/pbr_material_schema.h"

#include <png.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <string>

namespace
{

std::string JsonQuote( const std::string &value )
{
	std::string out = "\"";
	constexpr char digits[] = "0123456789abcdef";
	for ( unsigned char c : value )
	{
		switch ( c )
		{
		case '"':
			out += "\\\"";
			break;
		case '\\':
			out += "\\\\";
			break;
		case '\n':
			out += "\\n";
			break;
		case '\r':
			out += "\\r";
			break;
		case '\t':
			out += "\\t";
			break;
		default:
			if ( c < 0x20 )
			{
				out += "\\u00";
				out += digits[c >> 4];
				out += digits[c & 15];
			}
			else
			{
				out += static_cast<char>( c );
			}
		}
	}
	return out + '"';
}

bool IsMaterialPath( const std::string &path, const std::string &extension )
{
	return path.starts_with( "materials/" ) && path.ends_with( extension ) &&
	       path.find( ".." ) == std::string::npos;
}

int PrintSchema()
{
	using render::pbr::MaterialParameter;
	using render::pbr::Parameter;
	std::cout << "{\"shader\":\"PBRMetalRough\""
	          << ",\"base\":" << JsonQuote( Parameter( MaterialParameter::kBaseTexture ).name )
	          << ",\"mrao\":" << JsonQuote( Parameter( MaterialParameter::kMraoTexture ).name )
	          << ",\"normal\":" << JsonQuote( Parameter( MaterialParameter::kBumpMap ).name )
	          << ",\"alphatest\":" << JsonQuote( Parameter( MaterialParameter::kAlphaTest ).name )
	          << ",\"alphatestreference\":"
	          << JsonQuote( Parameter( MaterialParameter::kAlphaTestReference ).name )
	          << ",\"translucent\":"
	          << JsonQuote( Parameter( MaterialParameter::kTranslucent ).name ) << ",\"fallback\":"
	          << JsonQuote( Parameter( MaterialParameter::kFallbackMaterial ).name ) << "}\n";
	return std::cout.good() ? 0 : 1;
}

std::string ReadFile( const std::string &path )
{
	std::ifstream file( path, std::ios::binary );
	if ( !file )
		return {};
	return std::string( std::istreambuf_iterator<char>( file ), std::istreambuf_iterator<char>() );
}

int Validate( const std::string &candidatePath, const std::string &fallbackPath )
{
	auto candidate = hammer::formats::ParseMaterial( ReadFile( candidatePath ) );
	const auto fallback = hammer::formats::ParseMaterial( ReadFile( fallbackPath ) );
	if ( !candidate || !fallback || fallback->IsPatch() )
	{
		std::cerr << "candidate or fallback VMT is invalid\n";
		return 3;
	}
	const auto lookup = []( const char *parameter, void *context ) -> const char *
	{
		const auto *material = static_cast<const hammer::formats::Material *>( context );
		const std::string *value = material->Param( parameter );
		return value ? value->c_str() : nullptr;
	};
	const auto definition =
	    render::pbr::ValidateDefinition( candidate->shader.c_str(), lookup, &*candidate );
	if ( definition.status != render::pbr::DefinitionStatus::kValid )
	{
		std::cerr << "candidate does not satisfy PBR schema: "
		          << ( definition.parameter ? definition.parameter : "shader" ) << '\n';
		return 3;
	}
	std::cout << "{\"valid\":true,\"fallback_shader\":" << JsonQuote( fallback->shader ) << "}\n";
	return 0;
}

bool WriteBytes( const std::filesystem::path &path, const std::string &bytes )
{
	std::error_code error;
	std::filesystem::create_directories( path.parent_path(), error );
	if ( error )
		return false;
	std::ofstream output( path, std::ios::binary );
	output.write( bytes.data(), static_cast<std::streamsize>( bytes.size() ) );
	return output.good();
}

void PrintParameters( const std::vector<hammer::formats::KeyValue> &parameters )
{
	std::cout << '[';
	bool first = true;
	for ( const auto &parameter : parameters )
	{
		if ( !first )
			std::cout << ',';
		first = false;
		std::cout << '[' << JsonQuote( parameter.key ) << ',' << JsonQuote( parameter.value )
		          << ']';
	}
	std::cout << ']';
}

bool EqualsIgnoreCase( const std::string &a, const std::string &b )
{
	return a.size() == b.size() &&
	       std::equal( a.begin(), a.end(), b.begin(), []( unsigned char x, unsigned char y )
	           { return std::tolower( x ) == std::tolower( y ); } );
}

// The DirectX 9 shader-fallback block ("<Shader>_HDR_DX9", else "<Shader>_DX9")
// overrides top-level parameters on the hardware the PBR family replaces. Its
// $bumpmap/$ssbump are otherwise invisible to the top-level material model.
std::optional<hammer::formats::KeyValueNode> FindDx9Block( const std::string &vmtText )
{
	const hammer::formats::ParseResult parsed = hammer::formats::ParseKeyValues( vmtText );
	if ( !parsed.ok || parsed.root.children.empty() )
		return std::nullopt;
	const hammer::formats::KeyValueNode &shader = parsed.root.children.front();
	for ( const char *suffix : { "_HDR_DX9", "_DX9" } )
	{
		for ( const auto &child : shader.children )
		{
			if ( EqualsIgnoreCase( child.name, shader.name + suffix ) )
				return child;
		}
	}
	return std::nullopt;
}

void PrintMaterialRecord( const std::string &path, const std::string &bytes )
{
	const auto material = hammer::formats::ParseMaterial( bytes );
	std::cout << "{\"path\":" << JsonQuote( path );
	if ( !material )
	{
		std::cout << ",\"error\":\"cannot parse VMT\"}\n";
		return;
	}
	std::cout << ",\"shader\":" << JsonQuote( material->shader )
	          << ",\"proxies\":" << ( material->hasProxies ? "true" : "false" )
	          << ",\"parameters\":";
	PrintParameters( material->parameters );
	std::cout << ",\"patch_replace\":";
	PrintParameters( material->patchReplace );
	std::cout << ",\"patch_insert\":";
	PrintParameters( material->patchInsert );
	const auto dx9 = FindDx9Block( bytes );
	std::cout << ",\"dx9_block\":" << ( dx9 ? JsonQuote( dx9->name ) : std::string( "null" ) )
	          << ",\"dx9_parameters\":";
	PrintParameters( dx9 ? dx9->pairs : std::vector<hammer::formats::KeyValue>() );
	std::cout << "}\n";
}

int Scan( const hammer::formats::VpkArchive &archive, const std::filesystem::path &legacyDir )
{
	for ( const auto &entry : archive.Entries() )
	{
		if ( !IsMaterialPath( entry.path, ".vmt" ) )
			continue;
		std::string bytes;
		if ( !archive.ReadAsset( entry.path, bytes ) )
		{
			std::cout << "{\"path\":" << JsonQuote( entry.path )
			          << ",\"error\":\"cannot read VMT\"}\n";
			continue;
		}
		const std::filesystem::path outputPath = legacyDir / entry.path.substr( 10 );
		if ( !WriteBytes( outputPath, bytes ) )
		{
			std::cerr << "cannot write " << outputPath << '\n';
			return 1;
		}
		PrintMaterialRecord( entry.path, bytes );
	}
	return std::cout.good() ? 0 : 1;
}

int PrintMaterial( const hammer::formats::VpkArchive &archive, const std::string &path,
    const char *vmtOutput )
{
	if ( !IsMaterialPath( path, ".vmt" ) )
	{
		std::cerr << "invalid material path\n";
		return 2;
	}
	std::string bytes;
	if ( !archive.ReadAsset( path, bytes ) )
	{
		std::cerr << "cannot read " << path << '\n';
		return 4;
	}
	if ( vmtOutput && !WriteBytes( vmtOutput, bytes ) )
	{
		std::cerr << "cannot write " << vmtOutput << '\n';
		return 1;
	}
	PrintMaterialRecord( path, bytes );
	return std::cout.good() ? 0 : 1;
}

int Decode( const hammer::formats::VpkArchive &archive, const std::string &path,
    const std::string &outputPath )
{
	if ( !IsMaterialPath( path, ".vtf" ) )
	{
		std::cerr << "invalid texture path\n";
		return 2;
	}
	std::string bytes;
	if ( !archive.ReadAsset( path, bytes ) )
	{
		std::cerr << "cannot read " << path << '\n';
		return 4;
	}
	std::string error;
	const auto info = hammer::formats::ReadVtfInfo( bytes, error );
	if ( !info )
	{
		std::cerr << error << '\n';
		return 2;
	}
	// This decoder yields only one frame/face/slice. Never mistake a cubemap or
	// animation for a complete editable texture set.
	if ( info->frameCount != 1 || ( info->flags & 0x00004000u ) != 0 )
	{
		std::cerr << "animated or environment VTF requires review\n";
		return 2;
	}
	// VTF 7.2+ stores depth as a little-endian u16 at 0x3f. The shared preview
	// decoder deliberately returns only slice zero, so reject volume textures.
	if ( info->minorVersion >= 2 )
	{
		if ( bytes.size() < 0x41 )
		{
			std::cerr << "truncated VTF depth field\n";
			return 2;
		}
		const auto *raw = reinterpret_cast<const unsigned char *>( bytes.data() );
		const unsigned depth = raw[0x3f] | ( unsigned( raw[0x40] ) << 8 );
		if ( depth > 1 )
		{
			std::cerr << "volume VTF requires review\n";
			return 2;
		}
	}
	const auto image = hammer::formats::DecodeVtf( bytes, error );
	if ( !image )
	{
		std::cerr << error << '\n';
		return 2;
	}
	png_image png{};
	png.version = PNG_IMAGE_VERSION;
	png.width = static_cast<png_uint_32>( image->width );
	png.height = static_cast<png_uint_32>( image->height );
	png.format = PNG_FORMAT_RGBA;
	const std::filesystem::path output( outputPath );
	std::error_code fsError;
	std::filesystem::create_directories( output.parent_path(), fsError );
	if ( fsError ||
	     !png_image_write_to_file( &png, outputPath.c_str(), 0, image->rgba.data(), 0, nullptr ) )
	{
		std::cerr << "PNG write failed: " << png.message << '\n';
		png_image_free( &png );
		return 1;
	}
	png_image_free( &png );
	std::cout << "{\"path\":" << JsonQuote( path ) << ",\"width\":" << info->width
	          << ",\"height\":" << info->height << ",\"flags\":" << info->flags
	          << ",\"format\":" << info->imageFormat << ",\"mips\":" << info->mipCount
	          << ",\"frames\":" << info->frameCount << "}\n";
	return 0;
}

} // namespace

int main( int argc, char **argv )
{
	if ( argc == 2 && std::string( argv[1] ) == "schema" )
		return PrintSchema();
	if ( argc == 4 && std::string( argv[1] ) == "validate" )
		return Validate( argv[2], argv[3] );
	if ( argc < 4 )
	{
		std::cerr << "usage: portal_assets scan <dir.vpk> <legacy-dir> | "
		             "decode <dir.vpk> <materials/path.vtf> <output.png> | "
		             "material <dir.vpk> <materials/path.vmt> [original.vmt]\n";
		return 2;
	}
	hammer::adapters::platform::DiskByteStore store;
	std::string error;
	std::unique_ptr<hammer::formats::VpkArchive> archive =
	    hammer::formats::VpkArchive::Open( store, argv[2], error );
	if ( !archive )
	{
		std::cerr << error << '\n';
		return 2;
	}
	const std::string mode = argv[1];
	if ( mode == "scan" && argc == 4 )
		return Scan( *archive, argv[3] );
	if ( mode == "material" && ( argc == 4 || argc == 5 ) )
		return PrintMaterial( *archive, argv[3], argc == 5 ? argv[4] : nullptr );
	if ( mode == "decode" && argc == 5 )
		return Decode( *archive, argv[3], argv[4] );
	std::cerr << "invalid arguments\n";
	return 2;
}
