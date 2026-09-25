//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Embedded SPIR-V names and debug variants (vulkan_shader_library.h).
//
//===========================================================================//

#include "vulkan_shader_library.h"

#include "material_spv_index.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <system_error>

namespace render_vulkan
{

namespace
{

const uint32_t kSpirvMagic = 0x07230203u;
const uint32_t kOpExtInstImport = 11;
const size_t kSpirvHeaderWords = 5;

// "<array>.<16 lowercase hex digits>.spv" -> the hash, else false.
bool ParseDebugFileName( const std::string &file, uint64_t *outHash )
{
	const size_t suffix = file.size() >= 4 ? file.size() - 4 : 0;
	if ( suffix < 18 || file.compare( suffix, 4, ".spv" ) != 0 || file[suffix - 17] != '.' )
		return false;
	uint64_t hash = 0;
	for ( size_t i = suffix - 16; i < suffix; ++i )
	{
		const char c = file[i];
		const int digit = ( c >= '0' && c <= '9' )   ? c - '0'
		                  : ( c >= 'a' && c <= 'f' ) ? c - 'a' + 10
		                                             : -1;
		if ( digit < 0 )
			return false;
		hash = ( hash << 4 ) | uint64_t( digit );
	}
	*outHash = hash;
	return true;
}

} // namespace

uint64_t SpirvHash( const uint32_t *code, size_t sizeBytes )
{
	uint64_t hash = 0xCBF29CE484222325ull;
	const unsigned char *bytes = reinterpret_cast<const unsigned char *>( code );
	for ( size_t i = 0; i < sizeBytes; ++i )
	{
		hash ^= bytes[i];
		hash *= 0x100000001B3ull;
	}
	return hash;
}

const char *EmbeddedShaderName( uint64_t hash )
{
	for ( const MaterialSpvIndexEntry &entry : g_materialSpvIndex )
		if ( entry.hash == hash )
			return entry.source;
	return nullptr;
}

bool SpirvUsesNonSemanticInfo( const uint32_t *code, size_t words )
{
	for ( size_t at = kSpirvHeaderWords; at < words; )
	{
		const uint32_t count = code[at] >> 16;
		if ( count == 0 || at + count > words )
			return false;
		if ( ( code[at] & 0xFFFFu ) == kOpExtInstImport && count > 2 )
		{
			const char *name = reinterpret_cast<const char *>( &code[at + 2] );
			const size_t bytes = ( count - 2 ) * sizeof( uint32_t );
			if ( strnlen( name, bytes ) >= 12 && std::strncmp( name, "NonSemantic.", 12 ) == 0 )
				return true;
		}
		at += count;
	}
	return false;
}

bool VulkanShaderLibrary::AddDebugVariant( uint64_t hash, std::vector<uint32_t> code,
    bool nonSemanticInfoSupported, std::string *outReason )
{
	const auto reject = [&]( const char *reason )
	{
		if ( outReason )
			*outReason = reason;
		return false;
	};
	if ( !EmbeddedShaderName( hash ) )
		return reject( "no embedded shader has this hash (material_spv.h changed since the "
		               "debug shaders were written; rerun regen_material_spv.py --debug-out)" );
	if ( code.size() <= kSpirvHeaderWords || code[0] != kSpirvMagic )
		return reject( "not SPIR-V" );
	if ( !nonSemanticInfoSupported && SpirvUsesNonSemanticInfo( code.data(), code.size() ) )
		return reject( "needs VK_KHR_shader_non_semantic_info, which the device lacks" );
	m_debug[hash] = std::move( code );
	return true;
}

VulkanShaderLibrary::LoadReport VulkanShaderLibrary::LoadDebugDirectory(
    const char *directory, bool nonSemanticInfoSupported )
{
	LoadReport report;
	m_debug.clear();
	std::error_code error;
	std::filesystem::directory_iterator files( directory, error );
	if ( error )
	{
		report.rejected.push_back( std::string( directory ) + ": " + error.message() );
		return report;
	}
	for ( const std::filesystem::directory_entry &entry : files )
	{
		const std::string file = entry.path().filename().string();
		uint64_t hash = 0;
		if ( !entry.is_regular_file( error ) || !ParseDebugFileName( file, &hash ) )
			continue;
		std::ifstream stream( entry.path(), std::ios::binary );
		const std::vector<char> bytes(
		    ( std::istreambuf_iterator<char>( stream ) ), std::istreambuf_iterator<char>() );
		std::string reason = "unreadable, or not a whole number of words";
		std::vector<uint32_t> code( bytes.size() / sizeof( uint32_t ) );
		if ( stream.bad() || bytes.empty() || bytes.size() % sizeof( uint32_t ) != 0 )
		{
			report.rejected.push_back( file + ": " + reason );
			continue;
		}
		std::memcpy( code.data(), bytes.data(), bytes.size() );
		if ( AddDebugVariant( hash, std::move( code ), nonSemanticInfoSupported, &reason ) )
			++report.loaded;
		else
			report.rejected.push_back( file + ": " + reason );
	}
	return report;
}

ShaderModuleCode VulkanShaderLibrary::Resolve( const uint32_t *code, size_t sizeBytes ) const
{
	ShaderModuleCode resolved;
	resolved.code = code;
	resolved.sizeBytes = sizeBytes;
	const uint64_t hash = SpirvHash( code, sizeBytes );
	resolved.name = EmbeddedShaderName( hash );
	const auto debug = m_debug.find( hash );
	if ( debug != m_debug.end() )
	{
		resolved.code = debug->second.data();
		resolved.sizeBytes = debug->second.size() * sizeof( uint32_t );
		resolved.debugVariant = true;
	}
	return resolved;
}

} // namespace render_vulkan
