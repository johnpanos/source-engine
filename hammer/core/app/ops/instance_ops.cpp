//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/instance_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/instance_ops.h"

#include "hammer/app/instance_fixup.h"
#include "hammer/app/ops/prefab_ops.h"
#include "hammer/scene/map_queries.h"
#include "mapgeometry/transform.h"

#include <algorithm>
#include <cctype>

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

bool IsInstance( const scene::Entity &e )
{
	return Lower( e.classname ) == "func_instance";
}

// Keys that name entities (and so take the fixup) when no catalog types them.
bool ConventionalNameKey( std::string_view key )
{
	static const char *const kKeys[] = { "targetname", "target", "parentname", "filtername",
		"damagefilter", "lightingorigin" };
	const std::string lower = Lower( key );
	return std::any_of( std::begin( kKeys ), std::end( kKeys ),
	    [&]( const char *k )
	    {
		    return lower == k;
	    } );
}

bool NamesEntity(
    const ports::IEntityCatalog *catalog, const std::string &classname, const std::string &key )
{
	const ports::EntityClassInfo *info = catalog ? catalog->Find( classname ) : nullptr;
	if ( !info )
	{
		return ConventionalNameKey( key );
	}
	const ports::KeyDefinition *def = info->FindKey( key );
	return def && ( def->type == ports::KeyType::TargetSource ||
	                  def->type == ports::KeyType::TargetDestination );
}

// The fixup name for an unnamed instance: InstanceAuto<n> with the smallest
// n that no target entity name mentions.
std::string AutoFixupName( const scene::DocumentReader &doc )
{
	std::vector<std::string> names;
	for ( ObjectId id : doc.EntityIds() )
	{
		names.push_back( Lower( doc.FindEntity( id )->Name() ) );
	}
	for ( int n = 1;; ++n )
	{
		const std::string candidate = "instanceauto" + std::to_string( n );
		if ( std::none_of( names.begin(), names.end(),
		         [&]( const std::string &name )
		         {
			         return name.find( candidate ) != std::string::npos;
		         } ) )
		{
			return "InstanceAuto" + std::to_string( n );
		}
	}
}

// Steps 1 and 2 of the collapse rule on one content entity.
void FixupEntity( scene::Entity &e, const std::vector<InstanceParameter> &parameters,
    const std::string &fixup, InstanceFixupStyle style, const ports::IEntityCatalog *catalog )
{
	e.classname = SubstituteInstanceParameters( e.classname, parameters );
	for ( kvtext::KeyValue &kv : e.keys )
	{
		kv.value = SubstituteInstanceParameters( kv.value, parameters );
	}
	for ( scene::Connection &c : e.connections )
	{
		c.target = SubstituteInstanceParameters( c.target, parameters );
		c.input = SubstituteInstanceParameters( c.input, parameters );
		c.parameter = SubstituteInstanceParameters( c.parameter, parameters );
	}
	if ( style == InstanceFixupStyle::None )
	{
		return;
	}
	for ( kvtext::KeyValue &kv : e.keys )
	{
		if ( NamesEntity( catalog, e.classname, kv.key ) )
		{
			kv.value = FixupInstanceName( kv.value, fixup, style );
		}
	}
	for ( scene::Connection &c : e.connections )
	{
		c.target = FixupInstanceName( c.target, fixup, style );
	}
}

} // namespace

std::vector<ObjectId> FindInstances( const scene::DocumentReader &doc )
{
	std::vector<ObjectId> out;
	for ( ObjectId id : doc.EntityIds() )
	{
		if ( IsInstance( *doc.FindEntity( id ) ) )
		{
			out.push_back( id );
		}
	}
	return out;
}

std::string InstanceFile( const scene::Entity &entity )
{
	const std::string *file = entity.Key( "file" );
	if ( !IsInstance( entity ) || !file || file->empty() )
	{
		return {};
	}
	std::string out = *file;
	std::replace( out.begin(), out.end(), '\\', '/' );
	const std::size_t slash = out.rfind( '/' );
	const std::size_t dot = out.rfind( '.' );
	if ( dot == std::string::npos || ( slash != std::string::npos && dot < slash ) )
	{
		out += ".vmf";
	}
	return out;
}

EditResult CollapseInstance( scene::DocumentEdit &edit, ObjectId instanceEntity,
    const MapFragment &instanceContent, std::vector<ObjectId> *created,
    const ports::IEntityCatalog *catalog )
{
	const scene::Entity *instance = edit.FindEntity( instanceEntity );
	if ( !instance || !IsInstance( *instance ) )
	{
		return Reject( "not a func_instance" );
	}
	if ( !scene::EntitySolids( edit, instanceEntity ).empty() )
	{
		return Reject( "a func_instance cannot own solids" );
	}
	const std::optional<Vec3d> origin = instance->Origin();
	if ( instance->Key( "origin" ) && !origin )
	{
		return Reject( "the instance origin is malformed" );
	}
	const std::optional<Vec3d> angles = instance->Angles();
	if ( instance->Key( "angles" ) && !angles )
	{
		return Reject( "the instance angles are malformed" );
	}
	const std::string *styleText = instance->Key( "fixup_style" );
	const std::optional<InstanceFixupStyle> style =
	    ParseFixupStyle( styleText ? *styleText : std::string() );
	if ( !style )
	{
		return Reject( "unknown fixup_style" );
	}
	std::string fixup( instance->Name() );
	if ( fixup.empty() )
	{
		const std::string *name = instance->Key( "name" );
		fixup = name && !name->empty() ? *name : AutoFixupName( edit );
	}
	const std::vector<InstanceParameter> parameters = InstanceParameters( instance->keys );
	const ObjectId instanceGroup = instance->group;

	std::vector<ObjectId> merged;
	if ( !instanceContent.Empty() )
	{
		MapFragment content = instanceContent;
		for ( scene::MapObject &o : content.objects )
		{
			if ( scene::Entity *e = std::get_if<scene::Entity>( &o ) )
			{
				FixupEntity( *e, parameters, fixup, *style, catalog );
			}
		}
		const Vec3d a = angles.value_or( Vec3d() );
		const mapgeometry::Affine xf =
		    mapgeometry::Compose( mapgeometry::Affine::Translation( origin.value_or( Vec3d() ) ),
		        mapgeometry::Affine::About( mapgeometry::AngleMatrix( a.x, a.y, a.z ), Vec3d() ) );
		std::optional<MapFragment> placed = TransformedFragment( content, xf );
		if ( !placed )
		{
			return Reject( "the instance transform would make a solid degenerate" );
		}
		if ( EditResult pasted = Paste( edit, *placed, PasteOptions{}, &merged ); !pasted )
		{
			return pasted;
		}
		if ( instanceGroup.IsValid() )
		{
			for ( ObjectId id : merged )
			{
				if ( scene::Solid *s = edit.MutableSolid( id ) )
				{
					s->group = instanceGroup;
				}
				else if ( scene::Entity *e = edit.MutableEntity( id ) )
				{
					e->group = instanceGroup;
				}
				else if ( scene::Group *g = edit.MutableGroup( id ) )
				{
					g->group = instanceGroup;
				}
			}
		}
	}
	edit.Remove( instanceEntity );
	if ( created )
	{
		created->insert( created->end(), merged.begin(), merged.end() );
	}
	return {};
}

} // namespace hammer::app::ops
