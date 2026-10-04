//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose: CPU-expanded Source rope ribbons in the core surface program.
//=============================================================================//
#ifndef RENDER_MATERIAL_CABLE_FAMILY_H
#define RENDER_MATERIAL_CABLE_FAMILY_H

#include "render/material/unlit_family.h"

namespace render::material
{
// UV0 samples the linear normal texture; UV1 samples the sRGB base texture.
// Vertex RGB is already linear illumination, and vertex alpha modulates coverage.
// The frozen API frontend owns ribbon expansion and the captured vertex lighting.
UnlitClaim ClaimCable( const ParameterBlock &block );
}

#endif // RENDER_MATERIAL_CABLE_FAMILY_H
