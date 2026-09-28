//========= F-Stop port =======================================================//
//
// Purpose: Per-particle neighbour lists that PhysTiler::processTiles builds.
//
// Clean-room implementation of the blobulator physics interface; see
// PhysParticle.h. get() returns the particle's list: every other inserted
// particle whose centre is strictly within the interaction radius, with the
// squared distance, ending at an entry whose particle is NULL. Lists are
// symmetric (b is in a's list exactly when a is in b's), so a caller that
// handles a pair only when a < b visits each pair once.
//
//=============================================================================//

#ifndef BLOBULATOR_PHYSPARTICLECACHE_H
#define BLOBULATOR_PHYSPARTICLECACHE_H
#ifdef _WIN32
#pragma once
#endif

#include "blobulator/physics/PhysParticle.h"
#include "blobulator/SmartArray.h"
#include "tier0/dbg.h"

struct PhysParticleAndDist
{
	PhysParticle *particle;	// NULL ends the list
	float distSq;
};

class PhysParticleCache
{
public:
	// The neighbour list of a particle inserted this frame, valid until the
	// tiler's next beginFrame. A particle that was not inserted gets an empty
	// list.
	PhysParticleAndDist *get( const PhysParticle *pParticle )
	{
		int nIndex = pParticle ? pParticle->tilerIndex : -1;
		if ( nIndex < 0 || nIndex >= m_Start.size || m_Owners[nIndex] != pParticle )
		{
			Assert( !"PhysParticleCache::get: particle not inserted this frame" );
			return &m_Empty;
		}
		return &m_Nodes[m_Start[nIndex]];
	}

	int NodeCount() const { return m_Nodes.size; }

private:
	friend class PhysTiler;

	SmartArray<PhysParticleAndDist, false, 16> m_Nodes;	// all lists, each NULL-terminated
	SmartArray<int, false, 16> m_Start;					// per slot: offset of its list
	SmartArray<const PhysParticle *, false, 16> m_Owners;	// per slot: the inserted particle
	PhysParticleAndDist m_Empty = { NULL, 0.0f };
};

#endif // BLOBULATOR_PHYSPARTICLECACHE_H
