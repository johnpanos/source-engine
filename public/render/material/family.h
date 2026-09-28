//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.material.v2 families (RFC 0016). A family is a material
//			kind with its parameter schema, the bind groups it uses (at most
//			four, in the port's role order) and the device capabilities it
//			requires. Families register at composition; materials are
//			instances of a family with a parameter block.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_FAMILY_H
#define RENDER_MATERIAL_FAMILY_H

#include "foundation/strong_id.h"
#include "render/device/facts.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace render::material
{

enum class ParameterType : std::uint8_t
{
	kFloat,
	kFloat2,
	kFloat3,
	kFloat4,
	kInt,
	kTexture,  // bound through the material group, not the uniform block
	kTransform // a texture transform: rows 0 and 1 of the 4x4 (8 floats), as shaders read it
};

struct ParameterDesc
{
	std::string name;
	ParameterType type = ParameterType::kFloat;
	float defaults[8] = {}; // a transform's two rows; other types use the first four
};

struct FamilyDesc
{
	std::string name;
	std::vector<ParameterDesc> parameters;
	std::uint32_t bindGroups = 0; // how many of the four roles the family binds
	device::CapabilitySet required;
};

using FamilyId = foundation::StrongId<struct FamilyTag, std::uint32_t>;

// Where each parameter lives in the family's uniform block. Scalars take 4
// bytes and vectors align to their size rounded to 8 or 16, as std140 does;
// textures take a slot instead.
struct ParameterLayout
{
	ParameterType type = ParameterType::kFloat;
	std::uint32_t offset = 0; // byte offset, or texture slot for kTexture
};

struct FamilySchema
{
	FamilyDesc desc;
	std::vector<ParameterLayout> layout; // one per desc.parameters entry
	std::uint32_t blockSize = 0;         // bytes, a multiple of 16
	std::uint32_t textureSlots = 0;

	std::optional<std::size_t> IndexOf( std::string_view name ) const;
};

// The first capability the family requires that facts lack.
std::optional<device::Capability> MissingCapability(
    const FamilySchema &family, const device::DeviceFacts &facts );

} // namespace render::material

#endif // RENDER_MATERIAL_FAMILY_H
