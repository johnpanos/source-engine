//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.present-policy.v1 — present-mode selection over every
//          vsync/support combination against the contract's table.
//
//===========================================================================//

#include "render/render_present_policy.h"
#include "testing/conformance_result.h"

#include <cstdio>

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

// The contract table: vsync always waits (FIFO). Without vsync the presenter
// must not wait when it can avoid it, preferring the D3D9 immediate interval.
render::PresentModeKind Expected( bool vsync, bool mailbox, bool immediate )
{
	if ( vsync )
		return render::PresentModeKind::Fifo;
	if ( immediate )
		return render::PresentModeKind::Immediate;
	return mailbox ? render::PresentModeKind::Mailbox : render::PresentModeKind::Fifo;
}

// The selector a suite must reject: mailbox preferred over immediate.
[[maybe_unused]] render::PresentModeKind MailboxFirst(
    bool vsync, const render::PresentModeSupport &supported )
{
	if ( vsync )
		return render::PresentModeKind::Fifo;
	if ( supported.mailbox )
		return render::PresentModeKind::Mailbox;
	return supported.immediate ? render::PresentModeKind::Immediate : render::PresentModeKind::Fifo;
}

template <typename Selector> bool AllCombinationsMatch( Selector select )
{
	for ( int vsync = 0; vsync < 2; ++vsync )
	{
		for ( int mask = 0; mask < 4; ++mask )
		{
			render::PresentModeSupport supported;
			supported.mailbox = ( mask & 1 ) != 0;
			supported.immediate = ( mask & 2 ) != 0;
			if ( select( vsync != 0, supported ) !=
			     Expected( vsync != 0, supported.mailbox, supported.immediate ) )
				return false;
		}
	}
	return true;
}
} // namespace

int main()
{
#ifdef PRESENT_POLICY_SEEDED_MAILBOX_FIRST
	return testing::ReportConformance( 1, AllCombinationsMatch( MailboxFirst ) ? 1 : 0 );
#else
	Check( AllCombinationsMatch(
	           []( bool vsync, const render::PresentModeSupport &supported )
	           {
		           return render::SelectPresentMode( vsync, supported );
	           } ),
	    "selection matches the table for every vsync/support combination" );

	render::PresentModeSupport all;
	all.mailbox = true;
	all.immediate = true;
	Check( render::SelectPresentMode( true, all ) == render::PresentModeKind::Fifo,
	    "vsync waits even when non-waiting modes exist" );
	Check( render::SelectPresentMode( false, all ) == render::PresentModeKind::Immediate,
	    "vsync off prefers immediate, like D3DPRESENT_INTERVAL_IMMEDIATE" );
	Check( render::SelectPresentMode( false, render::PresentModeSupport{} ) ==
	           render::PresentModeKind::Fifo,
	    "a FIFO-only surface (the Vulkan guarantee, e.g. Android) keeps FIFO" );
	static_assert(
	    render::SelectPresentMode( false, render::PresentModeSupport{ true, true, false } ) ==
	        render::PresentModeKind::Mailbox,
	    "selection is usable in constant expressions" );
	return testing::ReportConformance( g_checks, g_failures );
#endif
}
