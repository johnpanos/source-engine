//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/structure_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/structure_ops.h"

#include "hammer/app/ops/create_ops.h"
#include "hammer/scene/map_queries.h"

#include <set>

namespace hammer::app::ops
{

using scene::ObjectId;

namespace
{

// A solid owned by a brush entity stands for its entity.
ObjectId Representative( const scene::DocumentReader &doc, ObjectId id )
{
	if ( const scene::Solid *s = doc.FindSolid( id ) )
	{
		if ( s->owner.IsValid() && doc.FindEntity( s->owner ) )
		{
			return s->owner;
		}
	}
	return id;
}

std::vector<ObjectId> Representatives( const scene::DocumentReader &doc, const std::vector<ObjectId> &ids )
{
	std::set<ObjectId> out;
	for ( ObjectId id : ids )
	{
		if ( doc.KindOf( id ) )
		{
			out.insert( Representative( doc, id ) );
		}
	}
	return std::vector<ObjectId>( out.begin(), out.end() );
}

void SetGroupOf( scene::DocumentEdit &edit, ObjectId id, ObjectId group )
{
	if ( scene::Solid *s = edit.MutableSolid( id ) )
	{
		s->group = group;
	}
	else if ( scene::Entity *e = edit.MutableEntity( id ) )
	{
		e->group = group;
	}
	else if ( scene::Group *g = edit.MutableGroup( id ) )
	{
		g->group = group;
	}
}

// Only solids (not entities) that the ids stand for.
std::vector<ObjectId> Solids( const scene::DocumentReader &doc, const std::vector<ObjectId> &ids )
{
	std::vector<ObjectId> out;
	for ( ObjectId id : scene::ExpandToLeaves( doc, ids ) )
	{
		if ( doc.FindSolid( id ) )
		{
			out.push_back( id );
		}
	}
	return out;
}

} // namespace

void RemoveEmptyContainers( scene::DocumentEdit &edit, const std::vector<ObjectId> &formerOwners )
{
	const std::set<ObjectId> former( formerOwners.begin(), formerOwners.end() );
	for ( ObjectId id : edit.EntityIds() )
	{
		// Only brush entities empty out; a point entity never owned solids.
		const bool wasBrush = former.count( id ) || !scene::EntitySolids( edit.Base(), id ).empty();
		if ( wasBrush && scene::EntitySolids( edit, id ).empty() )
		{
			edit.Remove( id );
		}
	}
	// Groups: repeat until stable, so emptied parents go too.
	for ( bool changed = true; changed; )
	{
		changed = false;
		for ( ObjectId id : edit.GroupIds() )
		{
			// Only groups this edit emptied (or created): an authored empty group
			// in the base document is content, not debris.
			const bool wasPopulated = !edit.Base().FindGroup( id ) ||
			                          !scene::GroupMembers( edit.Base(), id ).empty();
			if ( wasPopulated && scene::GroupMembers( edit, id ).empty() )
			{
				edit.Remove( id );
				changed = true;
			}
		}
	}
}

EditResult DeleteObjects( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids )
{
	const std::vector<ObjectId> all = scene::ExpandObjects( edit, ids );
	if ( all.empty() )
	{
		return NothingToDo( "nothing selected to delete" );
	}
	std::vector<ObjectId> former;
	for ( ObjectId id : all )
	{
		if ( const scene::Solid *s = edit.FindSolid( id ) )
		{
			former.push_back( s->owner );
		}
		edit.Remove( id );
	}
	RemoveEmptyContainers( edit, former );
	return {};
}

EditResult GroupObjects( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids, ObjectId &created )
{
	// Group the top of each hit within the shared parent level.
	std::vector<ObjectId> members = Representatives( edit, ids );
	if ( members.empty() )
	{
		return NothingToDo( "nothing selected to group" );
	}
	std::set<ObjectId> parents;
	for ( ObjectId id : members )
	{
		parents.insert( scene::ContainerOf( edit, id ) );
	}
	scene::Group group;
	group.group = parents.size() == 1 ? *parents.begin() : ObjectId();
	// Grouping a group into itself (or its descendant) is refused.
	for ( ObjectId id : members )
	{
		ObjectId at = group.group;
		while ( at.IsValid() )
		{
			if ( at == id )
			{
				return Reject( "cannot group an object into itself" );
			}
			at = scene::ContainerOf( edit, at );
		}
	}
	created = edit.Add( group );
	for ( ObjectId id : members )
	{
		SetGroupOf( edit, id, created );
	}
	RemoveEmptyContainers( edit );
	return {};
}

EditResult UngroupObjects( scene::DocumentEdit &edit, const std::vector<ObjectId> &groups )
{
	bool any = false;
	for ( ObjectId id : groups )
	{
		const scene::Group *g = edit.FindGroup( id );
		if ( !g )
		{
			continue;
		}
		const ObjectId parent = g->group;
		for ( ObjectId member : scene::GroupMembers( edit, id ) )
		{
			SetGroupOf( edit, member, parent );
		}
		edit.Remove( id );
		any = true;
	}
	if ( !any )
	{
		return NothingToDo( "no groups selected" );
	}
	return {};
}

EditResult TieToEntity( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids, const std::string &classname,
    const ports::IEntityCatalog *catalog, ObjectId existing, ObjectId &entity )
{
	const std::vector<ObjectId> solids = Solids( edit, ids );
	if ( solids.empty() )
	{
		return NothingToDo( "no solids selected" );
	}
	if ( existing.IsValid() )
	{
		if ( !edit.FindEntity( existing ) )
		{
			return Reject( "the target is not an entity" );
		}
		entity = existing;
	}
	else
	{
		if ( classname.empty() )
		{
			return Reject( "no entity class" );
		}
		scene::Entity e;
		e.classname = classname;
		if ( catalog )
		{
			const ports::EntityClassInfo *info = catalog->Find( classname );
			if ( !info )
			{
				return Reject( "unknown entity class '" + classname + "'" );
			}
			if ( info->kind != ports::EntityClassKind::Solid )
			{
				return Reject( "'" + classname + "' is not a brush entity class" );
			}
			e.classname = info->name;
			ApplyClassDefaults( e, *info );
		}
		// The new entity takes the place of the first solid in its group.
		const scene::Solid *first = edit.FindSolid( solids.front() );
		e.group = first->owner.IsValid() ? scene::ContainerOf( edit, first->owner ) : first->group;
		entity = edit.Add( std::move( e ) );
	}
	std::vector<ObjectId> former;
	for ( ObjectId id : solids )
	{
		scene::Solid *s = edit.MutableSolid( id );
		former.push_back( s->owner );
		s->owner = entity;
		s->group = ObjectId(); // membership is the entity's
	}
	RemoveEmptyContainers( edit, former );
	return {};
}

EditResult MoveToWorld( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids )
{
	bool any = false;
	std::vector<ObjectId> former;
	for ( ObjectId id : Solids( edit, ids ) )
	{
		const scene::Solid *s = edit.FindSolid( id );
		if ( !s->owner.IsValid() )
		{
			continue;
		}
		former.push_back( s->owner );
		const scene::Entity *owner = edit.FindEntity( s->owner );
		const ObjectId group = owner ? owner->group : ObjectId();
		scene::Solid *m = edit.MutableSolid( id );
		m->owner = ObjectId();
		m->group = group;
		any = true;
	}
	if ( !any )
	{
		return NothingToDo( "no brush entity solids selected" );
	}
	RemoveEmptyContainers( edit, former );
	return {};
}

EditResult SetHidden( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids, bool hidden )
{
	bool any = false;
	for ( ObjectId id : ids )
	{
		if ( scene::Solid *s = edit.MutableSolid( id ) )
		{
			any = any || s->hidden != hidden;
			s->hidden = hidden;
		}
		else if ( scene::Entity *e = edit.MutableEntity( id ) )
		{
			any = any || e->hidden != hidden;
			e->hidden = hidden;
		}
		else if ( scene::Group *g = edit.MutableGroup( id ) )
		{
			any = any || g->hidden != hidden;
			g->hidden = hidden;
		}
	}
	if ( !any )
	{
		return NothingToDo( hidden ? "nothing to hide" : "nothing to unhide" );
	}
	return {};
}

EditResult HideUnselected( scene::DocumentEdit &edit, const std::vector<ObjectId> &keep )
{
	const std::vector<ObjectId> kept = scene::ExpandObjects( edit, keep );
	const std::set<ObjectId> keepSet( kept.begin(), kept.end() );
	// Hide at the top level so a group containing a kept object is not hidden
	// as a whole: hide its non-kept members instead.
	std::vector<ObjectId> toHide;
	std::vector<ObjectId> pending;
	for ( ObjectId id : edit.AllIds() )
	{
		if ( !scene::ContainerOf( edit, id ).IsValid() )
		{
			pending.push_back( id );
		}
	}
	while ( !pending.empty() )
	{
		const ObjectId id = pending.back();
		pending.pop_back();
		if ( keepSet.count( id ) || !scene::IsVisible( edit, id ) )
		{
			continue;
		}
		const std::vector<ObjectId> inside = scene::ExpandObjects( edit, { id } );
		bool containsKept = false;
		for ( ObjectId in : inside )
		{
			containsKept = containsKept || keepSet.count( in ) > 0;
		}
		if ( !containsKept )
		{
			toHide.push_back( id );
		}
		else if ( edit.FindGroup( id ) )
		{
			for ( ObjectId m : scene::GroupMembers( edit, id ) )
			{
				pending.push_back( m );
			}
		}
		else if ( edit.FindEntity( id ) )
		{
			for ( ObjectId m : scene::EntitySolids( edit, id ) )
			{
				pending.push_back( m );
			}
		}
	}
	return SetHidden( edit, toHide, true );
}

EditResult UnhideAll( scene::DocumentEdit &edit )
{
	std::vector<ObjectId> hidden;
	for ( ObjectId id : edit.AllIds() )
	{
		const scene::Solid *s = edit.FindSolid( id );
		const scene::Entity *e = edit.FindEntity( id );
		const scene::Group *g = edit.FindGroup( id );
		if ( ( s && s->hidden ) || ( e && e->hidden ) || ( g && g->hidden ) )
		{
			hidden.push_back( id );
		}
	}
	return SetHidden( edit, hidden, false );
}

} // namespace hammer::app::ops
