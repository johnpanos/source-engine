//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The scene's surface-program terms shared by render_lab and the
//          product world pass (RFC 0016 K11-K12). The caller reports what its
//          stage and view actually provide; this policy selects one variant.
//
//=============================================================================//

#ifndef RENDER_MATERIAL_SCENE_TERMS_H
#define RENDER_MATERIAL_SCENE_TERMS_H

#include "render/material/surface_program.h"

#include <cstdint>

namespace render::material
{

struct SceneTermInputs
{
	bool runtimeDirect = false;
	bool indirectLightmap = false;
	bool totalDirectionalLightmap = false;
	bool indirectDirectionalLightmap = false;
	bool probeVolume = false;
	bool probeBounce = false;
	bool reflectionProbes = false;
	bool ambientOcclusion = false;
};

// Runtime direct light needs the bake's indirect layer. Its directional
// basis, when present, belongs to that layer rather than the total page.
// A bounce atlas has no meaning without the probe volume it changes.
constexpr std::uint32_t SceneTerms( const SceneTermInputs &inputs )
{
	std::uint32_t terms = kSurfaceClustered;
	const bool runtimeDirect = inputs.runtimeDirect && inputs.indirectLightmap;
	if ( runtimeDirect )
		terms |= kSurfaceRuntimeDirect;
	if ( runtimeDirect ? inputs.indirectDirectionalLightmap : inputs.totalDirectionalLightmap )
		terms |= kSurfaceDirectionalLightmap;
	if ( inputs.probeVolume )
		terms |= kSurfaceProbeVolume;
	if ( inputs.probeVolume && inputs.probeBounce )
		terms |= kSurfaceProbeBounce;
	if ( inputs.reflectionProbes )
		terms |= kSurfaceReflectionProbes;
	if ( inputs.ambientOcclusion )
		terms |= kSurfaceAmbientOcclusion;
	return terms;
}

} // namespace render::material

#endif // RENDER_MATERIAL_SCENE_TERMS_H
