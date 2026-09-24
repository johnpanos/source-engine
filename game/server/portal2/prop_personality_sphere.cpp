//========= Portal 2 reconstruction ============================================//
//
// Purpose: Talking personality sphere prop and its info_target look-at markers.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "props.h"
#include "portal_player.h"
#include "ai_utils.h"
#include "world.h"
#include "fmtstr.h"
#include "saverestore_utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static const char *s_pAnimateThinkContext = "Animate";

#define PERSONALITY_SPHERE_MODEL	"models/npcs/personality_sphere/personality_sphere.mdl"

//-----------------------------------------------------------------------------
// Events the sphere can speak about. Each event has a line family in
// g_LineNames (numbered for the Richard core, single for the others).
//-----------------------------------------------------------------------------
enum SphereEvent_e
{
	SPHERE_EVENT_INFO_TARGET = 0,
	SPHERE_EVENT_WAITING,
	SPHERE_EVENT_HELD,
	SPHERE_EVENT_HELD_PANIC,
	SPHERE_EVENT_DROP,
	SPHERE_EVENT_NOTHELD,
	SPHERE_EVENT_NOTHELD_RELIEVED,
	SPHERE_EVENT_FIRST_PICKUP,
	SPHERE_EVENT_FRUSTRATION,
	SPHERE_EVENT_HURT,
	SPHERE_EVENT_HURT_REPEATEDLY,
	SPHERE_EVENT_VELOCITY_UP_HIGH,
	SPHERE_EVENT_VELOCITY_DOWN_HIGH,
	SPHERE_EVENT_VELOCITY_LATERAL_LOW,
	SPHERE_EVENT_VELOCITY_LATERAL_HIGH,

	SPHERE_EVENT_TOTAL
};

const char *g_LineNames[SPHERE_EVENT_TOTAL] =
{
	"InfoTarget",		// SPHERE_EVENT_INFO_TARGET
	"MUTTER",			// SPHERE_EVENT_WAITING
	"HELD",				// SPHERE_EVENT_HELD
	"HELDPANIC",		// SPHERE_EVENT_HELD_PANIC
	"DROP",				// SPHERE_EVENT_DROP
	"NOTHELD",			// SPHERE_EVENT_NOTHELD
	"UNHELDRELIEF",		// SPHERE_EVENT_NOTHELD_RELIEVED
	"PICKUPFIRST",		// SPHERE_EVENT_FIRST_PICKUP
	"FRUSTRATION",		// SPHERE_EVENT_FRUSTRATION
	"PAIN",				// SPHERE_EVENT_HURT
	"BASHING",			// SPHERE_EVENT_HURT_REPEATEDLY
	"WHOOP",			// SPHERE_EVENT_VELOCITY_UP_HIGH
	"WHOOP",			// SPHERE_EVENT_VELOCITY_DOWN_HIGH
	"WHOOP",			// SPHERE_EVENT_VELOCITY_LATERAL_LOW
	"WHOOP",			// SPHERE_EVENT_VELOCITY_LATERAL_HIGH
};


//-----------------------------------------------------------------------------
// Purpose: Marker the sphere comments on when the player looks at it
//-----------------------------------------------------------------------------
class CInfoTargetPersonalitySphere : public CPointEntity
{
	DECLARE_CLASS( CInfoTargetPersonalitySphere, CPointEntity );
public:
	float GetTargetRadius( void ) { return m_flRadius; }
	string_t GetSphereLine( void ) { return m_strSphereLine; }

private:
	DECLARE_DATADESC();

	virtual void Spawn( void ) { Precache(); BaseClass::Spawn(); }
	virtual void Precache( void ) { PrecacheScriptSound( STRING( m_strSphereLine ) ); }

	string_t m_strSphereLine;
	float m_flRadius;
};

LINK_ENTITY_TO_CLASS( info_target_personality_sphere, CInfoTargetPersonalitySphere );

BEGIN_DATADESC( CInfoTargetPersonalitySphere )
	DEFINE_KEYFIELD( m_strSphereLine, FIELD_STRING, "sphereLine" ),
	DEFINE_KEYFIELD( m_flRadius, FIELD_FLOAT, "radius" ),
END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Finds the info target closest to the center of a view ray, within
//			each target's radius
//-----------------------------------------------------------------------------
class CInfoTargetEntityEnumerator : public IEntityEnumerator
{
public:
	CInfoTargetEntityEnumerator( const Ray_t &ray )
		: m_pBestEnt( NULL ),
		m_ray( ray ),
		// Reconstruction note: m_flClosestDist is only written by the 2010
		// build; its initial value is not recoverable and FLT_MAX is assumed.
		m_flClosestDist( FLT_MAX ),
		m_flLargestDot( -1.0f )
	{
	}

	virtual bool EnumEntity( IHandleEntity *pHandleEntity )
	{
		CBaseEntity *pEnt = gEntList.GetBaseEntity( pHandleEntity->GetRefEHandle() );
		if ( pEnt == NULL )
			return true;

		if ( !FClassnameIs( pEnt, "info_target_personality_sphere" ) )
			return true;

		Vector vDirToEnt = pEnt->GetAbsOrigin() - m_ray.m_Start;
		float flDist = VectorNormalize( vDirToEnt );

		Vector vRayDir = m_ray.m_Delta;
		VectorNormalize( vRayDir );

		float flDot = vDirToEnt.Dot( vRayDir );

		// The target radius defines the cone it can be seen in from here
		float flToleranceAngle = atan2( static_cast<CInfoTargetPersonalitySphere*>( pEnt )->GetTargetRadius(), flDist );
		float flTargetAngle = acosf( flDot );
		if ( flDot > m_flLargestDot && flTargetAngle < flToleranceAngle )
		{
			m_flClosestDist = flDist;
			m_flLargestDot = flDot;
			m_pBestEnt = pEnt;
		}

		return true;
	}

	CBaseEntity *GetBestInfoTargetEntity( void ) { return m_pBestEnt; }

private:
	CBaseEntity *m_pBestEnt;
	const Ray_t &m_ray;
	float m_flClosestDist;
	float m_flLargestDot;
};


//-----------------------------------------------------------------------------
// Purpose: Personality sphere
//-----------------------------------------------------------------------------
class CPropPersonalitySphere : public CPhysicsProp
{
public:
	DECLARE_CLASS( CPropPersonalitySphere, CPhysicsProp );
	DECLARE_DATADESC();

	CPropPersonalitySphere();
	virtual ~CPropPersonalitySphere();

	// Reconstruction note: this enum is unnamed in the 2010 DWARF.
	enum CoreType_t
	{
		CORETYPE_CURIOUS = 0,
		CORETYPE_AGGRESSIVE,
		CORETYPE_CRAZY,
		CORETYPE_RICHARD,
		CORETYPE_AQUARIUM,
		CORETYPE_NONE,

		CORETYPE_TOTAL
	};

	virtual void Spawn( void );
	virtual void Precache( void );
	void RegisterSoundEvent( SphereEvent_e eventName, int count );

	virtual QAngle PreferredCarryAngles( void ) { return QAngle( 180.0f, -90.0f, 180.0f ); }
	virtual bool HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer ) { return true; }

	// Inputs
	void InputPanic( inputdata_t &inputdata ) { m_bPanicked = true; }
	void InputStartTalking( inputdata_t &inputdata );
	void StartTalking( float flDelay );

	void CheckForInfoTargets( void );
	void UpdatePositions( void );

	void TalkingThink( void );
	void AnimateThink( void );

	void SetupVOList( void );

	void TrySpeakLine( int line, int priority );

	virtual void OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
	virtual void OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason );

	virtual void VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );

	virtual void NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params );

	virtual int ObjectCaps( void ) { return BaseClass::ObjectCaps() | FCAP_IMPULSE_USE; }

private:
	// Which event line families this core has
	bool m_bHasEvent[SPHERE_EVENT_TOTAL];
	int m_iEyeballAttachment;
	float m_flBetweenVOPadding;
	bool m_bFirstPickup;
	bool m_bHeld;
	bool m_bPanicked;
	float m_flFirstNoSkipTime;
	float m_flLineCompletionTime;
	float m_flInfoTargetTime;
	float m_flIdleWaitTime;

	float m_flLastHeldTime;

	float m_flAbuseLevel;

	CUtlVector<Vector> m_pastPositions;

	CUtlVector< CUtlVector<int> > m_iLineOrder;

	int m_iCurrentLine;
	int m_iCurrentPriority;

	int m_iPendingLine;
	int m_iPendingPriority;

	string_t m_iszInfoTargetScriptName;
	string_t m_iszLastLinePlayed;

	string_t m_iszDeathSoundScriptName;
	string_t m_iszLookAnimationName;
	string_t m_iszVoiceName;

	CoreType_t m_iCoreType;
};

LINK_ENTITY_TO_CLASS( prop_personality_sphere, CPropPersonalitySphere );

BEGIN_DATADESC( CPropPersonalitySphere )

	DEFINE_FIELD( m_iEyeballAttachment, FIELD_INTEGER ),
	DEFINE_FIELD( m_iCurrentLine, FIELD_INTEGER ),
	DEFINE_FIELD( m_iCurrentPriority, FIELD_INTEGER ),
	DEFINE_FIELD( m_iszLastLinePlayed, FIELD_STRING ),
	DEFINE_FIELD( m_iszInfoTargetScriptName, FIELD_STRING ),
	DEFINE_FIELD( m_iszDeathSoundScriptName, FIELD_STRING ),
	DEFINE_FIELD( m_iszLookAnimationName, FIELD_STRING ),
	DEFINE_FIELD( m_iszVoiceName, FIELD_STRING ),
	DEFINE_FIELD( m_flLineCompletionTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_flFirstNoSkipTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_flIdleWaitTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_flLastHeldTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_flAbuseLevel, FIELD_FLOAT ),
	DEFINE_UTLVECTOR( m_pastPositions, FIELD_VECTOR ),
	DEFINE_FIELD( m_bFirstPickup, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bHeld, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bPanicked, FIELD_BOOLEAN ),

	DEFINE_KEYFIELD( m_iCoreType, FIELD_INTEGER, "CoreType" ),
	DEFINE_KEYFIELD( m_flBetweenVOPadding, FIELD_FLOAT, "DelayBetweenLines" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "StartTalking", InputStartTalking ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Panic", InputPanic ),

	DEFINE_THINKFUNC( TalkingThink ),
	DEFINE_THINKFUNC( AnimateThink ),

END_DATADESC()


CPropPersonalitySphere::CPropPersonalitySphere()
	: m_flBetweenVOPadding( 2.5f ),
	m_bFirstPickup( true ),
	m_bHeld( false ),
	m_bPanicked( false ),
	m_flIdleWaitTime( 4.0f ),
	m_flAbuseLevel( 0.0f ),
	m_iCurrentLine( 0 ),
	m_iCurrentPriority( 0 ),
	m_iszInfoTargetScriptName( NULL_STRING ),
	m_iszLastLinePlayed( NULL_STRING ),
	m_iszDeathSoundScriptName( NULL_STRING ),
	m_iszLookAnimationName( NULL_STRING ),
	m_iszVoiceName( NULL_STRING )
{
}


CPropPersonalitySphere::~CPropPersonalitySphere()
{
}


void CPropPersonalitySphere::Spawn( void )
{
	KeyValue( "model", PERSONALITY_SPHERE_MODEL );

	BaseClass::Spawn();

	ResetSequence( LookupSequence( "drop" ) );
	SetCycle( 1.0f );

	DisableAutoFade();

	m_iEyeballAttachment = LookupAttachment( "eyeball" );
	SetContextThink( &CPropPersonalitySphere::AnimateThink, gpGlobals->curtime + 0.1f, s_pAnimateThinkContext );

	// The aquarium core talks without being picked up
	if ( m_iCoreType == CORETYPE_AQUARIUM )
	{
		m_flLastHeldTime = gpGlobals->curtime;
		StartTalking( 0.0f );
	}
}


void CPropPersonalitySphere::RegisterSoundEvent( SphereEvent_e eventName, int count )
{
	m_bHasEvent[eventName] = true;

	// Info target lines come from the targets themselves
	if ( eventName == SPHERE_EVENT_INFO_TARGET )
		return;

	if ( m_iCoreType == CORETYPE_RICHARD )
	{
		// Numbered lines, played in a shuffled order
		for ( int i = 0; i < count; ++i )
		{
			PrecacheScriptSound( CFmtStr( "%s.%s%02d", STRING( m_iszVoiceName ), g_LineNames[eventName], i + 1 ) );

			m_iLineOrder[eventName].AddToTail( i + 1 );
			int iSwapIdx = random->RandomInt( 0, i );
			m_iLineOrder[eventName][i] = m_iLineOrder[eventName][iSwapIdx];
			m_iLineOrder[eventName][iSwapIdx] = i + 1;
		}
	}
	else
	{
		PrecacheScriptSound( CFmtStr( "%s.%s", STRING( m_iszVoiceName ), g_LineNames[eventName] ) );
	}
}


void CPropPersonalitySphere::Precache( void )
{
	BaseClass::Precache();

	SetupVOList();

	for ( unsigned int i = 0; i < SPHERE_EVENT_TOTAL; ++i )
	{
		m_bHasEvent[i] = false;
		m_iLineOrder.AddToTail();
	}

	if ( m_iCoreType == CORETYPE_RICHARD )
	{
		RegisterSoundEvent( SPHERE_EVENT_INFO_TARGET, 1 );
		RegisterSoundEvent( SPHERE_EVENT_WAITING, 7 );
		RegisterSoundEvent( SPHERE_EVENT_HELD, 36 );
		RegisterSoundEvent( SPHERE_EVENT_HELD_PANIC, 30 );
		RegisterSoundEvent( SPHERE_EVENT_DROP, 3 );
		RegisterSoundEvent( SPHERE_EVENT_NOTHELD, 10 );
		RegisterSoundEvent( SPHERE_EVENT_NOTHELD_RELIEVED, 7 );
		RegisterSoundEvent( SPHERE_EVENT_FIRST_PICKUP, 1 );
		RegisterSoundEvent( SPHERE_EVENT_FRUSTRATION, 13 );
		RegisterSoundEvent( SPHERE_EVENT_HURT, 8 );
		RegisterSoundEvent( SPHERE_EVENT_HURT_REPEATEDLY, 8 );
		RegisterSoundEvent( SPHERE_EVENT_VELOCITY_UP_HIGH, 1 );
		RegisterSoundEvent( SPHERE_EVENT_VELOCITY_DOWN_HIGH, 1 );
		RegisterSoundEvent( SPHERE_EVENT_VELOCITY_LATERAL_LOW, 1 );
		RegisterSoundEvent( SPHERE_EVENT_VELOCITY_LATERAL_HIGH, 1 );
	}
	else if ( m_iCoreType == CORETYPE_AQUARIUM )
	{
		RegisterSoundEvent( SPHERE_EVENT_NOTHELD, 1 );
		RegisterSoundEvent( SPHERE_EVENT_HELD, 1 );
		RegisterSoundEvent( SPHERE_EVENT_HURT, 1 );
	}

	PrecacheModel( PERSONALITY_SPHERE_MODEL );
}


void CPropPersonalitySphere::InputStartTalking( inputdata_t &inputdata )
{
	StartTalking( 0.0f );
}


void CPropPersonalitySphere::StartTalking( float flDelay )
{
	SetThink( &CPropPersonalitySphere::TalkingThink );
	SetNextThink( gpGlobals->curtime + m_flBetweenVOPadding + flDelay );
}


void CPropPersonalitySphere::TalkingThink( void )
{
	SetNextThink( gpGlobals->curtime + 0.05f );

	if ( !m_bFirstPickup || m_iCoreType == CORETYPE_AQUARIUM )
	{
		int iCurSequence = GetSequence();
		int iLookSequence = LookupSequence( STRING( m_iszLookAnimationName ) );
		if ( iCurSequence != iLookSequence )
		{
			ResetSequence( iLookSequence );
		}

		// Abuse wears off over time
		float flInterval = gpGlobals->curtime - GetLastThink();
		m_flAbuseLevel *= ( 1.0 - 0.5 * flInterval );

		CheckForInfoTargets();
		UpdatePositions();

		if ( m_iPendingPriority >= 0 )
		{
			TrySpeakLine( m_iPendingLine, m_iPendingPriority );
		}

		// Idle chatter while held
		if ( m_flLineCompletionTime + m_flIdleWaitTime < gpGlobals->curtime && m_bHeld )
		{
			if ( m_bPanicked )
			{
				TrySpeakLine( SPHERE_EVENT_HELD_PANIC, 0 );
			}
			else
			{
				TrySpeakLine( SPHERE_EVENT_HELD, 0 );
			}
		}
	}
	else
	{
		// Waiting to be picked up for the first time
		TrySpeakLine( SPHERE_EVENT_WAITING, 0 );
	}
}


void CPropPersonalitySphere::CheckForInfoTargets( void )
{
	// Single player only
	CPortal_Player *pPlayer = (CPortal_Player *)AI_GetSinglePlayer();
	if ( pPlayer == NULL || !m_bHeld )
		return;

	Vector vecStart = pPlayer->EyePosition();
	Vector vecForward, vecRight, vecUp;
	pPlayer->EyeVectors( &vecForward, &vecRight, &vecUp );

	Ray_t ray;
	ray.Init( vecStart, vecStart + 400.0f * vecForward );

	CInfoTargetEntityEnumerator InfoTargetEnum( ray );

	trace_t tr;
	UTIL_TraceRay( ray, MASK_SOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &tr );

	// Look for targets around where the view hits, wider the further away it is
	CBaseEntity *ppEnts[256];
	int nEntCount = UTIL_EntitiesInSphere( ppEnts, 256, tr.endpos, 200.0f * tr.fraction + 100.0f, 0 );
	for ( int i = 0; i < nEntCount; ++i )
	{
		if ( ppEnts[i] == NULL )
			continue;

		InfoTargetEnum.EnumEntity( ppEnts[i] );
	}

	CInfoTargetPersonalitySphere *pEnt = static_cast<CInfoTargetPersonalitySphere*>( InfoTargetEnum.GetBestInfoTargetEntity() );
	if ( pEnt )
	{
		m_iszInfoTargetScriptName = pEnt->GetSphereLine();
		TrySpeakLine( SPHERE_EVENT_INFO_TARGET, 8 );

		// Only comment on each target once
		UTIL_Remove( pEnt );
	}
}


void CPropPersonalitySphere::UpdatePositions( void )
{
	// Single player only
	CPortal_Player *pPlayer = (CPortal_Player *)AI_GetSinglePlayer();
	if ( pPlayer == NULL )
		return;

	m_pastPositions.AddToTail( GetAbsOrigin() );
	if ( m_pastPositions.Count() > 40 )
	{
		m_pastPositions.RemoveMultipleFromHead( 1 );
	}

	// Left alone for a while
	if ( !m_bHeld && m_flLastHeldTime + 4.0f < gpGlobals->curtime )
	{
		if ( m_bPanicked )
		{
			TrySpeakLine( SPHERE_EVENT_NOTHELD_RELIEVED, 1 );
		}
		else
		{
			TrySpeakLine( SPHERE_EVENT_NOTHELD, 1 );
		}
	}

	// Average movement per think over the recorded positions
	Vector vecVelocity = vec3_origin;
	for ( int i = 1; i < m_pastPositions.Count(); ++i )
	{
		vecVelocity += m_pastPositions[i] - m_pastPositions[i-1];
	}

	if ( m_pastPositions.Count() > 1 )
	{
		vecVelocity /= ( m_pastPositions.Count() - 1 );
	}

	if ( !m_bHeld )
	{
		if ( vecVelocity.Length() > 400.0f * ( gpGlobals->curtime - GetLastThink() ) )
		{
			TrySpeakLine( SPHERE_EVENT_VELOCITY_DOWN_HIGH, 5 );
		}
	}
	else
	{
		if ( pPlayer->GetAbsVelocity().z < -400.0f )
		{
			TrySpeakLine( SPHERE_EVENT_VELOCITY_DOWN_HIGH, 3 );
		}
		else if ( pPlayer->GetAbsVelocity().z > 400.0f )
		{
			TrySpeakLine( SPHERE_EVENT_VELOCITY_UP_HIGH, 3 );
		}
		else if ( pPlayer->GetAbsVelocity().Length() > 400.0f )
		{
			TrySpeakLine( SPHERE_EVENT_VELOCITY_LATERAL_HIGH, 3 );
		}
	}
}


void CPropPersonalitySphere::TrySpeakLine( int line, int priority )
{
	// This core has nothing to say about that
	if ( !m_bHasEvent[line] )
		return;

	// Queue lines that arrive just before the current one finishes
	float flFinishTime = m_flFirstNoSkipTime - gpGlobals->curtime;
	if ( flFinishTime > 0.0f )
	{
		if ( flFinishTime < 0.5f && line != SPHERE_EVENT_HURT && priority >= m_iPendingPriority )
		{
			m_iPendingPriority = priority;
			m_iPendingLine = line;
		}
	}

	// Higher priority lines and info targets interrupt
	if ( priority > m_iCurrentPriority || line == SPHERE_EVENT_INFO_TARGET )
	{
		m_flLineCompletionTime = gpGlobals->curtime;
	}

	if ( m_flLineCompletionTime > gpGlobals->curtime )
		return;

	// Too upset for small talk
	if ( m_flAbuseLevel > 1.0f && priority < 4 )
		return;

	float flAdditionalPadding = 1.5f;
	if ( line == SPHERE_EVENT_HURT )
	{
		flAdditionalPadding = 0.0f;
		if ( m_bHasEvent[SPHERE_EVENT_HURT_REPEATEDLY] )
		{
			m_flAbuseLevel += 1.0f;
			if ( m_flAbuseLevel > 2.0f )
			{
				line = SPHERE_EVENT_HURT_REPEATEDLY;
				flAdditionalPadding = 1.5f;
			}
		}
	}

	if ( line == SPHERE_EVENT_HELD || m_iCoreType == CORETYPE_AQUARIUM )
	{
		flAdditionalPadding = 0.5f;
	}

	StopSound( STRING( m_iszLastLinePlayed ) );

	if ( m_iCoreType == CORETYPE_RICHARD && line != SPHERE_EVENT_INFO_TARGET )
	{
		// Rotate through the shuffled line order
		int iIndex = m_iLineOrder[line].Head();
		m_iLineOrder[line].RemoveMultipleFromHead( 1 );
		m_iLineOrder[line].AddToTail( iIndex );

		m_iszLastLinePlayed = AllocPooledString( CFmtStr( "%s.%s%02d", STRING( m_iszVoiceName ), g_LineNames[line], iIndex ) );
	}
	else if ( line == SPHERE_EVENT_INFO_TARGET )
	{
		m_iszLastLinePlayed = AllocPooledString( STRING( m_iszInfoTargetScriptName ) );
	}
	else
	{
		m_iszLastLinePlayed = AllocPooledString( CFmtStr( "%s.%s", STRING( m_iszVoiceName ), g_LineNames[line] ) );
	}

	EmitSound( STRING( m_iszLastLinePlayed ) );

	m_iCurrentLine = line;
	m_iCurrentPriority = priority;
	m_iPendingPriority = -1;
	m_iPendingLine = -1;

	float flCurDuration = GetSoundDuration( STRING( m_iszLastLinePlayed ), PERSONALITY_SPHERE_MODEL );
	m_flFirstNoSkipTime = gpGlobals->curtime + flCurDuration;
	m_flLineCompletionTime = gpGlobals->curtime + m_flBetweenVOPadding + flCurDuration + flAdditionalPadding;

	// Pain lines can be followed immediately
	if ( line == SPHERE_EVENT_HURT || line == SPHERE_EVENT_HURT_REPEATEDLY )
	{
		m_flLineCompletionTime = m_flFirstNoSkipTime;
	}
}


void CPropPersonalitySphere::AnimateThink( void )
{
	StudioFrameAdvance();
	SetContextThink( &CPropPersonalitySphere::AnimateThink, gpGlobals->curtime + 0.1f, s_pAnimateThinkContext );
}


void CPropPersonalitySphere::SetupVOList( void )
{
	switch ( m_iCoreType )
	{
	case CORETYPE_CURIOUS:
		m_iszLookAnimationName = AllocPooledString( "personality_sphere_testanim" );
		m_nSkin = 1;
		break;

	case CORETYPE_AGGRESSIVE:
		m_iszLookAnimationName = AllocPooledString( "personality_sphere_testanim" );
		m_nSkin = 2;
		break;

	case CORETYPE_CRAZY:
		m_iszLookAnimationName = AllocPooledString( "personality_sphere_testanim" );
		m_nSkin = 3;
		break;

	case CORETYPE_RICHARD:
		m_iszLookAnimationName = AllocPooledString( "personality_sphere_testanim" );
		m_iszVoiceName = AllocPooledString( "sphere01" );
		m_nSkin = 1;
		break;

	case CORETYPE_AQUARIUM:
		m_iszLookAnimationName = AllocPooledString( "personality_sphere_testanim" );
		m_iszVoiceName = AllocPooledString( "sphere02" );
		m_nSkin = 2;
		break;

	default:
		m_iszLookAnimationName = AllocPooledString( "personality_sphere_testanim" );
		m_nSkin = 0;
		break;
	}

	m_iszDeathSoundScriptName = AllocPooledString( "Portal.Glados_core.Death" );
}


void CPropPersonalitySphere::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	if ( m_bFirstPickup )
	{
		StartTalking( 0.0f );
		TrySpeakLine( SPHERE_EVENT_FIRST_PICKUP, 10 );
	}
	else
	{
		TrySpeakLine( SPHERE_EVENT_FRUSTRATION, 3 );
	}

	m_bFirstPickup = false;
	m_bHeld = true;

	ResetSequence( LookupSequence( "turn" ) );

	EnableMotion();
	BaseClass::OnPhysGunPickup( pPhysGunUser, reason );
}


void CPropPersonalitySphere::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason )
{
	TrySpeakLine( SPHERE_EVENT_DROP, 4 );

	m_bHeld = false;
	m_flLastHeldTime = gpGlobals->curtime;

	BaseClass::OnPhysGunDrop( pPhysGunUser, reason );
}


void CPropPersonalitySphere::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	BaseClass::VPhysicsCollision( index, pEvent );

	CBaseEntity *pHitEntity = pEvent->pEntities[!index];
	if ( pHitEntity == GetWorldEntity() )
	{
		// Only hard knocks against the world hurt when loose
		if ( !m_bHeld )
		{
			Vector vecVelocity = pEvent->preVelocity[index];
			if ( vecVelocity.Length() <= 100.0f )
				return;
		}
	}
	else if ( pHitEntity->IsPlayer() )
	{
		return;
	}

	TrySpeakLine( SPHERE_EVENT_HURT, 6 );
}


void CPropPersonalitySphere::NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params )
{
	// Teleports shouldn't register as velocity
	if ( eventType == NOTIFY_EVENT_TELEPORT )
	{
		m_pastPositions.RemoveAll();
	}
}
