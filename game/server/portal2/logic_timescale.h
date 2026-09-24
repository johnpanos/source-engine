//========= Portal 2 reconstruction ============================================//
//
// Purpose: logic_timescale: blends the game timescale through CGameTimescale.
//
// Written for the Portal 2 port from consumers, FGD data and related Valve code
// (no original source, pseudocode or debug data exists). Members and inputs
// follow the retail Portal 2 server's CLogicTimeScale datadesc. Not original
// Valve source; the repository's provenance and distribution warning applies.
//
// Server-only entity; nothing is networked.
//
//=============================================================================//

#ifndef LOGIC_TIMESCALE_H
#define LOGIC_TIMESCALE_H
#ifdef _WIN32
#pragma once
#endif

class CLogicTimeScale : public CLogicalEntity
{
public:
	DECLARE_CLASS( CLogicTimeScale, CLogicalEntity );
	DECLARE_DATADESC();

	CLogicTimeScale( void );

	void InputSetDesiredTimescale( inputdata_t &inputdata );
	void InputSetTimescaleBlendTime( inputdata_t &inputdata );

private:
	float m_flBlendTime;	// Real-time seconds to ramp to a newly requested timescale
};

#endif // LOGIC_TIMESCALE_H
