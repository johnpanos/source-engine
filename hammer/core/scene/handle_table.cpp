//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the generational scene handle table (RFC 0002).
//			No tier0/MFC/PCH dependencies; compiles on the headless core profile.
//
//=============================================================================//

#include "hammer/scene/handle_table.h"

namespace hammer::scene
{

HandleTable::HandleTable( std::uint32_t documentId ) : m_documentId( documentId )
{
}

NodeHandle HandleTable::Allocate()
{
	// A table with no document identity cannot issue a valid handle.
	if ( m_documentId == 0 )
	{
		return kInvalidNodeHandle;
	}

	std::uint32_t index;
	if ( !m_freeList.empty() )
	{
		index = m_freeList.back();
		m_freeList.pop_back();
	}
	else
	{
		index = static_cast<std::uint32_t>( m_slots.size() );
		m_slots.push_back( Slot{} );
	}

	Slot &slot = m_slots[index];
	slot.alive = true;
	++m_liveCount;

	NodeHandle handle;
	handle.document = m_documentId;
	handle.index = index;
	handle.generation = slot.generation;
	return handle;
}

bool HandleTable::Free( const NodeHandle &handle )
{
	if ( !IsValid( handle ) )
	{
		return false;
	}

	Slot &slot = m_slots[handle.index];
	slot.alive = false;
	// Bump the generation so every handle previously issued for this slot is now
	// stale. Skip 0 to keep it distinct from a never-used generation.
	++slot.generation;
	if ( slot.generation == 0 )
	{
		slot.generation = 1;
	}
	m_freeList.push_back( handle.index );
	--m_liveCount;
	return true;
}

bool HandleTable::IsValid( const NodeHandle &handle ) const
{
	if ( handle.document != m_documentId || m_documentId == 0 )
	{
		return false;
	}
	if ( handle.index >= m_slots.size() )
	{
		return false;
	}
	const Slot &slot = m_slots[handle.index];
	return slot.alive && slot.generation == handle.generation;
}

std::size_t HandleTable::LiveCount() const
{
	return m_liveCount;
}

} // namespace hammer::scene
