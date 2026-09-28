//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.tools Selection tool (tools.selection_tool.v1): click,
//			Ctrl-toggle and empty-click selection on release; below-threshold
//			drags act as clicks; marquee (Inside, Ctrl adds); move with grid
//			snapping, Shift axis constraint and Alt free; moving an unselected
//			object selects it in the same history unit; scale and rotate
//			handles with the legacy click-again mode cycle; nudge, delete and
//			Escape keys; 3D ray selection. Negative checks: Escape and focus
//			loss mid-drag leave no edit and the selection untouched; an inverted
//			scale returns Failed with no edit; other buttons and hovers are
//			Ignored; a 3D drag makes no edit.
//
//=============================================================================//

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
	const scene::ObjectId B = b.Box( Vec3d( 128, 0, 0 ), Vec3d( 192, 64, 64 ) );
	const scene::ObjectId R = b.Box( Vec3d( 0, 200, 0 ), Vec3d( 64, 232, 16 ) );
	const scene::MapDocument base = b.doc;
	Rig rig( b.doc );
	tools::SelectionTool *tool = rig.Use( std::make_unique<tools::SelectionTool>() );
	const auto sel = [&]()
	{
		return rig.session.CurrentSelection();
	};

	// --- Click selection (on release) ---------------------------------------------------
	{
		const auto down = rig.Down( T, 32, 32 );
		checks.That( Is( down, ToolResultKind::CaptureRequested ) && rig.manager.HasCapture(),
		    "press captures" );
		checks.That( sel().Empty(), "nothing is selected on press" );
		checks.Equal( rig.Overlay( T ).CountRole( tools::OverlayRole::Hover ) > 0, true,
		    "pressed object hovers" );
		const auto up = rig.Up( T, 32, 32 );
		checks.That( Is( up, ToolResultKind::CaptureReleased ) && !rig.manager.HasCapture(),
		    "release ends capture" );
		checks.That( sel().objects == std::vector<scene::ObjectId>{ A }, "click selects A" );
		checks.Equal( rig.HistorySize(), std::size_t( 0 ), "selection records no history" );
	}
	checks.That(
	    rig.Overlay( T ).Count( tools::OverlayKind::ScreenHandle, tools::OverlayRole::Handle ) == 8,
	    "scale mode shows 8 handles" );
	checks.That(
	    rig.Overlay( T ).Count( tools::OverlayKind::WorldBox, tools::OverlayRole::Selection ) == 1,
	    "selection bounds box" );
	rig.Drag( T, 32, 32, 32, 32 );
	checks.That( tool->HandleMode( sel() ) == tools::BoxHandleMode::Rotate,
	    "clicking the selection again: rotate" );
	checks.That(
	    rig.Overlay( T ).Count( tools::OverlayKind::ScreenHandle, tools::OverlayRole::Handle ) == 4,
	    "rotate mode shows 4 corner handles" );
	rig.Drag( T, 32, 32, 32, 32 );
	checks.That( tool->HandleMode( sel() ) == tools::BoxHandleMode::Scale, "and back to scale" );

	rig.Drag( T, 160, 32, 160, 32, Mods( tools::kCtrl ) );
	checks.That( sel().objects == std::vector<scene::ObjectId>{ A, B }, "Ctrl-click adds B" );
	rig.Drag( T, 160, 32, 160, 32, Mods( tools::kCtrl ) );
	checks.That( sel().objects == std::vector<scene::ObjectId>{ A }, "Ctrl-click toggles B off" );
	rig.Drag( T, 300, 300, 302, 301 );
	checks.That( sel().Empty(), "a below-threshold drag on empty is a click: clears" );
	rig.Drag( T, 32, 32, 34, 30 );
	checks.That( sel().objects == std::vector<scene::ObjectId>{ A } && rig.HistorySize() == 0,
	    "a below-threshold drag on an object selects it without moving" );

	// --- Move -------------------------------------------------------------------------
	{
		rig.Down( T, 32, 32 );
		rig.Move( T, 50, 36 );
		rig.Move( T, 69, 37 );
		checks.That( scene::SameContent( rig.session.Document(), base ),
		    "no document change during a drag" );
		const tools::OverlayList overlay = rig.Overlay( T );
		checks.Equal( overlay.Count( tools::OverlayKind::WorldLine, tools::OverlayRole::Pending ),
		    std::size_t( 12 ), "the move preview draws the moved box's 12 edges" );
		checks.That(
		    rig.Overlay( ViewKind::Front )
		            .Count( tools::OverlayKind::WorldLine, tools::OverlayRole::Pending ) == 12,
		    "other views show the preview too" );
		checks.Equal( rig.CursorAt( T, rig.Px( T, 69, 37 ) ), tools::Cursor::Move,
		    "move cursor while dragging" );
		const auto up = rig.Up( T, 69, 37 );
		checks.That( Is( up, ToolResultKind::CaptureReleased ), "move release" );
		checks.Equal( rig.HistorySize(), std::size_t( 1 ), "one history unit for the drag" );
		checks.That(
		    BoundsOf( rig.session, A )->mins == Vec3d( 32, 0, 0 ), "raw (37, 5) snaps to (32, 0)" );
	}
	rig.Drag( T, 64, 32, 74, 72, Mods( tools::kShift ) );
	checks.That( BoundsOf( rig.session, A )->mins == Vec3d( 32, 48, 0 ),
	    "Shift keeps the dominant axis (0, 40->48)" );
	rig.Drag( T, 64, 80, 101, 85, Mods( tools::kAlt ) );
	checks.That( BoundsOf( rig.session, A )->mins == Vec3d( 69, 53, 0 ), "Alt disables snapping" );
	checks.Equal( rig.HistorySize(), std::size_t( 3 ), "three moves, three units" );
	rig.Drag( T, 100, 85, 100, 85 );
	rig.Drag( T, 100, 85, 100, 85 ); // back to scale mode

	// Escape and focus loss mid-drag: no edit, no selection change.
	{
		const std::uint64_t revision = rig.session.Revision();
		const app::Selection before = sel();
		rig.Down( T, 100, 85 );
		rig.Move( T, 200, 185 );
		const auto esc = rig.Key( T, tools::Key::Escape );
		checks.That( Is( esc, ToolResultKind::CaptureReleased ) && !rig.manager.HasCapture(),
		    "Escape releases" );
		rig.Up( T, 200, 185 );
		checks.That( rig.session.Revision() == revision && sel() == before,
		    "Escape: no edit, same selection" );
		rig.Down( T, 160, 32 );
		rig.Move( T, 260, 32 );
		rig.manager.OnFocusLost();
		checks.That( !tool->InGesture() && !rig.manager.HasCapture(), "focus loss cancels" );
		checks.That(
		    Is( rig.Up( T, 260, 32 ), ToolResultKind::Ignored ), "the stray release is ignored" );
		checks.That( rig.session.Revision() == revision && sel() == before,
		    "focus loss on an unselected object: no edit, no selection" );
	}

	// Moving an unselected object selects it in the same history unit.
	{
		const std::size_t history = rig.HistorySize();
		rig.Drag( T, 160, 32, 160, 64 );
		checks.That( BoundsOf( rig.session, B )->mins == Vec3d( 128, 32, 0 ), "B moved" );
		checks.That(
		    sel().objects == std::vector<scene::ObjectId>{ B }, "B selected with the move" );
		checks.Equal( rig.HistorySize(), history + 1, "one unit" );
		checks.That( rig.session.Undo().HasValue(), "undo" );
		checks.That( BoundsOf( rig.session, B )->mins == Vec3d( 128, 0, 0 ) &&
		                 sel().objects == std::vector<scene::ObjectId>{ A },
		    "undo restores B and the previous selection" );
	}

	// --- Marquee ------------------------------------------------------------------------
	checks.That( rig.session.Undo().HasValue(), "undo" );
	checks.That( rig.session.Undo().HasValue(), "undo" );
	checks.That( rig.session.Undo().HasValue(), "undo" );
	checks.That( BoundsOf( rig.session, A )->mins == Vec3d( 0, 0, 0 ), "A back at the origin" );
	rig.Drag( T, 40, -40, 40, -40 ); // clear
	rig.Drag( T, -20, -20, 210, 80 );
	checks.That(
	    sel().objects == std::vector<scene::ObjectId>{ A, B }, "marquee selects the boxes inside" );
	rig.Drag( T, -20, 180, 80, 250, Mods( tools::kCtrl ) );
	checks.That( sel().objects == std::vector<scene::ObjectId>{ A, B, R }, "Ctrl marquee adds" );
	{
		rig.Down( T, -50, -50 );
		rig.Move( T, 0, 0 );
		checks.Equal(
		    rig.Overlay( T ).Count( tools::OverlayKind::ScreenRect, tools::OverlayRole::Pending ),
		    std::size_t( 1 ), "marquee rectangle" );
		rig.manager.OnCaptureLost();
		checks.That( sel().objects.size() == 3, "capture loss cancels the marquee" );
	}

	// --- Scale handles --------------------------------------------------------------------
	rig.Drag( T, 40, -40, 40, -40 ); // clear
	rig.Drag( T, 32, 32, 32, 32 );
	checks.That( sel().objects == std::vector<scene::ObjectId>{ A }, "A alone" );
	{
		const viewport::ScreenPoint ne{ 464 + 6, 236 - 6 };
		checks.Equal( rig.CursorAt( T, ne ), tools::Cursor::ResizeNE, "NE handle cursor" );
		rig.Pointer( T, PointerPhase::Move, ne, {}, PointerButton::None );
		checks.Equal( rig.Overlay( T ).Count(
		                  tools::OverlayKind::ScreenHandle, tools::OverlayRole::HandleHot ),
		    std::size_t( 1 ), "hovered handle is hot" );
		const std::size_t history = rig.HistorySize();
		rig.Pointer( T, PointerPhase::Down, ne );
		rig.Pointer( T, PointerPhase::Move, { ne.x + 20, ne.y - 20 } );
		rig.Pointer( T, PointerPhase::Move, { ne.x + 30, ne.y - 34 } );
		const auto up = rig.Pointer( T, PointerPhase::Up, { ne.x + 30, ne.y - 34 } );
		checks.That( Is( up, ToolResultKind::CaptureReleased ), "scale release" );
		checks.That( BoundsOf( rig.session, A )->mins == Vec3d( 0, 0, 0 ) &&
		                 BoundsOf( rig.session, A )->maxs == Vec3d( 96, 96, 64 ),
		    "NE handle scales to the snapped corner, depth kept" );
		checks.Equal( rig.HistorySize(), history + 1, "one unit" );
	}
	{
		// Dragging the E handle past the W edge inverts the box: refused.
		const std::size_t history = rig.HistorySize();
		const viewport::ScreenPoint e = rig.Px( T, 96, 48 );
		const viewport::ScreenPoint eh{ e.x + 6, e.y };
		rig.Pointer( T, PointerPhase::Down, eh );
		rig.Pointer( T, PointerPhase::Move, { eh.x - 150, eh.y } );
		checks.That( rig.Overlay( T ).CountRole( tools::OverlayRole::Error ) > 0,
		    "inverted scale previews as Error" );
		const auto up = rig.Pointer( T, PointerPhase::Up, { eh.x - 150, eh.y } );
		checks.That( Is( up, ToolResultKind::Failed ) && !up.message.empty(),
		    "inverted scale fails with a reason" );
		checks.That( rig.HistorySize() == history && !rig.manager.HasCapture(),
		    "no edit, capture released" );
	}

	// --- Rotate handles ---------------------------------------------------------------------
	{
		rig.Drag( T, 32, 216, 32, 216 );
		checks.That( sel().objects == std::vector<scene::ObjectId>{ R }, "R selected" );
		rig.Drag( T, 32, 216, 32, 216 );
		checks.That( tool->HandleMode( sel() ) == tools::BoxHandleMode::Rotate, "rotate mode" );
		// R's plane rect is (0, 200)-(64, 232); NE handle 6 px out.
		const viewport::ScreenPoint c = rig.Px( T, 64, 232 );
		const viewport::ScreenPoint ne{ c.x + 6, c.y - 6 };
		checks.Equal( rig.CursorAt( T, ne ), tools::Cursor::Rotate, "rotate cursor" );
		const viewport::PlanePoint hp = rig.top.ScreenToPlane( ne.x, ne.y );
		const viewport::PlanePoint center{ 32, 216 };
		const double ru = hp.u - center.u;
		const double rv = hp.v - center.v;
		const viewport::ScreenPoint target =
		    rig.Px( T, center.u - rv, center.v + ru ); // +90 degrees
		const std::size_t history = rig.HistorySize();
		rig.Pointer( T, PointerPhase::Down, ne );
		rig.Pointer( T, PointerPhase::Move, target );
		rig.Pointer( T, PointerPhase::Up, target );
		const std::optional<scene::Box> rb = BoundsOf( rig.session, R );
		checks.That( rb && mapgeometry::NearlyEqual( rb->mins, Vec3d( 16, 184, 0 ), 1e-6 ) &&
		                 mapgeometry::NearlyEqual( rb->maxs, Vec3d( 48, 248, 16 ), 1e-6 ),
		    "90 degree rotation about the bounds center" );
		checks.Equal( rig.HistorySize(), history + 1, "one unit" );
		checks.That( tool->HandleMode( sel() ) == tools::BoxHandleMode::Rotate,
		    "rotate mode survives the rotation" );
	}

	// --- Keys -------------------------------------------------------------------------------
	rig.Drag( T, 48, 48, 48, 48 ); // select A (0,0)-(96,96)
	{
		const std::size_t history = rig.HistorySize();
		checks.That(
		    Is( rig.Key( T, tools::Key::Right ), ToolResultKind::Handled ), "nudge handled" );
		checks.That(
		    BoundsOf( rig.session, A )->mins == Vec3d( 16, 0, 0 ), "Right nudges +u by the grid" );
		rig.Key( T, tools::Key::Right, Mods( tools::kCtrl ) );
		checks.That(
		    BoundsOf( rig.session, A )->mins == Vec3d( 17, 0, 0 ), "Ctrl nudges one unit" );
		rig.Key( ViewKind::Front, tools::Key::Up );
		checks.That( BoundsOf( rig.session, A )->mins == Vec3d( 17, 0, 16 ), "Front Up nudges +z" );
		rig.Key( ViewKind::Camera3D, tools::Key::Up );
		checks.That(
		    BoundsOf( rig.session, A )->mins == Vec3d( 17, 16, 16 ), "3D Up nudges world +y" );
		checks.Equal( rig.HistorySize(), history + 4, "one unit per nudge" );
		checks.That( Is( rig.Key( T, tools::KeyEvent::Release( tools::Key::Right ) ),
		                 ToolResultKind::Ignored ),
		    "key releases are ignored" );
		checks.That( Is( rig.Key( T, tools::Key::Delete ), ToolResultKind::Handled ), "delete" );
		checks.That( !rig.session.Document().Contains( A ) && rig.HistorySize() == history + 5,
		    "A deleted, one unit" );
		checks.That( Is( rig.Key( T, tools::Key::Delete ), ToolResultKind::Ignored ),
		    "nothing to delete: ignored" );
		checks.That( rig.session.Undo().HasValue(), "undo delete" );
		checks.That( rig.session.Document().Contains( A ) && sel().Contains( A ), "undo delete" );
		checks.That(
		    Is( rig.Key( T, tools::Key::Escape ), ToolResultKind::Handled ) && sel().Empty(),
		    "Escape clears the selection" );
		checks.That( Is( rig.Key( T, tools::Key::Escape ), ToolResultKind::Ignored ),
		    "Escape with nothing: ignored" );
		checks.That( Is( rig.Key( T, tools::Key::Left ), ToolResultKind::Ignored ),
		    "nudge with nothing: ignored" );
	}

	// --- 3D and negatives ------------------------------------------------------------------------
	{
		rig.camera.SetPosition( Vec3d( 65, -300, 48 ) );
		rig.camera.SetAngles( 90, 0 );
		const auto down = rig.Pointer( ViewKind::Camera3D, PointerPhase::Down, { 400, 300 } );
		checks.That( Is( down, ToolResultKind::CaptureRequested ), "3D press" );
		rig.Pointer( ViewKind::Camera3D, PointerPhase::Up, { 400, 300 } );
		checks.That(
		    sel().objects == std::vector<scene::ObjectId>{ A }, "3D click selects by ray" );
		const std::uint64_t revision = rig.session.Revision();
		rig.Pointer( ViewKind::Camera3D, PointerPhase::Down, { 400, 300 } );
		rig.Pointer( ViewKind::Camera3D, PointerPhase::Move, { 500, 350 } );
		const auto up = rig.Pointer( ViewKind::Camera3D, PointerPhase::Up, { 500, 350 } );
		checks.That( Is( up, ToolResultKind::CaptureReleased ) &&
		                 rig.session.Revision() == revision &&
		                 sel().objects == std::vector<scene::ObjectId>{ A },
		    "a 3D drag makes no edit and keeps the selection" );
	}
	checks.That( Is( rig.Down( T, 32, 32, {}, PointerButton::Right ), ToolResultKind::Ignored ),
	    "right press ignored" );
	rig.Pointer(
	    T, PointerPhase::Move, { 5, 5 }, {}, PointerButton::None ); // leaves the hot handle
	checks.That( Is( rig.Pointer( T, PointerPhase::Move, { 6, 5 }, {}, PointerButton::None ),
	                 ToolResultKind::Ignored ),
	    "hover over nothing is ignored" );
	checks.Equal(
	    rig.CursorAt( T, { 5, 5 } ), tools::Cursor::Default, "default cursor over nothing" );
	checks.That( rig.session.Document().Validate().empty(), "document valid" );

	return checks.Report();
}
