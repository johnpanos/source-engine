//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless object-group resolution for the Hammer editor (RFC 0002,
//			hammer.formats). Hammer groups let several objects be selected and
//			transformed as one; a VMF stores `group` blocks (each with an id, and
//			possibly a parent group via its editor.groupid) in the world, and each
//			solid records its group in `editor { groupid N }`. Groups nest, and
//			selecting a member selects its whole TOP-LEVEL group. This strict,
//			MFC-free core parses the group hierarchy and expands a selection to its
//			group siblings, which is what the shell's group-select behavior needs.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_GROUPS_H
#define HAMMER_FORMATS_GROUPS_H

#include "hammer/formats/keyvalues.h"

#include <map>
#include <vector>

namespace hammer::formats
{

// The parsed group hierarchy: each solid's direct group and each group's parent.
struct GroupTable
{
	std::map<int, int> solidGroup;  // solid id -> its direct group id
	std::map<int, int> groupParent; // group id -> parent group id (0 = top-level)

	// The top-level group a solid belongs to (0 when it is ungrouped). Follows the
	// parent chain with a cycle guard.
	int TopLevelGroupOfSolid( int solidId ) const;
	// The top-level ancestor of a group id (the id itself when it has no parent).
	int TopLevelGroup( int groupId ) const;
};

// Builds the group table from a parsed VMF document.
GroupTable ParseGroups( const KeyValueNode &root );

// Expands 'selectedSolidIds' to include every world solid sharing a top-level
// group with any selected solid. Ungrouped selected solids bring in only
// themselves. The result is sorted and de-duplicated.
std::vector<int> ExpandGroupSelection(
    const KeyValueNode &root, const std::vector<int> &selectedSolidIds );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_GROUPS_H
