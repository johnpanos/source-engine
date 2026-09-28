//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The frontend's RenderMaterialBlocks001 (RFC 0016 K4); private to
//			render.legacy-frontend. The frontend holds it by value: IAppSystem
//			has no virtual destructor.
//
//=============================================================================//

#ifndef RENDER_LEGACY_FRONTEND_MATERIAL_BLOCKS_H
#define RENDER_LEGACY_FRONTEND_MATERIAL_BLOCKS_H

#include "render/legacy/material_blocks.h"
#include "render/material/registry.h"

namespace render::legacy
{

class FrontendMaterialBlocks final : public CBaseAppSystem<IRenderMaterialBlocks>
{
public:
	FrontendMaterialBlocks();

	int FormatBlock( const char *material, const char *pass, const char *shader, int count,
	    const char *const *keys, const char *const *values, char *out, int size ) override;

private:
	material::FamilyRegistry m_Registry; // the built-in mapping's families
};

} // namespace render::legacy

#endif // RENDER_LEGACY_FRONTEND_MATERIAL_BLOCKS_H
