//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.viewport picking (RFC 0002, R08 domain logic; contract
//			viewport.picking.v1): 2D picks by projected edge, centre handle and
//			entity marker (catalog sizes), the one hit-ordering policy
//			(distance, then projected area, then id), brush-entity hits naming
//			solid and owner, ray picks sorted by t with the entered FaceRef and
//			outward normal, and marquee Inside vs Touching with brush-entity
//			owner mapping. Negative checks: hidden (quick-hide and visgroup)
//			and originless objects are not pickable, invalid input yields no
//			hits, rays starting inside a solid or beyond maxDistance miss.
//
//=============================================================================//

#include "hammer/scene/change_set.h"
#include "hammer/scene/solid_geometry.h"
#include "hammer/viewport/picking.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>

using namespace hammer::viewport;
using hammer::scene::Box;
using hammer::scene::DocumentEdit;
using hammer::scene::Entity;
using hammer::scene::FaceTexture;
using hammer::scene::MapDocument;
using hammer::scene::ObjectId;
using hammer::scene::Solid;
using mapgeometry::Vec3d;
namespace ports = hammer::ports;

namespace
{

// A tiny in-memory entity catalog (display hints only).
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

struct Fixture
{
	MapDocument doc;
	LocalCatalog catalog;
	ObjectId big, small, door, doorSolid, doorSolid2, lamp, start, hiddenSolid, visHidden,
	    originless, twinA, twinB;
};

Solid BoxSolid( Vec3d mins, Vec3d maxs )
{
	FaceTexture texture;
	texture.material = "DEV/DEV_MEASUREGENERIC01";
	return hammer::scene::MakeBoxSolid( Box{ mins, maxs }, texture );
}

Entity PointEntity( const char *classname, Vec3d origin )
{
	Entity entity;
	entity.classname = classname;
	entity.SetOrigin( origin );
	return entity;
}

void Build( Fixture &f )
{
	ports::EntityClassInfo light;
	light.name = "light";
	light.boxMins = Vec3d( -8, -8, -8 );
	light.boxMaxs = Vec3d( 8, 8, 8 );
	f.catalog.Add( light );
	ports::EntityClassInfo player;
	player.name = "info_player_start";
	player.boxMins = Vec3d( -16, -16, 0 );
	player.boxMaxs = Vec3d( 16, 16, 72 );
	f.catalog.Add( player );

	DocumentEdit edit( f.doc );
	f.big = edit.Add( BoxSolid( Vec3d( 0, 0, 0 ), Vec3d( 128, 128, 128 ) ) );
	f.small = edit.Add( BoxSolid( Vec3d( 32, 32, 32 ), Vec3d( 64, 64, 64 ) ) );
	Entity door;
	door.classname = "func_door";
	door.SetOrigin( Vec3d( 288, 32, 64 ) ); // a brush entity with an origin still has no marker
	f.door = edit.Add( door );
	Solid ds = BoxSolid( Vec3d( 256, 0, 0 ), Vec3d( 320, 64, 128 ) );
	ds.owner = f.door;
	f.doorSolid = edit.Add( ds );
	Solid ds2 = BoxSolid( Vec3d( 256, 160, 0 ), Vec3d( 320, 200, 64 ) );
	ds2.owner = f.door;
	f.doorSolid2 = edit.Add( ds2 );
	f.lamp = edit.Add( PointEntity( "light", Vec3d( 500, 500, 16 ) ) );
	f.start = edit.Add( PointEntity( "info_player_start", Vec3d( 600, 0, 0 ) ) );
	Solid hidden = BoxSolid( Vec3d( 0, 300, 0 ), Vec3d( 64, 364, 64 ) );
	hidden.hidden = true;
	f.hiddenSolid = edit.Add( hidden );
	Entity vis = PointEntity( "info_target", Vec3d( 700, 0, 0 ) );
	vis.editor.visgroupShown = false;
	f.visHidden = edit.Add( vis );
	Entity bare;
	bare.classname = "logic_auto";
	f.originless = edit.Add( bare );
	f.twinA = edit.Add( BoxSolid( Vec3d( 800, 800, 0 ), Vec3d( 832, 832, 32 ) ) );
	f.twinB = edit.Add( BoxSolid( Vec3d( 800, 800, 0 ), Vec3d( 832, 832, 32 ) ) );
	hammer::scene::CommitEdit( f.doc, edit );
}

// Top view where screen (x, y) = (world x, 1000 - world y).
Camera2D TopCamera()
{
	Camera2D camera;
	camera.SetViewport( 1000, 1000 );
	camera.SetZoom( 1.0 );
	camera.SetCenter( { 500.0, 500.0 } );
	return camera;
}

bool Contains( const std::vector<Hit2D> &hits, ObjectId id )
{
	return std::any_of( hits.begin(), hits.end(),
	    [&]( const Hit2D &h )
	    {
		    return h.object == id;
	    } );
}

bool Contains( const std::vector<RayHit> &hits, ObjectId id )
{
	return std::any_of( hits.begin(), hits.end(),
	    [&]( const RayHit &h )
	    {
		    return h.object == id;
	    } );
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

// Oracle for the documented 2D order.
bool Ordered2D( const std::vector<Hit2D> &hits )
{
	return std::is_sorted( hits.begin(), hits.end(),
	    []( const Hit2D &a, const Hit2D &b )
	    {
		    if ( a.distance != b.distance )
			    return a.distance < b.distance;
		    if ( a.area != b.area )
			    return a.area < b.area;
		    return a.object < b.object;
	    } );
}

void Picks2D( testing::Checks &checks, const Fixture &f )
{
	const Camera2D camera = TopCamera();
	PickOptions options;
	options.catalog = &f.catalog;

	std::vector<Hit2D> hits = Pick2D( f.doc, camera, 128.0, 900.0, options );
	checks.That( hits.size() == 1 && hits[0].object == f.big &&
	                 hits[0].kind == HitKind::SolidEdge && hits[0].distance == 0.0,
	    "click on an edge picks the solid" );
	hits = Pick2D( f.doc, camera, 129.5, 900.0, options );
	checks.That( hits.size() == 1 && std::fabs( hits[0].distance - 1.5 ) < 1.0e-9,
	    "edge within tolerance, distance in pixels" );
	checks.That(
	    Pick2D( f.doc, camera, 140.0, 900.0, options ).empty(), "beyond tolerance misses" );
	checks.That( Pick2D( f.doc, camera, 100.0, 900.0, options ).empty(),
	    "clicking inside a solid away from edges and handle misses (legacy 2D)" );

	// A's centre handle sits on B's edge: same distance, B's smaller area wins.
	hits = Pick2D( f.doc, camera, 64.0, 936.0, options );
	checks.That( hits.size() == 2 && hits[0].object == f.small && hits[1].object == f.big,
	    "tie on distance: smaller projected area first" );
	checks.That( hits.size() == 2 && hits[1].kind == HitKind::SolidCenter,
	    "the large solid is hit by its centre handle" );
	hits = Pick2D( f.doc, camera, 48.0, 952.0, options );
	checks.That(
	    hits.size() == 1 && hits[0].object == f.small && hits[0].kind == HitKind::SolidCenter,
	    "centre handle of the small solid" );

	// Identical twins: same distance and area, id order.
	hits = Pick2D( f.doc, camera, 800.0, 184.0, options );
	checks.That( hits.size() == 2 && hits[0].object == f.twinA && hits[1].object == f.twinB,
	    "full tie resolves by id" );

	// Brush entity: the solid and its owner.
	hits = Pick2D( f.doc, camera, 256.0, 968.0, options );
	checks.That( hits.size() == 1 && hits[0].object == f.doorSolid && hits[0].owner == f.door,
	    "brush-entity solid reports solid and owner" );
	checks.That(
	    !hits.empty() && SelectionTarget( hits[0] ) == f.door, "a click selects the owner" );
	hits = Pick2D( f.doc, camera, 288.0, 968.0, options );
	checks.That( !Contains( hits, f.door ) && Contains( hits, f.doorSolid ),
	    "a brush entity with an origin has no marker (its solid's handle is hit)" );

	// Point entities, catalog sizes.
	hits = Pick2D( f.doc, camera, 500.0, 500.0, options );
	checks.That( hits.size() == 1 && hits[0].object == f.lamp &&
	                 hits[0].kind == HitKind::EntityMarker && hits[0].distance == 0.0 &&
	                 !hits[0].owner.IsValid(),
	    "click inside a marker" );
	hits = Pick2D( f.doc, camera, 511.0, 500.0, options );
	checks.That( hits.size() == 1 && hits[0].distance == 3.0, "marker within tolerance" );
	checks.That(
	    Pick2D( f.doc, camera, 512.0, 500.0, options ).empty(), "marker beyond tolerance" );
	checks.That( Contains( Pick2D( f.doc, camera, 614.0, 1000.0, options ), f.start ),
	    "catalog box (32 wide) picked near its edge" );
	checks.That( !Contains( Pick2D( f.doc, camera, 614.0, 1000.0, PickOptions{} ), f.start ),
	    "without a catalog the +/-8 box is used" );

	Camera2D front;
	front.SetKind( ViewKind::Front );
	front.SetViewport( 1000, 1000 );
	front.SetZoom( 1.0 );
	front.SetCenter( { 500.0, 500.0 } );
	const ScreenPoint head = front.WorldToScreen( Vec3d( 600, 0, 70 ) );
	checks.That( Contains( Pick2D( f.doc, front, head.x, head.y, options ), f.start ),
	    "Front view: the catalog height (72) is pickable" );
	checks.That( !Contains( Pick2D( f.doc, front, head.x, head.y, PickOptions{} ), f.start ),
	    "Front view: the default box is only 8 high" );

	// Visibility.
	checks.That( Pick2D( f.doc, camera, 0.0, 680.0, options ).empty(),
	    "a quick-hidden solid is not pickable (negative)" );
	checks.That( !Contains( Pick2D( f.doc, camera, 700.0, 1000.0, options ), f.visHidden ),
	    "a visgroup-hidden entity is not pickable (negative)" );
	PickOptions everything = options;
	everything.visible = ShowEverything();
	checks.That( Contains( Pick2D( f.doc, camera, 0.0, 680.0, everything ), f.hiddenSolid ) &&
	                 Contains( Pick2D( f.doc, camera, 700.0, 1000.0, everything ), f.visHidden ),
	    "a caller predicate can show hidden objects" );
	checks.That( !Contains( Pick2D( f.doc, camera, 500.0, 1000.0, everything ), f.originless ),
	    "an originless entity has no marker (negative)" );

	// Order holds for a crowded click.
	PickOptions wide = options;
	wide.edgeTolerancePixels = 200.0;
	wide.centerHandlePixels = 200.0;
	hits = Pick2D( f.doc, camera, 90.0, 910.0, wide );
	checks.That(
	    hits.size() >= 3 && Ordered2D( hits ), "crowded pick follows the ordering policy" );
	std::vector<Hit2D> reversed( hits.rbegin(), hits.rend() );
	checks.That( !Ordered2D( reversed ), "ordering oracle detects a reversed list (seeded)" );

	// Invalid input.
	const double nan = std::numeric_limits<double>::quiet_NaN();
	checks.That( Pick2D( f.doc, camera, nan, 900.0, options ).empty(),
	    "NaN pixel yields nothing (negative)" );
	checks.That( Pick2D( f.doc, Camera2D{}, 128.0, 900.0, options ).empty(),
	    "a camera without a viewport yields nothing (negative)" );
	PickOptions negative = options;
	negative.edgeTolerancePixels = -5.0;
	checks.That( Pick2D( f.doc, camera, 129.5, 900.0, negative ).empty(),
	    "negative tolerance counts as zero (negative)" );
	PickOptions noSolids = options;
	noSolids.solids = false;
	checks.That(
	    Pick2D( f.doc, camera, 128.0, 900.0, noSolids ).empty(), "solids can be excluded" );
}

void PicksRay( testing::Checks &checks, const Fixture &f )
{
	PickOptions options;
	options.catalog = &f.catalog;

	std::vector<RayHit> hits = PickRay( f.doc, Vec3d( 48, 48, 500 ), Vec3d( 0, 0, -1 ), options );
	checks.That( hits.size() == 2 && hits[0].object == f.big && hits[1].object == f.small,
	    "ray hits sorted by t (outer solid first)" );
	if ( hits.size() == 2 )
	{
		checks.Near( hits[0].t, 372.0, 1.0e-9, "t to the top face" );
		checks.That( hits[0].point == Vec3d( 48, 48, 128 ), "hit point on the top face" );
		checks.That( hits[0].normal == Vec3d( 0, 0, 1 ), "entered face normal points up" );
		checks.That( hits[0].face.solid == f.big &&
		                 hits[0].face.side == SideFacing( f.doc, f.big, Vec3d( 0, 0, 1 ) ) &&
		                 hits[0].face.side != 0,
		    "FaceRef names the top side's VMF id" );
		checks.That( hits[0].kind == HitKind::SolidFace, "solid hit kind" );
		checks.Near( hits[1].t, 436.0, 1.0e-9, "t to the inner solid" );
	}

	hits = PickRay( f.doc, Vec3d( -100, 16, 16 ), Vec3d( 5, 0, 0 ), options );
	checks.That( !hits.empty() && hits[0].object == f.big &&
	                 std::fabs( hits[0].t - 100.0 ) < 1.0e-9 &&
	                 hits[0].normal == Vec3d( -1, 0, 0 ) &&
	                 hits[0].face.side == SideFacing( f.doc, f.big, Vec3d( -1, 0, 0 ) ),
	    "non-unit direction: t in world units, -X face" );

	checks.That(
	    !Contains( PickRay( f.doc, Vec3d( 100, 100, 100 ), Vec3d( 0, 0, 1 ), options ), f.big ),
	    "a ray starting inside a solid does not hit it (negative)" );
	checks.That(
	    !Contains( PickRay( f.doc, Vec3d( -10, 200, 16 ), Vec3d( 1, 0, 0 ), options ), f.big ),
	    "a ray passing beside a solid misses it" );
	checks.That(
	    !Contains( PickRay( f.doc, Vec3d( 288, 32, 500 ), Vec3d( 0, 0, -1 ), options ), f.door ),
	    "a brush entity's origin is not a ray target" );

	hits = PickRay( f.doc, Vec3d( 500, 500, 100 ), Vec3d( 0, 0, -1 ), options );
	checks.That( hits.size() == 1 && hits[0].object == f.lamp &&
	                 hits[0].kind == HitKind::EntityMarker &&
	                 std::fabs( hits[0].t - 76.0 ) < 1.0e-9 && hits[0].normal == Vec3d( 0, 0, 1 ) &&
	                 !hits[0].face.solid.IsValid(),
	    "ray enters the marker's top" );

	hits = PickRay( f.doc, Vec3d( 288, 32, 500 ), Vec3d( 0, 0, -1 ), options );
	checks.That( hits.size() == 1 && hits[0].object == f.doorSolid && hits[0].owner == f.door &&
	                 SelectionTarget( hits[0] ) == f.door,
	    "ray on a brush entity names solid and owner" );

	PickOptions near = options;
	near.maxDistance = 300.0;
	checks.That( PickRay( f.doc, Vec3d( 48, 48, 500 ), Vec3d( 0, 0, -1 ), near ).empty(),
	    "hits beyond maxDistance are dropped (negative)" );
	checks.That( PickRay( f.doc, Vec3d( 32, 332, 500 ), Vec3d( 0, 0, -1 ), options ).empty(),
	    "a quick-hidden solid is not hit (negative)" );
	PickOptions everything = options;
	everything.visible = ShowEverything();
	checks.That( Contains( PickRay( f.doc, Vec3d( 32, 332, 500 ), Vec3d( 0, 0, -1 ), everything ),
	                 f.hiddenSolid ),
	    "a predicate override hits hidden solids" );
	checks.That( PickRay( f.doc, Vec3d( 48, 48, 500 ), Vec3d( 0, 0, 0 ), options ).empty(),
	    "zero direction yields nothing (negative)" );
	const double nan = std::numeric_limits<double>::quiet_NaN();
	checks.That( PickRay( f.doc, Vec3d( nan, 48, 500 ), Vec3d( 0, 0, -1 ), options ).empty(),
	    "NaN origin yields nothing (negative)" );

	// Oblique ray through a solid's edge region: every normal is a unit face normal.
	hits = PickRay( f.doc, Vec3d( -50, -50, 300 ), Vec3d( 1, 1, -1.5 ), everything );
	bool normalsOk = !hits.empty();
	for ( const RayHit &hit : hits )
	{
		normalsOk = normalsOk && std::fabs( mapgeometry::Length( hit.normal ) - 1.0 ) < 1.0e-9 &&
		            mapgeometry::Dot( hit.normal, Vec3d( 1, 1, -1.5 ) ) < 0.0;
	}
	checks.That( normalsOk, "entered normals are unit and face the ray" );
	checks.That( std::is_sorted( hits.begin(), hits.end(),
	                 []( const RayHit &a, const RayHit &b )
	                 {
		                 return a.t < b.t;
	                 } ),
	    "oblique hits sorted by t" );
}

void Marquee( testing::Checks &checks, const Fixture &f )
{
	const Camera2D camera = TopCamera();
	MarqueeOptions options;
	options.catalog = &f.catalog;

	// Screen rect for world u in [-10, 140], v in [-10, 140].
	const ScreenRect aroundBig{ { -10.0, 1010.0 }, { 140.0, 860.0 } };
	checks.That( MarqueeSelect( f.doc, camera, aroundBig, MarqueeMode::Inside, options ) ==
	                 std::vector<ObjectId>{ f.big, f.small },
	    "Inside selects wholly enclosed solids" );
	const ScreenRect reversed{ aroundBig.b, aroundBig.a };
	checks.That( MarqueeSelect( f.doc, camera, reversed, MarqueeMode::Inside, options ) ==
	                 MarqueeSelect( f.doc, camera, aroundBig, MarqueeMode::Inside, options ),
	    "corner order does not matter" );

	const ScreenRect sliver{ { 100.0, 900.0 }, { 200.0, 890.0 } };
	checks.That( MarqueeSelect( f.doc, camera, sliver, MarqueeMode::Touching, options ) ==
	                 std::vector<ObjectId>{ f.big },
	    "Touching selects overlapped bounds" );
	checks.That( MarqueeSelect( f.doc, camera, sliver, MarqueeMode::Inside, options ).empty(),
	    "Inside rejects partial overlap" );

	// Door: two solids; a rect around the first only.
	const ScreenRect aroundDoor1{ { 250.0, 1005.0 }, { 330.0, 930.0 } };
	checks.That( MarqueeSelect( f.doc, camera, aroundDoor1, MarqueeMode::Touching, options ) ==
	                 std::vector<ObjectId>{ f.door },
	    "Touching one door solid selects the owner" );
	checks.That( MarqueeSelect( f.doc, camera, aroundDoor1, MarqueeMode::Inside, options ).empty(),
	    "Inside needs every solid of the brush entity" );
	const ScreenRect aroundDoor{ { 250.0, 1005.0 }, { 330.0, 790.0 } };
	checks.That( MarqueeSelect( f.doc, camera, aroundDoor, MarqueeMode::Inside, options ) ==
	                 std::vector<ObjectId>{ f.door },
	    "Inside with every door solid selects the owner" );
	MarqueeOptions raw = options;
	raw.ownersForBrushEntities = false;
	checks.That( MarqueeSelect( f.doc, camera, aroundDoor1, MarqueeMode::Inside, raw ) ==
	                 std::vector<ObjectId>{ f.doorSolid },
	    "without owner mapping the solid itself is selected" );

	// Entities and visibility.
	const ScreenRect everything{ { -1000.0, -1000.0 }, { 2000.0, 2000.0 } };
	const std::vector<ObjectId> all =
	    MarqueeSelect( f.doc, camera, everything, MarqueeMode::Inside, options );
	checks.That(
	    all == std::vector<ObjectId>{ f.big, f.small, f.door, f.lamp, f.start, f.twinA, f.twinB },
	    "everything visible, in id order, without hidden or originless objects" );
	checks.That( std::find( all.begin(), all.end(), f.hiddenSolid ) == all.end() &&
	                 std::find( all.begin(), all.end(), f.visHidden ) == all.end(),
	    "hidden objects are not selected (negative)" );
	MarqueeOptions showAll = options;
	showAll.visible = ShowEverything();
	const std::vector<ObjectId> withHidden =
	    MarqueeSelect( f.doc, camera, everything, MarqueeMode::Inside, showAll );
	checks.That(
	    std::find( withHidden.begin(), withHidden.end(), f.hiddenSolid ) != withHidden.end() &&
	        std::find( withHidden.begin(), withHidden.end(), f.visHidden ) != withHidden.end(),
	    "a predicate override selects hidden objects" );
	const double nan = std::numeric_limits<double>::quiet_NaN();
	checks.That( MarqueeSelect( f.doc, camera, { { nan, 0.0 }, { 10.0, 10.0 } },
	                 MarqueeMode::Touching, options )
	                 .empty(),
	    "non-finite rectangle selects nothing (negative)" );
	checks.That( MarqueeSelect( f.doc, camera, { { 900.0, 10.0 }, { 950.0, 20.0 } },
	                 MarqueeMode::Touching, options )
	                 .empty(),
	    "an empty region selects nothing" );
}

} // namespace

int main()
{
	testing::Checks checks;
	Fixture fixture;
	Build( fixture );
	checks.That( fixture.doc.Validate().empty(), "fixture document is valid" );
	Picks2D( checks, fixture );
	PicksRay( checks, fixture );
	Marquee( checks, fixture );
	return checks.Report();
}
