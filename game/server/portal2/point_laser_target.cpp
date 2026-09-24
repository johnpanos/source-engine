//========= Portal 2 reconstruction ============================================//
//
// Purpose: Laser targets, catchers and relays struck by env_portal_laser.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "point_laser_target.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( point_laser_target, CPortalLaserTarget );

BEGIN_DATADESC( CPortalLaserTarget )

	DEFINE_THINKFUNC( DisableThink ),

	DEFINE_FIELD( m_pCatcher, FIELD_CLASSPTR ),
	DEFINE_FIELD( m_bPowered, FIELD_BOOLEAN ),

	DEFINE_KEYFIELD( m_ModelName, FIELD_MODELNAME, "model" ),
	DEFINE_KEYFIELD( m_bTerminalPoint, FIELD_BOOLEAN, "terminalpoint" ),

	DEFINE_OUTPUT( m_OnPowered, "OnPowered" ),
	DEFINE_OUTPUT( m_OnUnpowered, "OnUnpowered" ),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Laser targets are tiny invisible boxes that lasers can find
//-----------------------------------------------------------------------------
void CPortalLaserTarget::Spawn()
{
	// Lasers enumerate objects along their beam
	AddFlag( FL_OBJECT );
	m_bPowered = false;

	const float flWidth = 6.0f;
	UTIL_SetSize( this, -Vector( flWidth, flWidth, flWidth ), Vector( flWidth, flWidth, flWidth ) );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Lasers "damage" the target every tick they hit it
//-----------------------------------------------------------------------------
int CPortalLaserTarget::OnTakeDamage( const CTakeDamageInfo &info )
{
	if ( !m_bPowered )
	{
		// Only lasers power us
		if ( !FClassnameIs( info.GetAttacker(), "env_portal_laser" ) )
			return 0;

		m_bPowered = true;
		m_OnPowered.FireOutput( this, this );

		if ( m_pCatcher )
		{
			m_pCatcher->OnPowered();
		}

		SetThink( &CPortalLaserTarget::DisableThink );
	}

	// Stay powered as long as a laser keeps hitting us
	SetNextThink( gpGlobals->curtime + 0.1f );

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: No laser has hit us recently, power down
//-----------------------------------------------------------------------------
void CPortalLaserTarget::DisableThink()
{
	m_bPowered = false;
	m_OnUnpowered.FireOutput( this, this );

	if ( m_pCatcher )
	{
		m_pCatcher->OnUnPowered();
	}

	SetThink( NULL );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CPortalLaserTarget::IsPowered()
{
	return m_bPowered;
}


//=============================================================================
// Laser catcher
//=============================================================================

LINK_ENTITY_TO_CLASS( prop_laser_catcher, CLaserCatcher );

BEGIN_DATADESC( CLaserCatcher )

	DEFINE_FIELD( m_pCatcherLaserTarget, FIELD_CLASSPTR ),

	DEFINE_THINKFUNC( AnimateThink ),

	DEFINE_FIELD( m_IdleSequence, FIELD_INTEGER ),
	DEFINE_FIELD( m_PowerOnSequence, FIELD_INTEGER ),
	DEFINE_FIELD( m_iTargetAttachment, FIELD_INTEGER ),
	DEFINE_FIELD( m_iPowerState, FIELD_INTEGER ),

	DEFINE_OUTPUT( m_OnPowered, "OnPowered" ),
	DEFINE_OUTPUT( m_OnUnpowered, "OnUnpowered" ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CLaserCatcher::CLaserCatcher()
{
	m_iPowerState = POWER_STATE_IDLE;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLaserCatcher::Precache()
{
	PrecacheModel( GetCatcherModelName() );

	PrecacheParticleSystem( "laser_relay_powered" );

	PrecacheScriptSound( "prop_laser_catcher.poweron" );
	PrecacheScriptSound( "prop_laser_catcher.poweroff" );
	PrecacheScriptSound( "prop_laser_catcher.powerloop" );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLaserCatcher::Spawn()
{
	Precache();

	BaseClass::Spawn();

	SetModel( GetCatcherModelName() );

	SetSolid( SOLID_VPHYSICS );
	SetMoveType( MOVETYPE_VPHYSICS );

	m_IdleSequence = LookupSequence( "idle" );
	m_PowerOnSequence = LookupSequence( "spin" );
	m_iTargetAttachment = LookupAttachment( "laser_target" );

	// Start out spinning at rest
	ResetSequence( m_PowerOnSequence );
	SetPlaybackRate( 0.0f );

	CreateHelperEntities();

	CreateVPhysics();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CLaserCatcher::CreateVPhysics()
{
	VPhysicsInitStatic();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLaserCatcher::Activate()
{
	BaseClass::Activate();

	SetThink( &CLaserCatcher::AnimateThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: Spin the catcher up or down depending on the power state
//-----------------------------------------------------------------------------
void CLaserCatcher::AnimateThink()
{
	if ( PoweringDown() )
	{
		SetPlaybackRate( SpinDown( GetPlaybackRate() ) );
	}

	if ( PoweringUp() )
	{
		SetPlaybackRate( SpinUp( GetPlaybackRate() ) );
	}

	StudioFrameAdvance();
	DispatchAnimEvents( this );

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CLaserCatcher::UpdateOnRemove()
{
	if ( m_pCatcherLaserTarget )
	{
		UTIL_Remove( m_pCatcherLaserTarget );
	}

	StopSound( "prop_laser_catcher.powerloop" );
	StopParticleEffects( this );

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: Our target was struck by a laser
//-----------------------------------------------------------------------------
void CLaserCatcher::OnPowered()
{
	m_OnPowered.FireOutput( this, this );

	DispatchParticleEffect( "laser_relay_powered", PATTACH_POINT_FOLLOW, this, "particle_emitter" );

	EmitSound( "prop_laser_catcher.poweron" );
	EmitSound( "prop_laser_catcher.powerloop" );
	StopSound( "prop_laser_catcher.poweroff" );

	m_iPowerState = POWER_STATE_POWERING_UP;

	m_nSkin = 1;
}

//-----------------------------------------------------------------------------
// Purpose: Our target is no longer struck by a laser
//-----------------------------------------------------------------------------
void CLaserCatcher::OnUnPowered()
{
	m_OnUnpowered.FireOutput( this, this );

	StopParticleEffects( this );

	EmitSound( "prop_laser_catcher.poweroff" );
	StopSound( "prop_laser_catcher.powerloop" );

	m_iPowerState = POWER_STATE_POWERING_DOWN;

	m_nSkin = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Create the point_laser_target that lasers actually strike
//-----------------------------------------------------------------------------
void CLaserCatcher::CreateHelperEntities()
{
	if ( m_pCatcherLaserTarget == NULL )
	{
		Vector vecOrigin;
		GetAttachment( m_iTargetAttachment, vecOrigin );

		m_pCatcherLaserTarget = static_cast< CPortalLaserTarget* >( CreateEntityByName( "point_laser_target" ) );
		m_pCatcherLaserTarget->SetAbsOrigin( vecOrigin );
		m_pCatcherLaserTarget->SetAbsAngles( GetAbsAngles() );
		m_pCatcherLaserTarget->KeyValue( "terminalpoint", IsTerminalPoint() );
		DispatchSpawn( m_pCatcherLaserTarget );

		m_pCatcherLaserTarget->SetParent( this );
		m_pCatcherLaserTarget->SetCatcher( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CLaserCatcher::GetCatcherModelName()
{
	return ( STRING( GetModelName() )[0] != '\0' ) ? STRING( GetModelName() ) : "models/props/laser_catcher.mdl";
}

//-----------------------------------------------------------------------------
// Purpose: Catchers stop the laser
//-----------------------------------------------------------------------------
bool CLaserCatcher::IsTerminalPoint()
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CLaserCatcher::SpinDown( float flSpinRate )
{
	// Reconstruction note: the binary works on the magnitude of the current rate.
	float flNewSpinRate = fabs( flSpinRate ) - 0.1;

	if ( flNewSpinRate <= 0.0f )
	{
		m_iPowerState = POWER_STATE_IDLE;
		return 0.0f;
	}

	return flNewSpinRate;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CLaserCatcher::SpinUp( float flSpinRate )
{
	float flNewSpinRate = fabs( flSpinRate ) + 0.1;

	if ( flNewSpinRate >= 1.0f )
	{
		m_iPowerState = POWER_STATE_POWERING_UP;
		flNewSpinRate = 1.0f;
	}

	return flNewSpinRate;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CLaserCatcher::PoweringDown()
{
	return ( m_iPowerState == POWER_STATE_POWERING_DOWN );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CLaserCatcher::PoweringUp()
{
	return ( m_iPowerState == POWER_STATE_POWERING_UP );
}

//-----------------------------------------------------------------------------
// Purpose: Restore the target's catcher pointer and powered effects
//-----------------------------------------------------------------------------
void CLaserCatcher::OnRestore()
{
	BaseClass::OnRestore();

	if ( m_pCatcherLaserTarget )
	{
		m_pCatcherLaserTarget->SetCatcher( this );

		if ( m_pCatcherLaserTarget->IsPowered() )
		{
			EmitSound( "prop_laser_catcher.powerloop" );
			DispatchParticleEffect( "laser_relay_powered", PATTACH_POINT_FOLLOW, this, "particle_emitter" );
		}
	}
}


//=============================================================================
// Laser relay
//=============================================================================

LINK_ENTITY_TO_CLASS( prop_laser_relay, CLaserRelay );

BEGIN_DATADESC( CLaserRelay )
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *CLaserRelay::GetCatcherModelName()
{
	return ( STRING( GetModelName() )[0] != '\0' ) ? STRING( GetModelName() ) : "models/props/laser_receptacle.mdl";
}

//-----------------------------------------------------------------------------
// Purpose: Relays let the laser pass through
//-----------------------------------------------------------------------------
bool CLaserRelay::IsTerminalPoint()
{
	return false;
}
