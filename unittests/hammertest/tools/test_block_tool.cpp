//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.tools Block tool (tools.block_tool.v1): 2D drag draws a
//			snapped pending box whose depth comes from the previous box, the
//			selection bounds or one grid step; handles resize and a drag inside
//			moves it; Enter commits one "Create block" edit selecting the new
//			solid (primitive height axis = the drawing view's depth axis) or an
//			arch; Escape discards. 3D: a base drag on the surface workplane and
//			a height drag. Negative checks: clicks, flat drags, Escape and focus
//			loss mid-drag change nothing; a refused primitive returns Failed
//			with no edit and keeps the pending box; a workplane behind the
//			camera fails the press; Enter without a box is Ignored.
//
//=============================================================================//

#include "hammer/tools/block_tool.h"
#include "hammer/tools/selection_tool.h"
#include "testing/checks.h"
#include "tool_test_util.h"

#include <cmath>

using namespace tooltest;
using tools::ToolResultKind;

namespace
{

scene::Box MakeBox( Vec3d a, Vec3d b )
{
	return { a, b };
}

// Independent oracle: the z = 0 point under a pixel, snapped to 16 on x/y.
Vec3d FloorPoint( const viewport::Camera3D &camera, viewport::ScreenPoint p )
{
	const viewport::Ray ray = *camera.RayThroughPixel( p.x, p.y );
	const double t = -ray.origin.z / ray.direction.z;
	const Vec3d hit = ray.origin + ray.direction * t;
	const auto snap = []( double v )
	{
		return 16.0 * std::floor( v / 16.0 + 0.5 );
	};
	return Vec3d( snap( hit.x ), snap( hit.y ), 0.0 );
}

} // namespace

int main()
{
	testing::Checks checks;
	const ViewKind T = ViewKind::Top;

	DocBuilder b;
	const scene::ObjectId floor = b.Box( Vec3d( -512, -512, -16 ), Vec3d( 512, 512, 0 ) );
	Rig rig( b.doc );
	tools::BlockTool *tool = rig.Use( std::make_unique<tools::BlockTool>() );
	rig.manager.Add( std::make_unique<tools::SelectionTool>() );
	const auto pending = [&]()
	{
		return tool->Pending() ? tool->Pending()->box : scene::Box{};
	};

	// --- 2D ---------------------------------------------------------------------------
	checks.That(
	    Is( rig.Drag( T, 10, 10, 11, 12 ), ToolResultKind::CaptureReleased ) && !tool->Pending(),
	    "a click makes no box" );
	checks.That( Is( rig.Key( T, tools::Key::Enter ), ToolResultKind::Ignored ),
	    "Enter without a box: ignored" );
	checks.That( Is( rig.Down( T, 3, 5 ), ToolResultKind::CaptureRequested ), "press captures" );
	rig.Move( T, 30, 40 );
	rig.Move( T, 60, 70 );
	checks.Equal(
	    rig.Overlay( T ).Count( tools::OverlayKind::WorldBox, tools::OverlayRole::Pending ),
	    std::size_t( 1 ), "the box previews while dragging" );
	checks.Equal( rig.CursorAt( T, rig.Px( T, 60, 70 ) ), tools::Cursor::Crosshair,
	    "crosshair while drawing" );
	rig.Up( T, 60, 70 );
	checks.That( pending() == MakeBox( Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 16 ) ),
	    "snapped corners; default depth one grid step" );
	checks.Equal( rig.HistorySize(), std::size_t( 0 ), "a pending box is not an edit" );
	checks.Equal( tool->Status(), std::string( "Block 64 x 64 x 16" ), "status shows the size" );
	{
		const tools::OverlayList overlay = rig.Overlay( T );
		checks.Equal( overlay.Count( tools::OverlayKind::ScreenHandle, tools::OverlayRole::Handle ),
		    std::size_t( 8 ), "8 handles on the pending box" );
		checks.Equal( overlay.Count( tools::OverlayKind::ScreenLabel, tools::OverlayRole::Pending ),
		    std::size_t( 1 ), "size label" );
		checks.Equal( rig.Overlay( ViewKind::Front )
		                  .Count( tools::OverlayKind::WorldBox, tools::OverlayRole::Pending ),
		    std::size_t( 1 ), "other views show the box" );
	}
	// Resize with the E handle.
	{
		const viewport::ScreenPoint e = rig.Px( T, 64, 32 );
		const viewport::ScreenPoint eh{ e.x + 6, e.y };
		checks.Equal( rig.CursorAt( T, eh ), tools::Cursor::ResizeE, "E handle cursor" );
		rig.Pointer( T, PointerPhase::Down, eh );
		rig.Pointer( T, PointerPhase::Move, { eh.x + 30, eh.y + 2 } );
		rig.Pointer( T, PointerPhase::Up, { eh.x + 30, eh.y + 2 } );
		checks.That( pending() == MakeBox( Vec3d( 0, 0, 0 ), Vec3d( 96, 64, 16 ) ),
		    "E handle snaps the right edge" );
	}
	// Move inside; Shift constrains.
	checks.Equal(
	    rig.CursorAt( T, rig.Px( T, 48, 32 ) ), tools::Cursor::Move, "move cursor inside the box" );
	rig.Drag( T, 48, 32, 68, 35 );
	checks.That( pending() == MakeBox( Vec3d( 16, 0, 0 ), Vec3d( 112, 64, 16 ) ),
	    "moved by the snapped delta" );
	rig.Drag( T, 48, 32, 50, 60, Mods( tools::kShift ) );
	checks.That( pending() == MakeBox( Vec3d( 16, 32, 0 ), Vec3d( 112, 96, 16 ) ),
	    "Shift keeps the dominant axis" );
	// Escape and focus loss mid-drag restore the box.
	{
		const scene::Box before = pending();
		rig.Down( T, 48, 60 );
		rig.Move( T, 200, 200 );
		checks.That( Is( rig.Key( T, tools::Key::Escape ), ToolResultKind::CaptureReleased ),
		    "Escape releases" );
		checks.That( pending() == before && !rig.manager.HasCapture(), "Escape restores the box" );
		rig.Down( T, 300, 300 );
		rig.Move( T, 350, 350 );
		rig.manager.OnFocusLost();
		checks.That( pending() == before && !tool->InGesture(), "focus loss restores the box" );
	}
	// A flat drag is an Error preview and leaves the box alone.
	{
		const scene::Box before = pending();
		rig.Down( T, -200, -200 );
		rig.Move( T, -150, -205 );
		checks.That( rig.Overlay( T ).CountRole( tools::OverlayRole::Error ) == 1,
		    "a flat box previews as Error" );
		rig.Up( T, -150, -205 );
		checks.That( pending() == before, "a flat drag keeps the previous box" );
	}
	// Enter commits.
	{
		checks.That(
		    Is( rig.Key( T, tools::Key::Enter ), ToolResultKind::Handled ), "Enter commits" );
		checks.Equal( rig.HistorySize(), std::size_t( 1 ), "one history unit" );
		checks.Equal( rig.LastLabel(), std::string( "Create block" ), "labeled" );
		const app::Selection &sel = rig.session.CurrentSelection();
		checks.That(
		    sel.objects.size() == 1 && sel.objects[0] != floor, "the new solid is selected" );
		checks.That( BoundsOf( rig.session, sel.objects[0] ) ==
		                 MakeBox( Vec3d( 16, 32, 0 ), Vec3d( 112, 96, 16 ) ),
		    "the solid fills the box" );
		checks.That( !tool->Pending(), "pending box cleared" );
	}
	// The next box takes its depth from the previous one; Front draws along Y depth.
	{
		const ViewKind F = ViewKind::Front;
		rig.Drag( F, 0, 0, 32, 48 );
		checks.That( pending() == MakeBox( Vec3d( 0, 32, 0 ), Vec3d( 32, 96, 48 ) ),
		    "Front box: x/z from the drag, y from the previous box" );
		rig.Key( F, tools::Key::Enter );
		const scene::ObjectId id = rig.session.CurrentSelection().objects[0];
		checks.That(
		    BoundsOf( rig.session, id ) == MakeBox( Vec3d( 0, 32, 0 ), Vec3d( 32, 96, 48 ) ),
		    "Front block created" );
		checks.Equal( rig.HistorySize(), std::size_t( 2 ), "two units" );
	}
	// Escape without a drag discards; Enter then does nothing.
	rig.Drag( T, 200, 200, 264, 264 );
	checks.That( tool->Pending().has_value(), "a box" );
	checks.That(
	    Is( rig.Key( T, tools::Key::Escape ), ToolResultKind::Handled ) && !tool->Pending(),
	    "Escape discards" );
	checks.That(
	    Is( rig.Key( T, tools::Key::Enter ), ToolResultKind::Ignored ) && rig.HistorySize() == 2,
	    "nothing to commit" );
	// A refused primitive fails with no edit and keeps the box.
	{
		rig.Drag( T, 200, 200, 264, 264 );
		rig.editor.primitive.kind = app::ops::PrimitiveKind::Cylinder;
		rig.editor.primitive.sides = 2;
		const auto r = rig.Key( T, tools::Key::Enter );
		checks.That( Is( r, ToolResultKind::Failed ) && !r.message.empty(),
		    "bad side count fails with a reason" );
		checks.That(
		    rig.HistorySize() == 2 && tool->Pending().has_value(), "no edit; the box stays" );
		rig.editor.primitive.sides = 8;
		checks.That(
		    Is( rig.Key( T, tools::Key::Enter ), ToolResultKind::Handled ), "a cylinder commits" );
		checks.Equal( rig.LastLabel(), std::string( "Create primitive" ), "primitive label" );
		const scene::ObjectId id = rig.session.CurrentSelection().objects[0];
		checks.That( rig.session.Document().FindSolid( id ) &&
		                 rig.session.Document().FindSolid( id )->sides.size() == 10,
		    "an 8-sided prism along Z" );
		rig.editor.primitive.kind = app::ops::PrimitiveKind::Block;
	}
	// Arch.
	{
		rig.tool.makeArch = true;
		rig.Drag( T, -300, -300, -172, -172 );
		checks.That(
		    Is( rig.Key( T, tools::Key::Enter ), ToolResultKind::Handled ), "arch commits" );
		checks.Equal( rig.LastLabel(), std::string( "Create arch" ), "arch label" );
		const app::Selection &sel = rig.session.CurrentSelection();
		checks.That( sel.objects.size() == 1 && rig.session.Document().FindGroup( sel.objects[0] ),
		    "the arch group is selected" );
		rig.tool.makeArch = false;
	}
	// Switching tools discards the pending box.
	rig.Drag( T, 200, 200, 264, 264 );
	checks.That( rig.manager.Activate( tools::SelectionTool::kName ) &&
	                 rig.manager.Activate( tools::BlockTool::kName ),
	    "switch away and back" );
	checks.That( !tool->Pending(), "tool switch discards the pending box" );

	// --- Selection bounds depth rule (fresh tool, no previous box) ----------------------------
	{
		DocBuilder b2;
		const scene::ObjectId s = b2.Box( Vec3d( 0, 0, 32 ), Vec3d( 64, 64, 96 ) );
		Rig r2( b2.doc );
		tools::BlockTool *t2 = r2.Use( std::make_unique<tools::BlockTool>() );
		checks.That(
		    r2.session.SelectObjects( { s }, app::SelectMode::Replace ).HasValue(), "select" );
		r2.Drag( T, 100, 0, 164, 64 );
		checks.That( t2->Pending() &&
		                 t2->Pending()->box == MakeBox( Vec3d( 96, 0, 32 ), Vec3d( 160, 64, 96 ) ),
		    "depth from the selection bounds" );
	}

	// --- 3D (a fresh rig: only the floor) ------------------------------------------------------
	{
		DocBuilder b3;
		b3.Box( Vec3d( -512, -512, -16 ), Vec3d( 512, 512, 0 ) );
		Rig r3( b3.doc );
		tool = r3.Use( std::make_unique<tools::BlockTool>() );
		Rig &rig = r3;
		rig.camera.SetPosition( Vec3d( 0, -512, 256 ) );
		rig.camera.SetAngles( 90, 30 );
		const ViewKind C = ViewKind::Camera3D;
		const viewport::ScreenPoint p0{ 350, 380 };
		const viewport::ScreenPoint p1{ 470, 300 };
		const std::size_t history = rig.HistorySize();
		checks.That(
		    Is( rig.Pointer( C, PointerPhase::Down, p0 ), ToolResultKind::CaptureRequested ),
		    "base press" );
		rig.Pointer( C, PointerPhase::Move, p1 );
		rig.Pointer( C, PointerPhase::Up, p1 );
		const Vec3d a = FloorPoint( rig.camera, p0 );
		const Vec3d c = FloorPoint( rig.camera, p1 );
		const scene::Box expected{ Vec3d( std::min( a.x, c.x ), std::min( a.y, c.y ), 0 ),
		    Vec3d( std::max( a.x, c.x ), std::max( a.y, c.y ), 16 ) };
		checks.That( tool->Pending() && pending() == expected,
		    "base on the floor's workplane, one grid step tall" );
		checks.Equal( tool->Pending()->axis, 2, "height axis Z" );
		// Height drag on the box.
		const viewport::ScreenPoint center = *rig.camera.WorldToScreen( expected.Center() );
		rig.Pointer( C, PointerPhase::Down, center );
		checks.Equal( rig.CursorAt( C, center ), tools::Cursor::ResizeN, "height cursor" );
		rig.Pointer( C, PointerPhase::Move, { center.x, center.y - 60 } );
		rig.Pointer( C, PointerPhase::Up, { center.x, center.y - 60 } );
		const scene::Box raised = pending();
		checks.That( raised.mins == expected.mins && raised.maxs.x == expected.maxs.x &&
		                 raised.maxs.y == expected.maxs.y,
		    "height drag keeps the base" );
		checks.That( raised.maxs.z > 16 && std::fmod( raised.maxs.z, 16.0 ) == 0.0,
		    "height grows on the grid" );
		checks.That(
		    Is( rig.Key( C, tools::Key::Enter ), ToolResultKind::Handled ), "3D Enter commits" );
		checks.That( BoundsOf( rig.session, rig.session.CurrentSelection().objects[0] ) == raised &&
		                 rig.HistorySize() == history + 1,
		    "one block from the 3D box" );
		// Looking up at the sky: no workplane.
		rig.camera.SetPosition( Vec3d( 0, 0, 100 ) );
		rig.camera.SetAngles( 90, -45 );
		const auto r = rig.Pointer( C, PointerPhase::Down, { 400, 300 } );
		checks.That(
		    Is( r, ToolResultKind::Failed ) && !rig.manager.HasCapture() && !tool->InGesture(),
		    "no workplane: the press fails" );
	}
	checks.That( rig.session.Document().Validate().empty(), "document valid" );
	checks.That( Is( rig.Down( T, 0, 0, {}, PointerButton::Middle ), ToolResultKind::Ignored ),
	    "middle press ignored" );

	return checks.Report();
}
