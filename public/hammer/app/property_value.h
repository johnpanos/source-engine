//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Multi-selection property value aggregation (RFC 0002, hammer.app).
//			The RFC requires that "multi-selection mixed values remain
//			distinguishable from empty strings and unset properties." This value
//			type owns exactly that distinction so selection/inspector code shares
//			one definition instead of re-deriving it (and losing an empty-vs-unset
//			case). Dependency-free apart from std::string.
//
//			Three states:
//			  Unset  - no selected object contributes this property at all
//			  Single - every contributor agrees on one value (which MAY be "")
//			  Mixed  - contributors disagree
//
//=============================================================================//

#ifndef HAMMER_APP_PROPERTY_VALUE_H
#define HAMMER_APP_PROPERTY_VALUE_H

#include <string>

namespace hammer::app
{

enum class PropertyState
{
	kUnset,
	kSingle,
	kMixed,
};

class PropertyValue
{
public:
	// A fresh aggregate contributes nothing yet.
	PropertyValue() = default;

	static PropertyValue Unset() { return PropertyValue(); }
	static PropertyValue Single( std::string value );
	static PropertyValue Mixed();

	PropertyState State() const { return m_state; }
	bool IsUnset() const { return m_state == PropertyState::kUnset; }
	bool IsSingle() const { return m_state == PropertyState::kSingle; }
	bool IsMixed() const { return m_state == PropertyState::kMixed; }

	// The agreed value; only meaningful when State() == kSingle. Empty string is a
	// legitimate single value, distinct from Unset.
	const std::string &Value() const { return m_value; }

	// Folds one more contributor's value into the aggregate:
	//   Unset  + v -> Single(v)
	//   Single(v) + v -> Single(v)          (agreement, incl. "" + "")
	//   Single(a) + b -> Mixed              (a != b)
	//   Mixed  + v -> Mixed
	PropertyValue AddContributor( const std::string &value ) const;

	friend bool operator==( const PropertyValue &a, const PropertyValue &b )
	{
		return a.m_state == b.m_state && a.m_value == b.m_value;
	}

private:
	PropertyState m_state = PropertyState::kUnset;
	std::string m_value;
};

} // namespace hammer::app

#endif // HAMMER_APP_PROPERTY_VALUE_H
