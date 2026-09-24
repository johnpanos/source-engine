//========= Copyright © 1996-2009, Valve Corporation, All rights reserved. ============//
//
// Purpose: Shared hitbox damage particle and gib effects.
//
//=============================================================================//

#include "cbase.h"
#include "hitbox_damaged_entity_shared.h"
#include "particle_parse.h"

#ifdef CLIENT_DLL
#include "c_baseanimating.h"
#include "c_gib.h"
#else
#include "baseanimating.h"
#include "gib.h"
#include "vphysics_interface.h"
#endif

#include "tier0/memdbgon.h"

namespace
{
bool GetDamageInfo(
    DamagedEntityType entityType, int damageEffectIndex, const DamageInfo **ppDamageInfo )
{
	if ( ppDamageInfo == NULL || damageEffectIndex < 0 )
		return false;

	const DamageInfoVector &damageInfo = g_DamageDatabase.GetDamageInfoVector( entityType );
	if ( damageEffectIndex >= damageInfo.Count() )
		return false;

	*ppDamageInfo = &damageInfo[damageEffectIndex];
	return true;
}

Vector GetDamageEffectOrigin( CAnimatingForDamageEffects *pEntity )
{
	Vector origin;
	if ( pEntity != NULL && pEntity->LookupAttachment( "placementOrigin" ) > 0 &&
	     pEntity->GetAttachment( pEntity->LookupAttachment( "placementOrigin" ), origin ) )
	{
		return origin;
	}

	return pEntity != NULL ? pEntity->GetAbsOrigin() : vec3_origin;
}
}

void EmitParticles( CAnimatingForDamageEffects *pOwnerEntity, DamagedEntityType entityType,
    int damageEffectIndex, int particleEffectIndex )
{
	const DamageInfo *pDamageInfo = NULL;
	if ( pOwnerEntity == NULL || !GetDamageInfo( entityType, damageEffectIndex, &pDamageInfo ) ||
	     particleEffectIndex < 0 || particleEffectIndex >= pDamageInfo->particleEffectCount ||
	     particleEffectIndex >= ARRAYSIZE( pDamageInfo->particleEffects ) )
	{
		return;
	}

	const char *pszParticle = pDamageInfo->particleEffects[particleEffectIndex];
	if ( pszParticle[0] == '\0' )
		return;

	if ( pDamageInfo->particleAttachment[0] != '\0' )
	{
		const int attachment = pOwnerEntity->LookupAttachment( pDamageInfo->particleAttachment );
		if ( attachment > 0 )
		{
			DispatchParticleEffect( pszParticle, PATTACH_POINT_FOLLOW, pOwnerEntity, attachment );
		}
		return;
	}

	if ( pDamageInfo->particleAttachmentHitbox == -1 )
	{
		DispatchParticleEffect( pszParticle, PATTACH_ABSORIGIN_FOLLOW, pOwnerEntity );
		return;
	}

	Vector hitboxOrigin( static_cast<float>( pDamageInfo->particleAttachmentHitbox ), 0.0f, 0.0f );
	DispatchParticleEffect( pszParticle, GetDamageEffectOrigin( pOwnerEntity ), hitboxOrigin,
	    pOwnerEntity->GetAbsAngles(), pOwnerEntity );
}

void EmitGibs(
    CAnimatingForDamageEffects *pParentEntity, DamagedEntityType entityType, int damageEffectIndex )
{
	const DamageInfo *pDamageInfo = NULL;
	if ( pParentEntity == NULL || !GetDamageInfo( entityType, damageEffectIndex, &pDamageInfo ) ||
	     pDamageInfo->gibCount <= 0 )
	{
		return;
	}

	const int gibCount = MIN( pDamageInfo->gibCount, ARRAYSIZE( pDamageInfo->gibModels ) );
	const Vector burstCenter = GetDamageEffectOrigin( pParentEntity );
	Vector parentVelocity = pParentEntity->GetAbsVelocity();
#ifndef CLIENT_DLL
	IPhysicsObject *pParentPhysics = pParentEntity->VPhysicsGetObject();
	if ( pParentPhysics != NULL )
	{
		pParentPhysics->GetVelocityAtPoint( burstCenter, &parentVelocity );
	}
#endif

	for ( int gibIndex = 0; gibIndex < gibCount; ++gibIndex )
	{
		const char *pszModel = pDamageInfo->gibModels[gibIndex];
		if ( pszModel[0] == '\0' )
			continue;

#ifdef CLIENT_DLL
		Vector separation = GetDamageEffectOrigin( pParentEntity ) - burstCenter;
		if ( separation.IsZero() )
		{
			separation = pParentEntity->WorldSpaceCenter() - burstCenter;
		}
		if ( separation.IsZero() )
		{
			separation = Vector( 0.0f, 0.0f, 1.0f );
		}
		VectorNormalize( separation );
		separation *= pDamageInfo->gibSeparationSpeed;
		separation += parentVelocity;
		C_Gib::CreateClientsideGib( pszModel, burstCenter, separation,
		    AngularImpulse( 0.0f, 0.0f, 0.0f ), pDamageInfo->gibLifeTime );
#else
		CGib *pGib = static_cast<CGib *>( CreateEntityByName( "gib" ) );
		if ( pGib == NULL )
			continue;

		pGib->Spawn( pszModel, pDamageInfo->gibLifeTime );
		pGib->m_nBody = gibIndex;
		pGib->InitGib( pParentEntity, 0.0f, 0.0f );
		pGib->SetAbsOrigin( burstCenter );
		pGib->SetOwnerEntity( pParentEntity );

		if ( pDamageInfo->gibSeparationSpeed != 0.0f )
		{
			Vector separation = pGib->WorldSpaceCenter() - burstCenter;
			if ( separation.IsZero() )
			{
				separation = pParentEntity->WorldSpaceCenter() - burstCenter;
			}
			if ( separation.IsZero() )
			{
				separation = Vector( 0.0f, 0.0f, 1.0f );
			}
			VectorNormalize( separation );
			separation *= pDamageInfo->gibSeparationSpeed;
			pGib->ApplyAbsVelocityImpulse( separation );
		}
#endif
	}
}
