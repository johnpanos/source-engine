//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of hammer::formats object-group resolution. See
//			public/hammer/formats/groups.h.
//
//=============================================================================//

#include "hammer/formats/groups.h"

#include <algorithm>
#include <set>
#include <string>

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

// Reads "groupid" from an object's "editor" sub-block (0 when absent/malformed).
int EditorGroupId( const KeyValueNode &object )
{
	for ( const KeyValueNode &child : object.children )
	{
		if ( child.name != "editor" )
		{
			continue;
		}
		if ( const std::string *g = child.Find( "groupid" ) )
		{
			int id = 0;
			if ( ParseInt( *g, id ) )
			{
				return id;
			}
		}
	}
	return 0;
}

int ObjectId( const KeyValueNode &object )
{
	if ( const std::string *id = object.Find( "id" ) )
	{
		int value = 0;
		if ( ParseInt( *id, value ) )
		{
			return value;
		}
	}
	return 0;
}

} // namespace

int GroupTable::TopLevelGroup( int groupId ) const
{
	if ( groupId == 0 )
	{
		return 0;
	}
	std::set<int> visited;
	int current = groupId;
	while ( true )
	{
		if ( visited.count( current ) != 0 )
		{
			return current; // cycle guard: treat the cycle entry as top-level
		}
		visited.insert( current );
		auto it = groupParent.find( current );
		if ( it == groupParent.end() || it->second == 0 )
		{
			return current;
		}
		current = it->second;
	}
}

int GroupTable::TopLevelGroupOfSolid( int solidId ) const
{
	auto it = solidGroup.find( solidId );
	if ( it == solidGroup.end() || it->second == 0 )
	{
		return 0;
	}
	return TopLevelGroup( it->second );
}

GroupTable ParseGroups( const KeyValueNode &root )
{
	GroupTable table;
	for ( const KeyValueNode &block : root.children )
	{
		if ( block.name != "world" )
		{
			continue;
		}
		for ( const KeyValueNode &child : block.children )
		{
			if ( child.name == "group" )
			{
				const int id = ObjectId( child );
				if ( id != 0 )
				{
					table.groupParent[id] = EditorGroupId( child );
				}
			}
			else if ( child.name == "solid" )
			{
				const int id = ObjectId( child );
				if ( id != 0 )
				{
					table.solidGroup[id] = EditorGroupId( child );
				}
			}
		}
	}
	return table;
}

std::vector<int> ExpandGroupSelection(
    const KeyValueNode &root, const std::vector<int> &selectedSolidIds )
{
	const GroupTable table = ParseGroups( root );

	// Top-level groups implicated by the selection.
	std::set<int> selectedGroups;
	for ( int sid : selectedSolidIds )
	{
		const int top = table.TopLevelGroupOfSolid( sid );
		if ( top != 0 )
		{
			selectedGroups.insert( top );
		}
	}

	std::set<int> result( selectedSolidIds.begin(), selectedSolidIds.end() );
	for ( const auto &entry : table.solidGroup )
	{
		const int top = table.TopLevelGroupOfSolid( entry.first );
		if ( top != 0 && selectedGroups.count( top ) != 0 )
		{
			result.insert( entry.first );
		}
	}

	return std::vector<int>( result.begin(), result.end() );
}

} // namespace hammer::formats
