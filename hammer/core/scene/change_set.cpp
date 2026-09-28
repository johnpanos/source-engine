//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/scene/change_set.h.
//
//=============================================================================//

#include "hammer/scene/change_set.h"

#include "hammer/scene/solid_geometry.h"
#include "mapgeometry/polytope.h"

#include <algorithm>
#include <set>
#include <string>

namespace hammer::scene
{

ObjectId IdOf( const MapObject &object )
{
	return std::visit( []( const auto &o ) { return o.id; }, object );
}

ObjectKind KindOf( const MapObject &object )
{
	switch ( object.index() )
	{
	case 0:
		return ObjectKind::Solid;
	case 1:
		return ObjectKind::Entity;
	default:
		return ObjectKind::Group;
	}
}

std::vector<ObjectId> ChangeSet::Created() const
{
	std::vector<ObjectId> out;
	for ( const ObjectChange &c : objects )
	{
		if ( !c.before && c.after )
		{
			out.push_back( c.id );
		}
	}
	return out;
}

std::vector<ObjectId> ChangeSet::Removed() const
{
	std::vector<ObjectId> out;
	for ( const ObjectChange &c : objects )
	{
		if ( c.before && !c.after )
		{
			out.push_back( c.id );
		}
	}
	return out;
}

std::vector<ObjectId> ChangeSet::Modified() const
{
	std::vector<ObjectId> out;
	for ( const ObjectChange &c : objects )
	{
		if ( c.before && c.after )
		{
			out.push_back( c.id );
		}
	}
	return out;
}

namespace
{

void PutObject( MapDocument &doc, const MapObject &object )
{
	std::visit( [&]( const auto &o ) { doc.Put( o ); }, object );
}

} // namespace

bool Apply( MapDocument &doc, const ChangeSet &changes, ApplyDirection direction )
{
	for ( const ObjectChange &c : changes.objects )
	{
		if ( DocumentSerialOf( c.id ) != doc.Serial() )
		{
			return false;
		}
	}
	const bool forward = direction == ApplyDirection::Forward;
	// Erase first, then install, so a kind change under one id cannot collide.
	for ( const ObjectChange &c : changes.objects )
	{
		doc.Erase( c.id );
	}
	for ( const ObjectChange &c : changes.objects )
	{
		const std::optional<MapObject> &value = forward ? c.after : c.before;
		if ( value )
		{
			PutObject( doc, *value );
		}
	}
	const std::optional<DocumentSettings> &settings =
	    forward ? changes.settingsAfter : changes.settingsBefore;
	if ( settings )
	{
		doc.MutableSettings() = *settings;
	}
	return true;
}

DocumentEdit::DocumentEdit( const MapDocument &base )
    : m_base( base ), m_nextLocal( base.NextLocalId() ), m_nextVmfId( base.NextVmfId() )
{
}

const MapObject *DocumentEdit::StagedObject( ObjectId id ) const
{
	const auto it = m_staged.find( id );
	if ( it == m_staged.end() || !it->second )
	{
		return nullptr;
	}
	return &*it->second;
}

template <typename T> const T *DocumentEdit::FindStaged( ObjectId id ) const
{
	const auto it = m_staged.find( id );
	if ( it != m_staged.end() )
	{
		return it->second ? std::get_if<T>( &*it->second ) : nullptr;
	}
	if constexpr ( std::is_same_v<T, Solid> )
	{
		return m_base.FindSolid( id );
	}
	else if constexpr ( std::is_same_v<T, Entity> )
	{
		return m_base.FindEntity( id );
	}
	else
	{
		return m_base.FindGroup( id );
	}
}

const Solid *DocumentEdit::FindSolid( ObjectId id ) const
{
	return FindStaged<Solid>( id );
}

const Entity *DocumentEdit::FindEntity( ObjectId id ) const
{
	return FindStaged<Entity>( id );
}

const Group *DocumentEdit::FindGroup( ObjectId id ) const
{
	return FindStaged<Group>( id );
}

std::optional<ObjectKind> DocumentEdit::KindOf( ObjectId id ) const
{
	const auto it = m_staged.find( id );
	if ( it != m_staged.end() )
	{
		if ( !it->second )
		{
			return std::nullopt;
		}
		return scene::KindOf( *it->second );
	}
	return m_base.KindOf( id );
}

const DocumentSettings &DocumentEdit::Settings() const
{
	return m_settings ? *m_settings : m_base.Settings();
}

std::vector<ObjectId> DocumentEdit::IdsOfKind( ObjectKind kind ) const
{
	std::vector<ObjectId> out;
	auto collectBase = [&]( const auto &map )
	{
		for ( const auto &entry : map )
		{
			const auto it = m_staged.find( entry.first );
			if ( it == m_staged.end() )
			{
				out.push_back( entry.first );
			}
		}
	};
	switch ( kind )
	{
	case ObjectKind::Solid:
		collectBase( m_base.Solids() );
		break;
	case ObjectKind::Entity:
		collectBase( m_base.Entities() );
		break;
	case ObjectKind::Group:
		collectBase( m_base.Groups() );
		break;
	}
	for ( const auto &[id, value] : m_staged )
	{
		if ( value && scene::KindOf( *value ) == kind )
		{
			out.push_back( id );
		}
	}
	std::sort( out.begin(), out.end() );
	return out;
}

std::vector<ObjectId> DocumentEdit::SolidIds() const
{
	return IdsOfKind( ObjectKind::Solid );
}

std::vector<ObjectId> DocumentEdit::EntityIds() const
{
	return IdsOfKind( ObjectKind::Entity );
}

std::vector<ObjectId> DocumentEdit::GroupIds() const
{
	return IdsOfKind( ObjectKind::Group );
}

std::vector<ObjectId> DocumentEdit::AllIds() const
{
	std::vector<ObjectId> out = SolidIds();
	const std::vector<ObjectId> e = EntityIds();
	const std::vector<ObjectId> g = GroupIds();
	out.insert( out.end(), e.begin(), e.end() );
	out.insert( out.end(), g.begin(), g.end() );
	std::sort( out.begin(), out.end() );
	return out;
}

namespace
{

template <typename T> T *MutableOf( std::map<ObjectId, std::optional<MapObject>> &staged,
    const T *current, ObjectId id )
{
	if ( !current )
	{
		return nullptr;
	}
	auto it = staged.find( id );
	if ( it == staged.end() )
	{
		it = staged.emplace( id, MapObject( *current ) ).first;
	}
	return std::get_if<T>( &*it->second );
}

} // namespace

Solid *DocumentEdit::MutableSolid( ObjectId id )
{
	return MutableOf<Solid>( m_staged, FindSolid( id ), id );
}

Entity *DocumentEdit::MutableEntity( ObjectId id )
{
	return MutableOf<Entity>( m_staged, FindEntity( id ), id );
}

Group *DocumentEdit::MutableGroup( ObjectId id )
{
	return MutableOf<Group>( m_staged, FindGroup( id ), id );
}

DocumentSettings &DocumentEdit::MutableSettings()
{
	if ( !m_settings )
	{
		m_settings = m_base.Settings();
	}
	return *m_settings;
}

std::uint32_t DocumentEdit::AllocateVmfId()
{
	return m_nextVmfId++;
}

ObjectId DocumentEdit::AllocateId()
{
	ObjectId id;
	id.value = ( static_cast<std::uint64_t>( m_base.Serial() ) << 32 ) | m_nextLocal++;
	return id;
}

ObjectId DocumentEdit::Add( Solid solid )
{
	solid.id = AllocateId();
	if ( solid.vmfId == 0 )
	{
		solid.vmfId = AllocateVmfId();
	}
	for ( Side &side : solid.sides )
	{
		if ( side.vmfId == 0 )
		{
			side.vmfId = AllocateVmfId();
		}
	}
	const ObjectId id = solid.id;
	m_staged[id] = MapObject( std::move( solid ) );
	return id;
}

ObjectId DocumentEdit::Add( Entity entity )
{
	entity.id = AllocateId();
	if ( entity.vmfId == 0 )
	{
		entity.vmfId = AllocateVmfId();
	}
	const ObjectId id = entity.id;
	m_staged[id] = MapObject( std::move( entity ) );
	return id;
}

ObjectId DocumentEdit::Add( Group group )
{
	group.id = AllocateId();
	if ( group.vmfId == 0 )
	{
		group.vmfId = AllocateVmfId();
	}
	const ObjectId id = group.id;
	m_staged[id] = MapObject( std::move( group ) );
	return id;
}

void DocumentEdit::Put( MapObject object )
{
	const ObjectId id = IdOf( object );
	const std::uint32_t local = static_cast<std::uint32_t>( id.value & 0xffffffffu );
	if ( DocumentSerialOf( id ) == m_base.Serial() && local >= m_nextLocal )
	{
		m_nextLocal = local + 1;
	}
	m_staged[id] = std::move( object );
}

bool DocumentEdit::Remove( ObjectId id )
{
	if ( !KindOf( id ) )
	{
		return false;
	}
	m_staged[id] = std::nullopt;
	return true;
}

namespace
{

std::optional<MapObject> BaseObject( const MapDocument &doc, ObjectId id )
{
	if ( const Solid *s = doc.FindSolid( id ) )
	{
		return MapObject( *s );
	}
	if ( const Entity *e = doc.FindEntity( id ) )
	{
		return MapObject( *e );
	}
	if ( const Group *g = doc.FindGroup( id ) )
	{
		return MapObject( *g );
	}
	return std::nullopt;
}

} // namespace

ChangeSet DocumentEdit::Finish() const
{
	ChangeSet out;
	for ( const auto &[id, value] : m_staged )
	{
		std::optional<MapObject> before = BaseObject( m_base, id );
		if ( before == value )
		{
			continue;
		}
		out.objects.push_back( { id, std::move( before ), value } );
	}
	if ( m_settings && !( *m_settings == m_base.Settings() ) )
	{
		out.settingsBefore = m_base.Settings();
		out.settingsAfter = *m_settings;
	}
	return out;
}

std::vector<ObjectId> DocumentEdit::TouchedIds() const
{
	std::vector<ObjectId> out;
	for ( const auto &entry : m_staged )
	{
		out.push_back( entry.first );
	}
	return out;
}

std::vector<std::string> ValidateEdit( const DocumentEdit &edit )
{
	std::vector<std::string> problems;
	auto idText = []( ObjectId id ) { return std::to_string( id.value & 0xffffffffu ); };
	const std::vector<ObjectId> touched = edit.TouchedIds();

	std::set<std::uint32_t> touchedSides;
	std::set<ObjectId> touchedSolids;
	std::vector<ObjectId> removed;
	for ( ObjectId id : touched )
	{
		const std::optional<ObjectKind> kind = edit.KindOf( id );
		if ( !kind )
		{
			removed.push_back( id );
			continue;
		}
		auto checkGroup = [&]( ObjectId group, const char *what )
		{
			if ( group.IsValid() && !edit.FindGroup( group ) )
			{
				problems.push_back( std::string( what ) + " " + idText( id ) + " refers to a missing group" );
			}
		};
		if ( *kind == ObjectKind::Solid )
		{
			const Solid *s = edit.FindSolid( id );
			touchedSolids.insert( id );
			if ( s->owner.IsValid() && !edit.FindEntity( s->owner ) )
			{
				problems.push_back( "solid " + idText( id ) + " is owned by a missing entity" );
			}
			checkGroup( s->group, "solid" );
			if ( s->sides.size() < 4 )
			{
				problems.push_back( "solid " + idText( id ) + " has fewer than four sides" );
			}
			for ( const Side &side : s->sides )
			{
				if ( side.vmfId == 0 || !touchedSides.insert( side.vmfId ).second )
				{
					problems.push_back( "solid " + idText( id ) + " repeats or lacks side id " +
					                    std::to_string( side.vmfId ) );
				}
			}
			std::vector<mapgeometry::Plane> planes;
			for ( const Side &side : s->sides )
			{
				planes.push_back( side.Plane() );
			}
			if ( s->sides.size() >= 4 && !mapgeometry::IsClosedSolid( planes ) )
			{
				problems.push_back( "solid " + idText( id ) + " does not bound a closed volume" );
			}
		}
		else if ( *kind == ObjectKind::Entity )
		{
			checkGroup( edit.FindEntity( id )->group, "entity" );
		}
		else
		{
			const Group *g = edit.FindGroup( id );
			checkGroup( g->group, "group" );
			ObjectId at = g->group;
			for ( std::size_t steps = 0; at.IsValid() && steps < 4096; ++steps )
			{
				if ( at == id )
				{
					problems.push_back( "group " + idText( id ) + " contains itself" );
					break;
				}
				const Group *parent = edit.FindGroup( at );
				at = parent ? parent->group : ObjectId();
			}
		}
	}

	// Side ids of touched solids must not collide with untouched solids.
	if ( !touchedSides.empty() )
	{
		for ( ObjectId id : edit.SolidIds() )
		{
			if ( touchedSolids.count( id ) )
			{
				continue;
			}
			for ( const Side &side : edit.FindSolid( id )->sides )
			{
				if ( touchedSides.count( side.vmfId ) )
				{
					problems.push_back( "side id " + std::to_string( side.vmfId ) + " is not unique" );
				}
			}
		}
	}

	// Nothing live may refer to a removed object.
	if ( !removed.empty() )
	{
		const std::set<ObjectId> gone( removed.begin(), removed.end() );
		for ( ObjectId id : edit.SolidIds() )
		{
			const Solid *s = edit.FindSolid( id );
			if ( gone.count( s->owner ) || gone.count( s->group ) )
			{
				problems.push_back( "solid " + idText( id ) + " refers to a removed object" );
			}
		}
		for ( ObjectId id : edit.EntityIds() )
		{
			if ( gone.count( edit.FindEntity( id )->group ) )
			{
				problems.push_back( "entity " + idText( id ) + " refers to a removed group" );
			}
		}
		for ( ObjectId id : edit.GroupIds() )
		{
			if ( gone.count( edit.FindGroup( id )->group ) )
			{
				problems.push_back( "group " + idText( id ) + " refers to a removed group" );
			}
		}
	}
	return problems;
}

ChangeSet CommitEdit( MapDocument &doc, const DocumentEdit &edit )
{
	ChangeSet changes = edit.Finish();
	Apply( doc, changes, ApplyDirection::Forward );
	doc.AdvanceCounters( edit.NextLocal(), edit.NextVmfId() );
	return changes;
}

} // namespace hammer::scene
