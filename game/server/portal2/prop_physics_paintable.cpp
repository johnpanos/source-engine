//========= Portal 2 reconstruction ============================================//
//
// Purpose: Physics prop that can be painted and uses paint powers.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "props.h"
#include "player_pickup_paint_power_user.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CPropPhysicsPaintable : public PlayerPickupPaintPowerUser<CPhysicsProp>
{
	DECLARE_CLASS( CPropPhysicsPaintable, PlayerPickupPaintPowerUser<CPhysicsProp> );
};

LINK_ENTITY_TO_CLASS( prop_physics_paintable, CPropPhysicsPaintable );
