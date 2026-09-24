//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client copy of the Portal 2 player data sent only to the owning
//			client.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_portal_playerlocaldata.h"
#include "gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

EXTERN_RECV_TABLE( DT_CountdownTimer );

BEGIN_RECV_TABLE_NOBASE( C_PortalPlayerLocalData, DT_PortalLocal )
	RecvPropFloat( RECVINFO( m_flAirControlSupressionTime ) ),

	RecvPropArray3( RECVINFO_ARRAY( m_nLocatorEntityIndices ), RecvPropInt( RECVINFO( m_nLocatorEntityIndices[0] ) ) ),
	RecvPropBool( RECVINFO( m_bPlacingPhoto ) ),

	RecvPropArray3( RECVINFO_ARRAY( m_PaintPowerHudInfoPosition ), RecvPropVector( RECVINFO( m_PaintPowerHudInfoPosition[0] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_PaintPowerHudInfoNormal ), RecvPropVector( RECVINFO( m_PaintPowerHudInfoNormal[0] ) ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_PaintPowerHudInfoValidity ), RecvPropBool( RECVINFO( m_PaintPowerHudInfoValidity[0] ) ) ),

	RecvPropVector( RECVINFO( m_StickNormal ) ),
	RecvPropVector( RECVINFO( m_OldStickNormal ) ),
	RecvPropVector( RECVINFO( m_vPreUpdateVelocity ) ),
	RecvPropVector( RECVINFO( m_Up ) ),
	RecvPropVector( RECVINFO( m_vStickRotationAxis ) ),
	RecvPropVector( RECVINFO( m_StandHullMin ) ),
	RecvPropVector( RECVINFO( m_StandHullMax ) ),
	RecvPropVector( RECVINFO( m_DuckHullMin ) ),
	RecvPropVector( RECVINFO( m_DuckHullMax ) ),
	RecvPropVector( RECVINFO( m_CachedStandHullMinAttempt ) ),
	RecvPropVector( RECVINFO( m_CachedStandHullMaxAttempt ) ),
	RecvPropVector( RECVINFO( m_CachedDuckHullMinAttempt ) ),
	RecvPropVector( RECVINFO( m_CachedDuckHullMaxAttempt ) ),
	RecvPropVector( RECVINFO( m_vLocalUp ) ),
	RecvPropVector( RECVINFO( m_vEyeOffset ) ),
	RecvPropVector( RECVINFO( m_vEyeUpOffset ) ),
	RecvPropQAngles( RECVINFO( m_qQuaternionPunch ) ),

	RecvPropInt( RECVINFO( m_PaintedPowerType ) ),
	RecvPropDataTable( RECVINFO_DT( m_PaintedPowerTimer ), 0, &REFERENCE_RECV_TABLE( DT_CountdownTimer ) ),

	RecvPropFloat( RECVINFO( m_flCurrentStickTime ) ),
	RecvPropFloat( RECVINFO( m_flAirInputScale ) ),
	RecvPropInt( RECVINFO( m_nStickCameraState ) ),
	RecvPropInt( RECVINFO( m_InAirState ) ),
	RecvPropBool( RECVINFO( m_bDoneStickInterp ) ),
	RecvPropBool( RECVINFO( m_bDoneCorrectPitch ) ),
	RecvPropBool( RECVINFO( m_bJumpedThisFrame ) ),
	RecvPropBool( RECVINFO( m_bDuckedInAir ) ),
	RecvPropBool( RECVINFO( m_bBounced ) ),
	RecvPropBool( RECVINFO( m_bInTractorBeam ) ),

	// Reconstruction note: retail-only props; they match the server's
	// DT_PortalLocal additions in portal_playerlocaldata.cpp.
	RecvPropBool( RECVINFO( m_bZoomedIn ) ),
	RecvPropBool( RECVINFO( m_bShowingViewFinder ) ),
	RecvPropEHandle( RECVINFO( m_hTractorBeam ) ),
	RecvPropInt( RECVINFO( m_nTractorBeamCount ) ),
	RecvPropBool( RECVINFO( m_bSlowingTime ) ),
	RecvPropFloat( RECVINFO( m_flSlowTimeRemaining ) ),
	RecvPropFloat( RECVINFO( m_flSlowTimeMaximum ) ),
	RecvPropBool( RECVINFO( m_bPreventedCrouchJumpThisFrame ) ),
	RecvPropBool( RECVINFO( m_bBouncedThisFrame ) ),
	RecvPropFloat( RECVINFO( m_fBouncedTime ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA_NO_BASE( C_PortalPlayerLocalData )
	DEFINE_PRED_FIELD( m_OldStickNormal, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_StickNormal, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD_TOL( m_vPreUpdateVelocity, FIELD_VECTOR, FTYPEDESC_INSENDTABLE, 0.5f ),
	DEFINE_PRED_FIELD( m_Up, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vStickRotationAxis, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_StandHullMin, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_StandHullMax, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_DuckHullMin, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_DuckHullMax, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_CachedStandHullMinAttempt, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_CachedStandHullMaxAttempt, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_CachedDuckHullMinAttempt, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_CachedDuckHullMaxAttempt, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vLocalUp, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vEyeOffset, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_vEyeUpOffset, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_qQuaternionPunch, FIELD_VECTOR, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flCurrentStickTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flAirInputScale, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nStickCameraState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_InAirState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bDoneStickInterp, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bDoneCorrectPitch, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bJumpedThisFrame, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bDuckedInAir, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bBounced, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bInTractorBeam, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),

	// Reconstruction note: retail-only state the shared movement code predicts.
	DEFINE_PRED_FIELD( m_hTractorBeam, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_nTractorBeamCount, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bPreventedCrouchJumpThisFrame, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bBouncedThisFrame, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_fBouncedTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()

static const Vector DEFAULT_SURFACE_NORMAL( 0, 0, 1 );

// Reconstruction note: both 2010 builds seed m_flCurrentStickTime from the
// stick_surface_transition_delay ConVar. Retail portal_player_shared.cpp keeps
// that ConVar only as a comment (default ".5f"), so its default is used here.
static const float DEFAULT_STICK_SURFACE_TRANSITION_DELAY = 0.5f;

C_PortalPlayerLocalData::C_PortalPlayerLocalData() :
	m_StickNormal( DEFAULT_SURFACE_NORMAL ),
	m_OldStickNormal( DEFAULT_SURFACE_NORMAL ),
	m_vPreUpdateVelocity( vec3_origin ),
	m_Up( DEFAULT_SURFACE_NORMAL ),
	m_vStickRotationAxis( 0, 0, 0 ),
	m_StandHullMin( VEC_HULL_MIN ),
	m_StandHullMax( VEC_HULL_MAX ),
	m_DuckHullMin( VEC_DUCK_HULL_MIN ),
	m_DuckHullMax( VEC_DUCK_HULL_MAX ),
	m_CachedStandHullMinAttempt( VEC_HULL_MIN ),
	m_CachedStandHullMaxAttempt( VEC_HULL_MAX ),
	m_CachedDuckHullMinAttempt( VEC_DUCK_HULL_MIN ),
	m_CachedDuckHullMaxAttempt( VEC_DUCK_HULL_MAX ),
	m_vLocalUp( 0, 0, 1 ),
	m_vEyeOffset( 0, 0, 0 ),
	m_vEyeUpOffset( 0, 0, 0 ),
	m_qQuaternionPunch( 0, 0, 0 ),
	m_PaintedPowerType( NO_POWER ),
	m_flAirInputScale( 1.0f ),
	m_flCurrentStickTime( DEFAULT_STICK_SURFACE_TRANSITION_DELAY ),
	m_nStickCameraState( STICK_CAMERA_UPRIGHT ),
	m_InAirState( ON_GROUND ),
	m_bDoneStickInterp( true ),
	m_bDoneCorrectPitch( true ),
	m_bAttemptHullResize( false ),
	m_bJumpedThisFrame( false ),
	m_bDuckedInAir( false ),
	m_bBounced( false ),
	m_bInTractorBeam( false ),
	m_bZoomedIn( false ),
	m_bShowingViewFinder( false ),
	m_nTractorBeamCount( 0 ),
	m_bSlowingTime( false ),
	m_flSlowTimeRemaining( 0.0f ),
	m_flSlowTimeMaximum( 0.0f ),
	m_bPreventedCrouchJumpThisFrame( false ),
	m_bBouncedThisFrame( false ),
	m_fBouncedTime( 0.0f )
{
	m_flAirControlSupressionTime = 0.0f;

	// Reconstruction note: both 2010 clients clear only MAX_PORTAL_LOCATOR_ENTITIES
	// bytes here (the first four indices); the server clears every index. All
	// indices are cleared so the client starts from the same state.
	V_memset( m_nLocatorEntityIndices, 0xFF, sizeof( m_nLocatorEntityIndices ) );

	m_bPlacingPhoto = false;

	for ( unsigned i = 0; i < PAINT_POWER_TYPE_COUNT; ++i )
	{
		m_PaintPowerHudInfoPosition[i] = vec3_origin;
		m_PaintPowerHudInfoNormal[i] = vec3_origin;
		m_PaintPowerHudInfoValidity[i] = false;
		m_CachedPaintPowerChoiceResults[i].Initialize();
	}
}
