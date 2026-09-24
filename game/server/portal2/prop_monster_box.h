//========= Portal 2 reconstruction ============================================//
//
// Purpose: Monster turret that can fold into a physics box.
//
//=============================================================================//

#ifndef PROP_MONSTER_BOX_H
#define PROP_MONSTER_BOX_H

#include "props.h"

class CPropMonsterBox : public CPhysicsProp
{
	DECLARE_CLASS( CPropMonsterBox, CPhysicsProp );
	DECLARE_DATADESC();

public:
	CPropMonsterBox();
	void Precache() OVERRIDE;
	void Spawn() OVERRIDE;
	void BecomeBox( bool bPlayEffects );
	void BecomeMonster( bool bPlayEffects );

private:
	void InputBecomeBox( inputdata_t &inputData );
	void InputBecomeMonster( inputdata_t &inputData );
	void InputBecomeShortcircuit( inputdata_t &inputData );
	void InputSilentDissolve( inputdata_t &inputData );
	void InputDissolve( inputdata_t &inputData );
	void MonsterThink();
	void SetBoxState( bool bBox );

	bool m_bIsABox;
	bool m_bForcedAsBox;
	bool m_bIsShortcircuit;
	float m_flBoxSwitchSpeed;
	COutputEvent m_OnFizzled;
};

#endif // PROP_MONSTER_BOX_H
