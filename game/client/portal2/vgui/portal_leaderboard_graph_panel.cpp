//========= Portal 2 reconstruction ============================================//
//
// Purpose: Challenge mode leaderboard graph; see portal_leaderboard_graph_panel.h.
//
//=============================================================================//

#include "cbase.h"
#include "portal_leaderboard_graph_panel.h"
#include "splinegraphpanel.h"
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

// Leaderboard entries read for the friend graph; the leaderboard stops
// returning players well before this.
static const int LEADERBOARD_GRAPH_MAX_ENTRIES = 1024;

const char *ScoreToString( int nScore, LeaderboardType type )
{
	static char s_szScore[ 32 ];

	if ( nScore < 0 )
	{
		V_strncpy( s_szScore, "-", sizeof( s_szScore ) );
	}
	else if ( type == LEADERBOARD_TIME )
	{
		int nMinutes = nScore / 6000;
		int nSeconds = ( nScore / 100 ) % 60;
		int nHundredths = nScore % 100;
		V_snprintf( s_szScore, sizeof( s_szScore ), "%d:%02d.%02d", nMinutes, nSeconds, nHundredths );
	}
	else
	{
		V_snprintf( s_szScore, sizeof( s_szScore ), "%d", nScore );
	}

	return s_szScore;
}

CPortalLeaderboardGraphPanel::CPortalLeaderboardGraphPanel( Panel *pParent, const char *pName, LeaderboardType type ) : BaseClass( pParent, pName )
{
	m_Type = type;
	m_nMinScore = 0;
	m_nMaxScore = 0;
	m_nPlayerScore = -1;
	m_bHasData = false;

	m_pFriendGraph = new SplineGraphPanel( this, "FriendGraph" );
	m_pGlobalGraph = new SplineGraphPanel( this, "GlobalGraph" );
	m_pMinScoreText = new Label( this, "MinScoreText", "" );
	m_pMaxScoreText = new Label( this, "MaxScoreText", "" );
	m_pYourPositionImage = new ImagePanel( this, "YourPositionImage" );
}

void CPortalLeaderboardGraphPanel::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "Resource/UI/portal_leaderboard_graph_panel.res" );
	ClearData();
}

void CPortalLeaderboardGraphPanel::ClearData()
{
	m_pFriendGraph->ClearData();
	m_pGlobalGraph->ClearData();
	m_pMinScoreText->SetText( "" );
	m_pMaxScoreText->SetText( "" );
	m_nMinScore = m_nMaxScore = 0;
	m_bHasData = false;
	InvalidateLayout();
}

void CPortalLeaderboardGraphPanel::UpdateGraph( CPortalLeaderboard *pLeaderboard, LeaderboardType type )
{
	ClearData();
	m_Type = type;

	if ( !pLeaderboard || pLeaderboard->IsInvalid() )
		return;

	// Friend scores come from the leaderboard's own entries.
	CUtlVector< float > friendScores;
	int nMin = INT_MAX;
	int nMax = INT_MIN;
	for ( int i = 0; i < LEADERBOARD_GRAPH_MAX_ENTRIES; ++i )
	{
		const PortalLeaderboardItem_t *pItem = pLeaderboard->GetPlayerAtIndex( i, type );
		if ( !pItem )
			break;
		if ( pItem->m_iScore < 0 )
			continue;

		friendScores.AddToTail( (float)pItem->m_iScore );
		nMin = MIN( nMin, pItem->m_iScore );
		nMax = MAX( nMax, pItem->m_iScore );
	}

	// Portal 2 port: retail also drew the global score distribution, which it
	// downloaded from the Steam Web API. This build has no global leaderboard
	// data, so the global graph stays empty.
	static bool s_bWarned = false;
	if ( !s_bWarned )
	{
		s_bWarned = true;
		DevWarning( "Portal 2: global leaderboard graphs are not supported by this build\n" );
	}

	if ( friendScores.Count() == 0 )
		return;

	m_nMinScore = nMin;
	m_nMaxScore = nMax;
	m_bHasData = true;

	m_pFriendGraph->SetData( friendScores, (float)nMin, (float)nMax );
	m_pMinScoreText->SetText( ScoreToString( nMin, type ) );
	m_pMaxScoreText->SetText( ScoreToString( nMax, type ) );
	InvalidateLayout();
}

void CPortalLeaderboardGraphPanel::SetPlayerScore( int nScore )
{
	m_nPlayerScore = nScore;
	InvalidateLayout();
}

void CPortalLeaderboardGraphPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	int nWide, nTall;
	GetSize( nWide, nTall );

	// The graphs fill the panel inside the x/y buffer (the .res gives them no size).
	int nGraphX = m_nXOffset;
	int nGraphY = m_nYOffset;
	int nGraphWide = MAX( nWide - 2 * m_nXOffset, 0 );
	int nGraphTall = MAX( nTall - 2 * m_nYOffset, 0 );
	m_pFriendGraph->SetBounds( nGraphX, nGraphY, nGraphWide, nGraphTall );
	m_pGlobalGraph->SetBounds( nGraphX, nGraphY, nGraphWide, nGraphTall );

	// Score texts sit on the bottom corners, text_x/y_offset from the edges.
	int nTextWide, nTextTall;
	m_pMinScoreText->GetSize( nTextWide, nTextTall );
	m_pMinScoreText->SetPos( m_nTextXOffset, nTall - nTextTall - m_nTextYOffset );
	m_pMaxScoreText->GetSize( nTextWide, nTextTall );
	m_pMaxScoreText->SetPos( nWide - nTextWide - m_nTextXOffset, nTall - nTextTall - m_nTextYOffset );

	// The marker stands over the player's score along the graph's x axis.
	bool bShowMarker = m_bHasData && m_nPlayerScore >= 0;
	m_pYourPositionImage->SetVisible( bShowMarker );
	if ( bShowMarker )
	{
		float flFraction = 0.5f;
		if ( m_nMaxScore > m_nMinScore )
			flFraction = clamp( (float)( m_nPlayerScore - m_nMinScore ) / (float)( m_nMaxScore - m_nMinScore ), 0.0f, 1.0f );

		int nMarkerX, nMarkerY;
		m_pYourPositionImage->GetPos( nMarkerX, nMarkerY );
		nMarkerX = nGraphX + (int)( flFraction * nGraphWide ) - m_pYourPositionImage->GetWide() / 2;
		m_pYourPositionImage->SetPos( nMarkerX, nMarkerY );
	}
}
