//========= Portal 2 reconstruction ============================================//
//
// Purpose: Register co-op promotional wearable entity types.
//
// Class and data-table names come from the installed Portal 2 binaries. The
// class-specific cosmetic behavior is deferred. Not original Valve source;
// the repository's provenance warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_promo_items_shared.h"

#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( weapon_promo_helmet_ball, CWeaponPromoHelmetBall );
LINK_ENTITY_TO_CLASS( weapon_promo_helmet_egg, CWeaponPromoHelmetEgg );
LINK_ENTITY_TO_CLASS( weapon_promo_antenna_ball, CWeaponPromoAntennaBall );
LINK_ENTITY_TO_CLASS( weapon_promo_antenna_egg, CWeaponPromoAntennaEgg );

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPromoHelmetBall, DT_WeaponPromoHelmetBall )
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPromoHelmetEgg, DT_WeaponPromoHelmetEgg )
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPromoAntennaBall, DT_WeaponPromoAntennaBall )
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPromoAntennaEgg, DT_WeaponPromoAntennaEgg )

// clang-format off
BEGIN_NETWORK_TABLE( CWeaponPromoHelmetBall, DT_WeaponPromoHelmetBall )
END_NETWORK_TABLE()

BEGIN_NETWORK_TABLE( CWeaponPromoHelmetEgg, DT_WeaponPromoHelmetEgg )
END_NETWORK_TABLE()

BEGIN_NETWORK_TABLE( CWeaponPromoAntennaBall, DT_WeaponPromoAntennaBall )
END_NETWORK_TABLE()

BEGIN_NETWORK_TABLE( CWeaponPromoAntennaEgg, DT_WeaponPromoAntennaEgg )
END_NETWORK_TABLE()
// clang-format on
