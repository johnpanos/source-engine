//========= Portal 2 reconstruction ============================================//
//
// Purpose: Hovering laser sentry (npc_hover_turret) and its optional spring tether.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_basenpc_physicsflyer.h"
#include "ai_default.h"
#include "ai_senses.h"
#include "ai_memory.h"
#include "ai_utils.h"
#include "player_pickup.h"
#include "props.h"
#include "rope.h"
#include "rope_shared.h"
#include "Sprite.h"
#include "smoke_trail.h"
#include "explode.h"
#include "te_effect_dispatch.h"
#include "IEffects.h"
#include "movevars_shared.h"
#include "vphysics/constraints.h"
#include "physics_saverestore.h"
#include "portal_base2d.h"
#include "portal_util_shared.h"
#include "world.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define HOVER_TURRET_MODEL				"models/npcs/hover_turret.mdl"
#define HOVER_TURRET_GLOW_SPRITE		"sprites/light_glow03.vmt"
#define HOVER_TURRET_LASER_RANGE		8192.0f
#define HOVER_TURRET_TETHER_SLACK		20.0f

ConVar sk_hover_turret_health( "sk_hover_turret_health", "150" );

//-----------------------------------------------------------------------------
// Firing states
//-----------------------------------------------------------------------------
enum hoverTurretAttackState_e
{
	HOVER_TURRET_FIND_TARGET = 0,
	HOVER_TURRET_AIM_TARGET,
	HOVER_TURRET_SHOT_DISABLED,
	HOVER_TURRET_HIT_WITH_PHYSICS,
};

//-----------------------------------------------------------------------------
// Activities
//-----------------------------------------------------------------------------
int ACT_HOVER_TURRET_SEARCH;
int ACT_HOVER_TURRET_ALERT;
int ACT_HOVER_TURRET_ANGRY;
int ACT_HOVER_TURRET_DISABLED;

//-----------------------------------------------------------------------------
// Talk states (m_iDesiredState / m_iLastState index this table)
//-----------------------------------------------------------------------------
const char *g_HoverTurretTalkNames[] =
{
	"NPC_HoverTurret.TalkActive",		// 0
	"NPC_HoverTurret.TalkSearch",		// 1
	"NPC_FloorTurret.TalkTipped",		// 2
	"NPC_FloorTurret.TalkDisabled",		// 3
	"NPC_FloorTurret.TalkCollide",		// 4
	"NPC_HoverTurret.TalkPickup",		// 5
	"NPC_FloorTurret.TalkShotAt",		// 6
	"NPC_HoverTurret.TalkDissolved",	// 7
};

const char *GetHoverTurretTalkName( int iState )
{
	return g_HoverTurretTalkNames[ iState ];
}

//=============================================================================
// Tether: holds a hover turret on a spring to an attach point.
//=============================================================================
class CHoverTurretTether : public CBaseAnimating
{
	DECLARE_CLASS( CHoverTurretTether, CBaseAnimating );
public:
	CHoverTurretTether();
	virtual void UpdateOnRemove();

	void AttachEntities( EHANDLE hTurret, EHANDLE hAttachPoint );

	void ToggleThroughPortal( CPortal_Base2D *pEnteredPortal );

	void ReleaseTether();

	DECLARE_DATADESC();

private:
	void PullThink();
	virtual void Spawn();

	float					m_flNaturalDistance;
	EHANDLE					m_hAttachPoint;
	EHANDLE					m_hTurret;

	IPhysicsSpring			*m_pSpring;

	CHandle<CRopeKeyframe>	m_hRope;
};

LINK_ENTITY_TO_CLASS( ent_hover_turret_tether, CHoverTurretTether );

BEGIN_DATADESC( CHoverTurretTether )
	DEFINE_PHYSPTR( m_pSpring ),
	DEFINE_FIELD( m_hRope, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hAttachPoint, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hTurret, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flNaturalDistance, FIELD_FLOAT ),
	DEFINE_THINKFUNC( PullThink ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CHoverTurretTether::CHoverTurretTether()
{
	m_hTurret = NULL;
	m_hAttachPoint = NULL;
	m_pSpring = NULL;
	m_hRope = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Let go of the turret: destroy the spring and drop the rope end.
//-----------------------------------------------------------------------------
void CHoverTurretTether::ReleaseTether()
{
	if ( m_pSpring )
	{
		physenv->DestroySpring( m_pSpring );
		m_pSpring = NULL;
	}

	if ( m_hRope != NULL )
	{
		m_hRope->DetachPoint( 1 );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHoverTurretTether::UpdateOnRemove()
{
	if ( m_pSpring )
	{
		physenv->DestroySpring( m_pSpring );
		m_pSpring = NULL;
	}

	BaseClass::UpdateOnRemove();

	if ( m_hRope != NULL )
	{
		m_hRope->DetachPoint( 1 );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHoverTurretTether::AttachEntities( EHANDLE hTurret, EHANDLE hAttachPoint )
{
	m_hTurret = hTurret;
	m_hAttachPoint = hAttachPoint;
}

//-----------------------------------------------------------------------------
// Purpose: A tethered turret that passes through a portal snaps its tether and dies.
//-----------------------------------------------------------------------------
void CHoverTurretTether::ToggleThroughPortal( CPortal_Base2D *pEnteredPortal )
{
	if ( m_hTurret != NULL )
	{
		CAI_BaseNPC *pTurret = static_cast<CAI_BaseNPC *>( m_hTurret.Get() );
		pTurret->Event_Killed( CTakeDamageInfo( this, this, 200, DMG_BLAST ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHoverTurretTether::Spawn()
{
	BaseClass::Precache();
	SetModel( HOVER_TURRET_MODEL );
	AddEffects( EF_NODRAW );
	SetSolid( SOLID_VPHYSICS );
	AddSolidFlags( FSOLID_NOT_SOLID );
	VPhysicsInitShadow( false, false );
	SetMoveType( MOVETYPE_NONE );

	BaseClass::Spawn();

	if ( m_hAttachPoint != NULL )
	{
		SetNextThink( gpGlobals->curtime );
		SetThink( &CHoverTurretTether::PullThink );
		PullThink();
	}
}

ConVar hover_turret_break_dist( "hover_turret_break_dist", "10" );

//-----------------------------------------------------------------------------
// Purpose: Build the spring and rope, then break when overstretched or blocked.
//-----------------------------------------------------------------------------
void CHoverTurretTether::PullThink()
{
	CAI_BaseNPC *pTurret = static_cast<CAI_BaseNPC *>( m_hTurret.Get() );
	if ( !pTurret )
		return;

	if ( !m_pSpring )
	{
		Vector vecDistance = pTurret->GetAbsOrigin() - GetAbsOrigin();
		m_flNaturalDistance = vecDistance.Length() - HOVER_TURRET_TETHER_SLACK;

		IPhysicsObject *pPhysObject = pTurret->VPhysicsGetObject();
		IPhysicsObject *pAttachPhysObject = VPhysicsGetObject();

		springparams_t spring;
		spring.constant = 10000.0f;
		spring.damping = 16.0f;
		spring.naturalLength = m_flNaturalDistance;
		spring.relativeDamping = 0.0f;

		Vector up;
		GetVectors( NULL, NULL, &up );

		spring.startPosition = pTurret->GetAbsOrigin() + up * HOVER_TURRET_TETHER_SLACK;
		spring.endPosition = GetAbsOrigin();
		spring.useLocalPositions = false;
		spring.onlyStretch = true;

		m_pSpring = physenv->CreateSpring( pPhysObject, pAttachPhysObject, &spring );

		if ( !m_hRope )
		{
			// Portal 2 port: this CRopeKeyframe::Create() always creates a "keyframe_rope".
			m_hRope = CRopeKeyframe::Create( this, pTurret, 0, 0, 1, "cable/cable.vmt", 5 );
			m_hRope->EnableWind( true );
		}
	}

	Vector vecStart, vecEnd;
	m_pSpring->GetEndpoints( &vecStart, &vecEnd );
	vecStart -= vecEnd;
	if ( vecStart.Length() - m_flNaturalDistance > hover_turret_break_dist.GetFloat() )
	{
		// Stretched too far: snap
		pTurret->Event_Killed( CTakeDamageInfo( this, this, 200, DMG_BLAST ) );
		return;
	}

	Vector vecTraceStart = GetAbsOrigin();
	Ray_t ray;
	ray.Init( vecTraceStart, pTurret->GetAbsOrigin() );
	trace_t tr;
	UTIL_TraceRay( ray, MASK_SOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &tr );
	if ( tr.DidHitWorld() )
	{
		// The world got between the tether point and the turret: snap
		pTurret->Event_Killed( CTakeDamageInfo( this, this, 200, DMG_BLAST ) );
		return;
	}

	SetNextThink( gpGlobals->curtime );
}

//=============================================================================
// Hover turret
//=============================================================================
class CNPC_HoverTurret : public CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>, public CDefaultPlayerPickupVPhysics
{
	DECLARE_CLASS( CNPC_HoverTurret, CNPCBaseInteractive<CAI_BasePhysicsFlyingBot> );
	DECLARE_SERVERCLASS();

public:
	CNPC_HoverTurret();
	virtual ~CNPC_HoverTurret();

	virtual Class_T	Classify( void ) { return CLASS_COMBINE; }

	// Damage
	virtual void	Event_Killed( const CTakeDamageInfo &info );
	virtual void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	virtual void	NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params );

	void			ShootLaser( const Vector &vecSrc, const Vector &vecDirToEnemy );

	void			TalkThink( void );
	void			FindTargetThink( void );
	void			AimThink( void );

	void			SetFiringState( hoverTurretAttackState_e state );

	virtual void	UpdateOnRemove( void );

	virtual bool	OverrideMove( float flInterval );
	virtual void	MoveToTarget( float flInterval, const Vector &vMoveTarget );
	void			MoveExecute_Alive( float flInterval );
	void			MoveExecute_Dead( float flInterval );

	void			CreateSmokeTrail( void );
	void			DestroySmokeTrail( void );

	virtual Vector	BodyTarget( const Vector &posSrc, bool bNoisy = true ) { return WorldSpaceCenter(); }

	virtual float	GetHeadTurnRate( void ) { return 45.0f; }
	virtual void	TurnHeadToTarget( float flInterval, const Vector &MoveTarget );
	bool			UpdateFacing( void );
	void			UpdateMuzzleMatrix( void );
	Vector			GetClosestVisibleEnemyPosition( void );

	virtual void	Precache( void );
	virtual void	RunTask( const Task_t *pTask );
	virtual void	Spawn( void );
	virtual void	StartTask( const Task_t *pTask );

	virtual void	GatherConditions( void );
	virtual void	PrescheduleThink( void );
	void			Explode( void );
	void			StartDeathSequence( void );

	// Physics
	virtual void	VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	virtual void	ClampMotorForces( Vector &linear, AngularImpulse &angular );

	// Player pickup
	virtual void	OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
	virtual void	OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason );
	virtual bool	HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer );
	virtual QAngle	PreferredCarryAngles( void );

	void			ToggleUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	virtual int		ObjectCaps( void )
	{
		return BaseClass::ObjectCaps() | FCAP_IMPULSE_USE;
	}

	virtual void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
	{
		CBasePlayer *pPlayer = ToBasePlayer( pActivator );
		if ( pPlayer )
		{
			pPlayer->PickupObject( this, false );
		}
	}

	virtual CBasePlayer *HasPhysicsAttacker( float dt );

	float			GetMaxEnginePower( void );
	bool			IsMovementDisabled( void );

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

private:
	void			MaintainGroundHeight( void );
	void			TakeDamageFromPhysicsImpact( int index, gamevcollisionevent_t *pEvent );

	virtual bool	AllowedToIgnite( void ) { return true; }
	bool			OnBurning( void );

	// Talking
	float			m_fNextTalk;
	int				m_iDesiredState;
	int				m_iLastState;

	// Aiming
	bool			m_bAimingAtTarget;
	Vector			m_vecTargetPos;
	float			m_flAimStartTime;

	// Laser
	CNetworkVar( int, m_iLaserState );
	CNetworkVar( int, m_sLaserHaloSprite );

	// Carrying
	bool			m_bCarriedByPlayer;
	bool			m_bUseCarryAngles;

	// Movement
	Vector			m_vForceVelocity;
	Vector			m_vForceMoveTarget;

	Vector			m_vTargetBanking;

	EHANDLE			m_hAttachTarget;
	string_t		m_strAttachName;

	EHANDLE			m_hTether;
	float			m_flSentryTurnSpeed;

	bool			m_bCanPushPlayer;

	float			m_flEngineStallTime;

	bool			m_bInitialPositionSet;

	COutputEvent	m_OnPhysGunPickup;
	COutputEvent	m_OnPhysGunDrop;

	float			m_fSparkTime;
	float			m_flDeathTime;
	CHandle<SmokeTrail>	m_hSmokeTrail;

	hoverTurretAttackState_e	m_iFiringState;
	CHandle<CSprite>	m_hEyeGlow;

	int				m_iMuzzleAttachment;
	matrix3x4_t		m_muzzleToWorld;
	int				m_muzzleToWorldTick;

	QAngle			m_vecGoalAngles;
	QAngle			m_vInitialLookAngles;
};

LINK_ENTITY_TO_CLASS( npc_hover_turret, CNPC_HoverTurret );

BEGIN_DATADESC( CNPC_HoverTurret )
	DEFINE_FIELD( m_bAimingAtTarget, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecTargetPos, FIELD_VECTOR ),
	DEFINE_FIELD( m_flAimStartTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_iFiringState, FIELD_INTEGER ),
	DEFINE_FIELD( m_vForceVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( m_bInitialPositionSet, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecGoalAngles, FIELD_VECTOR ),
	DEFINE_FIELD( m_vInitialLookAngles, FIELD_VECTOR ),
	DEFINE_FIELD( m_vForceMoveTarget, FIELD_VECTOR ),
	DEFINE_FIELD( m_vTargetBanking, FIELD_VECTOR ),
	DEFINE_FIELD( m_flEngineStallTime, FIELD_TIME ),
	DEFINE_FIELD( m_fNextTalk, FIELD_FLOAT ),
	DEFINE_FIELD( m_iDesiredState, FIELD_INTEGER ),
	DEFINE_FIELD( m_iLastState, FIELD_INTEGER ),
	DEFINE_FIELD( m_bCarriedByPlayer, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bUseCarryAngles, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iMuzzleAttachment, FIELD_INTEGER ),
	DEFINE_FIELD( m_muzzleToWorldTick, FIELD_INTEGER ),
	DEFINE_FIELD( m_hEyeGlow, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flDeathTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_fSparkTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_hSmokeTrail, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hTether, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hAttachTarget, FIELD_EHANDLE ),

	DEFINE_KEYFIELD( m_strAttachName, FIELD_STRING, "attachTarget" ),
	DEFINE_KEYFIELD( m_flSentryTurnSpeed, FIELD_FLOAT, "sentryRotateSpeed" ),
	DEFINE_KEYFIELD( m_bCanPushPlayer, FIELD_BOOLEAN, "canPushPlayer" ),

	DEFINE_OUTPUT( m_OnPhysGunPickup, "OnPhysGunPickup" ),
	DEFINE_OUTPUT( m_OnPhysGunDrop, "OnPhysGunDrop" ),

	DEFINE_OUTPUT( m_OnAlyxStartedInteraction, "OnAlyxStartedInteraction" ),
	DEFINE_OUTPUT( m_OnAlyxFinishedInteraction, "OnAlyxFinishedInteraction" ),
	DEFINE_INPUTFUNC( FIELD_VOID, "InteractivePowerDown", InputPowerdown ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPC_HoverTurret, DT_NPC_HoverTurret )
	SendPropInt( SENDINFO( m_iLaserState ) ),
	SendPropInt( SENDINFO( m_sLaserHaloSprite ) ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CNPC_HoverTurret::CNPC_HoverTurret()
{
	m_bUseCarryAngles = true;
	m_iLaserState = 1;
	m_sLaserHaloSprite = 0;
	m_hTether = NULL;
	m_hAttachTarget = NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CNPC_HoverTurret::~CNPC_HoverTurret()
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::GatherConditions( void )
{
	BaseClass::GatherConditions();
}

//-----------------------------------------------------------------------------
// Purpose: Per-think driver: burn down, run the current firing state or
//			count down to self destruction.
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::PrescheduleThink( void )
{
	if ( IsOnFire() )
	{
		OnBurning();
	}
	else if ( m_iHealth > 0 )
	{
		switch ( m_iFiringState )
		{
		case HOVER_TURRET_AIM_TARGET:
			AimThink();
			break;

		case HOVER_TURRET_SHOT_DISABLED:
			return;

		case HOVER_TURRET_FIND_TARGET:
			FindTargetThink();
			break;

		default:
			break;
		}
	}
	else if ( m_flDeathTime != 0.0f && gpGlobals->curtime > m_flDeathTime )
	{
		Explode();
	}

	TalkThink();

	BaseClass::PrescheduleThink();
}

//-----------------------------------------------------------------------------
// Purpose: Change firing state, updating skin, activity, laser and eye glow.
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::SetFiringState( hoverTurretAttackState_e state )
{
	if ( !IsAlive() && state != HOVER_TURRET_SHOT_DISABLED )
		return;

	if ( !m_hEyeGlow )
	{
		m_hEyeGlow = CSprite::SpriteCreate( HOVER_TURRET_GLOW_SPRITE, GetLocalOrigin(), false );
		if ( !m_hEyeGlow )
			return;

		m_hEyeGlow->SetTransparency( kRenderWorldGlow, 255, 0, 0, 128, kRenderFxNoDissipation );
		m_hEyeGlow->SetAttachment( this, 1 );
	}

	bool bNewState = ( m_iFiringState != state );

	if ( state == HOVER_TURRET_SHOT_DISABLED )
	{
		m_iFiringState = HOVER_TURRET_SHOT_DISABLED;
		m_iDesiredState = 5;

		if ( bNewState )
		{
			SetActivity( (Activity)ACT_HOVER_TURRET_ANGRY );
			m_nSkin = 2;
			m_iLaserState = 0;
		}

		m_hEyeGlow->SetColor( 255, 0, 255 );
		m_hEyeGlow->SetScale( 0.1f, 3.0f );
		m_hEyeGlow->SetBrightness( 0, 3.0f );
		return;
	}

	int nBrightness;
	if ( state == HOVER_TURRET_HIT_WITH_PHYSICS )
	{
		// Knocked around: angry, laser off, but keep looking for targets
		m_iFiringState = HOVER_TURRET_FIND_TARGET;
		m_iDesiredState = 7;
		SetActivity( (Activity)ACT_HOVER_TURRET_ANGRY );
		m_nSkin = 2;
		m_iLaserState = 0;

		m_hEyeGlow->SetColor( 255, 0, 0 );
		nBrightness = 128;
	}
	else if ( state == HOVER_TURRET_AIM_TARGET )
	{
		m_iDesiredState = 0;
		m_bAimingAtTarget = true;
		m_vecTargetPos = GetClosestVisibleEnemyPosition();
		m_iFiringState = HOVER_TURRET_AIM_TARGET;
		m_flAimStartTime = gpGlobals->curtime;

		if ( bNewState )
		{
			SetActivity( (Activity)ACT_HOVER_TURRET_ALERT );
			m_nSkin = 1;
		}

		m_hEyeGlow->SetColor( 255, 255, 0 );
		nBrightness = 196;
	}
	else
	{
		m_bAimingAtTarget = false;
		m_flAimStartTime = 0.0f;
		m_iDesiredState = 1;
		m_iFiringState = HOVER_TURRET_FIND_TARGET;
		m_iLaserState = 1;

		if ( bNewState )
		{
			m_nSkin = 3;
			SetActivity( (Activity)ACT_HOVER_TURRET_SEARCH );
		}

		m_hEyeGlow->SetColor( 0, 0, 255 );
		nBrightness = 96;
	}

	m_hEyeGlow->SetBrightness( nBrightness, 0.1f );
	m_hEyeGlow->SetScale( 0.4f, 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: Play the talk sound for the desired state when it changes.
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::TalkThink( void )
{
	if ( IsDissolving() || IsOnFire() )
	{
		m_iDesiredState = 7;
	}

	// Important states interrupt whatever we were saying
	if ( m_iLastState != m_iDesiredState &&
		 ( m_iDesiredState == 2 || m_iDesiredState == 7 || m_iDesiredState == 5 ) )
	{
		m_fNextTalk = gpGlobals->curtime - 1.0f;
	}

	if ( gpGlobals->curtime > m_fNextTalk && m_iLastState != m_iDesiredState )
	{
		m_iLastState = m_iDesiredState;

		const char *pchScriptName = GetHoverTurretTalkName( m_iDesiredState );

		switch ( m_iDesiredState )
		{
		case 0:
			EmitSound( pchScriptName );
			m_fNextTalk = gpGlobals->curtime + 1.75f;
			break;

		case 1:
			EmitSound( pchScriptName );
			m_fNextTalk = gpGlobals->curtime + 2.75f;
			break;

		case 2:
			EmitSound( pchScriptName );
			m_fNextTalk = gpGlobals->curtime + 1.15f;
			break;

		case 5:
			EmitSound( pchScriptName );
			m_fNextTalk = gpGlobals->curtime + 5.0f;
			break;

		case 7:
			EmitSound( pchScriptName );
			m_fNextTalk = gpGlobals->curtime + 10.0f;
			break;

		default:
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Search state: look for enemies and start aiming when one is seen.
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::FindTargetThink( void )
{
	if ( IsMovementDisabled() )
		return;

	GetEnemies()->RefreshMemories();
	GetSenses()->Look( HOVER_TURRET_LASER_RANGE );
	SetEnemy( BestEnemy() );

	if ( m_iLaserState == 0 )
	{
		m_iLaserState = 1;
	}

	if ( HasCondition( COND_SEE_ENEMY ) )
	{
		CBaseEntity *pEnemy = GetEnemy();
		if ( pEnemy && pEnemy->IsAlive() )
		{
			bool bEnemyInFOV = FInViewCone( pEnemy );
			bool bEnemyVisible = FVisible( pEnemy );

			CPortal_Base2D *pPortal = FInViewConeThroughPortal( pEnemy );
			bool bEnemyVisibleThroughPortal = ( pPortal && FVisibleThroughPortal( pPortal, pEnemy ) );

			if ( ( bEnemyInFOV && bEnemyVisible ) || bEnemyVisibleThroughPortal )
			{
				SetFiringState( HOVER_TURRET_AIM_TARGET );
			}
		}
	}
	else
	{
		m_vecGoalAngles = GetAbsAngles();
		m_flFieldOfView = 0.4f;
	}

	UpdateFacing();
}

//-----------------------------------------------------------------------------
// Purpose: Aim state: track the enemy and fire once locked on.
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::AimThink( void )
{
	bool bAbortAim = false;

	if ( !HasCondition( COND_SEE_ENEMY ) || IsMovementDisabled() )
	{
		bAbortAim = true;
	}
	else if ( GetEnemy() )
	{
		// Give up if the enemy moved too far since the last update
		Vector vecEnemyPosition = GetClosestVisibleEnemyPosition();
		Vector vecEnemyMovement = vecEnemyPosition - m_vecTargetPos;
		m_vecTargetPos = vecEnemyPosition;

		if ( vecEnemyMovement.Length() > 60.0f )
		{
			bAbortAim = true;
		}
	}

	if ( bAbortAim )
	{
		m_vecGoalAngles = GetAbsAngles();
		UpdateFacing();

		if ( !m_bAimingAtTarget )
			return;

		SetFiringState( HOVER_TURRET_FIND_TARGET );
		return;
	}

	UpdateFacing();

	if ( GetActivity() != ACT_HOVER_TURRET_ALERT )
	{
		SetActivity( (Activity)ACT_HOVER_TURRET_ALERT );
	}

	if ( gpGlobals->curtime <= m_flAimStartTime + 1.5f )
		return;

	CBaseEntity *pEnemy = GetEnemy();
	if ( !pEnemy || !pEnemy->IsAlive() )
		return;

	bool bEnemyInFOV = FInViewCone( pEnemy );
	bool bEnemyVisible = FVisible( pEnemy );

	CPortal_Base2D *pPortal = FInViewConeThroughPortal( pEnemy );
	bool bEnemyVisibleThroughPortal = ( pPortal && FVisibleThroughPortal( pPortal, pEnemy ) );

	if ( ( bEnemyInFOV && bEnemyVisible ) || bEnemyVisibleThroughPortal )
	{
		// Keep the lock-on time from growing unbounded while firing
		m_flAimStartTime -= 0.05f;

		UpdateMuzzleMatrix();

		Vector vecMuzzle, vecMuzzleDir;
		MatrixGetColumn( m_muzzleToWorld, 3, vecMuzzle );
		MatrixGetColumn( m_muzzleToWorld, 0, vecMuzzleDir );

		ShootLaser( vecMuzzle, vecMuzzleDir );

		m_iLaserState = 2;
		return;
	}

	SetFiringState( HOVER_TURRET_FIND_TARGET );
}

//-----------------------------------------------------------------------------
// Purpose: Pick the enemy point to aim at, directly or through a portal, and
//			point the goal angles at it.
//-----------------------------------------------------------------------------
Vector CNPC_HoverTurret::GetClosestVisibleEnemyPosition( void )
{
	CBaseEntity *pEnemy = GetEnemy();

	UpdateMuzzleMatrix();
	Vector vecMuzzlePos;
	MatrixGetColumn( m_muzzleToWorld, 3, vecMuzzlePos );

	Vector vecMidEnemy = pEnemy->BodyTarget( vecMuzzlePos, false ) * 0.65f + pEnemy->CollisionProp()->WorldSpaceCenter() * 0.35f;

	Vector vecMid = EyePosition();
	Vector vecBody = pEnemy->BodyTarget( vecMid, false );

	bool bEnemyInFOV = FInViewCone( pEnemy );
	bool bEnemyVisible = FVisible( pEnemy ) && pEnemy->IsAlive();

	Vector vecDirToEnemy = vecMidEnemy - vecMuzzlePos;
	float flDistToEnemy = VectorNormalize( vecDirToEnemy );

	CPortal_Base2D *pPortal = pEnemy->IsAlive() ? FInViewConeThroughPortal( pEnemy ) : NULL;
	if ( pPortal && FVisibleThroughPortal( pPortal, pEnemy ) )
	{
		// Aim at the enemy's image on our side of the portal pair
		Vector vecMidEnemyTransformed;
		UTIL_Portal_PointTransform( pPortal->m_hLinkedPortal->MatrixThisToLinked(), vecMidEnemy, vecMidEnemyTransformed );

		Vector vecDirToEnemyTransformed = vecMidEnemyTransformed - vecMuzzlePos;
		float flDistToEnemyTransformed = VectorNormalize( vecDirToEnemyTransformed );

		if ( !bEnemyInFOV || !bEnemyVisible || flDistToEnemyTransformed < flDistToEnemy )
		{
			vecMidEnemy = vecMidEnemyTransformed;
			vecDirToEnemy = vecDirToEnemyTransformed;
		}
	}

	VectorNormalize( vecDirToEnemy );

	QAngle vecAnglesToEnemy;
	VectorAngles( vecDirToEnemy, vecAnglesToEnemy );
	m_vecGoalAngles = vecAnglesToEnemy;

	return vecMidEnemy;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	BaseClass::TraceAttack( info, vecDir, ptr, pAccumulator );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::Event_Killed( const CTakeDamageInfo &info )
{
	if ( m_hSmokeTrail == NULL )
	{
		CreateSmokeTrail();
		m_fSparkTime = gpGlobals->curtime + 0.1f;
	}

	if ( m_pMotionController )
	{
		physenv->DestroyMotionController( m_pMotionController );
		m_pMotionController = NULL;
	}

	if ( m_hTether )
	{
		CHoverTurretTether *pTether = static_cast<CHoverTurretTether *>( (CBaseEntity *)m_hTether );
		pTether->ReleaseTether();
		EmitSound( "NPC_HoverTurret.Snap" );
		UTIL_Remove( m_hTether );
		m_hTether = NULL;
	}

	if ( !m_bCarriedByPlayer )
	{
		StartDeathSequence();
	}

	SetFiringState( HOVER_TURRET_SHOT_DISABLED );

	m_iHealth = 0;

	m_OnDeath.FireOutput( this, this );
}

//-----------------------------------------------------------------------------
// Purpose: Beep a warning before self destructing.
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::StartDeathSequence( void )
{
	if ( m_flDeathTime == 0.0f )
	{
		m_flDeathTime = gpGlobals->curtime + 2.5f;

		EmitSound( "NPC_RocketTurret.LockingBeep" );
		EmitSound( "NPC_RocketTurret.LockingBeep", gpGlobals->curtime + 0.5f );
		EmitSound( "NPC_RocketTurret.LockingBeep", gpGlobals->curtime + 1.0f );
		EmitSound( "NPC_RocketTurret.LockingBeep", gpGlobals->curtime + 1.5f );
		EmitSound( "NPC_FloorTurret.LockedBeep", gpGlobals->curtime + 2.0f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: A tethered turret carried through a portal snaps its tether.
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params )
{
	if ( eventType == NOTIFY_EVENT_TELEPORT && IsAlive() )
	{
		if ( m_hTether != NULL )
		{
			CHoverTurretTether *pHoverTurretTether = static_cast<CHoverTurretTether *>( m_hTether.Get() );
			CPortal_Base2D *pEnteredPortal = dynamic_cast<CPortal_Base2D *>( pNotify );
			if ( pEnteredPortal )
			{
				pHoverTurretTether->ToggleThroughPortal( pEnteredPortal );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Heavy impacts (and energy balls) destroy the turret outright.
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::TakeDamageFromPhysicsImpact( int index, gamevcollisionevent_t *pEvent )
{
	CBaseEntity *pHitEntity = pEvent->pEntities[!index];

	float flDamageScale = 20.0f * m_impactEnergyScale;

	int damageType = 0;
	float damage = CalculateDefaultPhysicsDamage( index, pEvent, flDamageScale, true, damageType );

	if ( pHitEntity && FClassnameIs( pHitEntity, "prop_energy_ball" ) )
	{
		damage = 100.0f;
	}
	else if ( damage == 0 )
	{
		return;
	}

	Vector damagePos;
	pEvent->pInternalData->GetContactPoint( damagePos );
	Vector damageForce = pEvent->postVelocity[index] * pEvent->pObjects[index]->GetMass();
	if ( damageForce == vec3_origin )
	{
		// This can happen if this entity is motion disabled, and can't move.
		// Use the velocity of the entity that hit us instead.
		damageForce = pEvent->postVelocity[!index] * pEvent->pObjects[!index]->GetMass();
	}

	if ( damage > 50.0f )
	{
		Event_Killed( CTakeDamageInfo( pHitEntity, pHitEntity, damageForce, damagePos, damage, damageType ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::VPhysicsCollision( int index, gamevcollisionevent_t *pEvent )
{
	if ( m_iHealth <= 0 )
		return;

	BaseClass::VPhysicsCollision( index, pEvent );

	int otherIndex = !index;
	CBaseEntity *pHitEntity = pEvent->pEntities[otherIndex];

	m_flFieldOfView = -1.0f;

	if ( !pHitEntity )
		return;

	if ( pHitEntity->IsPlayer() )
	{
		// Bumped by a player: stall briefly and keep looking
		if ( m_iFiringState != HOVER_TURRET_AIM_TARGET )
		{
			SetFiringState( HOVER_TURRET_FIND_TARGET );
			m_flEngineStallTime = gpGlobals->curtime + 0.5f;
		}
		return;
	}

	if ( pHitEntity != GetWorldEntity() )
	{
		m_flEngineStallTime = gpGlobals->curtime + 4.0f;
		if ( m_iFiringState != HOVER_TURRET_SHOT_DISABLED )
		{
			SetFiringState( HOVER_TURRET_HIT_WITH_PHYSICS );
		}
	}

	TakeDamageFromPhysicsImpact( index, pEvent );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::MaintainGroundHeight( void )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::TurnHeadToTarget( float flInterval, const Vector &MoveTarget )
{
	float flDestYaw = VecToYaw( MoveTarget - GetLocalOrigin() );

	float newYaw = AI_ClampYaw( GetHeadTurnRate() * 10.0f, m_fHeadYaw, flDestYaw, gpGlobals->curtime - GetLastThink() );

	if ( newYaw != m_fHeadYaw )
	{
		m_fHeadYaw = newYaw;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Drive the aim pose parameters toward m_vecGoalAngles.
// Output : Returns true if the turret moved.
//-----------------------------------------------------------------------------
bool CNPC_HoverTurret::UpdateFacing( void )
{
	bool bMoved = false;
	UpdateMuzzleMatrix();

	Vector vecGoalDir;
	AngleVectors( m_vecGoalAngles, &vecGoalDir );

	Vector vecGoalLocalDir;
	VectorIRotate( vecGoalDir, m_muzzleToWorld, vecGoalLocalDir );

	QAngle vecGoalLocalAngles;
	VectorAngles( vecGoalLocalDir, vecGoalLocalAngles );

	// Update pitch
	float flDiff = AngleNormalize( UTIL_ApproachAngle( vecGoalLocalAngles.x, 0.0, 0.25f * MaxYawSpeed() ) );
	SetPoseParameter( m_poseAim_Pitch, GetPoseParameter( m_poseAim_Pitch ) + ( flDiff / 2.5f ) );

	if ( fabs( flDiff ) > 0.1f )
	{
		bMoved = true;
	}

	// Update yaw
	flDiff = AngleNormalize( UTIL_ApproachAngle( vecGoalLocalAngles.y, 0.0, 0.05f * MaxYawSpeed() ) );
	SetPoseParameter( m_poseAim_Yaw, GetPoseParameter( m_poseAim_Yaw ) + ( flDiff / 2.5f ) );

	if ( fabs( flDiff ) > 0.1f )
	{
		bMoved = true;
	}

	InvalidateBoneCache();

	return bMoved;
}

//-----------------------------------------------------------------------------
// Purpose: Cache the muzzle attachment transform once per tick.
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::UpdateMuzzleMatrix( void )
{
	if ( gpGlobals->tickcount != m_muzzleToWorldTick )
	{
		m_muzzleToWorldTick = gpGlobals->tickcount;
		GetAttachment( m_iMuzzleAttachment, m_muzzleToWorld );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CNPC_HoverTurret::OverrideMove( float flInterval )
{
	if ( m_iHealth <= 0 )
	{
		MoveExecute_Dead( flInterval );
		return true;
	}

	if ( !m_bInitialPositionSet )
	{
		m_vForceMoveTarget = GetAbsOrigin() + Vector( 0, 0, 52 );
		m_bInitialPositionSet = true;
	}

	if ( VPhysicsGetObject() )
	{
		VPhysicsGetObject()->Wake();
	}

	if ( !IsMovementDisabled() )
	{
		if ( !m_pMotionController )
		{
			m_pMotionController = physenv->CreateMotionController( this );
			m_pMotionController->AttachObject( VPhysicsGetObject(), true );
		}

		MoveToTarget( flInterval, m_vForceMoveTarget );
		MoveExecute_Alive( flInterval );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::MoveExecute_Alive( float flInterval )
{
	PhysicsCheckWaterTransition();

	Vector vCurrentVelocity = GetCurrentVelocity();

	if ( VPhysicsGetObject() && m_hTether != NULL )
	{
		VPhysicsGetObject()->Wake();
	}

	// Add in any forced velocity, and sag a little under gravity
	vCurrentVelocity.z += flInterval * -0.1f * sv_gravity.GetFloat();
	SetCurrentVelocity( vCurrentVelocity + m_vForceVelocity );
	m_vForceVelocity = vec3_origin;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::MoveExecute_Dead( float flInterval )
{
	Vector newVelocity = GetCurrentVelocity();

	Vector forward, up;
	GetVectors( &forward, NULL, &up );

	// Periodically emit sparks.
	if ( gpGlobals->curtime > m_fSparkTime )
	{
		g_pEffects->Sparks( GetAbsOrigin() );
		m_fSparkTime = gpGlobals->curtime + random->RandomFloat( 0.05f, 0.1f );
	}

	// Add in any forced velocity
	newVelocity += m_vForceVelocity;
	SetCurrentVelocity( newVelocity );
	m_vForceVelocity = vec3_origin;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	if ( !IsAlive() )
		return;

	if ( m_pMotionController )
	{
		physenv->DestroyMotionController( m_pMotionController );
		m_pMotionController = NULL;
	}

	m_bCarriedByPlayer = true;
	SetFiringState( HOVER_TURRET_SHOT_DISABLED );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason )
{
	m_bCarriedByPlayer = false;

	if ( m_iHealth <= 0 )
	{
		StartDeathSequence();
		return;
	}

	if ( m_hTether != NULL )
	{
		// Tethered turrets recover after a moment
		SetFiringState( HOVER_TURRET_FIND_TARGET );
		m_flEngineStallTime = gpGlobals->curtime + 2.5f;
		m_flFieldOfView = -1.0f;
		return;
	}

	m_iDesiredState = 7;
	m_nSkin = 0;
	SetActivity( (Activity)ACT_HOVER_TURRET_DISABLED );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CNPC_HoverTurret::HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer )
{
	return ( IsAlive() && m_bUseCarryAngles );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
QAngle CNPC_HoverTurret::PreferredCarryAngles( void )
{
	static QAngle g_prefAngles;

	Vector vecUserForward;
	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	pPlayer->EyeVectors( &vecUserForward );

	// If we're looking up, then face directly forward
	if ( vecUserForward.z >= 0.0f )
		return vec3_angle;

	// Otherwise, stay "upright"
	g_prefAngles.Init();
	g_prefAngles.x = -pPlayer->EyeAngles().x;

	return g_prefAngles;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::Precache( void )
{
	PrecacheModel( "models/props_junk/rock001a.mdl" );
	PrecacheModel( "effects/redlaser1.vmt" );
	PrecacheModel( "models/props/futbol_Gib01.mdl" );
	PrecacheModel( "models/props/futbol_Gib02.mdl" );
	PrecacheModel( "models/props/futbol_Gib03.mdl" );
	PrecacheModel( "models/props/futbol_Gib04.mdl" );
	PrecacheModel( HOVER_TURRET_MODEL );

	ADD_CUSTOM_ACTIVITY( CNPC_HoverTurret, ACT_HOVER_TURRET_SEARCH );
	ADD_CUSTOM_ACTIVITY( CNPC_HoverTurret, ACT_HOVER_TURRET_ALERT );
	ADD_CUSTOM_ACTIVITY( CNPC_HoverTurret, ACT_HOVER_TURRET_ANGRY );
	ADD_CUSTOM_ACTIVITY( CNPC_HoverTurret, ACT_HOVER_TURRET_DISABLED );

	PrecacheScriptSound( "NPC_RocketTurret.LockingBeep" );
	PrecacheScriptSound( "NPC_FloorTurret.LockedBeep" );
	PrecacheScriptSound( "HL2Player.BurnPain" );
	PrecacheScriptSound( "NPC_HoverTurret.Snap" );

	UTIL_PrecacheDecal( "decals/scorchfade", false );

	PrecacheModel( "models/props_bts/rocket_sentry.mdl" );
	PrecacheModel( "effects/bluelaser1.vmt" );

	m_sLaserHaloSprite = PrecacheModel( HOVER_TURRET_GLOW_SPRITE );

	for ( int iTalkScript = 0; iTalkScript < ARRAYSIZE( g_HoverTurretTalkNames ); ++iTalkScript )
	{
		PrecacheScriptSound( g_HoverTurretTalkNames[ iTalkScript ] );
	}

	UTIL_PrecacheOther( "prop_glass_futbol" );

	BaseClass::Precache();
}

// Reconstruction note: both 2010 builds compare the task id against 0x97, which is
// LAST_SHARED_TASK there (TASK_PRE_SCRIPT is 0x96). No custom task name survives in
// the debug data, so the first private task slot is spelled LAST_SHARED_TASK here.
#define TASK_HOVER_TURRET_FIRST_PRIVATE_TASK	LAST_SHARED_TASK

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_HOVER_TURRET_FIRST_PRIVATE_TASK:
		break;

	default:
		BaseClass::RunTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::Spawn( void )
{
	Precache();

	SetModel( HOVER_TURRET_MODEL );

	SetHullType( HULL_TINY_CENTERED );
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_VPHYSICS );

	m_iHealth = sk_hover_turret_health.GetFloat();
	m_NPCState = NPC_STATE_NONE;

	SetPoseParameter( m_poseAim_Yaw, 0 );
	SetPoseParameter( m_poseAim_Pitch, 0 );

	SetActivity( (Activity)ACT_HOVER_TURRET_SEARCH );

	m_iDesiredState = m_iLastState = 1;
	SetFiringState( HOVER_TURRET_FIND_TARGET );

	SetNavType( NAV_FLY );

	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL );
	AddEffects( EF_NOSHADOW );

	SetBloodColor( DONT_BLEED );
	SetCurrentVelocity( vec3_origin );
	m_vForceVelocity.Init();
	m_vCurrentBanking.Init();
	m_vTargetBanking.Init();

	CapabilitiesAdd( bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_MOVE_FLY | bits_CAP_SQUAD );

	// Noise modifier
	Vector bobAmount;
	bobAmount.x = random->RandomFloat( -1.0f, 1.0f );
	bobAmount.y = random->RandomFloat( -1.0f, 1.0f );
	bobAmount.z = random->RandomFloat( -1.0f, 1.0f );
	SetNoiseMod( bobAmount );

	m_fHeadYaw = 0;

	NPCInit();

	SetViewOffset( Vector( 0, 0, 10 ) );

	m_flFieldOfView = 0.4f;

	SetCollisionGroup( COLLISION_GROUP_NONE );

	m_iMuzzleAttachment = LookupAttachment( "eyes" );

	m_vecGoalAngles = GetAbsAngles();
	m_vInitialLookAngles = GetAbsAngles();

	m_bCarriedByPlayer = false;
	m_flDeathTime = 0.0f;

	if ( GetAbsOrigin() == vec3_origin )
	{
		m_bInitialPositionSet = false;
	}
	else
	{
		m_bInitialPositionSet = true;
		m_vForceMoveTarget = GetAbsOrigin();
	}

	m_hAttachTarget = gEntList.FindEntityByName( NULL, m_strAttachName );

	if ( m_hAttachTarget != NULL && m_hTether == NULL )
	{
		m_hTether = CreateEntityByName( "ent_hover_turret_tether" );
		m_hTether->SetAbsOrigin( m_hAttachTarget->GetAbsOrigin() );

		CHoverTurretTether *pHoverTurretTether = static_cast<CHoverTurretTether *>( m_hTether.Get() );
		pHoverTurretTether->AttachEntities( EHANDLE( this ), EHANDLE( m_hAttachTarget ) );
		DispatchSpawn( m_hTether );

		CBaseEntity *pAttachEnt = m_hAttachTarget.Get();
		Vector vecToAttach = pAttachEnt->GetAbsOrigin() - GetAbsOrigin();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_HOVER_TURRET_FIRST_PRIVATE_TASK:
		break;

	default:
		BaseClass::StartTask( pTask );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::UpdateOnRemove( void )
{
	TalkThink();
	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: Hover in place; drift a little while searching.
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::ClampMotorForces( Vector &linear, AngularImpulse &angular )
{
	if ( m_iHealth <= 0 )
	{
		linear.Init();
		angular.Init();
		return;
	}

	linear.Init();

	if ( !m_bAimingAtTarget )
	{
		angular.x *= 0.01f;
		angular.y *= 0.01f;

		if ( m_flSentryTurnSpeed == 0.0f )
		{
			linear.x = random->RandomFloat( -50.0f, 50.0f );
			linear.y = random->RandomFloat( -50.0f, 50.0f );
			linear.z = random->RandomFloat( -150.0f, 150.0f );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBasePlayer *CNPC_HoverTurret::HasPhysicsAttacker( float dt )
{
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
float CNPC_HoverTurret::GetMaxEnginePower( void )
{
	return 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CNPC_HoverTurret::IsMovementDisabled( void )
{
	return ( m_flEngineStallTime > gpGlobals->curtime || m_iFiringState == HOVER_TURRET_SHOT_DISABLED );
}

//-----------------------------------------------------------------------------
// Purpose: Turn toward the enemy, spin as a sentry, or look along the move.
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::MoveToTarget( float flInterval, const Vector &vMoveTarget )
{
	if ( flInterval <= 0 )
		return;

	if ( IsMovementDisabled() )
		return;

	if ( GetEnemy() != NULL && HasCondition( COND_SEE_ENEMY ) )
	{
		TurnHeadToTarget( flInterval, GetClosestVisibleEnemyPosition() );
	}
	else if ( m_flSentryTurnSpeed != 0.0f )
	{
		float newYaw = AI_ClampYaw( m_flSentryTurnSpeed, m_fHeadYaw, m_fHeadYaw + flInterval * m_flSentryTurnSpeed, gpGlobals->curtime - GetLastThink() );
		if ( newYaw != m_fHeadYaw )
		{
			m_fHeadYaw = newYaw;
		}
	}
	else
	{
		Vector vecMoveDir = vMoveTarget - GetAbsOrigin();
		vecMoveDir.NormalizeInPlace();

		Vector vecLookDir;
		AngleVectors( m_vInitialLookAngles, &vecLookDir );

		vecLookDir = vecMoveDir + vecLookDir * 5.0f;

		TurnHeadToTarget( flInterval, GetAbsOrigin() + vecLookDir );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Burn whatever the laser touches this frame.
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::ShootLaser( const Vector &vecSrc, const Vector &vecDirToEnemy )
{
	trace_t tr;
	UTIL_TraceLine( vecSrc, vecSrc + vecDirToEnemy * HOVER_TURRET_LASER_RANGE, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	if ( tr.fraction == 1.0f )
		return;

	CBaseEntity *pHit = tr.m_pEnt;
	if ( pHit )
	{
		ClearMultiDamage();

		Vector dir = tr.endpos - GetAbsOrigin();
		VectorNormalize( dir );

		int nDamageType = DMG_ENERGYBEAM;

		CTakeDamageInfo info( this, this, ( gpGlobals->curtime - GetLastThink() ) * 150.0f, nDamageType );
		CalculateMeleeDamageForce( &info, dir, tr.endpos, m_bCanPushPlayer ? 0.05f : 0.0f );
		pHit->DispatchTraceAttack( info, dir, &tr );
		ApplyMultiDamage();

		UTIL_DecalTrace( &tr, "RedGlowFade" );

		if ( pHit->IsPlayer() )
		{
			pHit->EmitSound( "HL2Player.BurnPain" );
		}
	}

	g_pEffects->Sparks( tr.endpos );
}

//-----------------------------------------------------------------------------
// Purpose: Lose health while burning; explode when it runs out.
// Output : Returns true while the turret survives.
//-----------------------------------------------------------------------------
bool CNPC_HoverTurret::OnBurning( void )
{
	m_iHealth -= 1;

	if ( m_iHealth > 0 )
		return true;

	Explode();
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Shatter into a glass futbol, damage nearby turrets/cubes/players
//			and remove ourselves.
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::Explode( void )
{
	CBreakableProp *pFutbol = static_cast<CBreakableProp *>( CBaseEntity::Create( "prop_glass_futbol", GetAbsOrigin(), GetAbsAngles() ) );
	if ( pFutbol )
	{
		Vector vecVelocity;
		GetVelocity( &vecVelocity, NULL );
		pFutbol->SetAbsVelocity( vecVelocity );
		pFutbol->Break( this, CTakeDamageInfo( this, this, 200, DMG_BLAST ) );
	}

	string_t strTurretName = AllocPooledString( "npc_hover_turret" );
	string_t strPropName = AllocPooledString( "prop_weighted_cube" );

	Vector vecRadius( 128, 128, 128 );
	CBaseEntity *pEnts[128];
	int nNumFound = UTIL_EntitiesInBox( pEnts, ARRAYSIZE( pEnts ), GetAbsOrigin() - vecRadius, GetAbsOrigin() + vecRadius, 0 );
	for ( int i = 0; i < nNumFound; ++i )
	{
		if ( pEnts[i] == NULL || pEnts[i] == this )
			continue;

		// Only other hover turrets, cubes and players get hurt
		if ( pEnts[i]->m_iClassname != strTurretName && pEnts[i]->m_iClassname != strPropName && !pEnts[i]->IsPlayer() )
			continue;

		trace_t tr;
		UTIL_TraceLine( GetAbsOrigin(), pEnts[i]->WorldSpaceCenter(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );
		if ( tr.fraction < 1.0f )
			continue;

		CTakeDamageInfo info( this, this, 50, DMG_ENERGYBEAM );
		info.SetDamagePosition( GetAbsOrigin() );
		CalculateExplosiveDamageForce( &info, pEnts[i]->GetAbsOrigin() - GetAbsOrigin(), GetAbsOrigin() );
		pEnts[i]->DispatchTraceAttack( info, vec3_origin, &tr );
		ApplyMultiDamage();
	}

	ExplosionCreate( WorldSpaceCenter(), vec3_angle, this, 200, 100,
		SF_ENVEXPLOSION_NODAMAGE | SF_ENVEXPLOSION_NOSMOKE | SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NOFIREBALLSMOKE | SF_ENVEXPLOSION_NODLIGHTS, 0.0f );

	UTIL_ScreenShake( WorldSpaceCenter(), 5.0f, 150.0f, 1.0f, 750.0f, SHAKE_START );

	CPVSFilter filter( WorldSpaceCenter() );
	Vector gibVelocity = RandomVector( -30.0f, 30.0f );
	int iModelIndex = modelinfo->GetModelIndex( g_PropDataSystem.GetRandomChunkModel( "MetalChunks" ) );
	for ( int i = 0; i < 16; i++ )
	{
		te->BreakModel( filter, 0.0, WorldSpaceCenter(), vec3_angle, Vector( 16, 16, 16 ), gibVelocity, iModelIndex, 400, 1, 2.5, BREAK_METAL );
	}

	AddEffects( EF_NODRAW );
	SetThink( &CBaseEntity::SUB_Remove );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Create smoke trail!
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::CreateSmokeTrail( void )
{
	if ( m_hSmokeTrail != NULL )
		return;

	SmokeTrail *pSmokeTrail = SmokeTrail::CreateSmokeTrail();
	if ( !pSmokeTrail )
		return;

	pSmokeTrail->m_SpawnRate = 64;
	pSmokeTrail->m_ParticleLifetime = 0.5f;
	pSmokeTrail->m_StartSize = 8;
	pSmokeTrail->m_EndSize = 16;
	pSmokeTrail->m_SpawnRadius = 16;
	pSmokeTrail->m_MinSpeed = 60;
	pSmokeTrail->m_MaxSpeed = 100;

	pSmokeTrail->m_StartColor.Init( 0.5f, 0.5f, 0.5f );
	pSmokeTrail->m_EndColor.Init( 0, 0, 0 );

	pSmokeTrail->SetLifetime( -1 );
	pSmokeTrail->FollowEntity( this );

	m_hSmokeTrail = pSmokeTrail;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_HoverTurret::DestroySmokeTrail( void )
{
	if ( m_hSmokeTrail.Get() )
	{
		UTIL_Remove( m_hSmokeTrail );
		m_hSmokeTrail = NULL;
	}
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC( npc_hover_turret, CNPC_HoverTurret )

AI_END_CUSTOM_NPC()
