//========= Portal 2 reconstruction ============================================//
//
// Purpose: Database of hitbox damage effects (wounds, gibs, particles) loaded
//			from the scripts/damageinfo_* files
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef DAMAGE_DATABASE_H
#define DAMAGE_DATABASE_H

#ifdef _WIN32
#pragma once
#endif

#include "igamesystem.h"
#include "utlvector.h"


//-----------------------------------------------------------------------------
// Each kind of entity that can be damaged by hitbox has its own damage info
// file and its own list of damage effects.
//-----------------------------------------------------------------------------
enum DamagedEntityType
{
	DAMAGED_TURRET = 0,

	DAMAGED_ENTITY_TYPE_COUNT
};


//-----------------------------------------------------------------------------
// A single damage effect read from a damage info file
//-----------------------------------------------------------------------------
struct DamageInfo
{
	char damageEffectName[64];

	int hitBoxIndices[8];
	int hitBoxCount;

	char exclusionListStrings[16][64];
	int exclusionListIndices[16];
	int exclusionCount;

	char deathThroesAnimation[64];

	int damageType;

	char gibModels[16][64];
	int gibCount;
	float gibSeparationSpeed;
	float gibLifeTime;

	char particleAttachment[64];
	int particleAttachmentHitbox;
	char particleEffects[8][64];
	int particleEffectsChance[8];
	int particleEffectTotalChance;
	int particleEffectCount;

	bool isFatalDamage;

	char swapModelName[64];
};


typedef CUtlVector< DamageInfo > DamageInfoVector;


class DamageDatabase : public CAutoGameSystem
{
public:
	DamageDatabase();

	virtual void LevelInitPreEntity();

	const DamageInfoVector &GetDamageInfoVector( DamagedEntityType entityType ) const;
	int FindPotentialDamageEffects( int *pPotentialDamageEffects, int maxDamageEffects, DamagedEntityType entityType, int damageType, int hitbox ) const;

private:
	DamageInfoVector m_DamageInfo[DAMAGED_ENTITY_TYPE_COUNT];

#ifdef GAME_DLL
	void PrecacheAssets() const;
#endif
};

extern DamageDatabase g_DamageDatabase;

#endif // DAMAGE_DATABASE_H
