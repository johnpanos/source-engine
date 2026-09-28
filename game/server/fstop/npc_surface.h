//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#ifndef NPC_SURFACE_H
#define NPC_SURFACE_H

#if defined( _WIN32 )
#pragma once
#endif

#include "ai_basenpc.h"
#include "soundenvelope.h"
#include "player_pickup.h"
#include "blobulator/SmartArray.h"
#include "blob_networkbypass.h"

class CNPC_Surface;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#define			MAX_SURFACE_ELEMENTS 750 // Ilya: Reduced because I was getting max prop buffer error 1000 // 200
#define			MAX_SURFACE_ELEMENTS_BITS 10 // How many bits are needed to send a number from 0 to MAX_SURFACE_ELEMENTS-1


class CNPC_Surface;

#include "blobulator/physics/PhysParticle.h"
#include "blobulator/physics/PhysParticleCache.h"
#include "blobulator/physics/PhysTiler.h"


// Valve compiled the Lennard-Jones cohesion out here ("#undef USE_BLOBULATOR //
// TODO (Ilya): I need to get this working again"). It is built, with its
// neighbour search on the clean-room PhysTiler, and runs only when
// sv_blob_lennard_jones is set, so the default is the F-Stop behaviour.
extern ConVar sv_blob_lennard_jones;

#if !defined(_X360)

extern ConVar sv_lj_strength;

class CLennardJonesForce
{
public:
	CLennardJonesForce( );
	~CLennardJonesForce( );

public:
	void InitParams( void )
	{
	}

	virtual void AddForces( 
		CUtlVector< IPhysicsObject* > &pObjects, 
		int nObjects, 
		float flRadius,
		float flStrength,
		Vector *pForces );

	//ParticleCache* m_pParticleCache;
	PhysTiler* m_pPhysTiler;				// owned; deleted in the destructor
	SmartArray<PhysParticle, false, 16> m_Particles;	// per-step copies of the VPhysics spheres
	float m_fInteractionRadius;
	float m_fSurfaceTension;
	float m_fLennardJonesRepulsion;
	float m_fLennardJonesAttraction;
	float m_fMaxRepulsion;
	float m_fMaxAttraction;

private:
	virtual void addParticleForce(PhysParticle* a, PhysParticle* b, float distSq, float flStrength, float ts) const;
};

#endif

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

//We'll be using the physics objects' GameIndex() to access an array of extra data for each individual particle. That data goes here.
struct ExpandedParticleData_t
{
	ExpandedParticleData_t( void ) : iEntityIndex( 0 ) { };
	int iEntityIndex; //the index of this particle within it's owner entity.
	int iNetworkBypassIndex;
};

class CNPC_Surface : public CAI_BaseNPC, public CDefaultPlayerPickupVPhysics
{
	DECLARE_CLASS( CNPC_Surface, CAI_BaseNPC );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:
	CNPC_Surface( void );

	virtual void		Spawn( void );
	virtual void		Precache( void );
	virtual void		Activate( void );
	virtual int		UpdateTransmitState();

	virtual void		Teleport( const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity );

	virtual Class_T		Classify( void );

	virtual int			Save( ISave &save );
	virtual int			Restore( IRestore &restore );
	virtual void		OnSave( IEntitySaveUtils *pUtils );
	virtual void		OnRestore( void );
	virtual bool		ShouldSavePhysics( void ) { return false; }

	bool m_physicsCreated;
	virtual void		VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	virtual bool		CreateVPhysics();
	virtual void		CreateBlobPhysics();
	virtual int			VPhysicsGetObjectList( IPhysicsObject **pList, int listMax );
	virtual bool		VPhysicsIsFlesh( void );
	virtual void		CleanupOnDeath( CBaseEntity *pCulprit, bool bFireDeathOutput);
	virtual void		VPhysicsDestroyObject();
	virtual void		UpdateOnRemove();
	virtual void		VPhysicsUpdate( IPhysicsObject *pPhysics );

	// Player pickup
	virtual void		OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
	virtual void		OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason );
	virtual bool		OnAttemptPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );

	virtual bool		TestCollision( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr );

	virtual float		MaxYawSpeed( void );

	virtual void		PrescheduleThink( void );

	virtual void        HandleAnimEvent( animevent_t *pEvent );

	static CUtlVector<CNPC_Surface *> s_BlobInstances;
	//static void PostPhysFrame_Static( float deltaTime );
	//virtual void PostPhysFrame( float deltaTime );

	Vector		m_vecStart;

	// Networked data
	//CUtlVector< Vector > m_vecSurfacePos;			// Sent via bypass (see blob_networkbypass.cpp)
	CUtlVector<uint16> m_iParticlePositionIndex;	// Indexes into data sent via bypass
	//CUtlVector< float > m_flSurfaceR;				// Sent via network bypass

	CNetworkVar( int, m_nActiveParticles );
	CNetworkVar( float, m_flRadius );

	// Physics data
	CUtlVector< IPhysicsObject* > m_vecPhysParticles;

public:
	virtual void Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular ) { };

	virtual Vector		EyePosition( void );
	virtual const QAngle &EyeAngles( void );

	virtual	Vector		BodyTarget( const Vector &posSrc, bool bNoisy) ;

	// --------------------------------

	// DEFINE_CUSTOM_AI;

	static ExpandedParticleData_t s_ExpandedParticleData[BLOB_MAX_LEVEL_PARTICLES];
	IPhysicsObject *CreateParticlePhysics( void );
	static void DestroyParticlePhysics( IPhysicsObject *pParticle );

	void SetParticleEntityIndex( IPhysicsObject *pParticle, int iNewIndex );

	static void UpdateBypassParticleData( void );

private:
	static float INVALID_PARTICLE_RADIUS;
	static float DEFAULT_PARTICLE_RADIUS;

	Vector * m_pRestorePositions;
	float  * m_pRestoreRadii;
	Vector * m_pRestoreClosestSurfDirs;
};


//-----------------------------------------------------------------------------


extern ConVar sv_surface_tension;
extern ConVar sv_surface_radius;
extern ConVar sv_surface_ideal;
extern ConVar sv_surface_nearby;
extern ConVar sv_surface_scale;

#endif // NPC_SURFACE_H
