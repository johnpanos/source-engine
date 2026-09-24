//========= Portal 2 reconstruction ============================================//
//
// Purpose: Exploding futbol physics prop that can be caught, carried and detonated.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef PROP_EXPLODING_FUTBOL_H
#define PROP_EXPLODING_FUTBOL_H
#ifdef _WIN32
#pragma once
#endif

enum ExplodingFutbolHolderType_t
{
	EXPLODING_FUTBOL_HELD_BY_NONE = 0,
	EXPLODING_FUTBOL_HELD_BY_PLAYER,
	EXPLODING_FUTBOL_HELD_BY_SPAWNER,
	EXPLODING_FUTBOL_HELD_BY_CATCHER,

	EXPLODING_FUTBOL_HELD_BY_COUNT
};

#include "props.h"

class CPropExplodingFutbolSpawner;
class CPortal_Player;
class CPortal_Base2D;

class CPropExplodingFutbol : public CPhysicsProp
{
public:
	DECLARE_CLASS( CPropExplodingFutbol, CPhysicsProp );
	DECLARE_DATADESC();

	CPropExplodingFutbol();

	virtual void Precache( void );
	virtual void Spawn( void );

	virtual void Event_Killed( const CTakeDamageInfo &info );

	// Who is currently holding this futbol
	ExplodingFutbolHolderType_t GetHolder( void );
	void SetHolder( ExplodingFutbolHolderType_t holder );
	CPortal_Player *GetLastPlayerToHold( void );
	virtual void SetSpawner( CPropExplodingFutbolSpawner *pSpawner );
	CPropExplodingFutbolSpawner *GetSpawner( void );

	void KillThink( void );
	void AnimThink( void );
	void TimerThink( void );

	void ActivateFutbolTimer( float flTimer );
	void StopFutbolTimer( void );

	// Player pickup
	virtual void OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
	virtual void OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason );

	virtual int OnTakeDamage( const CTakeDamageInfo &info );
	virtual void OnFizzled( void );
	virtual void VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	virtual void VPhysicsUpdate( IPhysicsObject *pPhysics );

private:
	void KillFutbol( void );
	void DestroyFutbol( bool bExplode );
	void ExplodeFutbol( void );
	void DoExplosion( const Vector &vecCenter, float flExplosionRadius, CPortal_Base2D *pPortal );
	void DamageEntity( CBaseEntity *pEntity, const Vector &vecCenter, const Vector &vecForward );

	void InputExplode( inputdata_t &in );

	ExplodingFutbolHolderType_t m_Holder;
	CHandle<CPropExplodingFutbolSpawner> m_hSpawner;
	string_t m_strSpawnerName;
	CHandle<CPortal_Player> m_hLastHeldByPlayer;
	CUtlVector<CBaseEntity*> m_ExplodedEntities;

	bool m_bShouldRespawn;
	bool m_bTimerActive;
	bool m_bExplodeOnTouch;
	float m_flExplosionTimer;
	float m_flTotalTimer;
	float m_flLastTickTime;
	float m_flLastTimerSoundTime;
	float m_flLastFlashTime;
};

#endif // PROP_EXPLODING_FUTBOL_H
