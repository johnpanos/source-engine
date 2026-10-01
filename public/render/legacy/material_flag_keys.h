//========= Copyright Valve Corporation, All rights reserved. ============//
#ifndef RENDER_LEGACY_MATERIAL_FLAG_KEYS_H
#define RENDER_LEGACY_MATERIAL_FLAG_KEYS_H
#include "materialsystem/imaterial.h"
namespace RenderLegacyMaterialFlags
{
struct MaterialFlagKey
{
	MaterialVarFlags_t flag;
	const char *key;
};
inline constexpr MaterialFlagKey Keys[] = {
    { MATERIAL_VAR_NO_DRAW, "$no_draw" },
    { MATERIAL_VAR_VERTEXCOLOR, "$vertexcolor" },
    { MATERIAL_VAR_VERTEXALPHA, "$vertexalpha" },
    { MATERIAL_VAR_SELFILLUM, "$selfillum" },
    { MATERIAL_VAR_ADDITIVE, "$additive" },
    { MATERIAL_VAR_ALPHATEST, "$alphatest" },
    { MATERIAL_VAR_MULTIPASS, "$multipass" },
    { MATERIAL_VAR_ZNEARER, "$znearer" },
    { MATERIAL_VAR_MODEL, "$model" },
    { MATERIAL_VAR_FLAT, "$flat" },
    { MATERIAL_VAR_NOCULL, "$nocull" },
    { MATERIAL_VAR_NOFOG, "$nofog" },
    { MATERIAL_VAR_IGNOREZ, "$ignorez" },
    { MATERIAL_VAR_DECAL, "$decal" },
    { MATERIAL_VAR_ENVMAPSPHERE, "$envmapsphere" },
    { MATERIAL_VAR_NOALPHAMOD, "$noalphamod" },
    { MATERIAL_VAR_ENVMAPCAMERASPACE, "$envmapcameraspace" },
    { MATERIAL_VAR_BASEALPHAENVMAPMASK, "$basealphaenvmapmask" },
    { MATERIAL_VAR_TRANSLUCENT, "$translucent" },
    { MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK, "$normalmapalphaenvmapmask" },
    { MATERIAL_VAR_ENVMAPMODE, "$envmapmode" },
    { MATERIAL_VAR_HALFLAMBERT, "$halflambert" },
    { MATERIAL_VAR_WIREFRAME, "$wireframe" },
    { MATERIAL_VAR_ALLOWALPHATOCOVERAGE, "$allowalphatocoverage" },
    { MATERIAL_VAR_IGNORE_ALPHA_MODULATION, "$ignore_alpha_modulation" },
};
} // namespace RenderLegacyMaterialFlags
#endif
