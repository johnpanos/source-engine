//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.viewport render-snapshot extraction (RFC 0002, R08 domain
//			logic; contract viewport.extraction.v1): face draws (side ids,
//			materials, outward counter-clockwise polygons), the color policy
//			(editor, owner class, defaults; class before editor for entities),
//			selection flags through containers and selected faces, hidden
//			filtering and keepHidden, entity display hints, SnapshotCache
//			incremental updates equal to a full rebuild (create/modify/remove,
//			undo, selection-only, a generated edit sequence), and 2D edge
//			projection with shared edges once. Negative checks: stale caches
//			are detected by the equality oracle, mismatched revisions rebuild,
//			degenerate input draws nothing.
//
//=============================================================================//

#include "hammer/scene/change_set.h"
#include "hammer/scene/solid_geometry.h"
#include "hammer/viewport/extraction.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

#include <algorithm>
#include <cctype>
#include <cstdint>

using namespace hammer::viewport;
using hammer::scene::Box;
using hammer::scene::ChangeSet;
using hammer::scene::DocumentEdit;
using hammer::scene::Entity;
using hammer::scene::FaceRef;
using hammer::scene::FaceTexture;
using hammer::scene::Group;
using hammer::scene::MapDocument;
using hammer::scene::ObjectId;
using hammer::scene::Rgb;
using hammer::scene::Solid;
using mapgeometry::Vec3d;
namespace ports = hammer::ports;

namespace
{

class LocalCatalog final : public ports::IEntityCatalog
{
public:
	void Add( ports::EntityClassInfo info ) { m_classes.push_back( std::move( info ) ); }

	const ports::EntityClassInfo *Find( std::string_view name ) const override
	{
		for ( const ports::EntityClassInfo &info : m_classes )
		{
			if ( std::equal( info.name.begin(), info.name.end(), name.begin(), name.end(),
			         []( char a, char b )
			         {
				         return std::tolower( static_cast<unsigned char>( a ) ) ==
				                std::tolower( static_cast<unsigned char>( b ) );
			         } ) )
			{
				return &info;
			}
		}
		return nullptr;
	}

	std::vector<std::string> ClassNames() const override
	{
		std::vector<std::string> names;
		for ( const ports::EntityClassInfo &info : m_classes )
		{
			names.push_back( info.name );
		}
		return names;
	}

private:
	std::vector<ports::EntityClassInfo> m_classes;
};

LocalCatalog MakeCatalog()
{
	LocalCatalog catalog;
	ports::EntityClassInfo door;
	door.name = "func_door";
	door.kind = ports::EntityClassKind::Solid;
	door.color = Vec3d( 0, 255, 0 );
	catalog.Add( door );
	ports::EntityClassInfo light;
	light.name = "light";
	light.color = Vec3d( 255, 255, 0 );
	light.boxMins = Vec3d( -8, -8, -8 );
	light.boxMaxs = Vec3d( 8, 8, 8 );
	light.sprite = "editor/light.vmt";
	catalog.Add( light );
	ports::EntityClassInfo start;
	start.name = "info_player_start";
	start.color = Vec3d( 0, 255, 300 ); // clamped to 255
	start.boxMins = Vec3d( -16, -16, 0 );
	start.boxMaxs = Vec3d( 16, 16, 72 );
	start.model = "models/editor/playerstart.mdl";
	catalog.Add( start );
	return catalog;
}

Solid BoxSolid( Vec3d mins, Vec3d maxs, const char *material = "BRICK/BRICKWALL001" )
{
	FaceTexture texture;
	texture.material = material;
	return hammer::scene::MakeBoxSolid( Box{ mins, maxs }, texture );
}

Entity PointEntity( const char *classname, Vec3d origin )
{
	Entity entity;
	entity.classname = classname;
	entity.SetOrigin( origin );
	return entity;
}

struct Fixture
{
	MapDocument doc;
	ObjectId colored, plain, door, doorSolid, wall, wallSolid, lamp, target, bare, prop, start,
	    hidden, group, groupedSolid;
};

void Build( Fixture &f )
{
	DocumentEdit edit( f.doc );
	Solid colored = BoxSolid( Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) );
	colored.editor.color = Rgb{ 10, 20, 30 };
	f.colored = edit.Add( colored );
	f.plain =
	    edit.Add( BoxSolid( Vec3d( 100, 0, 0 ), Vec3d( 164, 64, 64 ), "DEV/DEV_BLENDMEASURE" ) );
	Entity door;
	door.classname = "func_door";
	f.door = edit.Add( door );
	Solid ds = BoxSolid( Vec3d( 200, 0, 0 ), Vec3d( 264, 16, 128 ) );
	ds.owner = f.door;
	f.doorSolid = edit.Add( ds );
	Entity wall;
	wall.classname = "func_wall";
	f.wall = edit.Add( wall );
	Solid ws = BoxSolid( Vec3d( 300, 0, 0 ), Vec3d( 316, 64, 64 ) );
	ws.owner = f.wall;
	f.wallSolid = edit.Add( ws );
	Entity lamp = PointEntity( "light", Vec3d( 32, 32, 200 ) );
	lamp.editor.color = Rgb{ 1, 2, 3 };
	lamp.SetAngles( Vec3d( -90, 0, 0 ) );
	f.lamp = edit.Add( lamp );
	Entity target = PointEntity( "info_target", Vec3d( 500, 0, 0 ) );
	target.editor.color = Rgb{ 4, 5, 6 };
	f.target = edit.Add( target );
	f.bare = edit.Add( PointEntity( "info_null", Vec3d( 600, 0, 0 ) ) );
	Entity prop = PointEntity( "prop_static", Vec3d( 700, 0, 0 ) );
	prop.SetKey( "model", "models/props/crate.mdl" );
	f.prop = edit.Add( prop );
	f.start = edit.Add( PointEntity( "info_player_start", Vec3d( 800, 0, 0 ) ) );
	Solid hidden = BoxSolid( Vec3d( 0, 500, 0 ), Vec3d( 64, 564, 64 ) );
	hidden.hidden = true;
	f.hidden = edit.Add( hidden );
	Group group;
	group.hidden = true;
	f.group = edit.Add( group );
	Solid grouped = BoxSolid( Vec3d( 0, 700, 0 ), Vec3d( 64, 764, 64 ) );
	grouped.group = f.group;
	f.groupedSolid = edit.Add( grouped );
	hammer::scene::CommitEdit( f.doc, edit );
}

const SolidDraw *FindSolid( const RenderSnapshot &snapshot, ObjectId id )
{
	for ( const SolidDraw &draw : snapshot.solids )
	{
		if ( draw.id == id )
		{
			return &draw;
		}
	}
	return nullptr;
}

const EntityDraw *FindEntity( const RenderSnapshot &snapshot, ObjectId id )
{
	for ( const EntityDraw &draw : snapshot.entities )
	{
		if ( draw.id == id )
		{
			return &draw;
		}
	}
	return nullptr;
}

std::uint32_t SideFacing( const MapDocument &doc, ObjectId solid, Vec3d normal )
{
	for ( const hammer::scene::Side &side : doc.FindSolid( solid )->sides )
	{
		if ( mapgeometry::NearlyEqual( side.Plane().normal, normal, 1.0e-9 ) )
		{
			return side.vmfId;
		}
	}
	return 0;
}

void Content( testing::Checks &checks, const Fixture &f, const LocalCatalog &catalog )
{
	ExtractOptions options;
	options.catalog = &catalog;
	const RenderSnapshot snapshot = Extract( f.doc, {}, options );

	checks.Equal( snapshot.solids.size(), std::size_t( 4 ), "four shown solids" );
	checks.That( std::is_sorted( snapshot.solids.begin(), snapshot.solids.end(),
	                 []( const SolidDraw &a, const SolidDraw &b )
	                 {
		                 return a.id < b.id;
	                 } ),
	    "solids in id order" );
	const SolidDraw *colored = FindSolid( snapshot, f.colored );
	checks.That( colored && colored->faces.size() == 6, "a box draws six faces" );
	bool ccw = colored != nullptr;
	bool sidesKnown = colored != nullptr;
	if ( colored )
	{
		for ( const FaceDraw &face : colored->faces )
		{
			const Vec3d winding = mapgeometry::Cross(
			    face.vertices[1] - face.vertices[0], face.vertices[2] - face.vertices[0] );
			ccw = ccw && mapgeometry::Dot( winding, face.normal ) > 0.0;
			sidesKnown = sidesKnown && face.side != 0 &&
			             face.side == SideFacing( f.doc, f.colored, face.normal ) &&
			             face.material == "BRICK/BRICKWALL001";
		}
		checks.That(
		    colored->bounds == Box{ Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) }, "solid bounds" );
		checks.That( !colored->owner.IsValid() && !colored->selected && !colored->hidden,
		    "world solid flags" );
	}
	checks.That( ccw, "face vertices are counter-clockwise from outside" );
	checks.That( sidesKnown, "faces carry their side VMF id and material" );

	// Color policy.
	checks.That( colored && colored->color == Rgb{ 10, 20, 30 }, "editor color wins for solids" );
	const SolidDraw *plain = FindSolid( snapshot, f.plain );
	checks.That(
	    plain && plain->color == kDefaultWorldColor, "uncolored world solid: default world color" );
	const SolidDraw *doorSolid = FindSolid( snapshot, f.doorSolid );
	checks.That( doorSolid && doorSolid->color == Rgb{ 0, 255, 0 } && doorSolid->owner == f.door,
	    "brush-entity solid: owner's class color" );
	const SolidDraw *wallSolid = FindSolid( snapshot, f.wallSolid );
	checks.That( wallSolid && wallSolid->color == kDefaultEntityColor,
	    "owner class without color: default entity color" );
	const RenderSnapshot uncataloged = Extract( f.doc, {} );
	checks.That( FindSolid( uncataloged, f.doorSolid )->color == kDefaultEntityColor,
	    "no catalog: brush-entity solids use the default entity color" );

	const EntityDraw *lamp = FindEntity( snapshot, f.lamp );
	checks.That(
	    lamp && lamp->color == Rgb{ 255, 255, 0 }, "entity class color wins over editor color" );
	checks.That( lamp && lamp->mins == Vec3d( 24, 24, 192 ) && lamp->maxs == Vec3d( 40, 40, 208 ),
	    "marker box from the catalog" );
	checks.That( lamp && lamp->angles == Vec3d( -90, 0, 0 ) && lamp->origin == Vec3d( 32, 32, 200 ),
	    "origin and angles" );
	checks.That(
	    lamp && lamp->sprite == "editor/light.vmt" && lamp->model.empty(), "catalog sprite" );
	const EntityDraw *target = FindEntity( snapshot, f.target );
	checks.That( target && target->color == Rgb{ 4, 5, 6 }, "no class color: editor color" );
	checks.That( target && target->mins == Vec3d( 492, -8, -8 ), "no class box: +/-8" );
	const EntityDraw *bare = FindEntity( snapshot, f.bare );
	checks.That( bare && bare->color == kDefaultEntityColor && bare->angles == Vec3d(),
	    "no colors: default entity color, zero angles" );
	const EntityDraw *prop = FindEntity( snapshot, f.prop );
	checks.That( prop && prop->model == "models/props/crate.mdl", "model key" );
	const EntityDraw *start = FindEntity( snapshot, f.start );
	checks.That( start && start->model == "models/editor/playerstart.mdl" &&
	                 start->color == Rgb{ 0, 255, 255 },
	    "catalog model; catalog color clamped to 255" );
	checks.That( !FindEntity( snapshot, f.door ) && !FindEntity( snapshot, f.wall ),
	    "brush entities have no entity draw" );
	checks.Equal( snapshot.entities.size(), std::size_t( 5 ), "five point entities" );

	// Hidden filtering and bounds.
	checks.That( !FindSolid( snapshot, f.hidden ), "quick-hidden solid filtered (negative)" );
	checks.That(
	    !FindSolid( snapshot, f.groupedSolid ), "member of a hidden group filtered (negative)" );
	checks.That( snapshot.bounds && snapshot.bounds->mins == Vec3d( 0, -16, -8 ) &&
	                 snapshot.bounds->maxs == Vec3d( 816, 64, 208 ),
	    "snapshot bounds cover what is shown" );
	ExtractOptions keep = options;
	keep.keepHidden = true;
	const RenderSnapshot kept = Extract( f.doc, {}, keep );
	const SolidDraw *hidden = FindSolid( kept, f.hidden );
	checks.That( hidden && hidden->hidden && FindSolid( kept, f.groupedSolid ) &&
	                 FindSolid( kept, f.groupedSolid )->hidden,
	    "keepHidden includes hidden objects flagged" );
	checks.That( kept.bounds == snapshot.bounds, "hidden objects stay out of the bounds" );
	ExtractOptions all = options;
	all.visible = ShowEverything();
	const SolidDraw *shown = FindSolid( Extract( f.doc, {}, all ), f.hidden );
	checks.That( shown && !shown->hidden, "a predicate override shows hidden objects" );

	// Selection.
	SelectionInput selection;
	selection.objects = { f.door, f.lamp, f.group };
	const std::uint32_t topSide = SideFacing( f.doc, f.colored, Vec3d( 0, 0, 1 ) );
	selection.faces = { FaceRef{ f.colored, topSide } };
	const RenderSnapshot selected = Extract( f.doc, selection, keep );
	checks.That( FindSolid( selected, f.doorSolid )->selected,
	    "selecting a brush entity selects its solids" );
	checks.That( FindEntity( selected, f.lamp )->selected, "selected point entity" );
	checks.That(
	    FindSolid( selected, f.groupedSolid )->selected, "selecting a group selects its members" );
	checks.That( !FindSolid( selected, f.wallSolid )->selected &&
	                 !FindEntity( selected, f.target )->selected,
	    "unselected objects stay unselected" );
	const SolidDraw *withFace = FindSolid( selected, f.colored );
	int selectedFaces = 0;
	for ( const FaceDraw &face : withFace->faces )
	{
		selectedFaces += face.selected ? 1 : 0;
		if ( face.selected )
		{
			checks.That( face.side == topSide, "the selected face is the top side" );
		}
	}
	checks.Equal( selectedFaces, 1, "exactly one face selected" );
	checks.That( !withFace->selected, "face selection does not select the solid" );
	ObjectId stale;
	stale.value = 0xDEAD;
	selection.objects.push_back( stale );
	selection.objects.push_back( f.door ); // duplicate
	checks.That( Extract( f.doc, selection, keep ) == selected,
	    "unknown and duplicate ids are ignored (negative)" );

	// A solid whose sides bound nothing draws nothing.
	MapDocument broken;
	{
		DocumentEdit edit( broken );
		Solid degenerate = BoxSolid( Vec3d( 0, 0, 0 ), Vec3d( 16, 16, 16 ) );
		degenerate.sides.resize( 2 );
		edit.Add( degenerate );
		hammer::scene::CommitEdit( broken, edit );
	}
	const RenderSnapshot empty = Extract( broken, {} );
	checks.That( empty.solids.empty() && !empty.bounds, "an open solid draws nothing (negative)" );
}

// Oracle: the cache equals a full extraction of the same inputs.
bool MatchesFull( const SnapshotCache &cache, const MapDocument &doc,
    const SelectionInput &selection, const ExtractOptions &options )
{
	return cache.Snapshot() == Extract( doc, selection, options );
}

void Cache( testing::Checks &checks, Fixture &f, const LocalCatalog &catalog )
{
	ExtractOptions options;
	options.catalog = &catalog;
	// Unrelated filler, so a bounded update is measurably smaller than a rebuild.
	{
		DocumentEdit edit( f.doc );
		for ( int i = 0; i < 50; ++i )
		{
			edit.Add( BoxSolid( Vec3d( i * 16.0, -400, 0 ), Vec3d( i * 16.0 + 8, -392, 8 ) ) );
		}
		hammer::scene::CommitEdit( f.doc, edit );
	}
	SnapshotCache cache( options );
	checks.That( !cache.Built(), "a new cache is not built" );
	SelectionInput selection;
	selection.objects = { f.wall };
	cache.Rebuild( f.doc, selection, 1 );
	checks.That(
	    cache.Built() && cache.Revision() == 1 && MatchesFull( cache, f.doc, selection, options ),
	    "rebuild equals a full extraction" );
	const std::size_t total = cache.Snapshot().solids.size() + cache.Snapshot().entities.size();

	// Create, modify, remove; recolor through the owner; unhide a group;
	// make a point entity a brush entity.
	ChangeSet changes;
	ObjectId created;
	{
		DocumentEdit edit( f.doc );
		created = edit.Add( BoxSolid( Vec3d( 0, 0, 300 ), Vec3d( 32, 32, 332 ) ) );
		Solid *colored = edit.MutableSolid( f.colored );
		colored->editor.color = Rgb{ 200, 0, 0 };
		edit.Remove( f.plain );
		edit.MutableEntity( f.door )->classname = "func_wall";
		edit.MutableGroup( f.group )->hidden = false;
		Solid owned = BoxSolid( Vec3d( 490, -10, -10 ), Vec3d( 510, 10, 10 ) );
		owned.owner = f.target;
		edit.Add( owned );
		changes = hammer::scene::CommitEdit( f.doc, edit );
	}
	checks.That(
	    !changes.Created().empty() && !changes.Removed().empty() && !changes.Modified().empty(),
	    "the change set creates, modifies and removes" );
	checks.That( !MatchesFull( cache, f.doc, selection, options ),
	    "oracle detects the stale snapshot before the update (seeded)" );
	{
		SnapshotCache stale = cache;
		stale.Update( f.doc, ChangeSet{}, selection, 1, 2 );
		checks.That( !MatchesFull( stale, f.doc, selection, options ),
		    "an update that omits the change set stays stale and is detected (seeded)" );
	}
	checks.That(
	    cache.Update( f.doc, changes, selection, 1, 2 ) == SnapshotCache::UpdateKind::Incremental,
	    "matching base revision updates incrementally" );
	checks.That(
	    MatchesFull( cache, f.doc, selection, options ), "incremental update == full rebuild" );
	checks.That(
	    cache.LastRebuiltCount() < total, "the update re-extracted fewer objects than a rebuild" );
	checks.That( FindSolid( cache.Snapshot(), created ) &&
	                 !FindSolid( cache.Snapshot(), f.plain ) &&
	                 FindSolid( cache.Snapshot(), f.groupedSolid ) &&
	                 FindSolid( cache.Snapshot(), f.doorSolid )->color == kDefaultEntityColor &&
	                 !FindEntity( cache.Snapshot(), f.target ),
	    "created, removed, unhidden, recolored and newly brush-entity objects" );

	// Undo through the same change set.
	hammer::scene::Apply( f.doc, changes, hammer::scene::ApplyDirection::Backward );
	cache.Update( f.doc, changes, selection, 2, 3 );
	checks.That( MatchesFull( cache, f.doc, selection, options ), "undo: incremental == full" );
	checks.That( FindSolid( cache.Snapshot(), f.plain ) && FindEntity( cache.Snapshot(), f.target ),
	    "undo restores the removed solid and the point entity" );

	// Selection only.
	selection.objects = { f.door, f.group };
	selection.faces = { FaceRef{ f.colored, SideFacing( f.doc, f.colored, Vec3d( 1, 0, 0 ) ) } };
	cache.SetSelection( f.doc, selection );
	checks.That( MatchesFull( cache, f.doc, selection, options ) && cache.Revision() == 3,
	    "selection-only update == full, revision unchanged" );
	// {wall} -> {door, group}: wall, door, group and their leaves, plus the face's solid.
	checks.Equal( cache.LastRebuiltCount(), std::size_t( 7 ),
	    "selection change touches only affected objects" );

	// Mismatched revision rebuilds everything.
	checks.That(
	    cache.Update( f.doc, ChangeSet{}, selection, 99, 100 ) == SnapshotCache::UpdateKind::Full &&
	        cache.Revision() == 100 && MatchesFull( cache, f.doc, selection, options ),
	    "a base-revision mismatch rebuilds (negative)" );

	// A generated sequence of edits, compared with a full rebuild each step.
	std::uint32_t seed = 12345u;
	const auto next = [&]()
	{
		return seed = seed * 1664525u + 1013904223u;
	};
	bool allMatch = true;
	std::uint64_t revision = cache.Revision();
	for ( int step = 0; step < 40; ++step )
	{
		DocumentEdit edit( f.doc );
		const std::vector<ObjectId> solids = edit.SolidIds();
		const std::vector<ObjectId> entities = edit.EntityIds();
		switch ( next() % 6 )
		{
		case 0:
			edit.Add( BoxSolid( Vec3d( step * 8.0, 900, 0 ), Vec3d( step * 8.0 + 8, 908, 8 ) ) );
			break;
		case 1:
			if ( !solids.empty() )
				edit.Remove( solids[next() % solids.size()] );
			break;
		case 2:
			if ( !solids.empty() )
			{
				Solid *solid = edit.MutableSolid( solids[next() % solids.size()] );
				solid->hidden = !solid->hidden;
			}
			break;
		case 3:
			if ( !entities.empty() )
			{
				Entity *entity = edit.MutableEntity( entities[next() % entities.size()] );
				entity->editor.visgroupShown = !entity->editor.visgroupShown;
				entity->SetOrigin( Vec3d( step, step, step ) );
			}
			break;
		case 4:
			if ( !solids.empty() && !entities.empty() )
			{
				Solid *solid = edit.MutableSolid( solids[next() % solids.size()] );
				solid->owner = entities[next() % entities.size()];
			}
			break;
		default:
			selection.objects = {
			    entities.empty() ? ObjectId{} : entities[next() % entities.size()] };
			break;
		}
		const ChangeSet set = hammer::scene::CommitEdit( f.doc, edit );
		cache.Update( f.doc, set, selection, revision, revision + 1 );
		++revision;
		allMatch = allMatch && MatchesFull( cache, f.doc, selection, options );
	}
	checks.That( allMatch, "40 generated edits: every incremental snapshot == full rebuild" );
}

void Edges( testing::Checks &checks, const Fixture &f )
{
	const RenderSnapshot snapshot = Extract( f.doc, {} );
	const SolidDraw *box = FindSolid( snapshot, f.colored );
	if ( !box )
	{
		checks.That( false, "fixture box present" );
		return;
	}
	std::vector<WorldEdge> unique;
	std::size_t faceEdges = 0;
	for ( const FaceDraw &face : box->faces )
	{
		AppendUniqueEdges( unique, face.vertices );
		faceEdges += face.vertices.size();
	}
	checks.Equal( faceEdges, std::size_t( 24 ), "faces list 24 edges" );
	checks.Equal( unique.size(), std::size_t( 12 ), "a box has 12 unique edges" );

	Camera2D top;
	top.SetViewport( 200, 200 );
	top.SetZoom( 1.0 );
	top.SetCenter( { 32.0, 32.0 } );
	const std::vector<ScreenSegment> segments = ProjectEdges2D( *box, top );
	checks.Equal( segments.size(), std::size_t( 4 ),
	    "top view: vertical edges vanish and the top/bottom squares coincide" );
	bool onOutline = true;
	for ( const ScreenSegment &segment : segments )
	{
		for ( const ScreenPoint &p : { segment.a, segment.b } )
		{
			onOutline =
			    onOutline && ( p.x == 68.0 || p.x == 132.0 ) && ( p.y == 68.0 || p.y == 132.0 );
		}
	}
	checks.That( onOutline, "segments are the projected square" );
	checks.That(
	    ProjectEdges2D( SolidDraw{}, top ).empty(), "an empty draw has no edges (negative)" );

	// A wedge (five faces) keeps its sloped edges in the Front view.
	MapDocument doc;
	ObjectId wedge;
	{
		DocumentEdit edit( doc );
		Solid solid = BoxSolid( Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) );
		// Replace the +X side with a slope through (64,0,0)-(0,0,64) edge.
		for ( hammer::scene::Side &side : solid.sides )
		{
			if ( mapgeometry::NearlyEqual( side.Plane().normal, Vec3d( 1, 0, 0 ), 1.0e-9 ) )
			{
				side.points = { Vec3d( 64, 0, 0 ), Vec3d( 0, 0, 64 ), Vec3d( 0, 64, 64 ) };
			}
		}
		wedge = edit.Add( solid );
		hammer::scene::CommitEdit( doc, edit );
	}
	const RenderSnapshot wedgeSnapshot = Extract( doc, {} );
	const SolidDraw *wedgeDraw = FindSolid( wedgeSnapshot, wedge );
	std::vector<WorldEdge> wedgeEdges;
	if ( wedgeDraw )
	{
		for ( const FaceDraw &face : wedgeDraw->faces )
		{
			AppendUniqueEdges( wedgeEdges, face.vertices );
		}
	}
	checks.That( wedgeDraw && wedgeDraw->faces.size() == 5 && wedgeEdges.size() == 9,
	    "a wedge has five faces and nine unique edges" );
	Camera2D front;
	front.SetKind( ViewKind::Front );
	front.SetViewport( 200, 200 );
	front.SetZoom( 1.0 );
	checks.That( wedgeDraw && ProjectEdges2D( *wedgeDraw, front ).size() == 3,
	    "Front view of the wedge is a triangle" );
}

} // namespace

int main()
{
	testing::Checks checks;
	const LocalCatalog catalog = MakeCatalog();
	Fixture fixture;
	Build( fixture );
	checks.That( fixture.doc.Validate().empty(), "fixture document is valid" );
	Content( checks, fixture, catalog );
	Edges( checks, fixture );
	Cache( checks, fixture, catalog );
	// A displaced side carries its mesh; the other faces stay flat.
	{
		hammer::scene::MapDocument ddoc;
		hammer::scene::FaceTexture dtex;
		dtex.material = "NATURE/BLENDGRASSGRAVEL001A";
		hammer::scene::ObjectId ground;
		{
			hammer::scene::DocumentEdit edit( ddoc );
			hammer::scene::Solid s = hammer::scene::MakeBoxSolid(
			    { mapgeometry::Vec3d( 0, 0, -16 ), mapgeometry::Vec3d( 128, 128, 0 ) }, dtex );
			hammer::scene::Displacement d;
			d.power = 2;
			d.startPosition = mapgeometry::Vec3d( 0, 0, 0 );
			d.normals = std::vector<mapgeometry::Vec3d>( 25, mapgeometry::Vec3d( 0, 0, 1 ) );
			d.distances = std::vector<double>( 25, 0.0 );
			( *d.distances )[12] = 32.0;
			s.sides[0].displacement = d;
			ground = edit.Add( s );
			hammer::scene::CommitEdit( ddoc, edit );
		}
		const RenderSnapshot snap = Extract( ddoc, {}, {} );
		std::size_t displaced = 0;
		double peak = 0.0;
		for ( const FaceDraw &f : snap.solids.front().faces )
		{
			if ( f.displacement )
			{
				++displaced;
				for ( const mapgeometry::Vec3d &v : f.displacement->vertices )
					peak = std::max( peak, v.z );
			}
		}
		checks.Equal( displaced, std::size_t( 1 ), "one displaced face carries a mesh" );
		checks.Near( peak, 32.0, 1e-9, "the mesh is displaced" );
		hammer::scene::MapDocument broken = ddoc;
		hammer::scene::Solid bad = *broken.FindSolid( ground );
		bad.sides[0].displacement->distances->pop_back();
		broken.Put( bad );
		bool none = true;
		const RenderSnapshot brokenSnap = Extract( broken, {}, {} );
		for ( const FaceDraw &f : brokenSnap.solids.front().faces )
			none = none && !f.displacement;
		checks.That( none, "a malformed displacement draws no mesh (negative)" );
	}

	return checks.Report();
}
