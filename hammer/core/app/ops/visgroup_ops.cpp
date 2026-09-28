//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of public/hammer/app/ops/visgroup_ops.h.
//
//=============================================================================//

#include "hammer/app/ops/visgroup_ops.h"

#include "hammer/scene/map_queries.h"

#include <algorithm>
#include <map>
#include <set>

namespace hammer::app::ops
{

using scene::ObjectId;
using scene::Visgroup;

namespace
{

const scene::EditorInfo *EditorOf( const scene::DocumentReader &doc, ObjectId id )
{
	if ( const scene::Solid *s = doc.FindSolid( id ) )
	{
		return &s->editor;
	}
	if ( const scene::Entity *e = doc.FindEntity( id ) )
	{
		return &e->editor;
	}
	if ( const scene::Group *g = doc.FindGroup( id ) )
	{
		return &g->editor;
	}
	return nullptr;
}

scene::EditorInfo *MutableEditor( scene::DocumentEdit &edit, ObjectId id )
{
	if ( scene::Solid *s = edit.MutableSolid( id ) )
	{
		return &s->editor;
	}
	if ( scene::Entity *e = edit.MutableEntity( id ) )
	{
		return &e->editor;
	}
	if ( scene::Group *g = edit.MutableGroup( id ) )
	{
		return &g->editor;
	}
	return nullptr;
}

bool Lists( const scene::EditorInfo &info, int visgroupId )
{
	return std::find( info.visgroupIds.begin(), info.visgroupIds.end(), visgroupId ) !=
	       info.visgroupIds.end();
}

void CollectIds( const Visgroup &node, std::vector<int> &out )
{
	out.push_back( node.id );
	for ( const Visgroup &child : node.children )
	{
		CollectIds( child, out );
	}
}

int MaxTreeId( const std::vector<Visgroup> &tree )
{
	int best = 0;
	for ( const Visgroup &v : tree )
	{
		best = std::max( { best, v.id, MaxTreeId( v.children ) } );
	}
	return best;
}

// The list holding visgroup 'id' and its index there; nullptr when absent.
std::vector<Visgroup> *FindSiblings( std::vector<Visgroup> &tree, int id, std::size_t &index )
{
	for ( std::size_t i = 0; i < tree.size(); ++i )
	{
		if ( tree[i].id == id )
		{
			index = i;
			return &tree;
		}
		if ( std::vector<Visgroup> *found = FindSiblings( tree[i].children, id, index ) )
		{
			return found;
		}
	}
	return nullptr;
}

Visgroup *FindMutable( std::vector<Visgroup> &tree, int id )
{
	std::size_t index = 0;
	std::vector<Visgroup> *siblings = FindSiblings( tree, id, index );
	return siblings ? &( *siblings )[index] : nullptr;
}

// A brush entity's solid stands for its entity (it cannot hold visgroups).
std::vector<ObjectId> Representatives(
    const scene::DocumentReader &doc, const std::vector<ObjectId> &ids )
{
	std::set<ObjectId> out;
	for ( ObjectId id : ids )
	{
		if ( const scene::Solid *s = doc.FindSolid( id ) )
		{
			out.insert( s->owner.IsValid() && doc.FindEntity( s->owner ) ? s->owner : id );
		}
		else if ( doc.KindOf( id ) )
		{
			out.insert( id );
		}
	}
	return std::vector<ObjectId>( out.begin(), out.end() );
}

// Direct members of any id in 'set', in id order.
std::vector<ObjectId> MembersOf( const scene::DocumentReader &doc, const std::set<int> &set )
{
	std::vector<ObjectId> out;
	std::vector<ObjectId> all = doc.SolidIds();
	for ( ObjectId id : doc.EntityIds() )
	{
		all.push_back( id );
	}
	for ( ObjectId id : doc.GroupIds() )
	{
		all.push_back( id );
	}
	std::sort( all.begin(), all.end() );
	for ( ObjectId id : all )
	{
		const scene::EditorInfo *info = EditorOf( doc, id );
		for ( int v : info->visgroupIds )
		{
			if ( set.count( v ) )
			{
				out.push_back( id );
				break;
			}
		}
	}
	return out;
}

// Shows objects among 'affected' that are hidden and no longer covered by any
// visgroup (legacy CheckVisibility).
void RestoreOrphans( scene::DocumentEdit &edit, const std::vector<ObjectId> &changed )
{
	for ( ObjectId id : scene::ExpandObjects( edit, changed ) )
	{
		const scene::EditorInfo *info = EditorOf( edit, id );
		if ( info && !info->visgroupShown && !CoveredByVisgroup( edit, id ) )
		{
			MutableEditor( edit, id )->visgroupShown = true;
		}
	}
}

} // namespace

std::vector<int> VisgroupSubtree( const scene::DocumentReader &doc, int visgroupId )
{
	std::vector<int> out;
	if ( const Visgroup *v = scene::FindVisgroup( doc.Settings().visgroups, visgroupId ).visgroup )
	{
		CollectIds( *v, out );
	}
	return out;
}

bool CoveredByVisgroup( const scene::DocumentReader &doc, ObjectId id )
{
	ObjectId at = id;
	for ( int guard = 0; guard < 4096 && at.IsValid(); ++guard )
	{
		const scene::EditorInfo *info = EditorOf( doc, at );
		if ( info && !info->visgroupIds.empty() )
		{
			return true;
		}
		at = scene::ContainerOf( doc, at );
	}
	return false;
}

std::vector<ObjectId> VisgroupMembers(
    const scene::DocumentReader &doc, int visgroupId, bool recursive )
{
	if ( !scene::FindVisgroup( doc.Settings().visgroups, visgroupId ).visgroup )
	{
		return {};
	}
	std::set<int> set;
	if ( recursive )
	{
		const std::vector<int> ids = VisgroupSubtree( doc, visgroupId );
		set.insert( ids.begin(), ids.end() );
	}
	else
	{
		set.insert( visgroupId );
	}
	return MembersOf( doc, set );
}

int NextVisgroupId( const scene::DocumentReader &doc )
{
	int best = MaxTreeId( doc.Settings().visgroups );
	for ( const std::vector<ObjectId> &ids : { doc.SolidIds(), doc.EntityIds(), doc.GroupIds() } )
	{
		for ( ObjectId id : ids )
		{
			for ( int v : EditorOf( doc, id )->visgroupIds )
			{
				best = std::max( best, v );
			}
		}
	}
	return best + 1;
}

EditResult CreateVisgroup(
    scene::DocumentEdit &edit, const std::string &name, int parentId, int &createdId )
{
	if ( name.empty() )
	{
		return Reject( "a visgroup needs a name" );
	}
	if ( parentId != 0 && !scene::FindVisgroup( edit.Settings().visgroups, parentId ).visgroup )
	{
		return Reject( "unknown parent visgroup " + std::to_string( parentId ) );
	}
	Visgroup v;
	v.id = NextVisgroupId( edit );
	v.name = name;
	createdId = v.id;
	std::vector<Visgroup> &tree = edit.MutableSettings().visgroups;
	if ( parentId == 0 )
	{
		tree.push_back( std::move( v ) );
	}
	else
	{
		FindMutable( tree, parentId )->children.push_back( std::move( v ) );
	}
	return {};
}

EditResult RenameVisgroup( scene::DocumentEdit &edit, int visgroupId, const std::string &name )
{
	const Visgroup *v = scene::FindVisgroup( edit.Settings().visgroups, visgroupId ).visgroup;
	if ( !v )
	{
		return Reject( "unknown visgroup " + std::to_string( visgroupId ) );
	}
	if ( name.empty() )
	{
		return Reject( "a visgroup needs a name" );
	}
	if ( v->name == name )
	{
		return NothingToDo( "the visgroup already has that name" );
	}
	FindMutable( edit.MutableSettings().visgroups, visgroupId )->name = name;
	return {};
}

EditResult DeleteVisgroup( scene::DocumentEdit &edit, int visgroupId )
{
	if ( !scene::FindVisgroup( edit.Settings().visgroups, visgroupId ).visgroup )
	{
		return Reject( "unknown visgroup " + std::to_string( visgroupId ) );
	}
	{
		std::size_t index = 0;
		std::vector<Visgroup> *siblings =
		    FindSiblings( edit.MutableSettings().visgroups, visgroupId, index );
		std::vector<Visgroup> children = std::move( ( *siblings )[index].children );
		siblings->erase( siblings->begin() + static_cast<std::ptrdiff_t>( index ) );
		siblings->insert( siblings->begin() + static_cast<std::ptrdiff_t>( index ),
		    std::make_move_iterator( children.begin() ),
		    std::make_move_iterator( children.end() ) );
	}
	const std::vector<ObjectId> members = MembersOf( edit, { visgroupId } );
	for ( ObjectId id : members )
	{
		std::erase( MutableEditor( edit, id )->visgroupIds, visgroupId );
	}
	RestoreOrphans( edit, members );
	return {};
}

EditResult MoveVisgroup( scene::DocumentEdit &edit, int visgroupId, int newParentId )
{
	const scene::VisgroupLookup found =
	    scene::FindVisgroup( edit.Settings().visgroups, visgroupId );
	if ( !found.visgroup )
	{
		return Reject( "unknown visgroup " + std::to_string( visgroupId ) );
	}
	if ( newParentId != 0 &&
	     !scene::FindVisgroup( edit.Settings().visgroups, newParentId ).visgroup )
	{
		return Reject( "unknown parent visgroup " + std::to_string( newParentId ) );
	}
	const std::vector<int> subtree = VisgroupSubtree( edit, visgroupId );
	if ( std::find( subtree.begin(), subtree.end(), newParentId ) != subtree.end() )
	{
		return Reject( "a visgroup cannot move into itself or one of its descendants" );
	}
	if ( found.parentId == newParentId )
	{
		return NothingToDo( "the visgroup already has that parent" );
	}
	std::vector<Visgroup> &tree = edit.MutableSettings().visgroups;
	std::size_t index = 0;
	std::vector<Visgroup> *siblings = FindSiblings( tree, visgroupId, index );
	Visgroup moved = std::move( ( *siblings )[index] );
	siblings->erase( siblings->begin() + static_cast<std::ptrdiff_t>( index ) );
	if ( newParentId == 0 )
	{
		tree.push_back( std::move( moved ) );
	}
	else
	{
		FindMutable( tree, newParentId )->children.push_back( std::move( moved ) );
	}
	return {};
}

EditResult AddToVisgroup( scene::DocumentEdit &edit, const std::vector<ObjectId> &ids,
    int visgroupId, bool removeFromOthers )
{
	if ( !scene::FindVisgroup( edit.Settings().visgroups, visgroupId ).visgroup )
	{
		return Reject( "unknown visgroup " + std::to_string( visgroupId ) );
	}
	const std::vector<ObjectId> objects = Representatives( edit, ids );
	if ( objects.empty() )
	{
		return NothingToDo( "nothing selected to add to the visgroup" );
	}
	bool changed = false;
	for ( ObjectId id : objects )
	{
		const scene::EditorInfo *info = EditorOf( edit, id );
		const bool already = Lists( *info, visgroupId );
		const bool others = info->visgroupIds.size() > ( already ? 1u : 0u );
		if ( already && !( removeFromOthers && others ) )
		{
			continue;
		}
		scene::EditorInfo *m = MutableEditor( edit, id );
		if ( removeFromOthers )
		{
			m->visgroupIds.clear();
		}
		m->visgroupIds.push_back( visgroupId );
		changed = true;
	}
	if ( !changed )
	{
		return NothingToDo( "the objects are already in the visgroup" );
	}
	return {};
}

EditResult RemoveFromVisgroup(
    scene::DocumentEdit &edit, const std::vector<ObjectId> &ids, int visgroupId )
{
	if ( !scene::FindVisgroup( edit.Settings().visgroups, visgroupId ).visgroup )
	{
		return Reject( "unknown visgroup " + std::to_string( visgroupId ) );
	}
	std::vector<ObjectId> changed;
	for ( ObjectId id : Representatives( edit, ids ) )
	{
		if ( Lists( *EditorOf( edit, id ), visgroupId ) )
		{
			std::erase( MutableEditor( edit, id )->visgroupIds, visgroupId );
			changed.push_back( id );
		}
	}
	if ( changed.empty() )
	{
		return NothingToDo( "none of the objects is in the visgroup" );
	}
	RestoreOrphans( edit, changed );
	return {};
}

EditResult SetVisgroupVisible( scene::DocumentEdit &edit, int visgroupId, bool visible )
{
	if ( !scene::FindVisgroup( edit.Settings().visgroups, visgroupId ).visgroup )
	{
		return Reject( "unknown visgroup " + std::to_string( visgroupId ) );
	}
	const std::vector<int> subtreeIds = VisgroupSubtree( edit, visgroupId );
	const std::set<int> subtree( subtreeIds.begin(), subtreeIds.end() );
	const std::vector<ObjectId> touched = scene::ExpandObjects( edit, MembersOf( edit, subtree ) );
	if ( touched.empty() )
	{
		return NothingToDo( "the visgroup has no members" );
	}
	const std::set<ObjectId> touchedSet( touched.begin(), touched.end() );

	// W suppresses when its members outside this show exist and are all hidden.
	std::map<int, bool> suppresses;
	auto suppressing = [&]( int w )
	{
		const auto it = suppresses.find( w );
		if ( it != suppresses.end() )
		{
			return it->second;
		}
		bool any = false;
		bool allHidden = true;
		for ( ObjectId m : VisgroupMembers( edit, w, true ) )
		{
			if ( touchedSet.count( m ) )
			{
				continue;
			}
			any = true;
			allHidden = allHidden && !EditorOf( edit, m )->visgroupShown;
		}
		const bool result = any && allHidden;
		suppresses[w] = result;
		return result;
	};
	auto suppressed = [&]( ObjectId id )
	{
		ObjectId at = id;
		for ( int guard = 0; guard < 4096 && at.IsValid(); ++guard )
		{
			for ( int w : EditorOf( edit, at )->visgroupIds )
			{
				if ( !subtree.count( w ) &&
				     scene::FindVisgroup( edit.Settings().visgroups, w ).visgroup &&
				     suppressing( w ) )
				{
					return true;
				}
			}
			at = scene::ContainerOf( edit, at );
		}
		return false;
	};

	// Decide every object first, against the state before this operation.
	std::vector<ObjectId> show;
	if ( visible )
	{
		for ( ObjectId id : touched )
		{
			if ( !suppressed( id ) )
			{
				show.push_back( id );
			}
		}
	}
	bool changed = false;
	if ( visible )
	{
		for ( ObjectId id : show )
		{
			const scene::EditorInfo *info = EditorOf( edit, id );
			if ( !info->visgroupShown || !info->visgroupAutoShown )
			{
				scene::EditorInfo *m = MutableEditor( edit, id );
				m->visgroupShown = true;
				m->visgroupAutoShown = true;
				changed = true;
			}
		}
	}
	else
	{
		for ( ObjectId id : touched )
		{
			if ( EditorOf( edit, id )->visgroupShown )
			{
				MutableEditor( edit, id )->visgroupShown = false;
				changed = true;
			}
		}
	}
	if ( !changed )
	{
		return NothingToDo(
		    visible ? "the visgroup is already shown" : "the visgroup is already hidden" );
	}
	return {};
}

VisgroupState VisgroupVisibility( const scene::DocumentReader &doc, int visgroupId )
{
	bool shown = false;
	bool hidden = false;
	for ( ObjectId id : VisgroupMembers( doc, visgroupId, true ) )
	{
		( EditorOf( doc, id )->visgroupShown ? shown : hidden ) = true;
	}
	if ( shown && hidden )
	{
		return VisgroupState::Mixed;
	}
	if ( shown )
	{
		return VisgroupState::Shown;
	}
	return hidden ? VisgroupState::Hidden : VisgroupState::Empty;
}

} // namespace hammer::app::ops
