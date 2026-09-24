//========= Portal 2 reconstruction ============================================//
//
// Purpose: Test chamber indicator panels, countdown timers and tic-tac-toe panels
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef INDICATOR_PANEL_H
#define INDICATOR_PANEL_H
#ifdef _WIN32
#pragma once
#endif

#include "baseanimating.h"
#include "GameEventListener.h"

class CVGuiScreen;

class CLabIndicatorPanel : public CBaseEntity
{
public:
	DECLARE_CLASS( CLabIndicatorPanel, CBaseEntity );
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	virtual ~CLabIndicatorPanel();

	virtual int		UpdateTransmitState( void );
	virtual void	SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways );

	virtual void	Spawn( void );
	virtual void	Precache( void );
	virtual void	OnRestore( void );

	void			ScreenVisible( bool bVisible );

	void			Disable( void );
	void			Enable( void );

	// Inputs
	void			InputDisable( inputdata_t &inputdata );
	void			InputEnable( inputdata_t &inputdata );

	// Timer / indicator state
	void			StartTimer( float flDuration );
	void			ResetTimer( void );
	void			SetChecked( bool bIsChecked );
	void			SetPlayerPinged( int iPlayerPinged );
	void			SetIsTimer( bool bIsTimer );

private:

	void			GetControlPanelInfo( int nPanelIndex, const char *&pPanelName );
	void			GetControlPanelClassName( int nPanelIndex, const char *&pPanelName );
	void			SpawnControlPanels( void );
	void			RestoreControlPanels( void );

private:

	CNetworkVar( bool, m_bEnabled );
	CNetworkVar( bool, m_bStopped );
	CNetworkVar( bool, m_bCountingDown );
	CNetworkVar( bool, m_bIsCountdownTimer );
	CNetworkVar( bool, m_bIsChecked );
	CNetworkVar( int, m_iPlayerPinged );

	CNetworkVar( float, m_flTimerDuration );
	CNetworkVar( float, m_flTimerStart );

	// The vgui screen that draws this panel
	CHandle<CVGuiScreen>	m_hScreen;
};

//-----------------------------------------------------------------------------
// Purpose: Model-less world entity that owns and positions a CLabIndicatorPanel
//-----------------------------------------------------------------------------
class CPropIndicatorPanel : public CBaseAnimating
{
public:
	DECLARE_CLASS( CPropIndicatorPanel, CBaseAnimating );
	DECLARE_DATADESC();

	virtual void	Precache( void );
	virtual void	Spawn( void );

	virtual void	UpdateOnRemove( void );

	void			SetTimerDuration( float flDuration );
	void			StartTimer( void );
	void			StopTimer( void );
	void			ResetTimer( void );

	void			InputStart( inputdata_t &indata );
	void			InputStop( inputdata_t &indata );
	void			InputReset( inputdata_t &indata );
	void			InputCheck( inputdata_t &indata );
	void			InputUncheck( inputdata_t &indata );

private:

	void			CreateIndicatorPanel( void );
	void			ToggleIndicatorLights( bool bLightsOn );
	void			ToggleThink( void );

private:

	bool			m_bEnabled;
	bool			m_bStopped;
	bool			m_bCountingDown;
	bool			m_bIsCountdownTimer;
	bool			m_bIsChecked;
	string_t		m_strIndicatorLights;

	float			m_flTimerDuration;
	float			m_flTimerStart;

protected:
	CHandle<CLabIndicatorPanel>	m_hIndicatorPanel;
};

//-----------------------------------------------------------------------------
// Purpose: Indicator panel that shows which coop player pinged it
//-----------------------------------------------------------------------------
class CPropTicTacToePanel : public CPropIndicatorPanel, public CGameEventListener
{
public:
	DECLARE_CLASS( CPropTicTacToePanel, CPropIndicatorPanel );
	DECLARE_DATADESC();

	virtual void	Spawn( void );

	virtual void	FireGameEvent( IGameEvent *event );

	bool			m_bPlayed;

	COutputEvent	m_OnPlayer1Pinged;
	COutputEvent	m_OnPlayer2Pinged;
};

#endif // INDICATOR_PANEL_H
