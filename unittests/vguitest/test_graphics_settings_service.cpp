//========= Copyright Valve Corporation, All rights reserved. ============//

#include "gameui/graphics_settings_service.h"
#include "testing/conformance_result.h"

#include <cstdio>
#include <limits>
#include <initializer_list>

#define CHECK( expression )                                                                        \
	do                                                                                             \
	{                                                                                              \
		++checks;                                                                                  \
		if ( !( expression ) )                                                                     \
		{                                                                                          \
			++failures;                                                                            \
			std::fprintf( stderr, "check failed: %s at %d\n", #expression, __LINE__ );             \
		}                                                                                          \
	} while ( false )

using gameui::GraphicsSettings;
using gameui::GraphicsSettingsService;

class TestBackend final : public gameui::IGraphicsSettingsBackend
{
public:
	bool rejectApply = false;
	bool rejectSave = false;
	int applyCount = 0;
	int saveCount = 0;
	GraphicsSettings lastApplied;
	GraphicsSettings previous;

	bool Apply( const GraphicsSettings &from, const GraphicsSettings &to ) override
	{
		++applyCount;
		if ( rejectApply )
			return false;
		previous = from;
		lastApplied = to;
		return true;
	}

	bool Save() override
	{
		++saveCount;
		return !rejectSave;
	}
};

int main()
{
	int checks = 0;
	int failures = 0;
	GraphicsSettingsService service;
	TestBackend backend;
	GraphicsSettings original;
	original.width = 1280;
	original.height = 720;
	original.windowed = true;

	CHECK( !service.Apply( backend ) );
	CHECK( !service.Save( backend ) );
	service.Begin( original );
	GraphicsSettings draft = original;
	draft.width = 1920;
	CHECK( service.Stage( draft ) );
	CHECK( service.GetState() == GraphicsSettingsService::State::Editing );
	service.Cancel();
	CHECK( service.Draft() == original );
	CHECK( backend.applyCount == 0 && backend.saveCount == 0 );

	CHECK( service.Stage( draft ) );
	backend.rejectApply = true;
	CHECK( !service.Apply( backend ) );
	CHECK( !service.Save( backend ) );
	CHECK( service.GetState() == GraphicsSettingsService::State::Editing );
	backend.rejectApply = false;
	CHECK( service.Apply( backend ) );
	CHECK( backend.lastApplied == draft );
	backend.rejectSave = true;
	CHECK( !service.Save( backend ) );
	CHECK( service.GetState() == GraphicsSettingsService::State::Applied );
	backend.rejectSave = false;
	CHECK( service.Save( backend ) );
	CHECK( service.GetState() == GraphicsSettingsService::State::Saved );

	GraphicsSettings invalid = draft;
	invalid.width = 0;
	CHECK( !service.Stage( invalid ) );
	invalid = draft;
	invalid.uiScale = std::numeric_limits<float>::quiet_NaN();
	CHECK( !service.Stage( invalid ) );
	invalid = draft;
	invalid.windowed = false;
	invalid.borderless = true;
	CHECK( !service.Stage( invalid ) );
	CHECK( service.Draft() == draft );

	// Applied changes survive a later Cancel, but an unsaved apply remains pending.
	draft.height = 1080;
	CHECK( service.Stage( draft ) && service.Apply( backend ) );
	draft.width = 800;
	CHECK( service.Stage( draft ) );
	service.Cancel();
	CHECK( service.GetState() == GraphicsSettingsService::State::Applied );
	CHECK( service.Draft() == service.Applied() );
	CHECK( service.Save( backend ) );
	service.Begin( original );
	CHECK( service.GetState() == GraphicsSettingsService::State::Saved );
	CHECK( service.Applied() == original );

	// The same session can move through fullscreen, decorated and borderless.
	original.windowed = false;
	service.Begin( original );
	draft = original;
	draft.windowed = true;
	CHECK( service.Stage( draft ) && service.Apply( backend ) && service.Save( backend ) );
	CHECK( backend.previous == original && backend.lastApplied == draft );
	GraphicsSettings decorated = draft;
	draft.borderless = true;
	CHECK( service.Stage( draft ) && service.Apply( backend ) && service.Save( backend ) );
	CHECK( backend.previous == decorated && backend.lastApplied == draft );
	// Temporal quality is staged with the display mode; cancellation never applies it.
	service.Begin( original );
	draft = original;
	draft.width = 1920;
	draft.height = 1080;
	draft.temporalScale = 2.0f / 3.0f;
	CHECK( service.Stage( draft ) );
	service.Cancel();
	CHECK( service.Draft() == original );
	CHECK( service.Stage( draft ) && service.Apply( backend ) && service.Save( backend ) );
	CHECK( int( backend.lastApplied.width * backend.lastApplied.temporalScale ) == 1280 );
	CHECK( int( backend.lastApplied.height * backend.lastApplied.temporalScale ) == 720 );
	draft.temporalScale = 0.0f;
	CHECK( service.Stage( draft ) );
	service.Cancel();
	CHECK( service.Draft().temporalScale == 2.0f / 3.0f );
	CHECK( service.Stage( draft ) && service.Apply( backend ) && service.Save( backend ) );
	CHECK( backend.lastApplied.temporalScale == 0.0f );
	for ( float invalidScale : { 0.1f, 0.49f, 1.01f, std::numeric_limits<float>::infinity(),
	          std::numeric_limits<float>::quiet_NaN() } )
	{
		invalid = draft;
		invalid.temporalScale = invalidScale;
		CHECK( !service.Stage( invalid ) );
		CHECK( service.Draft() == draft );
	}
	return testing::ReportConformance( checks, failures );
}
