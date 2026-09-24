//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client floor turret (C_NPC_Portal_FloorTurret) class declaration.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
// Reconstruction note: neither 2010 client depot has DWARF data for this class.
// The declaration matches the class body in game/client/portal/
// c_npc_portal_turret_floor.cpp, whose receive table (m_bOutOfAmmo, m_bLaserOn,
// m_sLaserHaloSprite) agrees with the Portal 2 server header's network vars.
//
//=============================================================================//
#ifndef C_NPC_PORTAL_TURRET_FLOOR_H
#define C_NPC_PORTAL_TURRET_FLOOR_H

#ifdef _WIN32
#pragma once
#endif

#include "c_ai_basenpc.h"
#include "beam_shared.h"

class C_NPC_Portal_FloorTurret : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_NPC_Portal_FloorTurret, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();

	virtual ~C_NPC_Portal_FloorTurret( void );

	virtual void	Spawn( void );
	virtual void	ClientThink( void );

	bool	IsLaserOn( void ) { return m_pBeam != NULL; }
	void	LaserOff( void );
	void	LaserOn( void );
	float	LaserEndPointSize( void );

private:
	CBeam	*m_pBeam;

	bool	m_bOutOfAmmo;
	bool	m_bLaserOn;
	int		m_sLaserHaloSprite;
	float	m_fPulseOffset;

	float	m_bBeamFlickerOff;
	float	m_fBeamFlickerTime;
};

#endif // C_NPC_PORTAL_TURRET_FLOOR_H
