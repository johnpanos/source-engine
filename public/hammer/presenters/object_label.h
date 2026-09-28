//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The one labeling rule for map objects in hammer.presenters (RFC
//			0002, "DRY"): the outliner rows, the inspector's connection rows and
//			the status bar name objects the same way.
//
//			  entity: its targetname, or its classname when unnamed
//			  solid:  "solid <vmfId>"
//			  group:  "group <vmfId>"
//			  unknown or stale id: ""
//
//			And the one I/O target rule: a connection target names an entity when
//			it matches the entity's targetname (scene::NameMatches: case-insensitive
//			with a trailing '*' wildcard) or equals its classname ignoring case
//			(Source's entity I/O accepts class names); procedural targets ("!self",
//			"!activator", "!caller", "!player", ...) start with '!' and are always
//			considered valid.
//
//=============================================================================//

#ifndef HAMMER_PRESENTERS_OBJECT_LABEL_H
#define HAMMER_PRESENTERS_OBJECT_LABEL_H

#include "hammer/scene/map_document.h"

#include <string>
#include <string_view>

namespace hammer::presenters
{

std::string ObjectLabel( const scene::DocumentReader &doc, scene::ObjectId id );

// True when connection target 'target' names 'entity' (see above; a
// procedural '!' target names no particular entity and returns false here).
bool TargetNamesEntity( std::string_view target, const scene::Entity &entity );

// True when 'target' is procedural ('!' prefix).
bool IsProceduralTarget( std::string_view target );

} // namespace hammer::presenters

#endif // HAMMER_PRESENTERS_OBJECT_LABEL_H
