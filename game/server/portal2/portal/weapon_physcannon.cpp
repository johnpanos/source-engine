//========= Portal 2 reconstruction ============================================//
//
// Purpose: Portal 2's game/server/portal/weapon_physcannon.cpp. Portal 2 has no
//          gravity gun: the player's pickup and grab controllers live in
//          game/shared/portal2/portal_grabcontroller_shared.cpp, and this unit
//          only keeps the physcannon ConVars and the entry points that shared
//          HL2 code still calls, each answering "no physcannon".
//
// Reconstructed from the DWARF metadata and decompiler output of the Steam2
// depot 852 macOS server (every function returns immediately) and the
// retail string table (ConVar names and defaults). Not original Valve source;
// the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "portal/weapon_physcannon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar physcannon_minforce( "physcannon_minforce", "700" );
ConVar physcannon_maxforce( "physcannon_maxforce", "1500" );
ConVar physcannon_tracelength( "physcannon_tracelength", "250" );
ConVar player_throwforce( "player_throwforce", "1000" );

struct thrown_objects_t
{
	float				fTimeThrown;
	EHANDLE				hEntity;

	DECLARE_SIMPLE_DATADESC();
};

BEGIN_SIMPLE_DATADESC( thrown_objects_t )
	DEFINE_FIELD( fTimeThrown, FIELD_TIME ),
	DEFINE_FIELD( hEntity,	FIELD_EHANDLE	),
END_DATADESC()

bool PlayerHasMegaPhysCannon()
{
	return false;
}

void PhysCannonForceDrop( CBaseCombatWeapon *pActiveWeapon, CBaseEntity *pOnlyIfHoldingThis )
{
}

void PhysCannonBeginUpgrade( CBaseAnimating *pAnim )
{
}

float PhysCannonGetHeldObjectMass( CBaseCombatWeapon *pActiveWeapon, IPhysicsObject *pHeldObject )
{
	return 0.0f;
}

CBaseEntity *PhysCannonGetHeldEntity( CBaseCombatWeapon *pActiveWeapon )
{
	return NULL;
}

CGrabController *GetGrabControllerForPhysCannon( CBaseCombatWeapon *pActiveWeapon )
{
	return NULL;
}

bool PhysCannonAccountableForObject( CBaseCombatWeapon *pPhysCannon, CBaseEntity *pObject )
{
	return false;
}
