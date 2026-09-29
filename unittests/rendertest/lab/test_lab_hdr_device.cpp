//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.lab.hdr (RFC 0016 K11, render.output.v1 on a display):
//			render_lab's presenting host shows its HDR chart on an iPhone or
//			Apple TV through the core's output pass and the SDL3-Vulkan
//			presentation bridge (MoltenVK), and the presented swapchain image
//			is judged against the output oracle (output_oracle.h) at the
//			headroom the frame used.
//
//			H1 the host composes: the extended-linear range is granted, and the
//			   Metal layer shows high range in extended linear sRGB;
//			H2 the display's headroom rises above 1 while the chart presents;
//			   on tvOS the window asks for the HDR mode (HDR10 display
//			   criteria) and, with the user's "Match Dynamic Range" on, the
//			   TV switches and the declared HDR10 headroom applies (tvOS
//			   reports none for a television);
//			H3 every chart patch in the presented image equals the oracle's
//			   tone map of its value for that headroom, and values above white
//			   reach the display above 1;
//			H4 a debug view (toneMap false) reaches the display untouched: the
//			   16x white patch presents as 16;
//			H5 the control: a standard presentation (8-bit) at the legacy point
//			   (scene peak 1) shows the clip and sRGB encoding, and on tvOS it
//			   withdraws the HDR request.
//
//			RENDER_LAB_SHOW_SECONDS=n keeps the chart on screen for n seconds
//			after the checks, alternating the tone-mapped chart and its debug
//			view every four seconds, for a person to look at.
//
//			Runs through tools/quality/ios_conformance.py --app render_lab.
//
//=============================================================================//

#include "../../../render/lab/app/lab_app.h"
#include "../core/pass/output/output_oracle.h"
#include "testing/conformance_result.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace render;
using namespace render::lab::app;
namespace oracle = rendertest::output;

namespace
{

unsigned long g_Checks = 0;
unsigned long g_Failures = 0;

void Check( bool ok, const char *id, const std::string &detail )
{
	++g_Checks;
	if ( !ok )
		++g_Failures;
	std::printf( "%s %s: %s\n", ok ? "ok  " : "FAIL", id, detail.c_str() );
}

std::string Format( const char *format, double a, double b = 0, double c = 0, double d = 0 )
{
	char text[256];
	std::snprintf( text, sizeof( text ), format, a, b, c, d );
	return text;
}

// Presents frames for up to `seconds`, returning the highest current headroom
// seen; stops early once `enough` is reached. Prints each change of the
// headroom and of tvOS's mode switch, with its time.
float PresentFor( LabApp &app, double seconds, float enough, std::string *failure )
{
	float best = 1.0f;
	const std::uint64_t start = SDL_GetTicks();
	float lastCurrent = -1.0f, lastPotential = -1.0f;
	int lastSwitching = -1;
	while ( SDL_GetTicks() - start < std::uint64_t( seconds * 1000.0 ) )
	{
		float used = 1.0f;
		std::string error;
		if ( !app.Frame( LabFrame(), &used, &error ) )
		{
			*failure = error;
			break;
		}
		const RenderDynamicRangeState state = app.DynamicRange();
		const LabDisplayMode mode = app.ReadDisplayMode();
		if ( state.currentHeadroom != lastCurrent || state.potentialHeadroom != lastPotential ||
		     int( mode.switching ) != lastSwitching )
		{
			std::printf( "render.lab.hdr: t=%.2fs headroom %.2f of %.2f, switching %d\n",
			    ( SDL_GetTicks() - start ) / 1000.0, state.currentHeadroom, state.potentialHeadroom,
			    int( mode.switching ) );
			lastCurrent = state.currentHeadroom;
			lastPotential = state.potentialHeadroom;
			lastSwitching = int( mode.switching );
		}
		best = std::max( best, state.currentHeadroom );
		if ( best >= enough )
			break;
	}
	return best;
}

// Captures one frame and compares every patch with the oracle.
struct PatchResult
{
	bool captured = false;
	bool matches = true;
	float headroom = 1.0f;
	float brightest = 0.0f;
	std::string worst;
	LabCapture capture;
};

PatchResult JudgeExtended( LabApp &app, const LabFrame &frame )
{
	PatchResult result;
	LabFrame capture = frame;
	capture.capture = true;
	std::string error;
	if ( !app.Frame( capture, &result.headroom, &error ) || !app.ReadCapture( result.capture ) ||
	     result.capture.linear.empty() )
	{
		result.worst = error.empty() ? "no linear capture" : error;
		result.matches = false;
		return result;
	}
	result.captured = true;
	double worstError = 0.0;
	for ( const ChartPatch &patch : ChartPatches() )
	{
		const oracle::Rgb in = { patch.value[0], patch.value[1], patch.value[2] };
		const oracle::Rgb expected =
		    frame.toneMap ? oracle::ToneMap( in, frame.exposure, frame.scenePeak, result.headroom )
		                  : oracle::Rgb{ std::max( in[0], 0.0 ), std::max( in[1], 0.0 ),
		                        std::max( in[2], 0.0 ) };
		const std::array<float, 3> actual = result.capture.At( patch.u, patch.v );
		for ( int c = 0; c < 3; ++c )
		{
			result.brightest = std::max( result.brightest, actual[c] );
			// Half-float storage twice (scene, swapchain) and float math.
			const double tolerance = 4e-3 * std::max( 1.0, expected[c] );
			const double e = std::fabs( actual[c] - expected[c] );
			if ( e > tolerance )
				result.matches = false;
			if ( e > worstError )
			{
				worstError = e;
				result.worst = std::string( patch.name ) +
				               Format( " channel %.0f: presented %.4f, oracle %.4f", c, actual[c],
				                   expected[c] );
			}
		}
	}
	if ( result.worst.empty() )
		result.worst = "every patch exact";
	return result;
}

void RunExtended( std::string *failure )
{
	LabAppOptions options;
	std::string error;
	std::unique_ptr<LabApp> app = LabApp::Create( options, &error );
	Check( app != nullptr, "H1.the-host-composes",
	    app ? Format( "scene %.0fx%.0f", app->SceneWidth(), app->SceneHeight() ) : error );
	if ( !app )
		return;
	Check( app->Range() == RenderDynamicRange::kExtendedLinear, "H1.the-extended-range-is-granted",
	    app->Range() == RenderDynamicRange::kExtendedLinear
	        ? "an extended-linear kRGBA16Float presentation"
	        : "the surface refused the extended range; a standard presentation was made" );
	// H2: EDR ramps up over a few seconds; a tvOS mode switch takes longer.
	float headroom = PresentFor( *app, 6.0, 1e9f, failure );
	// The layer's range is set when the first frame builds the swapchain.
	bool extended = false, linear = false;
	const bool known = app->ReadLayer( &extended, &linear );
	Check( known && extended && linear, "H1.the-layer-shows-high-range-in-extended-linear-srgb",
	    Format( "known %.0f, high %.0f, extended linear %.0f", known, extended, linear ) );
	Check( headroom > 1.0f, "H2.the-headroom-rises-above-one",
	    Format( "highest current headroom %.2f (potential %.2f)", headroom,
	        app->DynamicRange().potentialHeadroom ) );
	// tvOS: the window's display mode request (known only there).
	const LabDisplayMode mode = app->ReadDisplayMode();
	std::printf( "render.lab.hdr: display mode request known %d; the display's HDR modes %u "
	             "(hlg 1, hdr10 2, dolby vision 4), eligible for HDR playback %d\n",
	    int( mode.known ), mode.hdrModes, int( mode.eligibleForHdr ) );
	if ( mode.known )
	{
		Check( mode.known && mode.askedForHdr, "H2.tvos.the-window-asks-for-the-hdr-mode",
		    Format( "known %.0f, asks for HDR %.0f, switching %.0f", mode.known, mode.askedForHdr,
		        mode.switching ) );
		Check( mode.matchingEnabled, "H2.tvos.match-dynamic-range-is-on",
		    mode.matchingEnabled ? "tvOS matches the display's range to content"
		                         : "Settings > Video and Audio > Match Content > Match Dynamic "
		                           "Range is off, so tvOS will not switch the TV into HDR" );
		headroom = std::max( headroom, PresentFor( *app, 20.0, 4.9f, failure ) );
		const LabDisplayMode after = app->ReadDisplayMode();
		// tvOS reports no headroom for an HDR television; once it is in HDR10
		// the bridge reports the declared HDR10 headroom (1000/203).
		Check( headroom >= 4.9f && !after.switching, "H2.tvos.the-tv-switches-into-hdr10",
		    Format( "headroom %.2f after the request (the declared HDR10 headroom 4.93 once tvOS "
		            "has switched; 1.2 in SDR); switching now %.0f",
		        headroom, after.switching ) );
	}

	// H3: the tone-mapped chart.
	const PatchResult mapped = JudgeExtended( *app, LabFrame() );
	Check( mapped.captured && mapped.matches, "H3.the-presented-chart-matches-the-output-oracle",
	    Format( "headroom %.2f; ", mapped.headroom ) + mapped.worst );
	Check( mapped.captured && ( mapped.headroom <= 1.01f || mapped.brightest > 1.01f ),
	    "H3.values-above-white-reach-the-display",
	    Format( "brightest presented %.3f at headroom %.2f", mapped.brightest, mapped.headroom ) );

	// H4: a debug view.
	LabFrame debug;
	debug.toneMap = false;
	const PatchResult view = JudgeExtended( *app, debug );
	const std::array<float, 3> sixteen = view.capture.At( 15.0f / 16.0f, 0.375f );
	Check( view.captured && view.matches && std::fabs( sixteen[0] - 16.0f ) < 0.05f,
	    "H4.a-debug-view-reaches-the-display-untouched",
	    Format( "the 16x patch presents %.3f; ", sixteen[0] ) + view.worst );
}

void RunStandard()
{
	LabAppOptions options;
	options.extended = false;
	std::string error;
	std::unique_ptr<LabApp> app = LabApp::Create( options, &error );
	if ( !app )
	{
		Check( false, "H5.the-standard-control-composes", error );
		return;
	}
	LabFrame legacy;
	legacy.scenePeak = 1.0f;
	legacy.capture = true;
	float used = 0.0f;
	LabCapture capture;
	const bool captured = PresentFor( *app, 0.5, 1e9f, &error ) >= 1.0f &&
	                      app->Frame( legacy, &used, &error ) && app->ReadCapture( capture ) &&
	                      !capture.rgba8.empty();
	bool matches = captured;
	std::string worst = captured ? "every patch within one level" : error;
	for ( const ChartPatch &patch : ChartPatches() )
	{
		if ( !captured )
			break;
		const std::array<float, 3> actual = capture.At( patch.u, patch.v );
		for ( int c = 0; c < 3; ++c )
		{
			const double expected =
			    255.0 * oracle::SrgbEncode( std::clamp( double( patch.value[c] ), 0.0, 1.0 ) );
			if ( std::fabs( actual[c] - expected ) > 1.0 )
			{
				matches = false;
				worst = std::string( patch.name ) +
				        Format( ": presented %.0f, legacy %.1f", actual[c], expected );
			}
		}
	}
	Check( app->Range() == RenderDynamicRange::kStandard && used == 1.0f && matches,
	    "H5.a-standard-presentation-shows-the-legacy-clip-and-srgb", worst );
	const LabDisplayMode mode = app->ReadDisplayMode();
	if ( mode.known )
	{
		Check( mode.known && !mode.askedForHdr,
		    "H5.tvos.the-standard-range-withdraws-the-hdr-request",
		    Format( "asks for HDR %.0f", mode.askedForHdr ) );
	}
}

void Show( double seconds )
{
	LabAppOptions options;
	std::string error;
	std::unique_ptr<LabApp> app = LabApp::Create( options, &error );
	if ( !app )
	{
		std::printf( "render.lab.hdr: show: %s\n", error.c_str() );
		return;
	}
	std::printf( "render.lab.hdr: showing the chart for %.0f s (%s)\n", seconds,
	    app->Range() == RenderDynamicRange::kExtendedLinear ? "extended linear" : "standard" );
	const std::uint64_t start = SDL_GetTicks();
	std::uint64_t lastReport = 0;
	while ( app->Pump() && SDL_GetTicks() - start < std::uint64_t( seconds * 1000.0 ) )
	{
		LabFrame frame;
		frame.toneMap = ( ( SDL_GetTicks() - start ) / 4000 ) % 2 == 0;
		float used = 1.0f;
		if ( !app->Frame( frame, &used, &error ) )
		{
			std::printf( "render.lab.hdr: show: %s\n", error.c_str() );
			break;
		}
		if ( SDL_GetTicks() - lastReport > 2000 )
		{
			lastReport = SDL_GetTicks();
			const RenderDynamicRangeState state = app->DynamicRange();
			std::printf( "render.lab.hdr: headroom %.2f of %.2f, %s\n", state.currentHeadroom,
			    state.potentialHeadroom, frame.toneMap ? "tone mapped" : "debug view" );
		}
	}
}

} // namespace

int main()
{
	std::setvbuf( stdout, nullptr, _IOLBF, 0 );
	std::string failure;
	RunExtended( &failure );
	if ( !failure.empty() )
		Check( false, "H.frames-present", failure );
	RunStandard();
	if ( const char *show = std::getenv( "RENDER_LAB_SHOW_SECONDS" ) )
		Show( std::atof( show ) );
	SDL_Quit();
	std::printf( "render.lab.hdr: %lu check(s), %lu failure(s)\n", g_Checks, g_Failures );
	return testing::ReportConformance( g_Checks, g_Failures );
}
