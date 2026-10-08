//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: the render core point a legacy frontend hands a bound material's
//			draws to (CoreMeshDraw::kind), from its shader and flags: one
//			classifier for every frontend (the native Vulkan shader API and
//			the 3DS one, RFC 0016 K8 and RFC 0026). A legacy-interop header:
//			only the frontends, which see the material system, include it.
//
//=============================================================================//

#ifndef RENDER_LEGACY_CORE_MESH_KIND_H
#define RENDER_LEGACY_CORE_MESH_KIND_H

#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "render/legacy/core_passes.h"
#include "tier1/strtools.h"

namespace render::legacy
{

inline CoreMeshKind CoreMeshKindFor( IMaterial *material )
{
	if ( material )
	{
		const char *shader = material->GetShaderName();
		// Frozen-path: hand the already-expanded rope ribbon to its core material point.
		if ( !V_stricmp( shader, "Cable" ) || !V_stricmp( shader, "Cable_DX9" ) ||
		     !V_stricmp( shader, "SplineRope" ) )
			return CoreMeshKind::kCable;
		// Frozen-path: capture moving brush surfaces after their material proxies run.
		// Frozen-path: core progress (RFC 0016 K8) - WorldVertexTransition's
		// blended displacements take the same lightmapped surface point.
		if ( !V_stricmp( shader, "LightmappedGeneric" ) ||
		     !V_stricmp( shader, "LightmappedGeneric_DX9" ) ||
		     !V_stricmp( shader, "WorldVertexTransition" ) ||
		     !V_stricmp( shader, "WorldVertexTransition_DX9" ) )
			return CoreMeshKind::kLightmappedSurface;
		if ( material->GetMaterialVarFlag( MATERIAL_VAR_DECAL ) ||
		     !V_stricmp( shader, "DecalModulate" ) || !V_stricmp( shader, "DecalModulate_dx9" ) )
			return CoreMeshKind::kDecal;
		// Frozen-path: the 2D sky box's faces (R_DrawSkyBox) to the core's unlit
		// point, which decodes the Sky shader's HDR encodings (RFC 0016 K8).
		if ( !V_stricmp( shader, "UnlitGeneric" ) || !V_stricmp( shader, "UnlitTwoTexture" ) ||
		     !V_stricmp( shader, "UnlitTwoTexture_dx9" ) || !V_stricmp( shader, "Sprite" ) ||
		     !V_stricmp( shader, "Sprite_DX9" ) || !V_stricmp( shader, "Sky" ) ||
		     !V_stricmp( shader, "Sky_HDR_DX9" ) || !V_stricmp( shader, "Sky_DX9" ) )
			return CoreMeshKind::kUnlit;
		// Frozen-path: the engine's bloom chain to render.pass.post, which claims
		// it by name and draws it once in the output stage (RFC 0016 K8).
		if ( !V_stricmp( shader, "Downsample_nohdr" ) || !V_stricmp( shader, "BlurFilterX" ) ||
		     !V_stricmp( shader, "BlurFilterY" ) || !V_stricmp( shader, "Engine_Post" ) ||
		     !V_stricmp( shader, "Engine_Post_dx9" ) || !V_stricmp( shader, "MotionBlur" ) ||
		     !V_stricmp( shader, "MotionBlur_dx9" ) ||
		     ( !V_stricmp( material->GetName(), "dev/bloomadd" ) &&
		         ( !V_stricmp( shader, "screenspace_general" ) ||
		             !V_stricmp( shader, "screenspace_general_dx9" ) ) ) )
			return CoreMeshKind::kScreenEffect;
		// Frozen-path: core progress (RFC 0016 K8) - the render-to-texture blob
		// shadows: casters into the page, decals from it.
		if ( !V_stricmp( shader, "Shadow" ) || !V_stricmp( shader, "Shadow_DX9" ) ||
		     !V_stricmp( shader, "ShadowBuild" ) || !V_stricmp( shader, "ShadowBuild_DX9" ) )
			return CoreMeshKind::kBlobShadow;
		// Frozen-path: core progress (RFC 0016 K8) - SolidEnergy to the core's
		// energy point.
		if ( !V_stricmp( shader, "SolidEnergy" ) || !V_stricmp( shader, "SolidEnergy_dx9" ) )
			return CoreMeshKind::kEnergy;
		// Frozen-path: core progress (RFC 0016 K8 particles) - SpriteCard's
		// card records to the core's sprite card point.
		if ( !V_stricmp( shader, "Spritecard" ) || !V_stricmp( shader, "Spritecard_DX8" ) )
			return CoreMeshKind::kParticle;
		if ( !V_stricmp( shader, "Refract" ) || !V_stricmp( shader, "Refract_DX90" ) )
			return CoreMeshKind::kTransmission;
		// Frozen-path: core progress (RFC 0016 surface model) - P2:CE's PBR
		// model surfaces (a Workshop view model, props) to the core's pbr point.
		if ( !V_stricmp( shader, "VertexLitGeneric" ) ||
		     !V_stricmp( shader, "VertexLitGeneric_DX9" ) || !V_stricmp( shader, "PBR" ) ||
		     !V_stricmp( shader, "EyeRefract" ) || !V_stricmp( shader, "EyeRefract_dx9" ) )
			return CoreMeshKind::kModelSurface;
		if ( !V_stricmp( shader, "PortalRefract" ) || !V_stricmp( shader, "PortalRefract_dx9" ) )
		{
			bool found = false;
			IMaterialVar *stage = material->FindVar( "$stage", &found, false );
			if ( found && stage->GetIntValue() == 1 )
				return CoreMeshKind::kDepthMask;
			// Frozen-path: core progress (RFC 0016 K8) - the refraction and
			// flame stages to the core's portal point.
			return CoreMeshKind::kPortal;
		}
		if ( !V_stricmp( shader, "PortalStaticOverlay" ) )
			return CoreMeshKind::kPortal;
		if ( !V_stricmp( shader, "Portal" ) || !V_stricmp( shader, "Portal_DX90" ) )
			return CoreMeshKind::kPortal;
		if ( !V_stricmp( shader, "WriteZ" ) || !V_stricmp( shader, "WriteZ_DX9" ) )
			return CoreMeshKind::kDepthMask;
		if ( !V_stricmp( shader, "BufferClearObeyStencil" ) ||
		     !V_stricmp( shader, "BufferClearObeyStencil_DX9" ) )
			return CoreMeshKind::kStencilClear;
	}
	return CoreMeshKind::kSurface;
}

} // namespace render::legacy

#endif // RENDER_LEGACY_CORE_MESH_KIND_H
