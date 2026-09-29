//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app InstancePreview, the viewports' func_instance content
//			(contract app.instance_preview.v1): the three lookup rules and
//			their order; placement by the collapse rule (parameters, yaw
//			rotation, translation) checked against hand-computed boxes and
//			against CollapseInstance on the same content; quick-hidden
//			objects left out; nested content merged with unique ids; cycles,
//			missing, undecodable and rejected files as statuses; one decode per
//			file and one placement per distinct instance; Refresh and path
//			changes invalidate. Files are real VMF text (VmfMapCodec) in an
//			in-memory store.
//
//=============================================================================//

#include "hammer/app/fragment_io.h"
#include "hammer/app/instance_preview.h"
#include "hammer/app/ops/instance_ops.h"
#include "hammer/formats/vmf_map_codec.h"
#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include "app/fake_file_store.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <set>

using namespace hammer;
using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

bool Near( const Vec3d &a, const Vec3d &b, double tolerance = 1e-6 )
{
	return std::fabs( a.x - b.x ) <= tolerance && std::fabs( a.y - b.y ) <= tolerance &&
	       std::fabs( a.z - b.z ) <= tolerance;
}

scene::FaceTexture Texture()
{
	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	return tex;
}

scene::Entity Instance(
    const std::string &file, const std::string &origin, const std::string &angles = "0 0 0" )
{
	scene::Entity e;
	e.classname = "func_instance";
	if ( !file.empty() )
	{
		e.SetKey( "file", file );
	}
	e.SetKey( "origin", origin );
	e.SetKey( "angles", angles );
	return e;
}

// The VMF text of a document built by 'fill'.
template <typename Fill> std::string Vmf( const formats::VmfMapCodec &codec, Fill fill )
{
	scene::MapDocument doc( 3 );
	{
		scene::DocumentEdit edit( doc );
		fill( edit );
		scene::CommitEdit( doc, edit );
	}
	auto text = codec.Encode( doc );
	return text.HasValue() ? text.Value() : std::string( "encode failed" );
}

std::vector<scene::Box> SolidBoxes( const std::vector<scene::MapObject> &objects )
{
	std::vector<scene::Box> out;
	for ( const scene::MapObject &o : objects )
	{
		if ( const scene::Solid *s = std::get_if<scene::Solid>( &o ) )
		{
			if ( std::optional<scene::Box> box = scene::SolidBounds( *s ) )
			{
				out.push_back( *box );
			}
		}
	}
	return out;
}

std::vector<const scene::Entity *> Entities( const std::vector<scene::MapObject> &objects )
{
	std::vector<const scene::Entity *> out;
	for ( const scene::MapObject &o : objects )
	{
		if ( const scene::Entity *e = std::get_if<scene::Entity>( &o ) )
		{
			out.push_back( e );
		}
	}
	return out;
}

bool HasBox( const std::vector<scene::Box> &boxes, const Vec3d &mins, const Vec3d &maxs )
{
	return std::any_of( boxes.begin(), boxes.end(),
	    [&]( const scene::Box &b )
	    {
		    return Near( b.mins, mins, 1e-4 ) && Near( b.maxs, maxs, 1e-4 );
	    } );
}

} // namespace

int main()
{
	testing::Checks checks;
	formats::VmfMapCodec codec;
	hammertest::InMemoryFileStore store;

	// box.vmf: a world box, a prop whose model is a parameter, a hidden box.
	store.files["/game/sdk/maps/instances/box.vmf"] = Vmf( codec,
	    []( scene::DocumentEdit &edit )
	    {
		    edit.Add( scene::MakeBoxSolid( { Vec3d( 0, -8, 0 ), Vec3d( 16, 8, 8 ) }, Texture() ) );
		    scene::Solid hidden = scene::MakeBoxSolid(
		        { Vec3d( 500, 500, 500 ), Vec3d( 510, 510, 510 ) }, Texture() );
		    hidden.hidden = true;
		    edit.Add( hidden );
		    scene::Entity prop;
		    prop.classname = "prop_static";
		    prop.SetKey( "model", "$prop" );
		    prop.SetOrigin( Vec3d( 32, 0, 0 ) );
		    prop.SetAngles( Vec3d( 0, 0, 0 ) );
		    edit.Add( prop );
	    } );
	// A different box.vmf nearer a deeper map (lookup rule 1 must win).
	store.files["/game/sdk/maps/sub/deeper/instances/box.vmf"] = Vmf( codec,
	    []( scene::DocumentEdit &edit )
	    {
		    edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 2, 2, 2 ) }, Texture() ) );
	    } );
	// outer.vmf: its own box and box.vmf 64 units up.
	store.files["/game/sdk/maps/instances/outer.vmf"] = Vmf( codec,
	    []( scene::DocumentEdit &edit )
	    {
		    edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 8, 8, 8 ) }, Texture() ) );
		    edit.Add( Instance( "instances/box.vmf", "0 0 64" ) );
	    } );
	// loop.vmf includes itself.
	store.files["/game/sdk/maps/instances/loop.vmf"] = Vmf( codec,
	    []( scene::DocumentEdit &edit )
	    {
		    edit.Add( Instance( "instances/loop.vmf", "0 0 0" ) );
	    } );
	store.files["/game/sdk/maps/instances/bad.vmf"] = "world\n{\n\t\"nonsense\n";

	app::InstancePreview preview( codec, store );
	preview.SetDocumentPath( "/game/sdk/maps/test.vmf" );

	// --- Placement ------------------------------------------------------------------
	scene::Entity a = Instance( "instances\\box", "100 200 0", "0 90 0" );
	a.SetKey( "replace01", "$prop models/props/crate.mdl" );
	auto content = preview.Content( a );
	checks.That( content && content->status == ports::InstanceStatus::Placed, "box.vmf placed" );
	if ( content && content->status != ports::InstanceStatus::Placed )
	{
		std::printf( "  box.vmf: %s\n", content->detail.c_str() );
	}
	if ( content && content->status == ports::InstanceStatus::Placed )
	{
		checks.Equal( content->file, std::string( "/game/sdk/maps/instances/box.vmf" ),
		    "rule 1: the map's directory, backslashes and .vmf added" );
		const std::vector<scene::Box> boxes = SolidBoxes( content->objects );
		checks.That( boxes.size() == 1, "the hidden solid is left out" );
		// Yaw 90 turns +x into +y: x in [0,16] -> y in [0,16]; y in [-8,8] -> x.
		checks.That( HasBox( boxes, Vec3d( 92, 200, 0 ), Vec3d( 108, 216, 8 ) ),
		    "the box is rotated by the yaw, then translated" );
		const auto entities = Entities( content->objects );
		checks.That(
		    entities.size() == 1 && entities[0]->classname == "prop_static", "the prop is merged" );
		if ( entities.size() == 1 )
		{
			checks.That( Near( entities[0]->Origin().value_or( Vec3d() ), Vec3d( 100, 232, 0 ) ),
			    "the prop origin is placed" );
			checks.That( Near( entities[0]->Angles().value_or( Vec3d() ), Vec3d( 0, 90, 0 ) ),
			    "the prop orientation composes with the instance" );
			checks.That( *entities[0]->Key( "model" ) == "models/props/crate.mdl",
			    "replaceNN parameters are substituted" );
		}

		// The same content CollapseInstance merges (the one rule).
		auto fragment = app::LoadFragment( codec, store, content->file );
		auto placed = app::ops::PlaceInstanceContent( a, fragment.Value(), "InstanceAuto1" );
		checks.That( placed.HasValue(), "PlaceInstanceContent places the fragment" );
		scene::MapDocument target( 9 );
		ObjectId instanceId;
		{
			scene::DocumentEdit edit( target );
			instanceId = edit.Add( a );
			scene::CommitEdit( target, edit );
		}
		{
			scene::DocumentEdit edit( target );
			checks.That(
			    app::ops::CollapseInstance( edit, instanceId, fragment.Value() ).HasValue(),
			    "collapse the same instance" );
			scene::CommitEdit( target, edit );
		}
		std::vector<scene::Box> collapsed;
		for ( ObjectId id : target.SolidIds() )
		{
			if ( !target.FindSolid( id )->hidden )
			{
				collapsed.push_back( *scene::SolidBounds( *target.FindSolid( id ) ) );
			}
		}
		checks.That( collapsed.size() == 1 && HasBox( boxes, collapsed[0].mins, collapsed[0].maxs ),
		    "the preview draws what CollapseInstance merges" );
	}

	// --- Lookup rules -----------------------------------------------------------------
	{
		app::InstancePreview deep( codec, store );
		deep.SetDocumentPath( "/game/sdk/maps/sub/deeper/test.vmf" );
		auto near = deep.Content( Instance( "instances/box.vmf", "0 0 0" ) );
		checks.That( near && near->file == "/game/sdk/maps/sub/deeper/instances/box.vmf",
		    "rule 1 before rule 2" );
		deep.SetDocumentPath( "/game/sdk/maps/sub/test.vmf" );
		auto maps = deep.Content( Instance( "instances/box.vmf", "0 0 0" ) );
		checks.That( maps && maps->file == "/game/sdk/maps/instances/box.vmf",
		    "rule 2 looks under the enclosing maps/ directory" );
		store.files["/Game/SDK/Maps/instances/c.vmf"] =
		    store.files["/game/sdk/maps/instances/box.vmf"];
		deep.SetDocumentPath( "/Game/SDK/Maps/sub/test.vmf" );
		auto upper = deep.Content( Instance( "instances/c.vmf", "0 0 0" ) );
		checks.That( upper && upper->file == "/Game/SDK/Maps/instances/c.vmf",
		    "rule 2 finds maps/ in any case" );
		deep.SetDocumentPath( "" );
		auto none = deep.Content( Instance( "instances/box.vmf", "0 0 0" ) );
		checks.That( none && none->status == ports::InstanceStatus::NotFound &&
		                 none->detail == "instances/box.vmf",
		    "no path and no roots: not found, the file named" );
		deep.SetSearchRoots( { "/nowhere", "/game/sdk/maps/" } );
		auto root = deep.Content( Instance( "instances/box.vmf", "0 0 0" ) );
		checks.That( root && root->file == "/game/sdk/maps/instances/box.vmf",
		    "rule 3: the search roots in order" );
	}

	// --- Statuses -----------------------------------------------------------------------
	scene::Entity notInstance = Instance( "instances/box.vmf", "0 0 0" );
	notInstance.classname = "info_target";
	checks.That( preview.Content( notInstance ) == nullptr, "a non-instance has no content" );
	checks.That(
	    preview.Content( Instance( "", "0 0 0" ) )->status == ports::InstanceStatus::NoFile,
	    "no file key" );
	checks.That( preview.Content( Instance( "instances/nothere.vmf", "0 0 0" ) )->status ==
	                 ports::InstanceStatus::NotFound,
	    "missing file" );
	checks.That( preview.Content( Instance( "instances/bad.vmf", "0 0 0" ) )->status ==
	                 ports::InstanceStatus::DecodeFailed,
	    "undecodable file" );
	checks.That( preview.Content( Instance( "instances/box.vmf", "0 0 0", "0 x 0" ) )->status ==
	                 ports::InstanceStatus::Rejected,
	    "malformed angles are the collapse rule's refusal" );
	auto loop = preview.Content( Instance( "instances/loop.vmf", "0 0 0" ) );
	checks.That( loop && loop->status == ports::InstanceStatus::Placed && loop->objects.empty() &&
	                 loop->nestedFailures == 1,
	    "a file that includes itself: the cycle is a nested failure, not a hang" );

	// --- Nesting --------------------------------------------------------------------------
	auto outer = preview.Content( Instance( "instances/outer.vmf", "1000 0 0" ) );
	checks.That(
	    outer && outer->status == ports::InstanceStatus::Placed && outer->nestedFailures == 0,
	    "outer.vmf placed with its nested instance" );
	if ( outer )
	{
		const std::vector<scene::Box> boxes = SolidBoxes( outer->objects );
		checks.That( boxes.size() == 2 &&
		                 HasBox( boxes, Vec3d( 1000, 0, 0 ), Vec3d( 1008, 8, 8 ) ) &&
		                 HasBox( boxes, Vec3d( 1000, -8, 64 ), Vec3d( 1016, 8, 72 ) ),
		    "nested content is placed by both transforms (nested file found by rule 2)" );
		std::set<std::uint64_t> ids;
		bool nestedLeft = false;
		for ( const scene::MapObject &o : outer->objects )
		{
			std::visit(
			    [&]( const auto &value )
			    {
				    ids.insert( value.id.value );
			    },
			    o );
			if ( const scene::Entity *e = std::get_if<scene::Entity>( &o ) )
			{
				nestedLeft = nestedLeft || e->classname == "func_instance";
			}
		}
		checks.That( ids.size() == outer->objects.size() && !ids.count( 0 ),
		    "merged content has unique, valid ids" );
		checks.That( !nestedLeft, "nested func_instances are replaced by their content" );
	}

	// --- Caching and invalidation ------------------------------------------------------------
	const std::size_t decoded = preview.FilesDecoded();
	const std::size_t placements = preview.Placements();
	checks.That(
	    preview.Content( a ) == content, "a repeated instance returns the cached content" );
	auto moved = preview.Content( Instance( "instances\\box", "0 0 0" ) );
	checks.That( preview.FilesDecoded() == decoded && preview.Placements() == placements + 1,
	    "another placement of a decoded file decodes nothing" );
	const std::uint64_t revision = preview.Revision();
	checks.That( !preview.Refresh() && preview.Revision() == revision,
	    "Refresh without changes keeps everything" );
	preview.SetDocumentPath( "/game/sdk/maps/test.vmf" );
	checks.That( preview.Revision() == revision, "the same document path changes nothing" );

	store.files["/game/sdk/maps/instances/box.vmf"] = Vmf( codec,
	    []( scene::DocumentEdit &edit )
	    {
		    edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 4, 4, 4 ) }, Texture() ) );
	    } );
	checks.That( preview.Refresh() && preview.Revision() != revision,
	    "a changed file on disk: Refresh reports it and bumps the revision" );
	auto changed = preview.Content( a );
	checks.That(
	    changed && changed != content && SolidBoxes( changed->objects ).size() == 1 &&
	        HasBox( SolidBoxes( changed->objects ), Vec3d( 96, 200, 0 ), Vec3d( 100, 204, 4 ) ),
	    "the content follows the file" );
	checks.That( preview.Content( Instance( "instances/nothere.vmf", "0 0 0" ) )->status ==
	                 ports::InstanceStatus::NotFound,
	    "still missing" );
	store.files["/game/sdk/maps/instances/nothere.vmf"] =
	    store.files["/game/sdk/maps/instances/outer.vmf"];
	checks.That( preview.Refresh(), "a file that appeared: Refresh reports it" );
	checks.That( preview.Content( Instance( "instances/nothere.vmf", "0 0 0" ) )->status ==
	                 ports::InstanceStatus::Placed,
	    "and the instance draws" );
	const std::uint64_t beforePath = preview.Revision();
	preview.SetDocumentPath( "/elsewhere/test.vmf" );
	checks.That( preview.Revision() != beforePath &&
	                 preview.Content( a )->status == ports::InstanceStatus::NotFound,
	    "a new document path forgets and looks up again" );
	return checks.Report();
}
