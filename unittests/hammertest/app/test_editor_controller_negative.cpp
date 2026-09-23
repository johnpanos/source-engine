//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Negative-provider check for the app.editor_controller.v1 contract (RFC
//			0002). RFC 0002/0005 require a deliberately-broken provider to prove the
//			conformance oracle actually detects violations and is not vacuous.
//
//			The shared predicate below expresses the contract's grid-snapping
//			clause -- a Block-tool brush created from unsnapped pointer input has
//			every in-plane bound on the grid. It runs against the real
//			EditorController (must pass) and against a NoSnapBuilder that records the
//			raw pointer coordinates (must be detected).
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/editor_controller.h"
#include "testing/conformance_result.h"

#include <cmath>
#include <cstdio>

using hammer::app::EditorController;
using hammer::app::Tool;
using hammer::app::ViewId;

namespace
{

struct Rect
{
	bool made = false;
	double x0 = 0;
	double y0 = 0;
	double x1 = 0;
	double y1 = 0;
};

// The real provider: drive the Block-tool flow and read back the created brush's
// X/Y bounds (Top view edits X/Y).
Rect RealBuild( int grid, double u0, double v0, double u1, double v1 )
{
	EditorController c;
	c.SetGridSize( grid );
	c.SetTool( Tool::Block );
	c.PointerDown( ViewId::Top, u0, v0 );
	c.PointerDrag( ViewId::Top, u1, v1 );
	c.PointerUp( ViewId::Top, u1, v1 );
	Rect r;
	r.made = c.Commit();
	if ( r.made && !c.Brushes().empty() )
	{
		const auto &b = c.Brushes().front();
		r.x0 = b.mins.x;
		r.y0 = b.mins.y;
		r.x1 = b.maxs.x;
		r.y1 = b.maxs.y;
	}
	return r;
}

// A deliberately non-conforming provider: it keeps the raw (unsnapped) pointer
// coordinates as the brush bounds, violating the grid-snapping clause.
Rect NoSnapBuild( int, double u0, double v0, double u1, double v1 )
{
	Rect r;
	r.made = true;
	r.x0 = std::min( u0, u1 );
	r.y0 = std::min( v0, v1 );
	r.x1 = std::max( u0, u1 );
	r.y1 = std::max( v0, v1 );
	return r;
}

// The grid-snapping clause as a provider-parameterized predicate: from unsnapped
// input, every in-plane bound of the created brush is a multiple of the grid.
template <typename Builder> bool ConformsGridSnap( Builder build )
{
	const int grid = 64;
	const Rect r = build( grid, 10.0, 20.0, 200.0, 150.0 );
	if ( !r.made )
	{
		return false;
	}
	for ( double v : { r.x0, r.y0, r.x1, r.y1 } )
	{
		if ( std::fabs( std::fmod( v, static_cast<double>( grid ) ) ) > 1.0e-6 )
		{
			return false;
		}
	}
	return true;
}

} // namespace

int main()
{
	const bool realConforms = ConformsGridSnap( RealBuild );
	const bool buggyConforms = ConformsGridSnap( NoSnapBuild );

	int checks = 0;
	int failures = 0;
	++checks;
	if ( !realConforms )
	{
		std::printf( "FAIL: real EditorController violated the grid-snapping clause\n" );
		++failures;
	}
	++checks;
	if ( buggyConforms )
	{
		std::printf( "FAIL: conformance predicate did NOT detect the no-snap provider\n" );
		++failures;
	}

	if ( failures != 0 )
	{
		std::printf( "hammer.app.editor_controller negative: %d check(s) FAILED\n", failures );
		return testing::ReportConformance( checks, failures );
	}
	std::printf( "hammer.app.editor_controller negative: oracle detects violations (real passes, "
	             "buggy caught)\n" );
	return testing::ReportConformance( checks, failures );
}
