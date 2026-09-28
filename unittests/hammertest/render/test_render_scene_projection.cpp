//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.adapters.render (RFC 0002, RFC 0016 A.7): the editor's own
//			render scene follows the document. Each instance's world bounds
//			equal its solid's bounds; a move is a transform update of the same
//			instance; a deleted solid is removed; an unchanged document commits
//			nothing; and the editor's scene is independent of any other scene.
//
//=============================================================================//

#include "hammer/adapters/render/render_scene_projection.h"
#include "kvtext/keyvalues.h"
#include "testing/checks.h"

#include <cmath>
#include <string>

namespace
{

std::string Box( int id, int x0, int y0, int z0, int x1, int y1, int z1 )
{
	auto plane = [&]( int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz )
	{
		return "side { \"plane\" \"(" + std::to_string( ax ) + " " + std::to_string( ay ) + " " +
		       std::to_string( az ) + ") (" + std::to_string( bx ) + " " + std::to_string( by ) +
		       " " + std::to_string( bz ) + ") (" + std::to_string( cx ) + " " +
		       std::to_string( cy ) + " " + std::to_string( cz ) +
		       ")\" \"material\" \"DEV/DEV_MEASUREGENERIC01\" }\n";
	};
	std::string s = "solid { \"id\" \"" + std::to_string( id ) + "\"\n";
	s += plane( x0, y1, z1, x1, y1, z1, x1, y0, z1 ); // top
	s += plane( x0, y0, z0, x1, y0, z0, x1, y1, z0 ); // bottom
	s += plane( x0, y1, z1, x0, y0, z1, x0, y0, z0 ); // left
	s += plane( x1, y1, z0, x1, y0, z0, x1, y0, z1 ); // right
	s += plane( x1, y1, z1, x0, y1, z1, x0, y1, z0 ); // back
	s += plane( x1, y0, z0, x0, y0, z0, x0, y0, z1 ); // front
	return s + "}\n";
}

kvtext::KeyValueNode Document( const std::string &solids )
{
	const std::string text = "world { \"id\" \"1\" \"classname\" \"worldspawn\"\n" + solids + "}\n";
	return kvtext::ParseKeyValues( text ).root;
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
	hammer::render_adapter::SceneProjection projection( { &render::scene::CreateRenderScene } );

	auto first = projection.Sync(
	    Document( Box( 2, 0, 0, 0, 64, 64, 64 ) + Box( 3, 128, 0, 0, 192, 32, 16 ) ) );
	checks.That( first && first.Value().added == 2 && first.Value().committed,
	    "hammer.render.solids-become-instances" );
	auto snapshot = projection.Scene().Snapshot();
	checks.That( Matches( *snapshot, 0, 0, 0, 0, 64, 64, 64 ) &&
	                 Matches( *snapshot, 1, 128, 0, 0, 192, 32, 16 ),
	    "hammer.render.instance-bounds-equal-solid-bounds" );
	const render::scene::InstanceId movedId = snapshot->instances[1].id;

	auto same = projection.Sync(
	    Document( Box( 2, 0, 0, 0, 64, 64, 64 ) + Box( 3, 128, 0, 0, 192, 32, 16 ) ) );
	checks.That( same && !same.Value().committed && same.Value().revision == first.Value().revision,
	    "hammer.render.an-unchanged-document-commits-nothing" );

	auto moved = projection.Sync(
	    Document( Box( 2, 0, 0, 0, 64, 64, 64 ) + Box( 3, 256, 0, 0, 320, 32, 16 ) ) );
	snapshot = projection.Scene().Snapshot();
	checks.That( moved && moved.Value().updated == 1 && moved.Value().added == 0 &&
	                 snapshot->instances[1].id == movedId &&
	                 Matches( *snapshot, 1, 256, 0, 0, 320, 32, 16 ),
	    "hammer.render.a-move-updates-the-same-instance" );

	auto removed = projection.Sync( Document( Box( 3, 256, 0, 0, 320, 32, 16 ) ) );
	snapshot = projection.Scene().Snapshot();
	checks.That( removed && removed.Value().removed == 1 && snapshot->instances.size() == 1 &&
	                 snapshot->instances[0].id == movedId,
	    "hammer.render.a-deleted-solid-is-removed" );

	auto other = render::scene::CreateRenderScene();
	checks.That( other->Snapshot()->instances.empty() && projection.SolidCount() == 1,
	    "hammer.render.the-editor-scene-is-its-own" );
	return checks.Report();
}
