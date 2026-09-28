//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.render (RFC 0002, RFC 0016 A.7): the editor's own
//			render scene follows the document through its render snapshot.
//			Each instance's world bounds equal its solid's bounds; a move is a
//			transform update of the same instance; a deleted solid is removed;
//			an unchanged document commits nothing; and the editor's scene is
//			independent of any other scene.
//
//=============================================================================//

#include "hammer/adapters/render/render_scene_projection.h"
#include "hammer/scene/change_set.h"
#include "hammer/scene/solid_geometry.h"
#include "testing/checks.h"

#include <cmath>

namespace
{

using hammer::scene::Box;
using hammer::scene::ObjectId;
using mapgeometry::Vec3d;

hammer::scene::Solid BoxSolid( Vec3d mins, Vec3d maxs )
{
	hammer::scene::FaceTexture texture;
	texture.material = "DEV/DEV_MEASUREGENERIC01";
	return hammer::scene::MakeBoxSolid( Box{ mins, maxs }, texture );
}

bool Matches( const render::scene::SceneSnapshot &snapshot, std::size_t index, float x0, float y0,
    float z0, float x1, float y1, float z1 )
{
	if ( index >= snapshot.instances.size() )
		return false;
	const render::math::Aabb &b = snapshot.instances[index].worldBounds;
	auto near = []( float a, float b )
	{
		return std::fabs( a - b ) < 1e-3f;
	};
	return near( b.min.x, x0 ) && near( b.min.y, y0 ) && near( b.min.z, z0 ) &&
	       near( b.max.x, x1 ) && near( b.max.y, y1 ) && near( b.max.z, z1 );
}

} // namespace

int main()
{
	testing::Checks checks;
	hammer::scene::MapDocument doc;
	ObjectId a;
	ObjectId b;
	{
		hammer::scene::DocumentEdit edit( doc );
		a = edit.Add( BoxSolid( Vec3d( 0, 0, 0 ), Vec3d( 64, 64, 64 ) ) );
		b = edit.Add( BoxSolid( Vec3d( 128, 0, 0 ), Vec3d( 192, 32, 16 ) ) );
		hammer::scene::CommitEdit( doc, edit );
	}
	hammer::render_adapter::SceneProjection projection( { &render::scene::CreateRenderScene } );
	auto first = projection.Sync( hammer::viewport::Extract( doc, {} ) );
	checks.That( first && first.Value().added == 2 && first.Value().committed,
	    "hammer.render.solids-become-instances" );
	auto snapshot = projection.Scene().Snapshot();
	checks.That( Matches( *snapshot, 0, 0, 0, 0, 64, 64, 64 ) &&
	                 Matches( *snapshot, 1, 128, 0, 0, 192, 32, 16 ),
	    "hammer.render.instance-bounds-equal-solid-bounds" );
	const render::scene::InstanceId movedId = snapshot->instances[1].id;

	auto same = projection.Sync( hammer::viewport::Extract( doc, {} ) );
	checks.That( same && !same.Value().committed && same.Value().revision == first.Value().revision,
	    "hammer.render.an-unchanged-document-commits-nothing" );

	{
		hammer::scene::DocumentEdit edit( doc );
		hammer::scene::Solid *moved = edit.MutableSolid( b );
		const hammer::scene::Solid replacement =
		    BoxSolid( Vec3d( 256, 0, 0 ), Vec3d( 320, 32, 16 ) );
		moved->sides = replacement.sides;
		hammer::scene::CommitEdit( doc, edit );
	}
	auto moved = projection.Sync( hammer::viewport::Extract( doc, {} ) );
	snapshot = projection.Scene().Snapshot();
	checks.That( moved && moved.Value().updated == 1 && moved.Value().added == 0 &&
	                 snapshot->instances[1].id == movedId &&
	                 Matches( *snapshot, 1, 256, 0, 0, 320, 32, 16 ),
	    "hammer.render.a-move-updates-the-same-instance" );

	{
		hammer::scene::DocumentEdit edit( doc );
		edit.Remove( a );
		hammer::scene::CommitEdit( doc, edit );
	}
	auto removed = projection.Sync( hammer::viewport::Extract( doc, {} ) );
	snapshot = projection.Scene().Snapshot();
	checks.That( removed && removed.Value().removed == 1 && snapshot->instances.size() == 1 &&
	                 snapshot->instances[0].id == movedId,
	    "hammer.render.a-deleted-solid-is-removed" );

	auto other = render::scene::CreateRenderScene();
	checks.That( other->Snapshot()->instances.empty() && projection.SolidCount() == 1,
	    "hammer.render.the-editor-scene-is-its-own" );
	return checks.Report();
}
