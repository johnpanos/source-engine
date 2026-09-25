//========= Portal 2 reconstruction ============================================//
//
// Purpose: Portal 2 floor turret (npc_portal_turret_floor) class declaration.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef NPC_PORTAL_TURRET_FLOOR_H
#define NPC_PORTAL_TURRET_FLOOR_H

#ifdef _WIN32
#pragma once
#endif

#include "npc_turret_floor.h"
#include "hitbox_damaged_entity.h"
#include "player_pickup_paint_power_user.h"

class CRopeKeyframe;
class CPortal_Base2D;
class CPortal_Player;

#define PORTAL_FLOOR_TURRET_NUM_ROPES	4

//-----------------------------------------------------------------------------
// Floor turret with portal awareness, paint, hitbox damage and tractor beams.
//-----------------------------------------------------------------------------
class CNPC_Portal_FloorTurret : public HitBoxDamagedEntity< PlayerPickupPaintPowerUser< CNPC_FloorTurret > >
{
	DECLARE_CLASS( CNPC_Portal_FloorTurret, HitBoxDamagedEntity< PlayerPickupPaintPowerUser< CNPC_FloorTurret > > );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:

	CNPC_Portal_FloorTurret( void );

	virtual void	Precache( void );
	virtual void	Spawn( void );
	virtual void	Activate( void );
	virtual void	UpdateOnRemove( void );
	virtual int		OnTakeDamage( const CTakeDamageInfo &info );

	virtual bool	ShouldAttractAutoAim( CBaseEntity *pAimingEnt );
	virtual float	GetAutoAimRadius();
	virtual Vector	GetAutoAimCenter();

	virtual void	OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );

	virtual void	NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params );

	virtual bool	PreThink( turretState_e state );
	virtual void	Shoot( const Vector &vecSrc, const Vector &vecDirToEnemy, bool bStrict = false );
	virtual void	SetEyeState( eyeState_t state );
	void			TryEmitSound( const char *soundname );

	virtual bool	OnSide( void );

	virtual float	GetAttackDamageScale( CBaseEntity *pVictim );
	virtual Vector	GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget );

	// Think functions
	virtual void	Retire( void );
	virtual void	Deploy( void );
	virtual void	ActiveThink( void );
	virtual void	SearchThink( void );
	virtual void	AutoSearchThink( void );
	virtual void	TippedThink( void );
	virtual void	HeldThink( void );
	virtual void	InactiveThink( void );
	virtual void	DieThink( void );
	virtual void	SuppressThink( void );
	virtual void	DisabledThink( void );
	virtual void	HackFindEnemy( void );
	virtual void	BurnThink( void );

	void			OnEnteredTractorBeam( void );
	void			OnExitedTractorBeam( void );
	void			TractorBeamThink( void );
	bool			m_bInTractorBeam;	// between OnEnteredTractorBeam() and OnExitedTractorBeam()

	virtual void	StartTouch( CBaseEntity *pOther );

	bool			IsLaserOn( void ) { return m_bLaserOn; }
	void			LaserOff( void );
	void			LaserOn( void );
	void			RopesOn( void );
	void			RopesOff( void );

	void			FireBullet( const char *pTargetName );

	// Retail-era addition used by CPortal_Player's "You Monster" achievement check
	bool			IsProjectedWallBlockingTurretFromPlayer( CPortal_Player *pPlayer );

	// Inputs
	void			InputFireBullet( inputdata_t &inputdata );
	void			InputEnableGagging( inputdata_t &inputdata );
	void			InputDisableGagging( inputdata_t &inputdata );
	void			InputEnablePickup( inputdata_t &inputdata );
	void			InputDisablePickup( inputdata_t &inputdata );
	void			InputSelfDestructImmediately( inputdata_t &inputdata );
	void			InputSetAsBouncePainted( inputdata_t &inputdata );
	void			InputSetModel( inputdata_t &inputdata );

	// Retail model variants (ModelIndex keyvalue and SetModel input)
	enum
	{
		TURRET_MODEL_NORMAL = 0,
		TURRET_MODEL_BOXED = 2,
		TURRET_MODEL_BACKWARDS = 3,
		TURRET_MODEL_SKELETON = 4,
	};
	void			SetTurretModel( int nModelIndex );
	float			GetTurretRange( void ) const { return m_flTurretRange; }

	virtual void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

protected:
	virtual float	GetFireConeZTolerance( void );

private:
	// Portal 2 port: turrets can burn (fire damage starts BurnThink, which blows
	// them up after sv_portal_turret_min/max_burn_time).
	virtual bool	AllowedToIgnite( void ) { return true; }
	void			StartBurning( void );

	CHandle<CRopeKeyframe>	m_hRopes[ PORTAL_FLOOR_TURRET_NUM_ROPES ];

	CNetworkVar( bool, m_bOutOfAmmo );
	CNetworkVar( bool, m_bLaserOn );
	CNetworkVar( int, m_sLaserHaloSprite );

	bool			m_bIsDead;

	int				m_iBarrelAttachments[ 4 ];
	bool			m_bShootWithBottomBarrels;
	bool			m_bDamageForce;
	bool			m_bPickupEnabled;

	float			m_fSearchSpeed;
	float			m_fMovingTargetThreashold;
	bool			m_bShootAtMovingObjects;
	float			m_flDistToEnemy;
	float			m_flPreviousVelocity;
	float			m_flBurnExplodeTime;

	turretState_e	m_iLastState;
	float			m_fNextTalk;
	bool			m_bDelayTippedTalk;
	bool			m_bUsedAsActor;
	bool			m_bGagged;
	bool			m_bSeeEnemyThroughPortal;

	// Retail keyvalues
	float			m_flTurretRange;				// TurretRange; 1024 when unset
	int				m_nCollisionType;				// CollisionType; 1 = debris
	bool			m_bAllowShootThroughPortals;	// AllowShootThroughPortals; set once picked up
	bool			m_bUseSuperDamageScale;			// UseSuperDamageScale
	bool			m_bLoadAlternativeModels;		// LoadAlternativeModels
	int				m_nTurretModelIndex;			// ModelIndex (TURRET_MODEL_*)
	bool			m_bDisableMotion;				// DisableMotion
	int				m_iNextShootingBarrel;			// retail fires one barrel per shot, in turn
	EHANDLE			m_hPreviousHeldOwner;
};

#endif // NPC_PORTAL_TURRET_FLOOR_H
