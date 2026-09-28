//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.tools interaction policy (tools.interaction_policy.v1): the
//			one owner of the drag threshold, handle hit metric, grid snapping
//			with Alt disabling it, Shift's dominant-axis constraint, reference
//			point drag deltas, rotation steps, nudge deltas per view, the grid
//			derived from EditorSettings, the 2D box handles, and hit ordering
//			through viewport picking + app::ResolvePick (groups, brush entities,
//			marquee resolution). Negative checks: sub-threshold motion, disabled
//			grids, non-arrow keys, misses.
//
//=============================================================================//

#include "hammer/tools/box_handles.h"
#include "hammer/tools/interaction_policy.h"
#include "testing/checks.h"
#include "tool_test_util.h"

using namespace tooltest;
using namespace hammer::tools;
using viewport::PlanePoint;

int main()
{
	testing::Checks checks;

	// --- Threshold and handles ------------------------------------------------------------------
	checks.That( !ExceedsDragThreshold( { 0, 0 }, { 3, -3 } ), "3 px is not a drag" );
	checks.That(
	    ExceedsDragThreshold( { 0, 0 }, { 3.5, 0 } ) && ExceedsDragThreshold( { 0, 0 }, { 0, -4 } ),
	    "more than 3 px on either axis is" );
	checks.That( WithinHandle( { 10, 10 }, { 14, 6 } ) && !WithinHandle( { 10, 10 }, { 14.5, 10 } ),
	    "square handle test" );

	// --- Snapping -----------------------------------------------------------------------------------
	viewport::GridPolicy grid;
	grid.SetSize( 16 );
	checks.Equal( SnapValue( grid, 23, {} ), 16.0, "nearest multiple" );
	checks.Equal( SnapValue( grid, 24, {} ), 32.0, "ties away from zero" );
	checks.Equal( SnapValue( grid, -24, {} ), -32.0, "negative ties away from zero" );
	checks.Equal( SnapValue( grid, 23, Mods( kAlt ) ), 23.0, "Alt disables snapping" );
	checks.That( SnapPlanePoint( grid, { 7, 9 }, {} ) == PlanePoint{ 0, 16 }, "plane point" );
	checks.That(
	    SnapWorldPoint( grid, Vec3d( 7, 9, 30 ), {}, { true, true, false } ) == Vec3d( 0, 16, 30 ),
	    "world point with an axis mask" );
	{
		viewport::GridPolicy off = grid;
		off.SetEnabled( false );
		checks.Equal( SnapValue( off, 23, {} ), 23.0, "a disabled grid does not snap" );
	}
	checks.That( ConstrainToDominantAxis( { 10, -40 } ) == PlanePoint{ 0, -40 } &&
	                 ConstrainToDominantAxis( { 10, 10 } ) == PlanePoint{ 10, 0 },
	    "dominant axis, ties keep u" );
	checks.That( DragDelta( grid, { 5, 0 }, { 20, 3 }, {} ) == PlanePoint{ 27, 0 },
	    "the reference lands on the grid" );
	checks.That( DragDelta( grid, { 0, 0 }, { 10, 40 }, Mods( kShift ) ) == PlanePoint{ 0, 48 },
	    "Shift constrains then snaps" );
	checks.That( DragDelta( grid, { 5, 5 }, { 10, 40 }, Mods( kShift ) ) == PlanePoint{ 0, 43 },
	    "a constrained-away axis stays zero even off the grid" );
	checks.That( DragDelta( grid, { 5, 0 }, { 20, 3 }, Mods( kAlt ) ) == PlanePoint{ 20, 3 },
	    "Alt: raw delta" );
	checks.That(
	    PlaneDeltaToWorld( viewport::AxesOf( ViewKind::Side ), { 1, 2 } ) == Vec3d( 0, 1, 2 ),
	    "Side u=y, v=z" );
	checks.Equal( SnapAngle( 7.3, {} ), 7.5, "half-degree steps" );
	checks.Equal( SnapAngle( -7.3, {} ), -7.5, "negative" );
	checks.Equal( SnapAngle( 50, Mods( kShift ) ), 45.0, "Shift: 15 degree steps" );
	checks.Equal( SnapAngle( 52.5, Mods( kShift ) ), 60.0, "ties away from zero" );
	checks.Equal( SnapAngle( 7.3, Mods( kAlt ) ), 7.3, "Alt: free" );

	// --- Nudge ----------------------------------------------------------------------------------------
	checks.That(
	    NudgeDelta( ViewKind::Top, Key::Right, {}, grid, NudgeStep::Grid ) == Vec3d( 16, 0, 0 ),
	    "Top Right" );
	checks.That(
	    NudgeDelta( ViewKind::Front, Key::Up, {}, grid, NudgeStep::Grid ) == Vec3d( 0, 0, 16 ),
	    "Front Up = +z" );
	checks.That(
	    NudgeDelta( ViewKind::Side, Key::Left, {}, grid, NudgeStep::Grid ) == Vec3d( 0, -16, 0 ),
	    "Side Left = -y" );
	checks.That( NudgeDelta( ViewKind::Camera3D, Key::Down, {}, grid, NudgeStep::Grid ) ==
	                 Vec3d( 0, -16, 0 ),
	    "3D Down = world -y" );
	checks.That( NudgeDelta( ViewKind::Top, Key::Right, Mods( kCtrl ), grid, NudgeStep::Grid ) ==
	                 Vec3d( 1, 0, 0 ),
	    "Ctrl: one unit" );
	checks.That(
	    NudgeDelta( ViewKind::Top, Key::Right, {}, grid, NudgeStep::Unit ) == Vec3d( 1, 0, 0 ),
	    "Unit step" );
	checks.That(
	    !NudgeDelta( ViewKind::Top, Key::Enter, {}, grid, NudgeStep::Grid ), "not an arrow" );

	// --- Grid from EditorSettings ------------------------------------------------------------------------
	{
		app::EditorSettings editor;
		editor.gridSize = 16;
		checks.Equal( GridFrom( editor ).Size(), 16, "16" );
		editor.gridSize = 24;
		checks.Equal( GridFrom( editor ).Size(), 32, "24 rounds to 32" );
		editor.gridSize = 0.3;
		checks.Equal( GridFrom( editor ).Size(), 1, "clamped to 1" );
		editor.gridSize = 5000;
		checks.Equal( GridFrom( editor ).Size(), 1024, "clamped to 1024" );
		editor.snapToGrid = false;
		checks.That( !GridFrom( editor ).Enabled(), "snap off" );
	}

	// --- Box handles -----------------------------------------------------------------------------------------
	{
		viewport::Camera2D top;
		top.SetViewport( 800, 600 );
		top.SetZoom( 1.0 );
		const viewport::PlaneRect rect{ { 0, 0 }, { 64, 32 } };
		checks.Equal(
		    VisibleHandles( BoxHandleMode::Scale ).size(), std::size_t( 8 ), "8 scale handles" );
		checks.Equal(
		    VisibleHandles( BoxHandleMode::Rotate ).size(), std::size_t( 4 ), "4 rotate handles" );
		const viewport::ScreenPoint ne = HandlePosition( top, rect, { 1, 1 } );
		checks.That( ne.x == 470 && ne.y == 262, "NE handle 6 px outside the corner" );
		checks.That(
		    HitHandle( top, rect, BoxHandleMode::Scale, 472, 259 ) == BoxHandle{ 1, 1 }, "hit NE" );
		checks.That( !HitHandle( top, rect, BoxHandleMode::Scale, 432, 284 ),
		    "the box interior is no handle" );
		checks.That( !HitHandle( top, rect, BoxHandleMode::Rotate, 470, 284 ),
		    "no edge handles in rotate mode" );
		checks.Equal(
		    HandleCursor( { -1, 0 }, BoxHandleMode::Scale ), Cursor::ResizeW, "W cursor" );
		checks.Equal(
		    HandleCursor( { 1, -1 }, BoxHandleMode::Scale ), Cursor::ResizeSE, "SE cursor" );
		const viewport::PlaneRect r = ResizeRect( grid, rect, { 1, 0 }, { 30, 99 }, {} );
		checks.That( r.max.u == 96 && r.max.v == 32 && r.min.u == 0,
		    "an E handle moves only the right edge, snapped" );
		checks.That( !RectHasArea( ResizeRect( grid, rect, { 1, 0 }, { -64, 0 }, {} ) ),
		    "collapsing leaves no area" );
	}

	// --- Hit ordering ------------------------------------------------------------------------------------------
	{
		DocBuilder b;
		const scene::ObjectId s1 = b.Box( Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) );
		const scene::ObjectId s2 = b.Box( Vec3d( 100, 0, 0 ), Vec3d( 164, 64, 64 ) );
		const scene::ObjectId s3 = b.Box( Vec3d( 300, 0, 0 ), Vec3d( 364, 64, 64 ) );
		scene::ObjectId group;
		scene::ObjectId door;
		{
			scene::DocumentEdit edit( b.doc );
			group = edit.Add( scene::Group{} );
			edit.MutableSolid( s1 )->group = group;
			edit.MutableSolid( s2 )->group = group;
			scene::Entity d;
			d.classname = "func_door";
			door = edit.Add( d );
			edit.MutableSolid( s3 )->owner = door;
			scene::CommitEdit( b.doc, edit );
		}
		viewport::Camera2D top;
		top.SetViewport( 800, 600 );
		top.SetZoom( 1.0 );
		PickSettings picking;
		picking.granularity = app::SelectionGranularity::Groups;
		checks.That(
		    PickObject2D( b.doc, top, 432, 268, picking )->target == group, "Groups: the group" );
		picking.granularity = app::SelectionGranularity::Solids;
		const auto solid = PickObject2D( b.doc, top, 432, 268, picking );
		checks.That( solid && solid->target == s1 && solid->object == s1, "Solids: the solid" );
		picking.granularity = app::SelectionGranularity::Objects;
		checks.That( PickObject2D( b.doc, top, 732, 268, picking )->target == door,
		    "Objects: the brush entity" );
		checks.That( !PickObject2D( b.doc, top, 50, 50, picking ), "a miss" );
		picking.granularity = app::SelectionGranularity::Groups;
		checks.That(
		    MarqueeTargets( b.doc, top, { { 380, 200 }, { 580, 320 } },
		        viewport::MarqueeMode::Inside, picking ) == std::vector<scene::ObjectId>{ group },
		    "a marquee over a group's solids yields the group once" );
		picking.granularity = app::SelectionGranularity::Solids;
		checks.That( MarqueeTargets( b.doc, top, { { 380, 200 }, { 800, 320 } },
		                 viewport::MarqueeMode::Inside,
		                 picking ) == std::vector<scene::ObjectId>{ s1, s2, s3 },
		    "Solids marquee yields the solids, brush-entity solids included" );
		viewport::Camera3D camera;
		camera.SetViewport( 800, 600 );
		camera.SetPosition( Vec3d( 32, -200, 32 ) );
		camera.SetAngles( 90, 0 );
		const auto hit = PickObject3D( b.doc, camera, 400, 300, picking );
		checks.That( hit && hit->target == s1 && hit->hasPoint && hit->face.solid == s1 &&
		                 hit->normal == Vec3d( 0, -1, 0 ),
		    "3D: the entered face, point and normal" );
		checks.Near( hit ? hit->point.y : 1.0, 0.0, 1e-9, "3D hit point on the face" );
	}

	return checks.Report();
}
