//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client identity for the always-transmitted projected sound proxy.
//
// Retail Portal 2 names ClientProjectedEntityAmbientSoundProxy and
// DT_ProjectedEntityAmbientSoundProxy. The table adds no fields beyond
// DT_BaseEntity; the parent is received through the base class. Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef C_PROJECTED_ENTITY_AMBIENT_SOUND_PROXY_H
#define C_PROJECTED_ENTITY_AMBIENT_SOUND_PROXY_H

#include "c_baseentity.h"

class ClientProjectedEntityAmbientSoundProxy : public C_BaseEntity
{
public:
	DECLARE_CLASS( ClientProjectedEntityAmbientSoundProxy, C_BaseEntity );
	DECLARE_CLIENTCLASS();
};

#endif // C_PROJECTED_ENTITY_AMBIENT_SOUND_PROXY_H
