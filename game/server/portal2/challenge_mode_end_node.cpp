//========= Portal 2 reconstruction ============================================//
//
// Purpose: Optional challenge-mode finish marker.
//
// The challenge position catalog and finish/timer logic are deferred while
// the base gameplay build is restored. Not original Valve source; the
// repository's provenance warning applies.
//
//=============================================================================//

#include "cbase.h"

#include "tier0/memdbgon.h"

class CChallengeModeEndNode : public CBaseAnimating
{
	DECLARE_CLASS( CChallengeModeEndNode, CBaseAnimating );

public:
	void Precache() OVERRIDE
	{
		PrecacheModel( "models/props/stopwatch_finish_line.mdl" );
		BaseClass::Precache();
	}

	void Spawn() OVERRIDE
	{
		Precache();
		SetModel( "models/props/stopwatch_finish_line.mdl" );
		BaseClass::Spawn();
		SetSolid( SOLID_NONE );
		SetMoveType( MOVETYPE_NONE );
	}
};

LINK_ENTITY_TO_CLASS( challenge_mode_end_node, CChallengeModeEndNode );
