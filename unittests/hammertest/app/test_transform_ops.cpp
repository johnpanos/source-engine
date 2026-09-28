//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app transform operations (RFC 0002 "Transform operations"):
//			translation, rotation, scale-to-box, mirror, snap and align over
//			solids, point and brush entities and groups; texture lock on and
//			off; entity angle composition (angles and legacy yaw-only angle);
//			mirrored side winding stays outward; displacement rows follow the
//			map. Negative checks: degenerate maps refuse the whole selection
//			with nothing staged, bad axes, flat target boxes, empty selections.
//
//=============================================================================//

#include "hammer/app/ops/texture_ops.h"
#include "hammer/app/ops/transform_ops.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

using namespace hammer;
using namespace hammer::app::ops;
using mapgeometry::Vec3d;

namespace
{

bool OutwardPlanes( const scene::Solid &s )
{
	const std::optional<scene::Box> b = scene::SolidBounds( s );
	if ( !b )
		return false;
	for ( const scene::Side &side : s.sides )
		if ( mapgeometry::PlaneDistance( side.Plane(), b->Center() ) >= 0 )
			return false;
	return true;
}

} // namespace

int main()
{
	testing::Checks checks;

	scene::MapDocument doc;
	scene::FaceTexture tex;
	tex.material = "DEV/DEV_MEASUREGENERIC01B";
	scene::ObjectId box, light, prop, door, doorSolid, group, grouped;
	{
		scene::DocumentEdit edit( doc );
		box = edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, 0 ), Vec3d( 64, 32, 16 ) }, tex ) );
		scene::Entity l;
		l.classname = "light";
		l.SetOrigin( Vec3d( 8, 8, 8 ) );
		light = edit.Add( l );
		scene::Entity p;
		p.classname = "prop_static";
		p.SetOrigin( Vec3d( 100, 0, 0 ) );
		p.SetAngles( Vec3d( 0, 0, 0 ) );
		prop = edit.Add( p );
		scene::Entity d;
		d.classname = "func_door";
		d.SetKey( "angle", "90" );
		door = edit.Add( d );
		scene::Solid ds = scene::MakeBoxSolid( { Vec3d( 0, 100, 0 ), Vec3d( 8, 164, 128 ) }, tex );
		ds.owner = door;
		doorSolid = edit.Add( ds );
		group = edit.Add( scene::Group{} );
		scene::Solid gs = scene::MakeBoxSolid( { Vec3d( 200, 0, 0 ), Vec3d( 216, 16, 16 ) }, tex );
		gs.group = group;
		grouped = edit.Add( gs );
		scene::CommitEdit( doc, edit );
	}

	// Translation moves everything, with texture lock.
	{
		scene::DocumentEdit edit( doc );
		checks.That( Translate( edit, { box, light, door, group }, Vec3d( 16, 0, 0 ) ).HasValue(),
		    "translate" );
		const std::optional<scene::Box> b = scene::SolidBounds( *edit.FindSolid( box ) );
		checks.That(
		    b && b->mins == Vec3d( 16, 0, 0 ) && b->maxs == Vec3d( 80, 32, 16 ), "solid moved" );
		checks.That(
		    edit.FindEntity( light )->Origin() == Vec3d( 24, 8, 8 ), "entity origin moved" );
		checks.That(
		    scene::SolidBounds( *edit.FindSolid( doorSolid ) )->mins == Vec3d( 16, 100, 0 ),
		    "brush entity solids move with the entity" );
		checks.That( scene::SolidBounds( *edit.FindSolid( grouped ) )->mins == Vec3d( 216, 0, 0 ),
		    "group members move" );
		checks.That(
		    *edit.FindEntity( door )->Key( "angle" ) == "90", "translation leaves orientation" );
		const scene::Side &top0 = doc.FindSolid( box )->sides[0];
		const scene::Side &top1 = edit.FindSolid( box )->sides[0];
		checks.Near( TexelU( top0.texture, Vec3d( 10, 5, 16 ) ),
		    TexelU( top1.texture, Vec3d( 26, 5, 16 ) ), 1e-9,
		    "texture lock: the texel follows the face" );
		checks.That(
		    Translate( edit, { box }, Vec3d() ).Error().code == app::EditErrorCode::Nothing,
		    "zero translation is nothing (negative)" );
	}
	{
		scene::DocumentEdit edit( doc );
		TransformOptions noLock;
		noLock.textureLock = false;
		checks.That( Translate( edit, { box }, Vec3d( 16, 0, 0 ), noLock ).HasValue(),
		    "translate unlocked" );
		checks.That(
		    edit.FindSolid( box )->sides[0].texture == doc.FindSolid( box )->sides[0].texture,
		    "without lock the texture stays in world space" );
	}

	// Rotation: solids, entity angles, legacy yaw.
	{
		scene::DocumentEdit edit( doc );
		checks.That( Rotate( edit, { box, prop, door }, 2, 90, Vec3d( 0, 0, 0 ) ).HasValue(),
		    "rotate 90 about Z" );
		const std::optional<scene::Box> b = scene::SolidBounds( *edit.FindSolid( box ) );
		checks.That( b && b->mins == Vec3d( -32, 0, 0 ) && b->maxs == Vec3d( 0, 64, 16 ),
		    "exact quarter turn" );
		checks.That( OutwardPlanes( *edit.FindSolid( box ) ), "rotated planes face out" );
		checks.That(
		    edit.FindEntity( prop )->Origin() == Vec3d( 0, 100, 0 ), "entity origin rotates" );
		checks.That( *edit.FindEntity( prop )->Key( "angles" ) == "0 90 0",
		    "angles compose with the rotation" );
		checks.That(
		    *edit.FindEntity( door )->Key( "angle" ) == "180", "legacy yaw-only angle rotates" );
		checks.That( !Rotate( edit, { box }, 3, 90, Vec3d() ), "bad axis refused (negative)" );
		checks.That( !Rotate( edit, { box }, 2, 360, Vec3d() ), "full turn is nothing (negative)" );
	}
	{
		scene::DocumentEdit edit( doc );
		checks.That(
		    Rotate( edit, { prop }, 1, 90, Vec3d( 100, 0, 0 ) ).HasValue(), "pitch the prop" );
		// Right-handed +90 about Y turns +X to -Z: forward points down, which is
		// a positive Source pitch.
		checks.That( *edit.FindEntity( prop )->Key( "angles" ) == "90 0 0",
		    "rotation about Y pitches the forward axis down" );
	}

	// Scale to box and mirror.
	{
		scene::DocumentEdit edit( doc );
		const scene::Box from{ Vec3d( 0, 0, 0 ), Vec3d( 64, 32, 16 ) };
		const scene::Box to{ Vec3d( 0, 0, 0 ), Vec3d( 128, 32, 32 ) };
		checks.That( ScaleToBox( edit, { box }, from, to ).HasValue(), "scale to a box" );
		checks.That( scene::SolidBounds( *edit.FindSolid( box ) )->maxs == Vec3d( 128, 32, 32 ),
		    "scaled bounds" );
		const scene::Side &side0 = doc.FindSolid( box )->sides[0];
		const scene::Side &side1 = edit.FindSolid( box )->sides[0];
		checks.Near( TexelU( side0.texture, Vec3d( 32, 8, 16 ) ),
		    TexelU( side1.texture, Vec3d( 64, 8, 32 ) ), 1e-9,
		    "texture lock stretches with the scale" );
		checks.That( !ScaleToBox( edit, { box }, from, { Vec3d( 0, 0, 0 ), Vec3d( 0, 32, 16 ) } ),
		    "a flat target refuses (negative)" );
	}
	{
		scene::DocumentEdit edit( doc );
		checks.That( Mirror( edit, { box, prop }, 0, Vec3d( 0, 0, 0 ) ).HasValue(), "mirror X" );
		const std::optional<scene::Box> b = scene::SolidBounds( *edit.FindSolid( box ) );
		checks.That( b && b->mins == Vec3d( -64, 0, 0 ) && b->maxs == Vec3d( 0, 32, 16 ),
		    "mirrored bounds" );
		checks.That(
		    OutwardPlanes( *edit.FindSolid( box ) ), "mirrored winding keeps planes outward" );
		checks.That(
		    *edit.FindEntity( prop )->Key( "angles" ) == "0 180 0", "mirrored forward axis" );
		const scene::Side &s0 = doc.FindSolid( box )->sides[0];
		const scene::Side &s1 = edit.FindSolid( box )->sides[0];
		checks.Near( TexelU( s0.texture, Vec3d( 10, 5, 16 ) ),
		    TexelU( s1.texture, Vec3d( -10, 5, 16 ) ), 1e-9, "texture lock mirrors the texture" );
	}

	// Degenerate: a zero scale refuses everything with nothing staged.
	{
		scene::DocumentEdit edit( doc );
		mapgeometry::Affine flat =
		    mapgeometry::Affine::About( mapgeometry::Mat3::Scale( Vec3d( 1, 1, 0 ) ), Vec3d() );
		checks.That( !TransformObjects( edit, { light, box }, flat ) && edit.Finish().Empty(),
		    "a degenerate map refuses atomically (negative)" );
		checks.That(
		    TransformObjects( edit, {}, mapgeometry::Affine::Translation( Vec3d( 1, 0, 0 ) ) )
		            .Error()
		            .code == app::EditErrorCode::Nothing,
		    "an empty selection is nothing (negative)" );
	}

	// Snap and align.
	{
		scene::DocumentEdit edit( doc );
		checks.That( Translate( edit, { box }, Vec3d( 3, -5, 1 ) ).HasValue(), "offset the box" );
		checks.That( SnapToGrid( edit, { box }, 8 ).HasValue(), "snap" );
		checks.That( scene::SolidBounds( *edit.FindSolid( box ) )->mins == Vec3d( 0, -8, 0 ),
		    "snapped to the grid" );
		checks.That( SnapToGrid( edit, { box }, 8 ).Error().code == app::EditErrorCode::Nothing,
		    "snapping an aligned selection is nothing" );
		checks.That( !SnapToGrid( edit, { box }, 0 ), "zero grid refused (negative)" );
		checks.That( AlignObjects( edit, { box, grouped }, 2, true ).Error().code ==
		                 app::EditErrorCode::Nothing,
		    "level tops are already aligned" );
		checks.That(
		    AlignObjects( edit, { box, grouped }, 0, true ).HasValue(), "align right edges" );
		checks.Near( scene::SolidBounds( *edit.FindSolid( box ) )->maxs.x,
		    scene::SolidBounds( *edit.FindSolid( grouped ) )->maxs.x, 0, "right edges aligned" );
		checks.That(
		    !AlignObjects( edit, { box }, 2, true ), "align needs two objects (negative)" );
	}

	// Displacement rows follow the map.
	{
		scene::DocumentEdit edit( doc );
		scene::Solid *s = edit.MutableSolid( box );
		scene::Displacement disp;
		disp.power = 2;
		disp.startPosition = Vec3d( 0, 0, 16 );
		disp.normals = std::vector<Vec3d>{ Vec3d( 0, 0, 1 ), Vec3d( 1, 0, 0 ) };
		disp.distances = std::vector<double>{ 4, 2 };
		disp.offsets = std::vector<Vec3d>{ Vec3d( 0, 0, 3 ) };
		disp.offsetNormals = std::vector<Vec3d>{ Vec3d( 0, 0, 1 ) };
		s->sides[0].displacement = disp;
		mapgeometry::Affine xf =
		    mapgeometry::Affine::About( mapgeometry::Mat3::Scale( Vec3d( 1, 1, 2 ) ), Vec3d() );
		checks.That( TransformObjects( edit, { box }, xf ).HasValue(), "scale a displaced solid" );
		const scene::Displacement &out = *edit.FindSolid( box )->sides[0].displacement;
		checks.That( out.startPosition == Vec3d( 0, 0, 32 ), "start position is a point" );
		checks.That( *out.normals == std::vector<Vec3d>{ Vec3d( 0, 0, 1 ), Vec3d( 1, 0, 0 ) },
		    "normals stay unit" );
		checks.That( *out.distances == std::vector<double>{ 8, 2 },
		    "distances absorb the normal's stretch" );
		checks.That(
		    *out.offsets == std::vector<Vec3d>{ Vec3d( 0, 0, 6 ) }, "offsets are vectors" );
		checks.That( *out.offsetNormals == std::vector<Vec3d>{ Vec3d( 0, 0, 1 ) },
		    "offset normals are renormalized" );
		checks.That( out.power == 2 && !out.alphas, "other displacement fields are untouched" );
	}

	return checks.Report();
}
