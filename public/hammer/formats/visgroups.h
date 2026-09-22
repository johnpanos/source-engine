//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Headless VisGroup parsing + visibility filtering for the Hammer editor
//			(RFC 0002, hammer.formats). VisGroups organize map objects into named,
//			possibly nested groups the user can hide/show; a VMF stores the group
//			tree in a top-level "visgroups" block and each object's membership in
//			its "editor { visgroupid N }" sub-block. This strict, MFC-free core
//			parses the tree and computes which objects are visible given a set of
//			hidden groups, so a shell can drive the classic VisGroup checkbox list.
//
//			Hiding a group hides its descendants (the hidden set is expanded down
//			the tree). An object is hidden iff any of its visgroup memberships is in
//			the expanded hidden set; an object with no membership is always visible.
//
//=============================================================================//

#ifndef HAMMER_FORMATS_VISGROUPS_H
#define HAMMER_FORMATS_VISGROUPS_H

#include "hammer/formats/keyvalues.h"

#include <cstddef>
#include <vector>

namespace hammer::formats
{

// One VisGroup: its id, name, and nested child groups.
struct VisGroup
{
	int id = 0;
	std::string name;
	std::vector<VisGroup> children;
};

// Parses the top-level "visgroups" block into the group tree (empty if absent).
std::vector<VisGroup> ParseVisGroups( const KeyValueNode &root );

struct VisGroupFilterResult
{
	KeyValueNode document;          // the document with hidden objects removed
	std::size_t visibleObjects = 0; // world solids + entities kept
	std::size_t hiddenObjects = 0;  // world solids + entities removed
};

// Returns 'root' with every world solid and entity whose visgroup membership falls
// in 'hiddenGroupIds' (expanded to include descendant groups per the tree) removed.
// Objects with no membership are kept. Non-world/entity blocks are preserved.
VisGroupFilterResult ApplyVisGroupFilter(
    const KeyValueNode &root, const std::vector<int> &hiddenGroupIds );

} // namespace hammer::formats

#endif // HAMMER_FORMATS_VISGROUPS_H
