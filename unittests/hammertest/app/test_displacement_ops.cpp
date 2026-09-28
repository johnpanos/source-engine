//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: hammer.app displacement editing (RFC 0002, R08 domain logic):
//			creating flat displacements on quad faces (deterministic start
//			corner, legacy tags: flat = walkable + buildable), sculpting (raise
//			with falloff, lower, set, smooth), alpha painting with clamping,
//			power changes that keep grid-coincident heights, elevation, sewing a
//			crack between neighbors shut, and the steep-slope tag rule.
//			Negative checks: non-quad and already-displaced faces, powers out of
//			range, a brush that reaches nothing, radius 0, unknown faces.
//
//=============================================================================//

#include "hammer/app/ops/displacement_ops.h"
#include "hammer/scene/displacement_geometry.h"
#include "mapgeometry/polytope.h"
#include "mapgeometry/vec3.h"
#include "testing/checks.h"

#include <cmath>

using namespace hammer;
using namespace hammer::app::ops;
using mapgeometry::Vec3d;
using scene::FaceRef;

namespace
{

// The displaced vertex nearest 'p' (in XY) and its height.
double HeightAt( const scene::DocumentReader &doc, const FaceRef &face, double x, double y )
{
	const scene::Solid &s = *doc.FindSolid( face.solid );
	std::size_t index = 0;
	for ( std::size_t i = 0; i < s.sides.size(); ++i )
		if ( s.sides[i].vmfId == face.side )
			index = i;
	const auto surface = scene::BuildDisplacement( s, index );
	double best = 1e30;
	double z = -1e30;
	for ( const Vec3d &v : surface->vertices )
	{
		const double d = std::hypot( v.x - x, v.y - y );
		if ( d < best )
		{
			best = d;
			z = v.z;
		}
	}
	return z;
}

} // namespace

int main()
{
	testing::Checks checks;

	scene::FaceTexture tex;
	tex.material = "NATURE/BLENDGRASSGRAVEL001A";
	scene::MapDocument doc;
	scene::ObjectId a, b, wedge;
	{
		scene::DocumentEdit edit( doc );
		a = edit.Add( scene::MakeBoxSolid( { Vec3d( 0, 0, -16 ), Vec3d( 256, 256, 0 ) }, tex ) );
		b = edit.Add( scene::MakeBoxSolid( { Vec3d( 256, 0, -16 ), Vec3d( 512, 256, 0 ) }, tex ) );
		scene::Solid w = scene::MakeBoxSolid( { Vec3d( 0, 400, 0 ), Vec3d( 64, 464, 64 ) }, tex );
		w.sides.push_back( w.sides[0] );
		w.sides.back().points = scene::PointsFromPlane( mapgeometry::PlaneThrough(
		    Vec3d( 64, 400, 32 ), mapgeometry::Normalize( Vec3d( 1, 0, 1 ) ) ) );
		wedge = edit.Add( *scene::NormalizeSides( w ) );
		scene::CommitEdit( doc, edit );
	}
	const FaceRef topA{ a, doc.FindSolid( a )->sides[0].vmfId };
	const FaceRef topB{ b, doc.FindSolid( b )->sides[0].vmfId };

	// Creation.
	{
		scene::DocumentEdit edit( doc );
		checks.That( CreateDisplacement( edit, { topA, topB }, 3 ).HasValue(),
		    "create power-3 displacements" );
		const scene::Displacement &d = *edit.FindSolid( a )->sides[0].displacement;
		checks.That( d.power == 3 && d.normals && d.normals->size() == 81 && d.distances &&
		                 d.alphas && d.offsets && d.offsetNormals && d.allowedVerts &&
		                 d.allowedVerts->size() == 10,
		    "every array sized for power 3" );
		checks.That( d.startPosition == Vec3d( 0, 0, 0 ), "start corner is the smallest corner" );
		checks.That( d.triangleTags && d.triangleTags->size() == 128 &&
		                 ( *d.triangleTags )[0] == ( kDispTagWalkable | kDispTagBuildable ),
		    "flat triangles are walkable and buildable" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid" );
		checks.That( !CreateDisplacement( edit, { topA }, 3 ), "already displaced (negative)" );
		checks.That( !CreateDisplacement( edit, { { a, doc.FindSolid( a )->sides[1].vmfId } }, 5 ),
		    "power 5 (negative)" );
		scene::CommitEdit( const_cast<scene::MapDocument &>( doc ), edit );
	}
	// The wedge's sloped face is a quad too; its triangle face is not.
	{
		scene::DocumentEdit edit( doc );
		const scene::Solid &w = *edit.FindSolid( wedge );
		std::vector<FaceRef> tris;
		for ( std::size_t i = 0; i < w.sides.size(); ++i )
			if ( !scene::QuadCorners( w, i ) )
				tris.push_back( { wedge, w.sides[i].vmfId } );
		checks.That( !tris.empty() && !CreateDisplacement( edit, { tris[0] }, 2 ),
		    "a non-quad face refuses (negative)" );
	}

	// Sculpting.
	{
		scene::DocumentEdit edit( doc );
		SculptBrush raise;
		raise.center = Vec3d( 128, 128, 0 );
		raise.radius = 64;
		raise.amount = 32;
		checks.That( Sculpt( edit, { topA }, raise ).HasValue(), "raise" );
		checks.Near( HeightAt( edit, topA, 128, 128 ), 32, 1e-9, "full strength at the center" );
		checks.Near(
		    HeightAt( edit, topA, 160, 128 ), 16, 1e-9, "half strength halfway (linear falloff)" );
		checks.Near( HeightAt( edit, topA, 0, 0 ), 0, 1e-9, "untouched outside the radius" );
		// A 16-unit rise over the 32-unit grid spacing (normal z 0.89) keeps
		// every triangle buildable.
		bool allBuildable = true;
		for ( int t : *edit.FindSolid( a )->sides[0].displacement->triangleTags )
			allBuildable = allBuildable && ( t & kDispTagBuildable );
		checks.That( allBuildable, "gentle slopes stay buildable" );

		SculptBrush lower = raise;
		lower.mode = SculptMode::Lower;
		lower.amount = 8;
		lower.falloff = false;
		checks.That( Sculpt( edit, { topA }, lower ).HasValue(), "lower" );
		checks.Near( HeightAt( edit, topA, 128, 128 ), 24, 1e-9, "lowered" );

		// The brush reaches vertices by their displaced position.
		SculptBrush set = raise;
		set.center = Vec3d( 128, 128, 24 );
		set.mode = SculptMode::Set;
		set.amount = 10;
		set.falloff = false;
		set.radius = 1;
		checks.That( Sculpt( edit, { topA }, set ).HasValue(), "set" );
		checks.Near( HeightAt( edit, topA, 128, 128 ), 10, 1e-9, "set height" );

		SculptBrush smooth = raise;
		smooth.center = Vec3d( 128, 128, 10 );
		smooth.mode = SculptMode::Smooth;
		smooth.amount = 1;
		smooth.falloff = false;
		smooth.radius = 1;
		const double before = HeightAt( edit, topA, 128, 128 );
		checks.That( Sculpt( edit, { topA }, smooth ).HasValue(), "smooth" );
		checks.That(
		    HeightAt( edit, topA, 128, 128 ) != before, "smoothing moves toward the neighbors" );

		SculptBrush far = raise;
		far.center = Vec3d( 5000, 0, 0 );
		checks.That( Sculpt( edit, { topA }, far ).Error().code == app::EditErrorCode::Nothing,
		    "out of reach (negative)" );
		far.radius = 0;
		checks.That( !Sculpt( edit, { topA }, far ), "zero radius (negative)" );
		checks.That( !Sculpt( edit, { { a, doc.FindSolid( a )->sides[1].vmfId } }, raise ),
		    "not a displacement (negative)" );
		checks.That( !Sculpt( edit, { { a, 999999 } }, raise ), "unknown face (negative)" );
	}

	// Steep slopes lose the buildable tag, and past 45 degrees the walkable one.
	{
		scene::DocumentEdit edit( doc );
		SculptBrush spike;
		spike.center = Vec3d( 128, 128, 0 );
		spike.radius = 1;
		spike.amount = 64;
		spike.falloff = false;
		checks.That( Sculpt( edit, { topA }, spike ).HasValue(), "a spike" );
		bool unbuildable = false;
		bool unwalkable = false;
		for ( int t : *edit.FindSolid( a )->sides[0].displacement->triangleTags )
		{
			unbuildable = unbuildable || !( t & kDispTagBuildable );
			unwalkable = unwalkable || !( t & kDispTagWalkable );
		}
		checks.That( unbuildable && unwalkable, "steep triangles lose both tags" );
	}

	// Alpha.
	{
		scene::DocumentEdit edit( doc );
		checks.That(
		    PaintAlpha( edit, { topA }, Vec3d( 128, 128, 0 ), 32, 300, AlphaMode::Raise, false )
		        .HasValue(),
		    "paint alpha" );
		const scene::Displacement &d = *edit.FindSolid( a )->sides[0].displacement;
		double maxAlpha = 0;
		for ( double v : *d.alphas )
			maxAlpha = std::max( maxAlpha, v );
		checks.That( maxAlpha == 255, "alpha clamps to 255" );
		checks.That(
		    PaintAlpha( edit, { topA }, Vec3d( 128, 128, 0 ), 32, 255, AlphaMode::Set, false )
		            .Error()
		            .code == app::EditErrorCode::Nothing,
		    "no change is nothing" );
	}

	// Power changes keep coincident heights.
	{
		scene::DocumentEdit edit( doc );
		SculptBrush raise;
		raise.center = Vec3d( 128, 128, 0 );
		raise.radius = 100;
		raise.amount = 40;
		checks.That( Sculpt( edit, { topA }, raise ).HasValue(), "shape it" );
		const double centre = HeightAt( edit, topA, 128, 128 );
		checks.That( SetDisplacementPower( edit, { topA }, 4 ).HasValue(), "power 4" );
		checks.Equal( edit.FindSolid( a )->sides[0].displacement->normals->size(),
		    std::size_t( 289 ), "resampled to 17x17" );
		checks.Near(
		    HeightAt( edit, topA, 128, 128 ), centre, 1e-9, "grid-coincident heights survive" );
		checks.That(
		    SetDisplacementPower( edit, { topA }, 4 ).Error().code == app::EditErrorCode::Nothing,
		    "same power" );
		checks.That( SetDisplacementElevation( edit, { topA }, 8 ).HasValue(), "elevation" );
		checks.Near( HeightAt( edit, topA, 0, 0 ), 8, 1e-9, "elevation lifts the whole surface" );
		checks.That( scene::ValidateEdit( edit ).empty(), "valid after edits" );
	}

	// Sewing: raise the shared edge on one side only, then sew.
	{
		scene::DocumentEdit edit( doc );
		SculptBrush edge;
		edge.center = Vec3d( 256, 128, 0 );
		edge.radius = 20;
		edge.amount = 16;
		edge.falloff = false;
		checks.That( Sculpt( edit, { topA }, edge ).HasValue(), "raise A's edge" );
		checks.That(
		    HeightAt( edit, topA, 256, 128 ) != HeightAt( edit, topB, 256, 128 ), "a crack opens" );
		checks.That( SewDisplacements( edit, { topA, topB } ).HasValue(), "sew" );
		checks.Near( HeightAt( edit, topA, 256, 128 ), HeightAt( edit, topB, 256, 128 ), 1e-9,
		    "the crack is closed" );
		checks.Near( HeightAt( edit, topA, 256, 128 ), 8, 1e-9, "at the average height" );
		checks.That(
		    SewDisplacements( edit, { topA, topB } ).Error().code == app::EditErrorCode::Nothing,
		    "already sewn" );
		checks.That( DestroyDisplacement( edit, { topB } ).HasValue() &&
		                 !edit.FindSolid( b )->sides[0].displacement,
		    "destroy" );
		checks.That( !DestroyDisplacement( edit, { topB } ), "destroy twice (negative)" );
	}

	return checks.Report();
}
