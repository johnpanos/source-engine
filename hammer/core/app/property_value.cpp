//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of multi-selection property value aggregation
//			(RFC 0002). No tier0/MFC/PCH dependencies; headless-core profile.
//
//=============================================================================//

#include "hammer/app/property_value.h"

#include <utility>

namespace hammer::app
{

PropertyValue PropertyValue::Single( std::string value )
{
	PropertyValue result;
	result.m_state = PropertyState::kSingle;
	result.m_value = std::move( value );
	return result;
}

PropertyValue PropertyValue::Mixed()
{
	PropertyValue result;
	result.m_state = PropertyState::kMixed;
	return result;
}

PropertyValue PropertyValue::AddContributor( const std::string &value ) const
{
	switch ( m_state )
	{
	case PropertyState::kUnset:
		return Single( value );
	case PropertyState::kSingle:
		// Empty string agrees with empty string; only a genuine difference mixes.
		return ( m_value == value ) ? *this : Mixed();
	case PropertyState::kMixed:
		return *this;
	}
	return *this;
}

} // namespace hammer::app
