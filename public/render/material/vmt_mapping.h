//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The one mapping from legacy VMT materials to render.material.v2
//			families (RFC 0016 "Materials", K4 "VMT corpus"). Shader rows name
//			the legacy shaders that map cleanly onto a core family, with the
//			reason; every other stdshader_dx9 shader runs in the `legacy`
//			family through the frontend's ports. Key rows map a VMT key to a
//			family parameter and say how its value is read. The families'
//			schemas are built from the key rows (FamiliesFromMapping), so the
//			importer and the families cannot disagree. The PBR rows come from
//			RFC 0007's schema (render/pbr_material_schema.h), its one owner.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_VMT_MAPPING_H
#define RENDER_MATERIAL_VMT_MAPPING_H

#include "render/material/family.h"

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace render::material
{

inline constexpr std::string_view kLegacyFamily = "legacy";

// How a VMT value is read: the legacy IMaterialVar rules.
enum class ValueKind : std::uint8_t
{
	kTexture,  // a texture name; "materials/" is prefixed, ".vtf" dropped
	kFloat,    // a number; the first component of a vector
	kFloat2,   // "[x y]", "{X Y}" (0-255) or a scalar for every component
	kFloat3,   // as kFloat2 with three components (colors)
	kFloat4,   // as kFloat2 with four components
	kInt,      // an integer; a float truncates
	kBool,     // an integer flag: nonzero is true
	kEnum,     // one of the row's names, stored as its index
	kMaterial, // another material's name (no prefix): not a GPU parameter
	// A texture transform, as the material system parses one: "[16 numbers]"
	// (row-major) or "center u v scale u v rotate degrees translate u v";
	// rows 0 and 1 are kept. The default is the identity.
	kTransform
};

struct VmtKeyRow
{
	std::string_view family;
	std::string_view key;       // lower case, with its '$'
	std::string_view parameter; // the family parameter it sets
	ValueKind kind = ValueKind::kFloat;
	// The default as VMT text ("" is zero); for kEnum the names, '|'-separated,
	// the first being the default.
	std::string_view fallback;
};

struct VmtShaderRow
{
	std::string_view shader; // lower-case legacy shader name, after fallback aliases
	std::string_view family;
	std::string_view reason;
};

// Keys read outside the renderer (physics, decals, the compile tools), kept as
// material metadata rather than reported as unmapped.
struct VmtMetadataRow
{
	std::string_view key; // lower case
	std::string_view consumer;
};

// A legacy shader name the product's shader library registers, and the name
// it falls back to (DEFINE_FALLBACK_SHADER), or empty.
struct LegacyShaderName
{
	std::string_view name;   // lower case
	std::string_view target; // lower case
};

struct VmtMappingTable
{
	std::span<const VmtShaderRow> shaders;
	std::span<const VmtKeyRow> keys;
	std::span<const VmtMetadataRow> metadata;
	std::span<const LegacyShaderName> legacyShaders;
	std::string_view legacyReason;
};

const VmtMappingTable &BuiltinVmtMapping();

// The families the key rows define, one per family named by a shader row, in
// row order. Every family binds the port's four groups; kMaterial rows are not
// GPU parameters and are left out; kInt, kBool and kEnum rows are kInt.
std::vector<FamilyDesc> FamiliesFromMapping( const VmtMappingTable &mapping );

} // namespace render::material

#endif // RENDER_MATERIAL_VMT_MAPPING_H
