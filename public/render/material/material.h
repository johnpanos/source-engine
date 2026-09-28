//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: render.material.v2 materials (RFC 0016): an instance of a family
//			with its parameter block. VMT import (vmt_import.h) is K4 work.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_MATERIAL_H
#define RENDER_MATERIAL_MATERIAL_H

#include "foundation/strong_id.h"
#include "render/material/parameter_block.h"

#include <cstdint>
#include <string>

namespace render::material
{

using MaterialId = foundation::StrongId<struct MaterialTag, std::uint64_t>;

struct MaterialInstance
{
	MaterialId id;
	FamilyId family;
	std::string name;
	ParameterBlock parameters;
};

} // namespace render::material

#endif // RENDER_MATERIAL_MATERIAL_H
