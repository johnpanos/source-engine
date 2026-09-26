//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Device conformance suite for the Game Center achievement platform
//			(platform/apple) under QueuedAchievementService, against the
//			device's real Game Center:
//			  - the authentication handler resolves, and the sign-in state it
//			    reports is GKLocalPlayer's;
//			  - Submit completes exactly once, and never accepts while no
//			    player is signed in;
//			  - signed out, the service waits: no achievements screen, no
//			    completion banner claimed;
//			  - the bridge claims Game Center's completion banner.
//
//			The conformance app has no Game Center entitlement or App Store
//			Connect record, so the device exercises the signed-out path; a
//			signed-in run needs an entitled build (the product app).
//
//			Apple devices only (profile apple-uikit-device), through
//			tools/quality/ios_conformance.py.
//
//=============================================================================//

#import <Foundation/Foundation.h>
#import <GameKit/GameKit.h>

#include "../../../platform/apple/game_center_achievements.h"
#include "testing/conformance_result.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <functional>

namespace
{

int g_checks = 0;
int g_failures = 0;

void Check( bool ok, const char *what )
{
	++g_checks;
	if ( !ok )
	{
		++g_failures;
		std::printf( "FAIL %s\n", what );
	}
}

#define GC_CHECK( cond ) Check( ( cond ), #cond )

// Runs the main run loop (GameKit calls back on the main queue) until `done`
// or the timeout.
bool PumpUntil( const std::function<bool()> &done, double seconds )
{
	const auto deadline = std::chrono::steady_clock::now() +
	                      std::chrono::duration<double>( seconds );
	while ( !done() )
	{
		if ( std::chrono::steady_clock::now() > deadline )
			return false;
		CFRunLoopRunInMode( kCFRunLoopDefaultMode, 0.05, true );
	}
	return true;
}

} // namespace

int main()
{
	@autoreleasepool
	{
		platform::GameCenterAchievements gameCenter;
		platform::QueuedAchievementService service( gameCenter );

		GC_CHECK( gameCenter.AnnouncesCompletions() );

		// Authentication resolves with GKLocalPlayer's state.
		std::atomic<int> signInCalls{ 0 };
		std::atomic<bool> lastSignedIn{ false };
		gameCenter.Authenticate( [&]( bool signedIn ) {
			lastSignedIn = signedIn;
			++signInCalls;
			service.SetSignedIn( signedIn );
		} );
		const bool resolved = PumpUntil( [&] { return signInCalls.load() > 0; }, 30.0 );
		GC_CHECK( resolved );
		const bool authenticated = GKLocalPlayer.localPlayer.isAuthenticated;
		std::printf( "game center: %s after %d callback(s)\n",
		    authenticated ? "signed in" : "signed out", signInCalls.load() );
		if ( resolved )
			GC_CHECK( lastSignedIn.load() == authenticated );

		// Submit completes exactly once; never accepted while signed out.
		std::atomic<int> completions{ 0 };
		std::atomic<bool> accepted{ false };
		gameCenter.Submit( { "CONFORMANCE_PROBE" }, [&]( bool ok ) {
			accepted = ok;
			++completions;
		} );
		GC_CHECK( PumpUntil( [&] { return completions.load() > 0; }, 30.0 ) );
		PumpUntil( [] { return false; }, 1.0 );
		GC_CHECK( completions.load() == 1 );
		if ( !authenticated )
			GC_CHECK( !accepted.load() );

		// The service follows the sign-in state.
		GC_CHECK( service.ReportCompleted( "CONFORMANCE_PROBE" ) );
		GC_CHECK( !service.ReportCompleted( "not valid" ) );
		if ( !authenticated )
		{
			GC_CHECK( !gameCenter.Present() );
			GC_CHECK( !service.ShowAchievements() );
			GC_CHECK( !service.AnnouncesCompletions() );
		}
		else
		{
			GC_CHECK( service.AnnouncesCompletions() );
		}

		// Detach the handler so nothing calls into this frame after it returns.
		GKLocalPlayer.localPlayer.authenticateHandler = nil;
	}
	return testing::ReportConformance( g_checks, g_failures );
}
