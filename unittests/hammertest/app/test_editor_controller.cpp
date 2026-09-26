//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Conformance UI test for the headless editor interaction authority
//			(RFC 0002, HAM-WORKFLOW-001 / hammer.app). It drives EditorController
//			with SIMULATED normalized input -- the exact calls the GTK presenter
//			makes from gestures/keys -- to exercise the core Hammer UX flows and
//			builds a simple map end to end: Block-tool create with grid snapping,
//			multi-view extrusion, Selection-tool pick/move/delete, undo/redo, and
//			VMF save round-trip. It also builds a complete little room FROM SCRATCH
//			through simulated input -- four walls (Block tool), a wall texture
//			applied with the Material tool, and an info_player_start plus a
//			weapon_portalgun placed with the Entity tool -- then proves the saved
//			VMF round-trips every brush, material, and entity. Because the shipped UI
//			routes through this same authority, this test covers the real editing
//			logic, not a mock.
//
//			Build/run: unittests/hammertest/run_headless.sh
//
//=============================================================================//

#include "hammer/app/editor_controller.h"
#include "hammer/formats/keyvalues.h"
#include "hammer/geometry/texture_axes.h"
#include "testing/conformance_result.h"

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
int g_checks = 0;

void Check( bool condition, const char *expression, int line )
{
	++g_checks;
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

// Counts the top-level VMF blocks named 'name' in a parsed document.
int CountBlocks( const hammer::formats::KeyValueNode &root, const char *name )
{
	int n = 0;
	for ( const auto &block : root.children )
	{
		if ( block.name == name )
		{
			++n;
		}
	}
	return n;
}

// THE GOAL FLOW: build a complete little room from scratch with simulated input --
// four textured walls, an info_player_start and a weapon_portalgun -- then save it
// to VMF and prove the whole thing round-trips (geometry, materials, entities).
void TestBuildRoomFromScratch()
{
	using hammer::app::MapEntity;

	EditorController c;
	c.SetGridSize( 64 );
	c.SetBlockDepth( 256 ); // walls 256 units tall

	// --- Four walls around a 512x512 footprint, drawn in the Top (X/Y) view. Each
	//     wall is a 64-thick box; drawing wall centres apart keeps them pickable. ---
	CHECK( BlockDrag( c, ViewId::Top, 0, 0, 512, 64 ) );    // south (Y 0..64)
	CHECK( BlockDrag( c, ViewId::Top, 0, 448, 512, 512 ) ); // north (Y 448..512)
	CHECK( BlockDrag( c, ViewId::Top, 0, 0, 64, 512 ) );    // west  (X 0..64)
	CHECK( BlockDrag( c, ViewId::Top, 448, 0, 512, 512 ) ); // east  (X 448..512)
	CHECK( c.Brushes().size() == 4 );

	// --- Texture the walls: pick a wall material and apply it with the Material
	//     tool by clicking each wall's centre (the exact presenter gesture). ---
	const std::string kWall = "BRICK/BRICKWALL001A";
	c.SetActiveMaterial( kWall );
	c.SetTool( Tool::Material );
	const double wallCentres[4][2] = {
	    { 256, 32 },  // south
	    { 256, 480 }, // north
	    { 32, 256 },  // west
	    { 480, 256 }, // east
	};
	for ( const auto &p : wallCentres )
	{
		c.PointerDown( ViewId::Top, p[0], p[1] );
		c.PointerUp( ViewId::Top, p[0], p[1] );
	}
	// Every face of every wall now carries the wall material.
	for ( const auto &b : c.Brushes() )
	{
		CHECK( !b.materials.empty() );
		for ( const auto &m : b.materials )
		{
			CHECK( m == kWall );
		}
	}

	// --- Place the two point entities with the Entity tool (a click each). ---
	c.SetTool( Tool::Entity );
	c.SetEntityClass( "info_player_start" );
	c.PointerDown( ViewId::Top, 256, 256 ); // room centre
	c.PointerUp( ViewId::Top, 256, 256 );
	CHECK( c.SelectedEntity().has_value() );

	c.SetEntityClass( "weapon_portalgun" );
	c.PointerDown( ViewId::Top, 128, 256 );
	c.PointerUp( ViewId::Top, 128, 256 );
	CHECK( c.Entities().size() == 2 );

	// Give the spawn point a name -- exercises the entity property owner + history.
	int spawnId = 0;
	for ( const MapEntity &e : c.Entities() )
	{
		if ( e.classname == "info_player_start" )
		{
			spawnId = e.id;
		}
	}
	CHECK( spawnId != 0 );
	CHECK( c.SetEntityProperty( spawnId, "targetname", "spawn1" ) );
	CHECK( !c.SetEntityProperty( spawnId, "targetname", "spawn1" ) ); // no-op, no history

	// The renderable scene sees four wall solids and two entity markers.
	const auto scene = c.BuildScene();
	CHECK( scene.solids.size() == 4 );
	CHECK( scene.entities.size() == 2 );

	// --- Save to VMF and validate structure directly. ---
	const std::string vmf = c.ToVmf();
	hammer::formats::ParseResult pr = hammer::formats::ParseKeyValues( vmf );
	CHECK( pr.ok );
	CHECK( CountBlocks( pr.root, "world" ) == 1 );
	CHECK( CountBlocks( pr.root, "entity" ) == 2 );

	int worldSolids = 0;
	int wallSides = 0;
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
							const std::string *mat = s.Find( "material" );
							if ( mat && *mat == kWall )
							{
								++wallSides;
							}
						}
					}
				}
			}
		}
	}
	CHECK( worldSolids == 4 );
	CHECK( wallSides == 24 ); // 4 walls * 6 faces, all the wall material

	// Both entities are present with the right classnames and named spawn.
	bool sawSpawn = false;
	bool sawGun = false;
	for ( const auto &block : pr.root.children )
	{
		if ( block.name != "entity" )
		{
			continue;
		}
		const std::string *cls = block.Find( "classname" );
		const std::string *origin = block.Find( "origin" );
		CHECK( cls != nullptr );
		CHECK( origin != nullptr );
		if ( cls && *cls == "info_player_start" )
		{
			sawSpawn = true;
			const std::string *name = block.Find( "targetname" );
			CHECK( name != nullptr && *name == "spawn1" );
		}
		if ( cls && *cls == "weapon_portalgun" )
		{
			sawGun = true;
		}
	}
	CHECK( sawSpawn );
	CHECK( sawGun );

	// --- Round-trip: reload the saved VMF and confirm everything survives. ---
	EditorController loaded;
	std::string error;
	CHECK( loaded.LoadVmf( vmf, error ) );
	CHECK( error.empty() );
	CHECK( !loaded.IsModified() );
	CHECK( loaded.Brushes().size() == 4 );
	CHECK( loaded.Entities().size() == 2 );

	// Reloaded walls keep their material.
	for ( const auto &b : loaded.Brushes() )
	{
		for ( const auto &m : b.materials )
		{
			CHECK( m == kWall );
		}
	}

	// Reloaded entities keep classname, origin, and the spawn's name.
	bool reSpawn = false;
	bool reGun = false;
	for ( const MapEntity &e : loaded.Entities() )
	{
		if ( e.classname == "info_player_start" )
		{
			reSpawn = true;
			CHECK( Near( e.origin.x, 256 ) && Near( e.origin.y, 256 ) && Near( e.origin.z, 0 ) );
			bool named = false;
			for ( const auto &p : e.properties )
			{
				if ( p.key == "targetname" && p.value == "spawn1" )
				{
					named = true;
				}
			}
			CHECK( named );
		}
		if ( e.classname == "weapon_portalgun" )
		{
			reGun = true;
			CHECK( Near( e.origin.x, 128 ) && Near( e.origin.y, 256 ) );
		}
	}
	CHECK( reSpawn );
	CHECK( reGun );
}

// Placing an entity, then deleting it, must undo/redo cleanly through the shared
// history -- the entity tool routes through the same one authority as brushes.
void TestEntityPlaceDeleteUndo()
{
	EditorController c;
	c.SetGridSize( 64 );

	c.SetTool( Tool::Entity );
	c.SetEntityClass( "info_player_start" );
	c.PointerDown( ViewId::Top, 128, 128 );
	c.PointerUp( ViewId::Top, 128, 128 );
	CHECK( c.Entities().size() == 1 );
	CHECK( c.IsModified() );

	// Select it with the Selection tool (nearest within tolerance) and delete it.
	c.SetTool( Tool::Select );
	c.PointerDown( ViewId::Top, 128, 128 );
	c.PointerUp( ViewId::Top, 128, 128 );
	CHECK( c.SelectedEntity().has_value() );
	CHECK( c.DeleteSelection() );
	CHECK( c.Entities().empty() );

	// Undo restores the entity; a second undo removes the placement (empty base).
	CHECK( c.Undo() );
	CHECK( c.Entities().size() == 1 );
	CHECK( c.Undo() );
	CHECK( c.Entities().empty() );
	CHECK( !c.IsModified() );

	// Redo replays placement then deletion.
	CHECK( c.Redo() );
	CHECK( c.Entities().size() == 1 );
	CHECK( c.Redo() );
	CHECK( c.Entities().empty() );
}

// A loaded map's displacement (dispinfo) surfaces must be carried through
// BuildScene() so the interactive viewport renders terrain, and cleared by NewMap.
void TestLoadDisplacementCarries()
{
	const char *kVmf =
	    "world\n{\n\t\"id\" \"1\"\n\t\"classname\" \"worldspawn\"\n"
	    "\tsolid\n\t{\n\t\t\"id\" \"2\"\n"
	    "\t\tside\n\t\t{\n\t\t\t\"plane\" \"(0 64 64) (64 64 64) (64 0 64)\"\n"
	    "\t\t\t\"material\" \"DEV/A\"\n"
	    "\t\t\tdispinfo\n\t\t\t{\n\t\t\t\t\"power\" \"1\"\n\t\t\t\t\"startposition\" \"[0 0 64]\"\n"
	    "\t\t\t\t\"elevation\" \"0\"\n"
	    "\t\t\t\tnormals\n\t\t\t\t{\n\t\t\t\t\t\"row0\" \"0 0 1 0 0 1 0 0 1\"\n"
	    "\t\t\t\t\t\"row1\" \"0 0 1 0 0 1 0 0 1\"\n\t\t\t\t\t\"row2\" \"0 0 1 0 0 1 0 0 "
	    "1\"\n\t\t\t\t}\n"
	    "\t\t\t\tdistances\n\t\t\t\t{\n\t\t\t\t\t\"row0\" \"0 0 0\"\n"
	    "\t\t\t\t\t\"row1\" \"0 32 0\"\n\t\t\t\t\t\"row2\" \"0 0 0\"\n\t\t\t\t}\n\t\t\t}\n\t\t}\n"
	    "\t\tside { \"plane\" \"(0 0 0) (64 0 0) (64 64 0)\" \"material\" \"DEV/B\" }\n"
	    "\t\tside { \"plane\" \"(64 0 0) (64 0 64) (64 64 64)\" \"material\" \"DEV/C\" }\n"
	    "\t\tside { \"plane\" \"(0 64 0) (0 64 64) (0 0 64)\" \"material\" \"DEV/D\" }\n"
	    "\t\tside { \"plane\" \"(64 64 0) (64 64 64) (0 64 64)\" \"material\" \"DEV/E\" }\n"
	    "\t\tside { \"plane\" \"(0 0 0) (0 0 64) (64 0 64)\" \"material\" \"DEV/F\" }\n"
	    "\t}\n}\n";

	EditorController c;
	std::string error;
	CHECK( c.LoadVmf( kVmf, error ) );
	const auto scene = c.BuildScene();
	CHECK( scene.solids.size() == 1 );        // the box brush
	CHECK( scene.displacements.size() == 1 ); // its displaced top face, carried through
	if ( scene.displacements.size() == 1 )
	{
		CHECK( scene.displacements[0].vertices.size() == 9 );  // (2^1+1)^2
		CHECK( scene.displacements[0].triangles.size() == 8 ); // 2*(2^1)^2
	}
	// NewMap clears carried displacements.
	c.NewMap();
	CHECK( c.BuildScene().displacements.empty() );
}

// Exact domain operations (the command layer's authority): value-addressed
// block and entity authoring, one undo unit each, and the same selection the
// gestures give.
void TestExactDomainOperations()
{
	EditorController c;
	c.NewMap();
	CHECK( !c.CreateBlock( Vec3d( 0, 0, 0 ), Vec3d( 0, 64, 64 ) ) ); // degenerate
	CHECK( !c.CanUndo() );
	const auto block = c.CreateBlock( Vec3d( -64, -64, -16 ), Vec3d( 64, 64, 0 ) );
	CHECK( block.has_value() && c.Brushes().size() == 1 );
	CHECK( block && c.Selection() == *block && !c.SelectedEntity() );
	const auto start = c.PlaceEntity( "info_player_start", Vec3d( 0, 0, 1 ) );
	CHECK( start.has_value() && c.Entities().size() == 1 );
	CHECK( start && c.SelectedEntity() == *start && !c.Selection() );
	CHECK( !c.PlaceEntity( "", Vec3d( 0, 0, 0 ) ) );
	CHECK( start && c.SetEntityOrigin( *start, Vec3d( 8, 8, 1 ) ) );
	CHECK( start && !c.SetEntityOrigin( *start, Vec3d( 8, 8, 1 ) ) ); // no-op
	CHECK( !c.SetEntityOrigin( 9999, Vec3d( 0, 0, 0 ) ) );
	CHECK( c.Entities().size() == 1 && c.Entities()[0].origin.x == 8 );
	CHECK( c.Undo() && c.Entities()[0].origin.x == 0 ); // origin edit undone
	CHECK( c.Undo() && c.Entities().empty() );         // placement undone
	CHECK( c.Undo() && c.Brushes().empty() );          // block undone
	CHECK( !c.Undo() );
	CHECK( c.Redo() && c.Redo() && c.Brushes().size() == 1 && c.Entities().size() == 1 );
}

// Worldspawn properties are undoable document state and round-trip through VMF.
void TestWorldPropertiesRoundTrip()
{
	EditorController c;
	c.NewMap();
	CHECK( c.WorldProperties().size() == 1 && c.WorldProperties()[0].key == "skyname" );
	CHECK( !c.SetWorldProperty( "classname", "func_detail" ) );
	CHECK( !c.SetWorldProperty( "skyname", "sky_day01_01" ) ); // unchanged
	CHECK( c.SetWorldProperty( "skyname", "sky_black" ) );
	CHECK( c.SetWorldProperty( "detailvbsp", "detail.vbsp" ) );
	CHECK( c.Undo() && c.WorldProperties().size() == 1 );
	CHECK( c.Redo() && c.WorldProperties().size() == 2 );
	// A brush makes the reload rebuild plane normals; the save must stay byte
	// identical (no "-0" components).
	CHECK( c.CreateBlock( Vec3d( 0, 0, 0 ), Vec3d( 8, 8, 8 ) ).has_value() );
	const std::string vmf = c.ToVmf();
	CHECK( vmf.find( "\"skyname\" \"sky_black\"" ) != std::string::npos );
	EditorController reloaded;
	std::string error;
	CHECK( reloaded.LoadVmf( vmf, error ) );
	CHECK( reloaded.WorldProperties().size() == 2 );
	CHECK( reloaded.WorldProperties().size() == 2 &&
	       reloaded.WorldProperties()[0].value == "sky_black" );
	CHECK( reloaded.ToVmf() == vmf );
}

// The texture-axis owner reproduces legacy Hammer's table and tie order: a
// tie keeps the earlier entry (floor before walls, x walls before y walls).
void TestWorldAlignedTextureAxesPolicy()
{
	using hammer::geometry::WorldAlignedTextureAxes;
	auto same = []( const Vec3d &a, double x, double y, double z )
	{ return a.x == x && a.y == y && a.z == z; };
	const double h = std::sqrt( 0.5 );
	auto floor = WorldAlignedTextureAxes( Vec3d( 0, 0, 1 ) );
	CHECK( same( floor.u, 1, 0, 0 ) && same( floor.v, 0, -1, 0 ) );
	auto ceiling = WorldAlignedTextureAxes( Vec3d( 0, 0, -1 ) );
	CHECK( same( ceiling.u, 1, 0, 0 ) && same( ceiling.v, 0, -1, 0 ) );
	auto east = WorldAlignedTextureAxes( Vec3d( -1, 0, 0 ) );
	CHECK( same( east.u, 0, 1, 0 ) && same( east.v, 0, 0, -1 ) );
	auto north = WorldAlignedTextureAxes( Vec3d( 0, -1, 0 ) );
	CHECK( same( north.u, 1, 0, 0 ) && same( north.v, 0, 0, -1 ) );
	auto floorTie = WorldAlignedTextureAxes( Vec3d( h, 0, h ) );
	CHECK( same( floorTie.u, 1, 0, 0 ) && same( floorTie.v, 0, -1, 0 ) );
	auto wallTie = WorldAlignedTextureAxes( Vec3d( h, h, 0 ) );
	CHECK( same( wallTie.u, 0, 1, 0 ) && same( wallTie.v, 0, 0, -1 ) );
	auto none = WorldAlignedTextureAxes( Vec3d( 0, 0, 0 ) );
	CHECK( same( none.u, 1, 0, 0 ) && same( none.v, 0, -1, 0 ) );
}

// Each saved side gets legacy Hammer's world-aligned axes for its own normal,
// not the floor's projection on every face.
void TestSavedTextureAxesFollowFaceNormal()
{
	EditorController c;
	c.NewMap();
	CHECK( c.CreateBlock( Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) ).has_value() );
	hammer::formats::ParseResult parsed = hammer::formats::ParseKeyValues( c.ToVmf() );
	CHECK( parsed.ok );
	int sides = 0;
	int walls = 0;
	for ( const auto &block : parsed.root.children )
	{
		if ( block.name != "world" )
			continue;
		for ( const auto &solid : block.children )
		{
			for ( const auto &side : solid.children )
			{
				const std::string *u = side.Find( "uaxis" );
				const std::string *v = side.Find( "vaxis" );
				CHECK( u != nullptr && v != nullptr );
				if ( u == nullptr || v == nullptr )
					continue;
				++sides;
				// Floors and ceilings use [1 0 0]/[0 -1 0]; walls project onto a
				// vertical v axis [0 0 -1].
				if ( *v == "[0 0 -1 0] 0.25" )
				{
					++walls;
					CHECK( *u == "[0 1 0 0] 0.25" || *u == "[1 0 0 0] 0.25" );
				}
				else
				{
					CHECK( *u == "[1 0 0 0] 0.25" && *v == "[0 -1 0 0] 0.25" );
				}
			}
		}
	}
	CHECK( sides == 6 && walls == 4 );
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
	TestBuildRoomFromScratch();
	TestEntityPlaceDeleteUndo();
	TestLoadDisplacementCarries();
	TestExactDomainOperations();
	TestWorldPropertiesRoundTrip();
	TestSavedTextureAxesFollowFaceNormal();
	TestWorldAlignedTextureAxesPolicy();

	if ( g_failures != 0 )
	{
		std::printf( "hammer.app.editor_controller: %d check(s) FAILED\n", g_failures );
		return testing::ReportConformance( g_checks, g_failures );
	}
	std::printf( "hammer.app.editor_controller: all checks passed\n" );
	return testing::ReportConformance( g_checks, g_failures );
}
