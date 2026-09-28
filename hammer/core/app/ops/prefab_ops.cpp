//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/prefab_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/prefab_ops.h"

#include "hammer/app/ops/decal_ops.h"
#include "hammer/app/ops/transform_ops.h"
#include "mapgeometry/vec3.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <set>

namespace hammer::app::ops
{

using mapgeometry::Vec3d;
using scene::ObjectId;

namespace
{

std::string Lower( std::string_view text )
{
	std::string out( text );
	for ( char &c : out )
	{
		c = static_cast<char>( std::tolower( static_cast<unsigned char>( c ) ) );
	}
	return out;
}

bool EqualsNoCase( std::string_view a, std::string_view b )
{
	return a.size() == b.size() && Lower( a ) == Lower( b );
}

bool Finite( const Vec3d &v )
{
	return std::isfinite( v.x ) && std::isfinite( v.y ) && std::isfinite( v.z );
}

// The fragment's bounds: solids' vertices and, for entities that own no solid
// of the fragment, their origin +/- 8 (scene::ObjectBounds' rule).
std::optional<scene::Box> BoundsOf( const std::vector<scene::MapObject> &objects )
{
	std::set<ObjectId> owners;
	for ( const scene::MapObject &o : objects )
	{
		if ( const scene::Solid *s = std::get_if<scene::Solid>( &o ) )
		{
			if ( s->owner.IsValid() )
			{
				owners.insert( s->owner );
			}
		}
	}
	std::optional<scene::Box> bounds;
	auto extend = [&]( const scene::Box &box )
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
	for ( const scene::MapObject &o : objects )
	{
		if ( const scene::Solid *s = std::get_if<scene::Solid>( &o ) )
		{
			if ( const std::optional<scene::Box> b = scene::SolidBounds( *s ) )
			{
				extend( *b );
			}
		}
		else if ( const scene::Entity *e = std::get_if<scene::Entity>( &o ) )
		{
			const std::optional<Vec3d> origin = e->Origin();
			if ( origin && !owners.count( e->id ) )
			{
				extend( scene::Box{ *origin - Vec3d( 8, 8, 8 ), *origin + Vec3d( 8, 8, 8 ) } );
			}
		}
	}
	return bounds;
}

// Rewrites key values and connection targets equal to 'from' (legacy
// CMapEntity::ReplaceTargetname).
void ReplaceName( scene::Entity &e, const std::string &from, const std::string &to )
{
	for ( kvtext::KeyValue &kv : e.keys )
	{
		if ( EqualsNoCase( kv.value, from ) )
		{
			kv.value = to;
		}
	}
	for ( scene::Connection &c : e.connections )
	{
		if ( EqualsNoCase( c.target, from ) )
		{
			c.target = to;
		}
	}
}

std::size_t TopLevelCount( const MapFragment &fragment )
{
	std::set<ObjectId> local;
	for ( const scene::MapObject &o : fragment.objects )
	{
		local.insert( scene::IdOf( o ) );
	}
	std::size_t count = 0;
	for ( const scene::MapObject &o : fragment.objects )
	{
		const ObjectId group = std::visit(
		    []( const auto &v )
		    {
			    return v.group;
		    },
		    o );
		const scene::Solid *s = std::get_if<scene::Solid>( &o );
		const bool owned = s && s->owner.IsValid() && local.count( s->owner );
		if ( !owned && !local.count( group ) )
		{
			++count;
		}
	}
	return count;
}

} // namespace

std::optional<MapFragment> FragmentFromDocument( const scene::MapDocument &doc )
{
	std::vector<ObjectId> ids = doc.SolidIds();
	for ( ObjectId id : doc.EntityIds() )
	{
		ids.push_back( id );
	}
	for ( ObjectId id : doc.GroupIds() )
	{
		ids.push_back( id );
	}
	MapFragment fragment = Copy( doc, ids );
	if ( fragment.Empty() )
	{
		return std::nullopt;
	}
	return fragment;
}

std::optional<MapFragment> TransformedFragment( const MapFragment &fragment, const mapgeometry::Affine &xf )
{
	MapFragment out;
	out.visgroups = fragment.visgroups;
	for ( const scene::MapObject &o : fragment.objects )
	{
		if ( const scene::Solid *s = std::get_if<scene::Solid>( &o ) )
		{
			std::optional<scene::Solid> moved = TransformedSolid( *s, xf, TransformOptions{ true } );
			if ( !moved )
			{
				return std::nullopt;
			}
			out.objects.emplace_back( std::move( *moved ) );
		}
		else if ( const scene::Entity *e = std::get_if<scene::Entity>( &o ) )
		{
			out.objects.emplace_back( TransformedOverlay( TransformedEntity( *e, xf ), xf ) );
		}
		else
		{
			out.objects.push_back( o );
		}
	}
	out.bounds = BoundsOf( out.objects );
	return out;
}

std::string ExpandNameKeyword( const std::string &name, const scene::DocumentReader &target )
{
	const std::size_t at = name.find( "&i" );
	if ( at == std::string::npos )
	{
		return name;
	}
	const std::string prefix = name.substr( 0, at );
	const std::string suffix = name.substr( at + 2 );
	long long highest = 0;
	for ( ObjectId id : target.EntityIds() )
	{
		const std::string_view other = target.FindEntity( id )->Name();
		if ( other.size() <= prefix.size() + suffix.size() ||
		     !EqualsNoCase( other.substr( 0, prefix.size() ), prefix ) ||
		     !EqualsNoCase( other.substr( other.size() - suffix.size() ), suffix ) )
		{
			continue;
		}
		const std::string_view digits =
		    other.substr( prefix.size(), other.size() - prefix.size() - suffix.size() );
		if ( !std::all_of( digits.begin(), digits.end(),
		         []( char c )
		         {
			         return std::isdigit( static_cast<unsigned char>( c ) );
		         } ) )
		{
			continue;
		}
		long long value = 0;
		for ( char c : digits )
		{
			value = value < 100000000000000LL ? value * 10 + ( c - '0' ) : value;
		}
		highest = std::max( highest, value );
	}
	return prefix + std::to_string( highest + 1 ) + suffix;
}

EditResult InsertPrefab( scene::DocumentEdit &edit, const MapFragment &prefab, const Vec3d &at,
    const Vec3d &anglesPYR, bool group, std::vector<ObjectId> *created, PrefabAnchor anchor )
{
	if ( prefab.Empty() )
	{
		return NothingToDo( "the prefab is empty" );
	}
	if ( !Finite( at ) || !Finite( anglesPYR ) )
	{
		return Reject( "the prefab position and angles must be finite" );
	}
	Vec3d pivot;
	if ( anchor == PrefabAnchor::BoundsCenter )
	{
		const std::optional<scene::Box> bounds = BoundsOf( prefab.objects );
		if ( !bounds )
		{
			return Reject( "the prefab has no extent to center" );
		}
		pivot = bounds->Center();
	}
	const mapgeometry::Affine xf = mapgeometry::Compose( mapgeometry::Affine::Translation( at - pivot ),
	    mapgeometry::Affine::About(
	        mapgeometry::AngleMatrix( anglesPYR.x, anglesPYR.y, anglesPYR.z ), pivot ) );
	std::optional<MapFragment> placed = TransformedFragment( prefab, xf );
	if ( !placed )
	{
		return Reject( "the prefab rotation would make a solid degenerate" );
	}

	// Name keywords, against the target as it is before the insertion.
	std::vector<std::pair<std::string, std::string>> renames;
	for ( const scene::MapObject &o : placed->objects )
	{
		if ( const scene::Entity *e = std::get_if<scene::Entity>( &o ) )
		{
			const std::string name( e->Name() );
			const std::string expanded = ExpandNameKeyword( name, edit );
			if ( expanded != name && std::none_of( renames.begin(), renames.end(),
			                             [&]( const auto &r )
			                             {
				                             return r.first == name;
			                             } ) )
			{
				renames.emplace_back( name, expanded );
			}
		}
	}
	for ( scene::MapObject &o : placed->objects )
	{
		if ( scene::Entity *e = std::get_if<scene::Entity>( &o ) )
		{
			for ( const auto &[from, to] : renames )
			{
				ReplaceName( *e, from, to );
			}
		}
	}

	PasteOptions options;
	options.group = group && TopLevelCount( *placed ) > 1;
	return Paste( edit, *placed, options, created );
}

} // namespace hammer::app::ops
