//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless oracle for the GTK Hammer shell's 3D camera navigation math
//			(RFC 0002, linux-gtk-desktop). It checks hammergtk::Renderer's free-fly
//			navigation the interactive presenter drives -- FlyMove (WASD/QE flight),
//			FlyLook (Z mouse-look, which must keep the eye fixed), and PixelToRay
//			(the click-to-select ray) -- without a GL context, since these methods
//			are pure camera math. Run by hammer/gtk/tests/camera_nav_test.sh.
//
//=============================================================================//

#include "renderer.h"

#include <cmath>
#include <cstdio>

using namespace hammergtk;

static int g_fail = 0;
static void Check( bool ok, const char *what )
{
	std::printf( "  [%s] %s\n", ok ? "PASS" : "FAIL", what );
	if ( !ok )
	{
		++g_fail;
	}
}
static bool Near( float a, float b, float eps = 1e-3f )
{
	return std::fabs( a - b ) < eps;
}

// Recompute the eye the same way the renderer does, for oracles below.
static void Eye( const Camera &c, float out[3] )
{
	const float y = c.yawDeg * 3.14159265f / 180.0f;
	const float p = c.pitchDeg * 3.14159265f / 180.0f;
	out[0] = c.target[0] + c.distance * std::cos( p ) * std::cos( y );
	out[1] = c.target[1] + c.distance * std::cos( p ) * std::sin( y );
	out[2] = c.target[2] + c.distance * std::sin( p );
}

int main()
{
	std::printf( "camera-math:\n" );

	// --- PixelToRay: centre pixel looks straight down the view direction -----
	{
		Renderer r;
		r.SetViewMode( ViewMode::Perspective );
		Camera &c = r.Cam();
		c.yawDeg = 0.0f;
		c.pitchDeg = 0.0f;
		c.distance = 1000.0f;
		c.target[0] = c.target[1] = c.target[2] = 0.0f;

		float eye[3];
		Eye( c, eye );
		float o[3], d[3];
		bool ok = r.PixelToRay( 400.0f, 300.0f, 800, 600, o, d );
		Check( ok, "PixelToRay returns true for the 3D view" );
		Check( Near( o[0], eye[0] ) && Near( o[1], eye[1] ) && Near( o[2], eye[2] ),
		    "ray origin is the eye" );
		// forward = normalize(target - eye); here eye=(+1000,0,0), so forward=(-1,0,0).
		Check( Near( d[0], -1.0f ) && Near( d[1], 0.0f ) && Near( d[2], 0.0f ),
		    "centre ray points along the look direction" );

		// A pixel to the right of centre must tilt the ray toward screen-right.
		float d2[3];
		r.PixelToRay( 600.0f, 300.0f, 800, 600, o, d2 );
		Check( d2[1] < d[1] - 1e-3f || d2[1] > d[1] + 1e-3f, "off-centre ray differs in Y" );
	}

	// --- PixelToRay: 2D views return false (caller falls back to projection) --
	{
		Renderer r;
		r.SetViewMode( ViewMode::Top );
		float o[3], d[3];
		Check( !r.PixelToRay( 10, 10, 100, 100, o, d ), "PixelToRay is false for a 2D view" );
	}

	// --- FlyMove: flying forward advances the eye along the look direction ----
	{
		Renderer r;
		r.SetViewMode( ViewMode::Perspective );
		Camera &c = r.Cam();
		c.yawDeg = 0.0f;
		c.pitchDeg = 0.0f;
		c.distance = 500.0f;
		c.target[0] = c.target[1] = c.target[2] = 0.0f;
		float before[3];
		Eye( c, before );
		r.FlyMove( 100.0f, 0.0f, 0.0f ); // forward = (-1,0,0)
		float after[3];
		Eye( c, after );
		Check( Near( after[0], before[0] - 100.0f ) && Near( after[1], before[1] ) &&
		           Near( after[2], before[2] ),
		    "FlyMove(forward) moves the eye -100 in X" );

		// Strafe right moves along screen-right (here -Y for this orientation).
		Eye( c, before );
		r.FlyMove( 0.0f, 50.0f, 0.0f );
		Eye( c, after );
		Check( Near( after[0], before[0] ) && !Near( after[1], before[1] ),
		    "FlyMove(right) moves laterally, not forward" );

		// Vertical is world +Z.
		Eye( c, before );
		r.FlyMove( 0.0f, 0.0f, 25.0f );
		Eye( c, after );
		Check( Near( after[2], before[2] + 25.0f ) && Near( after[0], before[0] ),
		    "FlyMove(up) rises along world Z" );
	}

	// --- FlyLook: rotating in place keeps the eye fixed -----------------------
	{
		Renderer r;
		r.SetViewMode( ViewMode::Perspective );
		Camera &c = r.Cam();
		c.yawDeg = 30.0f;
		c.pitchDeg = 10.0f;
		c.distance = 700.0f;
		c.target[0] = 100.0f;
		c.target[1] = -50.0f;
		c.target[2] = 20.0f;
		float before[3];
		Eye( c, before );
		const float yawBefore = c.yawDeg;
		r.FlyLook( 15.0f, -5.0f );
		float after[3];
		Eye( c, after );
		Check( Near( after[0], before[0] ) && Near( after[1], before[1] ) &&
		           Near( after[2], before[2] ),
		    "FlyLook keeps the eye position fixed" );
		Check( !Near( c.yawDeg, yawBefore ), "FlyLook changed the yaw" );
	}

	// --- FlyMove/FlyLook are no-ops in 2D views -------------------------------
	{
		Renderer r;
		r.SetViewMode( ViewMode::Front );
		Camera &c = r.Cam();
		const float y = c.yawDeg;
		r.FlyLook( 90.0f, 45.0f );
		r.FlyMove( 999.0f, 0.0f, 0.0f );
		Check(
		    Near( c.yawDeg, y ) && Near( c.target[0], 0.0f ), "fly ops do nothing for a 2D view" );
	}

	std::printf( g_fail == 0 ? "camera-math: ALL PASS\n" : "camera-math: %d FAIL\n", g_fail );
	return g_fail == 0 ? 0 : 1;
}
