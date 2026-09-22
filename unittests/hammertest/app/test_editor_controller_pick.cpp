//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance test for EditorController::PickByRay (RFC 0002,
//			HAM-WORKFLOW-001 / hammer.app) -- the 3D camera view's click-to-select.
//			It drives the authority with SIMULATED world-space rays, the exact call
//			the GTK presenter makes from a 3D-viewport click (it builds the ray from
//			the camera and calls PickByRay). The oracle proves: an axis ray selects
//			the nearest brush it enters; reversing the ray selects the far brush; a
//			ray into empty space (or facing away from a brush) deselects, like an
//			empty 2D click; and a pick records NO undo step (selection-only, exactly
//			like the 2D projection pick). Because the shipped 3D selection routes
//			through this same call, this covers the real logic, not a mock.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/editor_controller.h"

#include <cmath>
#include <cstdio>
#include <string>

using hammer::app::EditorController;
using hammer::geometry::Vec3d;

namespace
{

int g_failures = 0;

void Check( bool condition, const char *expression, int line )
{
	if ( !condition )
	{
		std::printf( "FAIL %s:%d: %s\n", "test_editor_controller_pick.cpp", line, expression );
		++g_failures;
	}
}

#define CHECK( cond ) Check( ( cond ), #cond, __LINE__ )

bool Near( double a, double b )
{
	return std::fabs( a - b ) <= 1.0e-4;
}

// Two axis-aligned box brushes: one around x=0, one around x=500, so a ray down
// the X axis meets a well-defined near/far ordering.
const char *kTwoBoxes =
    "versioninfo { \"editorversion\" \"400\" }\n"
    "world\n"
    "{\n"
    "	\"classname\" \"worldspawn\"\n"
    "	solid\n"
    "	{\n"
    "		side { \"plane\" \"(-64 -64 -64) (-64 64 -64) (64 64 -64)\" \"material\" \"A\" }\n"
    "		side { \"plane\" \"(-64 -64 64) (64 -64 64) (64 64 64)\" \"material\" \"A\" }\n"
    "		side { \"plane\" \"(-64 -64 -64) (-64 -64 64) (-64 64 64)\" \"material\" \"A\" }\n"
    "		side { \"plane\" \"(64 -64 -64) (64 64 -64) (64 64 64)\" \"material\" \"A\" }\n"
    "		side { \"plane\" \"(-64 -64 -64) (64 -64 -64) (64 -64 64)\" \"material\" \"A\" }\n"
    "		side { \"plane\" \"(-64 64 -64) (-64 64 64) (64 64 64)\" \"material\" \"A\" }\n"
    "	}\n"
    "	solid\n"
    "	{\n"
    "		side { \"plane\" \"(436 -64 -64) (436 64 -64) (564 64 -64)\" \"material\" \"B\" }\n"
    "		side { \"plane\" \"(436 -64 64) (564 -64 64) (564 64 64)\" \"material\" \"B\" }\n"
    "		side { \"plane\" \"(436 -64 -64) (436 -64 64) (436 64 64)\" \"material\" \"B\" }\n"
    "		side { \"plane\" \"(564 -64 -64) (564 64 -64) (564 64 64)\" \"material\" \"B\" }\n"
    "		side { \"plane\" \"(436 -64 -64) (564 -64 -64) (564 -64 64)\" \"material\" \"B\" }\n"
    "		side { \"plane\" \"(436 64 -64) (436 64 64) (564 64 64)\" \"material\" \"B\" }\n"
    "	}\n"
    "}\n";

void TestPickNearestAlongRay()
{
	EditorController c;
	std::string error;
	CHECK( c.LoadVmf( kTwoBoxes, error ) );
	CHECK( c.Brushes().size() == 2 );
	if ( c.Brushes().size() != 2 )
	{
		return;
	}
	const int idNear = c.Brushes()[0].id; // x ~ 0
	const int idFar = c.Brushes()[1].id;  // x ~ 500

	// From far -X aiming +X, the near box (A) is entered first.
	CHECK( c.PickByRay( Vec3d( -2000, 0, 0 ), Vec3d( 1, 0, 0 ) ) );
	CHECK( c.Selection().has_value() && *c.Selection() == idNear );

	// From far +X aiming -X, the far box (B) is now the first entered.
	CHECK( c.PickByRay( Vec3d( 2000, 0, 0 ), Vec3d( -1, 0, 0 ) ) );
	CHECK( c.Selection().has_value() && *c.Selection() == idFar );

	// A non-normalized direction picks the same brush (dir need not be unit length).
	CHECK( c.PickByRay( Vec3d( -2000, 0, 0 ), Vec3d( 7, 0, 0 ) ) );
	CHECK( c.Selection().has_value() && *c.Selection() == idNear );
}

void TestMissDeselects()
{
	EditorController c;
	std::string error;
	CHECK( c.LoadVmf( kTwoBoxes, error ) );

	CHECK( c.PickByRay( Vec3d( -2000, 0, 0 ), Vec3d( 1, 0, 0 ) ) ); // select something
	CHECK( c.Selection().has_value() );

	// A ray into empty space misses and clears the selection (like an empty click).
	CHECK( !c.PickByRay( Vec3d( 0, 0, 2000 ), Vec3d( 0, 0, 1 ) ) );
	CHECK( !c.Selection().has_value() );

	// A ray whose origin is near a box but points away from it does not hit it.
	CHECK( c.PickByRay( Vec3d( -2000, 0, 0 ), Vec3d( 1, 0, 0 ) ) ); // reselect
	CHECK( !c.PickByRay( Vec3d( -2000, 0, 0 ), Vec3d( -1, 0, 0 ) ) );
	CHECK( !c.Selection().has_value() );
}

void TestPickRecordsNoUndo()
{
	EditorController c;
	std::string error;
	CHECK( c.LoadVmf( kTwoBoxes, error ) );
	CHECK( !c.CanUndo() );
	c.PickByRay( Vec3d( -2000, 0, 0 ), Vec3d( 1, 0, 0 ) );
	c.PickByRay( Vec3d( 0, 0, 2000 ), Vec3d( 0, 0, 1 ) );
	// Selection is not a document mutation, so it must add no history unit.
	CHECK( !c.CanUndo() );
}

void TestPickOnEmptyMap()
{
	EditorController c;
	c.NewMap();
	CHECK( !c.PickByRay( Vec3d( 0, 0, 0 ), Vec3d( 1, 0, 0 ) ) );
	CHECK( !c.Selection().has_value() );
}

// A triangular prism (wedge): cross-section triangle (0,0)-(128,0)-(0,64) in XZ,
// extruded in Y over [0,128]. Its bounding box is the full [0,128]x[0,128]x[0,64]
// box, but the top-outer corner (large X, large Z) is empty. Winding is derived
// from an interior point by the brush bridge, so any consistent point triple works.
const char *kWedge =
    "versioninfo { \"editorversion\" \"400\" }\n"
    "world\n"
    "{\n"
    "	\"classname\" \"worldspawn\"\n"
    "	solid\n"
    "	{\n"
    "		side { \"plane\" \"(0 0 0) (128 0 0) (128 128 0)\" \"material\" \"W\" }\n"    // z = 0
    "		side { \"plane\" \"(0 0 0) (0 128 0) (0 0 64)\" \"material\" \"W\" }\n"       // x = 0
    "		side { \"plane\" \"(0 0 0) (0 0 64) (128 0 0)\" \"material\" \"W\" }\n"       // y = 0
    "		side { \"plane\" \"(0 128 0) (128 128 0) (0 128 64)\" \"material\" \"W\" }\n" // y = 128
    "		side { \"plane\" \"(128 0 0) (128 128 0) (0 0 64)\" \"material\" \"W\" }\n"   // slope
    "	}\n"
    "}\n";

void TestPreciseConvexPick()
{
	EditorController c;
	std::string error;
	CHECK( c.LoadVmf( kWedge, error ) );
	CHECK( c.Brushes().size() == 1 );
	if ( c.Brushes().size() != 1 )
	{
		return;
	}
	const auto &w = c.Brushes()[0];
	// Real shape preserved: a 5-face prism, whose AABB is the full box.
	CHECK( w.planes.size() == 5 );
	CHECK( Near( w.mins.x, 0.0 ) && Near( w.mins.y, 0.0 ) && Near( w.mins.z, 0.0 ) );
	CHECK( Near( w.maxs.x, 128.0 ) && Near( w.maxs.y, 128.0 ) && Near( w.maxs.z, 64.0 ) );
	const int id = w.id;

	// A ray through the solid body selects it (proves outward normals + pick work).
	CHECK( c.PickByRay( Vec3d( 2000, 64, 10 ), Vec3d( -1, 0, 0 ) ) );
	CHECK( c.Selection().has_value() && *c.Selection() == id );

	// (100,64,60) lies inside the bounding box but ABOVE the slope, in the empty
	// corner. A ray from there pointing further out (+x,+z) misses the solid -- the
	// AABB broad phase contains the origin, so only the precise convex test can
	// reject it. A bounding-box-only pick would wrongly report a hit here.
	CHECK( !c.PickByRay( Vec3d( 100, 64, 60 ), Vec3d( 1, 0, 1 ) ) );
	CHECK( !c.Selection().has_value() );

	// From the SAME corner point, pointing back into the solid does hit it, proving
	// the point is genuinely outside the solid yet inside its bounding box.
	CHECK( c.PickByRay( Vec3d( 100, 64, 60 ), Vec3d( -1, 0, -1 ) ) );
	CHECK( c.Selection().has_value() && *c.Selection() == id );
}

void TestNudgeSelection()
{
	EditorController c;
	std::string error;
	CHECK( c.LoadVmf( kTwoBoxes, error ) );
	// Select the near box via a 3D pick, then nudge it like an arrow key would.
	CHECK( c.PickByRay( Vec3d( -2000, 0, 0 ), Vec3d( 1, 0, 0 ) ) );
	CHECK( c.Selection().has_value() );
	const int id = *c.Selection();
	const auto findMinsX = [&]()
	{
		for ( const auto &b : c.Brushes() )
		{
			if ( b.id == id )
			{
				return b.mins.x;
			}
		}
		return 0.0;
	};
	const double before = findMinsX();
	CHECK( c.MoveSelectionBy( 64, 0, 0 ) );
	CHECK( findMinsX() == before + 64.0 );
	CHECK( c.CanUndo() ); // a nudge is one undo unit
	CHECK( c.Undo() );
	CHECK( findMinsX() == before ); // undo restores the position

	// Nudge with nothing selected is a no-op.
	c.PickByRay( Vec3d( 0, 0, 2000 ), Vec3d( 0, 0, 1 ) ); // miss -> deselect
	CHECK( !c.Selection().has_value() );
	CHECK( !c.MoveSelectionBy( 64, 0, 0 ) );
}

void TestMultiSelect()
{
	EditorController c;
	std::string error;
	CHECK( c.LoadVmf( kTwoBoxes, error ) );
	const int idA = c.Brushes()[0].id;
	const int idB = c.Brushes()[1].id;

	// Ctrl-add both boxes to the selection (additive picks).
	CHECK( c.PickByRay( Vec3d( -2000, 0, 0 ), Vec3d( 1, 0, 0 ), true ) ); // A
	CHECK( c.PickByRay( Vec3d( 2000, 0, 0 ), Vec3d( -1, 0, 0 ), true ) ); // B
	CHECK( c.SelectionCount() == 2 );
	CHECK( c.IsSelected( idA ) && c.IsSelected( idB ) );
	CHECK( c.Selections().size() == 2 );

	// A nudge moves BOTH selected brushes together (one undo unit). The multi-
	// selection stays intact across a nudge (nudge does not clear it).
	const auto minsX = [&]( int id )
	{
		for ( const auto &b : c.Brushes() )
		{
			if ( b.id == id )
			{
				return b.mins.x;
			}
		}
		return 0.0;
	};
	const double ax = minsX( idA );
	const double bx = minsX( idB );
	CHECK( c.MoveSelectionBy( 32, 0, 0 ) );
	CHECK( minsX( idA ) == ax + 32.0 && minsX( idB ) == bx + 32.0 );
	CHECK( c.SelectionCount() == 2 );

	// Ctrl-clicking an already-selected brush toggles just it back off (A now sits
	// at x=[-32,96] after the nudge, still on the -X ray).
	CHECK( c.PickByRay( Vec3d( -2000, 0, 0 ), Vec3d( 1, 0, 0 ), true ) ); // toggle A off
	CHECK( c.SelectionCount() == 1 );
	CHECK( !c.IsSelected( idA ) && c.IsSelected( idB ) );

	// A plain (non-additive) pick collapses back to a single selection.
	CHECK( c.PickByRay( Vec3d( -2000, 0, 0 ), Vec3d( 1, 0, 0 ), false ) );
	CHECK( c.SelectionCount() == 1 );
	CHECK( c.IsSelected( idA ) && !c.IsSelected( idB ) );
}

void TestMultiDelete()
{
	EditorController c;
	std::string error;
	CHECK( c.LoadVmf( kTwoBoxes, error ) );
	CHECK( c.PickByRay( Vec3d( -2000, 0, 0 ), Vec3d( 1, 0, 0 ), true ) );
	CHECK( c.PickByRay( Vec3d( 2000, 0, 0 ), Vec3d( -1, 0, 0 ), true ) );
	CHECK( c.SelectionCount() == 2 );
	// Delete removes every selected brush in one undo unit.
	CHECK( c.DeleteSelection() );
	CHECK( c.Brushes().empty() );
	CHECK( c.SelectionCount() == 0 );
	CHECK( c.Undo() ); // both come back
	CHECK( c.Brushes().size() == 2 );
}

} // namespace

int main()
{
	TestPickNearestAlongRay();
	TestMissDeselects();
	TestPickRecordsNoUndo();
	TestPickOnEmptyMap();
	TestPreciseConvexPick();
	TestNudgeSelection();
	TestMultiSelect();
	TestMultiDelete();

	if ( g_failures != 0 )
	{
		std::printf( "hammer.app.editor_controller_pick: %d check(s) FAILED\n", g_failures );
		return 1;
	}
	std::printf( "hammer.app.editor_controller_pick: all checks passed\n" );
	return 0;
}
