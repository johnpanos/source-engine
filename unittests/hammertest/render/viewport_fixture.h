//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared fixture of the hammer.adapters.render viewport suites: a
//			small document (two boxes, one selected, and a light) as a render
//			snapshot, and cameras framed on it.
//
//=============================================================================//

#ifndef HAMMERTEST_RENDER_VIEWPORT_FIXTURE_H
#define HAMMERTEST_RENDER_VIEWPORT_FIXTURE_H

#include "hammer/scene/change_set.h"
#include "hammer/scene/solid_geometry.h"
#include "hammer/viewport/camera.h"
#include "hammer/viewport/extraction.h"
#include "mapgeometry/vec3.h"

#include <algorithm>

namespace hammertest::viewport_render
{

using hammer::scene::ObjectId;
using mapgeometry::Vec3d;

struct Document
{
	hammer::scene::MapDocument doc;
	ObjectId left;  // (-128, -64, 0) .. (0, 64, 128), selected
	ObjectId right; // (64, -64, 0) .. (192, 64, 64)
	ObjectId light; // at (0, 0, 256)
};

inline void Build( Document &d )
{
	hammer::scene::DocumentEdit edit( d.doc );
	hammer::scene::FaceTexture texture;
	texture.material = "DEV/DEV_MEASUREGENERIC01B";
	d.left = edit.Add( hammer::scene::MakeBoxSolid(
	    hammer::scene::Box{ Vec3d( -128, -64, 0 ), Vec3d( 0, 64, 128 ) }, texture ) );
	d.right = edit.Add( hammer::scene::MakeBoxSolid(
	    hammer::scene::Box{ Vec3d( 64, -64, 0 ), Vec3d( 192, 64, 64 ) }, texture ) );
	hammer::scene::Entity light;
	light.classname = "light";
	light.SetOrigin( Vec3d( 0, 0, 256 ) );
	d.light = edit.Add( light );
	hammer::scene::CommitEdit( d.doc, edit );
}

inline hammer::viewport::RenderSnapshot Snapshot( const Document &d, bool selectLeft = true )
{
	hammer::viewport::SelectionInput selection;
	if ( selectLeft )
		selection.objects.push_back( d.left );
	return hammer::viewport::Extract( d.doc, selection );
}

inline void Translate( hammer::viewport::SolidDraw &solid, const Vec3d &by )
{
	for ( hammer::viewport::FaceDraw &face : solid.faces )
	{
		for ( Vec3d &v : face.vertices )
			v = v + by;
	}
	solid.bounds.mins = solid.bounds.mins + by;
	solid.bounds.maxs = solid.bounds.maxs + by;
}

inline void Rebound( hammer::viewport::RenderSnapshot &snapshot )
{
	snapshot.bounds.reset();
	for ( const hammer::viewport::SolidDraw &solid : snapshot.solids )
	{
		if ( !snapshot.bounds )
		{
			snapshot.bounds = solid.bounds;
			continue;
		}
		hammer::scene::Box &b = *snapshot.bounds;
		b.mins = Vec3d( std::min( b.mins.x, solid.bounds.mins.x ),
		    std::min( b.mins.y, solid.bounds.mins.y ), std::min( b.mins.z, solid.bounds.mins.z ) );
		b.maxs = Vec3d( std::max( b.maxs.x, solid.bounds.maxs.x ),
		    std::max( b.maxs.y, solid.bounds.maxs.y ), std::max( b.maxs.z, solid.bounds.maxs.z ) );
	}
}

// The fixture's snapshot (nothing selected) and 'copies' more boxes like the
// right one, ids 128 on (chunks 2 and up of the renderer's 64-id chunks; the
// fixture's own ids carry the document serial), copy i moved 160 * (i + 1)
// along y.
inline hammer::viewport::RenderSnapshot Spread( const Document &d, int copies )
{
	hammer::viewport::RenderSnapshot snapshot = Snapshot( d, false );
	hammer::viewport::SolidDraw right;
	for ( const hammer::viewport::SolidDraw &solid : snapshot.solids )
	{
		if ( solid.id == d.right )
			right = solid;
	}
	for ( int i = 0; i < copies; ++i )
	{
		hammer::viewport::SolidDraw copy = right;
		copy.id = ObjectId{ std::uint64_t( 128 + i ) };
		Translate( copy, Vec3d( 0, 160.0 * ( i + 1 ), 0 ) );
		snapshot.solids.push_back( copy );
	}
	Rebound( snapshot );
	return snapshot;
}

// 'snapshot' with solid 'id' moved by 'by'.
inline hammer::viewport::RenderSnapshot Moved(
    hammer::viewport::RenderSnapshot snapshot, ObjectId id, const Vec3d &by )
{
	for ( hammer::viewport::SolidDraw &solid : snapshot.solids )
	{
		if ( solid.id == id )
			Translate( solid, by );
	}
	Rebound( snapshot );
	return snapshot;
}

inline hammer::viewport::Camera2D TopCamera( int width = 256, int height = 192 )
{
	hammer::viewport::Camera2D camera;
	camera.SetKind( hammer::viewport::ViewKind::Top );
	camera.SetViewport( width, height );
	camera.SetZoom( 0.5 );
	camera.SetCenter( { 32, 0 } );
	return camera;
}

inline hammer::viewport::Camera3D EyeCamera( int width = 256, int height = 192 )
{
	hammer::viewport::Camera3D camera;
	camera.SetViewport( width, height );
	camera.SetPosition( Vec3d( 32, -600, 300 ) );
	camera.LookAt( Vec3d( 32, 0, 32 ) );
	return camera;
}

} // namespace hammertest::viewport_render

#endif // HAMMERTEST_RENDER_VIEWPORT_FIXTURE_H
