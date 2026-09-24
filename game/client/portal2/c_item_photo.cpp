#if 0 // Portal 2-only implementation stubbed for this SDK build.
//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client photograph item and its material proxies.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_baseanimating.h"
#include "c_portal_player.h"
#include "proxyentity.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "imaterialproxydict.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: A photograph whose texture is chosen by the server
//-----------------------------------------------------------------------------
class C_Photograph : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_Photograph, C_BaseAnimating );
	DECLARE_CLIENTCLASS();

	const char *GetTextureName( void ) { return m_szTextureName; }
	char m_szTextureName[MAX_PATH];
};

IMPLEMENT_CLIENTCLASS_DT( C_Photograph, DT_Photograph, CPhotograph )
	RecvPropString( RECVINFO( m_szTextureName ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Puts the photograph's texture into $basetexture
//-----------------------------------------------------------------------------
class CPhotoMaterialProxy : public CEntityMaterialProxy
{
public:
	CPhotoMaterialProxy() { m_BaseTextureVar = NULL; }
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( C_BaseEntity *pC_BaseEntity );
	virtual IMaterial *GetMaterial();

private:
	IMaterialVar *m_BaseTextureVar;
};

bool CPhotoMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	bool foundVar;
	m_BaseTextureVar = pMaterial->FindVar( "$basetexture", &foundVar, false );
	return foundVar;
}

void CPhotoMaterialProxy::OnBind( C_BaseEntity *pC_BaseEntity )
{
	if ( !pC_BaseEntity )
		return;

	C_Photograph *pPhoto = dynamic_cast< C_Photograph * >( pC_BaseEntity );
	if ( !pPhoto )
		return;

	ITexture *pTexture = materials->FindTexture( pPhoto->GetTextureName(), TEXTURE_GROUP_MODEL, true, 0 );
	m_BaseTextureVar->SetTextureValue( pTexture );
}

IMaterial *CPhotoMaterialProxy::GetMaterial()
{
	if ( !m_BaseTextureVar )
		return NULL;

	return m_BaseTextureVar->GetOwningMaterial();
}

EXPOSE_MATERIAL_PROXY( CPhotoMaterialProxy, PhotoMaterial );

//-----------------------------------------------------------------------------
// Purpose: Fades a material out while the local player is placing a photo
//-----------------------------------------------------------------------------
class CPlacementPhotoMaterialProxy : public CEntityMaterialProxy
{
public:
	CPlacementPhotoMaterialProxy()
	{
		m_pMaterial = NULL;
		m_pAlphaVar = NULL;
		m_flStartAlpha = 1.0f;
		m_flTargetAlpha = 1.0f;
		m_flFadeDuration = 1.0f;
		m_flFadeStartTime = 0.0f;
		m_bLastState = false;
	}

	virtual void OnBind( C_BaseEntity *pC_BaseEntity );
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual IMaterial *GetMaterial() { return m_pMaterial; }

private:
	float GetAlphaFade( void );

	IMaterial *m_pMaterial;
	IMaterialVar *m_pAlphaVar;
	bool m_bLastState;
	float m_flStartAlpha;
	float m_flTargetAlpha;
	float m_flFadeStartTime;
	float m_flFadeDuration;
};

bool CPlacementPhotoMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	m_pMaterial = pMaterial;

	bool found;
	m_pAlphaVar = pMaterial->FindVar( "$alpha", &found, true );
	if ( !found )
	{
		m_pAlphaVar = NULL;
		return false;
	}

	pMaterial->SetMaterialVarFlag( MATERIAL_VAR_ALPHA_MODIFIED_BY_PROXY, true );
	m_bLastState = false;

	return true;
}

float CPlacementPhotoMaterialProxy::GetAlphaFade( void )
{
	return SimpleSplineRemapValClamped( gpGlobals->curtime, m_flFadeStartTime, m_flFadeStartTime + m_flFadeDuration, m_flStartAlpha, m_flTargetAlpha );
}

void CPlacementPhotoMaterialProxy::OnBind( C_BaseEntity *pC_BaseEntity )
{
	C_Portal_Player *pPlayer = dynamic_cast< C_Portal_Player * >( C_BasePlayer::GetLocalPlayer() );
	if ( !pPlayer )
		return;

	// Start a new fade from wherever the current one is when the state flips
	if ( m_bLastState != pPlayer->GetPortalPlayerLocalData().m_bPlacingPhoto )
	{
		m_flStartAlpha = GetAlphaFade();
		m_flFadeStartTime = gpGlobals->curtime;
		m_flFadeDuration = 0.4f;
		m_flTargetAlpha = m_bLastState ? 1.0f : 0.0f;
		m_bLastState = pPlayer->GetPortalPlayerLocalData().m_bPlacingPhoto;
	}

	m_pAlphaVar->SetFloatValue( GetAlphaFade() );
}

EXPOSE_MATERIAL_PROXY( CPlacementPhotoMaterialProxy, PlacementPhoto );

#endif
