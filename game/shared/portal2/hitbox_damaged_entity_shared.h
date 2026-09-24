//========= Copyright © 1996-2009, Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared helpers for hitbox damage effects.
//
//=============================================================================//

#ifndef HITBOX_DAMAGED_ENTITY_SHARED_H
#define HITBOX_DAMAGED_ENTITY_SHARED_H

#ifdef _WIN32
#pragma once
#endif

#include "damage_database.h"

#if defined( CLIENT_DLL )
class C_BaseAnimating;
typedef C_BaseAnimating CAnimatingForDamageEffects;
#else
class CBaseAnimating;
typedef CBaseAnimating CAnimatingForDamageEffects;
#endif

void EmitParticles( CAnimatingForDamageEffects *pOwnerEntity, DamagedEntityType entityType,
    int damageEffectIndex, int particleEffectIndex );
void EmitGibs( CAnimatingForDamageEffects *pParentEntity, DamagedEntityType entityType,
    int damageEffectIndex );

#endif // HITBOX_DAMAGED_ENTITY_SHARED_H
