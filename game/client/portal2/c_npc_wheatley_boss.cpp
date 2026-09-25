//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the scripted Wheatley boss actor (npc_wheatley_boss).
//
// The retail Linux client.so names C_NPC_Wheatley_Boss and receives
// DT_NPC_Wheatley_Boss, which adds no properties to the base NPC table. Not
// original Valve source; the repository's provenance warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_ai_basenpc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_NPC_Wheatley_Boss : public C_AI_BaseNPC
{
public:
	DECLARE_CLASS( C_NPC_Wheatley_Boss, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();
};

IMPLEMENT_CLIENTCLASS_DT( C_NPC_Wheatley_Boss, DT_NPC_Wheatley_Boss, CNPC_Wheatley_Boss )
END_RECV_TABLE()
