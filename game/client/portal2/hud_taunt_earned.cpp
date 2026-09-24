//========= Portal 2 reconstruction ============================================//
//
// Purpose: HUD notice for gestures (taunts) that were earned but not yet used
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/AnimationController.h>
#include "hud_taunt_earned.h"
#include "radialmenu_taunt.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar cl_hud_taunt_earned_show( "cl_hud_taunt_earned_show", "0" );


//-----------------------------------------------------------------------------
// Purpose: Shows the earned gestures that haven't been performed yet
//-----------------------------------------------------------------------------
class CHUDTauntEarned : public CHudElement, public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHUDTauntEarned, vgui::EditablePanel );

public:
	CHUDTauntEarned( const char *pElementName );

	virtual void OnTick( void );

	virtual void PaintBackground( void );
	virtual void Paint( void );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:

	vgui::Label *m_pGestureLabel;
	vgui::ImagePanel *m_pGestureImage;
	Color m_rgbaCurrent;

	bool m_bActive;
};

DECLARE_HUDELEMENT( CHUDTauntEarned );


CHUDTauntEarned::CHUDTauntEarned( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "TauntEarned" )
{
	SetParent( GetClientMode()->GetViewport() );

	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	vgui::ivgui()->AddTickSignal( GetVPanel() );

	m_pGestureLabel = new vgui::Label( this, "GestureLabel", "" );
	m_pGestureImage = new vgui::ImagePanel( this, "GestureImage" );

	m_bActive = false;
}

void CHUDTauntEarned::OnTick( void )
{
	BaseClass::OnTick();

	bool bVisible = cl_hud_taunt_earned_show.GetBool();

	ClientMenuManagerTaunt &tauntManager = GetClientMenuManagerTaunt();
	int nUnusedCount = tauntManager.GetNumTauntsUnused();
	if ( nUnusedCount != 0 )
	{
		// Cycle through the unused taunts every couple of seconds
		const TauntStatusData *pTaunt = tauntManager.GetUnusedTaunt( static_cast< int >( gpGlobals->curtime * 0.5f ) % nUnusedCount );
		m_pGestureImage->SetImage( pTaunt->szIcon );

		if ( m_pGestureImage->GetNumFrames() > 1 )
		{
			// Animated icons swing back and forth through their frames
			int nFrame = static_cast< int >( MIN( ( sinf( gpGlobals->curtime * 7.5f ) + 1.0f ) * 0.5f * m_pGestureImage->GetNumFrames(),
												  static_cast< float >( m_pGestureImage->GetNumFrames() - 1 ) ) );
			m_pGestureImage->SetFrame( nFrame );
		}
		else
		{
			m_pGestureImage->SetFrame( 0 );
		}

		bVisible = true;
	}

	// Pulse the green channel
	m_rgbaCurrent = Color( 8, static_cast< int >( ( sinf( gpGlobals->curtime * 4.0f ) + 1.0f ) * 0.25f * 127.0f + 128.0f ), 16, 255 );
	m_pGestureImage->SetDrawColor( m_rgbaCurrent );
	m_pGestureLabel->SetFgColor( m_rgbaCurrent );

	if ( bVisible )
	{
		if ( !m_bActive )
		{
			CLocalPlayerFilter filter;
			C_BaseEntity::EmitSound( filter, -1, "Weapon_Portalgun.powerup" );

			m_bActive = true;

			// Pop up in the middle of the screen then slide down to the bottom left
			SetAlpha( 0 );
			SetPos( ScreenWidth() / 2 - GetWide() / 2, ScreenHeight() / 2 - GetTall() / 2 );
			GetAnimationController()->RunAnimationCommand( this, "alpha", 255.0f, 0.0f, 0.5f, AnimationController::INTERPOLATOR_DEACCEL );
			GetAnimationController()->RunAnimationCommand( this, "xpos", 0.0f, 1.5f, 0.5f, AnimationController::INTERPOLATOR_DEACCEL );
			GetAnimationController()->RunAnimationCommand( this, "ypos", static_cast< float >( ScreenHeight() - GetTall() ), 1.5f, 0.5f, AnimationController::INTERPOLATOR_DEACCEL );
		}
	}
	else
	{
		m_bActive = false;
	}

	SetVisible( bVisible );
}

void CHUDTauntEarned::Paint( void )
{
	BaseClass::Paint();
}

void CHUDTauntEarned::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/ui/hud_taunt_earned.res" );
}

void CHUDTauntEarned::PaintBackground( void )
{
	// Marching dashes around the border
	vgui::surface()->DrawSetColor( m_rgbaCurrent );

	const int nSpacing = static_cast< int >( static_cast< float >( GetWide() * GetTall() ) * 0.0006f );
	const int nSize = 3;
	const int nYBottom = GetTall() - nSize;
	const int nXRight = GetWide() - nSize;

	int nX = nSize;
	int nOffset = static_cast< int >( gpGlobals->curtime * 45.0f ) % nSpacing;

	for ( ; nX < GetWide() + nSpacing; nX += nSpacing )
	{
		int nXStart = nX + nOffset;
		if ( nXStart < nXRight - nSize )
		{
			vgui::surface()->DrawFilledRect( nXStart, nSize, nXStart + nSize, nSize * 2 );
		}

		nXStart = nX + nSpacing - nOffset;
		if ( nXStart < nXRight - nSize )
		{
			vgui::surface()->DrawFilledRect( nXStart, nYBottom - nSize, nXStart + nSize, nYBottom );
		}
	}

	for ( int nY = nSize; nY < GetTall() + nSpacing; nY += nSpacing )
	{
		int nYStart = nY + nSpacing - nOffset;
		if ( nYStart < nYBottom - nSize )
		{
			vgui::surface()->DrawFilledRect( nSize, nYStart, nSize * 2, nYStart + nSize );
		}

		nYStart = nY + nOffset;
		if ( nYStart < nYBottom - nSize )
		{
			vgui::surface()->DrawFilledRect( nXRight - nSize, nYStart, nXRight, nYStart + nSize );
		}
	}
}
