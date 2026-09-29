//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Textures the engine samples on the CPU (sample_textures.h).
//
//===========================================================================//

#include "render_pch.h"
#include "sample_textures.h"

#include "filesystem_engine.h"

#include <map>
#include <mutex>
#include <string>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int g_nMapLoadCount;

namespace
{
constexpr int kMaxDimension = 256;

struct Cache
{
	int map = -1;
	std::mutex mutex;
	std::map<std::string, vtf_sample::Texture> textures;
};

Cache &C()
{
	static Cache cache;
	return cache;
}
} // namespace

const vtf_sample::Texture *EngineSampleTexture( const char *pName )
{
	if ( !pName || !pName[0] )
		return NULL;
	Cache &c = C();
	std::lock_guard<std::mutex> lock( c.mutex );
	if ( c.map != g_nMapLoadCount )
	{
		c.map = g_nMapLoadCount;
		c.textures.clear();
	}
	auto found = c.textures.find( pName );
	if ( found == c.textures.end() )
	{
		vtf_sample::Texture &texture = c.textures[pName];
		char path[MAX_PATH];
		Q_snprintf( path, sizeof( path ), "materials/%s.vtf", pName );
		CUtlBuffer buf;
		if ( g_pFileSystem->ReadFile( path, "GAME", buf ) )
			vtf_sample::Decode( buf, kMaxDimension, texture );
		found = c.textures.find( pName );
	}
	return found->second.valid ? &found->second : NULL;
}
