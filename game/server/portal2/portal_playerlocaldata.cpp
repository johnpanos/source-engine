//========= Portal 2 reconstruction ============================================//
//
// Purpose: Portal 2 player data that is sent only to the owning client.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal_playerlocaldata.h"
#include "gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

EXTERN_SEND_TABLE( DT_CountdownTimer );

BEGIN_SEND_TABLE_NOBASE( CPortalPlayerLocalData, DT_PortalLocal )
	SendPropFloat( SENDINFO( m_flAirControlSupressionTime ) ),

	SendPropArray3( SENDINFO_ARRAY3( m_nLocatorEntityIndices ), SendPropInt( SENDINFO_ARRAY( m_nLocatorEntityIndices ) ) ),
	SendPropBool( SENDINFO( m_bPlacingPhoto ) ),

	SendPropArray3( SENDINFO_ARRAY3( m_PaintPowerHudInfoPosition ), SendPropVector( SENDINFO_ARRAY( m_PaintPowerHudInfoPosition ) ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_PaintPowerHudInfoNormal ), SendPropVector( SENDINFO_ARRAY( m_PaintPowerHudInfoNormal ) ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_PaintPowerHudInfoValidity ), SendPropBool( SENDINFO_ARRAY( m_PaintPowerHudInfoValidity ) ) ),

	SendPropVector( SENDINFO( m_StickNormal ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_OldStickNormal ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vPreUpdateVelocity ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_Up ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vStickRotationAxis ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_StandHullMin ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_StandHullMax ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_DuckHullMin ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_DuckHullMax ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_CachedStandHullMinAttempt ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_CachedStandHullMaxAttempt ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_CachedDuckHullMinAttempt ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_CachedDuckHullMaxAttempt ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vLocalUp ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vEyeOffset ), 0, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vEyeUpOffset ), 0, SPROP_NOSCALE ),
	SendPropQAngles( SENDINFO( m_qQuaternionPunch ), 0, SPROP_NOSCALE ),

	SendPropInt( SENDINFO( m_PaintedPowerType ) ),
	SendPropDataTable( SENDINFO_DT( m_PaintedPowerTimer ), &REFERENCE_SEND_TABLE( DT_CountdownTimer ) ),

	SendPropFloat( SENDINFO( m_flCurrentStickTime ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flAirInputScale ), 0, SPROP_NOSCALE ),
	SendPropInt( SENDINFO( m_nStickCameraState ) ),
	SendPropInt( SENDINFO( m_InAirState ) ),
	SendPropBool( SENDINFO( m_bDoneStickInterp ) ),
	SendPropBool( SENDINFO( m_bDoneCorrectPitch ) ),
	SendPropBool( SENDINFO( m_bJumpedThisFrame ) ),
	SendPropBool( SENDINFO( m_bDuckedInAir ) ),
	SendPropBool( SENDINFO( m_bBounced ) ),
	SendPropBool( SENDINFO( m_bInTractorBeam ) ),

	// Reconstruction note: neither 2010 table sends the props below (841 declares
	// m_bZoomedIn without sending it). The retail client reads them through
	// C_Portal_Player (IsZoomed, IsSlowingTime, IsShowingViewFinder,
	// GetTractorBeam) and predicts the rest in the shared movement code, so they
	// are sent to the owning client; the client table matches.
	SendPropBool( SENDINFO( m_bZoomedIn ) ),
	SendPropBool( SENDINFO( m_bShowingViewFinder ) ),
	SendPropEHandle( SENDINFO( m_hTractorBeam ) ),
	SendPropInt( SENDINFO( m_nTractorBeamCount ) ),
	SendPropBool( SENDINFO( m_bSlowingTime ) ),
	SendPropFloat( SENDINFO( m_flSlowTimeRemaining ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flSlowTimeMaximum ), 0, SPROP_NOSCALE ),
	SendPropBool( SENDINFO( m_bPreventedCrouchJumpThisFrame ) ),
	SendPropBool( SENDINFO( m_bBouncedThisFrame ) ),
	SendPropFloat( SENDINFO( m_fBouncedTime ), 0, SPROP_NOSCALE ),
END_SEND_TABLE()

BEGIN_SIMPLE_DATADESC( CPortalPlayerLocalData )
	DEFINE_FIELD( m_flAirControlSupressionTime, FIELD_FLOAT ),
	DEFINE_AUTO_ARRAY( m_nLocatorEntityIndices, FIELD_INTEGER ),
	DEFINE_FIELD( m_bPlacingPhoto, FIELD_BOOLEAN ),
	DEFINE_AUTO_ARRAY( m_PaintPowerHudInfoPosition, FIELD_VECTOR ),
	DEFINE_AUTO_ARRAY( m_PaintPowerHudInfoNormal, FIELD_VECTOR ),
	DEFINE_AUTO_ARRAY( m_PaintPowerHudInfoValidity, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_StickNormal, FIELD_VECTOR ),
	DEFINE_FIELD( m_OldStickNormal, FIELD_VECTOR ),
	DEFINE_FIELD( m_vPreUpdateVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( m_Up, FIELD_VECTOR ),
	DEFINE_FIELD( m_vStickRotationAxis, FIELD_VECTOR ),
	DEFINE_FIELD( m_StandHullMin, FIELD_VECTOR ),
	DEFINE_FIELD( m_StandHullMax, FIELD_VECTOR ),
	DEFINE_FIELD( m_DuckHullMin, FIELD_VECTOR ),
	DEFINE_FIELD( m_DuckHullMax, FIELD_VECTOR ),
	DEFINE_FIELD( m_CachedStandHullMinAttempt, FIELD_VECTOR ),
	DEFINE_FIELD( m_CachedStandHullMaxAttempt, FIELD_VECTOR ),
	DEFINE_FIELD( m_CachedDuckHullMinAttempt, FIELD_VECTOR ),
	DEFINE_FIELD( m_CachedDuckHullMaxAttempt, FIELD_VECTOR ),
	DEFINE_FIELD( m_vLocalUp, FIELD_VECTOR ),
	DEFINE_FIELD( m_vEyeOffset, FIELD_VECTOR ),
	DEFINE_FIELD( m_vEyeUpOffset, FIELD_VECTOR ),
	DEFINE_FIELD( m_qQuaternionPunch, FIELD_VECTOR ),
	DEFINE_FIELD( m_PaintedPowerType, FIELD_INTEGER ),
	DEFINE_FIELD( m_flAirInputScale, FIELD_FLOAT ),
	DEFINE_FIELD( m_flCurrentStickTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_nStickCameraState, FIELD_INTEGER ),
	DEFINE_FIELD( m_InAirState, FIELD_INTEGER ),
	DEFINE_FIELD( m_bDoneStickInterp, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDoneCorrectPitch, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bAttemptHullResize, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bJumpedThisFrame, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDuckedInAir, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bBounced, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bInTractorBeam, FIELD_BOOLEAN ),
END_DATADESC()

static const Vector DEFAULT_SURFACE_NORMAL( 0, 0, 1 );

// Reconstruction note: both 2010 builds seed m_flCurrentStickTime from the
// stick_surface_transition_delay ConVar. Retail portal_player_shared.cpp keeps
// that ConVar only as a comment (default ".5f"), so its default is used here.
static const float DEFAULT_STICK_SURFACE_TRANSITION_DELAY = 0.5f;

CPortalPlayerLocalData::CPortalPlayerLocalData()
{
	m_flAirControlSupressionTime = 0.0f;

	for ( int i = 0; i < MAX_PORTAL_LOCATOR_ENTITIES; ++i )
	{
		m_nLocatorEntityIndices.Set( i, -1 );
	}

	m_bPlacingPhoto = false;

	for ( unsigned i = 0; i < PAINT_POWER_TYPE_COUNT; ++i )
	{
		m_PaintPowerHudInfoPosition.Set( i, vec3_origin );
		m_PaintPowerHudInfoNormal.Set( i, vec3_origin );
		m_PaintPowerHudInfoValidity.Set( i, false );
	}

	m_StickNormal = DEFAULT_SURFACE_NORMAL;
	m_OldStickNormal = DEFAULT_SURFACE_NORMAL;
	m_vPreUpdateVelocity = vec3_origin;
	m_Up = DEFAULT_SURFACE_NORMAL;
	m_vStickRotationAxis = Vector( 0, 0, 0 );

	m_StandHullMin = VEC_HULL_MIN;
	m_StandHullMax = VEC_HULL_MAX;
	m_DuckHullMin = VEC_DUCK_HULL_MIN;
	m_DuckHullMax = VEC_DUCK_HULL_MAX;
	m_CachedStandHullMinAttempt = VEC_HULL_MIN;
	m_CachedStandHullMaxAttempt = VEC_HULL_MAX;
	m_CachedDuckHullMinAttempt = VEC_DUCK_HULL_MIN;
	m_CachedDuckHullMaxAttempt = VEC_DUCK_HULL_MAX;
	m_bAttemptHullResize = false;

	m_vLocalUp = Vector( 0, 0, 1 );
	m_vEyeOffset = Vector( 0, 0, 0 );
	m_vEyeUpOffset = Vector( 0, 0, 0 );
	m_qQuaternionPunch = QAngle( 0, 0, 0 );

	m_PaintedPowerType = NO_POWER;
	m_PaintedPowerTimer.Invalidate();

	m_flAirInputScale = 1.0f;
	m_flCurrentStickTime = DEFAULT_STICK_SURFACE_TRANSITION_DELAY;

	m_nStickCameraState = STICK_CAMERA_UPRIGHT;

	m_InAirState = ON_GROUND;
	m_bJumpedThisFrame = false;

	m_bDoneStickInterp = true;
	m_bDoneCorrectPitch = true;

	m_bDuckedInAir = false;

	m_bBounced = false;

	m_bInTractorBeam = false;

	// Retail-only state (see portal_playerlocaldata.h).
	m_bZoomedIn = false;
	m_bShowingViewFinder = false;
	m_hTractorBeam = NULL;
	m_nTractorBeamCount = 0;
	m_bSlowingTime = false;
	m_flSlowTimeRemaining = 0.0f;
	m_flSlowTimeMaximum = 0.0f;
	m_bPreventedCrouchJumpThisFrame = false;
	m_bBouncedThisFrame = false;
	m_fBouncedTime = 0.0f;

	for ( int i = 0; i < PAINT_POWER_TYPE_COUNT; ++i )
	{
		m_CachedPaintPowerChoiceResults[i].Initialize();
	}
}
