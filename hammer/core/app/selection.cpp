//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/selection.h.
//
//=============================================================================//

#include "hammer/app/selection.h"

#include "hammer/scene/map_queries.h"

#include <algorithm>
#include <set>

namespace hammer::app
{

using scene::FaceRef;
using scene::ObjectId;

bool Selection::Contains( ObjectId id ) const
{
	return std::binary_search( objects.begin(), objects.end(), id );
}

bool Selection::ContainsFace( const FaceRef &face ) const
{
	return std::binary_search( faces.begin(), faces.end(), face );
}

ObjectId ResolvePick(
    const scene::DocumentReader &doc, ObjectId hit, SelectionGranularity granularity )
{
	if ( !doc.KindOf( hit ) )
	{
		return ObjectId();
	}
	switch ( granularity )
	{
	case SelectionGranularity::Solids:
		return hit;
	case SelectionGranularity::Objects:
		if ( const scene::Solid *s = doc.FindSolid( hit ) )
		{
			if ( s->owner.IsValid() && doc.FindEntity( s->owner ) )
			{
				return s->owner;
			}
		}
		return hit;
	case SelectionGranularity::Groups:
		return scene::TopLevelOf( doc, hit );
	}
	return hit;
}

namespace
{

template <typename T> void Normalize( std::vector<T> &v )
{
	std::sort( v.begin(), v.end() );
	v.erase( std::unique( v.begin(), v.end() ), v.end() );
}

template <typename T>
std::vector<T> Combine( const std::vector<T> &current, const std::vector<T> &items, SelectMode mode,
    T &lastAdded, bool &added )
{
	std::set<T> out;
	if ( mode != SelectMode::Replace )
	{
		out.insert( current.begin(), current.end() );
	}
	added = false;
	for ( const T &item : items )
	{
		switch ( mode )
		{
		case SelectMode::Replace:
		case SelectMode::Add:
			out.insert( item );
			lastAdded = item;
			added = true;
			break;
		case SelectMode::Toggle:
			if ( out.erase( item ) == 0 )
			{
				out.insert( item );
				lastAdded = item;
				added = true;
			}
			break;
		case SelectMode::Remove:
			out.erase( item );
			break;
		}
	}
	return std::vector<T>( out.begin(), out.end() );
}

} // namespace

Selection CombineObjects(
    const Selection &current, const std::vector<ObjectId> &ids, SelectMode mode )
{
	Selection out = current;
	ObjectId last;
	bool added = false;
	std::vector<ObjectId> items = ids;
	std::erase_if( items,
	    []( ObjectId id )
	    {
		    return !id.IsValid();
	    } );
	out.objects = Combine( current.objects, items, mode, last, added );
	if ( added )
	{
		out.primary = last;
	}
	if ( !out.Contains( out.primary ) )
	{
		out.primary = out.objects.empty() ? ObjectId() : out.objects.back();
	}
	return out;
}

Selection CombineFaces(
    const Selection &current, const std::vector<FaceRef> &faces, SelectMode mode )
{
	Selection out = current;
	FaceRef last;
	bool added = false;
	out.faces = Combine( current.faces, faces, mode, last, added );
	return out;
}

Selection Prune( const scene::DocumentReader &doc, const Selection &selection )
{
	Selection out;
	for ( ObjectId id : selection.objects )
	{
		if ( doc.KindOf( id ) )
		{
			out.objects.push_back( id );
		}
	}
	for ( const FaceRef &f : selection.faces )
	{
		const scene::Solid *s = doc.FindSolid( f.solid );
		if ( s && s->FindSide( f.side ) )
		{
			out.faces.push_back( f );
		}
	}
	out.primary = out.Contains( selection.primary )
	                  ? selection.primary
	                  : ( out.objects.empty() ? ObjectId() : out.objects.back() );
	return out;
}

namespace
{

std::vector<ObjectId> Selectables(
    const scene::DocumentReader &doc, SelectionGranularity granularity, bool includeHidden )
{
	std::vector<ObjectId> out;
	auto consider = [&]( ObjectId id )
	{
		if ( !includeHidden && !scene::IsVisible( doc, id ) )
		{
			return;
		}
		out.push_back( ResolvePick( doc, id, granularity ) );
	};
	for ( ObjectId id : doc.SolidIds() )
	{
		consider( id );
	}
	for ( ObjectId id : doc.EntityIds() )
	{
		consider( id );
	}
	Normalize( out );
	return out;
}

} // namespace

Selection SelectAll(
    const scene::DocumentReader &doc, SelectionGranularity granularity, bool includeHidden )
{
	Selection out;
	out.objects = Selectables( doc, granularity, includeHidden );
	out.primary = out.objects.empty() ? ObjectId() : out.objects.back();
	return out;
}

Selection InvertSelection(
    const scene::DocumentReader &doc, const Selection &current, SelectionGranularity granularity )
{
	Selection out;
	for ( ObjectId id : Selectables( doc, granularity, false ) )
	{
		if ( !current.Contains( id ) )
		{
			out.objects.push_back( id );
		}
	}
	out.primary = out.objects.empty() ? ObjectId() : out.objects.back();
	return out;
}

} // namespace hammer::app
