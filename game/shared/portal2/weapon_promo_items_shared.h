//========= Portal 2 reconstruction ============================================//
//
// Purpose: Cosmetic co-op promotional wearable entity types.
//
//=============================================================================//

#ifndef WEAPON_PROMO_ITEMS_SHARED_H
#define WEAPON_PROMO_ITEMS_SHARED_H

#include "weapon_item_base_shared.h"

#if defined( CLIENT_DLL )
#define CWeaponPromoHelmetBall C_WeaponPromoHelmetBall
#define CWeaponPromoHelmetEgg C_WeaponPromoHelmetEgg
#define CWeaponPromoAntennaBall C_WeaponPromoAntennaBall
#define CWeaponPromoAntennaEgg C_WeaponPromoAntennaEgg
#endif

class CWeaponPromoHelmetBall : public CWeaponItemBase
{
	DECLARE_CLASS( CWeaponPromoHelmetBall, CWeaponItemBase );
	DECLARE_NETWORKCLASS();
};

class CWeaponPromoHelmetEgg : public CWeaponItemBase
{
	DECLARE_CLASS( CWeaponPromoHelmetEgg, CWeaponItemBase );
	DECLARE_NETWORKCLASS();
};

class CWeaponPromoAntennaBall : public CWeaponItemBase
{
	DECLARE_CLASS( CWeaponPromoAntennaBall, CWeaponItemBase );
	DECLARE_NETWORKCLASS();
};

class CWeaponPromoAntennaEgg : public CWeaponItemBase
{
	DECLARE_CLASS( CWeaponPromoAntennaEgg, CWeaponItemBase );
	DECLARE_NETWORKCLASS();
};

#endif // WEAPON_PROMO_ITEMS_SHARED_H
