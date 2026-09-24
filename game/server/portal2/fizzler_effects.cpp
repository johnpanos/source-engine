//========= Portal 2 reconstruction ============================================//
//
// Purpose: Effect entity attached to objects being fizzled by a cleanser field
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "fizzler_effects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar fizzler_fx_model_target_scale( "fizzler_fx_model_target_scale", "0.001f", FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT );


LINK_ENTITY_TO_CLASS( env_fizzler_effects, FizzlerEffects );

BEGIN_DATADESC( FizzlerEffects )
	DEFINE_THINKFUNC( FizzleThink ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( FizzlerEffects, DT_FizzlerEffects )
END_SEND_TABLE()


FizzlerEffects *FizzlerEffects::Create( CBaseEntity *pAttachTo )
{
	FizzlerEffects *pFizzlerEffects = static_cast< FizzlerEffects * >( CreateEntityByName( "env_fizzler_effects" ) );
	if ( pFizzlerEffects == NULL )
		return NULL;

	pFizzlerEffects->AttachToEntity( pAttachTo );
	pFizzlerEffects->Spawn();

	pAttachTo->DispatchResponse( "TLK_DISSOLVESCREAM" );

	CBaseAnimating *pDissolvingAnimating = pAttachTo->GetBaseAnimating();
	if ( pDissolvingAnimating )
	{
		pDissolvingAnimating->AddFlag( FL_DISSOLVING );
		pDissolvingAnimating->SetEffectEntity( pFizzlerEffects );

		// Play the fizzle sequence and shrink the model over its duration
		pDissolvingAnimating->ResetSequence( 1 );
		pDissolvingAnimating->SetModelScale(
		    fizzler_fx_model_target_scale.GetFloat(), pDissolvingAnimating->SequenceDuration() );
	}

	return pFizzlerEffects;
}


void FizzlerEffects::Spawn( void )
{
	BaseClass::Spawn();

	SetThink( &FizzlerEffects::FizzleThink );
	SetNextThink( gpGlobals->curtime );
}


void FizzlerEffects::AttachToEntity( CBaseEntity *pAttachTo )
{
	SetParent( pAttachTo );
	SetLocalOrigin( vec3_origin );
	SetLocalAngles( vec3_angle );
}


void FizzlerEffects::FizzleThink( void )
{
	CBaseAnimating *pDissolving = GetMoveParent() ? GetMoveParent()->GetBaseAnimating() : NULL;
	if ( pDissolving )
	{
		pDissolving->SetCollisionGroup( COLLISION_GROUP_DISSOLVING );
		pDissolving->StudioFrameAdvance();

		if ( !pDissolving->IsSequenceFinished() )
		{
			SetNextThink( gpGlobals->curtime );
			return;
		}

		UTIL_Remove( pDissolving );
	}

	UTIL_Remove( this );
}
