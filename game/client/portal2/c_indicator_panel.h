//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the test chamber indicator / countdown panel
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef C_INDICATOR_PANEL_H
#define C_INDICATOR_PANEL_H
#ifdef _WIN32
#pragma once
#endif

class C_LabIndicatorPanel : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_LabIndicatorPanel, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	bool	IsEnabled( void ) { return m_bEnabled; }
	float	GetCountdownPercentage( void );
	bool	IsCountingDown( void ) { return m_bCountingDown; }
	bool	IsCountdownTimer( void ) { return m_bIsCountdownTimer; }
	bool	IsPaused( void ) { return m_bStopped; }
	bool	IsChecked( void ) { return m_bIsChecked; }
	int		GetPlayerPinged( void ) { return m_iPlayerPinged; }

private:
	bool	m_bEnabled;
	bool	m_bStopped;
	bool	m_bCountingDown;
	bool	m_bIsCountdownTimer;
	bool	m_bIsChecked;
	int		m_iPlayerPinged;
	float	m_flTimerDuration;
	float	m_flTimerStart;
};

#endif // C_INDICATOR_PANEL_H
