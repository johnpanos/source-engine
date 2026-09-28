//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app prefab insertion (RFC 0002, R08 domain logic):
//			FragmentFromDocument takes every object of a decoded prefab;
//			LoadFragment reads and decodes through the codec and file-store
//			ports (read and decode failures, and an empty file, are errors);
//			InsertPrefab centers the prefab's bounds on the point (or puts its
//			origin there), turns it about the anchor with texture lock and
//			overlay bases, gives fresh runtime and VMF ids, groups more than
//			one top-level object when asked, and expands "&i" name keywords
//			with references inside the prefab. Every result passes
//			ValidateEdit. Negative checks: an empty prefab, non-finite input
//			and a prefab without extent stage nothing.
//
//=============================================================================//

#include "hammer/app/fragment_io.h"
#include "hammer/app/ops/decal_ops.h"
#include "hammer/app/ops/prefab_ops.h"
#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include "app/fake_file_store.h"
#include "fakes/fake_map_codec.h"

#include <cmath>
#include <limits>
#include <set>

using namespace hammer;
using namespace hammer::app;
using namespace hammer::app::ops;
using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

bool Near( const Vec3d &a, const Vec3d &b, double tolerance = 1e-6 )
{
	return std::fabs( a.x - b.x ) <= tolerance && std::fabs( a.y - b.y ) <= tolerance &&
	       std::fabs( a.z - b.z ) <= tolerance;
}

bool UniqueVmfIds( const scene::DocumentReader &doc )
{
	std::set<std::uint32_t> solids;
	std::set<std::uint32_t> sides;
	std::set<std::uint32_t> entities;
	for ( ObjectId id : doc.SolidIds() )
	{
		const scene::Solid *s = doc.FindSolid( id );
		if ( !solids.insert( s->vmfId ).second )
		{
			return false;
		}
		for ( const scene::Side &side : s->sides )
		{
			if ( !sides.insert( side.vmfId ).second )
			{
				return false;
			}
		}
	}
	for ( ObjectId id : doc.EntityIds() )
	{
		if ( !entities.insert( doc.FindEntity( id )->vmfId ).second )
		{
			return false;
		}
	}
	return true;
}

const scene::Entity *EntityNamed( const scene::DocumentReader &doc, std::string_view name )
{
	const std::vector<ObjectId> ids = scene::FindEntitiesByName( doc, name );
	return ids.size() == 1 ? doc.FindEntity( ids[0] ) : nullptr;
}

} // namespace

int main()
{
	testing::Checks checks;

	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";

	// The prefab: two world boxes, a lamp named with the keyword, a switch
	// referring to it, and an overlay on the first box's top face.
	scene::MapDocument prefabDoc( 7 );
	std::uint32_t topSide = 0;
	{
		scene::DocumentEdit edit( prefabDoc );
		const ObjectId a =
		    edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 64, 32, 16 ) }, tex ) );
		edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 16 ), Vec3d( 16, 16, 48 ) }, tex ) );
		scene::Entity lamp;
		lamp.classname = "light";
		lamp.SetKey( "targetname", "lamp&i" );
		lamp.SetOrigin( Vec3d( 32, 16, 40 ) );
		edit.Add( lamp );
		scene::Entity button;
		button.classname = "func_button";
		button.SetKey( "target", "LAMP&i" );
		button.SetOrigin( Vec3d( 8, 8, 20 ) );
		button.connections.push_back(
		    *scene::ParseConnection( "OnPressed", "lamp&i,Toggle,,0,-1" ) );
		edit.Add( button );
		for ( const scene::Side &side : edit.FindSolid( a )->sides )
		{
			if ( side.Plane().normal.z > 0.5 )
			{
				topSide = side.vmfId;
			}
		}
		ObjectId overlay;
		checks.That( PlaceOverlay( edit, { scene::FaceRef{ a, topSide } }, Vec3d( 32, 16, 16 ),
		                 "decals/x", 16, 8, overlay )
		                 .HasValue(),
		    "fixture overlay" );
		checks.That( scene::ValidateEdit( edit ).empty(), "prefab fixture valid" );
		scene::CommitEdit( prefabDoc, edit );
	}

	// FragmentFromDocument.
	const std::optional<MapFragment> prefab = FragmentFromDocument( prefabDoc );
	checks.That( prefab && prefab->Count() == 5, "every prefab object is in the fragment" );
	checks.That( prefab && prefab->bounds && Near( prefab->bounds->Center(), Vec3d( 32, 16, 24 ) ),
	    "fragment bounds" );
	checks.That( !FragmentFromDocument( scene::MapDocument( 3 ) ), "an empty document (negative)" );

	// LoadFragment through the ports.
	{
		hammertest::FakeMapCodec codec;
		hammertest::InMemoryFileStore store;
		store.files["prefabs/room.vmf"] = codec.Encode( prefabDoc ).Value();
		store.files["prefabs/empty.vmf"] = codec.Encode( scene::MapDocument( 4 ) ).Value();
		store.files["prefabs/bad.vmf"] = "not a map";
		auto loaded = LoadFragment( codec, store, "prefabs/room.vmf" );
		checks.That( loaded.HasValue() && loaded.Value().Count() == 5, "LoadFragment" );
		auto missing = LoadFragment( codec, store, "prefabs/none.vmf" );
		checks.That( !missing && missing.Error().status == DocumentIoStatus::ReadFailed,
		    "a missing file is a read failure (negative)" );
		auto bad = LoadFragment( codec, store, "prefabs/bad.vmf" );
		checks.That(
		    !bad && bad.Error().status == DocumentIoStatus::DecodeFailed && bad.Error().line == 1,
		    "a decode failure keeps the codec line (negative)" );
		auto empty = LoadFragment( codec, store, "prefabs/empty.vmf" );
		checks.That( !empty && empty.Error().status == DocumentIoStatus::DecodeFailed,
		    "a file without objects is refused (negative)" );
	}

	// The target map already uses lamp1 and lamp7 (and lampX, not a number).
	scene::MapDocument doc;
	{
		scene::DocumentEdit edit( doc );
		for ( const char *name : { "lamp1", "LAMP7", "lampX" } )
		{
			scene::Entity e;
			e.classname = "info_target";
			e.SetKey( "targetname", name );
			e.SetOrigin( Vec3d( 1000, 0, 0 ) );
			edit.Add( e );
		}
		scene::CommitEdit( doc, edit );
	}
	checks.Equal( ExpandNameKeyword( "lamp&i", doc ), std::string( "lamp8" ), "&i expansion" );
	checks.Equal( ExpandNameKeyword( "x&iy", doc ), std::string( "x1y" ), "&i with a suffix" );
	checks.Equal( ExpandNameKeyword( "plain", doc ), std::string( "plain" ), "no keyword" );

	// Bounds-center insertion, grouped.
	{
		scene::DocumentEdit edit( doc );
		std::vector<ObjectId> created;
		checks.That( InsertPrefab( edit, *prefab, Vec3d( 256, 256, 64 ), Vec3d(), true, &created )
		                 .HasValue(),
		    "insert" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after insert" );
		checks.That( UniqueVmfIds( edit ), "fresh VMF ids" );
		checks.That( created.size() == 1 && edit.FindGroup( created[0] ),
		    "several top-level objects are wrapped in one group" );
		const std::optional<scene::Box> bounds = scene::ObjectsBounds( edit, created );
		checks.That( bounds && Near( bounds->Center(), Vec3d( 256, 256, 64 ) ),
		    "the bounds center lands on the point" );
		checks.Equal(
		    edit.Finish().Created().size(), std::size_t( 6 ), "five objects and a group" );
		const scene::Entity *lamp = EntityNamed( edit, "lamp8" );
		checks.That( lamp != nullptr, "the keyword name is expanded" );
		const std::vector<ObjectId> buttons = scene::FindEntitiesByClass( edit, "func_button" );
		const scene::Entity *button = buttons.size() == 1 ? edit.FindEntity( buttons[0] ) : nullptr;
		checks.That( button && *button->Key( "target" ) == "lamp8" &&
		                 button->connections[0].target == "lamp8",
		    "references inside the prefab follow the expanded name" );
		const std::vector<ObjectId> overlays = scene::FindEntitiesByClass( edit, "info_overlay" );
		const scene::Entity *ov = overlays.size() == 1 ? edit.FindEntity( overlays[0] ) : nullptr;
		const std::optional<scene::FaceRef> face =
		    ov ? scene::FindSideById(
		             edit, static_cast<std::uint32_t>( std::stoul( *ov->Key( "sides" ) ) ) )
		       : std::nullopt;
		checks.That(
		    face && face->side != topSide && *ov->Key( "sides" ) != std::to_string( topSide ),
		    "overlay sides name the inserted face" );
		checks.That( ov && *ov->Key( "BasisOrigin" ) == "256 256 56", "overlay basis moved" );
		scene::MapDocument committed = doc;
		scene::CommitEdit( committed, edit );
		checks.That( committed.Validate().empty(), "commits cleanly" );
		scene::DocumentEdit second( committed );
		checks.That( InsertPrefab( second, *prefab, Vec3d(), Vec3d(), false ).HasValue() &&
		                 EntityNamed( second, "lamp9" ) != nullptr,
		    "the next insertion takes the next number" );
	}

	// Origin anchor, rotation, and a single object is never wrapped.
	{
		const MapFragment one = Copy( prefabDoc, { prefabDoc.SolidIds()[0] } );
		scene::DocumentEdit edit( doc );
		std::vector<ObjectId> created;
		checks.That( InsertPrefab( edit, one, Vec3d( 64, 0, 0 ), Vec3d(), true, &created,
		                 PrefabAnchor::Origin )
		                 .HasValue(),
		    "insert at the origin anchor" );
		checks.That(
		    created.size() == 1 && edit.FindSolid( created[0] ), "a single object is not grouped" );
		const std::optional<scene::Box> b =
		    created.empty() ? std::nullopt : scene::SolidBounds( *edit.FindSolid( created[0] ) );
		checks.That(
		    b && Near( b->mins, Vec3d( 64, 0, 0 ) ) && Near( b->maxs, Vec3d( 128, 32, 16 ) ),
		    "the prefab origin lands on the point" );
		created.clear();
		checks.That(
		    InsertPrefab( edit, one, Vec3d( 0, 0, 500 ), Vec3d( 0, 90, 0 ), false, &created )
		        .HasValue(),
		    "insert turned" );
		const std::optional<scene::Box> r =
		    created.empty() ? std::nullopt : scene::SolidBounds( *edit.FindSolid( created[0] ) );
		checks.That(
		    r && Near( r->Size(), Vec3d( 32, 64, 16 ) ) && Near( r->Center(), Vec3d( 0, 0, 500 ) ),
		    "a yaw of 90 turns the prefab about its center" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid" );
	}

	// TransformedFragment turns overlays too.
	{
		const std::optional<MapFragment> turned = TransformedFragment(
		    *prefab, mapgeometry::Affine::About( mapgeometry::AngleMatrix( 0, 90, 0 ), Vec3d() ) );
		const scene::Entity *ov = nullptr;
		for ( const scene::MapObject &o : turned->objects )
		{
			const scene::Entity *e = std::get_if<scene::Entity>( &o );
			ov = e && e->classname == "info_overlay" ? e : ov;
		}
		checks.That( ov && *ov->Key( "BasisU" ) == "0 1 0" && *ov->Key( "BasisV" ) == "-1 0 0" &&
		                 *ov->Key( "BasisNormal" ) == "0 0 1",
		    "the overlay basis turns with the prefab" );
	}

	// Refusals stage nothing.
	{
		scene::DocumentEdit edit( doc );
		checks.That( InsertPrefab( edit, MapFragment{}, Vec3d(), Vec3d(), true ).Error().code ==
		                 EditErrorCode::Nothing,
		    "an empty prefab (negative)" );
		const double nan = std::numeric_limits<double>::quiet_NaN();
		checks.That(
		    InsertPrefab( edit, *prefab, Vec3d( nan, 0, 0 ), Vec3d(), true ).Error().code ==
		        EditErrorCode::Rejected,
		    "a NaN point (negative)" );
		checks.That( InsertPrefab( edit, *prefab, Vec3d(),
		                 Vec3d( 0, std::numeric_limits<double>::infinity(), 0 ), true )
		                     .Error()
		                     .code == EditErrorCode::Rejected,
		    "infinite angles (negative)" );
		MapFragment shapeless;
		scene::Entity noOrigin;
		noOrigin.id = ObjectId( ( std::uint64_t( 7 ) << 32 ) | 99 );
		noOrigin.classname = "logic_auto";
		shapeless.objects.emplace_back( noOrigin );
		checks.That( InsertPrefab( edit, shapeless, Vec3d(), Vec3d(), false ).Error().code ==
		                 EditErrorCode::Rejected,
		    "no extent to center (negative)" );
		checks.That(
		    InsertPrefab( edit, shapeless, Vec3d(), Vec3d(), false, nullptr, PrefabAnchor::Origin )
		        .HasValue(),
		    "but it inserts at its origin" );
		checks.Equal(
		    edit.Finish().Created().size(), std::size_t( 1 ), "only the accepted insert staged" );
	}

	return checks.Report();
}
