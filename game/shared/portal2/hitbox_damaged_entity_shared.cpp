//========= Portal 2 reconstruction ============================================//
//
// Purpose: Particle and gib emission for hitbox damage effects
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "damage_database.h"
#include "particle_parse.h"
#include "animation.h"
#include "studio.h"
#include "bone_setup.h"

#ifdef GAME_DLL
#include "hitbox_damaged_entity.h"
#include "gib.h"
#else
#include "c_hitbox_damaged_entity.h"
#include "c_gib.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void EmitParticles( CBaseAnimating *pOwnerEntity, DamagedEntityType entityType,
    int damageEffectIndex, int particleEffectIndex )
{
	const DamageInfoVector &damageInfoVector = g_DamageDatabase.GetDamageInfoVector( entityType );
	if ( damageEffectIndex < 0 || damageEffectIndex >= damageInfoVector.Count() )
		return;

	const DamageInfo &damageInfo = damageInfoVector[damageEffectIndex];
	if ( particleEffectIndex < 0 || particleEffectIndex >= damageInfo.particleEffectCount )
		return;

	if ( damageInfo.particleAttachment[0] != '\0' )
	{
		int attachmentIndex = pOwnerEntity->LookupAttachment( damageInfo.particleAttachment );
		if ( attachmentIndex > 0 )
		{
			DispatchParticleEffect( damageInfo.particleEffects[particleEffectIndex],
			    PATTACH_POINT_FOLLOW, pOwnerEntity, attachmentIndex );
		}
	}
	else if ( damageInfo.particleAttachmentHitbox != -1 )
	{
		// Reconstruction note: both binaries pass the hitbox index in the x component
		// of the effect's end position.
		DispatchParticleEffect( damageInfo.particleEffects[particleEffectIndex], vec3_origin,
		    Vector( damageInfo.particleAttachmentHitbox, 0, 0 ), vec3_angle, pOwnerEntity );
	}
	else
	{
		DispatchParticleEffect( damageInfo.particleEffects[particleEffectIndex],
		    PATTACH_ABSORIGIN_FOLLOW, pOwnerEntity );
	}
}

void EmitGibs( CBaseAnimating *pParentEntity, DamagedEntityType entityType, int damageEffectIndex )
{
	if ( !pParentEntity )
		return;

	const DamageInfoVector &damageInfoVector = g_DamageDatabase.GetDamageInfoVector( entityType );
	if ( damageEffectIndex < 0 || damageEffectIndex >= damageInfoVector.Count() )
		return;

	const DamageInfo &damageInfo = damageInfoVector[damageEffectIndex];
	if ( damageInfo.gibCount <= 0 )
		return;

	// Gib models are authored relative to the parent's placement origin
	matrix3x4_t localToWorld;
	Vector parentOrigin = vec3_origin;
	int parentAttachment = pParentEntity->LookupAttachment( "placementOrigin" );
	if ( parentAttachment > 0 )
	{
		pParentEntity->GetAttachmentLocal( parentAttachment, localToWorld );
		MatrixGetColumn( localToWorld, 3, parentOrigin );
	}
	else
	{
		SetIdentityMatrix( localToWorld );
	}

	// Gibs fly away from the burst center
	const matrix3x4_t &parentTransform = pParentEntity->EntityToWorldTransform();
	Vector burstCenter = pParentEntity->GetAbsOrigin();
	const int parentBurstCenterAttachment = pParentEntity->LookupAttachment( "burstCenter" );
	if ( parentBurstCenterAttachment > 0 )
	{
		matrix3x4_t burstCenterTransform;
		pParentEntity->GetAttachment( parentBurstCenterAttachment, burstCenterTransform );
		MatrixGetColumn( burstCenterTransform, 3, burstCenter );
	}

	for ( int gibIndex = 0; gibIndex < damageInfo.gibCount; ++gibIndex )
	{
		const char *gibModelName = damageInfo.gibModels[gibIndex];
		const int gibModelIndex = modelinfo->GetModelIndex( gibModelName );
		if ( gibModelIndex <= 0 )
		{
			Warning( "Unable to create non-precached gib model %s\n", gibModelName );
			continue;
		}

		CStudioHdr gibStudioHdr;
		const model_t *gibModel = modelinfo->GetModel( gibModelIndex );
		if ( gibModel )
		{
			gibStudioHdr.Init( modelinfo->GetStudiomodel( gibModel ) );
		}

		Vector position = vec3_origin;
		Vector placementOrigin = parentOrigin;
		QAngle angles = pParentEntity->GetAbsAngles();
		const int placementIndex = Studio_FindAttachment( &gibStudioHdr, "placementOrigin" ) + 1;
		if ( placementIndex > 0 )
		{
			// Offset of the gib's placement origin from the parent's
			GetAttachmentLocalSpace( &gibStudioHdr, placementIndex - 1, localToWorld );
			MatrixGetColumn( localToWorld, 3, placementOrigin );
			placementOrigin -= parentOrigin;
		}

		VectorTransform( -placementOrigin, parentTransform, position );

		// Reconstruction note: the velocity at the gib position is computed but
		// unused by both binaries.
		Vector velocity = pParentEntity->GetAbsVelocity();
		IPhysicsObject *pPhysics = pParentEntity->VPhysicsGetObject();
		if ( pPhysics )
		{
			pPhysics->GetVelocityAtPoint( position, &velocity );
		}

#ifdef GAME_DLL
		CGib *pGib = CREATE_ENTITY( CGib, "gib" );
		pGib->Spawn( gibModelName, 25.0f );
		pGib->m_nBody = gibIndex;
		pGib->InitGib( pParentEntity, 0.0f, 0.0f );
		pGib->m_lifeTime = damageInfo.gibLifeTime;
		pGib->SetOwnerEntity( pParentEntity );
#else
		C_Gib *pGib = C_Gib::CreateClientsideGib(
		    gibModelName, position, vec3_origin, vec3_origin, damageInfo.gibLifeTime );
		if ( !pGib )
			continue;
#endif

		if ( pParentEntity->IsEffectActive( EF_NOSHADOW ) )
		{
			pGib->AddEffects( EF_NOSHADOW );
		}

		if ( damageInfo.gibSeparationSpeed != 0.0f )
		{
			Vector separationDirection = position - burstCenter;

			// Gibs placed exactly at the burst center use their own center instead
			if ( separationDirection == vec3_origin )
			{
				separationDirection = pGib->WorldSpaceCenter() - burstCenter;
			}

			separationDirection.NormalizeInPlace();
			pGib->ApplyAbsVelocityImpulse( damageInfo.gibSeparationSpeed * separationDirection );
		}
	}
}
