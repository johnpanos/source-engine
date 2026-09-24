//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of point_survey: opens the survey panel on request.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_baseentity.h"
#include "baseviewport.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "portal2/vgui/surveypanel.h"
#include "portal2_engine_compat.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_PointSurvey : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_PointSurvey, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_PointSurvey()
	{
	}

	virtual void PostDataUpdate( DataUpdateType_t updateType )
	{
	}

	const char *GetSurveyName( void ) { return m_iszSurveyName; }

protected:
	char m_iszSurveyName[MAX_PATH];
};

IMPLEMENT_CLIENTCLASS_DT( C_PointSurvey, DT_PointSurvey, CPointSurvey )
	RecvPropString( RECVINFO( m_iszSurveyName ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: Server asks every client to show a point_survey's survey
//-----------------------------------------------------------------------------
void __MsgFunc_StartSurvey( bf_read &msg )
{
	C_PointSurvey *pPointSurvey = static_cast< C_PointSurvey * >( Portal2Engine::EntityFromUserMessageEHandle( msg.ReadLong() ) );
	if ( !pPointSurvey )
	{
		Warning( "PointSurvey: Unable to find client side survey entity\n" );
		return;
	}

	if ( !ShowSurveyPanel( pPointSurvey->GetSurveyName() ) )
	{
		// No panel (disabled or failed to load): report completion right away
		CUtlString strCmd;
		int nEntityIndex = pPointSurvey->entindex();
		strCmd.Format( "survey_done %i", nEntityIndex );
		engine->ClientCmd( strCmd );
		return;
	}

	// Remember which entity to notify when the panel closes
	CBaseViewport *pViewport = dynamic_cast< CBaseViewport * >( GetClientMode()->GetViewport() );
	if ( pViewport )
	{
		CSurveyPanel *pSurveyPanel = static_cast< CSurveyPanel * >( pViewport->FindPanelByName( PANEL_SURVEY ) );
		if ( pSurveyPanel )
		{
			pSurveyPanel->m_hPointSurveyEntity = pPointSurvey;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Hooks the StartSurvey user message for every split-screen slot
//-----------------------------------------------------------------------------
class C_PointSurveyHelper : public CAutoGameSystem
{
	virtual bool Init( void )
	{
		for ( int i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i )
		{
			ACTIVE_SPLITSCREEN_PLAYER_GUARD( i );
			HOOK_MESSAGE( StartSurvey );
		}

		return true;
	}
};

static C_PointSurveyHelper s_PointSurveyHelper;
