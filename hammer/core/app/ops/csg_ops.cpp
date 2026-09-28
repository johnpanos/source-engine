//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/csg_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/csg_ops.h"

#include "hammer/scene/map_queries.h"
#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/polytope.h"
#include "mapgeometry/vec3.h"

#include <cmath>
#include <set>

namespace hammer::app::ops
{

using mapgeometry::Plane;
using mapgeometry::Vec3d;

namespace
{

scene::Side CapSide( const Plane &plane, const scene::FaceTexture &texture, bool worldAlign )
{
	scene::Side side;
	side.points = scene::PointsFromPlane( plane );
	side.texture = worldAlign ? scene::WorldAlignedTexture( texture, plane.normal ) : texture;
	return side;
}

// 'solid' with an extra side; the closed, normalized result or nothing.
std::optional<scene::Solid> WithSide( const scene::Solid &solid, scene::Side side )
{
	scene::Solid out = solid;
	out.sides.push_back( std::move( side ) );
	return scene::NormalizeSides( out );
}

// Clears persistent side ids so the document allocates fresh ones.
scene::Solid Fresh( scene::Solid solid )
{
	solid.vmfId = 0;
	for ( scene::Side &side : solid.sides )
	{
		side.vmfId = 0;
	}
	return solid;
}

std::vector<scene::ObjectId> SolidsOf(
    const scene::DocumentReader &doc, const std::vector<scene::ObjectId> &ids )
{
	std::vector<scene::ObjectId> out;
	for ( scene::ObjectId id : scene::ExpandToLeaves( doc, ids ) )
	{
		if ( doc.FindSolid( id ) )
		{
			out.push_back( id );
		}
	}
	return out;
}

// Replaces solid 'id' by 'pieces': the first keeps the id, the rest are new.
void ReplaceWithPieces( scene::DocumentEdit &edit, scene::ObjectId id,
    std::vector<scene::Solid> pieces, std::vector<scene::ObjectId> *created )
{
	if ( pieces.empty() )
	{
		edit.Remove( id );
		return;
	}
	const scene::Solid original = *edit.FindSolid( id );
	scene::Solid first = std::move( pieces.front() );
	first.id = id;
	first.vmfId = original.vmfId;
	// New cap sides (vmfId 0) need ids; the first piece is staged by Put, so
	// allocate here.
	for ( scene::Side &side : first.sides )
	{
		if ( side.vmfId == 0 )
		{
			side.vmfId = edit.AllocateVmfId();
		}
	}
	edit.Put( std::move( first ) );
	for ( std::size_t i = 1; i < pieces.size(); ++i )
	{
		const scene::ObjectId added = edit.Add( Fresh( std::move( pieces[i] ) ) );
		if ( created )
		{
			created->push_back( added );
		}
	}
}

} // namespace

SplitResult SplitSolid(
    const scene::Solid &solid, const Plane &plane, const scene::FaceTexture &capTexture )
{
	SplitResult out;
	const Plane n = mapgeometry::PlaneThrough( plane.normal * plane.dist, plane.normal );
	out.back = WithSide( solid, CapSide( n, capTexture, true ) );
	out.front = WithSide( solid, CapSide( mapgeometry::Flipped( n ), capTexture, true ) );
	return out;
}

EditResult ClipSolids( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    const Plane &plane, ClipKeep keep, const scene::FaceTexture &capTexture,
    std::vector<scene::ObjectId> *created )
{
	if ( mapgeometry::Length( plane.normal ) < 1.0e-9 )
	{
		return Reject( "the clip plane has no normal" );
	}
	if ( capTexture.material.empty() )
	{
		return Reject( "no material for the clipped faces" );
	}
	const std::vector<scene::ObjectId> solids = SolidsOf( edit, ids );
	if ( solids.empty() )
	{
		return NothingToDo( "no solids selected" );
	}
	const Plane unit =
	    mapgeometry::PlaneThrough( mapgeometry::Normalize( plane.normal ) *
	                                   ( plane.dist / mapgeometry::Length( plane.normal ) ),
	        plane.normal );

	bool any = false;
	for ( scene::ObjectId id : solids )
	{
		const scene::Solid &solid = *edit.FindSolid( id );
		const std::vector<Vec3d> vertices =
		    mapgeometry::SolidVertices( scene::BuildGeometry( solid ) );
		if ( mapgeometry::ClassifyPoints( vertices, unit ) != mapgeometry::PlaneSide::Spanning )
		{
			continue;
		}
		SplitResult split = SplitSolid( solid, unit, capTexture );
		std::vector<scene::Solid> pieces;
		if ( ( keep == ClipKeep::Back || keep == ClipKeep::Both ) && split.back )
		{
			pieces.push_back( std::move( *split.back ) );
		}
		if ( ( keep == ClipKeep::Front || keep == ClipKeep::Both ) && split.front )
		{
			pieces.push_back( std::move( *split.front ) );
		}
		ReplaceWithPieces( edit, id, std::move( pieces ), created );
		any = true;
	}
	if ( !any )
	{
		return NothingToDo( "the plane does not cross any selected solid" );
	}
	return {};
}

EditResult Carve( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &carvers,
    const std::vector<scene::ObjectId> &targets, std::vector<scene::ObjectId> *created )
{
	const std::vector<scene::ObjectId> carverSolids = SolidsOf( edit, carvers );
	if ( carverSolids.empty() )
	{
		return NothingToDo( "no carving solids selected" );
	}
	const std::set<scene::ObjectId> carverSet( carverSolids.begin(), carverSolids.end() );
	std::vector<scene::ObjectId> targetSolids;
	for ( scene::ObjectId id : SolidsOf( edit, targets ) )
	{
		if ( !carverSet.count( id ) )
		{
			targetSolids.push_back( id );
		}
	}

	bool any = false;
	for ( scene::ObjectId carverId : carverSolids )
	{
		const scene::Solid carver = *edit.FindSolid( carverId );
		const mapgeometry::BrushSolid carverGeometry = scene::BuildGeometry( carver );
		std::vector<scene::ObjectId> next;
		for ( scene::ObjectId targetId : targetSolids )
		{
			const scene::Solid *target = edit.FindSolid( targetId );
			if ( !target )
			{
				continue;
			}
			if ( !mapgeometry::SolidsOverlap( scene::BuildGeometry( *target ), carverGeometry ) )
			{
				next.push_back( targetId );
				continue;
			}
			// Peel off the part outside each carver face; what stays inside all
			// of them is the carved-away core.
			std::vector<scene::Solid> pieces;
			scene::Solid remaining = *target;
			for ( const mapgeometry::BrushFace &face : carverGeometry.faces )
			{
				const scene::Side &carverSide =
				    carver.sides[static_cast<std::size_t>( face.sourcePlane )];
				scene::Side outside =
				    CapSide( mapgeometry::Flipped( face.plane ), carverSide.texture, false );
				if ( std::optional<scene::Solid> piece = WithSide( remaining, outside ) )
				{
					pieces.push_back( std::move( *piece ) );
				}
				scene::Side inside = CapSide( face.plane, carverSide.texture, false );
				std::optional<scene::Solid> rest = WithSide( remaining, inside );
				if ( !rest )
				{
					break;
				}
				remaining = std::move( *rest );
			}
			std::vector<scene::ObjectId> added;
			ReplaceWithPieces( edit, targetId, std::move( pieces ), &added );
			if ( edit.FindSolid( targetId ) )
			{
				next.push_back( targetId );
			}
			next.insert( next.end(), added.begin(), added.end() );
			if ( created )
			{
				created->insert( created->end(), added.begin(), added.end() );
			}
			any = true;
		}
		targetSolids = std::move( next );
	}
	if ( !any )
	{
		return NothingToDo( "the carving solids overlap nothing" );
	}
	return {};
}

EditResult Hollow( scene::DocumentEdit &edit, const std::vector<scene::ObjectId> &ids,
    double thickness, std::vector<scene::ObjectId> *groups )
{
	if ( thickness == 0.0 )
	{
		return Reject( "wall thickness cannot be zero" );
	}
	const std::vector<scene::ObjectId> solids = SolidsOf( edit, ids );
	if ( solids.empty() )
	{
		return NothingToDo( "no solids selected" );
	}
	// Build every result first so a too-thin solid refuses the whole operation.
	struct Plan
	{
		scene::ObjectId id;
		std::vector<scene::Solid> walls;
	};
	std::vector<Plan> plans;
	for ( scene::ObjectId id : solids )
	{
		const scene::Solid &solid = *edit.FindSolid( id );
		// The offset copy: every plane moved inward (or outward) by the thickness.
		scene::Solid offset = solid;
		for ( scene::Side &side : offset.sides )
		{
			const Plane p = side.Plane();
			for ( Vec3d &point : side.points )
			{
				point = point - p.normal * thickness;
			}
		}
		std::optional<scene::Solid> core = scene::NormalizeSides( offset );
		if ( !core ||
		     core->sides.size() != scene::NormalizeSides( solid ).value_or( solid ).sides.size() )
		{
			return Reject( "a solid is too thin for walls of " +
			               scene::FormatNumber( std::fabs( thickness ) ) );
		}
		// Walls: the outer solid carved by the inner one.
		const scene::Solid &outer = thickness > 0 ? solid : *core;
		const scene::Solid &inner = thickness > 0 ? *core : solid;
		const mapgeometry::BrushSolid innerGeometry = scene::BuildGeometry( inner );
		Plan plan{ id, {} };
		scene::Solid remaining = outer;
		for ( const mapgeometry::BrushFace &face : innerGeometry.faces )
		{
			const scene::Side &innerSide =
			    inner.sides[static_cast<std::size_t>( face.sourcePlane )];
			scene::Side outside =
			    CapSide( mapgeometry::Flipped( face.plane ), innerSide.texture, false );
			if ( std::optional<scene::Solid> wall = WithSide( remaining, outside ) )
			{
				plan.walls.push_back( std::move( *wall ) );
			}
			std::optional<scene::Solid> rest =
			    WithSide( remaining, CapSide( face.plane, innerSide.texture, false ) );
			if ( !rest )
			{
				break;
			}
			remaining = std::move( *rest );
		}
		if ( plan.walls.empty() )
		{
			return Reject( "hollowing produced no walls" );
		}
		plans.push_back( std::move( plan ) );
	}

	for ( Plan &plan : plans )
	{
		const scene::Solid original = *edit.FindSolid( plan.id );
		scene::Group group;
		group.group = original.owner.IsValid() ? scene::ObjectId() : original.group;
		const scene::ObjectId groupId =
		    original.owner.IsValid() ? scene::ObjectId() : edit.Add( group );
		edit.Remove( plan.id );
		for ( scene::Solid &wall : plan.walls )
		{
			scene::Solid w = Fresh( std::move( wall ) );
			w.owner = original.owner;
			w.group = original.owner.IsValid() ? original.group : groupId;
			edit.Add( std::move( w ) );
		}
		if ( groups && groupId.IsValid() )
		{
			groups->push_back( groupId );
		}
	}
	return {};
}

} // namespace hammer::app::ops
