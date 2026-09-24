//========= Portal 2 reconstruction ============================================//
//
// Purpose: Server side of the paint gun weapon.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef WEAPON_PAINTGUN_H
#define WEAPON_PAINTGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_portalbasecombatweapon.h"
#include "paint_color_manager.h"

class CSoundPatch;

class CWeaponPaintGun : public CBasePortalCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponPaintGun, CBasePortalCombatWeapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeaponPaintGun();
	virtual ~CWeaponPaintGun();

	virtual void	Spawn();
	virtual void	Precache();
	virtual void	PrimaryAttack();
	virtual void	SecondaryAttack();
	virtual bool	Deploy();
	virtual void	Drop( const Vector &vecVelocity );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	virtual bool	SendWeaponAnim( int iActivity );
	virtual void	SetSubType( int iType );
	virtual void	WeaponIdle();
	void			PaintGunThink();
	virtual void	ItemPostFrame();
	virtual void	UpdateOnRemove();

	void			SetCurrentPaint( PaintPowerType nIndex );
	PaintPowerType	GetCurrentPaint();
	int				GetPaintCount();

	bool			ShouldDisplayHUDHint();

	void			ActivatePaint( PaintPowerType nIndex );
	void			ResetPaint();
	void			DeactivatePaint( PaintPowerType nIndex );

	void			CleansePaint();

	bool			HasPaintPower( PaintPowerType nIndex );
	bool			HasAnyPaintPower();

	// Paint ammo (implemented in weapon_paintgun_shared.cpp)
	bool			HasPaintAmmo( unsigned paintType ) const;
	void			DecrementPaintAmmo( unsigned paintType );
	void			ResetAmmo();

	DECLARE_ACTTABLE();

private:
	bool			HasCurrentColor();
	void			SprayPaint( float flDeltaTime, int paintType );
	void			StartShootingSound();
	void			StopShootingSound();

	Vector			m_vecOldBlobFirePos;

	CNetworkArray( bool, m_bHasPaint, PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER );
	CNetworkArray( EHANDLE, m_hPaintStream, PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER );

	float			m_flAccumulatedTime;
	CNetworkVar( int, m_nCurrentColor );

	CNetworkVar( bool, m_bFiringPaint );
	CNetworkVar( bool, m_bFiringErase );

	CSoundPatch		*m_pLiquidLoop;
	CSoundPatch		*m_pLiquidStart;

	int				m_nBlobRandomSeed;

	// Reconstruction note: the paint ammo state is absent from the 2010 DWARF
	// layout; weapon_paintgun_shared.cpp (retail) reads and writes it. The
	// 2010 send table does not network it, so it stays server/client local.
	CNetworkVar( int, m_nPaintAmmo );
	CNetworkArray( int, m_PaintAmmoPerType, PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER );
};

#endif // WEAPON_PAINTGUN_H
