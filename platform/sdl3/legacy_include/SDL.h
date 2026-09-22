// Temporary adapter for the remaining legacy SDL2 callers. New providers use
// SDL3 directly. Preserve index-based display selection and SDL2's success=0
// convention at this boundary; SDL3 IDs are never treated as array indices.
#ifndef PLATFORM_SDL3_LEGACY_SDL_H
#define PLATFORM_SDL3_LEGACY_SDL_H
#define SDL_ENABLE_OLD_NAMES
#include <SDL3/SDL.h>

inline SDL_DisplayID LegacySDL_DisplayId( int index )
{
	int count = 0;
	SDL_DisplayID *displays = SDL_GetDisplays( &count );
	SDL_DisplayID id = displays && index >= 0 && index < count ? displays[index] : 0;
	SDL_free( displays );
	return id;
}

inline int SDL_GetNumVideoDisplays()
{
	int count = 0;
	SDL_DisplayID *displays = SDL_GetDisplays( &count );
	SDL_free( displays );
	return count;
}

inline int SDL_GetDisplayBounds( int index, SDL_Rect *bounds )
{
	SDL_DisplayID id = LegacySDL_DisplayId( index );
	return id && SDL_GetDisplayBounds( id, bounds ) ? 0 : -1;
}

inline int SDL_GetDesktopDisplayMode( int index, SDL_DisplayMode *mode )
{
	const SDL_DisplayMode *current = SDL_GetDesktopDisplayMode( LegacySDL_DisplayId( index ) );
	if ( !current || !mode )
		return -1;
	*mode = *current;
	return 0;
}

inline int SDL_GetCurrentDisplayMode( int index, SDL_DisplayMode *mode )
{
	const SDL_DisplayMode *current = SDL_GetCurrentDisplayMode( LegacySDL_DisplayId( index ) );
	if ( !current || !mode )
		return -1;
	*mode = *current;
	return 0;
}

inline Uint32 SDL_GetMouseState( int *x, int *y )
{
	float preciseX = 0.0f, preciseY = 0.0f;
	Uint32 buttons = SDL_GetMouseState( &preciseX, &preciseY );
	if ( x )
		*x = static_cast<int>( preciseX );
	if ( y )
		*y = static_cast<int>( preciseY );
	return buttons;
}

#endif
