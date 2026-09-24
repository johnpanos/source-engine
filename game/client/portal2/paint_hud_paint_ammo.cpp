//========= Portal 2 reconstruction ============================================//
//
// Purpose: Paint gun ammo bar HUD element (CHUDPaintAmmo)
//
// Reconstructed from the retail Linux client.so (stripped i386; RTTI, vtables,
// strings and Ghidra decompiler output). No original source or Steam2
// pseudocode exists for this file. Retail ShouldDraw() always returns false
// and the VPKs ship no resource/ui/hud_paint_ammo.res, so the element is
// created and updated but never drawn. Not original Valve source; the
// repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include <vgui/IVGui.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ProgressBar.h>
#include "c_weapon_paintgun.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar cl_debug_paint_ammo_bar( "cl_debug_paint_ammo_bar", "0", FCVAR_CHEAT );

// Retail divides the ammo count by this literal before clamping to [0,1].
#define PAINT_AMMO_BAR_MAX 60

//-----------------------------------------------------------------------------
// Purpose: Vertical bar showing the local player's paint gun ammo
//-----------------------------------------------------------------------------
class CHUDPaintAmmo : public CHudElement, public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CHUDPaintAmmo, EditablePanel );

public:
	CHUDPaintAmmo( const char *pElementName );

	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual bool ShouldDraw( void );

	virtual void OnTick( void );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	vgui::ContinuousProgressBar *m_pAmmoBar;
};

DECLARE_HUDELEMENT( CHUDPaintAmmo );

CHUDPaintAmmo::CHUDPaintAmmo( const char *pElementName ) : CHudElement( pElementName ), BaseClass( NULL, "PaintAmmo" )
{
	SetParent( GetClientMode()->GetViewport() );
	SetHiddenBits( HIDEHUD_PLAYERDEAD );

	m_pAmmoBar = new ContinuousProgressBar( this, "PaintAmmoBar" );
	m_pAmmoBar->SetProgressDirection( ProgressBar::PROGRESS_NORTH );

	vgui::ivgui()->AddTickSignal( GetVPanel(), 50 );
}

void CHUDPaintAmmo::Init( void )
{
	Reset();
}

void CHUDPaintAmmo::VidInit( void )
{
	Reset();
}

void CHUDPaintAmmo::Reset( void )
{
	if ( m_pAmmoBar )
	{
		m_pAmmoBar->SetBgColor( Color( 0, 0, 0, 128 ) );
		m_pAmmoBar->SetFgColor( Color( 255, 255, 255, 255 ) );
	}
}

bool CHUDPaintAmmo::ShouldDraw( void )
{
	return false;
}

void CHUDPaintAmmo::OnTick( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !pPlayer || !pPlayer->GetActiveWeapon() )
		return;

	C_WeaponPaintGun *pPaintGun = dynamic_cast< C_WeaponPaintGun * >( pPlayer->GetActiveWeapon() );
	if ( !pPaintGun )
		return;

	// Retail performs an integer division before converting to float.
	float flPercent = clamp( (float)( pPaintGun->GetPaintAmmo() / PAINT_AMMO_BAR_MAX ), 0.0f, 1.0f );
	m_pAmmoBar->SetProgress( flPercent );

	if ( cl_debug_paint_ammo_bar.GetBool() )
	{
		DevMsg( "Max: %d, Ammo: %d, Percent: %f\n", PAINT_AMMO_BAR_MAX, pPaintGun->GetPaintAmmo(), flPercent );
	}
}

void CHUDPaintAmmo::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	LoadControlSettings( "resource/ui/hud_paint_ammo.res" );
}
