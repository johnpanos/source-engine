//========= F-Stop port =======================================================//
//
// Purpose: A particle the blobulator physics tiler finds neighbours for.
//
// Clean-room implementation of the blobulator physics interface the F-Stop
// blob NPCs (npc_surface, npc_fountain, npc_demomonster) use. It is not
// Valve's blobulator physics library, whose source is unavailable. The
// particles themselves are VPhysics spheres; this layer only carries the
// per-step copy the tiler bins and the forces the caller accumulates.
//
//=============================================================================//

#ifndef BLOBULATOR_PHYSPARTICLE_H
#define BLOBULATOR_PHYSPARTICLE_H
#ifdef _WIN32
#pragma once
#endif

#include "blobulator/Point3D.h"

class ALIGN16 PhysParticle
{
public:
	PhysParticle() : radius( 1.0f ), group( 0 ), neighbor_count( 0 ), temp1( 0 ), tilerIndex( -1 ) {}

	Point3D center;			// position, in the units of the tiler's interaction radius
	Point3D force;			// accumulated by the caller's force pass
	// The NPCs scale positions by 1 / sphere radius and leave this at 1, so two
	// particles in contact are 2 apart.
	float radius;
	int group;
	int neighbor_count;
	int temp1;				// caller scratch (npc_demomonster keeps its object index here)

	// PhysTiler bookkeeping: the particle's slot in the current frame, -1 if
	// it was not inserted. Callers do not set it.
	int tilerIndex;
};

#endif // BLOBULATOR_PHYSPARTICLE_H
