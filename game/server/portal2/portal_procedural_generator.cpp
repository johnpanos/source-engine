//========= Portal 2 reconstruction ============================================//
//
// Purpose: Optional Puzzle Maker procedural generator entity.
//
// Puzzle generation is deferred for the selected Linux build, which does not
// enable PORTAL2_PUZZLEMAKER. Not original Valve source; the repository's
// provenance warning applies.
//
//=============================================================================//

#include "cbase.h"

#include "tier0/memdbgon.h"

class CProceduralGenerator : public CBaseEntity
{
	DECLARE_CLASS( CProceduralGenerator, CBaseEntity );

public:
	void Spawn() OVERRIDE
	{
		BaseClass::Spawn();
		SetSolid( SOLID_NONE );
		SetMoveType( MOVETYPE_NONE );
	}
};

LINK_ENTITY_TO_CLASS( portal_procedural_generator, CProceduralGenerator );
