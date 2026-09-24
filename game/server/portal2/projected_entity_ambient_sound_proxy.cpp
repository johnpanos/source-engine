//========= Portal 2 reconstruction ============================================//
//
// Purpose: projected_entity_ambient_sound_proxy: an always-transmitted sound
//          source parented to a projected entity.
//
// Written for the Portal 2 port from consumers, FGD data and related Valve code
// (no original source, pseudocode or debug data exists). Behavior follows the
// retail Portal 2 server's ProjectedEntityAmbientSoundProxy. Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "projected_entity_ambient_sound_proxy.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST( ProjectedEntityAmbientSoundProxy, DT_ProjectedEntityAmbientSoundProxy )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( projected_entity_ambient_sound_proxy, ProjectedEntityAmbientSoundProxy );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ProjectedEntityAmbientSoundProxy *ProjectedEntityAmbientSoundProxy::Create( CBaseEntity *pAttachTo )
{
	ProjectedEntityAmbientSoundProxy *pProxy = static_cast< ProjectedEntityAmbientSoundProxy * >( CreateEntityByName( "projected_entity_ambient_sound_proxy" ) );
	if ( pProxy )
	{
		pProxy->AttachToEntity( pAttachTo );
	}

	return pProxy;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ProjectedEntityAmbientSoundProxy::AttachToEntity( CBaseEntity *pAttachTo )
{
	SetParent( pAttachTo );
	SetLocalOrigin( vec3_origin );
	SetLocalAngles( vec3_angle );
}

//-----------------------------------------------------------------------------
// Purpose: The ambient loop must keep playing when the parent is out of the PVS
//-----------------------------------------------------------------------------
int ProjectedEntityAmbientSoundProxy::UpdateTransmitState( void )
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}
