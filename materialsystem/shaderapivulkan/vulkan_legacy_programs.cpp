//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The legacy shader ports the native Vulkan backend draws
//          (vulkan_legacy_programs.h). One entry per D3D9 shader pair; the
//          GLSL is in shaders/legacy/, and each pixel stage registers the pairs
//          it ports with its @legacy lines (shaders/regen_legacy_spv.py
//          compiles both into legacy_spv.h).
//
//===========================================================================//

#include "vulkan_legacy_programs.h"
#include "spv/legacy_spv.h"

#include <cctype>

namespace render_vulkan
{
namespace
{

bool SameName( const char *a, const char *b )
{
	for ( ; *a && *b; ++a, ++b )
	{
		if ( std::tolower( static_cast<unsigned char>( *a ) ) !=
		     std::tolower( static_cast<unsigned char>( *b ) ) )
			return false;
	}
	return *a == *b;
}

// Generated from each port's @legacy lines (shaders/regen_legacy_spv.py).
const LegacyProgram kPrograms[] = { LEGACY_PROGRAM_TABLE };

} // namespace

int FindLegacyProgram( const char *pixelShader, const char *vertexShader )
{
	if ( !pixelShader || !vertexShader )
		return -1;
	for ( int i = 0; i < LegacyProgramCount(); ++i )
	{
		if ( SameName( kPrograms[i].pixelShader, pixelShader ) &&
		     SameName( kPrograms[i].vertexShader, vertexShader ) )
			return i;
	}
	return -1;
}

int LegacyProgramCount()
{
	return static_cast<int>( sizeof( kPrograms ) / sizeof( kPrograms[0] ) );
}

const LegacyProgram &GetLegacyProgram( int id )
{
	return kPrograms[id];
}

bool LegacyProgramReadsSampler( const LegacyProgram &program, int sampler )
{
	for ( const LegacySamplerSlot &slot : program.samplers )
	{
		if ( slot.sampler == sampler )
			return true;
	}
	return false;
}

} // namespace render_vulkan
