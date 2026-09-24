//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.display-modes.v1 — the back-buffer sizes a desktop-fullscreen
//          presenter offers, checked by an independent list oracle.
//
//===========================================================================//

#include "render/render_display_modes.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <vector>

namespace
{
unsigned long g_checks = 0;
unsigned long g_failures = 0;

[[maybe_unused]] void Check( bool condition, const char *description )
{
	++g_checks;
	if ( !condition )
	{
		++g_failures;
		std::fprintf( stderr, "FAIL: %s\n", description );
	}
}

render::DisplayModeFacts Desktop( int width, int height, int refresh = 60 )
{
	render::DisplayModeFacts mode;
	mode.width = width;
	mode.height = height;
	mode.refreshNumerator = refresh;
	mode.refreshDenominator = 1;
	return mode;
}

bool Contains( const std::vector<render::DisplayModeFacts> &modes, int width, int height )
{
	for ( const render::DisplayModeFacts &mode : modes )
	{
		if ( mode.width == width && mode.height == height )
			return true;
	}
	return false;
}

// The contract's invariants, independent of how the list is built: nonempty,
// holds the desktop, every size positive and within the desktop, the desktop
// refresh everywhere, strictly increasing by (width, height).
bool ListIsValid(
    const render::DisplayModeFacts &desktop, const std::vector<render::DisplayModeFacts> &modes )
{
	if ( modes.empty() || !Contains( modes, desktop.width, desktop.height ) )
		return false;
	for ( size_t i = 0; i < modes.size(); ++i )
	{
		const render::DisplayModeFacts &mode = modes[i];
		if ( mode.width <= 0 || mode.height <= 0 || mode.width > desktop.width ||
		     mode.height > desktop.height )
			return false;
		if ( mode.refreshNumerator != desktop.refreshNumerator ||
		     mode.refreshDenominator != desktop.refreshDenominator )
			return false;
		if ( i > 0 )
		{
			const render::DisplayModeFacts &prev = modes[i - 1];
			const bool increasing = prev.width < mode.width ||
			                        ( prev.width == mode.width && prev.height < mode.height );
			if ( !increasing )
				return false;
		}
	}
	return true;
}
} // namespace

int main()
{
	using render::BuildBackBufferModeList;

	const render::DisplayModeFacts fullHd = Desktop( 1920, 1080, 144 );
	std::vector<render::DisplayModeFacts> modes = BuildBackBufferModeList( fullHd );
#ifdef DISPLAY_MODES_SEEDED_KEEP_OVERSIZE
	// Seeded defect: a size larger than the desktop (the hardcoded 2560x1440 an
	// enumerator might keep) must be rejected by the oracle.
	modes.push_back( Desktop( 2560, 1440, 144 ) );
	return testing::ReportConformance( 1, ListIsValid( fullHd, modes ) ? 1 : 0 );
#else
	Check( ListIsValid( fullHd, modes ), "1080p desktop list satisfies the contract invariants" );
	Check( Contains( modes, 1280, 720 ) && Contains( modes, 1600, 900 ) &&
	           Contains( modes, 1024, 768 ) && Contains( modes, 1680, 1050 ),
	    "1080p desktop offers the standard sizes that fit" );
	Check( !Contains( modes, 1920, 1200 ) && !Contains( modes, 2560, 1440 ),
	    "sizes taller or wider than the desktop are not offered" );
	Check( Contains( modes, 960, 540 ), "half the desktop is offered" );
	Check(
	    Contains( modes, 1024, 576 ), "the desktop-aspect size of each standard width is offered" );

	const render::DisplayModeFacts ultrawide = Desktop( 3440, 1440, 100 );
	modes = BuildBackBufferModeList( ultrawide );
	Check( ListIsValid( ultrawide, modes ),
	    "ultrawide desktop list satisfies the contract invariants" );
	Check( Contains( modes, 2560, 1440 ) && Contains( modes, 1280, 535 ),
	    "ultrawide desktop offers 16:9 and desktop-aspect sizes" );

	const render::DisplayModeFacts portrait = Desktop( 1080, 2400, 120 );
	modes = BuildBackBufferModeList( portrait );
	Check(
	    ListIsValid( portrait, modes ), "portrait (phone) desktop list satisfies the invariants" );
	Check( Contains( modes, 540, 1200 ) && Contains( modes, 640, 480 ),
	    "portrait desktop offers half size and the fitting landscape sizes" );

	const render::DisplayModeFacts small = Desktop( 800, 600 );
	modes = BuildBackBufferModeList( small );
	Check(
	    ListIsValid( small, modes ) && Contains( modes, 640, 480 ) && Contains( modes, 400, 300 ),
	    "an 800x600 desktop offers 640x480 and its half size" );

	Check( BuildBackBufferModeList( Desktop( 0, 0 ) ).empty() &&
	           BuildBackBufferModeList( Desktop( 1920, 0 ) ).empty(),
	    "a desktop without a positive extent yields no modes (no invented size)" );

	const render::DisplayModeFacts odd = Desktop( 1366, 768, 60 );
	modes = BuildBackBufferModeList( odd );
	Check( ListIsValid( odd, modes ) && Contains( modes, 683, 384 ),
	    "an odd desktop size keeps its own and its truncated half size" );
	return testing::ReportConformance( g_checks, g_failures );
#endif
}
