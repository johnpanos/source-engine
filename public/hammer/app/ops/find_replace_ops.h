//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity search and key-value replacement (RFC 0002, hammer.app;
//			legacy CEntityReportDlg filters and CSearchReplaceDlg Find /
//			Replace All). One query value drives both.
//
//			Matching. An entity matches when every given filter matches:
//			  * class: 'classPattern' against the classname, always
//			    case-insensitive (the entity report's class filter is a
//			    case-insensitive substring: TextMatch::Contains);
//			  * key and value: some key named 'key' (case-insensitive; empty =
//			    any key) whose value matches 'valuePattern' (empty = any
//			    value, so a key alone asks "has this key"). With no 'key',
//			    each connection's target and parameter are searched too (the
//			    legacy Find dialog);
//			  * visibility ('visibleOnly', scene::IsVisible) and scope
//			    ('within': the entities the ids stand for; nothing = the
//			    whole document).
//			Text matching: Contains (substring; legacy default), Whole (the
//			entire value; legacy "whole word" and the report's "exact"),
//			Wildcard (the entire value against a glob of '*' and '?', an
//			addition). Values honor 'caseSensitive' (default off, as legacy).
//
//			Replacement (legacy FindReplace over the matched entities): each
//			matching key value, and with no 'key' each matching connection
//			target and parameter, is rewritten. Whole and Wildcard replace the
//			whole value; Contains replaces every non-overlapping occurrence of
//			the pattern (legacy replaced only the first, so a value such as
//			"a_a" needed two passes; every occurrence is a deliberate fix).
//			'count' is the number of values changed. Names are replaced as
//			text: references to a renamed entity are not followed (use
//			RenameEntity for that). Refuses an empty value pattern; nothing
//			matched or changed is Nothing.
//
//=============================================================================//

#ifndef HAMMER_APP_OPS_FIND_REPLACE_OPS_H
#define HAMMER_APP_OPS_FIND_REPLACE_OPS_H

#include "hammer/app/edit_session.h"
#include "hammer/scene/change_set.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::app::ops
{

enum class TextMatch
{
	Contains,
	Whole,
	Wildcard,
};

struct EntityQuery
{
	std::string classPattern; // empty = any class
	TextMatch classMatch = TextMatch::Contains;
	std::string key;          // empty = any key (and connections)
	std::string valuePattern; // empty = any value
	TextMatch valueMatch = TextMatch::Contains;
	bool caseSensitive = false; // values only
	bool visibleOnly = false;
	std::optional<std::vector<scene::ObjectId>> within;
};

// True when 'text' matches 'pattern' under 'mode'.
bool TextMatches( std::string_view text, std::string_view pattern, TextMatch mode, bool caseSensitive );

// The matching entities, in id order.
std::vector<scene::ObjectId> FindEntities( const scene::DocumentReader &doc, const EntityQuery &query );

EditResult ReplaceKeyValues(
    scene::DocumentEdit &edit, const EntityQuery &query, const std::string &newValue, int &count );

} // namespace hammer::app::ops

#endif // HAMMER_APP_OPS_FIND_REPLACE_OPS_H
