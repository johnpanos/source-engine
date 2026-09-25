//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The native backend's capture-tool support, without a device:
//          - material_spv_index.h agrees with material_spv.h (every embedded
//            array's C++ hash is the generator's), so modules get their names;
//          - VulkanShaderLibrary takes a debug variant only for a known hash,
//            well-formed SPIR-V and a device that can take NonSemantic info,
//            from files and from a directory scan with stale and malformed
//            files, and resolves embedded code to it;
//          - DebugLabelsWanted's policy table;
//          - VulkanDebugUtils over recording entry points: inert until
//            loaded, names, and frame labels that stay balanced in every
//            command buffer under seeded random push/pop/insert/frame
//            sequences (with a control proving the balance check fails on
//            an unbalanced log).
//
//===========================================================================//

#include "testing/conformance_result.h"

#include "materialsystem/shaderapivulkan/material_spv.h"
#include "materialsystem/shaderapivulkan/material_spv_index.h"
#include "materialsystem/shaderapivulkan/vulkan_debug_utils.h"
#include "materialsystem/shaderapivulkan/vulkan_shader_library.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <random>
#include <string>
#include <vector>

using namespace render_vulkan;

namespace
{

unsigned long g_checks = 0;
unsigned long g_failures = 0;

void Check( bool ok, const std::string &what )
{
	++g_checks;
	if ( !ok )
	{
		++g_failures;
		std::printf( "FAIL: %s\n", what.c_str() );
	}
}

// -- the generated index ------------------------------------------------------

const MaterialSpvIndexEntry *IndexEntry( const char *array )
{
	for ( const MaterialSpvIndexEntry &entry : g_materialSpvIndex )
		if ( std::strcmp( entry.array, array ) == 0 )
			return &entry;
	return nullptr;
}

void CheckIndex()
{
	size_t arrays = 0;
#define CHECK_ARRAY( array )                                                                       \
	{                                                                                              \
		++arrays;                                                                                  \
		const MaterialSpvIndexEntry *entry = IndexEntry( #array );                                 \
		const uint64_t hash = SpirvHash( array, sizeof( array ) );                                 \
		Check( entry && entry->hash == hash, #array ": the index holds its C++ hash" );            \
		Check( EmbeddedShaderName( hash ) != nullptr, #array ": its hash names a shader" );        \
		Check( !SpirvUsesNonSemanticInfo( array, sizeof( array ) / 4 ),                            \
		    #array ": the embedded release code carries no NonSemantic info" );                    \
	}
	MATERIAL_SPV_ARRAYS( CHECK_ARRAY )
#undef CHECK_ARRAY
	Check( arrays == sizeof( g_materialSpvIndex ) / sizeof( g_materialSpvIndex[0] ),
	    "the index lists every embedded array once" );
	Check( std::strcmp( EmbeddedShaderName( SpirvHash( g_modelPbrFragSpv,
	                        sizeof( g_modelPbrFragSpv ) ) ),
	           "model_pbr.frag" ) == 0,
	    "g_modelPbrFragSpv is named by its source" );
	std::vector<uint32_t> changed( g_modelPbrFragSpv,
	    g_modelPbrFragSpv + sizeof( g_modelPbrFragSpv ) / sizeof( uint32_t ) );
	changed.back() ^= 1u;
	Check( EmbeddedShaderName( SpirvHash( changed.data(), changed.size() * 4 ) ) == nullptr,
	    "control: one changed bit leaves code unnamed" );
	// FNV-1a 64 reference values (the generator's spirv_hash).
	Check( SpirvHash( nullptr, 0 ) == 0xcbf29ce484222325ull, "FNV-1a 64 of nothing" );
	const uint32_t word = 0x64636261u; // "abcd"
	Check( SpirvHash( &word, 4 ) == 0xfc179f83ee0724ddull, "FNV-1a 64 of \"abcd\"" );
}

// -- the shader library ---------------------------------------------------------

std::vector<uint32_t> Copy( const uint32_t *code, size_t bytes )
{
	return std::vector<uint32_t>( code, code + bytes / sizeof( uint32_t ) );
}

// A minimal module importing `set` (only its header and one import are read).
std::vector<uint32_t> ModuleImporting( const char *set )
{
	std::vector<uint32_t> code = { 0x07230203u, 0x00010000u, 0u, 16u, 0u };
	const size_t chars = std::strlen( set ) + 1;
	const uint32_t words = uint32_t( ( chars + 3 ) / 4 );
	code.push_back( ( ( 2 + words ) << 16 ) | 11u ); // OpExtInstImport
	code.push_back( 1u );                             // result id
	std::vector<uint32_t> literal( words, 0u );
	std::memcpy( literal.data(), set, chars );
	code.insert( code.end(), literal.begin(), literal.end() );
	return code;
}

void CheckLibrary()
{
	const uint64_t pbr = SpirvHash( g_modelPbrFragSpv, sizeof( g_modelPbrFragSpv ) );
	const uint64_t world = SpirvHash( g_worldPbrFragSpv, sizeof( g_worldPbrFragSpv ) );
	std::string reason;

	VulkanShaderLibrary empty;
	const ShaderModuleCode plain = empty.Resolve( g_modelPbrFragSpv, sizeof( g_modelPbrFragSpv ) );
	Check( plain.code == g_modelPbrFragSpv && plain.sizeBytes == sizeof( g_modelPbrFragSpv ) &&
	           !plain.debugVariant && plain.name && std::strcmp( plain.name, "model_pbr.frag" ) == 0,
	    "without variants the embedded code resolves to itself, named" );

	VulkanShaderLibrary library;
	// The variant here is the world shader's code: Resolve must return what
	// was loaded for the hash, not the embedded words.
	Check( library.AddDebugVariant( pbr, Copy( g_worldPbrFragSpv, sizeof( g_worldPbrFragSpv ) ),
	           false, &reason ),
	    "a well-formed variant for a known hash is taken" );
	const ShaderModuleCode resolved =
	    library.Resolve( g_modelPbrFragSpv, sizeof( g_modelPbrFragSpv ) );
	Check( resolved.debugVariant && resolved.sizeBytes == sizeof( g_worldPbrFragSpv ) &&
	           std::memcmp( resolved.code, g_worldPbrFragSpv, sizeof( g_worldPbrFragSpv ) ) == 0,
	    "Resolve returns the loaded variant for the embedded code's hash" );
	Check( resolved.name && std::strcmp( resolved.name, "model_pbr.frag" ) == 0,
	    "the variant keeps the embedded shader's name" );
	Check( !library.Resolve( g_worldPbrFragSpv, sizeof( g_worldPbrFragSpv ) ).debugVariant,
	    "other embedded code is not substituted" );

	Check( !library.AddDebugVariant( 0x0123456789abcdefull,
	           Copy( g_worldPbrFragSpv, sizeof( g_worldPbrFragSpv ) ), false, &reason ) &&
	           reason.find( "no embedded shader" ) != std::string::npos,
	    "a variant for an unknown (stale) hash is rejected" );
	std::vector<uint32_t> notSpirv = Copy( g_worldPbrFragSpv, sizeof( g_worldPbrFragSpv ) );
	notSpirv[0] = 0xdeadbeefu;
	Check( !library.AddDebugVariant( world, notSpirv, false, &reason ) &&
	           reason == "not SPIR-V",
	    "a variant without the SPIR-V magic is rejected" );
	Check( !library.AddDebugVariant( world, { 0x07230203u, 0u }, false, &reason ),
	    "a truncated variant is rejected" );

	const std::vector<uint32_t> nonSemantic = ModuleImporting( "NonSemantic.Shader.DebugInfo.100" );
	Check( SpirvUsesNonSemanticInfo( nonSemantic.data(), nonSemantic.size() ),
	    "a NonSemantic import is found" );
	Check( !SpirvUsesNonSemanticInfo( ModuleImporting( "GLSL.std.450" ).data(),
	           ModuleImporting( "GLSL.std.450" ).size() ),
	    "GLSL.std.450 is not NonSemantic" );
	Check( !library.AddDebugVariant( world, nonSemantic, false, &reason ) &&
	           reason.find( "VK_KHR_shader_non_semantic_info" ) != std::string::npos,
	    "NonSemantic code is rejected for a device without the extension" );
	Check( library.AddDebugVariant( world, nonSemantic, true, &reason ),
	    "NonSemantic code is taken when the device has the extension" );
	Check( library.DebugVariants() == 2, "two variants loaded" );

	// A directory scan: one good file, one stale, one partial word, one
	// NonSemantic file for a device without the extension, and files the
	// scan ignores.
	const std::filesystem::path dir =
	    std::filesystem::temp_directory_path() / ( "vulkan_debug_tools_" + std::to_string( pbr ) );
	std::filesystem::remove_all( dir );
	std::filesystem::create_directories( dir );
	const auto write = [&]( const std::string &name, const void *data, size_t bytes )
	{
		std::ofstream( dir / name, std::ios::binary )
		    .write( static_cast<const char *>( data ), std::streamsize( bytes ) );
	};
	char hex[32];
	std::snprintf( hex, sizeof( hex ), "%016llx", static_cast<unsigned long long>( pbr ) );
	write( std::string( "g_modelPbrFragSpv." ) + hex + ".spv", g_worldPbrFragSpv,
	    sizeof( g_worldPbrFragSpv ) );
	write( "g_stale.00000000000000aa.spv", g_worldPbrFragSpv, sizeof( g_worldPbrFragSpv ) );
	std::snprintf( hex, sizeof( hex ), "%016llx", static_cast<unsigned long long>( world ) );
	write( std::string( "g_partial." ) + hex + ".spv", g_worldPbrFragSpv, 10 );
	const uint64_t skin = SpirvHash( g_skinFragSpv, sizeof( g_skinFragSpv ) );
	std::snprintf( hex, sizeof( hex ), "%016llx", static_cast<unsigned long long>( skin ) );
	write( std::string( "g_skinFragSpv." ) + hex + ".spv", nonSemantic.data(),
	    nonSemantic.size() * 4 );
	write( "manifest.json", "{}", 2 );
	write( "g_upper.00000000000000AA.spv", g_worldPbrFragSpv, sizeof( g_worldPbrFragSpv ) );
	VulkanShaderLibrary scanned;
	const VulkanShaderLibrary::LoadReport report =
	    scanned.LoadDebugDirectory( dir.string().c_str(), false );
	Check( report.loaded == 1 && scanned.DebugVariants() == 1, "the scan loads the one good file" );
	Check( report.rejected.size() == 3, "the scan reports the stale, partial and NonSemantic files" );
	size_t reasons = 0;
	for ( const std::string &line : report.rejected )
		reasons += ( line.find( "no embedded shader" ) != std::string::npos ) +
		           ( line.find( "whole number of words" ) != std::string::npos ) +
		           ( line.find( "non_semantic" ) != std::string::npos );
	Check( reasons == 3, "each rejection names its reason" );
	Check( scanned.Resolve( g_modelPbrFragSpv, sizeof( g_modelPbrFragSpv ) ).debugVariant,
	    "the scanned variant substitutes its embedded code" );
	Check( scanned.LoadDebugDirectory( dir.string().c_str(), true ).loaded == 2,
	    "a rescan replaces the load; with the extension the NonSemantic file is taken" );
	std::filesystem::remove_all( dir );
	VulkanShaderLibrary missing;
	Check( missing.LoadDebugDirectory( dir.string().c_str(), true ).rejected.size() == 1 &&
	           missing.DebugVariants() == 0,
	    "a missing directory is reported and loads nothing" );
}

// -- labels ---------------------------------------------------------------------

struct Call
{
	std::string op; // name, begin, end, insert
	uint64_t target = 0;
	std::string text;
	float color[4] = {};
};
std::vector<Call> g_calls;

uint64_t Bits( VkCommandBuffer cmd )
{
	return static_cast<uint64_t>( reinterpret_cast<uintptr_t>( cmd ) );
}

VKAPI_ATTR VkResult VKAPI_CALL FakeName( VkDevice, const VkDebugUtilsObjectNameInfoEXT *info )
{
	Call call;
	call.op = "name";
	call.target = info->objectHandle;
	call.text = std::to_string( int( info->objectType ) ) + ":" + info->pObjectName;
	g_calls.push_back( call );
	return VK_SUCCESS;
}
VKAPI_ATTR void VKAPI_CALL FakeBegin( VkCommandBuffer cmd, const VkDebugUtilsLabelEXT *label )
{
	Call call;
	call.op = "begin";
	call.target = Bits( cmd );
	call.text = label->pLabelName;
	std::memcpy( call.color, label->color, sizeof( call.color ) );
	g_calls.push_back( call );
}
VKAPI_ATTR void VKAPI_CALL FakeEnd( VkCommandBuffer cmd )
{
	Call call;
	call.op = "end";
	call.target = Bits( cmd );
	g_calls.push_back( call );
}
VKAPI_ATTR void VKAPI_CALL FakeInsert( VkCommandBuffer cmd, const VkDebugUtilsLabelEXT *label )
{
	Call call;
	call.op = "insert";
	call.target = Bits( cmd );
	call.text = label->pLabelName;
	g_calls.push_back( call );
}

VulkanDebugUtils::EntryPoints Fakes()
{
	VulkanDebugUtils::EntryPoints entry;
	entry.setObjectName = FakeName;
	entry.beginLabel = FakeBegin;
	entry.endLabel = FakeEnd;
	entry.insertLabel = FakeInsert;
	return entry;
}

// Every command buffer's labels close in order and never close unopened ones:
// what the Vulkan spec requires of a submitted command buffer.
bool Balanced( const std::vector<Call> &calls )
{
	std::map<uint64_t, int> depth;
	for ( const Call &call : calls )
	{
		if ( call.op == "begin" )
			++depth[call.target];
		else if ( call.op == "end" && --depth[call.target] < 0 )
			return false;
	}
	for ( const auto &open : depth )
		if ( open.second != 0 )
			return false;
	return true;
}

VkDevice FakeDevice()
{
	return reinterpret_cast<VkDevice>( uintptr_t( 0x1000 ) );
}
VkCommandBuffer FakeCmd( uintptr_t n )
{
	return reinterpret_cast<VkCommandBuffer>( 0x2000 + n * 16 );
}

void CheckPolicy()
{
	Check( !DebugLabelsWanted( DebugLabelPolicy::Auto, false, false ), "Auto: off on a plain run" );
	Check( DebugLabelsWanted( DebugLabelPolicy::Auto, true, false ), "Auto: on under validation" );
	Check( DebugLabelsWanted( DebugLabelPolicy::Auto, false, true ), "Auto: on for a capture tool" );
	Check( DebugLabelsWanted( DebugLabelPolicy::On, false, false ), "On: on without tools" );
	Check( !DebugLabelsWanted( DebugLabelPolicy::Off, true, true ), "Off: off even under tools" );
}

void CheckLabels()
{
	g_calls.clear();
	VulkanDebugUtils debug;
	debug.Name( VK_OBJECT_TYPE_IMAGE, uint64_t( 0x1234 ), "unloaded" );
	debug.BeginFrameCommands( FakeCmd( 1 ) );
	debug.PushLabel( "unloaded" );
	Check( g_calls.empty() && !debug.Active(), "unloaded: every call is inert" );

	VulkanDebugUtils::EntryPoints partial = Fakes();
	partial.insertLabel = nullptr;
	Check( !debug.Load( FakeDevice(), partial ) && !debug.Active(),
	    "a missing entry point leaves it inert" );
	Check( debug.Load( FakeDevice(), Fakes() ) && debug.Active(), "all entry points load" );

	debug.Name( VK_OBJECT_TYPE_IMAGE, uint64_t( 0x1234 ), "RPRB reflection probes" );
	debug.NameF( VK_OBJECT_TYPE_PIPELINE, uint64_t( 0x99 ), "%s / %s", "skin.vert", "skin.frag" );
	debug.Name( VK_OBJECT_TYPE_IMAGE, uint64_t( 0 ), "null handle" );
	Check( g_calls.size() == 2 && g_calls[0].target == 0x1234 &&
	           g_calls[0].text == std::to_string( int( VK_OBJECT_TYPE_IMAGE ) ) +
	                                  ":RPRB reflection probes" &&
	           g_calls[1].text == std::to_string( int( VK_OBJECT_TYPE_PIPELINE ) ) +
	                                  ":skin.vert / skin.frag",
	    "names reach the device with type and handle; a null handle is skipped" );

	g_calls.clear();
	debug.PushLabel( "outside a frame" );
	debug.PopLabel();
	Check( g_calls.empty(), "labels outside a frame's recording are ignored" );

	const float orange[4] = { 1.0f, 0.5f, 0.0f, 1.0f };
	debug.BeginFrameCommands( FakeCmd( 2 ) );
	debug.PushLabel( "CSimpleWorldView::Draw", orange );
	debug.PushLabel( "DrawTransLists" );
	debug.PopLabel();
	debug.InsertLabel( "pass: back buffer" );
	debug.EndFrameCommands();
	Check( g_calls.size() == 5 && g_calls[0].op == "begin" &&
	           g_calls[0].text == "CSimpleWorldView::Draw" && g_calls[0].color[1] == 0.5f &&
	           g_calls[1].op == "begin" && g_calls[2].op == "end" && g_calls[3].op == "insert" &&
	           g_calls[4].op == "end",
	    "a frame's labels nest in order; the open one closes at frame end" );
	Check( debug.ClosedAtFrameEnd() == 1 && debug.OpenLabels() == 0,
	    "the region left open is closed and counted" );
	Check( Balanced( g_calls ), "the frame's command buffer is balanced" );

	g_calls.clear();
	debug.BeginFrameCommands( FakeCmd( 3 ) );
	debug.PopLabel();
	debug.EndFrameCommands();
	Check( g_calls.empty() && debug.UnbalancedPops() == 1,
	    "a pop with nothing open is ignored and counted" );

	g_calls.clear();
	debug.BeginLabel( FakeCmd( 4 ), "upload" );
	debug.EndLabel( FakeCmd( 4 ) );
	Check( g_calls.size() == 2 && g_calls[0].target == Bits( FakeCmd( 4 ) ) && Balanced( g_calls ),
	    "an explicit command buffer's label pair" );

	// Seeded random sequences over several frames stay balanced per buffer.
	std::mt19937 random( 20260925u );
	bool allBalanced = true;
	for ( int sequence = 0; sequence < 200; ++sequence )
	{
		g_calls.clear();
		uintptr_t cmd = 10;
		bool recording = false;
		for ( int step = 0; step < 64; ++step )
		{
			switch ( random() % 6 )
			{
			case 0:
				if ( !recording )
				{
					debug.BeginFrameCommands( FakeCmd( ++cmd ) );
					recording = true;
				}
				break;
			case 1:
				debug.EndFrameCommands();
				recording = false;
				break;
			case 2:
			case 3:
				debug.PushLabel( "event" );
				break;
			case 4:
				debug.PopLabel();
				break;
			default:
				debug.InsertLabel( "marker" );
				break;
			}
		}
		debug.EndFrameCommands();
		allBalanced = allBalanced && Balanced( g_calls );
	}
	Check( allBalanced, "200 seeded push/pop/insert/frame sequences stay balanced" );
	std::vector<Call> unbalanced( 1 );
	unbalanced[0].op = "begin";
	Check( !Balanced( unbalanced ), "control: the balance check rejects an unclosed label" );
	std::vector<Call> underflow( 1 );
	underflow[0].op = "end";
	Check( !Balanced( underflow ), "control: the balance check rejects an unopened end" );

	debug.Reset();
	g_calls.clear();
	debug.Name( VK_OBJECT_TYPE_IMAGE, uint64_t( 1 ), "after reset" );
	Check( g_calls.empty() && !debug.Active(), "Reset makes it inert again" );
}

} // namespace

int main()
{
	CheckIndex();
	CheckLibrary();
	CheckPolicy();
	CheckLabels();
	return testing::ReportConformance( g_checks, g_failures );
}
