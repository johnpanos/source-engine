//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Strongly-typed runtime handle for scene nodes (RFC 0002,
//			hammer.scene). A runtime handle is distinct from a persistent VMF id,
//			from history identity, and from a GPU pick id. It carries a document
//			identity plus a slot index and a generation so a stale handle -- one
//			whose slot was freed, or that belongs to a different (or reused)
//			document -- is rejected instead of silently resolving to the wrong
//			node. Dependency-free: no MFC, tier0, PCH, or GPU.
//
//=============================================================================//

#ifndef HAMMER_SCENE_HANDLE_H
#define HAMMER_SCENE_HANDLE_H

#include <cstdint>

namespace hammer::scene
{

// An opaque runtime reference to a scene node. Value type; cheap to copy and
// compare. A default-constructed handle is invalid and never resolves.
struct NodeHandle
{
	std::uint32_t document = 0;   // owning document identity (0 == none)
	std::uint32_t index = 0;      // slot index within that document's table
	std::uint32_t generation = 0; // slot generation at the time of issue

	friend constexpr bool operator==( const NodeHandle &a, const NodeHandle &b )
	{
		return a.document == b.document && a.index == b.index && a.generation == b.generation;
	}

	friend constexpr bool operator!=( const NodeHandle &a, const NodeHandle &b )
	{
		return !( a == b );
	}
};

// The invalid handle. Distinguished by document == 0, which no live table issues.
inline constexpr NodeHandle kInvalidNodeHandle{};

constexpr bool IsInvalid( const NodeHandle &handle )
{
	return handle.document == 0;
}

} // namespace hammer::scene

#endif // HAMMER_SCENE_HANDLE_H
