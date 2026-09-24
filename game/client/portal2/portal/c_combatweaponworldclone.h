//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client-side clone of a combat weapon's world model.
//
// The Portal 2 source for this header is not available. The retail Linux
// client's RTTI names the class C_CombatWeaponClone with the single base
// C_BaseAnimating. The retained Portal 2 sources only test for this type
// (C_PortalGhostRenderable treats a ghosted clone like a combat weapon); none
// of them create one, so this port declares the type with its weapon owner and
// no additional behavior.
//
//=============================================================================//

#ifndef C_COMBATWEAPONWORLDCLONE_H
#define C_COMBATWEAPONWORLDCLONE_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseanimating.h"
#include "c_basecombatweapon.h"

class C_CombatWeaponClone : public C_BaseAnimating
{
	DECLARE_CLASS( C_CombatWeaponClone, C_BaseAnimating );

public:
	explicit C_CombatWeaponClone( C_BaseCombatWeapon *pWeaponParent ) : m_hWeaponParent( pWeaponParent ) {}

	// The weapon whose world model this entity draws.
	C_BaseCombatWeapon *GetWeaponParent() const { return m_hWeaponParent; }

private:
	CHandle< C_BaseCombatWeapon > m_hWeaponParent;
};

#endif // C_COMBATWEAPONWORLDCLONE_H
