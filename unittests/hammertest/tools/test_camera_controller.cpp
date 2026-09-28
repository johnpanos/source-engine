//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.tools navigation (tools.camera_controller.v1): 2D pan
//			(Middle, Space+Left), wheel zoom about the cursor, keyboard zoom;
//			3D fly keys with Advance and Shift speed, look (armed Right press,
//			Space+Left), orbit about the pivot (Alt+Left), Middle pan, wheel
//			dolly; framing the selection; the host arbitration protocol with a
//			tool (a right click reaches the Face tool, a right drag looks and
//			cancels the tool's press with no edit). Negative checks: plain Left
//			presses are Ignored, focus loss releases keys and drags, framing
//			nothing changes nothing, the document is never edited.
//
//=============================================================================//

#include "hammer/tools/camera_controller.h"
#include "hammer/tools/face_tool.h"
#include "testing/checks.h"
#include "tool_test_util.h"

#include <cmath>

using namespace tooltest;
using tools::ToolResultKind;

namespace
{

tools::PointerEvent Ev( ViewKind view, PointerPhase phase, double x, double y, PointerButton button,
    tools::Modifiers mods = {} )
{
	tools::PointerEvent e;
	e.view = view;
	e.phase = phase;
	e.x = x;
	e.y = y;
	e.button = button;
	e.modifiers = mods;
	return e;
}

tools::WheelEvent Wheel( ViewKind view, double steps, double x, double y )
{
	tools::WheelEvent e;
	e.view = view;
	e.steps = steps;
	e.x = x;
	e.y = y;
	return e;
}

} // namespace

int main()
{
	testing::Checks checks;
	const ViewKind T = ViewKind::Top;
	const ViewKind V = ViewKind::Camera3D;
	const PointerButton L = PointerButton::Left;
	const PointerButton M = PointerButton::Middle;
	const PointerButton R = PointerButton::Right;

	DocBuilder b;
	const scene::ObjectId A = b.Box( Vec3d( 1000, 1000, 0 ), Vec3d( 1064, 1064, 64 ) );
	const scene::ObjectId wall = b.Box( Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) );
	Rig rig( b.doc );
	tools::CameraController nav;
	viewport::Camera2D &top = rig.top;
	viewport::Camera3D &cam = rig.camera;

	// --- 2D ---------------------------------------------------------------------------------
	checks.That( Is( nav.OnPointer( top, Ev( T, PointerPhase::Down, 400, 300, M ) ),
	                 ToolResultKind::CaptureRequested ) &&
	                 nav.HasCapture(),
	    "middle press captures" );
	nav.OnPointer( top, Ev( T, PointerPhase::Move, 405, 310, M ) );
	checks.That( Is( nav.OnPointer( top, Ev( T, PointerPhase::Up, 410, 320, M ) ),
	                 ToolResultKind::CaptureReleased ),
	    "release" );
	checks.That( top.Center() == viewport::PlanePoint{ -10, 20 },
	    "the content follows the pointer (10, 20) px" );
	checks.That( Is( nav.OnPointer( top, Ev( T, PointerPhase::Down, 400, 300, L ) ),
	                 ToolResultKind::Ignored ),
	    "plain left press is the tool's" );
	checks.That( Is( nav.OnKey( top, tools::KeyEvent::Press( tools::Key::Space ) ),
	                 ToolResultKind::Handled ),
	    "space held" );
	nav.OnPointer( top, Ev( T, PointerPhase::Down, 400, 300, L ) );
	nav.OnPointer( top, Ev( T, PointerPhase::Up, 390, 300, L ) );
	checks.That( top.Center() == viewport::PlanePoint{ 0, 20 }, "Space + Left pans" );
	nav.OnKey( top, tools::KeyEvent::Release( tools::Key::Space ) );
	checks.That( Is( nav.OnPointer( top, Ev( T, PointerPhase::Down, 400, 300, L ) ),
	                 ToolResultKind::Ignored ),
	    "space released: left is the tool's again" );
	{
		const viewport::PlanePoint under = top.ScreenToPlane( 500, 200 );
		checks.That( Is( nav.OnWheel( top, Wheel( T, 1, 500, 200 ) ), ToolResultKind::Handled ),
		    "wheel zooms" );
		checks.Near( top.Zoom(), 1.25, 1e-12, "one notch" );
		const viewport::PlanePoint after = top.ScreenToPlane( 500, 200 );
		checks.That( std::fabs( after.u - under.u ) < 1e-9 && std::fabs( after.v - under.v ) < 1e-9,
		    "the point under the cursor stays put" );
		nav.OnKey( top, tools::KeyEvent::Char( '=' ) );
		checks.Near( top.Zoom(), 2.5, 1e-12, "'=' doubles the zoom" );
		nav.OnKey( top, tools::KeyEvent::Char( '-' ) );
		checks.Near( top.Zoom(), 1.25, 1e-12, "'-' halves it" );
	}

	// --- 3D fly ----------------------------------------------------------------------------------
	cam.SetPosition( Vec3d( 0, -500, 32 ) );
	cam.SetAngles( 90, 0 );
	checks.That( !nav.Advance( cam, 0.5 ), "no keys: no motion" );
	checks.That(
	    Is( nav.OnKey( tools::KeyEvent::Char( 'w' ) ), ToolResultKind::Handled ) && nav.Flying(),
	    "W held" );
	checks.That( nav.Advance( cam, 0.5 ), "moves" );
	checks.That( mapgeometry::NearlyEqual( cam.Position(), Vec3d( 0, -244, 32 ), 1e-9 ),
	    "forward 256 in half a second" );
	nav.OnKey( tools::KeyEvent::Char( 'e', Mods( tools::kShift ) ) );
	nav.Advance( cam, 0.25 );
	checks.That( mapgeometry::NearlyEqual( cam.Position(), Vec3d( 0, 268, 544 ), 1e-9 ),
	    "Shift: four times, E climbs" );
	nav.OnKey( tools::KeyEvent::Char( 'w', {}, tools::KeyPhase::Release ) );
	nav.OnKey( tools::KeyEvent::Char( 'e', {}, tools::KeyPhase::Release ) );
	checks.That( !nav.Flying() && !nav.Advance( cam, 1 ), "released" );
	checks.That( Is( nav.OnKey( tools::KeyEvent::Char( 'z' ) ), ToolResultKind::Ignored ),
	    "other keys ignored" );

	// Wheel dolly.
	cam.SetPosition( Vec3d( 0, -500, 32 ) );
	nav.OnWheel( cam, Wheel( V, 2, 400, 300 ) );
	checks.That( mapgeometry::NearlyEqual( cam.Position(), Vec3d( 0, -372, 32 ), 1e-9 ),
	    "two notches dolly 128" );

	// Look with the armed Right press.
	{
		cam.SetAngles( 90, 0 );
		checks.That( Is( nav.OnPointer( cam, Ev( V, PointerPhase::Down, 400, 300, R ) ),
		                 ToolResultKind::Ignored ),
		    "a right press is only armed" );
		checks.That( Is( nav.OnPointer( cam, Ev( V, PointerPhase::Move, 402, 300, R ) ),
		                 ToolResultKind::Ignored ),
		    "below the threshold: still the tool's" );
		checks.That( Is( nav.OnPointer( cam, Ev( V, PointerPhase::Move, 440, 300, R ) ),
		                 ToolResultKind::CaptureRequested ),
		    "past the threshold: look captures" );
		checks.Near( cam.Yaw(), 80.0, 1e-9, "40 px right turns 10 degrees right" );
		nav.OnPointer( cam, Ev( V, PointerPhase::Move, 440, 340, R ) );
		checks.Near( cam.Pitch(), 10.0, 1e-9, "40 px down looks 10 degrees down" );
		checks.That( Is( nav.OnPointer( cam, Ev( V, PointerPhase::Up, 440, 340, R ) ),
		                 ToolResultKind::CaptureReleased ),
		    "release" );
		nav.OnPointer( cam, Ev( V, PointerPhase::Down, 400, 300, R ) );
		checks.That( Is( nav.OnPointer( cam, Ev( V, PointerPhase::Up, 400, 300, R ) ),
		                 ToolResultKind::Ignored ),
		    "an armed release without a drag is the tool's" );
	}
	// Orbit.
	{
		const Vec3d pivot( 32, 32, 32 );
		nav.SetOrbitPivot( pivot );
		cam.SetPosition( Vec3d( 32, -268, 32 ) );
		cam.SetAngles( 90, 0 );
		checks.That(
		    Is( nav.OnPointer( cam, Ev( V, PointerPhase::Down, 400, 300, L, Mods( tools::kAlt ) ) ),
		        ToolResultKind::CaptureRequested ),
		    "Alt + Left orbits" );
		nav.OnPointer( cam, Ev( V, PointerPhase::Move, 580, 300, L, Mods( tools::kAlt ) ) );
		nav.OnPointer( cam, Ev( V, PointerPhase::Up, 580, 300, L, Mods( tools::kAlt ) ) );
		checks.Near( mapgeometry::Length( cam.Position() - pivot ), 300.0, 1e-6,
		    "distance to the pivot kept" );
		checks.Near( cam.Yaw(), 0.0, 1e-9, "180 px = 90 degrees about the pivot" );
		checks.That( mapgeometry::NearlyEqual(
		                 cam.Forward(), mapgeometry::Normalize( pivot - cam.Position() ), 1e-9 ),
		    "still looking at the pivot" );
	}
	// Middle pan in 3D.
	{
		cam.SetPosition( Vec3d( 0, 0, 0 ) );
		cam.SetAngles( 90, 0 );
		nav.OnPointer( cam, Ev( V, PointerPhase::Down, 400, 300, M ) );
		nav.OnPointer( cam, Ev( V, PointerPhase::Up, 410, 290, M ) );
		checks.That( mapgeometry::NearlyEqual( cam.Position(), Vec3d( -10, 0, -10 ), 1e-9 ),
		    "the content follows the pointer: the eye moves left and down" );
	}
	// Focus loss.
	{
		nav.OnKey( tools::KeyEvent::Char( 'w' ) );
		nav.OnPointer( cam, Ev( V, PointerPhase::Down, 400, 300, M ) );
		nav.OnFocusLost();
		checks.That( !nav.Flying() && !nav.HasCapture() && !nav.Advance( cam, 1 ),
		    "focus loss releases keys and drags" );
		checks.That( Is( nav.OnPointer( cam, Ev( V, PointerPhase::Up, 400, 300, M ) ),
		                 ToolResultKind::Ignored ),
		    "the stray release is ignored" );
	}

	// --- Framing -------------------------------------------------------------------------------------
	{
		viewport::Camera2D before = top;
		checks.That( !nav.FrameSelection( top, rig.session ) && top.Center() == before.Center(),
		    "nothing to frame" );
		checks.That(
		    rig.session.SelectObjects( { A }, app::SelectMode::Replace ).HasValue(), "select A" );
		checks.That( nav.FrameSelection( top, rig.session ), "frame 2D" );
		checks.That( top.Center() == viewport::PlanePoint{ 1032, 1032 }, "2D centred on A" );
		cam.SetAngles( 45, 20 );
		checks.That( nav.FrameSelection( cam, rig.session ), "frame 3D" );
		const std::optional<viewport::ScreenPoint> c = cam.WorldToScreen( Vec3d( 1032, 1032, 32 ) );
		checks.That( c && std::fabs( c->x - 400 ) < 1e-6 && std::fabs( c->y - 300 ) < 1e-6,
		    "3D: A's centre on screen centre" );
		checks.Near( cam.Yaw(), 45.0, 1e-9, "3D framing keeps the angles" );
	}

	// --- Host arbitration with a tool -------------------------------------------------------------
	{
		tools::FaceTool *face = rig.Use( std::make_unique<tools::FaceTool>() );
		(void)face;
		rig.editor.faceTexture.material = "TOOLS/TOOLSNODRAW";
		cam.SetPosition( Vec3d( 32, -200, 32 ) );
		cam.SetAngles( 90, 0 );
		// The host routine: navigation first; a navigation capture cancels the tool's press.
		const auto route = [&]( const tools::PointerEvent &e )
		{
			const tools::ToolResult r = nav.OnPointer( cam, e );
			if ( !Is( r, ToolResultKind::Ignored ) )
			{
				if ( Is( r, ToolResultKind::CaptureRequested ) )
					rig.manager.OnCaptureLost();
				return r;
			}
			tools::ToolContext ctx = rig.Ctx( V );
			return rig.manager.OnPointer( ctx, e );
		};
		const std::size_t history = rig.HistorySize();
		checks.That( Is( route( Ev( V, PointerPhase::Down, 400, 300, R ) ),
		                 ToolResultKind::CaptureRequested ),
		    "the face tool takes the armed right press" );
		route( Ev( V, PointerPhase::Up, 400, 300, R ) );
		checks.Equal(
		    rig.HistorySize(), history + 1, "a right click applies the material through the tool" );
		const double yaw = cam.Yaw();
		route( Ev( V, PointerPhase::Down, 400, 300, R ) );
		checks.That( rig.manager.HasCapture(), "the tool holds the press" );
		checks.That( Is( route( Ev( V, PointerPhase::Move, 440, 300, R ) ),
		                 ToolResultKind::CaptureRequested ),
		    "the drag becomes a look" );
		checks.That( !rig.manager.HasCapture(), "the tool's press was cancelled" );
		route( Ev( V, PointerPhase::Up, 440, 300, R ) );
		checks.Equal( rig.HistorySize(), history + 1, "a right drag makes no edit" );
		checks.Near( cam.Yaw(), yaw - 10.0, 1e-9, "and looked" );
	}
	checks.That( rig.session.Document().Contains( wall ), "navigation never edits the document" );

	return checks.Report();
}
