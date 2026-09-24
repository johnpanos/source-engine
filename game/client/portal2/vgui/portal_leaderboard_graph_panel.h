//========= Portal 2 reconstruction ============================================//
//
// Purpose: Challenge mode leaderboard graph: how a map's scores are spread
//          among friends (and, in retail, globally), with a marker at the
//          local player's score.
//
// Reconstructed from the retail class, child and animation-variable names
// (CPortalLeaderboardGraphPanel, FriendGraph, GlobalGraph, MinScoreText,
// MaxScoreText, YourPositionImage, x/y and text offsets) and
// resource/ui/portal_leaderboard_graph_panel.res; the code is not original
// Valve source. The repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PORTAL_LEADERBOARD_GRAPH_PANEL_H
#define PORTAL_LEADERBOARD_GRAPH_PANEL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include "portal2_leaderboard.h"

namespace vgui
{
	class Label;
	class ImagePanel;
}
class SplineGraphPanel;
class CPortalLeaderboard;

// Text for a leaderboard score: a portal count, or a time in hundredths of a
// second as M:SS.hh. Negative scores (no score) are "-". The returned buffer
// is reused by the next call.
const char *ScoreToString( int nScore, LeaderboardType type );

class CPortalLeaderboardGraphPanel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CPortalLeaderboardGraphPanel, vgui::EditablePanel );

public:
	CPortalLeaderboardGraphPanel( vgui::Panel *pParent, const char *pName, LeaderboardType type );

	// Rebuild the graphs from the leaderboard's entries of the given type.
	void UpdateGraph( CPortalLeaderboard *pLeaderboard, LeaderboardType type );
	void ClearData();

	// The local player's score, or -1 to hide the marker.
	void SetPlayerScore( int nScore );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();

private:
	LeaderboardType m_Type;

	SplineGraphPanel *m_pFriendGraph;
	SplineGraphPanel *m_pGlobalGraph;
	vgui::Label *m_pMinScoreText;
	vgui::Label *m_pMaxScoreText;
	vgui::ImagePanel *m_pYourPositionImage;

	int m_nMinScore;
	int m_nMaxScore;
	int m_nPlayerScore;
	bool m_bHasData;

	// Buffer around the graphs, and distance of the score texts from the edges.
	CPanelAnimationVarAliasType( int, m_nXOffset, "x_offset", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_nYOffset, "y_offset", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_nTextXOffset, "text_x_offset", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_nTextYOffset, "text_y_offset", "0", "proportional_int" );
};

#endif // PORTAL_LEADERBOARD_GRAPH_PANEL_H
