//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/scene/map_document.h.
//
//=============================================================================//

#include "hammer/scene/map_document.h"

#include <set>

namespace hammer::scene
{

const std::string *DocumentSettings::WorldKey( std::string_view key ) const
{
	for ( const kvtext::KeyValue &kv : worldKeys )
	{
		if ( kv.key == key )
		{
			return &kv.value;
		}
	}
	return nullptr;
}

bool DocumentSettings::SetWorldKey( std::string_view key, std::string_view value )
{
	for ( kvtext::KeyValue &kv : worldKeys )
	{
		if ( kv.key == key )
		{
			if ( kv.value == value )
			{
				return false;
			}
			kv.value = std::string( value );
			return true;
		}
	}
	worldKeys.push_back( { std::string( key ), std::string( value ) } );
	return true;
}

VisgroupLookup FindVisgroup( const std::vector<Visgroup> &tree, int id )
{
	struct Walker
	{
		int id;
		VisgroupLookup Walk( const std::vector<Visgroup> &level, int parent ) const
		{
			for ( const Visgroup &v : level )
			{
				if ( v.id == id )
				{
					return { &v, parent };
				}
				const VisgroupLookup inner = Walk( v.children, v.id );
				if ( inner.visgroup )
				{
					return inner;
				}
			}
			return {};
		}
	};
	return Walker{ id }.Walk( tree, 0 );
}

MapDocument::MapDocument( std::uint32_t serial ) : m_serial( serial == 0 ? 1 : serial )
{
	m_settings.worldKeys.push_back( { "mapversion", "1" } );
}

const Solid *MapDocument::FindSolid( ObjectId id ) const
{
	const auto it = m_solids.find( id );
	return it == m_solids.end() ? nullptr : &it->second;
}

const Entity *MapDocument::FindEntity( ObjectId id ) const
{
	const auto it = m_entities.find( id );
	return it == m_entities.end() ? nullptr : &it->second;
}

const Group *MapDocument::FindGroup( ObjectId id ) const
{
	const auto it = m_groups.find( id );
	return it == m_groups.end() ? nullptr : &it->second;
}

std::optional<ObjectKind> MapDocument::KindOf( ObjectId id ) const
{
	if ( m_solids.count( id ) )
	{
		return ObjectKind::Solid;
	}
	if ( m_entities.count( id ) )
	{
		return ObjectKind::Entity;
	}
	if ( m_groups.count( id ) )
	{
		return ObjectKind::Group;
	}
	return std::nullopt;
}

namespace
{

template <typename T> std::vector<ObjectId> KeysOf( const std::map<ObjectId, T> &map )
{
	std::vector<ObjectId> out;
	out.reserve( map.size() );
	for ( const auto &entry : map )
	{
		out.push_back( entry.first );
	}
	return out;
}

} // namespace

std::vector<ObjectId> MapDocument::SolidIds() const
{
	return KeysOf( m_solids );
}

std::vector<ObjectId> MapDocument::EntityIds() const
{
	return KeysOf( m_entities );
}

std::vector<ObjectId> MapDocument::GroupIds() const
{
	return KeysOf( m_groups );
}

ObjectId MapDocument::AllocateId()
{
	ObjectId id;
	id.value = ( static_cast<std::uint64_t>( m_serial ) << 32 ) | m_nextLocal++;
	return id;
}

std::uint32_t MapDocument::AllocateVmfId()
{
	return m_nextVmfId++;
}

void MapDocument::NoteVmfId( std::uint32_t seen )
{
	if ( seen >= m_nextVmfId )
	{
		m_nextVmfId = seen + 1;
	}
}

void MapDocument::AdvanceCounters( std::uint32_t nextLocal, std::uint32_t nextVmfId )
{
	m_nextLocal = nextLocal > m_nextLocal ? nextLocal : m_nextLocal;
	m_nextVmfId = nextVmfId > m_nextVmfId ? nextVmfId : m_nextVmfId;
}

namespace
{

template <typename T>
bool PutInto( std::map<ObjectId, T> &into, T value, std::uint32_t serial, std::uint32_t &nextLocal )
{
	if ( !value.id.IsValid() || DocumentSerialOf( value.id ) != serial )
	{
		return false;
	}
	// An id restored by undo (or copied from another snapshot of this document)
	// must never be re-issued.
	const std::uint32_t local = static_cast<std::uint32_t>( value.id.value & 0xffffffffu );
	if ( local >= nextLocal )
	{
		nextLocal = local + 1;
	}
	const ObjectId id = value.id;
	into.insert_or_assign( id, std::move( value ) );
	return true;
}

} // namespace

bool MapDocument::Put( Solid solid )
{
	if ( m_entities.count( solid.id ) || m_groups.count( solid.id ) )
	{
		return false;
	}
	for ( const Side &side : solid.sides )
	{
		NoteVmfId( side.vmfId );
	}
	NoteVmfId( solid.vmfId );
	return PutInto( m_solids, std::move( solid ), m_serial, m_nextLocal );
}

bool MapDocument::Put( Entity entity )
{
	if ( m_solids.count( entity.id ) || m_groups.count( entity.id ) )
	{
		return false;
	}
	NoteVmfId( entity.vmfId );
	return PutInto( m_entities, std::move( entity ), m_serial, m_nextLocal );
}

bool MapDocument::Put( Group group )
{
	if ( m_solids.count( group.id ) || m_entities.count( group.id ) )
	{
		return false;
	}
	NoteVmfId( group.vmfId );
	return PutInto( m_groups, std::move( group ), m_serial, m_nextLocal );
}

bool MapDocument::Erase( ObjectId id )
{
	return m_solids.erase( id ) + m_entities.erase( id ) + m_groups.erase( id ) > 0;
}

std::vector<std::string> MapDocument::Validate() const
{
	std::vector<std::string> problems;
	auto idText = []( ObjectId id )
	{
		return std::to_string( id.value & 0xffffffffu );
	};

	auto checkGroupRef = [&]( ObjectId group, ObjectId self, const char *what )
	{
		if ( group.IsValid() && !m_groups.count( group ) )
		{
			problems.push_back(
			    std::string( what ) + " " + idText( self ) + " refers to a missing group" );
		}
	};

	std::set<std::uint32_t> sideIds;
	for ( const auto &[id, solid] : m_solids )
	{
		if ( solid.owner.IsValid() && !m_entities.count( solid.owner ) )
		{
			problems.push_back( "solid " + idText( id ) + " is owned by a missing entity" );
		}
		checkGroupRef( solid.group, id, "solid" );
		if ( solid.sides.size() < 4 )
		{
			problems.push_back( "solid " + idText( id ) + " has fewer than four sides" );
		}
		for ( const Side &side : solid.sides )
		{
			if ( !sideIds.insert( side.vmfId ).second )
			{
				problems.push_back( "side id " + std::to_string( side.vmfId ) + " is not unique" );
			}
		}
	}
	for ( const auto &[id, entity] : m_entities )
	{
		checkGroupRef( entity.group, id, "entity" );
	}
	for ( const auto &[id, group] : m_groups )
	{
		checkGroupRef( group.group, id, "group" );
		// Walk up; a chain longer than the group count is a cycle.
		ObjectId at = group.group;
		std::size_t steps = 0;
		while ( at.IsValid() && steps <= m_groups.size() )
		{
			if ( at == id )
			{
				problems.push_back( "group " + idText( id ) + " contains itself" );
				break;
			}
			const Group *parent = FindGroup( at );
			at = parent ? parent->group : ObjectId();
			++steps;
		}
	}
	return problems;
}

bool SameContent( const MapDocument &a, const MapDocument &b )
{
	return a.Solids() == b.Solids() && a.Entities() == b.Entities() && a.Groups() == b.Groups() &&
	       a.Settings() == b.Settings();
}

} // namespace hammer::scene
