//========= Copyright Valve Corporation, All rights reserved. ============//
#include "gameui/graphics_settings_service.h"
#include "testing/conformance_result.h"
#include <cstdio>
#include <limits>

using namespace gameui;
class Backend final : public IGraphicsSettingsBackend
{
public:
	bool reject = false;
	bool rejectSave = false;
	int applies = 0;
	int saves = 0;
	GraphicsSettings current;
	bool Apply( const GraphicsSettings &, const GraphicsSettings &to ) override
	{
		++applies;
		if ( reject )
			return false;
		current = to;
		return true;
	}
	bool Save() override { ++saves; return !rejectSave; }
};

int main()
{
	int checks = 0, failures = 0;
	const auto check = [&]( bool passed, const char *name )
	{
		++checks;
		if ( !passed ) { ++failures; std::fprintf( stderr, "HDR menu: %s\n", name ); }
	};
	GraphicsSettings initial;
	initial.width = 1280;
	initial.height = 720;
	initial.windowed = true;
	GraphicsSettingsService video;
	video.Begin( initial );
	Backend backend;
	HdrSettingsMenu menu( video );
	using Action = HdrSettingsMenu::Action;
	check( initial.hdr.exposure == 0.0f && initial.hdr.peakNits == 0,
	    "defaults come from the display" );
	check( menu.Command( "HdrMode0" ) == Action::Changed, "select SDR" );
	check( video.GetState() == GraphicsSettingsService::State::Editing, "Video becomes Editing" );
	check( !video.Draft().hdr.automatic && video.Applied() == initial, "no premature apply" );
	check( menu.Command( "HdrExposure150" ) == Action::Changed, "select exposure" );
	check( menu.Command( "HdrPeak617" ) == Action::Changed, "select calibrated peak" );
	check( menu.Draft().peakNits == 617, "exact user calibration preserved" );
	check( menu.Command( "HdrPeak0" ) == Action::Changed && menu.Draft().peakNits == 0,
	    "peak back to the display's" );
	check( menu.Command( "HdrExposure0" ) == Action::Changed && menu.Draft().exposure == 0.0f,
	    "exposure back to the system's" );
	check( menu.Command( "HdrExposure150" ) == Action::Changed &&
	           menu.Command( "HdrPeak617" ) == Action::Changed,
	    "manual override again" );
	check( backend.applies == 0 && backend.saves == 0, "selection has no external effects" );
	for ( const char *invalid :
	    { "HdrMode2", "HdrMode", "HdrExposure24", "HdrExposure401", "HdrPeak202", "HdrPeak1",
	        "HdrPeak10001", "HdrPeak1000junk", "HdrExposure99999999999" } )
	{
		const auto before = video.Draft();
		check( menu.Command( invalid ) == Action::Invalid, "malformed/range refusal" );
		check( video.Draft() == before, "refusal preserves full draft" );
	}
	auto draft = video.Draft();
	draft.width = 1920;
	draft.temporalScale = 0.5f;
	check( video.Stage( draft ), "other Video edits" );
	check( menu.Command( "Cancel" ) == Action::Cancel, "Back action" );
	check( video.Draft().hdr == initial.hdr && video.Draft().width == 1920 &&
	           video.Draft().temporalScale == 0.5f, "Back restores only HDR" );
	check( backend.applies == 0, "Back does not apply" );
	check( menu.Command( "HdrExposure200" ) == Action::Changed, "edit after cancel" );
	check( menu.Command( "ApplyHDR" ) == Action::Apply, "Apply routes to Video" );
	backend.reject = true;
	check( !video.Apply( backend ), "backend refusal" );
	check( video.Applied() == initial && video.GetState() == GraphicsSettingsService::State::Editing,
	    "failed apply stays retryable" );
	backend.reject = false;
	check( video.Apply( backend ), "apply complete Video draft" );
	check( backend.current.hdr.exposure == 2.0f && backend.current.width == 1920 &&
	           backend.current.temporalScale == 0.5f, "one transaction includes HDR/display/FSR" );
	menu.Commit();
	backend.rejectSave = true;
	check( !video.Save( backend ) && video.GetState() == GraphicsSettingsService::State::Applied,
	    "save failure preserves Applied" );
	backend.rejectSave = false;
	check( video.Save( backend ), "save retry" );
	check( menu.Command( "HdrMode0" ) == Action::Changed, "edit applied settings" );
	menu.Cancel();
	check( video.Draft() == video.Applied(), "cancel after Apply keeps committed HDR" );
	check( video.GetState() == GraphicsSettingsService::State::Saved, "cancel returns Saved" );
	HdrSettingsMenu reopened( video );
	check( reopened.Draft() == backend.current.hdr, "reopen reflects persisted settings" );
	check( reopened.Command( "HdrPeak600" ) == Action::Changed, "reopen edit" );
	video.Cancel();
	check( video.Draft() == video.Applied(), "parent Video discard includes HDR" );
	draft = video.Draft();
	draft.hdr.exposure = std::numeric_limits<float>::quiet_NaN();
	check( !video.Stage( draft ), "NaN rejected before mutation" );
	return testing::ReportConformance( checks, failures );
}
