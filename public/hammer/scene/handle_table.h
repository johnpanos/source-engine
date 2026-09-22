//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Generational slot table that issues and validates NodeHandles for one
//			document (RFC 0002, hammer.scene). Freeing a slot bumps its generation
//			so every previously-issued handle to that slot becomes stale. Handles
//			from a different document are rejected. This is the mechanism behind
//			the RFC's "stale-reference check" and "independent documents"
//			requirements. Dependency-free apart from the C++ standard library.
//
//=============================================================================//

#ifndef HAMMER_SCENE_HANDLE_TABLE_H
#define HAMMER_SCENE_HANDLE_TABLE_H

#include "hammer/scene/handle.h"

#include <cstdint>
#include <vector>

namespace hammer::scene
{

class HandleTable
{
public:
	// documentId identifies this table's document and is stamped into every
	// handle. It must be non-zero (0 is reserved for the invalid handle). A zero
	// id makes the table issue only invalid handles; callers must supply a real
	// id from the session's document-identity owner.
	explicit HandleTable( std::uint32_t documentId );

	// Allocates a slot and returns a fresh valid handle, reusing a freed slot
	// when one is available (with an incremented generation).
	NodeHandle Allocate();

	// Frees the slot a valid handle refers to, invalidating that handle and any
	// copy of it. Returns false if the handle is already stale or foreign; in
	// that case nothing changes.
	bool Free( const NodeHandle &handle );

	// True only if the handle was issued by this table, still refers to a live
	// slot, and matches that slot's current generation.
	bool IsValid( const NodeHandle &handle ) const;

	// Number of currently-live slots.
	std::size_t LiveCount() const;

	std::uint32_t DocumentId() const { return m_documentId; }

private:
	struct Slot
	{
		std::uint32_t generation = 1; // live generations are >= 1
		bool alive = false;
	};

	std::uint32_t m_documentId;
	std::vector<Slot> m_slots;
	std::vector<std::uint32_t> m_freeList;
	std::size_t m_liveCount = 0;
};

} // namespace hammer::scene

#endif // HAMMER_SCENE_HANDLE_TABLE_H
