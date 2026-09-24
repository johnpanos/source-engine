//========= Portal 2 reconstruction ============================================//
//
// Purpose: Shared lightboard ("progress sign") vgui screen used by the single
//          player elevators (sp_progress_sign) and the co-op tracks
//          (mp_progress_sign): level number, progress bar, test icons, dirt
//          overlay and the power-on flicker.
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output) and the retail scripts
// scripts/vscripts/transitions/*.txt and scripts/vscripts/coop/*.txt. No
// original source or Steam2 pseudocode exists for this file. Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef VGUI_BASE_PROGRESS_SCREEN_H
#define VGUI_BASE_PROGRESS_SCREEN_H
#ifdef _WIN32
#pragma once
#endif

#include "c_vguiscreen.h"
#include "igamesystem.h"
#include "utlvector.h"

#define LIGHTBOARD_MAX_ICONS		10
#define LIGHTBOARD_DIRT_LEVELS		3

// Indices into the lightboard icon sheet (screens/p2_lightboard_vgui). Index 0
// is a placeholder; the sheet cell for icon N is N - 1.
extern const char *g_pszLightboardIcons[];

//-----------------------------------------------------------------------------
// Timing of the power-on flicker, from lightboard_startup_sequences.txt
//-----------------------------------------------------------------------------
struct LightboardStartupSequence_t
{
	char	m_szName[128];
	float	m_flFlickerRateMin;
	float	m_flFlickerRateMax;
	float	m_flFlickerQuickMin;
	float	m_flFlickerQuickMax;
	float	m_flBackgroundFlickerLength;
	float	m_flLevelNumberDelay;
	float	m_flProgressDelay;
	float	m_flIconDelay;
};

//-----------------------------------------------------------------------------
// Loads the lightboard scripts once for every progress sign
//-----------------------------------------------------------------------------
class CProgressSignScreenKeys : public CAutoGameSystem
{
public:
	CProgressSignScreenKeys( const char *pszName );
	virtual ~CProgressSignScreenKeys();

	virtual bool Init();

	KeyValues *GetSinglePlayerKeys() const { return m_pSinglePlayerKeys; }
	KeyValues *GetCoopKeys() const { return m_pCoopKeys; }

	int FindStartupSequence( const char *pszName ) const;
	const LightboardStartupSequence_t &GetStartupSequence( int nIndex ) const { return m_StartupSequences[ nIndex ]; }

private:
	bool LoadStartupSequences();

	KeyValues *m_pSinglePlayerKeys;
	KeyValues *m_pCoopKeys;
	bool m_bStartupSequencesLoaded;
	CUtlVector< LightboardStartupSequence_t > m_StartupSequences;
};

extern CProgressSignScreenKeys g_ProgressSignScreenKeys;

//-----------------------------------------------------------------------------
// Base lightboard screen
//-----------------------------------------------------------------------------
class CVGUI_Base_ProgressSignScreen : public CVGuiScreenPanel
{
	DECLARE_CLASS( CVGUI_Base_ProgressSignScreen, CVGuiScreenPanel );

public:
	CVGUI_Base_ProgressSignScreen( vgui::Panel *pParent, const char *pMetaClassName );
	virtual ~CVGUI_Base_ProgressSignScreen();

	virtual void OnTick( void );
	virtual bool Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData );

protected:
	void ClearIcons( void );
	void StartFlicker( void );

	// Advances the flicker and returns the alpha for the level number, the
	// progress bar and the label text.
	float UpdateFlicker( void );

	void PaintBoardBackground( void );
	void PaintLevelNumber( int nLevel, float flAlpha );
	void PaintProgressBar( int nLevel, int nTotalLevels, float flAlpha );
	void PaintIcons( void );
	void PaintDirt( void );

	void SetStartupSequence( const char *pszName );
	void SetDirtLevel( KeyValues *pDirt );

	int		m_nIcons[ LIGHTBOARD_MAX_ICONS ];
	bool	m_bIconEnabled[ LIGHTBOARD_MAX_ICONS ];

	int		m_nStartupSequence;
	bool	m_bFlickering;
	int		m_nOverlayAlpha;
	int		m_nBaseBrightness;
	float	m_flDisabledIconAlpha;
	int		m_nDirtLevel;

	int		m_nBoardTextureID;
	int		m_nNumbersTextureID;
	int		m_nBarTextureID;
	int		m_nIconsTextureID;

	float	m_flFlickerStartTime;
	float	m_flFlickerLength;
	int		m_nBrightness;
	float	m_flIconAlphaScale;
	bool	m_bWasEnabled;
	float	m_flNextFlickerDuration;
	float	m_flLastFlickerTime;
	float	m_flQuickFlickerDuration;
	int		m_nDirtTextureIDs[ LIGHTBOARD_DIRT_LEVELS ];
	bool	m_bShownOnce;
};

#endif // VGUI_BASE_PROGRESS_SCREEN_H
