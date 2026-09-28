//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.material.v2 family registry (RFC 0016). The composition
//			root registers the families a product ships; registration validates
//			the schema and fixes its layout.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_REGISTRY_H
#define RENDER_MATERIAL_REGISTRY_H

#include "foundation/expected.h"
#include "render/material/family.h"

#include <cstdint>
#include <deque>
#include <string_view>

namespace render::material
{

enum class MaterialStatus : std::uint8_t
{
	kDuplicateFamily = 1,
	kDuplicateParameter,
	kTooManyBindGroups,
	kUnknownFamily,
	kUnknownParameter,
	kTypeMismatch,
	kInvalidDescription
};

struct MaterialError
{
	MaterialStatus status = MaterialStatus::kInvalidDescription;
};

class FamilyRegistry
{
public:
	foundation::Expected<FamilyId, MaterialError> Register( const FamilyDesc &desc );
	const FamilySchema *Find( FamilyId id ) const;
	const FamilySchema *Find( std::string_view name ) const;
	std::optional<FamilyId> IdOf( std::string_view name ) const;
	std::size_t Count() const { return m_Families.size(); }

private:
	// FamilyId value = index + 1. A deque, so schemas never move: parameter
	// blocks point at theirs.
	std::deque<FamilySchema> m_Families;
};

} // namespace render::material

#endif // RENDER_MATERIAL_REGISTRY_H
