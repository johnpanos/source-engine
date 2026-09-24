//========= Portal 2 reconstruction ============================================//
//
// Purpose: Non-firing base for player wearable weapons.
//
// The retail server RTTI identifies CBasePortalCombatWeapon as this class's
// parent. The wearable behavior is kept minimal while restoring the build.
// Not original Valve source; the provenance warning applies.
//
//=============================================================================//

#include "cbase.h"
#include "weapon_item_base_shared.h"

#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponItemBase, DT_WeaponItemBase )

// clang-format off
BEGIN_NETWORK_TABLE( CWeaponItemBase, DT_WeaponItemBase )
END_NETWORK_TABLE()
// clang-format on

void CWeaponItemBase::PrimaryAttack()
{
}

void CWeaponItemBase::SecondaryAttack()
{
}
