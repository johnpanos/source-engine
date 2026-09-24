//========= Portal 2 reconstruction ============================================//
//
// Purpose: Race-mode checkpoint hologram that activates when a player touches it.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "baseanimating.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

Color checkpointActiveColor( 255, 106, 0, 255 );
Color checkpointInactiveColor( 0, 165, 255, 255 );

ConVar sv_debug_portal_race_checkpoint( "sv_debug_portal_race_checkpoint", "0", 0 );
ConVar sv_portal_race_checkpoint_model_scale( "sv_portal_race_checkpoint_model_scale", "2.0f", FCVAR_CHEAT );

#define CHECKPOINT_MODEL_NAME	"models/effects/cappoint_hologram.mdl"
const char *g_szCheckpointStartSpinningThinkContext = "PortalRaceCheckpointStartSpinningThinkContext";

//-----------------------------------------------------------------------------
// Purpose: Race checkpoint
//-----------------------------------------------------------------------------
class CPortalRaceCheckpoint : public CBaseAnimating
{
public:
	DECLARE_CLASS( CPortalRaceCheckpoint, CBaseAnimating );
	DECLARE_DATADESC();

	CPortalRaceCheckpoint();
	virtual ~CPortalRaceCheckpoint();

	virtual void Precache( void );
	virtual void Spawn( void );

	virtual void StartTouch( CBaseEntity *pOther );

	void ActivatedThink( void );
	void StartSpinningThink( void );

	void Lock( void );
	void Unlock( void );

private:
	void ActivateCheckpoint( void );

	void OnCheckpointActivated( void );
	void OnCheckpointReset( bool bFireOutput );

	COutputEvent m_OnCheckpointActivated;
	COutputEvent m_OnCheckpointReset;

	void InputActivate( inputdata_t &inputData );
	void InputCancelActivate( inputdata_t &inputData );

	void InputLock( inputdata_t &inputData );
	void InputUnlock( inputdata_t &inputData );

	float m_fResetTime;
	float m_fActivatedTime;
	float m_fLastTimerSound;

	CNetworkVar( bool, m_bCheckpointActive );
	bool m_bLocked;
};

BEGIN_DATADESC( CPortalRaceCheckpoint )
	DEFINE_FIELD( m_bCheckpointActive, FIELD_BOOLEAN ),
	DEFINE_KEYFIELD( m_fResetTime, FIELD_FLOAT, "ResetTime" ),

	DEFINE_OUTPUT( m_OnCheckpointActivated, "OnCheckpointActivated" ),
	DEFINE_OUTPUT( m_OnCheckpointReset, "OnCheckpointReset" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Activate", InputActivate ),
	DEFINE_INPUTFUNC( FIELD_VOID, "CancelActivate", InputCancelActivate ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Lock", InputLock ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Unlock", InputUnlock ),

	DEFINE_THINKFUNC( ActivatedThink ),
	DEFINE_THINKFUNC( StartSpinningThink ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( portal_race_checkpoint, CPortalRaceCheckpoint );

// Reconstruction note: neither 2010 build defines a server class for this
// entity, so m_bCheckpointActive is never sent and the client-side
// C_PortalRaceCheckpoint (c_portal_race_checkpoint.cpp) is not linked to it.
CPortalRaceCheckpoint::CPortalRaceCheckpoint()
{
	m_bCheckpointActive = false;
	m_bLocked = false;
}

CPortalRaceCheckpoint::~CPortalRaceCheckpoint()
{
}

void CPortalRaceCheckpoint::Precache( void )
{
	BaseClass::Precache();

	PrecacheModel( CHECKPOINT_MODEL_NAME );

	PrecacheScriptSound( "Portal.button_down" );
	PrecacheScriptSound( "Portal.button_up" );
	PrecacheScriptSound( "Portal.button_locked" );
	PrecacheScriptSound( "Portal.room1_TickTock" );
}

void CPortalRaceCheckpoint::Spawn( void )
{
	Precache();
	SetModel( CHECKPOINT_MODEL_NAME );

	SetMoveType( MOVETYPE_NOCLIP );
	SetSolid( SOLID_BBOX );
	SetSolidFlags( FSOLID_NOT_SOLID | FSOLID_TRIGGER );
	SetCollisionGroup( COLLISION_GROUP_PLAYER );

	SetRenderColor( checkpointInactiveColor.r(), checkpointInactiveColor.g(), checkpointInactiveColor.b() );

	// The hologram model is authored around its center; sink it so the scaled
	// model rests on the checkpoint's origin.
	float flModelScale = sv_portal_race_checkpoint_model_scale.GetFloat();
	SetModelScale( flModelScale );
	float fScaledExtent = flModelScale * 24.0f;
	const Vector vModelOffset( 0.0f, 0.0f, -fScaledExtent );
	SetAbsOrigin( GetAbsOrigin() + vModelOffset );

	SetSize( Vector( -50.0f, -50.0f, 0.0f ), Vector( 50.0f, 50.0f, 100.0f ) );

	// Stagger the spin so neighbouring checkpoints are out of phase
	SetContextThink( &CPortalRaceCheckpoint::StartSpinningThink, gpGlobals->curtime + RandomFloat( 0.0f, 0.5f ), g_szCheckpointStartSpinningThinkContext );

	BaseClass::Spawn();
}

void CPortalRaceCheckpoint::StartSpinningThink( void )
{
	ApplyLocalAngularVelocityImpulse( AngularImpulse( 0.0f, 0.0f, 180.0f ) );
}

void CPortalRaceCheckpoint::OnCheckpointActivated( void )
{
	m_bCheckpointActive = true;

	EmitSound( "Portal.button_down" );

	// Tick on the first think
	m_fLastTimerSound = gpGlobals->curtime - 1.0f;

	SetRenderColor( checkpointActiveColor.r(), checkpointActiveColor.g(), checkpointActiveColor.b() );

	m_OnCheckpointActivated.FireOutput( this, this );
}

void CPortalRaceCheckpoint::OnCheckpointReset( bool bFireOutput )
{
	m_bCheckpointActive = false;

	SetThink( NULL );

	if ( bFireOutput )
	{
		EmitSound( "Portal.button_up" );
		m_OnCheckpointReset.FireOutput( this, this );

		SetRenderColor( checkpointInactiveColor.r(), checkpointInactiveColor.g(), checkpointInactiveColor.b() );
	}
}

void CPortalRaceCheckpoint::StartTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() )
		return;

	if ( m_bLocked )
	{
		EmitSound( "Portal.button_locked" );
		return;
	}

	ActivateCheckpoint();
}

void CPortalRaceCheckpoint::ActivateCheckpoint( void )
{
	SetThink( &CPortalRaceCheckpoint::ActivatedThink );
	SetNextThink( gpGlobals->curtime + 0.1f );

	m_fActivatedTime = gpGlobals->curtime;

	OnCheckpointActivated();
}

void CPortalRaceCheckpoint::ActivatedThink( void )
{
	if ( gpGlobals->curtime - m_fActivatedTime > m_fResetTime )
	{
		OnCheckpointReset( true );
		return;
	}

	if ( gpGlobals->curtime - m_fLastTimerSound > 1.0f )
	{
		EmitSound( "Portal.room1_TickTock" );
		m_fLastTimerSound = gpGlobals->curtime;
	}

	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CPortalRaceCheckpoint::Lock( void )
{
	// Stop spinning
	QAngle angImpulse;
	AngularImpulseToQAngle( AngularImpulse( 0.0f, 0.0f, 0.0f ), angImpulse );
	SetLocalAngularVelocity( angImpulse );

	m_bLocked = true;
}

void CPortalRaceCheckpoint::InputLock( inputdata_t &inputData )
{
	Lock();
}

void CPortalRaceCheckpoint::Unlock( void )
{
	m_bLocked = false;
}

void CPortalRaceCheckpoint::InputUnlock( inputdata_t &inputData )
{
	Unlock();
}

void CPortalRaceCheckpoint::InputCancelActivate( inputdata_t &inputData )
{
	OnCheckpointReset( false );
}

void CPortalRaceCheckpoint::InputActivate( inputdata_t &inputData )
{
	ActivateCheckpoint();
}
