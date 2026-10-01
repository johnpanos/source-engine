//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Revision: $
// $NoKeywords: $
//
// This file contains a little interface to deal with static prop collisions
//
//=============================================================================//

#ifndef STATICPROPMGR_H
#define STATICPROPMGR_H

#include "mathlib/vector.h"
#include "engine/IStaticPropMgr.h"

// FIXME: Remove! Only here for the test against old code
#include "enginetrace.h"


//-----------------------------------------------------------------------------
// foward declarations
//-----------------------------------------------------------------------------
class ICollideable;
FORWARD_DECLARE_HANDLE( LightCacheHandle_t );
class IPooledVBAllocator;
struct model_t;

//-----------------------------------------------------------------------------
// The engine's static prop manager
//-----------------------------------------------------------------------------
abstract_class IStaticPropMgrEngine
{
public:
	virtual bool Init() = 0;
	virtual void Shutdown() = 0;

	// Call at the beginning of the level, will unserialize all static
	// props and add them to the main collision tree
	virtual void LevelInit() = 0;

	// Call this when there's a client, *after* LevelInit, and after the world entity is loaded
	virtual void LevelInitClient() = 0;

	// Call this when there's a client, *before* LevelShutdown
	virtual void LevelShutdownClient() = 0;

	// Call at the end of the level, cleans up the static props
	virtual void LevelShutdown() = 0;

	// Call this to recompute static prop lighting when necessary
	virtual void RecomputeStaticLighting() = 0;

	// Check if a static prop is in a particular PVS.
	virtual bool IsPropInPVS( IHandleEntity *pHandleEntity, const byte *pVis ) const = 0;

	// returns a collideable interface to static props
	virtual ICollideable *GetStaticProp( IHandleEntity *pHandleEntity ) = 0;

	// returns the lightcache handle associated with a static prop
	virtual LightCacheHandle_t GetLightCacheHandleForStaticProp( IHandleEntity *pHandleEntity ) = 0;

	// Is a base handle a static prop?
	virtual bool IsStaticProp( IHandleEntity *pHandleEntity ) const = 0;
	virtual bool IsStaticProp( CBaseHandle handle ) const = 0;

	// Returns the static prop index (useful for networking)
	virtual int GetStaticPropIndex( IHandleEntity *pHandleEntity ) const = 0;

	virtual bool PropHasBakedLightingDisabled( IHandleEntity *pHandleEntity) const = 0;
};


//-----------------------------------------------------------------------------
// Singleton access
//-----------------------------------------------------------------------------
IStaticPropMgrEngine* StaticPropMgr();

// RFC 0016 K5 (render_core_world.cpp): the static props' world render boxes.
int StaticPropMgr_CorePropCount();
void StaticPropMgr_CorePropBounds( int nProp, Vector &mins, Vector &maxs );
int StaticPropMgr_CoreModelCount();
const model_t *StaticPropMgr_CoreModel( int nModel );
void StaticPropMgr_CorePropInfo( int nProp, const model_t **model, float world[12], int *skin,
    unsigned char *alpha, float modulation[3] );

#endif	// STATICPROPMGR_H
