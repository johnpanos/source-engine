//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The back-buffer sizes a desktop-fullscreen presenter offers as video
//          modes (render.display-modes.v1).
//
//          An SDL3 fullscreen window always covers the desktop, and the
//          presenter scales the back buffer to it, so a "mode" is a back-buffer
//          size, not a display mode switch. The list is derived only from the
//          desktop mode; small-mode filtering and refresh selection stay with
//          the engine's video-mode owner.
//
//===========================================================================//

#ifndef RENDER_DISPLAY_MODES_H
#define RENDER_DISPLAY_MODES_H

#include <algorithm>
#include <vector>

namespace render
{
struct DisplayModeFacts
{
	int width = 0;
	int height = 0;
	int refreshNumerator = 0;
	int refreshDenominator = 1;
};

// Standard 4:3, 16:9 and 16:10 window sizes: the table togl offers
// (appframework/glmdisplaydb_linuxwin.inl, GLMDisplayInfo::PopulateModes)
// plus 2560x1440 and 3840x2160.
struct StandardModeSize
{
	int width;
	int height;
};
inline constexpr StandardModeSize kStandardModeSizes[] = {
	{ 640, 480 }, { 800, 600 }, { 1024, 768 }, { 1152, 864 }, { 1280, 960 },
	{ 1600, 1200 }, { 1920, 1440 }, { 2048, 1536 },
	{ 1280, 720 }, { 1366, 768 }, { 1600, 900 }, { 1920, 1080 }, { 2560, 1440 },
	{ 3840, 2160 },
	{ 720, 480 }, { 1280, 800 }, { 1680, 1050 }, { 1920, 1200 }, { 2560, 1600 },
};

// Every size is at most the desktop in both dimensions and carries the desktop
// refresh rate (the presenter never changes it). The result holds the desktop
// size, each standard size that fits, the desktop-aspect size of the same width
// and half the desktop, sorted by width then height without duplicates. A
// desktop without a positive extent yields no modes.
[[nodiscard]] inline std::vector<DisplayModeFacts> BuildBackBufferModeList(
    const DisplayModeFacts &desktop )
{
	std::vector<DisplayModeFacts> modes;
	if ( desktop.width <= 0 || desktop.height <= 0 )
		return modes;

	const auto add = [&]( int width, int height ) {
		if ( width <= 0 || height <= 0 || width > desktop.width || height > desktop.height )
			return;
		DisplayModeFacts mode = desktop;
		mode.width = width;
		mode.height = height;
		modes.push_back( mode );
	};

	add( desktop.width, desktop.height );
	for ( const StandardModeSize &size : kStandardModeSizes )
	{
		if ( size.width > desktop.width || size.height > desktop.height )
			continue;
		add( size.width, size.height );
		const long long aspectHeight =
		    static_cast<long long>( size.width ) * desktop.height / desktop.width;
		add( size.width, static_cast<int>( aspectHeight ) );
	}
	add( desktop.width / 2, desktop.height / 2 );

	std::sort( modes.begin(), modes.end(), []( const DisplayModeFacts &a, const DisplayModeFacts &b ) {
		return a.width != b.width ? a.width < b.width : a.height < b.height;
	} );
	modes.erase( std::unique( modes.begin(), modes.end(),
	                 []( const DisplayModeFacts &a, const DisplayModeFacts &b ) {
		                 return a.width == b.width && a.height == b.height;
	                 } ),
	    modes.end() );
	return modes;
}
} // namespace render

#endif // RENDER_DISPLAY_MODES_H
