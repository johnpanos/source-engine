//========= F-Stop port =======================================================//
//
// Purpose: Client side of the F-Stop blob NPCs (npc_surface, npc_blob_fountain,
//          npc_blob_demomonster).
//
// The F-Stop drop has only the server NPCs; their client classes are written
// here against the server send tables. The particle positions and radii come
// through the blob network bypass (game/shared/fstop/blob_networkbypass), and
// the surface is drawn as an isosurface with the clean-room blobulator tiler
// (public/blobulator/Implicit/ImpTiler.h), as CS:GO's c_surfacerender and the
// Portal 2 paint renderer do.
//
//=============================================================================//

#ifndef C_NPC_SURFACE_H
#define C_NPC_SURFACE_H
#ifdef _WIN32
#pragma once
#endif

#include "c_ai_basenpc.h"
#include "utlvector.h"

// Must match game/server/fstop/npc_surface.h.
#define MAX_SURFACE_ELEMENTS 750

class C_NPC_Surface : public C_AI_BaseNPC
{
	DECLARE_CLASS( C_NPC_Surface, C_AI_BaseNPC );
public:
	DECLARE_CLIENTCLASS();

	C_NPC_Surface();

	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void ClientThink();
	virtual void GetRenderBounds( Vector &mins, Vector &maxs );
	virtual int DrawModel( int flags );
	virtual bool IsTransparent() { return false; }

	// Indexes into the network bypass arrays, one per active particle.
	CUtlVector<uint16> m_iParticlePositionIndex;
	int m_nActiveParticles;
	float m_flRadius;

private:
	// Collects the live particles (interpolated positions relative to the
	// render origin, radius > 0) into m_Particles; returns their count.
	int GatherParticles();

	Vector m_vecRenderMins;
	Vector m_vecRenderMaxs;
};

class C_NPC_BlobFountain : public C_NPC_Surface
{
	DECLARE_CLASS( C_NPC_BlobFountain, C_NPC_Surface );
public:
	DECLARE_CLIENTCLASS();
};

class C_NPC_BlobDemoMonster : public C_NPC_Surface
{
	DECLARE_CLASS( C_NPC_BlobDemoMonster, C_NPC_Surface );
public:
	DECLARE_CLIENTCLASS();

	CUtlVector<float> m_flSurfaceV;
};

#endif // C_NPC_SURFACE_H
