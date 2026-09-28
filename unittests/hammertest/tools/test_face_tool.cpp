//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.tools Face tool (tools.face_tool.v1): 3D left click selects
//			the ray-hit face (Ctrl toggles, Shift takes the solid's faces, a
//			click on nothing clears); right click applies the active material
//			("Apply material"); Alt+right applies with the first selected face's
//			alignment ("Apply texture"); Alt+left lifts the face texture through
//			the getter and the callback. Negative checks: right click on
//			nothing, no active material and Alt+right without a selected face
//			return Failed with no edit; a press that drags is abandoned with no
//			effect; 2D views are not a supported context.
//
//=============================================================================//

#include "hammer/app/ops/texture_ops.h"
#include "hammer/tools/face_tool.h"
#include "testing/checks.h"
#include "tool_test_util.h"

using namespace tooltest;
using tools::ToolResultKind;

namespace
{

const scene::Side *FaceWithNormal( const app::EditSession &s, scene::ObjectId id, Vec3d normal )
{
	for ( const scene::Side &side : s.Document().FindSolid( id )->sides )
		if ( mapgeometry::NearlyEqual( side.Plane().normal, normal, 1e-9 ) )
			return &side;
	return nullptr;
}

} // namespace

int main()
{
	testing::Checks checks;
	const ViewKind V = ViewKind::Camera3D;
	const viewport::ScreenPoint center{ 400, 300 };
	const viewport::ScreenPoint sky{ 5, 5 };

	DocBuilder b;
	const scene::ObjectId A = b.Box( Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) );
	scene::ObjectId B;
	{
		scene::FaceTexture brick = DevTexture();
		brick.material = "BRICK/BRICKWALL001";
		brick.u.scale = 0.5;
		brick.v.scale = 0.5;
		brick.u.shift = 5;
		brick.lightmapScale = 8;
		scene::DocumentEdit edit( b.doc );
		scene::Solid solid =
		    scene::MakeBoxSolid( { Vec3d( 128, 0, 0 ), Vec3d( 192, 64, 64 ) }, brick );
		for ( scene::Side &side : solid.sides )
			side.texture.u.shift = 5; // MakeBoxSolid resets shifts
		B = edit.Add( solid );
		scene::CommitEdit( b.doc, edit );
	}
	Rig rig( b.doc );
	tools::FaceTool *tool = rig.Use( std::make_unique<tools::FaceTool>() );
	std::optional<scene::FaceTexture> lifted;
	tool->SetLiftCallback(
	    [&]( const scene::FaceTexture &t )
	    {
		    lifted = t;
	    } );
	const auto lookAt = [&]( double x )
	{
		rig.camera.SetPosition( Vec3d( x, -200, 32 ) );
	};
	rig.camera.SetAngles( 90, 0 );
	lookAt( 32 );
	const auto click = [&]( viewport::ScreenPoint p, tools::Modifiers m = {},
	                       PointerButton button = PointerButton::Left )
	{
		rig.Pointer( V, PointerPhase::Down, p, m, button );
		return rig.Pointer( V, PointerPhase::Up, p, m, button );
	};
	const auto faces = [&]()
	{
		return rig.session.CurrentSelection().faces;
	};
	const scene::Side *aFront = FaceWithNormal( rig.session, A, Vec3d( 0, -1, 0 ) );
	const scene::FaceRef aFace{ A, aFront->vmfId };

	// --- Selection ------------------------------------------------------------------------
	checks.That(
	    Is( rig.Pointer( V, PointerPhase::Down, center ), ToolResultKind::CaptureRequested ),
	    "press captures" );
	checks.That( faces().empty(), "nothing selected on press" );
	checks.That( Is( rig.Pointer( V, PointerPhase::Up, center ), ToolResultKind::CaptureReleased ),
	    "release" );
	checks.That( faces() == std::vector<scene::FaceRef>{ aFace }, "click selects the hit face" );
	checks.Equal( rig.HistorySize(), std::size_t( 0 ), "no history" );
	checks.Equal(
	    rig.Overlay( V ).Count( tools::OverlayKind::WorldPolygon, tools::OverlayRole::Selection ),
	    std::size_t( 1 ), "the face polygon is drawn" );
	click( center, Mods( tools::kCtrl ) );
	checks.That( faces().empty(), "Ctrl toggles it off" );
	click( center, Mods( tools::kShift ) );
	checks.Equal( faces().size(), std::size_t( 6 ), "Shift takes every face of the solid" );
	click( sky );
	checks.That( faces().empty(), "a click on nothing clears" );
	checks.Equal( rig.CursorAt( V, center ), tools::Cursor::Hand, "hand over a face" );
	checks.Equal( rig.CursorAt( V, sky ), tools::Cursor::Default, "default over nothing" );

	// --- Apply --------------------------------------------------------------------------------
	rig.editor.faceTexture.material = "TOOLS/TOOLSNODRAW";
	checks.That( Is( click( center, {}, PointerButton::Right ), ToolResultKind::CaptureReleased ),
	    "right click applies" );
	checks.Equal( app::ops::FindFace( rig.session.Document(), aFace )->texture.material,
	    std::string( "TOOLS/TOOLSNODRAW" ), "the hit face has the active material" );
	checks.Equal( FaceWithNormal( rig.session, A, Vec3d( 0, 0, 1 ) )->texture.material,
	    std::string( "DEV/DEV_MEASUREGENERIC01B" ), "other faces untouched" );
	checks.Equal( rig.HistorySize(), std::size_t( 1 ), "one unit" );
	checks.Equal( rig.LastLabel(), std::string( "Apply material" ), "labeled" );

	// Alt+right: alignment from the first selected face (B's), active material.
	{
		lookAt( 160 );
		click( center );
		const scene::FaceRef bFace = faces().at( 0 );
		checks.That( bFace.solid == B, "B's face selected" );
		lookAt( 32 );
		checks.That( Is( click( center, Mods( tools::kAlt ), PointerButton::Right ),
		                 ToolResultKind::CaptureReleased ),
		    "Alt+right applies with alignment" );
		const scene::FaceTexture &t = app::ops::FindFace( rig.session.Document(), aFace )->texture;
		checks.That( t.material == "TOOLS/TOOLSNODRAW" && t.u.scale == 0.5 && t.u.shift == 5 &&
		                 t.lightmapScale == 8,
		    "values from B's face, the active material" );
		checks.Equal( rig.LastLabel(), std::string( "Apply texture" ), "labeled" );
		checks.Equal( rig.HistorySize(), std::size_t( 2 ), "one unit" );
	}
	// Lift.
	{
		lookAt( 160 );
		checks.That( Is( click( center, Mods( tools::kAlt ) ), ToolResultKind::CaptureReleased ),
		    "Alt+left lifts" );
		checks.That(
		    tool->LiftedTexture() && tool->LiftedTexture()->material == "BRICK/BRICKWALL001",
		    "lifted getter" );
		checks.That( lifted && *lifted == *tool->LiftedTexture(), "lift callback" );
		checks.Equal( rig.editor.faceTexture.material, std::string( "TOOLS/TOOLSNODRAW" ),
		    "the tool never writes the settings" );
		checks.Equal( rig.HistorySize(), std::size_t( 2 ), "lifting is not an edit" );
	}

	// --- Failures and cancellation ----------------------------------------------------------------
	{
		const std::uint64_t revision = rig.session.Revision();
		auto r = click( sky, {}, PointerButton::Right );
		checks.That( Is( r, ToolResultKind::Failed ) && r.message == "no face under the pointer",
		    "right click on nothing" );
		rig.editor.faceTexture.material.clear();
		r = click( center, {}, PointerButton::Right );
		checks.That( Is( r, ToolResultKind::Failed ) && r.message == "no active material",
		    "no active material" );
		rig.editor.faceTexture.material = "TOOLS/TOOLSNODRAW";
		checks.That(
		    rig.session.SelectFaces( {}, app::SelectMode::Replace ).HasValue(), "clear faces" );
		r = click( center, Mods( tools::kAlt ), PointerButton::Right );
		checks.That( Is( r, ToolResultKind::Failed ) && !r.message.empty(),
		    "Alt+right without a selected face" );
		rig.Pointer( V, PointerPhase::Down, center, {}, PointerButton::Right );
		r = rig.Pointer( V, PointerPhase::Move, { center.x + 20, center.y } );
		checks.That( Is( r, ToolResultKind::CaptureReleased ) && !rig.manager.HasCapture(),
		    "a drag gives the pointer back" );
		checks.That( Is( rig.Pointer( V, PointerPhase::Up, { center.x + 20, center.y }, {},
		                     PointerButton::Right ),
		                 ToolResultKind::Ignored ),
		    "its release is ignored" );
		rig.Pointer( V, PointerPhase::Down, center, {}, PointerButton::Right );
		checks.That( Is( rig.Key( V, tools::Key::Escape ), ToolResultKind::CaptureReleased ),
		    "Escape cancels a press" );
		rig.Pointer( V, PointerPhase::Up, center, {}, PointerButton::Right );
		rig.Pointer( V, PointerPhase::Down, center, {}, PointerButton::Right );
		rig.manager.OnFocusLost();
		rig.Pointer( V, PointerPhase::Up, center, {}, PointerButton::Right );
		checks.Equal( rig.session.Revision(), revision, "no failure or cancellation made an edit" );
	}
	click( center );
	checks.That( Is( rig.Key( V, tools::Key::Escape ), ToolResultKind::Handled ) && faces().empty(),
	    "Escape clears the face selection" );
	checks.That( Is( rig.Key( V, tools::Key::Escape ), ToolResultKind::Ignored ),
	    "Escape with nothing: ignored" );
	checks.That(
	    Is( rig.Down( ViewKind::Top, 0, 0 ), ToolResultKind::Ignored ), "2D press ignored" );
	checks.Equal( rig.CursorAt( ViewKind::Top, { 400, 300 } ), tools::Cursor::Forbidden,
	    "2D cursor forbidden" );
	checks.That( Is( rig.Pointer( V, PointerPhase::Down, center, {}, PointerButton::Middle ),
	                 ToolResultKind::Ignored ),
	    "middle press ignored" );
	checks.That( rig.session.Document().Validate().empty(), "document valid" );

	return checks.Report();
}
