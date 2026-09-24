//========= Portal 2 reconstruction ============================================//
//
// Purpose: Non-firing base for player wearable weapons.
//
//=============================================================================//

#ifndef WEAPON_ITEM_BASE_SHARED_H
#define WEAPON_ITEM_BASE_SHARED_H

#include "weapon_portalbasecombatweapon.h"

#if defined( CLIENT_DLL )
#define CWeaponItemBase C_WeaponItemBase
#endif

class CWeaponItemBase : public CBasePortalCombatWeapon
{
	DECLARE_CLASS( CWeaponItemBase, CBasePortalCombatWeapon );
	DECLARE_NETWORKCLASS();

public:
	void PrimaryAttack() OVERRIDE;
	void SecondaryAttack() OVERRIDE;
};

#endif // WEAPON_ITEM_BASE_SHARED_H
