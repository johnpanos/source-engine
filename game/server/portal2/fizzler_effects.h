//========= Portal 2 reconstruction ============================================//
//
// Purpose: Effect entity attached to objects being fizzled by a cleanser field
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef FIZZLER_EFFECTS_H
#define FIZZLER_EFFECTS_H
#ifdef _WIN32
#pragma once
#endif

class FizzlerEffects : public CBaseEntity
{
	DECLARE_CLASS( FizzlerEffects, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:
	static FizzlerEffects *Create( CBaseEntity *pAttachTo );

	virtual void Spawn( void );
	virtual int UpdateTransmitState( void ) { return SetTransmitState( FL_EDICT_PVSCHECK ); }
	void AttachToEntity( CBaseEntity *pAttachTo );

private:
	void FizzleThink( void );
};

#endif // FIZZLER_EFFECTS_H
