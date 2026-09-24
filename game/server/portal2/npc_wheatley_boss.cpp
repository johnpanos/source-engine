//========= Portal 2 reconstruction ============================================//
//
// Purpose: Register the scripted Wheatley boss actor entity.
//
// Retail RTTI identifies CAI_BaseActor as the parent. The historical Portal 2
// FGD calls this entity a stub; map scripts provide the boss behavior. Not
// original Valve source; the repository's provenance warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "ai_baseactor.h"

#include "tier0/memdbgon.h"

class CNPC_Wheatley_Boss : public CAI_BaseActor
{
	DECLARE_CLASS( CNPC_Wheatley_Boss, CAI_BaseActor );
};

LINK_ENTITY_TO_CLASS( npc_wheatley_boss, CNPC_Wheatley_Boss );
