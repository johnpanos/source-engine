//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.tools Entity tool (tools.entity_tool.v1): 2D click places
//			at the snapped point (depth from the last 3D placement, else 0) with
//			class defaults and selects it in one "Place entity" unit; a drag
//			places at the release point; 3D places on the ray-hit surface pushed
//			out by the class marker box; hover previews. Negative checks:
//			Escape and focus loss place nothing; no class, an unknown class, a
//			brush class and a click on the sky return Failed with no edit;
//			other buttons are Ignored.
//
//=============================================================================//

#include "fakes/fake_entity_catalog.h"
#include "hammer/tools/entity_tool.h"
#include "testing/checks.h"
#include "tool_test_util.h"

#include <cmath>

using namespace tooltest;
using tools::ToolResultKind;

int main()
{
	testing::Checks checks;
	const ViewKind T = ViewKind::Top;

	hammertest::FakeEntityCatalog catalog;
	using C = hammertest::FakeEntityCatalog;
	catalog.AddPoint( "info_player_start", { C::Key( "angles", "angle", "0 0 0" ) } )
	    .Box( { -16, -16, 0 }, { 16, 16, 72 } );
	catalog.AddPoint( "light", { C::Key( "_light", "color255", "255 255 255 200" ) } );
	catalog.AddSolid( "func_door" );

	DocBuilder b;
	b.Box( Vec3d( -512, -512, -16 ), Vec3d( 512, 512, 0 ) );
	b.Box( Vec3d( 200, -512, 0 ), Vec3d( 216, 512, 256 ) );
	Rig rig( b.doc );
	rig.catalog = &catalog;
	tools::EntityTool *tool = rig.Use( std::make_unique<tools::EntityTool>() );
	const auto selected = [&]() -> const scene::Entity *
	{
		const app::Selection &sel = rig.session.CurrentSelection();
		return sel.objects.size() == 1 ? rig.session.Document().FindEntity( sel.objects[0] )
		                               : nullptr;
	};

	// --- 2D click ----------------------------------------------------------------------
	{
		checks.That(
		    Is( rig.Down( T, 37, -21 ), ToolResultKind::CaptureRequested ), "press captures" );
		checks.Equal(
		    rig.Overlay( T ).Count( tools::OverlayKind::WorldBox, tools::OverlayRole::Pending ),
		    std::size_t( 1 ), "marker preview while pressed" );
		checks.That( rig.session.Document().EntityIds().empty(), "nothing placed on press" );
		checks.That(
		    Is( rig.Up( T, 37, -21 ), ToolResultKind::CaptureReleased ), "release places" );
		const scene::Entity *e = selected();
		checks.That( e && e->classname == "info_player_start" && e->Origin() == Vec3d( 32, -16, 0 ),
		    "placed at the snapped point, depth 0, and selected" );
		checks.That(
		    e && e->Key( "angles" ) && *e->Key( "angles" ) == "0 0 0", "class defaults applied" );
		checks.Equal( rig.HistorySize(), std::size_t( 1 ), "one unit" );
		checks.Equal( rig.LastLabel(), std::string( "Place entity" ), "labeled" );
	}
	rig.Drag( T, 64, 64, 65, 66 );
	checks.That( selected() && selected()->Origin() == Vec3d( 64, 64, 0 ),
	    "a below-threshold drag places at the press" );
	rig.Drag( T, 100, 100, 160, 100 );
	checks.That(
	    selected() && selected()->Origin() == Vec3d( 160, 96, 0 ), "a drag places at the release" );
	checks.Equal( rig.HistorySize(), std::size_t( 3 ), "one unit per placement" );

	// --- Cancellation -------------------------------------------------------------------------
	{
		rig.Down( T, 0, 0 );
		checks.That( Is( rig.Key( T, tools::Key::Escape ), ToolResultKind::CaptureReleased ),
		    "Escape releases" );
		checks.That( Is( rig.Up( T, 0, 0 ), ToolResultKind::Ignored ),
		    "the release after Escape is ignored" );
		rig.Down( T, 0, 0 );
		rig.manager.OnFocusLost();
		rig.Up( T, 0, 0 );
		rig.Down( T, 0, 0 );
		rig.manager.OnCaptureLost();
		checks.Equal(
		    rig.HistorySize(), std::size_t( 3 ), "Escape, focus and capture loss place nothing" );
		checks.That( Is( rig.Key( T, tools::Key::Escape ), ToolResultKind::Ignored ),
		    "Escape when idle: ignored" );
	}

	// --- 3D -----------------------------------------------------------------------------------
	const ViewKind V = ViewKind::Camera3D;
	{
		rig.camera.SetPosition( Vec3d( 0, -300, 100 ) );
		rig.camera.SetAngles( 90, 20 );
		checks.Equal(
		    rig.CursorAt( V, { 400, 300 } ), tools::Cursor::Crosshair, "crosshair over the floor" );
		rig.Pointer( V, PointerPhase::Down, { 400, 300 } );
		rig.Pointer( V, PointerPhase::Up, { 400, 300 } );
		const scene::Entity *e = selected();
		checks.That( e && e->Origin() && e->Origin()->z == 0.0,
		    "player start stands on the floor (box mins z 0)" );
		checks.That(
		    e && e->Origin() && std::fmod( e->Origin()->y, 16.0 ) == 0.0 && e->Origin()->x == 0.0,
		    "surface point snapped on the tangent axes" );
		checks.That( tool->Last3DPlacement() && e && *tool->Last3DPlacement() == *e->Origin(),
		    "last 3D placement kept" );
	}
	{
		rig.editor.entityClass = "light";
		rig.camera.SetPosition( Vec3d( 0, 0, 64 ) );
		rig.camera.SetAngles( 0, 0 );
		rig.Pointer( V, PointerPhase::Down, { 400, 300 } );
		rig.Pointer( V, PointerPhase::Up, { 400, 300 } );
		checks.That( selected() && selected()->Origin() == Vec3d( 192, 0, 64 ),
		    "a light on the wall sits half its marker out along the normal" );
	}
	// 2D depth follows the last 3D placement.
	rig.Drag( T, 10, 10, 10, 10 );
	checks.That( selected() && selected()->Origin() == Vec3d( 16, 16, 64 ),
	    "Top depth from the last 3D placement" );
	rig.Drag( ViewKind::Front, 10, 10, 10, 10 );
	checks.That( selected() && selected()->Origin() == Vec3d( 16, 0, 16 ),
	    "Front depth (y) from the last 3D placement" );

	// --- Hover ---------------------------------------------------------------------------------
	{
		checks.That(
		    Is( rig.Pointer( T, PointerPhase::Move, rig.Px( T, 40, 40 ), {}, PointerButton::None ),
		        ToolResultKind::Handled ),
		    "hover is handled when the preview moves" );
		checks.Equal(
		    rig.Overlay( T ).Count( tools::OverlayKind::WorldBox, tools::OverlayRole::Hover ),
		    std::size_t( 1 ), "hover marker in the hovered view" );
		checks.Equal( rig.Overlay( ViewKind::Front ).CountRole( tools::OverlayRole::Hover ),
		    std::size_t( 0 ), "not in other views" );
		checks.That(
		    Is( rig.Pointer( T, PointerPhase::Move, rig.Px( T, 41, 40 ), {}, PointerButton::None ),
		        ToolResultKind::Ignored ),
		    "a hover that snaps to the same point is ignored" );
	}

	// --- Failures ------------------------------------------------------------------------------
	{
		const std::size_t history = rig.HistorySize();
		rig.editor.entityClass.clear();
		auto r = rig.Drag( T, 0, 0, 0, 0 );
		checks.That(
		    Is( r, ToolResultKind::Failed ) && !r.message.empty() && !rig.manager.HasCapture(),
		    "no class" );
		rig.editor.entityClass = "no_such_class";
		r = rig.Drag( T, 0, 0, 0, 0 );
		checks.That( Is( r, ToolResultKind::Failed ) && !r.message.empty(), "unknown class fails" );
		rig.editor.entityClass = "func_door";
		r = rig.Drag( T, 0, 0, 0, 0 );
		checks.That( Is( r, ToolResultKind::Failed ), "a brush class cannot be placed" );
		rig.editor.entityClass = "light";
		rig.camera.SetPosition( Vec3d( 0, 0, 64 ) );
		rig.camera.SetAngles( 180, -60 );
		checks.Equal(
		    rig.CursorAt( V, { 400, 300 } ), tools::Cursor::Forbidden, "forbidden over the sky" );
		rig.Pointer( V, PointerPhase::Down, { 400, 300 } );
		r = rig.Pointer( V, PointerPhase::Up, { 400, 300 } );
		checks.That( Is( r, ToolResultKind::Failed ) && r.message == "no surface under the pointer",
		    "sky fails" );
		checks.Equal( rig.HistorySize(), history, "no failed placement made an edit" );
	}
	checks.That( Is( rig.Down( T, 0, 0, {}, PointerButton::Right ), ToolResultKind::Ignored ),
	    "right press ignored" );
	checks.That(
	    Is( rig.Up( T, 0, 0 ), ToolResultKind::Ignored ), "release without press ignored" );
	checks.That( rig.session.Document().Validate().empty(), "document valid" );

	return checks.Report();
}
