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

#include "cbase.h"
#include "damage_database.h"
#include "gamerules.h"
#include "weapon_parse.h"
#include "particle_parse.h"
#include "filesystem.h"
#include "KeyValues.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar max_hitbox_damage_effects_per_entity( "max_hitbox_damage_effects_per_entity", "2",
    FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY,
    "Maximum number of damage effects an entity can have." );
ConVar hitbox_damage_enabled( "hitbox_damage_enabled", "0",
    FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY, "Enable/disable hitbox damage." );

bool ReadDamageCutoutDataFromFile( DamageInfoVector &damageInfoArray, IFileSystem *filesystem,
    const char *szCutoutDataFilename, const unsigned char *pICEKey );
int ParseStringToIntArray( const char *pString, int *pIntArray, int nMaxArray );
int ParseStringToExclusionArray( const char *pString, char ( *ppStringArray )[64], int nMaxArray );
int ParseStringToGibArray(
    const char *pString, const char *modelPath, char ( *ppStringArray )[64], int nMaxArray );
int ParseStringToEffectArray(
    const char *pString, char ( *ppEffectArray )[64], int *pChanceArray, int nMaxArray );
void ParseStringToVector( const char *pString, Vector &vector );

DamageDatabase g_DamageDatabase;

// One damage info file per damaged entity type
static const char *s_pDamageInfoFileNames[DAMAGED_ENTITY_TYPE_COUNT] = {
    "scripts/damageinfo_turret", // DAMAGED_TURRET
};

DamageDatabase::DamageDatabase() : CAutoGameSystem( "DamageDatabase" )
{
}

void DamageDatabase::LevelInitPreEntity()
{
	for ( int i = 0; i < DAMAGED_ENTITY_TYPE_COUNT; ++i )
	{
		ReadDamageCutoutDataFromFile( m_DamageInfo[i], filesystem, s_pDamageInfoFileNames[i],
		    g_pGameRules->GetEncryptionKey() );
	}

	// Reconstruction note: only the server binary precaches the gib models and
	// particle systems; the client's LevelInitPreEntity only reads the file.
#ifdef GAME_DLL
	PrecacheAssets();
#endif
}

const DamageInfoVector &DamageDatabase::GetDamageInfoVector( DamagedEntityType entityType ) const
{
	return m_DamageInfo[entityType];
}

//-----------------------------------------------------------------------------
// Purpose: Fills pPotentialDamageEffects with the indices of every damage effect
//			that responds to damageType on the given hitbox (-1 = any hitbox).
//			Returns the number of indices written.
//-----------------------------------------------------------------------------
int DamageDatabase::FindPotentialDamageEffects( int *pPotentialDamageEffects, int maxDamageEffects,
    DamagedEntityType entityType, int damageType, int hitbox ) const
{
	const DamageInfoVector &damageInfo = m_DamageInfo[entityType];

	int potentialDamageEffectCount = 0;
	for ( int damageIndex = 0;
	    damageIndex < damageInfo.Count() && potentialDamageEffectCount < maxDamageEffects;
	    ++damageIndex )
	{
		if ( ( damageInfo[damageIndex].damageType & damageType ) == 0 )
			continue;

		if ( hitbox == -1 )
		{
			pPotentialDamageEffects[potentialDamageEffectCount++] = damageIndex;
			continue;
		}

		for ( int hitboxIndex = 0; hitboxIndex < damageInfo[damageIndex].hitBoxCount &&
		                           potentialDamageEffectCount < maxDamageEffects;
		    ++hitboxIndex )
		{
			const int nEffectHitbox = damageInfo[damageIndex].hitBoxIndices[hitboxIndex];
			if ( nEffectHitbox >= 0 && nEffectHitbox == hitbox )
			{
				pPotentialDamageEffects[potentialDamageEffectCount++] = damageIndex;
			}
		}
	}

	return potentialDamageEffectCount;
}

#ifdef GAME_DLL
void DamageDatabase::PrecacheAssets() const
{
	for ( int damageInfoIndex = 0; damageInfoIndex < DAMAGED_ENTITY_TYPE_COUNT; ++damageInfoIndex )
	{
		const DamageInfoVector &damageInfo = m_DamageInfo[damageInfoIndex];
		for ( int i = 0; i < damageInfo.Count(); ++i )
		{
			CBaseEntity::PrecacheModel( damageInfo[i].swapModelName );

			// Gib models
			for ( int j = 0; j < damageInfo[i].gibCount; ++j )
			{
				CBaseEntity::PrecacheModel( damageInfo[i].gibModels[j] );
			}

			// Particle effects
			for ( int j = 0; j < damageInfo[i].particleEffectCount; ++j )
			{
				PrecacheParticleSystem( damageInfo[i].particleEffects[j] );
			}
		}
	}
}
#endif // GAME_DLL

//-----------------------------------------------------------------------------
// Purpose: Reads a damage info (cutout) file into damageInfoArray. Does nothing
//			if the array has already been filled.
//-----------------------------------------------------------------------------
bool ReadDamageCutoutDataFromFile( DamageInfoVector &damageInfoArray, IFileSystem *filesystem,
    const char *szCutoutDataFilename, const unsigned char *pICEKey )
{
	if ( damageInfoArray.Count() > 0 )
		return false;

	KeyValues *pKV = ReadEncryptedKVFile( filesystem, szCutoutDataFilename, pICEKey );
	if ( !pKV )
		return false;

	KeyValues *pModelPathKey = pKV->FindKey( "ModelPath" );
	const char *modelPath = pModelPathKey ? pModelPathKey->GetString() : "";

	for ( KeyValues *pMainDamageKeyValues = pKV->GetFirstTrueSubKey(); pMainDamageKeyValues;
	    pMainDamageKeyValues = pMainDamageKeyValues->GetNextTrueSubKey() )
	{
		DamageInfo newInfo;

		// Model to swap to once this damage is applied
		KeyValues *pKey = pMainDamageKeyValues->FindKey( "SwapModel" );
		if ( pKey )
		{
			V_snprintf( newInfo.swapModelName, sizeof( newInfo.swapModelName ), "%s/%s.mdl",
			    modelPath, pKey->GetString() );
		}
		else
		{
			newInfo.swapModelName[0] = '\0';
		}

		// Hitboxes this damage applies to
		pKey = pMainDamageKeyValues->FindKey( "HitBoxes" );
		newInfo.hitBoxCount = pKey ? ParseStringToIntArray( pKey->GetString(),
		                                 newInfo.hitBoxIndices, ARRAYSIZE( newInfo.hitBoxIndices ) )
		                           : 0;

		// Damage effects that can't be combined with this one
		pKey = pMainDamageKeyValues->FindKey( "ExclusionList" );
		newInfo.exclusionCount =
		    pKey ? ParseStringToExclusionArray( pKey->GetString(), newInfo.exclusionListStrings,
		               ARRAYSIZE( newInfo.exclusionListStrings ) )
		         : 0;

		// Death animation
		newInfo.deathThroesAnimation[0] = '\0';
		pKey = pMainDamageKeyValues->FindKey( "DeathThroes" );
		if ( pKey )
		{
			V_strncpy( newInfo.deathThroesAnimation, pKey->GetString(),
			    sizeof( newInfo.deathThroesAnimation ) );
		}

		// Reconstruction note: both binaries skip (and never store) an entry that
		// has no "Type" key.
		pKey = pMainDamageKeyValues->FindKey( "Type" );
		if ( !pKey )
			continue;

		// Damage types that cause this effect
		newInfo.damageType = 0;
		if ( V_stristr( pKey->GetString(), "heat" ) )
		{
			newInfo.damageType |= DMG_BURN;
		}
		if ( V_stristr( pKey->GetString(), "explosion" ) )
		{
			newInfo.damageType |= DMG_BLAST;
		}
		if ( V_stristr( pKey->GetString(), "fizzler" ) )
		{
			newInfo.damageType |= DMG_DISSOLVE;
		}
		if ( V_stristr( pKey->GetString(), "corrosive" ) )
		{
			newInfo.damageType |= DMG_ACID;
		}
		if ( V_stristr( pKey->GetString(), "slice" ) )
		{
			newInfo.damageType |= DMG_SLASH;
		}
		if ( V_stristr( pKey->GetString(), "crush" ) )
		{
			newInfo.damageType |= DMG_CRUSH;
		}
		if ( V_stristr( pKey->GetString(), "impact" ) )
		{
			newInfo.damageType |= DMG_FALL;
		}

		pKey = pMainDamageKeyValues->FindKey( "Fatal" );
		newInfo.isFatalDamage = pKey ? pKey->GetBool() : false;

		// Gibs
		pKey = pMainDamageKeyValues->FindKey( "Gibs" );
		newInfo.gibCount = ParseStringToGibArray(
		    pKey->GetString(), modelPath, newInfo.gibModels, ARRAYSIZE( newInfo.gibModels ) );

		pKey = pMainDamageKeyValues->FindKey( "Gib Lifetime" );
		newInfo.gibLifeTime = pKey ? pKey->GetFloat() : 0.0f;

		pKey = pMainDamageKeyValues->FindKey( "Gib Speed" );
		newInfo.gibSeparationSpeed = pKey ? pKey->GetFloat() : 0.0f;

		// Particles
		newInfo.particleAttachment[0] = '\0';
		pKey = pMainDamageKeyValues->FindKey( "ParticleAttach" );
		if ( pKey )
		{
			V_strncpy( newInfo.particleAttachment, pKey->GetString(),
			    sizeof( newInfo.particleAttachment ) );
		}

		pKey = pMainDamageKeyValues->FindKey( "ParticleAttachHitbox" );
		newInfo.particleAttachmentHitbox = pKey ? pKey->GetInt() : -1;

		pKey = pMainDamageKeyValues->FindKey( "ParticleEffect" );
		newInfo.particleEffectCount =
		    ParseStringToEffectArray( pKey->GetString(), newInfo.particleEffects,
		        newInfo.particleEffectsChance, ARRAYSIZE( newInfo.particleEffects ) );

		newInfo.particleEffectTotalChance = 0;
		for ( int j = 0; j < newInfo.particleEffectCount; ++j )
		{
			newInfo.particleEffectTotalChance += newInfo.particleEffectsChance[j];
		}

		// Reconstruction note: the binaries pass the key name as the format string.
		V_snprintf( newInfo.damageEffectName, sizeof( newInfo.damageEffectName ), "%s",
		    pMainDamageKeyValues->GetName() );

		damageInfoArray.AddToTail( newInfo );
	}

	// Resolve the exclusion list names to damage effect indices
	for ( int i = 0; i < damageInfoArray.Count(); ++i )
	{
		for ( int j = 0; j < damageInfoArray[i].exclusionCount; ++j )
		{
			damageInfoArray[i].exclusionListIndices[j] = -1;

			for ( int k = 0; k < damageInfoArray.Count(); ++k )
			{
				if ( !V_stricmp( damageInfoArray[i].exclusionListStrings[j],
				         damageInfoArray[k].damageEffectName ) )
				{
					damageInfoArray[i].exclusionListIndices[j] = k;
					break;
				}
			}

			if ( damageInfoArray[i].exclusionListIndices[j] == -1 )
			{
				Warning( "Unable to find exclusion %s for damage effect %s.\n",
				    damageInfoArray[i].exclusionListStrings[j],
				    damageInfoArray[i].damageEffectName );
			}
		}
	}

	pKV->deleteThis();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Parses a space separated list of integers
//-----------------------------------------------------------------------------
int ParseStringToIntArray( const char *pString, int *pIntArray, int nMaxArray )
{
	int nInts = 0;

	char buffer[1024];
	V_strncpy( buffer, pString, sizeof( buffer ) );

	char *pBuffer = buffer;
	char *pSpace = V_stristr( pBuffer, " " );
	while ( pSpace && nInts < nMaxArray - 1 )
	{
		*pSpace = '\0';
		pIntArray[nInts++] = V_atoi( pBuffer );

		pBuffer = pSpace + 1;
		pSpace = V_stristr( pBuffer, " " );
	}

	if ( *pBuffer )
	{
		pIntArray[nInts++] = V_atoi( pBuffer );
	}

	return nInts;
}

//-----------------------------------------------------------------------------
// Purpose: Parses a space separated list of damage effect names
//-----------------------------------------------------------------------------
int ParseStringToExclusionArray( const char *pString, char ( *ppStringArray )[64], int nMaxArray )
{
	int nStrings = 0;

	if ( V_strlen( pString ) )
	{
		char buffer[1024];
		V_strncpy( buffer, pString, sizeof( buffer ) );

		char *pBuffer = buffer;
		char *pSpace = V_stristr( pBuffer, " " );
		while ( pSpace && nStrings < nMaxArray - 1 )
		{
			*pSpace = '\0';
			V_strncpy( ppStringArray[nStrings++], pBuffer, 64 );

			pBuffer = pSpace + 1;
			pSpace = V_stristr( pBuffer, " " );
		}

		// Reconstruction note: the binaries use an unbounded strcpy for the last name.
		V_strncpy( ppStringArray[nStrings++], pBuffer, 64 );
	}

	return nStrings;
}

//-----------------------------------------------------------------------------
// Purpose: Parses a space separated list of gib model names into model paths
//-----------------------------------------------------------------------------
int ParseStringToGibArray(
    const char *pString, const char *modelPath, char ( *ppStringArray )[64], int nMaxArray )
{
	int nStrings = 0;

	if ( V_strlen( pString ) )
	{
		char buffer[1024];
		V_strncpy( buffer, pString, sizeof( buffer ) );

		char *pBuffer = buffer;
		char *pSpace = V_stristr( pBuffer, " " );
		while ( pSpace && nStrings < nMaxArray - 1 )
		{
			*pSpace = '\0';
			V_snprintf( ppStringArray[nStrings++], 64, "%s/%s.mdl", modelPath, pBuffer );

			pBuffer = pSpace + 1;
			pSpace = V_stristr( pBuffer, " " );
		}

		V_snprintf( ppStringArray[nStrings++], 64, "%s/%s.mdl", modelPath, pBuffer );
	}

	return nStrings;
}

//-----------------------------------------------------------------------------
// Purpose: Parses a space separated list of "effect chance" pairs
//-----------------------------------------------------------------------------
int ParseStringToEffectArray(
    const char *pString, char ( *ppEffectArray )[64], int *pChanceArray, int nMaxArray )
{
	int nStrings = 0;

	if ( V_strlen( pString ) )
	{
		char buffer[1024];
		V_strncpy( buffer, pString, sizeof( buffer ) );

		char *pBuffer = buffer;
		char *pSpace = V_stristr( pBuffer, " " );
		while ( pSpace && nStrings < nMaxArray - 1 )
		{
			// Effect name
			*pSpace = '\0';
			V_strncpy( ppEffectArray[nStrings], pBuffer, 64 );

			pBuffer = pSpace + 1;
			pSpace = V_stristr( pBuffer, " " );
			if ( !pSpace )
			{
				// Last chance value
				pChanceArray[nStrings++] = V_atoi( pBuffer );
				break;
			}

			// Chance
			*pSpace = '\0';
			pChanceArray[nStrings++] = V_atoi( pBuffer );

			pBuffer = pSpace + 1;
			pSpace = V_stristr( pBuffer, " " );
		}
	}

	return nStrings;
}

//-----------------------------------------------------------------------------
// Purpose: Parses "x y z"
//-----------------------------------------------------------------------------
void ParseStringToVector( const char *pString, Vector &vector )
{
	char buffer[1024];
	V_strncpy( buffer, pString, sizeof( buffer ) );

	char *pBuffer = buffer;
	char *pSpace = V_stristr( pBuffer, " " );
	if ( pSpace )
	{
		*pSpace = '\0';
		vector.x = V_atof( pBuffer );

		pBuffer = pSpace + 1;
		pSpace = V_stristr( pBuffer, " " );
		if ( pSpace )
		{
			*pSpace = '\0';
			vector.y = V_atof( pBuffer );

			pBuffer = pSpace + 1;
			vector.z = V_atof( pBuffer );
		}
	}
}
