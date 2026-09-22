//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of the editor selection value type (RFC 0002). No
//			tier0/MFC/PCH dependencies; headless-core profile.
//
//=============================================================================//

#include "hammer/app/entity_selection.h"

#include <algorithm>

namespace hammer::app
{

bool EntitySelection::Add( std::size_t index )
{
	const auto at = std::lower_bound( m_indices.begin(), m_indices.end(), index );
	if ( at != m_indices.end() && *at == index )
	{
		return false; // already selected
	}
	m_indices.insert( at, index );
	return true;
}

bool EntitySelection::Remove( std::size_t index )
{
	const auto at = std::lower_bound( m_indices.begin(), m_indices.end(), index );
	if ( at == m_indices.end() || *at != index )
	{
		return false; // not selected
	}
	m_indices.erase( at );
	return true;
}

bool EntitySelection::Toggle( std::size_t index )
{
	if ( Remove( index ) )
	{
		return false; // was selected, now cleared
	}
	Add( index );
	return true; // now selected
}

void EntitySelection::Clear()
{
	m_indices.clear();
}

bool EntitySelection::Contains( std::size_t index ) const
{
	return std::binary_search( m_indices.begin(), m_indices.end(), index );
}

} // namespace hammer::app
