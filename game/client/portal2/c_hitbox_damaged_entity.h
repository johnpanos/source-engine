//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client interface for entities that show hitbox damage effects
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef C_HITBOX_DAMAGED_ENTITY_H
#define C_HITBOX_DAMAGED_ENTITY_H
#ifdef _WIN32
#pragma once
#endif

class IClientHitboxDamagedEntity
{
public:
	virtual ~IClientHitboxDamagedEntity();

	virtual void ApplyDamageEffect( int damageEffectIndex, int particleEffectIndex ) = 0;
};

#include "damage_database.h"

// Defined in hitbox_damaged_entity_shared.cpp
void EmitParticles( C_BaseAnimating *pOwnerEntity, DamagedEntityType entityType, int damageEffectIndex, int particleEffectIndex );
void EmitGibs( C_BaseAnimating *pParentEntity, DamagedEntityType entityType, int damageEffectIndex );

#endif // C_HITBOX_DAMAGED_ENTITY_H
