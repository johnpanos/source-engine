//========= Portal 2 reconstruction ============================================//
//
// Purpose: Security camera (npc_security_camera) class declaration.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
// Reconstruction note: neither 2010 depot carries DWARF data for this header or
// its unit. The declaration follows the Portal 1 class in
// game/server/portal/npc_security_camera.cpp and adds the members the imported
// Portal 2 consumers use (IsActive, TauntedByPlayer, TauntedByPlayerFinished).
//
//=============================================================================//
#ifndef NPC_SECURITY_CAMERA_H
#define NPC_SECURITY_CAMERA_H

#ifdef _WIN32
#pragma once
#endif

#include "ai_basenpc.h"
#include "player_pickup.h"
// Shares turretState_e with the floor turrets so both headers can be included together
#include "npc_turret_floor.h"

class CRopeKeyframe;
class CSprite;
class CSoundPatch;
class CPortal_Player;

#define SECURITY_CAMERA_NUM_ROPES	2

//-----------------------------------------------------------------------------
// Security camera
//-----------------------------------------------------------------------------
class CNPC_SecurityCamera : public CNPCBaseInteractive<CAI_BaseNPC>, public CDefaultPlayerPickupVPhysics
{
	DECLARE_CLASS( CNPC_SecurityCamera, CNPCBaseInteractive<CAI_BaseNPC> );
public:

	CNPC_SecurityCamera( void );
	~CNPC_SecurityCamera( void );

	void			Precache( void );
	virtual void	CreateSounds( void );
	virtual void	StopLoopingSounds( void );
	virtual void	Spawn( void );
	virtual void	Activate( void );
	bool			CreateVPhysics( void );
	virtual void	UpdateOnRemove( void );
	virtual void	NotifySystemEvent( CBaseEntity *pNotify, notify_system_event_t eventType, const notify_system_event_params_t &params );
	virtual int		ObjectCaps( void );
	void			Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	// Think functions
	void	Retire( void );
	void	Deploy( void );
	void	ActiveThink( void );
	void	SearchThink( void );
	void	DeathThink( void );

	// Inputs
	void	InputToggle( inputdata_t &inputdata );
	void	InputEnable( inputdata_t &inputdata );
	void	InputDisable( inputdata_t &inputdata );
	void	InputRagdoll( inputdata_t &inputdata );

	void	SetLastSightTime();

	int		OnTakeDamage( const CTakeDamageInfo &inputInfo );
	virtual void	PlayerPenetratingVPhysics( void );
	bool	OnAttemptPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );

	bool	ShouldSavePhysics() { return true; }

	virtual bool CanBeAnEnemyOf( CBaseEntity *pEnemy );

	// Defined with the class implementation: the Portal 1 camera classifies as
	// CLASS_COMBINE while enabled, which is not part of every game's Class_T.
	Class_T	Classify( void );

	bool	FVisible( CBaseEntity *pEntity, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = NULL );

	Vector	EyeOffset( Activity nActivity )
	{
		Vector vForward;

		GetVectors( &vForward, 0, 0 );

		return vForward * 10.0f;
	}

	Vector	EyePosition( void )
	{
		return GetAbsOrigin() + EyeOffset( GetActivity() );
	}

	// Deployed and looking for targets
	bool	IsActive( void ) const { return m_bActive; }

	// Remote taunt camera support (see CPortal_Player::FindRemoteTauntViewpoint)
	void	TauntedByPlayer( CPortal_Player *pPlayer );
	void	TauntedByPlayerFinished( CPortal_Player *pPlayer );

protected:

	bool	PreThink( turretState_e state );
	void	Ping( void );
	void	Toggle( void );
	void	Enable( void );
	void	Disable( void );

	void	RopesOn( void );
	void	RopesOff( void );
	void	EyeOn( void );
	void	EyeOff( void );

	bool	UpdateFacing( void );

private:

	CHandle<CRopeKeyframe>	m_hRopes[ SECURITY_CAMERA_NUM_ROPES ];
	CHandle<CSprite>		m_hEyeGlow;

	bool	m_bAutoStart;
	bool	m_bActive;		//Denotes the turret is deployed and looking for targets
	bool	m_bBlinkState;
	bool	m_bEnabled;		//Denotes whether the turret is able to deploy or not

	float	m_flLastSight;
	float	m_flPingTime;

	QAngle	m_vecGoalAngles;
	QAngle	m_vecCurrentAngles;
	Vector	m_vNoisePos;
	int		m_iTicksTillNextNoise;

	CSoundPatch		*m_pMovementSound;

	COutputEvent m_OnDeploy;
	COutputEvent m_OnRetire;

	DECLARE_DATADESC();
};

#endif // NPC_SECURITY_CAMERA_H
