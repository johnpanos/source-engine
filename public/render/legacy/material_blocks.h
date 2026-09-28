//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: RenderMaterialBlocks001 (RFC 0016 K4 "Proxy corpus", the
//			frontend side): a bound material's variables written into its
//			render.material family's parameter block, by the importer's own
//			mapping (MapVariables, ApplyValues), and reported as one JSON line
//			so a capture can compare the block with the legacy IMaterialVar
//			values the material's proxies wrote. The launcher adds it as an app
//			system beside RenderStageMarkers001; a product without the render
//			core has no such system.
//
//			The caller formats each defined variable as the material system
//			prints a VMT value: floats and vector components with %.9g, a
//			vector or matrix in brackets, an integer, a texture's or material's
//			name, or the string.
//
//			A preserved ABI package (architecture/modules.json legacyAbi):
//			C++11 and plain C types only.
//
//=============================================================================//

#ifndef RENDER_LEGACY_MATERIAL_BLOCKS_H
#define RENDER_LEGACY_MATERIAL_BLOCKS_H

#include "appframework/IAppSystem.h"

#define RENDER_MATERIAL_BLOCKS_INTERFACE_VERSION "RenderMaterialBlocks001"

class IRenderMaterialBlocks : public IAppSystem
{
public:
	// Maps `count` variables (keys with their '$', values as above) of a
	// material of `shader` into its family's block and writes
	//   {"kind":"block","name":...,"pass":...,"family":...,"params":[...],
	//    "unmapped":[...]}
	// to `out` (NUL-terminated, truncated to `size`). Returns the line's
	// length without the terminator, which may exceed `size`; 0 for a material
	// of the legacy family (no block, nothing written); -1 for a shader no
	// family or legacy port knows.
	virtual int FormatBlock( const char *material, const char *pass, const char *shader, int count,
	    const char *const *keys, const char *const *values, char *out, int size ) = 0;
};

#endif // RENDER_LEGACY_MATERIAL_BLOCKS_H
