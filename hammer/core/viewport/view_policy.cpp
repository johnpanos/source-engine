//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Presentation policies picking and extraction share (RFC 0002,
//			hammer.viewport). See public/hammer/viewport/view_policy.h.
//
//=============================================================================//

#include "hammer/viewport/view_policy.h"

#include "hammer/scene/map_queries.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cmath>

namespace hammer::viewport
{

using mapgeometry::Vec3d;

bool IsShown(
    const VisibilityPredicate &predicate, const scene::DocumentReader &doc, scene::ObjectId id )
{
	return predicate ? predicate( doc, id ) : scene::IsVisible( doc, id );
}

VisibilityPredicate ShowEverything()
{
	return []( const scene::DocumentReader &doc, scene::ObjectId id )
	{
		return doc.KindOf( id ).has_value();
	};
}

std::optional<scene::Box> EntityMarkerBox(
    const scene::Entity &entity, const ports::IEntityCatalog *catalog, double pointHalfSize )
{
	const std::optional<Vec3d> origin = entity.Origin();
	if ( !origin )
	{
		return std::nullopt;
	}
	if ( catalog )
	{
		const ports::EntityClassInfo *info = catalog->Find( entity.classname );
		if ( info && info->boxMins && info->boxMaxs )
		{
			scene::Box box{ *origin + *info->boxMins, *origin + *info->boxMaxs };
			// A schema with swapped corners still gives a proper box.
			scene::Box ordered = scene::PointBox( box.mins );
			ordered.Extend( box.maxs );
			return ordered;
		}
	}
	const double half = std::isfinite( pointHalfSize ) ? std::fabs( pointHalfSize ) : 0.0;
	const Vec3d extent( half, half, half );
	return scene::Box{ *origin - extent, *origin + extent };
}

std::optional<scene::Rgb> CatalogColor(
    const ports::IEntityCatalog *catalog, std::string_view classname )
{
	if ( !catalog )
	{
		return std::nullopt;
	}
	const ports::EntityClassInfo *info = catalog->Find( classname );
	if ( !info || !info->color )
	{
		return std::nullopt;
	}
	const auto channel = []( double value )
	{
		if ( !std::isfinite( value ) )
		{
			return 0;
		}
		return static_cast<int>( std::clamp( std::round( value ), 0.0, 255.0 ) );
	};
	return scene::Rgb{
	    channel( info->color->x ), channel( info->color->y ), channel( info->color->z ) };
}

scene::Rgb SolidColor( const scene::DocumentReader &doc, const scene::Solid &solid,
    const ports::IEntityCatalog *catalog )
{
	if ( solid.editor.color )
	{
		return *solid.editor.color;
	}
	if ( solid.owner.IsValid() )
	{
		if ( const scene::Entity *owner = doc.FindEntity( solid.owner ) )
		{
			return CatalogColor( catalog, owner->classname ).value_or( kDefaultEntityColor );
		}
	}
	return kDefaultWorldColor;
}

scene::Rgb EntityColor( const scene::Entity &entity, const ports::IEntityCatalog *catalog )
{
	if ( const std::optional<scene::Rgb> color = CatalogColor( catalog, entity.classname ) )
	{
		return *color;
	}
	return entity.editor.color.value_or( kDefaultEntityColor );
}

void AppendUniqueEdges( std::vector<WorldEdge> &edges, const std::vector<Vec3d> &polygon )
{
	const std::size_t count = polygon.size();
	if ( count < 2 )
	{
		return;
	}
	for ( std::size_t i = 0; i < count; ++i )
	{
		const Vec3d &a = polygon[i];
		const Vec3d &b = polygon[( i + 1 ) % count];
		if ( mapgeometry::NearlyEqual( a, b, kEdgeEpsilon ) )
		{
			continue;
		}
		const bool known = std::any_of( edges.begin(), edges.end(),
		    [&]( const WorldEdge &edge )
		    {
			    return ( mapgeometry::NearlyEqual( edge.a, a, kEdgeEpsilon ) &&
			               mapgeometry::NearlyEqual( edge.b, b, kEdgeEpsilon ) ) ||
			           ( mapgeometry::NearlyEqual( edge.a, b, kEdgeEpsilon ) &&
			               mapgeometry::NearlyEqual( edge.b, a, kEdgeEpsilon ) );
		    } );
		if ( !known )
		{
			edges.push_back( { a, b } );
		}
	}
}

std::vector<WorldEdge> UniqueEdges( const mapgeometry::BrushSolid &solid )
{
	std::vector<WorldEdge> edges;
	for ( const mapgeometry::BrushFace &face : solid.faces )
	{
		AppendUniqueEdges( edges, face.vertices );
	}
	return edges;
}

} // namespace hammer::viewport
