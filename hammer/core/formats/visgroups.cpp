//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer::formats VisGroup parsing + visibility
//			filtering. See public/hammer/formats/visgroups.h.
//
//=============================================================================//

#include "hammer/formats/visgroups.h"

#include <set>
#include <string>
#include <vector>

namespace hammer::formats
{

namespace
{

bool ParseInt( const std::string &text, int &out )
{
	try
	{
		std::size_t consumed = 0;
		const int value = std::stoi( text, &consumed );
		if ( consumed == 0 )
		{
			return false;
		}
		out = value;
		return true;
	}
	catch ( ... )
	{
		return false;
	}
}

// Parses a single "visgroup" block (recursively) into a VisGroup.
VisGroup ParseOneVisGroup( const KeyValueNode &block )
{
	VisGroup group;
	if ( const std::string *id = block.Find( "visgroupid" ) )
	{
		ParseInt( *id, group.id );
	}
	if ( const std::string *name = block.Find( "name" ) )
	{
		group.name = *name;
	}
	for ( const KeyValueNode &child : block.children )
	{
		if ( child.name == "visgroup" )
		{
			group.children.push_back( ParseOneVisGroup( child ) );
		}
	}
	return group;
}

// Collects an object's visgroup memberships from its "editor" sub-block (an object
// may belong to several groups: multiple "visgroupid" pairs).
void CollectMemberships( const KeyValueNode &object, std::vector<int> &out )
{
	for ( const KeyValueNode &child : object.children )
	{
		if ( child.name != "editor" )
		{
			continue;
		}
		for ( const KeyValue &kv : child.pairs )
		{
			if ( kv.key == "visgroupid" )
			{
				int id = 0;
				if ( ParseInt( kv.value, id ) )
				{
					out.push_back( id );
				}
			}
		}
	}
}

// Adds 'group' and all its descendants' ids to 'out'.
void AddSubtreeIds( const VisGroup &group, std::set<int> &out )
{
	out.insert( group.id );
	for ( const VisGroup &child : group.children )
	{
		AddSubtreeIds( child, out );
	}
}

// If 'group' (or a descendant) is a seed id, marks its whole subtree hidden.
void ExpandHidden( const VisGroup &group, const std::set<int> &seeds, std::set<int> &hidden )
{
	if ( seeds.count( group.id ) != 0 )
	{
		AddSubtreeIds( group, hidden );
	}
	for ( const VisGroup &child : group.children )
	{
		ExpandHidden( child, seeds, hidden );
	}
}

bool ObjectHidden( const KeyValueNode &object, const std::set<int> &hidden )
{
	std::vector<int> memberships;
	CollectMemberships( object, memberships );
	for ( int id : memberships )
	{
		if ( hidden.count( id ) != 0 )
		{
			return true;
		}
	}
	return false;
}

} // namespace

std::vector<VisGroup> ParseVisGroups( const KeyValueNode &root )
{
	std::vector<VisGroup> groups;
	for ( const KeyValueNode &block : root.children )
	{
		if ( block.name != "visgroups" )
		{
			continue;
		}
		for ( const KeyValueNode &child : block.children )
		{
			if ( child.name == "visgroup" )
			{
				groups.push_back( ParseOneVisGroup( child ) );
			}
		}
	}
	return groups;
}

VisGroupFilterResult ApplyVisGroupFilter(
    const KeyValueNode &root, const std::vector<int> &hiddenGroupIds )
{
	// Expand the seed ids to include descendants (hiding a parent hides children).
	const std::vector<VisGroup> tree = ParseVisGroups( root );
	std::set<int> seeds( hiddenGroupIds.begin(), hiddenGroupIds.end() );
	std::set<int> hidden;
	for ( const VisGroup &group : tree )
	{
		ExpandHidden( group, seeds, hidden );
	}
	// A seed id that names no group in the tree still hides direct members.
	for ( int seed : seeds )
	{
		hidden.insert( seed );
	}

	VisGroupFilterResult result;
	result.document.name = root.name;

	for ( const KeyValueNode &block : root.children )
	{
		if ( block.name == "world" )
		{
			KeyValueNode worldOut;
			worldOut.name = block.name;
			worldOut.pairs = block.pairs;
			for ( const KeyValueNode &child : block.children )
			{
				if ( child.name == "solid" )
				{
					if ( ObjectHidden( child, hidden ) )
					{
						++result.hiddenObjects;
						continue;
					}
					++result.visibleObjects;
				}
				worldOut.children.push_back( child );
			}
			result.document.children.push_back( std::move( worldOut ) );
		}
		else if ( block.name == "entity" )
		{
			if ( ObjectHidden( block, hidden ) )
			{
				++result.hiddenObjects;
				continue;
			}
			++result.visibleObjects;
			result.document.children.push_back( block );
		}
		else
		{
			result.document.children.push_back( block );
		}
	}

	return result;
}

} // namespace hammer::formats
