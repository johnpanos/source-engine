//========= Portal 2 reconstruction ============================================//
//
// Purpose: projected_entity_ambient_sound_proxy: an always-transmitted entity
//          parented to a projected entity (light bridge, tractor beam) so the
//          projected entity's ambient loop keeps playing when the projected
//          entity itself leaves the PVS.
//
// Written for the Portal 2 port from consumers, FGD data and related Valve code
// (no original source, pseudocode or debug data exists). Behavior follows the
// retail Portal 2 server's ProjectedEntityAmbientSoundProxy. Not original
// Valve source; the repository's provenance and distribution warning applies.
//
// Networked: DT_ProjectedEntityAmbientSoundProxy has no fields of its own
// (baseclass DT_BaseEntity; the parent arrives through DT_BaseEntity's
// moveparent). Server class name "ProjectedEntityAmbientSoundProxy"; the retail
// client class is ClientProjectedEntityAmbientSoundProxy.
//
//=============================================================================//

#ifndef PROJECTED_ENTITY_AMBIENT_SOUND_PROXY_H
#define PROJECTED_ENTITY_AMBIENT_SOUND_PROXY_H
#ifdef _WIN32
#pragma once
#endif

class ProjectedEntityAmbientSoundProxy : public CBaseEntity
{
public:
	DECLARE_CLASS( ProjectedEntityAmbientSoundProxy, CBaseEntity );
	DECLARE_SERVERCLASS();

	// Creates a proxy attached to pAttachTo
	static ProjectedEntityAmbientSoundProxy *Create( CBaseEntity *pAttachTo );

	// Parents the proxy to pAttachTo at its origin
	void			AttachToEntity( CBaseEntity *pAttachTo );

	virtual int		UpdateTransmitState( void );
};

#endif // PROJECTED_ENTITY_AMBIENT_SOUND_PROXY_H
