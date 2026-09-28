//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/entity_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/entity_ops.h"

#include "hammer/app/ops/create_ops.h"
#include "hammer/scene/map_queries.h"

#include <cstdlib>
#include <set>

namespace hammer::app::ops
{

using scene::ObjectId;

namespace
{

bool Reserved( const std::string &key )
{
	return key.empty() || key == "classname" || key == "id";
}

// Entities to edit, or an error when there are none.
EditResult Targets( const scene::DocumentReader &doc, const std::vector<ObjectId> &ids, std::vector<ObjectId> &out )
{
	out = EntitiesOf( doc, ids );
	if ( out.empty() )
	{
		return NothingToDo( "no entities selected" );
	}
	return {};
}

} // namespace

std::vector<ObjectId> EntitiesOf( const scene::DocumentReader &doc, const std::vector<ObjectId> &ids )
{
	std::set<ObjectId> out;
	for ( ObjectId id : scene::ExpandObjects( doc, ids ) )
	{
		if ( doc.FindEntity( id ) )
		{
			out.insert( id );
		}
		else if ( const scene::Solid *s = doc.FindSolid( id ) )
		{
			if ( s->owner.IsValid() && doc.FindEntity( s->owner ) )
			{
				out.insert( s->owner );
			}
		}
	}
	return std::vector<ObjectId>( out.begin(), out.end() );
}

EditResult SetKey( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids, const std::string &key,
    const std::string &value )
{
	if ( Reserved( key ) )
	{
		return Reject( "'" + key + "' cannot be set as a key" );
	}
	std::vector<ObjectId> entities;
	if ( EditResult r = Targets( edit, ids, entities ); !r )
	{
		return r;
	}
	bool changed = false;
	for ( ObjectId id : entities )
	{
		const std::string *current = edit.FindEntity( id )->Key( key );
		if ( current && *current == value )
		{
			continue;
		}
		edit.MutableEntity( id )->SetKey( key, value );
		changed = true;
	}
	if ( !changed )
	{
		return NothingToDo( "every entity already has that value" );
	}
	return {};
}

EditResult RemoveKey( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids, const std::string &key )
{
	if ( Reserved( key ) )
	{
		return Reject( "'" + key + "' cannot be removed" );
	}
	std::vector<ObjectId> entities;
	if ( EditResult r = Targets( edit, ids, entities ); !r )
	{
		return r;
	}
	bool changed = false;
	for ( ObjectId id : entities )
	{
		if ( edit.FindEntity( id )->Key( key ) )
		{
			edit.MutableEntity( id )->RemoveKey( key );
			changed = true;
		}
	}
	if ( !changed )
	{
		return NothingToDo( "no entity has '" + key + "'" );
	}
	return {};
}

EditResult RenameKey( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids, const std::string &from,
    const std::string &to )
{
	if ( Reserved( from ) || Reserved( to ) )
	{
		return Reject( "reserved key" );
	}
	if ( from == to )
	{
		return NothingToDo( "same name" );
	}
	std::vector<ObjectId> entities;
	if ( EditResult r = Targets( edit, ids, entities ); !r )
	{
		return r;
	}
	for ( ObjectId id : entities )
	{
		const scene::Entity *e = edit.FindEntity( id );
		if ( e->Key( from ) && e->Key( to ) )
		{
			return Reject( "an entity already has '" + to + "'" );
		}
	}
	bool changed = false;
	for ( ObjectId id : entities )
	{
		if ( !edit.FindEntity( id )->Key( from ) )
		{
			continue;
		}
		for ( kvtext::KeyValue &kv : edit.MutableEntity( id )->keys )
		{
			if ( kv.key == from )
			{
				kv.key = to;
			}
		}
		changed = true;
	}
	if ( !changed )
	{
		return NothingToDo( "no entity has '" + from + "'" );
	}
	return {};
}

EditResult SetClass( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids, const std::string &classname,
    const ports::IEntityCatalog *catalog )
{
	if ( classname.empty() )
	{
		return Reject( "no entity class" );
	}
	std::vector<ObjectId> entities;
	if ( EditResult r = Targets( edit, ids, entities ); !r )
	{
		return r;
	}
	const ports::EntityClassInfo *info = catalog ? catalog->Find( classname ) : nullptr;
	if ( catalog && !info )
	{
		return Reject( "unknown entity class '" + classname + "'" );
	}
	for ( ObjectId id : entities )
	{
		const bool brush = !scene::EntitySolids( edit, id ).empty();
		if ( info && brush != ( info->kind == ports::EntityClassKind::Solid ) )
		{
			return Reject( std::string( brush ? "a brush entity needs a solid class"
			                                  : "a point entity cannot take a solid class" ) );
		}
	}
	bool changed = false;
	for ( ObjectId id : entities )
	{
		scene::Entity *e = edit.MutableEntity( id );
		const std::string name = info ? info->name : classname;
		if ( e->classname != name )
		{
			e->classname = name;
			changed = true;
		}
		if ( info )
		{
			const std::size_t before = e->keys.size();
			ApplyClassDefaults( *e, *info );
			changed = changed || e->keys.size() != before;
		}
	}
	if ( !changed )
	{
		return NothingToDo( "the entities already have that class" );
	}
	return {};
}

EditResult SetSpawnFlag( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids, long long flag, bool on )
{
	if ( flag <= 0 || ( flag & ( flag - 1 ) ) != 0 )
	{
		return Reject( "a spawnflag is a single bit" );
	}
	std::vector<ObjectId> entities;
	if ( EditResult r = Targets( edit, ids, entities ); !r )
	{
		return r;
	}
	bool changed = false;
	for ( ObjectId id : entities )
	{
		const std::string *current = edit.FindEntity( id )->Key( "spawnflags" );
		const long long value = current ? std::atoll( current->c_str() ) : 0;
		const long long next = on ? ( value | flag ) : ( value & ~flag );
		if ( next != value || !current )
		{
			edit.MutableEntity( id )->SetKey( "spawnflags", std::to_string( next ) );
			changed = true;
		}
	}
	if ( !changed )
	{
		return NothingToDo( "the flag is already set that way" );
	}
	return {};
}

EditResult AddConnection( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids, const scene::Connection &c )
{
	if ( c.output.empty() || c.target.empty() || c.input.empty() )
	{
		return Reject( "a connection needs an output, a target and an input" );
	}
	if ( c.delay < 0.0 )
	{
		return Reject( "a connection delay cannot be negative" );
	}
	std::vector<ObjectId> entities;
	if ( EditResult r = Targets( edit, ids, entities ); !r )
	{
		return r;
	}
	for ( ObjectId id : entities )
	{
		edit.MutableEntity( id )->connections.push_back( c );
	}
	return {};
}

EditResult ReplaceConnection( scene::DocumentEdit &edit, ObjectId id, std::size_t index, const scene::Connection &c )
{
	const scene::Entity *e = edit.FindEntity( id );
	if ( !e || index >= e->connections.size() )
	{
		return Reject( "no such connection" );
	}
	if ( c.output.empty() || c.target.empty() || c.input.empty() || c.delay < 0.0 )
	{
		return Reject( "an incomplete connection" );
	}
	if ( e->connections[index] == c )
	{
		return NothingToDo( "the connection is unchanged" );
	}
	edit.MutableEntity( id )->connections[index] = c;
	return {};
}

EditResult RemoveConnections( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids,
    const std::function<bool( const scene::Connection & )> &match )
{
	std::vector<ObjectId> entities;
	if ( EditResult r = Targets( edit, ids, entities ); !r )
	{
		return r;
	}
	bool changed = false;
	for ( ObjectId id : entities )
	{
		bool any = false;
		for ( const scene::Connection &c : edit.FindEntity( id )->connections )
		{
			any = any || match( c );
		}
		if ( any )
		{
			std::erase_if( edit.MutableEntity( id )->connections, match );
			changed = true;
		}
	}
	if ( !changed )
	{
		return NothingToDo( "no connection matched" );
	}
	return {};
}

EditResult RenameEntity( scene::DocumentEdit &edit, ObjectId id, const std::string &name, bool updateReferences,
    const ports::IEntityCatalog *catalog )
{
	const scene::Entity *e = edit.FindEntity( id );
	if ( !e )
	{
		return Reject( "not an entity" );
	}
	const std::string old( e->Name() );
	if ( old == name )
	{
		return NothingToDo( "same name" );
	}
	if ( name.empty() )
	{
		edit.MutableEntity( id )->RemoveKey( "targetname" );
	}
	else
	{
		edit.MutableEntity( id )->SetKey( "targetname", name );
	}
	if ( !updateReferences || old.empty() || name.empty() )
	{
		return {};
	}
	static const char *const kConventional[] = { "target", "parentname", "filtername", "damagefilter",
		"lightingorigin" };
	for ( ObjectId other : edit.EntityIds() )
	{
		const scene::Entity *o = edit.FindEntity( other );
		const ports::EntityClassInfo *info = catalog ? catalog->Find( o->classname ) : nullptr;
		auto namesEntities = [&]( const std::string &key )
		{
			if ( info )
			{
				const ports::KeyDefinition *def = info->FindKey( key );
				return def && def->type == ports::KeyType::TargetDestination;
			}
			for ( const char *k : kConventional )
			{
				if ( key == k )
				{
					return true;
				}
			}
			return false;
		};
		bool touches = false;
		for ( const scene::Connection &c : o->connections )
		{
			touches = touches || c.target == old;
		}
		for ( const kvtext::KeyValue &kv : o->keys )
		{
			touches = touches || ( kv.value == old && namesEntities( kv.key ) );
		}
		if ( !touches )
		{
			continue;
		}
		scene::Entity *m = edit.MutableEntity( other );
		for ( scene::Connection &c : m->connections )
		{
			if ( c.target == old )
			{
				c.target = name;
			}
		}
		for ( kvtext::KeyValue &kv : m->keys )
		{
			if ( kv.value == old && namesEntities( kv.key ) )
			{
				kv.value = name;
			}
		}
	}
	return {};
}

EditResult SetWorldKey( scene::DocumentEdit &edit, const std::string &key, const std::string &value )
{
	if ( Reserved( key ) )
	{
		return Reject( "'" + key + "' cannot be set on the world" );
	}
	const std::string *current = edit.Settings().WorldKey( key );
	if ( current && *current == value )
	{
		return NothingToDo( "the world already has that value" );
	}
	edit.MutableSettings().SetWorldKey( key, value );
	return {};
}

} // namespace hammer::app::ops
