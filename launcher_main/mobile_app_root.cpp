//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Helpers shared by the mobile application roots; see mobile_app_root.h.
//
//=============================================================================//

#include "mobile_app_root.h"

#include <SDL3/SDL.h>

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

namespace mobileapp
{

namespace
{

bool MakeDirectories( const char *path )
{
	char partial[PATH_MAX];
	snprintf( partial, sizeof( partial ), "%s", path );
	for ( char *p = partial + 1; *p; ++p )
	{
		if ( *p != '/' )
			continue;
		*p = '\0';
		if ( mkdir( partial, 0775 ) != 0 && errno != EEXIST )
			return false;
		*p = '/';
	}
	return mkdir( partial, 0775 ) == 0 || errno == EEXIST;
}

bool FileHasContents( const char *path, const void *data, size_t size )
{
	FILE *file = fopen( path, "rb" );
	if ( !file )
		return false;
	bool same = true;
	char buffer[16384];
	size_t offset = 0;
	for ( size_t read; ( read = fread( buffer, 1, sizeof( buffer ), file ) ) != 0; offset += read )
	{
		if ( offset + read > size ||
		     memcmp( buffer, static_cast<const char *>( data ) + offset, read ) != 0 )
		{
			same = false;
			break;
		}
	}
	fclose( file );
	return same && offset == size;
}

} // namespace

int AppendArgumentsFile( const char *path, char **argv, int argc, char *storage, size_t size )
{
	FILE *file = fopen( path, "rb" );
	if ( !file )
		return argc;
	const size_t length = fread( storage, 1, size - 1, file );
	fclose( file );
	storage[length] = '\0';
	for ( char *token = strtok( storage, " \t\r\n" ); token && argc < kMaxArgs - 1;
	    token = strtok( NULL, " \t\r\n" ) )
	{
		argv[argc++] = token;
	}
	return argc;
}

void InstallTouchIcons( const char *assetRoot, const char *gameDir )
{
	char manifestPath[PATH_MAX];
	snprintf( manifestPath, sizeof( manifestPath ), "%stouch/touch_icons.txt", assetRoot );
	size_t manifestSize = 0;
	char *manifest = static_cast<char *>( SDL_LoadFile( manifestPath, &manifestSize ) );
	if ( !manifest )
	{
		SDL_Log( "Source: no packaged touch icons: %s", SDL_GetError() );
		return;
	}
	int installed = 0;
	char *save = NULL;
	for ( char *entry = strtok_r( manifest, "\r\n", &save ); entry;
	    entry = strtok_r( NULL, "\r\n", &save ) )
	{
		if ( strstr( entry, ".." ) || entry[0] == '/' )
			continue;
		char asset[PATH_MAX], target[PATH_MAX];
		snprintf( asset, sizeof( asset ), "%stouch/%s", assetRoot, entry );
		snprintf( target, sizeof( target ), "%s/custom/android_touch/%s", gameDir, entry );
		size_t size = 0;
		void *data = SDL_LoadFile( asset, &size );
		if ( !data )
			continue;
		if ( !FileHasContents( target, data, size ) )
		{
			char directory[PATH_MAX];
			snprintf( directory, sizeof( directory ), "%s", target );
			*strrchr( directory, '/' ) = '\0';
			FILE *file = MakeDirectories( directory ) ? fopen( target, "wb" ) : NULL;
			if ( file && fwrite( data, 1, size, file ) == size )
				++installed;
			else
				SDL_Log( "Source: cannot install %s", target );
			if ( file )
				fclose( file );
		}
		SDL_free( data );
	}
	SDL_free( manifest );
	if ( installed )
		SDL_Log(
		    "Source: installed %d touch icon(s) into %s/custom/android_touch", installed, gameDir );
}

} // namespace mobileapp
