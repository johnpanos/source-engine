//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance UI test for the headless editor interaction authority
//			(RFC 0002, HAM-WORKFLOW-001 / hammer.app). It drives EditorController
//			with SIMULATED normalized input -- the exact calls the GTK presenter
//			makes from gestures/keys -- to exercise the core Hammer UX flows and
//			builds a simple map end to end: Block-tool create with grid snapping,
//			multi-view extrusion, Selection-tool pick/move/delete, undo/redo, and
//			VMF save round-trip. Because the shipped UI routes through this same
//			authority, this test covers the real editing logic, not a mock.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/editor_controller.h"
#include "hammer/formats/keyvalues.h"

#include <cmath>
#include <cstdio>

using hammer::app::EditorController;
using hammer::app::MapBrush;
using hammer::app::Tool;
using hammer::app::ViewId;
using hammer::geometry::Vec3d;

namespace
{

int g_failures = 0;

void Check( bool condition, const char *expression, int line )
{
	if ( !condition )
	{
		std::printf( "FAIL %s:%d: %s\n", "test_editor_controller.cpp", line, expression );
		++g_failures;
	}
}

#define CHECK( cond ) Check( ( cond ), #cond, __LINE__ )

bool Near( double a, double b )
{
	return std::fabs( a - b ) <= 1.0e-6;
}

bool BoxIs( const MapBrush &b, double x0, double y0, double z0, double x1, double y1, double z1 )
{
	return Near( b.mins.x, x0 ) && Near( b.mins.y, y0 ) && Near( b.mins.z, z0 ) &&
	       Near( b.maxs.x, x1 ) && Near( b.maxs.y, y1 ) && Near( b.maxs.z, z1 );
}

// Simulates the Block-tool gesture in one view: press, drag, release, commit.
bool BlockDrag( EditorController &c, ViewId view, double u0, double v0, double u1, double v1 )
{
	c.SetTool( Tool::Block );
	c.PointerDown( view, u0, v0 );
	c.PointerDrag( view, u1, v1 );
	c.PointerUp( view, u1, v1 );
	return c.Commit();
}

void TestBlockCreateAndSnap()
{
	EditorController c;
	c.SetGridSize( 64 );

	// Unsnapped input must snap to the grid: (10,20)->(200,150) => (0,0)->(192,128).
	CHECK( BlockDrag( c, ViewId::Top, 10, 20, 200, 150 ) );
	CHECK( c.Brushes().size() == 1 );
	if ( !c.Brushes().empty() )
	{
		// Default block depth (128) extrudes along the free axis Z.
		CHECK( BoxIs( c.Brushes()[0], 0, 0, 0, 192, 128, 128 ) );
		// Every bound is a grid multiple.
		const MapBrush &b = c.Brushes()[0];
		for ( double v : { b.mins.x, b.mins.y, b.maxs.x, b.maxs.y } )
		{
			CHECK( std::fabs( std::fmod( v, 64.0 ) ) < 1.0e-6 );
		}
	}
	CHECK( c.IsModified() );
	CHECK( c.CanUndo() );

	// One brush -> one 6-faced solid in the renderable scene.
	const auto scene = c.BuildScene();
	CHECK( scene.solids.size() == 1 );
	CHECK( scene.TotalFaces() == 6 );
}

void TestMultiViewExtrude()
{
	EditorController c;
	c.SetGridSize( 64 );

	// Front view edits X/Z; the free axis (Y) gets the default depth.
	CHECK( BlockDrag( c, ViewId::Front, 0, 0, 128, 256 ) );
	CHECK( c.Brushes().size() == 1 );
	if ( !c.Brushes().empty() )
	{
		CHECK( BoxIs( c.Brushes()[0], 0, 0, 0, 128, 128, 256 ) );
	}

	// Side view edits Y/Z; the free axis (X) gets the default depth.
	EditorController c2;
	c2.SetGridSize( 64 );
	CHECK( BlockDrag( c2, ViewId::Side, 0, 0, 192, 64 ) );
	if ( !c2.Brushes().empty() )
	{
		CHECK( BoxIs( c2.Brushes()[0], 0, 0, 0, 128, 192, 64 ) );
	}
}

void TestSelectMoveDelete()
{
	EditorController c;
	c.SetGridSize( 64 );
	CHECK( BlockDrag( c, ViewId::Top, 0, 0, 256, 128 ) ); // brush [0..256,0..128]

	c.SetTool( Tool::Select );
	c.PointerDown( ViewId::Top, 64, 64 ); // inside the brush
	CHECK( c.Selection().has_value() );

	// Drag +64 in X.
	c.PointerDrag( ViewId::Top, 128, 64 );
	c.PointerUp( ViewId::Top, 128, 64 );
	CHECK( c.Brushes().size() == 1 );
	if ( !c.Brushes().empty() )
	{
		CHECK( BoxIs( c.Brushes()[0], 64, 0, 0, 320, 128, 128 ) );
	}

	// Clicking empty space deselects.
	c.PointerDown( ViewId::Top, 2000, 2000 );
	c.PointerUp( ViewId::Top, 2000, 2000 );
	CHECK( !c.Selection().has_value() );

	// Re-select and delete.
	c.PointerDown( ViewId::Top, 128, 64 );
	c.PointerUp( ViewId::Top, 128, 64 );
	CHECK( c.Selection().has_value() );
	CHECK( c.DeleteSelection() );
	CHECK( c.Brushes().empty() );
	CHECK( !c.Selection().has_value() );
}

void TestUndoRedo()
{
	EditorController c;
	c.SetGridSize( 64 );
	CHECK( !c.CanUndo() );
	CHECK( BlockDrag( c, ViewId::Top, 0, 0, 128, 128 ) );
	CHECK( BlockDrag( c, ViewId::Top, 256, 256, 384, 384 ) );
	CHECK( c.Brushes().size() == 2 );

	CHECK( c.Undo() );
	CHECK( c.Brushes().size() == 1 );
	CHECK( c.Undo() );
	CHECK( c.Brushes().empty() );
	CHECK( !c.CanUndo() );

	CHECK( c.Redo() );
	CHECK( c.Brushes().size() == 1 );
	CHECK( c.Redo() );
	CHECK( c.Brushes().size() == 2 );
	CHECK( !c.CanRedo() );

	// A new commit after undo clears the redo tail.
	CHECK( c.Undo() );                                      // back to 1
	CHECK( BlockDrag( c, ViewId::Top, 512, 0, 640, 128 ) ); // new branch
	CHECK( c.Brushes().size() == 2 );
	CHECK( !c.CanRedo() );

	// Undo returns to the saved (empty) base and clears the modified flag.
	CHECK( c.Undo() );
	CHECK( c.Undo() );
	CHECK( c.Brushes().empty() );
	CHECK( !c.IsModified() );
}

void TestGuardsRejectBadInput()
{
	EditorController c;
	c.SetGridSize( 64 );

	// Degenerate (zero-width) rectangle creates nothing.
	CHECK( !BlockDrag( c, ViewId::Top, 0, 0, 0, 128 ) );
	CHECK( c.Brushes().empty() );

	// Camera-view input is navigation-only: it never creates geometry.
	c.SetTool( Tool::Block );
	c.PointerDown( ViewId::Camera, 0, 0 );
	c.PointerDrag( ViewId::Camera, 256, 256 );
	c.PointerUp( ViewId::Camera, 256, 256 );
	CHECK( !c.Commit() );
	CHECK( c.Brushes().empty() );

	// Switching tool mid-drag clears the pending rectangle.
	c.SetTool( Tool::Block );
	c.PointerDown( ViewId::Top, 0, 0 );
	c.PointerDrag( ViewId::Top, 128, 128 );
	c.SetTool( Tool::Select );
	CHECK( !c.HasPending() );
	CHECK( !c.Commit() );
	CHECK( c.Brushes().empty() );
}

// A non-box brush (5-sided triangular prism) must load, render, and round-trip as
// its REAL shape -- not be collapsed to a bounding box.
void TestLoadNonBoxPreservesShape()
{
	// Right-triangular prism: x>=0, y>=0, z in [0,128], x+y<=128.
	const char *kWedge =
	    "world\n{\n\t\"id\" \"1\"\n\t\"classname\" \"worldspawn\"\n"
	    "\tsolid\n\t{\n\t\t\"id\" \"2\"\n"
	    "\t\tside { \"plane\" \"(0 0 0) (64 0 0) (0 64 0)\" \"material\" \"DEV/A\" }\n"
	    "\t\tside { \"plane\" \"(0 0 128) (0 64 128) (64 0 128)\" \"material\" \"DEV/B\" }\n"
	    "\t\tside { \"plane\" \"(0 0 0) (0 64 0) (0 0 128)\" \"material\" \"DEV/C\" }\n"
	    "\t\tside { \"plane\" \"(0 0 0) (64 0 0) (0 0 128)\" \"material\" \"DEV/D\" }\n"
	    "\t\tside { \"plane\" \"(128 0 0) (0 128 0) (128 0 128)\" \"material\" \"DEV/E\" }\n"
	    "\t}\n}\n";

	EditorController c;
	std::string error;
	CHECK( c.LoadVmf( kWedge, error ) );
	CHECK( c.Brushes().size() == 1 );
	if ( c.Brushes().empty() )
	{
		return;
	}
	// Five real side planes preserved (a box would have six).
	CHECK( c.Brushes()[0].planes.size() == 5 );
	// Cached AABB spans the prism's extent.
	CHECK( BoxIs( c.Brushes()[0], 0, 0, 0, 128, 128, 128 ) );

	// The rendered solid is the true 5-faced shape, not a 6-faced box.
	const auto scene = c.BuildScene();
	CHECK( scene.solids.size() == 1 );
	if ( !scene.solids.empty() )
	{
		CHECK( scene.solids[0].faces.size() == 5 );
	}

	// Saving and reloading preserves the shape (still 5 faces), not a box.
	EditorController c2;
	CHECK( c2.LoadVmf( c.ToVmf(), error ) );
	CHECK( c2.Brushes().size() == 1 );
	if ( !c2.Brushes().empty() )
	{
		CHECK( c2.Brushes()[0].planes.size() == 5 );
		CHECK( c2.BuildScene().solids[0].faces.size() == 5 );
	}
}

// The headline flow: build a simple two-brush map with simulated input, save it
// to VMF, and prove it round-trips back to the same geometry.
void TestBuildSimpleMapAndSave()
{
	EditorController c;
	c.SetGridSize( 64 );

	CHECK( BlockDrag( c, ViewId::Top, 0, 0, 512, 512 ) );     // floor footprint
	CHECK( BlockDrag( c, ViewId::Front, 0, 128, 512, 384 ) ); // a wall above it
	CHECK( c.Brushes().size() == 2 );

	const std::string vmf = c.ToVmf();

	// The serialised document is a well-formed VMF: world + two solids of 6 sides.
	hammer::formats::ParseResult pr = hammer::formats::ParseKeyValues( vmf );
	CHECK( pr.ok );
	int worldSolids = 0;
	int totalSides = 0;
	for ( const auto &block : pr.root.children )
	{
		if ( block.name == "world" )
		{
			for ( const auto &child : block.children )
			{
				if ( child.name == "solid" )
				{
					++worldSolids;
					for ( const auto &s : child.children )
					{
						if ( s.name == "side" )
						{
							++totalSides;
						}
					}
				}
			}
		}
	}
	CHECK( worldSolids == 2 );
	CHECK( totalSides == 12 );

	// Reloading reproduces the same two brushes and is an unmodified base state.
	EditorController loaded;
	std::string error;
	CHECK( loaded.LoadVmf( vmf, error ) );
	CHECK( error.empty() );
	CHECK( loaded.Brushes().size() == 2 );
	CHECK( !loaded.IsModified() );

	// Bounds survive the round-trip (order preserved).
	if ( loaded.Brushes().size() == 2 && c.Brushes().size() == 2 )
	{
		for ( std::size_t i = 0; i < 2; ++i )
		{
			CHECK( BoxIs( loaded.Brushes()[i], c.Brushes()[i].mins.x, c.Brushes()[i].mins.y,
			    c.Brushes()[i].mins.z, c.Brushes()[i].maxs.x, c.Brushes()[i].maxs.y,
			    c.Brushes()[i].maxs.z ) );
		}
	}
}

} // namespace

int main()
{
	TestBlockCreateAndSnap();
	TestMultiViewExtrude();
	TestSelectMoveDelete();
	TestUndoRedo();
	TestGuardsRejectBadInput();
	TestLoadNonBoxPreservesShape();
	TestBuildSimpleMapAndSave();

	if ( g_failures != 0 )
	{
		std::printf( "hammer.app.editor_controller: %d check(s) FAILED\n", g_failures );
		return 1;
	}
	std::printf( "hammer.app.editor_controller: all checks passed\n" );
	return 0;
}
