//========= F-Stop port =======================================================//
//
// Purpose: Neighbour search for blob particles simulated by VPhysics.
//
// Clean-room implementation of the blobulator physics interface the F-Stop
// blob NPCs use (see PhysParticle.h). Each step the NPC copies its VPhysics
// sphere positions into PhysParticles, the tiler finds every pair closer than
// the interaction radius, the NPC computes Lennard-Jones and surface-tension
// forces over those pairs, and VPhysics (a motion controller or
// ApplyForceCenter) integrates them. Integration and collision stay in the
// physics provider; this layer is only the pair search.
//
// Frame protocol (one tiler per search; from PhysTilerFactory or owned):
//   beginFrame -> insertParticle* -> processTiles -> getParticleCache()->get()*
//   -> endFrame
//
// processTiles bins the particles into a uniform grid whose cells are one
// interaction radius wide, anchored at beginFrame's origin, and tests each
// particle against its own and the 26 adjacent cells. The result is exactly
// the brute-force set { (a, b) : a != b, |a - b|^2 < r^2 } computed in float,
// in an order fixed by the inputs (unittests/blobulatortest/test_phystiler.cpp).
// A tiler is used by one thread at a time; separate tilers are independent.
//
//=============================================================================//

#ifndef BLOBULATOR_PHYSTILER_H
#define BLOBULATOR_PHYSTILER_H
#ifdef _WIN32
#pragma once
#endif

#include "blobulator/physics/PhysParticle.h"
#include "blobulator/physics/PhysParticleCache.h"
#include "blobulator/SmartArray.h"
#include "tier0/threadtools.h"

class PhysTiler
{
public:
	explicit PhysTiler( float flInteractionRadius = 1.0f );

	// The pair distance, in the particles' units; clamped to a small positive
	// value. Takes effect at the next processTiles.
	void setInteractionRadius( float flInteractionRadius );
	float getInteractionRadius() const { return m_flInteractionRadius; }

	// Starts a frame: forgets the previous frame's particles and lists. The
	// origin anchors the grid; any origin gives the same pairs.
	void beginFrame( const Point3D &origin );
	// The particle must stay alive and unmoved until endFrame.
	void insertParticle( PhysParticle *pParticle );
	void processTiles();
	PhysParticleCache *getParticleCache() { return &m_Cache; }
	void endFrame();

	int getParticleCount() const { return m_Particles.size; }

private:
	struct CellEntry_t
	{
		unsigned long long key;
		int nParticle;
	};

	unsigned long long CellKey( const Point3D &center, int nOffsetX, int nOffsetY, int nOffsetZ ) const;

	float m_flInteractionRadius;
	Point3D m_Origin;
	SmartArray<PhysParticle *, false, 16> m_Particles;
	SmartArray<CellEntry_t, false, 16> m_Cells;		// sorted by (key, particle)
	PhysParticleCache m_Cache;
};

class PhysTilerFactory
{
public:
	static PhysTilerFactory *factory;

	// A tiler with interaction radius 1; set it before processTiles.
	PhysTiler *getTiler();
	void returnTiler( PhysTiler *pTiler );

	PhysTilerFactory() {}
	~PhysTilerFactory();

private:
	SmartArray<PhysTiler *, false, 16> m_Tilers;
	CThreadFastMutex m_Mutex;
};

#endif // BLOBULATOR_PHYSTILER_H
