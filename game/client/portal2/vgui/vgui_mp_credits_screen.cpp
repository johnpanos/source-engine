//========= Portal 2 reconstruction ============================================//
//
// Purpose: Co-op ending credits screen (vgui_screen panel
//          "mp_coop_credits_screen" in mp_coop_credits): scrolling names and
//          credit lines plus the "scanning" readout, driven by the co-op
//          game rules' credits state.
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output) and the retail
// scripts/screens/mp_coop_credits_screen.res. No original source or Steam2
// pseudocode exists for this file. Not original Valve source; the
// repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_vguiscreen.h"
#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/AnimationController.h>
#include "portal_mp_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define CREDITS_NAME_LINES	8
#define CREDITS_TEXT_LINES	10
#define CREDITS_DATA_DIGITS	4

// Credits state from CPortalMPGameRules::GetCoopCreditsState()
enum
{
	COOP_CREDITS_NAMES = 0,
	COOP_CREDITS_TEXT = 1,
};

class CVGUI_MP_CreditsScreen : public CVGuiScreenPanel
{
	DECLARE_CLASS( CVGUI_MP_CreditsScreen, CVGuiScreenPanel );

public:
	CVGUI_MP_CreditsScreen( vgui::Panel *pParent, const char *pPanelName );

	virtual bool Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData );
	virtual void OnTick( void );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout( void );
	virtual void Paint( void );

	virtual void UpdateCredits( void );

private:
	void FadeCredits( int nState );
	void AddCreditName( void );
	void AddCreditText( void );

	int m_nLastCreditsIndex;
	int m_nNameIndex;
	int m_nTextIndex;

	vgui::Label *m_pCreditNames[ CREDITS_NAME_LINES ];
	vgui::Label *m_pCreditText[ CREDITS_TEXT_LINES ];
	vgui::ImagePanel *m_pScanHuman;
	vgui::ImagePanel *m_pScanUnknown;
	vgui::ImagePanel *m_pScanningLine;
	vgui::ImagePanel *m_pBlackDataBlocker;
	vgui::Label *m_pDataDigits[ CREDITS_DATA_DIGITS ];
	vgui::Label *m_pScanningText;
	vgui::Label *m_pJobDescription;

	bool m_bLabelsReady;
	bool m_bShownOnce;
	int m_nNameLineTall;
	int m_nTextLineTall;
	int m_nCreditsState;
	int m_nLastScanState;
	bool m_bLastFadeState;
	int m_nTopY;
};

DECLARE_VGUI_SCREEN_FACTORY( CVGUI_MP_CreditsScreen, "mp_coop_credits_screen" );

CVGUI_MP_CreditsScreen::CVGUI_MP_CreditsScreen( vgui::Panel *pParent, const char *pPanelName )
	: BaseClass( pParent, "CVGUI_MP_CreditsScreen" )
{
	SetScheme( "basemodui_scheme" );

	m_bLastFadeState = false;
	m_nLastCreditsIndex = 0;
	m_nNameIndex = 0;
	m_nTextIndex = 0;
	m_pScanHuman = NULL;
	m_pScanUnknown = NULL;
	m_pScanningLine = NULL;
	m_pBlackDataBlocker = NULL;
	m_pScanningText = NULL;
	m_pJobDescription = NULL;
	m_bLabelsReady = false;
	m_bShownOnce = false;
	m_nNameLineTall = 0;
	m_nTextLineTall = 0;
	m_nCreditsState = 0;
	m_nLastScanState = 0;
	m_nTopY = 0;

	for ( int i = 0; i < CREDITS_DATA_DIGITS; ++i )
	{
		m_pDataDigits[ i ] = NULL;
	}

	for ( int i = 0; i < CREDITS_NAME_LINES; ++i )
	{
		m_pCreditNames[ i ] = new vgui::Label( this, VarArgs( "CreditName%d", i ), "" );
	}

	for ( int i = 0; i < CREDITS_TEXT_LINES; ++i )
	{
		m_pCreditText[ i ] = new vgui::Label( this, VarArgs( "CreditText%d", i ), "" );
	}
}

bool CVGUI_MP_CreditsScreen::Init( KeyValues *pKeyValues, VGuiScreenInitData_t *pInitData )
{
	vgui::ivgui()->AddTickSignal( GetVPanel() );
	return BaseClass::Init( pKeyValues, pInitData );
}

void CVGUI_MP_CreditsScreen::OnTick( void )
{
	BaseClass::OnTick();

	if ( !m_bShownOnce )
	{
		SetVisible( true );
		m_bShownOnce = true;
	}

	UpdateCredits();
}

void CVGUI_MP_CreditsScreen::Paint( void )
{
	BaseClass::Paint();
}

template < class T >
static T *FindCreditsControl( vgui::Panel *pParent, const char *pszName )
{
	return dynamic_cast< T * >( pParent->FindChildByName( pszName ) );
}

void CVGUI_MP_CreditsScreen::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetPaintBackgroundType( 0 );
	SetPaintBackgroundEnabled( true );
	SetBgColor( Color( 16, 16, 16, 192 ) );
	SetAlpha( 255 );
	SetMouseInputEnabled( false );

	vgui::Label *pLines[ CREDITS_NAME_LINES + CREDITS_TEXT_LINES ];
	for ( int i = 0; i < CREDITS_NAME_LINES; ++i )
		pLines[ i ] = m_pCreditNames[ i ];
	for ( int i = 0; i < CREDITS_TEXT_LINES; ++i )
		pLines[ CREDITS_NAME_LINES + i ] = m_pCreditText[ i ];

	for ( int i = 0; i < ARRAYSIZE( pLines ); ++i )
	{
		pLines[ i ]->SetFont( pScheme->GetFont( "CoopCreditsFont", false ) );
		pLines[ i ]->SetFgColor( Color( 255, 135, 30, 255 ) );
		pLines[ i ]->SetText( "" );
		pLines[ i ]->SetContentAlignment( vgui::Label::a_east );
		pLines[ i ]->SetAlpha( 255 );
		pLines[ i ]->SetVisible( true );
		pLines[ i ]->SetZPos( 10 );
	}

	for ( int i = 0; i < CREDITS_DATA_DIGITS; ++i )
	{
		m_pDataDigits[ i ] = FindCreditsControl< vgui::Label >( this, VarArgs( "DataDigits%d", i ) );
		if ( m_pDataDigits[ i ] )
		{
			m_pDataDigits[ i ]->SetAlpha( 200 );
			m_pDataDigits[ i ]->SetVisible( true );
		}
	}

	m_pScanningText = FindCreditsControl< vgui::Label >( this, "ScanningText" );
	if ( !m_pScanningText )
	{
		Msg( "Can't find Label named ScanningText\n" );
		return;
	}

	m_pJobDescription = FindCreditsControl< vgui::Label >( this, "JobDescription" );
	if ( !m_pJobDescription )
	{
		Msg( "Can't find Label named JobDescription\n" );
		return;
	}

	m_pScanningLine = FindCreditsControl< vgui::ImagePanel >( this, "ScanningLine" );
	if ( m_pScanningLine )
	{
		m_pScanningLine->SetVisible( true );
		m_pScanningLine->SetDrawColor( Color( 255, 170, 60, 255 ) );
	}

	m_pBlackDataBlocker = FindCreditsControl< vgui::ImagePanel >( this, "BlackDataBlocker" );
	if ( m_pBlackDataBlocker )
	{
		m_pBlackDataBlocker->SetVisible( true );
		m_pBlackDataBlocker->SetAlpha( 0 );
	}

	m_pScanHuman = FindCreditsControl< vgui::ImagePanel >( this, "Scan_Human" );
	if ( m_pScanHuman )
	{
		m_pScanHuman->SetVisible( true );
		m_pScanHuman->SetAlpha( 0 );
	}

	m_pScanUnknown = FindCreditsControl< vgui::ImagePanel >( this, "Scan_Unknown" );
	if ( m_pScanUnknown )
	{
		m_pScanUnknown->SetVisible( true );
		m_pScanUnknown->SetAlpha( 0 );
	}

	m_bLabelsReady = true;
}

//-----------------------------------------------------------------------------
// Purpose: Eight name rows and ten text rows fill the space below the header.
//-----------------------------------------------------------------------------
void CVGUI_MP_CreditsScreen::PerformLayout( void )
{
	BaseClass::PerformLayout();

	int w, h;
	GetSize( w, h );

	m_nTopY = 94;
	m_nNameLineTall = ( h - m_nTopY ) / 6;
	int y = m_nTopY - m_nNameLineTall;
	for ( int i = 0; i < CREDITS_NAME_LINES; ++i )
	{
		m_pCreditNames[ i ]->SetBounds( 0, y, w - 12, m_nNameLineTall );
		y += m_nNameLineTall;
	}

	m_nTextLineTall = ( h - m_nTopY ) / 10;
	y = m_nTopY;
	for ( int i = 0; i < CREDITS_TEXT_LINES; ++i )
	{
		m_pCreditText[ i ]->SetBounds( 0, y, w - 12, m_nTextLineTall );
		y += m_nTextLineTall;
	}

	if ( m_pDataDigits[ 0 ] )
	{
		y = m_nTopY + 122;
		for ( int i = 0; i < CREDITS_DATA_DIGITS; ++i )
		{
			if ( m_pDataDigits[ i ] )
			{
				m_pDataDigits[ i ]->SetBounds( 0, y, w / 2 - 110, 40 );
			}
			y += 40;
		}
	}
}

void CVGUI_MP_CreditsScreen::FadeCredits( int nState )
{
	AnimationController *pAnimation = GetAnimationController();

	if ( m_pBlackDataBlocker )
	{
		if ( nState == COOP_CREDITS_NAMES )
		{
			pAnimation->RunAnimationCommand( m_pBlackDataBlocker, "alpha", 0.0f, 0.0f, 0.5f, AnimationController::INTERPOLATOR_LINEAR );
		}
		else if ( nState == COOP_CREDITS_TEXT )
		{
			pAnimation->RunAnimationCommand( m_pBlackDataBlocker, "alpha", 255.0f, 0.0f, 0.5f, AnimationController::INTERPOLATOR_LINEAR );
		}
	}

	for ( int i = 0; i < CREDITS_NAME_LINES; ++i )
	{
		pAnimation->RunAnimationCommand( m_pCreditNames[ i ], "alpha", 0.0f, 0.0f, 0.5f, AnimationController::INTERPOLATOR_LINEAR );
	}

	for ( int i = 0; i < CREDITS_TEXT_LINES; ++i )
	{
		pAnimation->RunAnimationCommand( m_pCreditText[ i ], "alpha", 0.0f, 0.0f, 0.5f, AnimationController::INTERPOLATOR_LINEAR );
	}
}

//-----------------------------------------------------------------------------
// Purpose: The newest name slides in at the top; the others move down a row
//          and the oldest moves off screen to become the next new row.
//-----------------------------------------------------------------------------
void CVGUI_MP_CreditsScreen::AddCreditName( void )
{
	CPortalMPGameRules *pRules = PortalMPGameRules();

	// The rules string is single-byte; widen it byte by byte like retail.
	wchar_t wszName[128];
	const char *pszName = pRules->GetCoopCreditsNameSingle();
	int nLength = 0;
	while ( nLength < ARRAYSIZE( wszName ) - 1 && pszName[ nLength ] )
	{
		wszName[ nLength ] = (unsigned char)pszName[ nLength ];
		++nLength;
	}
	wszName[ nLength ] = 0;

	AnimationController *pAnimation = GetAnimationController();
	vgui::Label *pNewName = m_pCreditNames[ m_nNameIndex ];
	pNewName->SetText( wszName );
	pNewName->SetAlpha( 0 );
	pAnimation->RunAnimationCommand( pNewName, "alpha", 255.0f, 0.0f, 0.25f, AnimationController::INTERPOLATOR_LINEAR );
	pAnimation->RunAnimationCommand( pNewName, "xpos", 0.0f, 0.0f, 0.25f, AnimationController::INTERPOLATOR_LINEAR );

	int nOldest = m_nNameIndex - 1;
	if ( nOldest < 0 )
		nOldest = CREDITS_NAME_LINES - 1;

	int w, h;
	GetSize( w, h );

	for ( int i = 0; i < CREDITS_NAME_LINES; ++i )
	{
		if ( i == nOldest )
		{
			m_pCreditNames[ i ]->SetPos( -w, m_nTopY );
		}
		else if ( i != m_nNameIndex )
		{
			m_pCreditNames[ i ]->SetAlpha( 255 );
			int x, y;
			m_pCreditNames[ i ]->GetPos( x, y );
			pAnimation->RunAnimationCommand( m_pCreditNames[ i ], "ypos", (float)( y + m_nNameLineTall ), 0.0f, 0.25f, AnimationController::INTERPOLATOR_LINEAR );
		}
	}

	m_nNameIndex = nOldest;
}

void CVGUI_MP_CreditsScreen::AddCreditText( void )
{
	char szText[128];
	V_snprintf( szText, sizeof( szText ), "%s", PortalMPGameRules()->GetCoopCreditsNameSingle() );

	vgui::Label *pLine = m_pCreditText[ m_nTextIndex ];
	pLine->SetText( szText );
	pLine->SetAlpha( 0 );
	GetAnimationController()->RunAnimationCommand( pLine, "alpha", 255.0f, 0.0f, 0.5f, AnimationController::INTERPOLATOR_LINEAR );

	++m_nTextIndex;
	if ( m_nTextIndex >= CREDITS_TEXT_LINES )
		m_nTextIndex = 0;
}

void CVGUI_MP_CreditsScreen::UpdateCredits( void )
{
	if ( !g_pGameRules || !g_pGameRules->IsMultiplayer() )
		return;

	CPortalMPGameRules *pRules = PortalMPGameRules();
	if ( !pRules || !pRules->IsCoOp() || !m_bLabelsReady )
		return;

	bool bFadeState = pRules->GetCoopCreditsFadeState();
	int nState = pRules->GetCoopCreditsState();
	int nIndex;
	int nScanState;

	if ( m_bLastFadeState == bFadeState )
	{
		m_nCreditsState = nState;
		nIndex = pRules->GetCoopCreditsNameIndex();
		nScanState = pRules->GetCoopCreditsScanState();
		if ( m_nLastCreditsIndex == nIndex && m_nLastScanState == nScanState )
			return;
	}
	else
	{
		m_nTextIndex = 0;
		m_nNameIndex = 0;
		FadeCredits( nState );
		m_nCreditsState = nState;
		m_bLastFadeState = bFadeState;
		nIndex = pRules->GetCoopCreditsNameIndex();
		nScanState = pRules->GetCoopCreditsScanState();
	}

	if ( m_pScanHuman )
		m_pScanHuman->SetAlpha( 0 );
	if ( m_pScanUnknown )
		m_pScanUnknown->SetAlpha( 0 );
	m_pJobDescription->SetAlpha( 0 );

	if ( nScanState > 0 )
	{
		AnimationController *pAnimation = GetAnimationController();

		int w, h;
		GetSize( w, h );
		if ( m_pScanningLine )
		{
			m_pScanningLine->SetPos( 198, 210 );
			pAnimation->RunAnimationCommand( m_pScanningLine, "ypos", (float)( h - 54 ), 0.0f, 0.25f, AnimationController::INTERPOLATOR_LINEAR );
		}

		// Scan states 1 and 2 identify a human with a job title.
		bool bHuman = ( nScanState == 1 || nScanState == 2 );
		float flDelay = 0.1f;
		if ( bHuman )
		{
			if ( m_pScanHuman )
				pAnimation->RunAnimationCommand( m_pScanHuman, "alpha", 255.0f, flDelay, 0.75f, AnimationController::INTERPOLATOR_DEACCEL );

			m_pJobDescription->SetText( pRules->GetCoopCreditsJobTitle() );
			pAnimation->RunAnimationCommand( m_pJobDescription, "alpha", 255.0f, flDelay, 0.75f, AnimationController::INTERPOLATOR_DEACCEL );
		}
		else if ( m_pScanUnknown )
		{
			pAnimation->RunAnimationCommand( m_pScanUnknown, "alpha", 255.0f, flDelay, 0.75f, AnimationController::INTERPOLATOR_DEACCEL );
		}

		m_pScanningText->SetText( "#P2COOP_Scanning" );

		for ( int i = 0; i < CREDITS_DATA_DIGITS; ++i )
		{
			if ( !m_pDataDigits[ i ] )
				continue;

			m_pDataDigits[ i ]->SetAlpha( 0 );
			if ( bHuman )
			{
				m_pDataDigits[ i ]->SetText( VarArgs( "%6d", RandomInt( 0, 999999 ) ) );
			}
			else
			{
				m_pDataDigits[ i ]->SetText( "??????" );
			}

			pAnimation->RunAnimationCommand( m_pDataDigits[ i ], "alpha", 200.0f, flDelay, 0.5f, AnimationController::INTERPOLATOR_DEACCEL );
			flDelay += 0.05f;
		}
	}

	m_nLastScanState = nScanState;

	if ( m_nLastCreditsIndex != nIndex )
	{
		m_nLastCreditsIndex = nIndex;
		if ( pRules->GetCoopCreditsState() == COOP_CREDITS_NAMES )
		{
			AddCreditName();
		}
		else if ( pRules->GetCoopCreditsState() == COOP_CREDITS_TEXT )
		{
			AddCreditText();
		}
	}
}
