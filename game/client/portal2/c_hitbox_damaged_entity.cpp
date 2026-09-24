//========= Portal 2 reconstruction ============================================//
//
// Purpose: Receives hitbox damage effects applied by the server
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#include "cbase.h"
#include "c_hitbox_damaged_entity.h"
#include "cliententitylist.h"
#include "hud_macros.h"
#include "tier0/memdbgon.h"

IClientHitboxDamagedEntity::~IClientHitboxDamagedEntity()
{
}

void __MsgFunc_ApplyHitBoxDamageEffect( bf_read &msg )
{
	const CBaseHandle hEntity( msg.ReadLong() );
	IClientHitboxDamagedEntity *pDamagedEntity = dynamic_cast< IClientHitboxDamagedEntity* >( cl_entitylist->GetBaseEntityFromHandle( hEntity ) );
	if ( pDamagedEntity )
	{
		const int damageEffectIndex = msg.ReadByte();
		const int particleEffectIndex = msg.ReadByte();
		pDamagedEntity->ApplyDamageEffect( damageEffectIndex, particleEffectIndex );
	}
}


class C_HitBoxDamageInitHelper : public CAutoGameSystem
{
	virtual bool Init()
	{
		HOOK_MESSAGE( ApplyHitBoxDamageEffect );

		return true;
	}
};

// Hooks the user message once the user message system is up
C_HitBoxDamageInitHelper g_HitBoxDamageInitHelper;
