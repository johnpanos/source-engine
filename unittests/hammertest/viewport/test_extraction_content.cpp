//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The render snapshot's models and instances (contract
//			viewport.extraction.v1, "Models and instances"): EntityDraw
//			carries a studio model's skin, scale and render color (malformed
//			keys give the defaults; sprites are not models); one InstanceDraw
//			per func_instance, built from the instance port, with content
//			colors, brush-entity solids, selection through the instance and
//			its groups, visibility, bounds kept out of the snapshot's; the
//			SnapshotCache re-extracts instances when the port's revision
//			changes and after an instance edit, and always equals a full
//			Extract. The placement oracle computes the expected world box by
//			hand and rejects four seeded wrong transforms (yaw sign, dropped
//			translation, rotation after translation, pitch for yaw).
//
//=============================================================================//

#include "hammer/app/instance_preview.h"
#include "hammer/formats/vmf_map_codec.h"
#include "hammer/scene/solid_geometry.h"
#include "hammer/viewport/extraction.h"
#include "testing/checks.h"

#include "app/fake_file_store.h"
#include "fakes/fake_entity_catalog.h"

#include <cmath>

using namespace hammer;
using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

scene::FaceTexture Texture( const char *material = "DEV/DEV_MEASUREGENERIC01B" )
{
	scene::FaceTexture tex;
	tex.material = material;
	return tex;
}

bool Near( const Vec3d &a, const Vec3d &b, double tolerance = 1e-4 )
{
	return std::fabs( a.x - b.x ) <= tolerance && std::fabs( a.y - b.y ) <= tolerance &&
	       std::fabs( a.z - b.z ) <= tolerance;
}

// The oracle: the world box of 'local' after a yaw of 'yawDegrees' about z
// and a translation by 'origin', from the rotated corners, computed here
// independently of the code under test.
scene::Box ExpectedBox( const scene::Box &local, const Vec3d &origin, double yawDegrees )
{
	const double r = yawDegrees * 3.14159265358979323846 / 180.0;
	const double c = std::cos( r );
	const double s = std::sin( r );
	std::optional<scene::Box> out;
	for ( int i = 0; i < 8; ++i )
	{
		const Vec3d p( i & 1 ? local.maxs.x : local.mins.x, i & 2 ? local.maxs.y : local.mins.y,
		    i & 4 ? local.maxs.z : local.mins.z );
		const Vec3d w( origin.x + c * p.x - s * p.y, origin.y + s * p.x + c * p.y, origin.z + p.z );
		if ( out )
		{
			out->Extend( w );
		}
		else
		{
			out = scene::PointBox( w );
		}
	}
	return *out;
}

bool DrawsBox( const viewport::InstanceDraw &draw, const scene::Box &box )
{
	for ( const viewport::SolidDraw &solid : draw.solids )
	{
		if ( Near( solid.bounds.mins, box.mins ) && Near( solid.bounds.maxs, box.maxs ) )
		{
			return true;
		}
	}
	return false;
}

const viewport::EntityDraw *EntityOf( const viewport::RenderSnapshot &snapshot, ObjectId id )
{
	for ( const viewport::EntityDraw &draw : snapshot.entities )
	{
		if ( draw.id == id )
		{
			return &draw;
		}
	}
	return nullptr;
}

scene::Entity PointEntity( const char *classname, Vec3d origin )
{
	scene::Entity e;
	e.classname = classname;
	e.SetOrigin( origin );
	return e;
}

} // namespace

int main()
{
	testing::Checks checks;
	hammertest::FakeEntityCatalog catalog;
	catalog.AddPoint( "info_player_start" ).Model( "models/editor/playerstart.mdl" );
	catalog.AddSolid( "func_door" ).Color( Vec3d( 10, 20, 30 ) );

	// --- Model keys -------------------------------------------------------------------
	{
		scene::MapDocument doc( 1 );
		ObjectId prop, bad, start, sprite;
		{
			scene::DocumentEdit edit( doc );
			scene::Entity p = PointEntity( "prop_static", Vec3d( 0, 0, 0 ) );
			p.SetKey( "model", "Models/Props/Crate.MDL" );
			p.SetKey( "skin", "2" );
			p.SetKey( "modelscale", "0.5" );
			p.SetKey( "rendercolor", "10 20 30" );
			prop = edit.Add( p );
			scene::Entity b = PointEntity( "prop_dynamic", Vec3d( 64, 0, 0 ) );
			b.SetKey( "model", "models/props/crate.mdl" );
			b.SetKey( "skin", "two" );
			b.SetKey( "modelscale", "-1" );
			b.SetKey( "rendercolor", "300 0 0" );
			bad = edit.Add( b );
			start = edit.Add( PointEntity( "info_player_start", Vec3d( 0, 64, 0 ) ) );
			scene::Entity s = PointEntity( "env_sprite", Vec3d( 0, 0, 64 ) );
			s.SetKey( "model", "sprites/glow01.vmt" );
			s.SetKey( "skin", "3" );
			sprite = edit.Add( s );
			scene::CommitEdit( doc, edit );
		}
		viewport::ExtractOptions options;
		options.catalog = &catalog;
		const viewport::RenderSnapshot snapshot = viewport::Extract( doc, {}, options );
		const viewport::EntityDraw *p = EntityOf( snapshot, prop );
		checks.That( p && viewport::IsStudioModelPath( p->model ) &&
		                 p->modelKeys == viewport::ModelKeys{ 2, 0.5, scene::Rgb{ 10, 20, 30 } },
		    "a prop carries its skin, scale and render color" );
		const viewport::EntityDraw *b = EntityOf( snapshot, bad );
		checks.That( b && b->modelKeys == viewport::ModelKeys{},
		    "malformed skin, scale and color give the defaults" );
		const viewport::EntityDraw *st = EntityOf( snapshot, start );
		checks.That( st && st->model == "models/editor/playerstart.mdl" &&
		                 viewport::IsStudioModelPath( st->model ),
		    "a catalog studio model is a model" );
		const viewport::EntityDraw *sp = EntityOf( snapshot, sprite );
		checks.That( sp && !viewport::IsStudioModelPath( sp->model ) &&
		                 sp->modelKeys == viewport::ModelKeys{},
		    "a sprite is not a model and reads no model keys" );
		checks.That( snapshot.instances.empty(), "no instance port: no InstanceDraws" );
		checks.That(
		    !viewport::IsStudioModelPath( ".mdl" ) && viewport::IsStudioModelPath( "a.MdL" ),
		    "IsStudioModelPath" );
	}

	// --- Instances through InstancePreview ------------------------------------------------
	formats::VmfMapCodec codec;
	hammertest::InMemoryFileStore store;
	const scene::Box localBox{ Vec3d( 0, -8, 0 ), Vec3d( 16, 8, 8 ) };
	const scene::Box localDoor{ Vec3d( 32, 0, 0 ), Vec3d( 40, 64, 96 ) };
	const auto writeContent = [&]( const scene::Box &box )
	{
		scene::MapDocument doc( 5 );
		scene::DocumentEdit edit( doc );
		edit.Add( scene::MakeBoxSolid( box, Texture() ) );
		scene::Entity door;
		door.classname = "func_door";
		const ObjectId doorId = edit.Add( door );
		scene::Solid leaf = scene::MakeBoxSolid( localDoor, Texture() );
		leaf.owner = doorId;
		edit.Add( leaf );
		scene::Entity prop = PointEntity( "prop_static", Vec3d( 0, 0, 0 ) );
		prop.SetKey( "model", "models/props/crate.mdl" );
		edit.Add( prop );
		scene::CommitEdit( doc, edit );
		store.files["/maps/instances/room.vmf"] = codec.Encode( doc ).Value();
	};
	writeContent( localBox );

	app::InstancePreview preview( codec, store, &catalog );
	preview.SetDocumentPath( "/maps/test.vmf" );

	scene::MapDocument doc( 1 );
	ObjectId instance, group, missing, other;
	{
		scene::DocumentEdit edit( doc );
		scene::Group g;
		group = edit.Add( g );
		scene::Entity e = PointEntity( "func_instance", Vec3d( 100, 200, 0 ) );
		e.SetKey( "file", "instances/room.vmf" );
		e.SetKey( "angles", "0 90 0" );
		e.group = group;
		instance = edit.Add( e );
		scene::Entity m = PointEntity( "func_instance", Vec3d( 0, 0, 0 ) );
		m.SetKey( "file", "instances/none.vmf" );
		missing = edit.Add( m );
		other =
		    edit.Add( scene::MakeBoxSolid( { Vec3d( -4, -4, -4 ), Vec3d( 4, 4, 4 ) }, Texture() ) );
		scene::CommitEdit( doc, edit );
	}
	viewport::ExtractOptions options;
	options.catalog = &catalog;
	options.instances = &preview;
	const viewport::RenderSnapshot snapshot = viewport::Extract( doc, {}, options );
	checks.That( snapshot.instances.size() == 2 && snapshot.instances[0].id == instance &&
	                 snapshot.instances[1].id == missing,
	    "one InstanceDraw per func_instance, in id order" );
	if ( snapshot.instances.size() == 2 )
	{
		const viewport::InstanceDraw &draw = snapshot.instances[0];
		checks.That(
		    draw.status == ports::InstanceStatus::Placed && draw.file == "/maps/instances/room.vmf",
		    "the resolved file" );
		checks.That( draw.solids.size() == 2 && draw.entities.size() == 1,
		    "content solids and point entities; the brush entity is its solid" );
		checks.That( DrawsBox( draw, ExpectedBox( localBox, Vec3d( 100, 200, 0 ), 90 ) ) &&
		                 DrawsBox( draw, ExpectedBox( localDoor, Vec3d( 100, 200, 0 ), 90 ) ),
		    "the oracle accepts the placed content" );
		bool doorColor = false;
		bool worldColor = false;
		for ( const viewport::SolidDraw &solid : draw.solids )
		{
			doorColor =
			    doorColor || ( solid.owner.IsValid() && solid.color == scene::Rgb{ 10, 20, 30 } );
			worldColor = worldColor ||
			             ( !solid.owner.IsValid() && solid.color == viewport::kDefaultWorldColor );
		}
		checks.That( doorColor && worldColor, "content colors: the owner's class, else world" );
		checks.That( !draw.entities.empty() &&
		                 viewport::IsStudioModelPath( draw.entities[0].model ) &&
		                 Near( draw.entities[0].origin, Vec3d( 100, 200, 0 ) ),
		    "a content prop is an EntityDraw with its model" );
		checks.That( !draw.selected && !draw.solids[0].selected, "unselected" );
		checks.That( draw.bounds && draw.bounds->Encloses(
		                                ExpectedBox( localDoor, Vec3d( 100, 200, 0 ), 90 ) ),
		    "content bounds" );
		checks.That( snapshot.bounds && !snapshot.bounds->Contains( Vec3d( 70, 240, 90 ) ),
		    "content is not part of the snapshot's bounds" );
		checks.That( snapshot.instances[1].status == ports::InstanceStatus::NotFound &&
		                 snapshot.instances[1].solids.empty() &&
		                 snapshot.instances[1].file == "instances/none.vmf",
		    "a missing file: status, the file named, nothing drawn" );
		checks.That(
		    EntityOf( snapshot, instance ) != nullptr, "the func_instance keeps its marker" );
	}

	// --- Seeded wrong transforms: the oracle must reject each --------------------------------
	{
		const scene::Box expected = ExpectedBox( localBox, Vec3d( 100, 200, 0 ), 90 );
		struct Seeded
		{
			const char *what;
			const char *origin;
			const char *angles;
		};
		const Seeded seeded[] = {
		    { "yaw sign flipped", "100 200 0", "0 -90 0" },
		    { "translation dropped", "0 0 0", "0 90 0" },
		    { "rotation after translation", "-200 100 0", "0 90 0" },
		    { "pitch for yaw", "100 200 0", "90 0 0" },
		};
		for ( const Seeded &s : seeded )
		{
			scene::MapDocument wrong( 2 );
			{
				scene::DocumentEdit edit( wrong );
				scene::Entity e;
				e.classname = "func_instance";
				e.SetKey( "file", "instances/room.vmf" );
				e.SetKey( "origin", s.origin );
				e.SetKey( "angles", s.angles );
				edit.Add( e );
				scene::CommitEdit( wrong, edit );
			}
			const viewport::RenderSnapshot seededSnapshot = viewport::Extract( wrong, {}, options );
			checks.That( seededSnapshot.instances.size() == 1 &&
			                 !DrawsBox( seededSnapshot.instances[0], expected ),
			    std::string( "the oracle rejects a seeded wrong transform: " ) + s.what );
		}
	}

	// --- Selection and visibility --------------------------------------------------------------
	{
		const viewport::RenderSnapshot selected =
		    viewport::Extract( doc, viewport::SelectionInput{ { group }, {} }, options );
		bool all = !selected.instances.empty() && selected.instances[0].selected;
		for ( const viewport::SolidDraw &solid : selected.instances[0].solids )
		{
			all = all && solid.selected;
		}
		for ( const viewport::EntityDraw &entity : selected.instances[0].entities )
		{
			all = all && entity.selected;
		}
		checks.That( all, "selecting the instance's group selects all its content" );
		checks.That( !selected.instances[1].selected, "other instances stay unselected" );

		viewport::ExtractOptions hide = options;
		hide.visible = [instance]( const scene::DocumentReader &d, ObjectId id )
		{
			return id != instance && d.KindOf( id ).has_value();
		};
		const viewport::RenderSnapshot hidden = viewport::Extract( doc, {}, hide );
		checks.That( hidden.instances.size() == 1 && hidden.instances[0].id == missing,
		    "a hidden instance draws nothing" );
		hide.keepHidden = true;
		const viewport::RenderSnapshot ghosted = viewport::Extract( doc, {}, hide );
		checks.That( ghosted.instances.size() == 2 && ghosted.instances[0].hidden &&
		                 ghosted.instances[0].solids[0].hidden,
		    "keepHidden: the content is ghosted" );
	}

	// --- SnapshotCache ----------------------------------------------------------------------------
	{
		viewport::SnapshotCache cache( options );
		cache.Rebuild( doc, {}, 1 );
		checks.That(
		    cache.Snapshot() == viewport::Extract( doc, {}, options ), "cache equals Extract" );

		// The file changes on disk; Refresh bumps the port's revision.
		writeContent( { Vec3d( 0, 0, 0 ), Vec3d( 4, 4, 4 ) } );
		checks.That( preview.Refresh(), "the port sees the changed file" );
		cache.SetSelection( doc, {} );
		checks.That( cache.LastRebuiltCount() == 2 &&
		                 DrawsBox( cache.Snapshot().instances[0],
		                     ExpectedBox( { Vec3d( 0, 0, 0 ), Vec3d( 4, 4, 4 ) },
		                         Vec3d( 100, 200, 0 ), 90 ) ),
		    "a new port revision re-extracts exactly the instances" );
		checks.That( cache.Snapshot() == viewport::Extract( doc, {}, options ),
		    "and the cache still equals Extract" );

		// An edit of the instance entity (move it) is incremental.
		scene::DocumentEdit edit( doc );
		edit.MutableEntity( instance )->SetOrigin( Vec3d( 0, 0, 512 ) );
		const scene::ChangeSet changes = scene::CommitEdit( doc, edit );
		cache.Update( doc, changes, {}, 1, 2 );
		checks.That(
		    DrawsBox( cache.Snapshot().instances[0],
		        ExpectedBox( { Vec3d( 0, 0, 0 ), Vec3d( 4, 4, 4 ) }, Vec3d( 0, 0, 512 ), 90 ) ),
		    "moving the instance moves its content" );
		checks.That( cache.Snapshot() == viewport::Extract( doc, {}, options ),
		    "the incremental snapshot equals Extract" );
		checks.That( other.IsValid(), "fixture" );
	}
	return checks.Report();
}
