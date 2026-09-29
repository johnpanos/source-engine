//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: What a $selfillum material draws as self-illumination (RFC 0011
//          area lights): its base color times its selfillum mask
//          ($selfillummask's color, the envmap mask's alpha with
//          $selfillum_envmapmask_alpha, or the base alpha), at tint 1, the
//          rule VertexLitGeneric and LightmappedGeneric share. One rule for
//          every emitter: models (game/client/emissive_area_lights.cpp) and
//          world faces and overlays (engine/world_emitters.cpp).
//
//===========================================================================//

#ifndef SELFILLUM_EMISSION_H
#define SELFILLUM_EMISSION_H

#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "vtf/vtf_sample.h"

namespace selfillum_emission
{

// `load( textureName )` returns the texture decoded for sampling, or null.
template <typename Load>
const vtf_sample::Texture *TextureOfVar( IMaterial *pMaterial, const char *pVarName, Load &load )
{
	bool bFound = false;
	IMaterialVar *pVar = pMaterial->FindVar( pVarName, &bFound, false );
	if ( !bFound || !pVar || !pVar->IsDefined() || !pVar->IsTexture() )
		return NULL;
	ITexture *pTexture = pVar->GetTextureValue();
	if ( !pTexture || pTexture->IsError() )
		return NULL;
	const vtf_sample::Texture *pSample = load( pTexture->GetName() );
	return pSample && pSample->valid ? pSample : NULL;
}

inline bool VarIsSet( IMaterial *pMaterial, const char *pVarName )
{
	bool bFound = false;
	IMaterialVar *pVar = pMaterial->FindVar( pVarName, &bFound, false );
	return bFound && pVar && pVar->IsDefined() && pVar->GetIntValue() != 0;
}

struct Emission
{
	const vtf_sample::Texture *m_pBase = NULL;
	const vtf_sample::Texture *m_pMask = NULL;
	bool m_bMaskFromAlpha = false; // m_pMask's alpha (the envmap mask)

	// False for a material that is not $selfillum or has no base texture.
	template <typename Load> bool Init( IMaterial *pMaterial, Load &load )
	{
		if ( !pMaterial || !pMaterial->GetMaterialVarFlag( MATERIAL_VAR_SELFILLUM ) )
			return false;
		m_pBase = TextureOfVar( pMaterial, "$basetexture", load );
		if ( !m_pBase )
			return false;
		m_pMask = TextureOfVar( pMaterial, "$selfillummask", load );
		m_bMaskFromAlpha = false;
		if ( !m_pMask && VarIsSet( pMaterial, "$selfillum_envmapmask_alpha" ) )
		{
			m_pMask = TextureOfVar( pMaterial, "$envmapmask", load );
			m_bMaskFromAlpha = m_pMask != NULL;
		}
		return true;
	}

	void operator()( float u, float v, float rgb[3] ) const
	{
		float alpha;
		m_pBase->Fetch( u, v, rgb, &alpha );
		float mask[3] = { alpha, alpha, alpha };
		if ( m_pMask )
		{
			float maskRgb[3], maskAlpha;
			m_pMask->Fetch( u, v, maskRgb, &maskAlpha );
			for ( int k = 0; k < 3; ++k )
				mask[k] = m_bMaskFromAlpha ? maskAlpha : maskRgb[k];
		}
		for ( int k = 0; k < 3; ++k )
			rgb[k] *= mask[k];
	}
};

} // namespace selfillum_emission

#endif // SELFILLUM_EMISSION_H
