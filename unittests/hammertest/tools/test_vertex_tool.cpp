//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.tools Vertex tool (tools.vertex_tool.v1): vertex and edge
//			handles of the selected solids; a 2D click selects every handle
//			stacked at the spot, Ctrl toggles, a click on nothing clears; a drag
//			moves the selected handles snapped (the grabbed handle lands on the
//			grid) with a RebuildFromVertices preview, committing MoveVertices for
//			every affected solid as ONE "Move vertices" unit and keeping the
//			moved handles selected; an edge handle moves both vertices.
//			Negative checks: flattening and interior-corner moves preview as Error and return Failed
//			with no edit; Escape and focus loss restore the handle selection
//			with no edit; an external change drops the handle selection; 3D
//			drags make no edit.
//
//=============================================================================//

#include "hammer/app/ops/vertex_ops.h"
#include "hammer/tools/vertex_tool.h"
#include "testing/checks.h"
#include "tool_test_util.h"

#include <algorithm>

using namespace tooltest;
using tools::ToolResultKind;

namespace
{

bool HasVertex( const app::EditSession &s, scene::ObjectId id, Vec3d v )
{
	const std::vector<Vec3d> list = app::ops::SolidVertexList( *s.Document().FindSolid( id ) );
	return std::find( list.begin(), list.end(), v ) != list.end();
}

} // namespace

int main()
{
	testing::Checks checks;
	const ViewKind T = ViewKind::Top;

	DocBuilder b;
	const scene::ObjectId A = b.Box( Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) );
	const scene::ObjectId B = b.Box( Vec3d( 128, 0, 0 ), Vec3d( 192, 64, 64 ) );
	Rig rig( b.doc );
	tools::VertexTool *tool = rig.Use( std::make_unique<tools::VertexTool>() );
	checks.That( rig.session.SelectObjects( { A, B }, app::SelectMode::Replace ).HasValue(),
	    "select A and B" );
	const auto handles = [&]()
	{
		return tool->SelectedHandles( rig.Ctx( T ) );
	};

	checks.Equal( tools::VertexTool::Handles( rig.Ctx( T ) ).size(), std::size_t( 40 ),
	    "8 vertices and 12 edge midpoints per box" );
	checks.Equal(
	    rig.Overlay( T ).Count( tools::OverlayKind::ScreenHandle, tools::OverlayRole::Handle ),
	    std::size_t( 40 ), "every handle drawn" );

	// --- Click selection ------------------------------------------------------------------
	checks.That( Is( rig.Drag( T, 64, 64, 64, 64 ), ToolResultKind::CaptureReleased ), "click" );
	checks.Equal( handles().size(), std::size_t( 2 ), "the two vertices stacked at the corner" );
	checks.Equal(
	    rig.Overlay( T ).Count( tools::OverlayKind::ScreenHandle, tools::OverlayRole::Selection ),
	    std::size_t( 2 ), "selected handles drawn as Selection" );
	checks.Equal(
	    rig.CursorAt( T, rig.Px( T, 64, 64 ) ), tools::Cursor::Move, "move cursor over a handle" );
	rig.Drag( T, 64, 0, 64, 0, Mods( tools::kCtrl ) );
	checks.Equal( handles().size(), std::size_t( 4 ), "Ctrl adds" );
	rig.Drag( T, 64, 0, 64, 0, Mods( tools::kCtrl ) );
	checks.Equal( handles().size(), std::size_t( 2 ), "Ctrl toggles off" );
	rig.Drag( T, 300, 300, 300, 300, Mods( tools::kCtrl ) );
	checks.Equal( handles().size(), std::size_t( 2 ), "Ctrl click on nothing keeps" );
	checks.Equal( rig.HistorySize(), std::size_t( 0 ), "selecting handles is not an edit" );

	// --- Drag -------------------------------------------------------------------------------
	{
		rig.Down( T, 64, 64 );
		rig.Move( T, 74, 66 );
		rig.Move( T, 84, 69 );
		checks.That( rig.Overlay( T ).Count(
		                 tools::OverlayKind::WorldLine, tools::OverlayRole::Pending ) > 0 &&
		                 rig.Overlay( T ).CountRole( tools::OverlayRole::Error ) == 0,
		    "valid move previews as Pending" );
		checks.That( HasVertex( rig.session, A, Vec3d( 64, 64, 0 ) ), "no change during the drag" );
		const auto up = rig.Up( T, 84, 69 );
		checks.That( Is( up, ToolResultKind::CaptureReleased ), "release commits" );
		checks.That( HasVertex( rig.session, A, Vec3d( 80, 64, 0 ) ) &&
		                 HasVertex( rig.session, A, Vec3d( 80, 64, 64 ) ),
		    "both stacked vertices moved by the snapped (16, 0)" );
		checks.Equal( rig.HistorySize(), std::size_t( 1 ), "one unit" );
		checks.Equal( rig.LastLabel(), std::string( "Move vertices" ), "labeled" );
		checks.Equal( handles().size(), std::size_t( 2 ), "moved handles stay selected" );
	}
	// A move that flattens a solid: Error preview, Failed, no edit.
	{
		rig.Drag( T, 192, 0, 192, 0 );
		rig.Drag( T, 192, 64, 192, 64, Mods( tools::kCtrl ) );
		checks.Equal( handles().size(), std::size_t( 4 ), "B's right corners" );
		rig.Down( T, 192, 0 );
		rig.Move( T, 128, 0 );
		checks.That( rig.Overlay( T ).CountRole( tools::OverlayRole::Error ) > 0,
		    "a flattening move previews as Error" );
		const auto up = rig.Up( T, 128, 0 );
		checks.That(
		    Is( up, ToolResultKind::Failed ) && !up.message.empty(), "it fails with a reason" );
		checks.That( rig.HistorySize() == 1 && HasVertex( rig.session, B, Vec3d( 192, 0, 0 ) ) &&
		                 !rig.manager.HasCapture(),
		    "no edit" );
	}
	// A corner dragged into the solid's interior (onto the top and bottom
	// faces) would be absorbed: refused, not silently repaired.
	{
		rig.Drag( T, 0, 0, 0, 0 );
		checks.Equal( handles().size(), std::size_t( 2 ), "A's stacked (0, 0) corner" );
		rig.Down( T, 0, 0 );
		rig.Move( T, 48, 48 );
		checks.That( rig.Overlay( T ).CountRole( tools::OverlayRole::Error ) > 0,
		    "an interior corner previews as Error" );
		const auto up = rig.Up( T, 48, 48 );
		checks.That(
		    Is( up, ToolResultKind::Failed ) && !up.message.empty(), "an interior corner fails" );
		checks.That(
		    rig.HistorySize() == 1 && HasVertex( rig.session, A, Vec3d( 0, 0, 0 ) ), "no edit" );
	}
	// Two solids in one unit.
	{
		rig.Drag( T, 0, 64, 0, 64 );
		rig.Drag( T, 192, 64, 192, 64, Mods( tools::kCtrl ) );
		checks.Equal( handles().size(), std::size_t( 4 ), "handles on both solids" );
		rig.Drag( T, 0, 64, 0, 80 );
		checks.That( HasVertex( rig.session, A, Vec3d( 0, 80, 0 ) ) &&
		                 HasVertex( rig.session, B, Vec3d( 192, 80, 64 ) ),
		    "both solids moved" );
		checks.Equal( rig.HistorySize(), std::size_t( 2 ), "one unit for both" );
		checks.That( rig.session.Undo().HasValue(), "undo" );
		checks.That( HasVertex( rig.session, A, Vec3d( 0, 64, 0 ) ) &&
		                 HasVertex( rig.session, B, Vec3d( 192, 64, 64 ) ),
		    "one undo reverts both" );
		checks.That( handles().empty(), "an external change (undo) drops the handle selection" );
	}
	// Edge handle moves both vertices.
	{
		rig.Drag( T, 160, 0, 160, 0 );
		checks.Equal( handles().size(), std::size_t( 2 ), "the two stacked bottom edges" );
		rig.Drag( T, 160, 0, 160, -16 );
		const std::optional<scene::Box> bb = BoundsOf( rig.session, B );
		checks.That( bb && bb->mins == Vec3d( 128, -16, 0 ) && bb->maxs == Vec3d( 192, 64, 64 ),
		    "the -y face moved out by one grid step" );
	}
	// Cancellation restores the handle selection.
	{
		const std::uint64_t revision = rig.session.Revision();
		const auto before = handles();
		rig.Down( T, 0, 0 ); // unselected: becomes the selection during the press
		rig.Move( T, 30, 30 );
		checks.That( handles() != before, "pressing unselected handles selects them" );
		checks.That( Is( rig.Key( T, tools::Key::Escape ), ToolResultKind::CaptureReleased ),
		    "Escape releases" );
		checks.That(
		    handles() == before && rig.session.Revision() == revision, "Escape restores, no edit" );
		rig.Down( T, 0, 0 );
		rig.Move( T, 30, 30 );
		rig.manager.OnFocusLost();
		checks.That( handles() == before && rig.session.Revision() == revision,
		    "focus loss restores, no edit" );
	}
	rig.Drag( T, 300, 300, 300, 300 );
	checks.That( handles().empty(), "a click on nothing clears" );
	rig.Drag( T, 64, 0, 64, 0 );
	checks.That(
	    Is( rig.Key( T, tools::Key::Escape ), ToolResultKind::Handled ) && handles().empty(),
	    "Escape clears the handle selection" );
	checks.That( Is( rig.Key( T, tools::Key::Escape ), ToolResultKind::Ignored ),
	    "Escape with nothing: ignored" );

	// --- 3D -----------------------------------------------------------------------------------
	{
		rig.camera.SetPosition( Vec3d( 32, -200, 32 ) );
		rig.camera.SetAngles( 90, 0 );
		const viewport::ScreenPoint corner = *rig.camera.WorldToScreen( Vec3d( 0, 0, 64 ) );
		rig.Pointer( ViewKind::Camera3D, PointerPhase::Down, corner );
		rig.Pointer( ViewKind::Camera3D, PointerPhase::Up, corner );
		const auto h = tool->SelectedHandles( rig.Ctx( ViewKind::Camera3D ) );
		checks.That( h.size() == 1 && !h[0].edge && h[0].solid == A, "3D click picks the vertex" );
		const std::uint64_t revision = rig.session.Revision();
		rig.Pointer( ViewKind::Camera3D, PointerPhase::Down, corner );
		rig.Pointer( ViewKind::Camera3D, PointerPhase::Move, { corner.x + 40, corner.y } );
		rig.Pointer( ViewKind::Camera3D, PointerPhase::Up, { corner.x + 40, corner.y } );
		checks.Equal( rig.session.Revision(), revision, "3D drags make no edit" );
	}
	checks.That( Is( rig.Down( T, 0, 0, {}, PointerButton::Right ), ToolResultKind::Ignored ),
	    "right press ignored" );
	checks.That( rig.session.Document().Validate().empty(), "document valid" );

	return checks.Report();
}
