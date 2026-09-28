//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/scene/map_queries.h.
//
//=============================================================================//

#include "hammer/scene/map_queries.h"

#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cctype>
#include <set>

namespace hammer::scene
{

using mapgeometry::Vec3d;

std::vector<ObjectId> GroupMembers( const DocumentReader &doc, ObjectId group )
{
	std::vector<ObjectId> out;
	for ( ObjectId id : doc.SolidIds() )
	{
		const Solid *s = doc.FindSolid( id );
		if ( s->group == group && !s->owner.IsValid() )
		{
			out.push_back( id );
		}
	}
	for ( ObjectId id : doc.EntityIds() )
	{
		if ( doc.FindEntity( id )->group == group )
		{
			out.push_back( id );
		}
	}
	for ( ObjectId id : doc.GroupIds() )
	{
		if ( id != group && doc.FindGroup( id )->group == group )
		{
			out.push_back( id );
		}
	}
	std::sort( out.begin(), out.end() );
	return out;
}

std::vector<ObjectId> EntitySolids( const DocumentReader &doc, ObjectId entity )
{
	std::vector<ObjectId> out;
	for ( ObjectId id : doc.SolidIds() )
	{
		if ( doc.FindSolid( id )->owner == entity )
		{
			out.push_back( id );
		}
	}
	return out;
}

std::vector<ObjectId> ExpandObjects( const DocumentReader &doc, const std::vector<ObjectId> &ids )
{
	std::set<ObjectId> seen;
	std::vector<ObjectId> stack( ids.rbegin(), ids.rend() );
	while ( !stack.empty() )
	{
		const ObjectId id = stack.back();
		stack.pop_back();
		const std::optional<ObjectKind> kind = doc.KindOf( id );
		if ( !kind || !seen.insert( id ).second )
		{
			continue;
		}
		if ( *kind == ObjectKind::Group )
		{
			for ( ObjectId m : GroupMembers( doc, id ) )
			{
				stack.push_back( m );
			}
		}
		else if ( *kind == ObjectKind::Entity )
		{
			for ( ObjectId s : EntitySolids( doc, id ) )
			{
				stack.push_back( s );
			}
		}
	}
	return std::vector<ObjectId>( seen.begin(), seen.end() );
}

std::vector<ObjectId> ExpandToLeaves( const DocumentReader &doc, const std::vector<ObjectId> &ids )
{
	std::vector<ObjectId> out;
	for ( ObjectId id : ExpandObjects( doc, ids ) )
	{
		if ( doc.KindOf( id ) != ObjectKind::Group )
		{
			out.push_back( id );
		}
	}
	return out;
}

ObjectId ContainerOf( const DocumentReader &doc, ObjectId id )
{
	if ( const Solid *s = doc.FindSolid( id ) )
	{
		return s->owner.IsValid() ? s->owner : s->group;
	}
	if ( const Entity *e = doc.FindEntity( id ) )
	{
		return e->group;
	}
	if ( const Group *g = doc.FindGroup( id ) )
	{
		return g->group;
	}
	return ObjectId();
}

ObjectId TopLevelOf( const DocumentReader &doc, ObjectId id )
{
	ObjectId at = id;
	// Bounded walk: a malformed cycle cannot loop forever.
	for ( int guard = 0; guard < 4096; ++guard )
	{
		const ObjectId up = ContainerOf( doc, at );
		if ( !up.IsValid() || !doc.KindOf( up ) )
		{
			return at;
		}
		at = up;
	}
	return at;
}

namespace
{

bool SelfVisible( const DocumentReader &doc, ObjectId id )
{
	if ( const Solid *s = doc.FindSolid( id ) )
	{
		return !s->hidden && s->editor.visgroupShown;
	}
	if ( const Entity *e = doc.FindEntity( id ) )
	{
		return !e->hidden && e->editor.visgroupShown;
	}
	if ( const Group *g = doc.FindGroup( id ) )
	{
		return !g->hidden && g->editor.visgroupShown;
	}
	return false;
}

} // namespace

bool IsVisible( const DocumentReader &doc, ObjectId id )
{
	ObjectId at = id;
	for ( int guard = 0; guard < 4096 && at.IsValid(); ++guard )
	{
		if ( !SelfVisible( doc, at ) )
		{
			return false;
		}
		at = ContainerOf( doc, at );
	}
	return true;
}

std::optional<Box> ObjectBounds( const DocumentReader &doc, ObjectId id, double pointHalfSize )
{
	const std::optional<ObjectKind> kind = doc.KindOf( id );
	if ( !kind )
	{
		return std::nullopt;
	}
	if ( *kind == ObjectKind::Solid )
	{
		return SolidBounds( *doc.FindSolid( id ) );
	}
	if ( *kind == ObjectKind::Entity )
	{
		const std::vector<ObjectId> solids = EntitySolids( doc, id );
		if ( solids.empty() )
		{
			const std::optional<Vec3d> origin = doc.FindEntity( id )->Origin();
			if ( !origin )
			{
				return std::nullopt;
			}
			const Vec3d half( pointHalfSize, pointHalfSize, pointHalfSize );
			return Box{ *origin - half, *origin + half };
		}
		return ObjectsBounds( doc, solids, pointHalfSize );
	}
	return ObjectsBounds( doc, GroupMembers( doc, id ), pointHalfSize );
}

std::optional<Box> ObjectsBounds(
    const DocumentReader &doc, const std::vector<ObjectId> &ids, double pointHalfSize )
{
	std::optional<Box> out;
	for ( ObjectId id : ids )
	{
		const std::optional<Box> b = ObjectBounds( doc, id, pointHalfSize );
		if ( !b )
		{
			continue;
		}
		if ( out )
		{
			out->Extend( *b );
		}
		else
		{
			out = b;
		}
	}
	return out;
}

bool NameMatches( std::string_view pattern, std::string_view name )
{
	auto lower = []( char c ) { return static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) ); };
	if ( !pattern.empty() && pattern.back() == '*' )
	{
		const std::string_view prefix = pattern.substr( 0, pattern.size() - 1 );
		if ( name.size() < prefix.size() )
		{
			return false;
		}
		for ( std::size_t i = 0; i < prefix.size(); ++i )
		{
			if ( lower( prefix[i] ) != lower( name[i] ) )
			{
				return false;
			}
		}
		return true;
	}
	if ( pattern.size() != name.size() )
	{
		return false;
	}
	for ( std::size_t i = 0; i < name.size(); ++i )
	{
		if ( lower( pattern[i] ) != lower( name[i] ) )
		{
			return false;
		}
	}
	return true;
}

std::vector<ObjectId> FindEntitiesByName( const DocumentReader &doc, std::string_view pattern )
{
	std::vector<ObjectId> out;
	if ( pattern.empty() )
	{
		return out;
	}
	for ( ObjectId id : doc.EntityIds() )
	{
		const std::string_view name = doc.FindEntity( id )->Name();
		if ( !name.empty() && NameMatches( pattern, name ) )
		{
			out.push_back( id );
		}
	}
	return out;
}

std::vector<ObjectId> FindEntitiesByClass( const DocumentReader &doc, std::string_view classname )
{
	std::vector<ObjectId> out;
	for ( ObjectId id : doc.EntityIds() )
	{
		if ( NameMatches( classname, doc.FindEntity( id )->classname ) )
		{
			out.push_back( id );
		}
	}
	return out;
}

std::optional<FaceRef> FindSideById( const DocumentReader &doc, std::uint32_t sideVmfId )
{
	for ( ObjectId id : doc.SolidIds() )
	{
		if ( doc.FindSolid( id )->FindSide( sideVmfId ) )
		{
			return FaceRef{ id, sideVmfId };
		}
	}
	return std::nullopt;
}

} // namespace hammer::scene
