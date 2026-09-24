//========= Portal 2 reconstruction ============================================//
//
// Purpose: Register the Hot Potato physics prop and its map entities.
//
// The retail server RTTI identifies CPhysicsProp as CHotPotato's parent,
// CBaseAnimating as CHotPotatoCatcher's parent, and the spawner/socket as
// catcher subclasses. The timed minigame behavior is deferred for the base
// campaign build. Not original Valve source; the provenance warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "props.h"

#include "tier0/memdbgon.h"

class CHotPotato : public CPhysicsProp
{
	DECLARE_CLASS( CHotPotato, CPhysicsProp );
};

class CHotPotatoCatcher : public CBaseAnimating
{
	DECLARE_CLASS( CHotPotatoCatcher, CBaseAnimating );

public:
	void Spawn() OVERRIDE
	{
		BaseClass::Spawn();
		SetSolid( SOLID_NONE );
		SetMoveType( MOVETYPE_NONE );
	}
};

class CHotPotatoSpawner : public CHotPotatoCatcher
{
	DECLARE_CLASS( CHotPotatoSpawner, CHotPotatoCatcher );
};

class CHotPotatoSocket : public CHotPotatoCatcher
{
	DECLARE_CLASS( CHotPotatoSocket, CHotPotatoCatcher );
};

LINK_ENTITY_TO_CLASS( hot_potato, CHotPotato );
LINK_ENTITY_TO_CLASS( hot_potato_catcher, CHotPotatoCatcher );
LINK_ENTITY_TO_CLASS( hot_potato_spawner, CHotPotatoSpawner );
LINK_ENTITY_TO_CLASS( hot_potato_socket, CHotPotatoSocket );
