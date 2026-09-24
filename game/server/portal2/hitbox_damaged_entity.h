//========= Portal 2 reconstruction ============================================//
//
// Purpose: Template mixin for entities that show hitbox damage effects
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//
#ifndef HITBOX_DAMAGED_ENTITY_H
#define HITBOX_DAMAGED_ENTITY_H

#ifdef _WIN32
#pragma once
#endif

#include "damage_database.h"
#include "ipredictionsystem.h"

extern ConVar max_hitbox_damage_effects_per_entity;
extern ConVar hitbox_damage_enabled;

// Defined in hitbox_damaged_entity_shared.cpp
void EmitParticles( CBaseAnimating *pOwnerEntity, DamagedEntityType entityType, int damageEffectIndex, int particleEffectIndex );
void EmitGibs( CBaseAnimating *pParentEntity, DamagedEntityType entityType, int damageEffectIndex );


//-----------------------------------------------------------------------------
// Adds hitbox driven damage effects (wounds, model swaps, particles) to T.
//-----------------------------------------------------------------------------
template< class T >
class HitBoxDamagedEntity : public T
{
	typedef T BaseClass;

#include "hitbox_damaged_entity_shared.h"

private:
	// Indices (into the damage database for m_DamagedEntityType) of every
	// damage effect already applied to this entity
	CUtlVector< int > m_AppliedDamageEffects;
	DamagedEntityType m_DamagedEntityType;

public:

	HitBoxDamagedEntity( DamagedEntityType damagedEntityType )
		: m_DamagedEntityType( damagedEntityType )
	{
	}

	virtual ~HitBoxDamagedEntity()
	{
	}

	virtual void Spawn()
	{
		BaseClass::Spawn();

		// Freshly spawned entities have no wounds
		m_AppliedDamageEffects.RemoveAll();
	}

	virtual void TraceAttack( const CTakeDamageInfo &info, const Vector &direction, trace_t *pTrace )
	{
		BaseClass::TraceAttack( info, direction, pTrace );

		if ( hitbox_damage_enabled.GetBool() )
		{
			TakeDamage( info, direction, pTrace );
		}
	}

	bool ApplyDamageEffect( int damageEffectIndex )
	{
		if ( CanApplyDamageEffect( damageEffectIndex ) )
		{
			const DamageInfoVector &damageInfo = g_DamageDatabase.GetDamageInfoVector( m_DamagedEntityType );

			// Pick one of the effect's particle systems by weighted chance
			int particleEffectIndex = -1;
			int nCurChance = RandomInt( 0, damageInfo[damageEffectIndex].particleEffectTotalChance );
			for ( int i = 0; i < damageInfo[damageEffectIndex].particleEffectCount; ++i )
			{
				nCurChance -= damageInfo[damageEffectIndex].particleEffectsChance[i];
				if ( nCurChance <= 0 )
				{
					particleEffectIndex = i;
					break;
				}
			}

			ApplyDamageEffectShared( damageEffectIndex, particleEffectIndex );

			// Tell the clients to apply the same effect
			CReliableBroadcastRecipientFilter filter;
			UserMessageBegin( filter, "ApplyHitBoxDamageEffect" );
				WRITE_EHANDLE( this );
				WRITE_BYTE( damageEffectIndex );
				WRITE_BYTE( particleEffectIndex );
			MessageEnd();

			m_AppliedDamageEffects.AddToTail( damageEffectIndex );
			return true;
		}

		return false;
	}

	DamagedEntityType GetDamagedEntityType() const { return m_DamagedEntityType; }

protected:

	virtual void TakeDamage( const CTakeDamageInfo &info, const Vector &direction, trace_t *pTrace )
	{
		// Send the effect message to every client, including the attacker
		CDisablePredictionFiltering disabler;

		// Find all of the effects that respond to this damage on this hitbox
		int potentialDamageEffects[16];
		const int potentialEffectCount = g_DamageDatabase.FindPotentialDamageEffects( potentialDamageEffects, ARRAYSIZE( potentialDamageEffects ),
																					  m_DamagedEntityType, info.GetDamageType(), pTrace->hitbox );
		if ( potentialEffectCount > 0 )
		{
			// Start at a random effect and try the others in order until one applies
			const int randomPotentialEffectIndex = RandomInt( 0, potentialEffectCount - 1 );

			int appliedEffectIndex = randomPotentialEffectIndex;
			if ( !ApplyDamageEffect( potentialDamageEffects[appliedEffectIndex] ) )
			{
				for ( int i = ( appliedEffectIndex + 1 ) % potentialEffectCount; i != randomPotentialEffectIndex; i = ( i + 1 ) % potentialEffectCount )
				{
					if ( ApplyDamageEffect( potentialDamageEffects[i] ) )
						break;
				}
			}
		}
	}

	virtual bool CanApplyDamageEffect( int damageEffectIndex ) const
	{
		// Limit the number of wounds on one entity
		if ( m_AppliedDamageEffects.Count() >= max_hitbox_damage_effects_per_entity.GetInt() )
			return false;

		const DamageInfoVector &damageInfo = g_DamageDatabase.GetDamageInfoVector( m_DamagedEntityType );
		if ( damageEffectIndex < 0 || damageEffectIndex >= damageInfo.Count() )
			return false;

		bool isValidWound = true;
		for ( int appliedDamageIndex = 0; isValidWound && appliedDamageIndex < m_AppliedDamageEffects.Count(); ++appliedDamageIndex )
		{
			const DamageInfo &damageEffect = damageInfo[m_AppliedDamageEffects[appliedDamageIndex]];

			// Already applied
			if ( m_AppliedDamageEffects[appliedDamageIndex] == damageEffectIndex )
				return false;

			// Excluded by an applied effect
			for ( int exclusionIndex = 0; exclusionIndex < damageEffect.exclusionCount; ++exclusionIndex )
			{
				if ( damageEffect.exclusionListIndices[exclusionIndex] == damageEffectIndex )
				{
					isValidWound = false;
					break;
				}
			}
		}

		return isValidWound;
	}
};

#endif // HITBOX_DAMAGED_ENTITY_H
