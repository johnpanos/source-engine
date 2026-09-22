//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the scene node graph mutation/query boundary
//			(RFC 0002). No tier0/MFC/PCH dependencies; headless-core profile.
//
//=============================================================================//

#include "hammer/scene/scene_graph.h"

#include <algorithm>

namespace hammer::scene
{

const std::vector<NodeHandle> SceneGraph::s_noChildren{};

SceneGraph::SceneGraph( std::uint32_t documentId ) : m_table( documentId ) {}

NodeHandle SceneGraph::CreateNode()
{
	const NodeHandle handle = m_table.Allocate();
	if ( IsInvalid( handle ) )
	{
		return kInvalidNodeHandle;
	}

	if ( handle.index >= m_records.size() )
	{
		m_records.resize( handle.index + 1 );
	}

	Record &record = m_records[handle.index];
	record.self = handle;
	record.parent = kInvalidNodeHandle;
	record.children.clear();
	return handle;
}

bool SceneGraph::IsValid( const NodeHandle &node ) const
{
	return m_table.IsValid( node );
}

std::int64_t SceneGraph::RecordIndex( const NodeHandle &node ) const
{
	if ( !m_table.IsValid( node ) )
	{
		return -1;
	}
	// A valid handle always has a matching record from CreateNode.
	return static_cast<std::int64_t>( node.index );
}

bool SceneGraph::IsAncestor( const NodeHandle &ancestor, const NodeHandle &node ) const
{
	if ( !IsValid( ancestor ) || !IsValid( node ) )
	{
		return false;
	}
	NodeHandle current = node;
	while ( !IsInvalid( current ) )
	{
		if ( current == ancestor )
		{
			return true;
		}
		const std::int64_t index = RecordIndex( current );
		if ( index < 0 )
		{
			break;
		}
		current = m_records[static_cast<std::size_t>( index )].parent;
	}
	return false;
}

void SceneGraph::DetachFromParent( const NodeHandle &child )
{
	const std::int64_t childIndex = RecordIndex( child );
	if ( childIndex < 0 )
	{
		return;
	}
	const NodeHandle parent = m_records[static_cast<std::size_t>( childIndex )].parent;
	const std::int64_t parentIndex = RecordIndex( parent );
	if ( parentIndex >= 0 )
	{
		std::vector<NodeHandle> &siblings = m_records[static_cast<std::size_t>( parentIndex )].children;
		siblings.erase( std::remove( siblings.begin(), siblings.end(), child ), siblings.end() );
	}
	m_records[static_cast<std::size_t>( childIndex )].parent = kInvalidNodeHandle;
}

ReparentResult SceneGraph::SetParent( const NodeHandle &child, const NodeHandle &parent )
{
	if ( !IsValid( child ) )
	{
		return ReparentResult::kInvalidChild;
	}

	// A null parent means "make this a root": always valid for a valid child.
	if ( IsInvalid( parent ) )
	{
		DetachFromParent( child );
		return ReparentResult::kOk;
	}

	if ( parent.document != m_table.DocumentId() )
	{
		return ReparentResult::kForeignDocument;
	}
	if ( !IsValid( parent ) )
	{
		return ReparentResult::kInvalidParent;
	}
	if ( child == parent )
	{
		return ReparentResult::kSelfParent;
	}
	// Reparenting under one's own descendant (or self) would form a cycle.
	if ( IsAncestor( child, parent ) )
	{
		return ReparentResult::kWouldCycle;
	}

	// All checks passed; now mutate. Detach from the old parent first.
	DetachFromParent( child );
	const std::int64_t childIndex = RecordIndex( child );
	const std::int64_t parentIndex = RecordIndex( parent );
	m_records[static_cast<std::size_t>( childIndex )].parent = parent;
	m_records[static_cast<std::size_t>( parentIndex )].children.push_back( child );
	return ReparentResult::kOk;
}

void SceneGraph::CollectSubtree( const NodeHandle &node, std::vector<NodeHandle> &out ) const
{
	const std::int64_t index = RecordIndex( node );
	if ( index < 0 )
	{
		return;
	}
	out.push_back( node );
	for ( const NodeHandle &child : m_records[static_cast<std::size_t>( index )].children )
	{
		CollectSubtree( child, out );
	}
}

bool SceneGraph::Remove( const NodeHandle &node )
{
	if ( !IsValid( node ) )
	{
		return false;
	}

	// Detach the subtree root from its parent, then free the whole subtree. The
	// gather happens before any free so handles stay resolvable during the walk.
	DetachFromParent( node );
	std::vector<NodeHandle> subtree;
	CollectSubtree( node, subtree );
	for ( const NodeHandle &handle : subtree )
	{
		const std::int64_t index = RecordIndex( handle );
		if ( index >= 0 )
		{
			m_records[static_cast<std::size_t>( index )].children.clear();
			m_records[static_cast<std::size_t>( index )].parent = kInvalidNodeHandle;
		}
		m_table.Free( handle );
	}
	return true;
}

NodeHandle SceneGraph::GetParent( const NodeHandle &node ) const
{
	const std::int64_t index = RecordIndex( node );
	if ( index < 0 )
	{
		return kInvalidNodeHandle;
	}
	return m_records[static_cast<std::size_t>( index )].parent;
}

const std::vector<NodeHandle> &SceneGraph::Children( const NodeHandle &node ) const
{
	const std::int64_t index = RecordIndex( node );
	if ( index < 0 )
	{
		return s_noChildren;
	}
	return m_records[static_cast<std::size_t>( index )].children;
}

std::size_t SceneGraph::NodeCount() const
{
	return m_table.LiveCount();
}

} // namespace hammer::scene
