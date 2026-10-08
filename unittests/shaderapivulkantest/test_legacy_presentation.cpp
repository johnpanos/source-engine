//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render_vulkan::ILegacyPresentation, the SDL3-Vulkan bridge's
//			presentation state for the legacy backend's window (RFC 0016
//			legacy device facade F3): the mode request and the gamma ramp
//			with their revisions, the mode-change callbacks dispatched once
//			per change, a window the bridge refuses, and the ramp set from
//			another thread.
//
//=============================================================================//

#include "../../render/bridge/sdl3-vulkan/legacy_presentation.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <thread>

namespace
{
unsigned long g_Checks = 0;
unsigned long g_Failures = 0;

void Check( bool passed, const char *what, int line )
{
	++g_Checks;
	if ( !passed )
	{
		++g_Failures;
		std::printf( "FAIL legacy presentation line %d: %s\n", line, what );
	}
}

#define CHECK( condition ) Check( ( condition ), #condition, __LINE__ )

int g_First = 0;
int g_Second = 0;
void First()
{
	++g_First;
}
void Second()
{
	++g_Second;
}

void CheckMode( render_vulkan::ILegacyPresentation &presentation )
{
	CHECK( presentation.ModeRevision() == 0 );
	render_vulkan::ILegacyPresentation::Mode mode;
	mode.width = 1280;
	mode.height = 720;
	mode.vsync = false;
	mode.samples = 4;
	presentation.RequestMode( mode );
	CHECK( presentation.ModeRevision() == 1 );
	const render_vulkan::ILegacyPresentation::Mode got = presentation.GetMode();
	CHECK( got.width == 1280 && got.height == 720 && !got.vsync && got.samples == 4 );
	presentation.RequestMode( mode ); // every request is a revision
	CHECK( presentation.ModeRevision() == 2 );
}

void CheckGamma( render_vulkan::ILegacyPresentation &presentation )
{
	render::GammaRamp16 ramp = {};
	std::uint64_t revision = 0;
	CHECK( !presentation.GetGammaRamp( &ramp, &revision ) ); // none set yet
	render::GammaRamp16 set;
	render::GammaRampParams params;
	params.gamma = 2.0f;
	render::BuildGammaRamp16( params, set );
	presentation.SetGammaRamp( set );
	CHECK( presentation.GetGammaRamp( &ramp, &revision ) && revision == 1 && ramp == set );
	// Another thread sets it (mat_monitorgamma); the present reads it.
	std::thread writer(
	    [&]
	    {
		    for ( int i = 0; i < 1000; ++i )
			    presentation.SetGammaRamp( set );
	    } );
	for ( int i = 0; i < 1000; ++i )
		presentation.GetGammaRamp( &ramp, &revision );
	writer.join();
	CHECK( presentation.GetGammaRamp( &ramp, &revision ) && revision == 1001 && ramp == set );
}

void CheckCallbacks( render_vulkan::ILegacyPresentation &presentation )
{
	presentation.AddModeChangeCallback( First );
	presentation.AddModeChangeCallback( First ); // registered once
	presentation.AddModeChangeCallback( Second );
	CHECK( !presentation.DispatchModeChange() ); // no change pending
	CHECK( g_First == 0 && g_Second == 0 );
	presentation.ModeChanged();
	presentation.ModeChanged(); // one pending change
	CHECK( presentation.DispatchModeChange() );
	CHECK( g_First == 1 && g_Second == 1 );
	CHECK( !presentation.DispatchModeChange() );
	presentation.RemoveModeChangeCallback( First );
	presentation.ModeChanged();
	CHECK( presentation.DispatchModeChange() );
	CHECK( g_First == 1 && g_Second == 2 );
}

void CheckWindow( render_vulkan::ILegacyPresentation &presentation )
{
	char error[256] = { 'x', '\0' };
	CHECK( presentation.Open( nullptr, error, sizeof( error ) ) == nullptr ); // no window
	CHECK( error[0] != '\0' && error[0] != 'x' );
	CHECK( presentation.SurfaceHost() == nullptr );
	presentation.Close(); // closing nothing is fine
	CHECK( presentation.SurfaceHost() == nullptr );
}
} // namespace

int main()
{
	render_vulkan::ILegacyPresentation *presentation =
	    render_vulkan::CreateSdl3LegacyPresentation();
	CHECK( presentation != nullptr );
	if ( presentation )
	{
		CheckMode( *presentation );
		CheckGamma( *presentation );
		CheckCallbacks( *presentation );
		CheckWindow( *presentation );
		render_vulkan::DestroySdl3LegacyPresentation( presentation );
	}
	return testing::ReportConformance( g_Checks, g_Failures );
}
