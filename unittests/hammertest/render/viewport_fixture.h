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
