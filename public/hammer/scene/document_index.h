//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A derived index over a map document or staged edit (RFC 0002,
//			hammer.scene; "Derived caches carry revisions/epochs"). The
//			structural queries in map_queries.h scan the document, which is
//			right for one question and quadratic for a pass that asks it of
//			every object (the map check, the outliner). Such passes build this
//			index once and ask it instead.
//
//			The index answers exactly what the scanning queries answer (the
//			suite checks them against each other); it is a snapshot, valid until
//			the reader changes. Callers that keep one across edits key it by the
//			session revision.
//
//=============================================================================//

#ifndef HAMMER_SCENE_DOCUMENT_INDEX_H
#define HAMMER_SCENE_DOCUMENT_INDEX_H

#include "hammer/scene/map_document.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace hammer::scene
{

class DocumentIndex
{
public:
	explicit DocumentIndex( const DocumentReader &doc );

	// As scene::EntitySolids and scene::GroupMembers (id order).
	const std::vector<ObjectId> &EntitySolids( ObjectId entity ) const;
	const std::vector<ObjectId> &GroupMembers( ObjectId group ) const;

	// As scene::FindEntitiesByName: case-insensitive, trailing '*' matches a
	// prefix; empty for an empty pattern (id order).
	std::vector<ObjectId> EntitiesNamed( std::string_view pattern ) const;
	bool AnyEntityNamed( std::string_view pattern ) const;

	// Every object of 'kind' with persistent id 'vmfId' (id order).
	const std::vector<ObjectId> &WithVmfId( ObjectKind kind, std::uint32_t vmfId ) const;
	// Every solid with a side whose persistent id is 'sideId' (id order; a
	// solid appears once per such side).
	const std::vector<ObjectId> &SolidsWithSide( std::uint32_t sideId ) const;

private:
	std::map<ObjectId, std::vector<ObjectId>> m_entitySolids;
	std::map<ObjectId, std::vector<ObjectId>> m_groupMembers;
	std::map<std::string, std::vector<ObjectId>> m_names; // lower-cased targetname
	std::map<std::pair<int, std::uint32_t>, std::vector<ObjectId>> m_vmfIds;
	std::map<std::uint32_t, std::vector<ObjectId>> m_sideIds;
	std::vector<ObjectId> m_none;
};

} // namespace hammer::scene

#endif // HAMMER_SCENE_DOCUMENT_INDEX_H
