//========= Copyright (c) 1996-2007, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Base class for simple projectiles
//
// $NoKeywords: $
//=============================================================================//

#ifndef CBASEPROJECTILE_H
#define CBASEPROJECTILE_H
// F-Stop: CS:GO's simple studio-model projectile, renamed from
// CBaseProjectile because game/shared/baseprojectile.h (networked, TF-era)
// already owns that name in this game DLL.
#ifdef _WIN32
#pragma once
#endif


//=============================================================================
//=============================================================================
class CBaseSimpleProjectile : public CBaseAnimating
{
	DECLARE_DATADESC();
	DECLARE_CLASS( CBaseSimpleProjectile, CBaseAnimating );

public:
	void Touch( CBaseEntity *pOther );
	virtual void HandleTouch( CBaseEntity *pOther );

	void Think();
	virtual void HandleThink();

	void Spawn(	char *pszModel,
		const Vector &vecOrigin,
		const Vector &vecVelocity,
		edict_t *pOwner,
		MoveType_t	iMovetype,
		MoveCollide_t nMoveCollide,
		int	iDamage,
		int iDamageType,
		CBaseEntity *pIntendedTarget = NULL );

	virtual void Precache( void ) {};

	int	m_iDmg;
	int m_iDmgType;
	EHANDLE m_hIntendedTarget;
};

#endif // CBASEPROJECTILE_H
