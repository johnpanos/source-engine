//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The set of currently-selected editor objects (RFC 0002, hammer.app).
//			This is the small piece of *view* state a UI shell owns -- which
//			top-level objects the user has picked -- expressed as an ordered,
//			deduplicated set of entity indices into the document (see
//			EditorDocument::EntityCount). It is deliberately a plain value type
//			with no editor policy: the aggregation and mutation POLICY that
//			consumes a selection lives in EditorDocument, so both sibling shells
//			(hammer.adapters.gtk and hammer.adapters.mfc) drive the SAME entity
//			editor logic instead of each re-deriving multi-selection behavior.
//
//			Dependency-free apart from the C++ standard library: no MFC, tier0,
//			PCH, or GPU. Indices are opaque object identifiers here; whether a
//			given index is still in range is decided by the document that owns
//			the objects, not by this container.
//
//=============================================================================//

#ifndef HAMMER_APP_ENTITY_SELECTION_H
#define HAMMER_APP_ENTITY_SELECTION_H

#include <cstddef>
#include <vector>

namespace hammer::app
{

// An unordered set of entity indices, kept sorted ascending and unique so that
// two selections built by different click orders compare equal and iterate
// deterministically. Order in which the user clicked is not semantically
// meaningful for property aggregation, so it is intentionally not preserved.
class EntitySelection
{
public:
	EntitySelection() = default;

	// Adds 'index'. Returns true iff it was newly added (false if already present).
	bool Add( std::size_t index );

	// Removes 'index'. Returns true iff it was present.
	bool Remove( std::size_t index );

	// Toggles membership of 'index'. Returns the new membership state.
	bool Toggle( std::size_t index );

	// Drops every index.
	void Clear();

	bool Contains( std::size_t index ) const;
	bool Empty() const { return m_indices.empty(); }
	std::size_t Size() const { return m_indices.size(); }

	// Selected indices, sorted ascending and unique. Stable to iterate.
	const std::vector<std::size_t> &Indices() const { return m_indices; }

	friend bool operator==( const EntitySelection &a, const EntitySelection &b )
	{
		return a.m_indices == b.m_indices;
	}

private:
	std::vector<std::size_t> m_indices; // sorted ascending, unique
};

} // namespace hammer::app

#endif // HAMMER_APP_ENTITY_SELECTION_H
