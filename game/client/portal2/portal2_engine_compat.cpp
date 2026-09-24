//========= Portal 2 port ======================================================//
//
// Purpose: Client adapter layer for the imported Portal 2 client code; see
//          portal2_engine_compat.h for the rendering adaptation pattern and the
//          behavior of each adapter.
//
//=============================================================================//

#include "cbase.h"
#include "portal2_engine_compat.h"
#include "portal/c_portal_player.h"
#include "cliententitylist.h"
#include "prediction.h"
#include "inetchannelinfo.h"
#include "inputsystem/iinputsystem.h"
#include "engine/ishadowmgr.h"
#include "tier0/icommandline.h"
#include "tier1/fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern bool IsInCommentaryMode( void );

//-----------------------------------------------------------------------------
// One-time report of a CS:GO engine feature this engine does not provide.
//-----------------------------------------------------------------------------
#define PORTAL2_UNSUPPORTED( feature ) \
	do \
	{ \
		static bool s_bWarned = false; \
		if ( !s_bWarned ) \
		{ \
			s_bWarned = true; \
			DevWarning( "Portal 2: " feature " is not supported by this engine\n" ); \
		} \
	} while ( 0 )

void Portal2_ClientTeleport( C_BaseEntity *pEntity, const Vector *pOrigin,
							 const QAngle *pAngles, const Vector *pVelocity )
{
	if ( !pEntity )
		return;

	if ( pOrigin )
		pEntity->SetAbsOrigin( *pOrigin );
	if ( pAngles )
		pEntity->SetAbsAngles( *pAngles );
	if ( pVelocity )
		pEntity->SetAbsVelocity( *pVelocity );
	pEntity->ResetLatched();
}

void Portal2_ClientSetUseEntity( C_BasePlayer *pPlayer, C_BaseEntity *pUseEntity )
{
	UNREFERENCED_PARAMETER( pPlayer );
	UNREFERENCED_PARAMETER( pUseEntity );
	PORTAL2_UNSUPPORTED( "client-side SetUseEntity" );
}

void Portal2_ClientForceDropOfCarriedPhysObjects( C_BasePlayer *pPlayer )
{
	C_Portal_Player *pPortalPlayer = dynamic_cast< C_Portal_Player * >( pPlayer );
	if ( pPortalPlayer )
	{
		pPortalPlayer->ForceDropOfCarriedPhysObjects( NULL );
	}
}

//-----------------------------------------------------------------------------
// Stencil state
//-----------------------------------------------------------------------------
void Portal2_SetStencilState( IMatRenderContext *pRenderContext, const ShaderStencilState_t &state )
{
	pRenderContext->SetStencilEnable( state.m_bEnable );
	pRenderContext->SetStencilFailOperation( state.m_FailOp );
	pRenderContext->SetStencilZFailOperation( state.m_ZFailOp );
	pRenderContext->SetStencilPassOperation( state.m_PassOp );
	pRenderContext->SetStencilCompareFunction( state.m_CompareFunc );
	pRenderContext->SetStencilReferenceValue( state.m_nReferenceValue );
	pRenderContext->SetStencilTestMask( state.m_nTestMask );
	pRenderContext->SetStencilWriteMask( state.m_nWriteMask );
}

//-----------------------------------------------------------------------------
// Depth override
//-----------------------------------------------------------------------------
void Portal2_OverrideDepthEnable( IMatRenderContext *pRenderContext, bool bEnable, bool bDepthWriteEnable, bool bDepthTestEnable )
{
	if ( bEnable && !bDepthTestEnable )
	{
		PORTAL2_UNSUPPORTED( "disabling the depth test through OverrideDepthEnable" );
	}
	pRenderContext->OverrideDepthEnable( bEnable, bDepthWriteEnable );
}

//-----------------------------------------------------------------------------
// Cull mode flip
//-----------------------------------------------------------------------------
static MaterialCullMode_t s_nPortal2CullMode = MATERIAL_CULLMODE_CCW;

void Portal2_FlipCullMode( IMatRenderContext *pRenderContext )
{
	s_nPortal2CullMode = ( s_nPortal2CullMode == MATERIAL_CULLMODE_CCW ) ? MATERIAL_CULLMODE_CW : MATERIAL_CULLMODE_CCW;
	pRenderContext->CullMode( s_nPortal2CullMode );
}

//-----------------------------------------------------------------------------
// Scissor stack
//-----------------------------------------------------------------------------
struct Portal2ScissorRect_t
{
	int m_nLeft;
	int m_nTop;
	int m_nRight;
	int m_nBottom;
};
static CUtlVector< Portal2ScissorRect_t > s_Portal2ScissorStack;

void Portal2_PushScissorRect( IMatRenderContext *pRenderContext, int nLeft, int nTop, int nRight, int nBottom )
{
	Portal2ScissorRect_t rect = { nLeft, nTop, nRight, nBottom };
	s_Portal2ScissorStack.AddToTail( rect );
	pRenderContext->SetScissorRect( nLeft, nTop, nRight, nBottom, true );
}

void Portal2_PopScissorRect( IMatRenderContext *pRenderContext )
{
	Assert( s_Portal2ScissorStack.Count() > 0 );
	if ( s_Portal2ScissorStack.Count() > 0 )
	{
		s_Portal2ScissorStack.RemoveMultipleFromTail( 1 );
	}

	if ( s_Portal2ScissorStack.Count() > 0 )
	{
		const Portal2ScissorRect_t &rect = s_Portal2ScissorStack.Tail();
		pRenderContext->SetScissorRect( rect.m_nLeft, rect.m_nTop, rect.m_nRight, rect.m_nBottom, true );
	}
	else
	{
		pRenderContext->SetScissorRect( -1, -1, -1, -1, false );
	}
}

//-----------------------------------------------------------------------------
// Flashlight scissor bounds
//-----------------------------------------------------------------------------
void Portal2_PushFlashlightScissorBounds()
{
	PORTAL2_UNSUPPORTED( "IShadowMgr::PushFlashlightScissorBounds" );
}

void Portal2_PopFlashlightScissorBounds()
{
	PORTAL2_UNSUPPORTED( "IShadowMgr::PopFlashlightScissorBounds" );
}

//-----------------------------------------------------------------------------
// Aspect ratio
//-----------------------------------------------------------------------------
const AspectRatioInfo_t &Portal2_GetAspectRatioInfo()
{
	static AspectRatioInfo_t s_Info;

	int nWidth = 0, nHeight = 0;
	engine->GetScreenSize( nWidth, nHeight );
	if ( nWidth > 0 && nHeight > 0 )
	{
		s_Info.m_flFrameBufferAspectRatio = (float)nWidth / (float)nHeight;
		s_Info.m_flPhysicalAspectRatio = s_Info.m_flFrameBufferAspectRatio;
		s_Info.m_flFrameBuffertoPhysicalScalar = 1.0f;
		s_Info.m_flPhysicalToFrameBufferScalar = 1.0f;
		s_Info.m_bIsWidescreen = ( s_Info.m_flPhysicalAspectRatio >= 1.5999f );
		s_Info.m_bIsHidef = ( nHeight >= 720 );
		s_Info.m_bInitialized = true;
	}
	return s_Info;
}

//-----------------------------------------------------------------------------
// Split screen
//-----------------------------------------------------------------------------
C_BasePlayer *GetSplitScreenViewPlayer( int nSlot )
{
	Assert( nSlot == -1 || nSlot == 0 );
	return C_BasePlayer::GetLocalPlayer();
}

//-----------------------------------------------------------------------------
// IVEngineClient
//-----------------------------------------------------------------------------
namespace Portal2Engine
{

C_BaseEntity *EntityFromUserMessageEHandle( uint32 encodedHandle )
{
	return cl_entitylist->GetBaseEntityFromHandle( ClientEntityHandle_t( encodedHandle ) );
}

const char *GetLevelNameShort()
{
	static char s_szLevelNameShort[MAX_PATH];
	if ( !engine->IsConnected() )
	{
		return "";
	}

	const char *pszLevelName = engine->GetLevelName();
	if ( !pszLevelName || !pszLevelName[0] )
	{
		return "";
	}

	V_FileBase( pszLevelName, s_szLevelNameShort, sizeof( s_szLevelNameShort ) );
	return s_szLevelNameShort;
}

bool IsClientLocalToActiveServer()
{
	INetChannelInfo *pNetChannel = engine->GetNetChannelInfo();
	return pNetChannel && pNetChannel->IsLoopback();
}

const char *GetModDirectory()
{
	static char s_szModDirectory[MAX_PATH];
	V_FileBase( engine->GetGameDirectory(), s_szModDirectory, sizeof( s_szModDirectory ) );
	return s_szModDirectory;
}

const char *GetSaveDirName()
{
	// The engine's save/restore code writes to "save/" in the MOD path.
	return "save/";
}

int GetActiveSplitScreenPlayerSlot()
{
	return 0;
}

int SetActiveSplitScreenPlayerSlot( int nSlot )
{
	if ( nSlot != 0 )
	{
		PORTAL2_UNSUPPORTED( "split screen slots other than 0" );
	}
	return 0;
}

bool IsLocalPlayerResolvable()
{
	return true;
}

bool IsInCommentaryMode()
{
	return ::IsInCommentaryMode();
}

void GetStartupImage( char *pDest, int nDestLen )
{
	static int s_nWhich = 0;
	const int nStartupImages = 2;
	if ( !s_nWhich )
	{
		// Once chosen, the image stays the same for the whole session.
		int nOverride = CommandLine()->ParmValue( "-startup", 0 );
		if ( nOverride > 0 )
		{
			s_nWhich = clamp( nOverride, 1, nStartupImages );
		}
		else
		{
			s_nWhich = ( (unsigned int)Plat_MSTime() % nStartupImages ) + 1;
		}
	}

	const AspectRatioInfo_t &aspectRatioInfo = Portal2_GetAspectRatioInfo();
	V_snprintf( pDest, nDestLen, "console/portal2_product_%d%s", s_nWhich, aspectRatioInfo.m_bIsWidescreen ? "_widescreen" : "" );
}

void SetConnectionPassword( const char *pchCurrentPW )
{
	ConVarRef password( "password" );
	if ( password.IsValid() )
	{
		password.SetValue( pchCurrentPW ? pchCurrentPW : "" );
	}
}

int GetLastAcknowledgedCommand()
{
	return prediction ? prediction->GetLastAcknowledgedCommand() : 0;
}

bool GameHasShutdownAndFlushedMemory()
{
	return true;
}

// The CS:GO engine's IsKeyBoundedToBinding(): a binding matches when it is the
// command itself (ignoring a leading '+') or one entry of a ';' list.
static bool Portal2_IsKeyBoundToBinding( ButtonCode_t code, const char *pBind )
{
	const char *pszBinding = engine->Key_BindingForKey( code );
	if ( !pszBinding || !pszBinding[0] )
		return false;

	char szBinding[256];
	V_strncpy( szBinding, pszBinding, sizeof( szBinding ) );

	char *pToken = szBinding;
	while ( pToken && *pToken )
	{
		char *pNext = strchr( pToken, ';' );
		if ( pNext )
		{
			*pNext++ = '\0';
		}
		while ( *pToken == ' ' )
		{
			++pToken;
		}
		if ( *pToken == '+' )
		{
			++pToken;
		}
		int nLen = V_strlen( pToken );
		while ( nLen > 0 && pToken[nLen - 1] == ' ' )
		{
			pToken[--nLen] = '\0';
		}
		if ( !V_stricmp( pToken, pBind ) )
			return true;
		pToken = pNext;
	}
	return false;
}

static bool Portal2_ButtonMatchesLookup( int nCode, BindingLookupOption_t nFlags )
{
	switch ( nFlags )
	{
	case BINDINGLOOKUP_KEYBOARD_ONLY:
		return nCode < JOYSTICK_FIRST || nCode > JOYSTICK_LAST;
	case BINDINGLOOKUP_JOYSTICK_ONLY:
		return nCode >= JOYSTICK_FIRST && nCode <= JOYSTICK_LAST;
	case BINDINGLOOKUP_STEAMCONTROLLER_ONLY:
		// This engine has no Steam Controller button range.
		return false;
	default:
		return true;
	}
}

int Key_CodeForBinding( const char *pBinding, int nUserId, int nStartCount, BindingLookupOption_t nFlags )
{
	if ( !pBinding )
		return BUTTON_CODE_INVALID;

	const char *pBind = ( pBinding[0] == '+' ) ? pBinding + 1 : pBinding;
	int nCount = 0;
	for ( int i = 0; i < BUTTON_CODE_LAST; ++i )
	{
		if ( !Portal2_ButtonMatchesLookup( i, nFlags ) || !Portal2_IsKeyBoundToBinding( (ButtonCode_t)i, pBind ) )
			continue;

		if ( nCount == nStartCount )
			return i;
		++nCount;
	}
	return BUTTON_CODE_INVALID;
}

const char *Key_LookupBindingEx( const char *pBinding, int nUserId, int nStartCount, BindingLookupOption_t nFlags )
{
	int nCode = Key_CodeForBinding( pBinding, nUserId, nStartCount, nFlags );
	if ( nCode == BUTTON_CODE_INVALID )
		return NULL;
	return inputsystem->ButtonCodeToString( (ButtonCode_t)nCode );
}

void ComputeLightingCube( const Vector &pt, bool bClamp, Vector *pBoxColors )
{
	Vector vecColor;
	engine->ComputeLighting( pt, NULL, bClamp, vecColor, pBoxColors );
}

bool SaveGame( const char *pSaveFilename, bool bIsXSave, char *pOutName, int nOutNameSize, char *pOutComment, int nOutCommentSize )
{
	if ( !pSaveFilename || !pSaveFilename[0] || engine->IsSaveInProgress() )
		return false;

	char szName[MAX_PATH];
	V_FileBase( pSaveFilename, szName, sizeof( szName ) );
	if ( pOutName && nOutNameSize > 0 )
	{
		V_snprintf( pOutName, nOutNameSize, "%s%s.sav", GetSaveDirName(), szName );
	}
	if ( pOutComment && nOutCommentSize > 0 )
	{
		pOutComment[0] = '\0';
	}

	engine->ClientCmd_Unrestricted( CFmtStr( "save \"%s\"\n", szName ) );
	return true;
}

//-----------------------------------------------------------------------------
// Unsupported engine features
//-----------------------------------------------------------------------------
bool HasPaintmap()
{
	PORTAL2_UNSUPPORTED( "paint maps" );
	return false;
}

bool SpherePaintSurface( const model_t *pModel, const Vector &vPosition, unsigned char color, float flSphereRadius, float flPaintCoatPercent )
{
	PORTAL2_UNSUPPORTED( "paint maps" );
	return false;
}

void SphereTracePaintSurface( const model_t *pModel, const Vector &vPosition, const Vector &vContactNormal, float flSphereRadius, CUtlVector<unsigned char> &surfColors )
{
	PORTAL2_UNSUPPORTED( "paint maps" );
	surfColors.RemoveAll();
}

void RemoveAllPaint()
{
	PORTAL2_UNSUPPORTED( "paint maps" );
}

void PaintAllSurfaces( unsigned char color )
{
	PORTAL2_UNSUPPORTED( "paint maps" );
}

void RemovePaint( const model_t *pModel )
{
	PORTAL2_UNSUPPORTED( "paint maps" );
}

void LoadPaintmapData( const CUtlVector<unsigned char> &data, int nPaintmapID, int nPaintmapOffset )
{
	PORTAL2_UNSUPPORTED( "paint maps" );
}

void LoadPaintSurfBits( const CVarBitVec &paintSurfBits )
{
	PORTAL2_UNSUPPORTED( "paint maps" );
}

bool IsTransitioningToLoad()
{
	PORTAL2_UNSUPPORTED( "IVEngineClient::IsTransitioningToLoad" );
	return false;
}

void HideLoadingPlaque()
{
	PORTAL2_UNSUPPORTED( "IVEngineClient::HideLoadingPlaque" );
}

void SetBlurFade( float flScale )
{
	PORTAL2_UNSUPPORTED( "IVEngineClient::SetBlurFade" );
}

void AudioLanguageChanged()
{
	PORTAL2_UNSUPPORTED( "IVEngineClient::AudioLanguageChanged" );
}

void RegisterDemoCustomDataCallback( const char *pszName, pfnDemoCustomDataCallback pfnCallback )
{
	PORTAL2_UNSUPPORTED( "demo custom data" );
}

void RecordDemoCustomData( pfnDemoCustomDataCallback pfnCallback, const void *pData, size_t iDataLength )
{
	PORTAL2_UNSUPPORTED( "demo custom data" );
}

void SOSSetOpvarFloat( const char *pOpVarName, float flValue )
{
	PORTAL2_UNSUPPORTED( "sound operator variables" );
}

int GetMixLayerIndex( const char *pszMixLayerName )
{
	PORTAL2_UNSUPPORTED( "sound mix layers" );
	return -1;
}

int GetMixGroupIndex( const char *pszMixGroupName )
{
	PORTAL2_UNSUPPORTED( "sound mix layers" );
	return -1;
}

void SetMixLayerTriggerFactor( int nMixLayerIndex, int nMixGroupIndex, float flFactor )
{
	PORTAL2_UNSUPPORTED( "sound mix layers" );
}

} // namespace Portal2Engine
