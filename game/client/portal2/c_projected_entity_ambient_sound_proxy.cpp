//========= Portal 2 reconstruction ============================================//
//
// Purpose: Register the projected sound proxy's client network class.
//
// Retail Portal 2 names the class and receive table. The server table has no
// extra fields; C_BaseEntity receives the parent transform. Not original Valve
// source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "c_projected_entity_ambient_sound_proxy.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(
    projected_entity_ambient_sound_proxy, ClientProjectedEntityAmbientSoundProxy );

IMPLEMENT_CLIENTCLASS_DT( ClientProjectedEntityAmbientSoundProxy,
    DT_ProjectedEntityAmbientSoundProxy, ProjectedEntityAmbientSoundProxy )
END_RECV_TABLE()
