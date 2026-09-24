//========= Copyright 1996-2009, Valve Corporation, All rights reserved. =======//
//
// Purpose: Load and query the Portal 2 hitbox damage effect database.
//
//=============================================================================//

#include "cbase.h"
#include "damage_database.h"
#include "gamerules.h"
#include "particle_parse.h"
#include "weapon_parse.h"
#include "tier1/strtools.h"

#include <stdlib.h>
#include <string.h>

#include "tier0/memdbgon.h"

ConVar max_hitbox_damage_effects_per_entity( "max_hitbox_damage_effects_per_entity", "4",
    FCVAR_NONE, "Maximum hitbox damage effects selected for an entity." );
ConVar hitbox_damage_enabled(
    "hitbox_damage_enabled", "1", FCVAR_NONE, "Enable data-driven hitbox damage effects." );

DamageDatabase g_DamageDatabase;

namespace
{
const int kMaxHitboxes = 8;
const int kMaxExclusions = 16;
const int kMaxGibs = 16;
const int kMaxParticleEffects = 8;

void CopyString( char *pDest, int destSize, const char *pSource )
{
	if ( destSize > 0 )
	{
		V_strncpy( pDest, pSource ? pSource : "", destSize );
	}
}

KeyValues *FindValue( KeyValues *pParent, const char *pName )
{
	return pParent ? pParent->FindKey( pName, false ) : NULL;
}

char *NextToken( char *&pCursor, const char *pSeparators )
{
	if ( !pCursor )
	{
		return NULL;
	}
	while ( *pCursor && strchr( pSeparators, *pCursor ) )
	{
		++pCursor;
	}
	if ( !*pCursor )
	{
		pCursor = NULL;
		return NULL;
	}
	char *pToken = pCursor;
	while ( *pCursor && !strchr( pSeparators, *pCursor ) )
	{
		++pCursor;
	}
	if ( *pCursor )
	{
		*pCursor++ = '\0';
	}
	else
	{
		pCursor = NULL;
	}
	return pToken;
}

const char *GetString( KeyValues *pParent, const char *pName, const char *pDefault = "" )
{
	KeyValues *pValue = FindValue( pParent, pName );
	return pValue ? pValue->GetString( (const char *)NULL, pDefault ) : pDefault;
}

int ParseIntList( const char *pText, int *pValues, int maxValues )
{
	if ( !pText || !pValues || maxValues <= 0 )
	{
		return 0;
	}

	char buffer[1024];
	CopyString( buffer, sizeof( buffer ), pText );
	int count = 0;
	char *pCursor = buffer;
	for ( char *pToken = NextToken( pCursor, ",;| \t\r\n" ); pToken && count < maxValues;
	    pToken = NextToken( pCursor, ",;| \t\r\n" ) )
	{
		pValues[count++] = V_atoi( pToken );
	}
	return count;
}

int ParseStringList(
    const char *pText, char ( *pValues )[64], int maxValues, const char *pModelPath = NULL )
{
	if ( !pText || !pValues || maxValues <= 0 )
	{
		return 0;
	}

	char buffer[1024];
	CopyString( buffer, sizeof( buffer ), pText );
	int count = 0;
	char *pCursor = buffer;
	for ( char *pToken = NextToken( pCursor, ",;|\r\n" ); pToken && count < maxValues;
	    pToken = NextToken( pCursor, ",;|\r\n" ) )
	{
		while ( *pToken == ' ' || *pToken == '\t' )
		{
			++pToken;
		}
		if ( pModelPath && *pModelPath && pToken[0] != '/' && !( pToken[0] && pToken[1] == ':' ) )
		{
			V_snprintf( pValues[count], sizeof( pValues[count] ), "%s%s", pModelPath, pToken );
		}
		else
		{
			CopyString( pValues[count], sizeof( pValues[count] ), pToken );
		}
		++count;
	}
	return count;
}

int ParseDamageType( const char *pText )
{
	static const struct DamageName
	{
		const char *name;
		int bit;
	} damageNames[] = {
	    { "crush", DMG_CRUSH },
	    { "slash", DMG_SLASH },
	    { "burn", DMG_BURN },
	    { "blast", DMG_BLAST },
	    { "club", DMG_CLUB },
	    { "shock", DMG_SHOCK },
	    { "sonic", DMG_SONIC },
	    { "poison", DMG_POISON },
	    { "dissolve", DMG_DISSOLVE },
	    { "blast_surface", DMG_BLAST_SURFACE },
	    { "bullet", DMG_BULLET },
	};
	int result = 0;
	char buffer[1024];
	CopyString( buffer, sizeof( buffer ), pText );
	for ( char *p = buffer; *p; ++p )
	{
		if ( *p == ',' || *p == ';' || *p == '|' )
		{
			*p = ' ';
		}
	}
	char *pCursor = buffer;
	for ( char *pToken = NextToken( pCursor, " \t\r\n" ); pToken;
	    pToken = NextToken( pCursor, " \t\r\n" ) )
	{
		for ( int i = 0; i < ARRAYSIZE( damageNames ); ++i )
		{
			if ( !V_stricmp( pToken, damageNames[i].name ) )
			{
				result |= damageNames[i].bit;
				break;
			}
		}
	}
	return result;
}

void ParseParticleEffects( const char *pText, DamageInfo &info )
{
	char buffer[1024];
	CopyString( buffer, sizeof( buffer ), pText );
	char *pCursor = buffer;
	for ( char *pToken = NextToken( pCursor, ";|" );
	    pToken && info.particleEffectCount < kMaxParticleEffects;
	    pToken = NextToken( pCursor, ";|" ) )
	{
		while ( *pToken == ' ' || *pToken == '\t' )
		{
			++pToken;
		}
		char *pSeparator = strchr( pToken, ':' );
		if ( !pSeparator )
		{
			pSeparator = strchr( pToken, ',' );
		}
		int index = info.particleEffectCount++;
		if ( pSeparator )
		{
			*pSeparator++ = '\0';
			info.particleEffectsChance[index] = MAX( 0, V_atoi( pSeparator ) );
		}
		else
		{
			info.particleEffectsChance[index] = 1;
		}
		CopyString( info.particleEffects[index], sizeof( info.particleEffects[index] ), pToken );
		info.particleEffectTotalChance += info.particleEffectsChance[index];
	}
}

void ParseDamageInfo( KeyValues *pKey, const char *pModelPath, DamageInfo &info )
{
	memset( &info, 0, sizeof( info ) );
	for ( int i = 0; i < kMaxExclusions; ++i )
	{
		info.exclusionListIndices[i] = -1;
	}
	info.particleAttachmentHitbox = -1;
	info.hitBoxCount =
	    ParseIntList( GetString( pKey, "hitboxes" ), info.hitBoxIndices, kMaxHitboxes );
	info.exclusionCount = ParseStringList(
	    GetString( pKey, "exclusions" ), info.exclusionListStrings, kMaxExclusions );
	CopyString( info.deathThroesAnimation, sizeof( info.deathThroesAnimation ),
	    GetString( pKey, "deaththroes" ) );
	info.damageType = ParseDamageType( GetString( pKey, "damagetype" ) );
	info.gibCount =
	    ParseStringList( GetString( pKey, "gibs" ), info.gibModels, kMaxGibs, pModelPath );
	KeyValues *pValue = FindValue( pKey, "gibseparationspeed" );
	info.gibSeparationSpeed = pValue ? pValue->GetFloat( (const char *)NULL, 0.0f ) : 0.0f;
	pValue = FindValue( pKey, "giblifetime" );
	info.gibLifeTime = pValue ? pValue->GetFloat( (const char *)NULL, 0.0f ) : 0.0f;
	CopyString( info.particleAttachment, sizeof( info.particleAttachment ),
	    GetString( pKey, "particleattachment" ) );
	pValue = FindValue( pKey, "particleattachmenthitbox" );
	info.particleAttachmentHitbox = pValue ? pValue->GetInt( (const char *)NULL, -1 ) : -1;
	ParseParticleEffects( GetString( pKey, "particleeffects" ), info );
	pValue = FindValue( pKey, "fatal" );
	info.isFatalDamage = pValue ? pValue->GetBool( NULL, false ) : false;
	CopyString( info.swapModelName, sizeof( info.swapModelName ), GetString( pKey, "swapmodel" ) );
	CopyString( info.damageEffectName, sizeof( info.damageEffectName ), pKey->GetName() );
}

bool ReadDamageCutoutDataFromFile( DamageInfoVector &damageInfoArray, IFileSystem *pFileSystem,
    const char *pFilename, const unsigned char *pICEKey )
{
	if ( damageInfoArray.Count() > 0 )
	{
		return true;
	}

	KeyValues *pRoot = ReadEncryptedKVFile( pFileSystem, pFilename, pICEKey );
	if ( !pRoot )
	{
		return false;
	}

	const char *pModelPath = GetString( pRoot, "modelpath", "" );
	for ( KeyValues *pKey = pRoot->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey() )
	{
		DamageInfo info;
		ParseDamageInfo( pKey, pModelPath, info );
		damageInfoArray.AddToTail( info );
	}

	for ( int i = 0; i < damageInfoArray.Count(); ++i )
	{
		DamageInfo &info = damageInfoArray[i];
		for ( int j = 0; j < info.exclusionCount; ++j )
		{
			for ( int k = 0; k < damageInfoArray.Count(); ++k )
			{
				if ( !V_stricmp(
				         info.exclusionListStrings[j], damageInfoArray[k].damageEffectName ) )
				{
					info.exclusionListIndices[j] = k;
					break;
				}
			}
			if ( info.exclusionListIndices[j] < 0 )
			{
				Warning( "Damage effect '%s' excludes unknown effect '%s'.\n",
				    info.damageEffectName, info.exclusionListStrings[j] );
			}
		}
	}

	pRoot->deleteThis();
	return true;
}
}

DamageDatabase::DamageDatabase() : CAutoGameSystem( "DamageDatabase" )
{
}

DamageDatabase::~DamageDatabase()
{
}

void DamageDatabase::LevelInitPreEntity()
{
	const unsigned char *pICEKey = g_pGameRules ? g_pGameRules->GetEncryptionKey() : NULL;
	ReadDamageCutoutDataFromFile(
	    m_DamageInfo[DAMAGED_TURRET], filesystem, "scripts/damage_cutouts", pICEKey );
	PrecacheAssets();
}

const DamageInfoVector &DamageDatabase::GetDamageInfoVector( DamagedEntityType entityType ) const
{
	static const DamageInfoVector empty;
	if ( entityType < 0 || entityType >= DAMAGED_ENTITY_TYPE_COUNT )
	{
		return empty;
	}
	return m_DamageInfo[entityType];
}

int DamageDatabase::FindPotentialDamageEffects( int *pPotentialDamageEffects, int maxDamageEffects,
    DamagedEntityType entityType, int damageType, int hitbox ) const
{
	if ( !pPotentialDamageEffects || maxDamageEffects <= 0 || entityType < 0 ||
	     entityType >= DAMAGED_ENTITY_TYPE_COUNT || !hitbox_damage_enabled.GetBool() )
	{
		return 0;
	}

	const DamageInfoVector &damageInfo = m_DamageInfo[entityType];
	int count = 0;
	for ( int damageIndex = 0; damageIndex < damageInfo.Count() && count < maxDamageEffects;
	    ++damageIndex )
	{
		const DamageInfo &info = damageInfo[damageIndex];
		if ( !( info.damageType & damageType ) )
		{
			continue;
		}

		bool matchesHitbox = hitbox == -1;
		for ( int hitboxIndex = 0; !matchesHitbox && hitboxIndex < info.hitBoxCount; ++hitboxIndex )
		{
			matchesHitbox =
			    info.hitBoxIndices[hitboxIndex] < 0 || info.hitBoxIndices[hitboxIndex] == hitbox;
		}
		if ( matchesHitbox )
		{
			pPotentialDamageEffects[count++] = damageIndex;
		}
	}
	return count;
}

void DamageDatabase::PrecacheAssets() const
{
	for ( int i = 0; i < DAMAGED_ENTITY_TYPE_COUNT; ++i )
	{
		const DamageInfoVector &damageInfo = m_DamageInfo[i];
		for ( int j = 0; j < damageInfo.Count(); ++j )
		{
			const DamageInfo &info = damageInfo[j];
			if ( info.swapModelName[0] )
			{
				CBaseEntity::PrecacheModel( info.swapModelName );
			}
			for ( int k = 0; k < info.gibCount; ++k )
			{
				if ( info.gibModels[k][0] )
				{
					CBaseEntity::PrecacheModel( info.gibModels[k] );
				}
			}
			for ( int k = 0; k < info.particleEffectCount; ++k )
			{
				if ( info.particleEffects[k][0] )
				{
					PrecacheParticleSystem( info.particleEffects[k] );
				}
			}
		}
	}
}
