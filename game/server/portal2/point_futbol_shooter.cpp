//========= Portal 2 reconstruction ============================================//
//
// Purpose: Launches an exploding futbol at a target along a ballistic arc.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "props.h"
#include "movevars_shared.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar futbol_shooter_distance_from_target( "futbol_shooter_distance_from_target", "64", FCVAR_CHEAT );
ConVar futbol_shooter_target_height_offset( "futbol_shooter_target_height_offset", "0", FCVAR_CHEAT );


class CPointFutbolShooter : public CPointEntity
{
public:
	DECLARE_CLASS( CPointFutbolShooter, CPointEntity );
	DECLARE_DATADESC();

	virtual void Spawn( void );
	virtual void Precache( void );

protected:

	void InputShootFutbol( inputdata_t &data );
	void InputSetTarget( inputdata_t &data );

	COutputEvent m_OutputShootSuccess;
	COutputEvent m_OutputShootFail;

	float m_flPhysicsSpeed;
	Vector m_vTarget;
};

BEGIN_DATADESC( CPointFutbolShooter )

	DEFINE_KEYFIELD( m_flPhysicsSpeed, FIELD_FLOAT, "LaunchSpeed" ),

	DEFINE_INPUTFUNC( FIELD_STRING, "SetTarget", InputSetTarget ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ShootFutbol", InputShootFutbol ),

	DEFINE_OUTPUT( m_OutputShootSuccess, "OnShootSuccess" ),
	DEFINE_OUTPUT( m_OutputShootFail, "OnShootFail" ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( point_futbol_shooter, CPointFutbolShooter );


void CPointFutbolShooter::Precache( void )
{
	UTIL_PrecacheOther( "prop_exploding_futbol" );
	PrecacheParticleSystem( "command_target_ping" );
}


void CPointFutbolShooter::Spawn( void )
{
	BaseClass::Spawn();
	Precache();
}


void CPointFutbolShooter::InputShootFutbol( inputdata_t &data )
{
	Vector vecSourcePos = GetAbsOrigin();
	Vector vecTargetPos = m_vTarget;
	Vector vecVelocity = vecTargetPos - vecSourcePos;

	// Aim short of the target so the ball lands in front of it
	if ( futbol_shooter_distance_from_target.GetFloat() > 0.0f )
	{
		Vector vecToTarget = vecVelocity;
		vecToTarget.z = 0.0f;
		vecToTarget.NormalizeInPlace();
		vecTargetPos -= vecToTarget * futbol_shooter_distance_from_target.GetFloat();
	}

	// Can't shoot behind us
	Vector vecForward;
	AngleVectors( GetAbsAngles(), &vecForward );
	if ( DotProduct2D( vecVelocity.AsVector2D(), vecForward.AsVector2D() ) < 0.0f )
	{
		m_OutputShootFail.FireOutput( data.pActivator, data.pCaller );
		return;
	}

	CBaseEntity *pFutbol = CreateEntityByName( "prop_exploding_futbol" );
	if ( pFutbol == NULL )
	{
		DevMsg( "Failed to create exploding futbol\n!" );
		return;
	}

	pFutbol->SetAbsOrigin( vecSourcePos );
	pFutbol->Spawn();

	// Ballistic launch that reaches the target at the launch speed
	float flSpeed = m_flPhysicsSpeed;
	float flGravity = sv_gravity.GetFloat();

	float time = vecVelocity.Length() / flSpeed;
	vecVelocity = vecVelocity * ( 1.0f / time );
	vecVelocity.z += 0.5 * ( flGravity * time );

	IPhysicsObject *pPhysObject = pFutbol->VPhysicsGetObject();
	if ( pPhysObject )
	{
		AngularImpulse angImpulse = RandomAngularImpulse( -150.0f, 150.0f );
		pPhysObject->SetVelocityInstantaneous( &vecVelocity, &angImpulse );

		float flNull = 0.0f;
		pPhysObject->SetDragCoefficient( &flNull, &flNull );
		pPhysObject->SetDamping( &flNull, &flNull );

		// Treat it as launched so it arms like a thrown ball
		CPhysicsProp *pProp = dynamic_cast<CPhysicsProp*>( pFutbol );
		if ( pProp )
		{
			pProp->OnPhysGunDrop( UTIL_GetLocalPlayer(), LAUNCHED_BY_CANNON );
		}
	}

	m_OutputShootSuccess.FireOutput( data.pActivator, data.pCaller );

	// Mark the landing spot
	Vector color( 255, 0, 0 );
	Vector vecReticulePos = vecTargetPos;
	vecReticulePos.z -= futbol_shooter_target_height_offset.GetFloat();
	DispatchParticleEffect( "command_target_ping", vecReticulePos, color, vec3_angle );
}


void CPointFutbolShooter::InputSetTarget( inputdata_t &data )
{
	EHANDLE hTarget = gEntList.FindEntityByName( NULL, data.value.String() );
	CBaseEntity *pTargetEnt = hTarget.Get();
	if ( pTargetEnt == NULL )
	{
		DevMsg( "Invalid target entity!\n" );
		return;
	}

	m_vTarget = pTargetEnt->GetAbsOrigin();
}
