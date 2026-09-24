//========= Portal 2 port ======================================================//
//
// Purpose: The part of the CS:GO-era video config API (videocfg/videocfg.h)
//          that the Portal 2 video dialogs use, on this engine's material
//          system configuration.
//
//          CS:GO kept a separate video.txt that its engine read at startup.
//          This engine configures video from dxsupport.cfg recommendations
//          and the mat_* ConVars, and "mat_savechanges" persists the video
//          mode. So ReadCurrentVideoConfig() reports the dxsupport
//          recommendations (or the current ConVars) as "setting.*" keys, and
//          UpdateCurrentVideoConfig() has nothing extra to write.
//
//=============================================================================//

#include "cbase.h"
#include "videocfg/videocfg.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/materialsystem_config.h"
#include "IGameUIFuncs.h"
#include "engineinterface.h"
#include "tier1/KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Every "ConVar.<name>" key the material system recommends becomes
// "setting.<name>", the key form the Portal 2 dialogs read.
static void CopyConVarKeysAsSettings( KeyValues *pRecommended, KeyValues *pConfigKeys, bool bCurrentValues )
{
	static const char s_szConVarPrefix[] = "ConVar.";
	const int nPrefixLength = sizeof( s_szConVarPrefix ) - 1;

	for ( KeyValues *pKey = pRecommended->GetFirstValue(); pKey; pKey = pKey->GetNextValue() )
	{
		const char *pszName = pKey->GetName();
		if ( V_strnicmp( pszName, s_szConVarPrefix, nPrefixLength ) )
			continue;

		const char *pszConVar = pszName + nPrefixLength;
		const char *pszValue = pKey->GetString();
		if ( bCurrentValues )
		{
			const ConVar *pVar = g_pCVar->FindVar( pszConVar );
			if ( pVar )
			{
				pszValue = pVar->GetString();
			}
		}

		char szSetting[ 256 ];
		V_snprintf( szSetting, sizeof( szSetting ), "setting.%s", pszConVar );
		pConfigKeys->SetString( szSetting, pszValue );
	}
}

bool ReadCurrentVideoConfig( KeyValues *pConfigKeys, bool bDefault )
{
	if ( !pConfigKeys || !materials )
		return false;

	KeyValues *pRecommended = new KeyValues( "Recommended" );
	if ( !materials->GetRecommendedConfigurationInfo( 0, pRecommended ) )
	{
		pRecommended->deleteThis();
		return false;
	}

	CopyConVarKeysAsSettings( pRecommended, pConfigKeys, !bDefault );
	pRecommended->deleteThis();

	if ( bDefault )
	{
		// The recommended mode is fullscreen at the desktop resolution.
		int nWidth = 640;
		int nHeight = 480;
		if ( gameuifuncs )
		{
			gameuifuncs->GetDesktopResolution( nWidth, nHeight );
		}
		pConfigKeys->SetInt( "setting.defaultres", nWidth );
		pConfigKeys->SetInt( "setting.defaultresheight", nHeight );
		pConfigKeys->SetInt( "setting.fullscreen", 1 );
		pConfigKeys->SetInt( "setting.nowindowborder", 0 );
	}
	else
	{
		const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
		pConfigKeys->SetInt( "setting.defaultres", config.m_VideoMode.m_Width );
		pConfigKeys->SetInt( "setting.defaultresheight", config.m_VideoMode.m_Height );
		pConfigKeys->SetInt( "setting.fullscreen", config.Windowed() ? 0 : 1 );
		// This engine's material config has no borderless window mode.
		pConfigKeys->SetInt( "setting.nowindowborder", 0 );
	}

	return true;
}

bool UpdateCurrentVideoConfig( int nWidth, int nHeight, int nAspectRatioMode, bool bFullscreen, bool bNoWindowBorder, bool bUseRestartConvars )
{
	// The callers apply the mode with mat_setvideomode and persist it with
	// mat_savechanges before calling this; this engine has no video.txt.
	return true;
}
