//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Render-snapshot extraction for the Hammer viewports (RFC 0002,
//			hammer.viewport). See public/hammer/viewport/extraction.h.
//
//=============================================================================//

#include "hammer/viewport/extraction.h"

#include "hammer/scene/map_queries.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <set>

namespace hammer::viewport
{

using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

template <typename T> std::vector<T> SortedUnique( std::vector<T> values )
{
	std::sort( values.begin(), values.end() );
	values.erase( std::unique( values.begin(), values.end() ), values.end() );
	return values;
}

std::set<ObjectId> BrushEntities( const scene::DocumentReader &doc )
{
	std::set<ObjectId> owners;
	for ( ObjectId id : doc.SolidIds() )
	{
		const scene::Solid *solid = doc.FindSolid( id );
		if ( solid && solid->owner.IsValid() )
		{
			owners.insert( solid->owner );
		}
	}
	return owners;
}

// Everything one extraction pass needs, computed once per pass.
struct Context
{
	const scene::DocumentReader &doc;
	const ExtractOptions &options;
	const std::vector<ObjectId> &selected;            // sorted, unique
	const std::vector<scene::FaceRef> &selectedFaces; // sorted, unique
	std::set<ObjectId> brushEntities;

	bool Selected( ObjectId id ) const
	{
		ObjectId at = id;
		for ( int guard = 0; guard < 4096 && at.IsValid(); ++guard )
		{
			if ( std::binary_search( selected.begin(), selected.end(), at ) )
			{
				return true;
			}
			at = scene::ContainerOf( doc, at );
		}
		return false;
	}
};

std::optional<SolidDraw> DrawSolid( const Context &ctx, ObjectId id )
{
	const scene::Solid *solid = ctx.doc.FindSolid( id );
	if ( !solid )
	{
		return std::nullopt;
	}
	const bool shown = IsShown( ctx.options.visible, ctx.doc, id );
	if ( !shown && !ctx.options.keepHidden )
	{
		return std::nullopt;
	}
	mapgeometry::BrushSolid geometry = scene::BuildGeometry( *solid );
	if ( geometry.faces.empty() )
	{
		return std::nullopt;
	}
	SolidDraw draw;
	draw.id = id;
	draw.owner = solid->owner;
	draw.bounds = { geometry.mins, geometry.maxs };
	draw.color = SolidColor( ctx.doc, *solid, ctx.options.catalog );
	draw.selected = ctx.Selected( id );
	draw.hidden = !shown;
	draw.faces.reserve( geometry.faces.size() );
	for ( mapgeometry::BrushFace &face : geometry.faces )
	{
		FaceDraw faceDraw;
		if ( face.sourcePlane >= 0 &&
		     static_cast<std::size_t>( face.sourcePlane ) < solid->sides.size() )
		{
			faceDraw.side = solid->sides[static_cast<std::size_t>( face.sourcePlane )].vmfId;
		}
		faceDraw.material = std::move( face.material );
		faceDraw.vertices = std::move( face.vertices );
		faceDraw.normal = face.plane.normal;
		if ( face.sourcePlane >= 0 &&
		     solid->sides[static_cast<std::size_t>( face.sourcePlane )].displacement )
		{
			faceDraw.displacement =
			    scene::BuildDisplacement( *solid, static_cast<std::size_t>( face.sourcePlane ) );
		}
		faceDraw.selected = std::binary_search( ctx.selectedFaces.begin(), ctx.selectedFaces.end(),
		    scene::FaceRef{ id, faceDraw.side } );
		draw.faces.push_back( std::move( faceDraw ) );
	}
	return draw;
}

std::optional<EntityDraw> DrawEntity( const Context &ctx, ObjectId id )
{
	const scene::Entity *entity = ctx.doc.FindEntity( id );
	if ( !entity || ctx.brushEntities.count( id ) )
	{
		return std::nullopt;
	}
	const bool shown = IsShown( ctx.options.visible, ctx.doc, id );
	if ( !shown && !ctx.options.keepHidden )
	{
		return std::nullopt;
	}
	const std::optional<scene::Box> marker =
	    EntityMarkerBox( *entity, ctx.options.catalog, ctx.options.pointHalfSize );
	if ( !marker )
	{
		return std::nullopt;
	}
	EntityDraw draw;
	draw.id = id;
	draw.classname = entity->classname;
	draw.origin = entity->Origin().value_or( Vec3d() );
	draw.angles = entity->Angles().value_or( Vec3d() );
	draw.mins = marker->mins;
	draw.maxs = marker->maxs;
	draw.color = EntityColor( *entity, ctx.options.catalog );
	const ports::EntityClassInfo *info =
	    ctx.options.catalog ? ctx.options.catalog->Find( entity->classname ) : nullptr;
	if ( const std::string *model = entity->Key( "model" ); model && !model->empty() )
	{
		draw.model = *model;
	}
	else if ( info )
	{
		draw.model = info->model;
	}
	if ( info )
	{
		draw.sprite = info->sprite;
	}
	draw.selected = ctx.Selected( id );
	draw.hidden = !shown;
	return draw;
}

std::optional<scene::Box> BoundsOf( const RenderSnapshot &snapshot )
{
	std::optional<scene::Box> bounds;
	const auto extend = [&]( const scene::Box &box )
	{
		if ( bounds )
		{
			bounds->Extend( box );
		}
		else
		{
			bounds = box;
		}
	};
	for ( const SolidDraw &solid : snapshot.solids )
	{
		if ( !solid.hidden )
		{
			extend( solid.bounds );
		}
	}
	for ( const EntityDraw &entity : snapshot.entities )
	{
		if ( !entity.hidden )
		{
			extend( { entity.mins, entity.maxs } );
		}
	}
	return bounds;
}

// Inserts or replaces (value) or erases (nothing) the draw with 'id' in an
// id-ordered vector.
template <typename Draw>
void Place( std::vector<Draw> &draws, ObjectId id, std::optional<Draw> value )
{
	auto it = std::lower_bound( draws.begin(), draws.end(), id,
	    []( const Draw &draw, ObjectId key )
	    {
		    return draw.id < key;
	    } );
	const bool present = it != draws.end() && it->id == id;
	if ( value )
	{
		if ( present )
		{
			*it = std::move( *value );
		}
		else
		{
			draws.insert( it, std::move( *value ) );
		}
	}
	else if ( present )
	{
		draws.erase( it );
	}
}

template <typename T>
std::vector<T> SymmetricDifference( const std::vector<T> &a, const std::vector<T> &b )
{
	std::vector<T> out;
	std::set_symmetric_difference(
	    a.begin(), a.end(), b.begin(), b.end(), std::back_inserter( out ) );
	return out;
}

void AppendLeaves( const scene::DocumentReader &doc, ObjectId id, std::vector<ObjectId> &out )
{
	out.push_back( id );
	const std::vector<ObjectId> leaves = scene::ExpandToLeaves( doc, { id } );
	out.insert( out.end(), leaves.begin(), leaves.end() );
}

} // namespace

RenderSnapshot Extract( const scene::DocumentReader &doc, const SelectionInput &selection,
    const ExtractOptions &options )
{
	const std::vector<ObjectId> selected = SortedUnique( selection.objects );
	const std::vector<scene::FaceRef> faces = SortedUnique( selection.faces );
	const Context ctx{ doc, options, selected, faces, BrushEntities( doc ) };

	RenderSnapshot snapshot;
	for ( ObjectId id : doc.SolidIds() )
	{
		if ( std::optional<SolidDraw> draw = DrawSolid( ctx, id ) )
		{
			snapshot.solids.push_back( std::move( *draw ) );
		}
	}
	for ( ObjectId id : doc.EntityIds() )
	{
		if ( std::optional<EntityDraw> draw = DrawEntity( ctx, id ) )
		{
			snapshot.entities.push_back( std::move( *draw ) );
		}
	}
	snapshot.bounds = BoundsOf( snapshot );
	return snapshot;
}

SnapshotCache::SnapshotCache( ExtractOptions options ) : m_options( std::move( options ) )
{
}

void SnapshotCache::Rebuild(
    const scene::DocumentReader &doc, const SelectionInput &selection, std::uint64_t revision )
{
	m_snapshot = Extract( doc, selection, m_options );
	m_selected = SortedUnique( selection.objects );
	m_selectedFaces = SortedUnique( selection.faces );
	m_revision = revision;
	m_built = true;
	m_lastRebuilt = m_snapshot.solids.size() + m_snapshot.entities.size();
}

SnapshotCache::UpdateKind SnapshotCache::Update( const scene::DocumentReader &doc,
    const scene::ChangeSet &changes, const SelectionInput &selection, std::uint64_t baseRevision,
    std::uint64_t revision )
{
	if ( !m_built || baseRevision != m_revision )
	{
		Rebuild( doc, selection, revision );
		return UpdateKind::Full;
	}

	std::vector<ObjectId> dirty;
	const auto noteValue = [&]( const scene::MapObject &value )
	{
		if ( const scene::Solid *solid = std::get_if<scene::Solid>( &value ) )
		{
			if ( solid->owner.IsValid() )
			{
				dirty.push_back( solid->owner );
			}
		}
	};
	for ( const scene::ObjectChange &change : changes.objects )
	{
		AppendLeaves( doc, change.id, dirty );
		if ( change.before )
		{
			noteValue( *change.before );
		}
		if ( change.after )
		{
			noteValue( *change.after );
		}
	}

	std::vector<ObjectId> selected = SortedUnique( selection.objects );
	std::vector<scene::FaceRef> faces = SortedUnique( selection.faces );
	for ( ObjectId id : SymmetricDifference( m_selected, selected ) )
	{
		AppendLeaves( doc, id, dirty );
	}
	for ( const scene::FaceRef &face : SymmetricDifference( m_selectedFaces, faces ) )
	{
		dirty.push_back( face.solid );
	}
	m_selected = std::move( selected );
	m_selectedFaces = std::move( faces );

	RefreshObjects( doc, std::move( dirty ) );
	m_revision = revision;
	return UpdateKind::Incremental;
}

void SnapshotCache::SetSelection(
    const scene::DocumentReader &doc, const SelectionInput &selection )
{
	if ( !m_built )
	{
		Rebuild( doc, selection, m_revision );
		return;
	}
	scene::ChangeSet none;
	Update( doc, none, selection, m_revision, m_revision );
}

void SnapshotCache::RefreshObjects( const scene::DocumentReader &doc, std::vector<ObjectId> ids )
{
	ids = SortedUnique( std::move( ids ) );
	const Context ctx{ doc, m_options, m_selected, m_selectedFaces, BrushEntities( doc ) };
	for ( ObjectId id : ids )
	{
		const std::optional<scene::ObjectKind> kind = doc.KindOf( id );
		Place( m_snapshot.solids, id,
		    kind == scene::ObjectKind::Solid ? DrawSolid( ctx, id ) : std::nullopt );
		Place( m_snapshot.entities, id,
		    kind == scene::ObjectKind::Entity ? DrawEntity( ctx, id ) : std::nullopt );
	}
	m_lastRebuilt = ids.size();
	RecomputeBounds();
}

void SnapshotCache::RecomputeBounds()
{
	m_snapshot.bounds = BoundsOf( m_snapshot );
}

std::vector<ScreenSegment> ProjectEdges2D( const SolidDraw &solid, const Camera2D &camera )
{
	constexpr double kPixelEpsilon = 1.0e-6;
	std::vector<WorldEdge> edges;
	for ( const FaceDraw &face : solid.faces )
	{
		AppendUniqueEdges( edges, face.vertices );
	}
	const auto same = []( ScreenPoint p, ScreenPoint q )
	{
		return std::fabs( p.x - q.x ) <= kPixelEpsilon && std::fabs( p.y - q.y ) <= kPixelEpsilon;
	};

	std::vector<ScreenSegment> segments;
	segments.reserve( edges.size() );
	for ( const WorldEdge &edge : edges )
	{
		const ScreenSegment segment{
		    camera.WorldToScreen( edge.a ), camera.WorldToScreen( edge.b ) };
		if ( same( segment.a, segment.b ) )
		{
			continue;
		}
		const bool repeated = std::any_of( segments.begin(), segments.end(),
		    [&]( const ScreenSegment &known )
		    {
			    return ( same( known.a, segment.a ) && same( known.b, segment.b ) ) ||
			           ( same( known.a, segment.b ) && same( known.b, segment.a ) );
		    } );
		if ( !repeated )
		{
			segments.push_back( segment );
		}
	}
	return segments;
}

} // namespace hammer::viewport
