//========= Portal 2 reconstruction ============================================//
//
// Purpose: Portal 2 player data that is sent only to the owning client.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef PORTAL_PLAYERLOCALDATA_H
#define PORTAL_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"
#include "ehandle.h"
#include "paint_enum.h"
#include "portal_player_shared.h"

class CTrigger_TractorBeam;

// Number of HUD locator targets the view finder can show.
// Reconstruction note: DWARF gives the array size (16); the original name of
// this constant is unknown.
#define MAX_PORTAL_LOCATOR_ENTITIES	16

//-----------------------------------------------------------------------------
// Purpose: Player specific data for Portal 2 ( sent only to local player, too )
//-----------------------------------------------------------------------------
class CPortalPlayerLocalData
{
public:
	// Save/restore
	DECLARE_SIMPLE_DATADESC();
	// Prediction data copying
	DECLARE_CLASS_NOBASE( CPortalPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	CPortalPlayerLocalData();

	// Air control is suppressed while this is non-zero
	CNetworkVar( float, m_flAirControlSupressionTime );

	// Entities the HUD view finder draws locators for
	CNetworkArray( int, m_nLocatorEntityIndices, MAX_PORTAL_LOCATOR_ENTITIES );
	CNetworkVar( bool, m_bPlacingPhoto );

	// Paint power HUD info
	CNetworkArray( Vector, m_PaintPowerHudInfoPosition, PAINT_POWER_TYPE_COUNT );
	CNetworkArray( Vector, m_PaintPowerHudInfoNormal, PAINT_POWER_TYPE_COUNT );
	CNetworkArray( bool, m_PaintPowerHudInfoValidity, PAINT_POWER_TYPE_COUNT );
	CNetworkVector( m_StickNormal );
	CNetworkVector( m_OldStickNormal );
	CNetworkVector( m_vPreUpdateVelocity );
	CNetworkVector( m_Up );
	CNetworkVector( m_vStickRotationAxis );
	CNetworkVector( m_StandHullMin );
	CNetworkVector( m_StandHullMax );
	CNetworkVector( m_DuckHullMin );
	CNetworkVector( m_DuckHullMax );
	CNetworkVector( m_CachedStandHullMinAttempt );
	CNetworkVector( m_CachedStandHullMaxAttempt );
	CNetworkVector( m_CachedDuckHullMinAttempt );
	CNetworkVector( m_CachedDuckHullMaxAttempt );
	CNetworkVector( m_vLocalUp );
	CNetworkVector( m_vEyeOffset );
	CNetworkVector( m_vEyeUpOffset );
	CNetworkQAngle( m_qQuaternionPunch );
	CNetworkVar( PaintPowerType, m_PaintedPowerType );
	CNetworkVarEmbedded( CountdownTimer, m_PaintedPowerTimer );
	CNetworkVar( float, m_flAirInputScale );
	CNetworkVar( float, m_flCurrentStickTime );
	CNetworkVar( StickCameraState, m_nStickCameraState );
	CNetworkVar( InAirState, m_InAirState );
	CNetworkVar( bool, m_bDoneStickInterp );
	CNetworkVar( bool, m_bDoneCorrectPitch );
	CNetworkVar( bool, m_bAttemptHullResize );
	CNetworkVar( bool, m_bJumpedThisFrame );
	CNetworkVar( bool, m_bDuckedInAir );
	CNetworkVar( bool, m_bBounced );
	CNetworkVar( bool, m_bInTractorBeam );
	CNetworkVar( bool, m_bZoomedIn );

	// Reconstruction note: the members below are absent from both 2010 builds.
	// The imported retail player, movement and animation-state code uses them,
	// so they are declared with the types that code requires.
	CNetworkVar( bool, m_bShowingViewFinder );
	CNetworkHandle( CTrigger_TractorBeam, m_hTractorBeam );
	CNetworkVar( int, m_nTractorBeamCount );
	CNetworkVar( bool, m_bSlowingTime );
	CNetworkVar( float, m_flSlowTimeRemaining );
	CNetworkVar( float, m_flSlowTimeMaximum );
	CNetworkVar( bool, m_bPreventedCrouchJumpThisFrame );
	CNetworkVar( bool, m_bBouncedThisFrame );
	CNetworkVar( float, m_fBouncedTime );

	// Predicted on both sides from the same inputs; never networked.
	CachedPaintPowerChoiceResult m_CachedPaintPowerChoiceResults[PAINT_POWER_TYPE_COUNT];
};

EXTERN_SEND_TABLE( DT_PortalLocal );

#endif // PORTAL_PLAYERLOCALDATA_H
