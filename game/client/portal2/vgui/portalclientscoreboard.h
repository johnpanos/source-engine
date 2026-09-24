//========= Portal 2 reconstruction ============================================//
//
// Purpose: Portal 2 co-op scoreboard (per-team player lists and spectators).
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PORTALCLIENTSCOREBOARDDIALOG_H
#define PORTALCLIENTSCOREBOARDDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <clientscoreboarddialog.h>

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CPortalClientScoreBoardDialog : public CClientScoreBoardDialog
{
	DECLARE_CLASS_SIMPLE( CPortalClientScoreBoardDialog, CClientScoreBoardDialog );

public:
	CPortalClientScoreBoardDialog( IViewPort *pViewPort );
	~CPortalClientScoreBoardDialog();

	virtual void Reset();
	virtual void Update();

	// vgui overrides for rounded corner background
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	void InitPlayerList( vgui::SectionedListPanel *pPlayerList, int teamNumber );
	virtual void UpdateTeamInfo();
	void UpdatePlayerList();
	void UpdateSpectatorList();
	virtual bool GetPlayerScoreInfo( int playerIndex, KeyValues *kv );

	bool ShouldShowAsSpectator( int iPlayerIndex );
	virtual void FireGameEvent( IGameEvent *event );

	static bool PortalPlayerSortFunc( vgui::SectionedListPanel *list, int itemID1, int itemID2 );

	// rounded corners
	Color m_bgColor;
	Color m_borderColor;

	// player lists
	vgui::SectionedListPanel *m_pPlayerListT;
	vgui::SectionedListPanel *m_pPlayerListCT;

	vgui::Label *m_pPlayerCountLabel_T;
	vgui::Label *m_pScoreLabel_T;
	vgui::Label *m_pPingLabel_T;
	vgui::Label *m_pPlayerCountLabel_CT;
	vgui::Label *m_pScoreLabel_CT;
	vgui::Label *m_pPingLabel_CT;
};

#endif // PORTALCLIENTSCOREBOARDDIALOG_H
