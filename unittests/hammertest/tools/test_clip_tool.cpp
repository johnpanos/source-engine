//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.tools Clip tool (tools.clip_tool.v1): a snapped two-point
//			line in a 2D view defines a plane through the view's free axis
//			(front = left of the line as drawn); Shift+X cycles Front -> Back ->
//			Both; the preview shows the kept pieces; Enter applies one "Clip"
//			edit keeping the pieces selected; line points can be dragged after
//			placement. Negative checks: clicks and coincident points make no
//			line; Escape, focus loss and tool switches change nothing; Enter
//			without selected solids or with a plane that misses them returns
//			Failed with no edit; the 3D view is not a supported context.
//
//=============================================================================//

#include "hammer/tools/clip_tool.h"
#include "hammer/tools/selection_tool.h"
#include "testing/checks.h"
#include "tool_test_util.h"

using namespace tooltest;
using tools::ToolResultKind;

int main()
{
	testing::Checks checks;
	const ViewKind T = ViewKind::Top;

	DocBuilder b;
	const scene::ObjectId A = b.Box( Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) );
	b.Box( Vec3d( 200, 0, 0 ), Vec3d( 264, 64, 64 ) );
	b.Point( "light", Vec3d( -100, -100, 8 ) );
	Rig rig( b.doc );
	tools::ClipTool *tool = rig.Use( std::make_unique<tools::ClipTool>() );
	rig.manager.Add( std::make_unique<tools::SelectionTool>() );
	checks.That(
	    rig.session.SelectObjects( { A }, app::SelectMode::Replace ).HasValue(), "select A" );
	const auto box = [&]( scene::ObjectId id )
	{
		return BoundsOf( rig.session, id );
	};

	// --- Plane convention ---------------------------------------------------------------
	{
		const auto plane = tools::ClipTool::PlaneOf( { T, { 32, -32 }, { 32, 96 } } );
		checks.That( plane && plane->normal == Vec3d( -1, 0, 0 ) && plane->dist == -32.0,
		    "Top line going up: normal -X (front = left of the line)" );
		const auto front = tools::ClipTool::PlaneOf( { ViewKind::Front, { -32, 32 }, { 96, 32 } } );
		checks.That( front && front->normal == Vec3d( 0, 0, 1 ) && front->dist == 32.0,
		    "Front line going right: normal +Z (front = above)" );
		checks.That(
		    !tools::ClipTool::PlaneOf( { T, { 1, 1 }, { 1, 1 } } ), "coincident points: no plane" );
	}

	// --- Line gestures ------------------------------------------------------------------------
	checks.That(
	    Is( rig.Drag( T, 10, 10, 11, 11 ), ToolResultKind::CaptureReleased ) && !tool->Line(),
	    "a click makes no line" );
	rig.Drag( T, 0, 0, 3, 4 );
	checks.That( !tool->Line(), "points snapping together make no line" );
	rig.Down( T, 30, -30 );
	rig.Move( T, 33, 90 );
	checks.That(
	    rig.Overlay( T ).Count( tools::OverlayKind::WorldLine, tools::OverlayRole::Clip ) == 13,
	    "while dragging: the kept front piece (12 edges) and the line" );
	rig.Up( T, 33, 90 );
	checks.That( tool->Line() && tool->Line()->a == viewport::PlanePoint{ 32, -32 } &&
	                 tool->Line()->b == viewport::PlanePoint{ 32, 96 },
	    "snapped line" );
	{
		const tools::OverlayList overlay = rig.Overlay( T );
		checks.Equal( overlay.Count( tools::OverlayKind::ScreenHandle, tools::OverlayRole::Handle ),
		    std::size_t( 2 ), "two point handles" );
		checks.Equal( rig.Overlay( ViewKind::Front )
		                  .Count( tools::OverlayKind::ScreenHandle, tools::OverlayRole::Handle ),
		    std::size_t( 0 ), "handles only in the drawing view" );
		checks.Equal( rig.Overlay( ViewKind::Front )
		                  .Count( tools::OverlayKind::WorldLine, tools::OverlayRole::Clip ),
		    std::size_t( 12 ), "other views show the kept piece" );
	}
	checks.Equal( tool->Status(), std::string( "Clip: keep front" ), "status" );
	checks.Equal( rig.HistorySize(), std::size_t( 0 ), "a line is not an edit" );

	// Moving a point; Escape mid-drag restores it.
	{
		const viewport::ScreenPoint pa = rig.Px( T, 32, -32 );
		checks.Equal( rig.CursorAt( T, pa ), tools::Cursor::Move, "move cursor over a point" );
		rig.Pointer( T, PointerPhase::Down, pa );
		rig.Move( T, 60, -40 );
		checks.Equal( rig.Overlay( T ).Count(
		                  tools::OverlayKind::ScreenHandle, tools::OverlayRole::HandleHot ),
		    std::size_t( 1 ), "the dragged point is hot" );
		rig.Key( T, tools::Key::Escape );
		checks.That(
		    tool->Line()->a == viewport::PlanePoint{ 32, -32 }, "Escape restores the point" );
		rig.Pointer( T, PointerPhase::Down, pa );
		rig.Move( T, 60, -40 );
		rig.Up( T, 60, -40 );
		checks.That( tool->Line()->a == viewport::PlanePoint{ 64, -48 },
		    "the point moved to the snapped pointer" );
		rig.Pointer( T, PointerPhase::Down, rig.Px( T, 64, -48 ) );
		rig.Move( T, 32, -32 );
		rig.manager.OnFocusLost();
		checks.That(
		    tool->Line()->a == viewport::PlanePoint{ 64, -48 }, "focus loss restores the point" );
		rig.Drag( T, 64, -48, 32, -32 );
		checks.That( tool->Line()->a == viewport::PlanePoint{ 32, -32 }, "moved back" );
	}

	// --- Apply --------------------------------------------------------------------------------
	checks.That( Is( rig.Key( T, tools::Key::Enter ), ToolResultKind::Handled ), "Enter clips" );
	checks.That(
	    box( A ) && box( A )->mins == Vec3d( 0, 0, 0 ) && box( A )->maxs == Vec3d( 32, 64, 64 ),
	    "keep front: the left half" );
	checks.Equal( rig.HistorySize(), std::size_t( 1 ), "one unit" );
	checks.Equal( rig.LastLabel(), std::string( "Clip" ), "labeled" );
	checks.That( !tool->Line() &&
	                 rig.session.CurrentSelection().objects == std::vector<scene::ObjectId>{ A },
	    "line cleared, A still selected" );
	checks.That( Is( rig.Key( T, tools::Key::Enter ), ToolResultKind::Ignored ),
	    "Enter without a line: ignored" );

	// Keep cycling: Front -> Back -> Both.
	checks.That( rig.session.Undo().HasValue(), "undo" );
	checks.That( Is( rig.Key( T, tools::KeyEvent::Char( 'x' ) ), ToolResultKind::Ignored ),
	    "plain x ignored" );
	checks.That( Is( rig.Key( T, tools::KeyEvent::Char( 'X', Mods( tools::kShift ) ) ),
	                 ToolResultKind::Handled ) &&
	                 tool->Keep() == app::ops::ClipKeep::Back,
	    "Shift+X: back" );
	rig.Drag( T, 32, -32, 32, 96 );
	rig.Key( T, tools::Key::Enter );
	checks.That( box( A )->mins == Vec3d( 32, 0, 0 ) && box( A )->maxs == Vec3d( 64, 64, 64 ),
	    "keep back: the right half" );
	checks.That( rig.session.Undo().HasValue(), "undo" );
	rig.Key( T, tools::KeyEvent::Char( 'x', Mods( tools::kShift ) ) );
	checks.That( tool->Keep() == app::ops::ClipKeep::Both, "Shift+X: both" );
	rig.Drag( T, 32, -32, 32, 96 );
	{
		checks.Equal(
		    rig.Overlay( T ).Count( tools::OverlayKind::WorldLine, tools::OverlayRole::Clip ),
		    std::size_t( 25 ), "both pieces previewed (12 + 12) and the line" );
		const std::size_t solids = rig.session.Document().SolidIds().size();
		rig.Key( T, tools::Key::Enter );
		checks.Equal(
		    rig.session.Document().SolidIds().size(), solids + 1, "split into two solids" );
		checks.Equal( rig.session.CurrentSelection().objects.size(), std::size_t( 2 ),
		    "both pieces selected" );
		checks.Equal( rig.HistorySize(), std::size_t( 1 ), "one unit (after the undos)" );
	}
	rig.Key( T, tools::KeyEvent::Char( 'x', Mods( tools::kShift ) ) );
	checks.That( tool->Keep() == app::ops::ClipKeep::Front, "Shift+X wraps to front" );

	// Front view.
	checks.That( rig.session.Undo().HasValue(), "undo" );
	rig.Drag( ViewKind::Front, -30, 30, 90, 34 );
	rig.Key( ViewKind::Front, tools::Key::Enter );
	checks.That( box( A )->mins == Vec3d( 0, 0, 32 ) && box( A )->maxs == Vec3d( 64, 64, 64 ),
	    "Front clip keeps the top" );

	// --- Failures ------------------------------------------------------------------------------
	{
		const std::uint64_t revision = rig.session.Revision();
		rig.Drag( T, 500, -32, 500, 96 );
		auto r = rig.Key( T, tools::Key::Enter );
		checks.That( Is( r, ToolResultKind::Failed ) && !r.message.empty(),
		    "a plane missing the selection fails" );
		checks.That( tool->Line().has_value(), "the line stays after a failure" );
		checks.That( rig.session.ClearSelection().HasValue(), "clear" );
		rig.Drag( T, 32, -32, 32, 96 );
		r = rig.Key( T, tools::Key::Enter );
		checks.That( Is( r, ToolResultKind::Failed ) && r.message == "no solids selected",
		    "no selection fails" );
		checks.That( rig.session.Revision() == revision, "no edit" );
		checks.That(
		    Is( rig.Key( T, tools::Key::Escape ), ToolResultKind::Handled ) && !tool->Line(),
		    "Escape clears the line" );
		checks.That( Is( rig.Key( T, tools::Key::Escape ), ToolResultKind::Ignored ),
		    "Escape with no line: ignored" );
	}
	// Tool switch clears the line; 3D is unsupported.
	rig.Drag( T, 32, -32, 32, 96 );
	rig.manager.Activate( tools::SelectionTool::kName );
	rig.manager.Activate( tools::ClipTool::kName );
	checks.That( !tool->Line(), "tool switch clears the line" );
	checks.That( Is( rig.Pointer( ViewKind::Camera3D, PointerPhase::Down, { 400, 300 } ),
	                 ToolResultKind::Ignored ),
	    "3D press ignored (unsupported view)" );
	checks.That( Is( rig.Key( ViewKind::Camera3D, tools::Key::Enter ), ToolResultKind::Ignored ),
	    "3D key ignored" );
	checks.Equal( rig.CursorAt( ViewKind::Camera3D, { 400, 300 } ), tools::Cursor::Forbidden,
	    "3D cursor forbidden" );
	checks.That( rig.session.Document().Validate().empty(), "document valid" );

	return checks.Report();
}
