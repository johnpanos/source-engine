//========= Portal 2 reconstruction ============================================//
//
// Purpose: Co-op versus score HUD; see hud_vs_score_panel.cpp.
//
// Reconstructed from the retail Linux client.so. Not original Valve source;
// the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef HUD_VS_SCORE_PANEL_H
#define HUD_VS_SCORE_PANEL_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/EditablePanel.h>

namespace vgui
{
	class Label;
}

enum
{
	VS_WINNER_BLUE = 0,
	VS_WINNER_ORANGE = 1,
	VS_WINNER_TIE = 2,
};

class CHUDVSScorePanel : public CHudElement, public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHUDVSScorePanel, vgui::EditablePanel );

public:
	CHUDVSScorePanel( const char *pElementName );

	virtual bool ShouldDraw( void );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void Paint( void );
	virtual void PerformLayout( void );
	virtual void OnTick( void );

	void SetShowPanel( bool bShow );
	void SetGameInfo( float flRoundTime, const char *pDescription, int nRoundsToWin );
	void SetScores( int nBlueScore, int nOrangeScore, int nBlueRounds, int nOrangeRounds );
	void SetWinner( int nWinner );
	void StartRound( bool bCountdown );
	void RoundOver( int nWinner );

private:
	void ResetPanel( void );
	void ShowScoreLabels( void );
	void UpdateTimerLabel( void );

	vgui::Label *m_pBlueTeamLabel;
	vgui::Label *m_pOrangeTeamLabel;
	vgui::Label *m_pBlueScoreLabel;
	vgui::Label *m_pOrangeScoreLabel;
	vgui::Label *m_pWinnerLabel;
	vgui::Label *m_pGameDescriptionLabel;
	vgui::Label *m_pCountdownLabel;
	vgui::Label *m_pTimerLabel;
	vgui::Label *m_pBlueRoundsLabel;
	vgui::Label *m_pOrangeRoundsLabel;

	float m_flRoundTime;
	float m_flTimeRemaining;
	float m_flLastTickTime;
	float m_flLastFlashTime;
	float m_flCountdownStartTime;
	bool m_bFlashOn;
	bool m_bCountingDown;
	int m_nRoundsToWin;
	bool m_bShowPanel;
};

#endif // HUD_VS_SCORE_PANEL_H
