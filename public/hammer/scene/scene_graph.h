//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Minimal scene node graph and its mutation/query boundary (RFC 0002,
//			hammer.scene H1 seam). Nodes are addressed only by NodeHandle, so a
//			stale reference is rejected rather than dereferenced. Reparenting
//			validates document ownership, self-parenting, and cycles BEFORE
//			changing either side of the relationship; on failure the graph is
//			unchanged. Delete removes a node and its whole subtree atomically,
//			invalidating every affected handle. Dependency-free apart from the
//			C++ standard library and hammer.scene handles.
//
//=============================================================================//

#ifndef HAMMER_SCENE_SCENE_GRAPH_H
#define HAMMER_SCENE_SCENE_GRAPH_H

#include "hammer/scene/handle.h"
#include "hammer/scene/handle_table.h"

#include <cstdint>
#include <vector>

namespace hammer::scene
{

// Explicit outcome of a reparent request. Every failure leaves the graph
// unchanged; success is the only mutating outcome.
enum class ReparentResult
{
	kOk,
	kInvalidChild,    // child handle is stale/foreign
	kInvalidParent,   // parent handle is non-null but stale/foreign
	kForeignDocument, // parent belongs to a different document than the graph
	kSelfParent,      // child == parent
	kWouldCycle,      // parent is the child or one of its descendants
};

class SceneGraph
{
public:
	explicit SceneGraph( std::uint32_t documentId );

	// Creates a new root node and returns its handle.
	NodeHandle CreateNode();

	bool IsValid( const NodeHandle &node ) const;

	// Sets (or clears, when parent is the invalid handle) a node's parent.
	// Validates fully before mutating; on any failure the graph is unchanged.
	ReparentResult SetParent( const NodeHandle &child, const NodeHandle &parent );

	// Convenience for provider-parameterized conformance: true iff SetParent
	// succeeded.
	bool TrySetParent( const NodeHandle &child, const NodeHandle &parent )
	{
		return SetParent( child, parent ) == ReparentResult::kOk;
	}

	// Removes a node and its entire subtree atomically, invalidating every
	// affected handle. Returns false (no change) if the node is invalid.
	bool Remove( const NodeHandle &node );

	// The node's parent, or the invalid handle when it is a root or invalid.
	NodeHandle GetParent( const NodeHandle &node ) const;

	// The node's direct children (empty when the node is invalid).
	const std::vector<NodeHandle> &Children( const NodeHandle &node ) const;

	// True if 'ancestor' is 'node' itself or an ancestor of it.
	bool IsAncestor( const NodeHandle &ancestor, const NodeHandle &node ) const;

	std::size_t NodeCount() const;

	std::uint32_t DocumentId() const { return m_table.DocumentId(); }

private:
	struct Record
	{
		NodeHandle self;
		NodeHandle parent;
		std::vector<NodeHandle> children;
	};

	// Returns the live record index for a valid handle, or -1.
	std::int64_t RecordIndex( const NodeHandle &node ) const;

	void DetachFromParent( const NodeHandle &child );
	void CollectSubtree( const NodeHandle &node, std::vector<NodeHandle> &out ) const;

	HandleTable m_table;
	std::vector<Record> m_records;
	static const std::vector<NodeHandle> s_noChildren;
};

} // namespace hammer::scene

#endif // HAMMER_SCENE_SCENE_GRAPH_H
