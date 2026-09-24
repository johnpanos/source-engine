//========= Portal 2 reconstruction ============================================//
//
// Purpose: Client side of the paint gun weapon.
//
// Reconstructed from DWARF metadata and decompiler output of the Steam2 depot
// 841/852 macOS builds (external/portal2_steam2_decompiled). Not original
// Valve source; the repository's provenance and distribution warning applies.
//
//=============================================================================//

#ifndef C_WEAPON_PAINTGUN_H
#define C_WEAPON_PAINTGUN_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_portalbasecombatweapon.h"
#include "paint_color_manager.h"
#include "utlreference.h"

class CNewParticleEffect;

// The shared paint gun code uses the server class name
#define CWeaponPaintGun C_WeaponPaintGun

class C_WeaponPaintGun : public C_BasePortalCombatWeapon
{
public:
	DECLARE_CLASS( C_WeaponPaintGun, C_BasePortalCombatWeapon );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_WeaponPaintGun();

	virtual void	Precache();
	virtual void	Spawn();
	virtual void	WeaponIdle();
	virtual void	ClientThink();

	// Portal 2 port: the SDK always simulates client entities.
	virtual void	Simulate();
	virtual void	OnPreDataChanged( DataUpdateType_t type );
	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual bool	Deploy();
	virtual bool	Holster( C_BaseCombatWeapon *pSwitchingTo = NULL );
	virtual void	Drop( const Vector &vecVelocity );
	virtual void	SetSubType( int iType );
	virtual void	ItemPostFrame();

	void			StartHoseEffect();
	void			StopHoseEffect();

	C_BaseViewModel	*GetEffectViewModel();

	virtual void	AddViewmodelBob( C_BaseViewModel *viewmodel, Vector &origin, QAngle &angles );
	virtual float	CalcViewmodelBob();

	void			SetPaintPower( PaintPowerType type );
	void			CyclePaintPower( bool bForward );
	PaintPowerType	GetCurrentPaint();
	bool			HasPaintPower( PaintPowerType nIndex );
	bool			HasAnyPaintPower();
	void			ChangeRenderColor( bool bForce = false );

	virtual void	PrimaryAttack();
	virtual void	SecondaryAttack();

	// Paint ammo (implemented in weapon_paintgun_shared.cpp)
	bool			HasPaintAmmo( unsigned paintType ) const;
	void			DecrementPaintAmmo( unsigned paintType );
	void			ResetAmmo();

	DECLARE_ACTTABLE();

private:
	void			SprayPaint( float flDeltaTime, int paintType );

	CUtlReference< CNewParticleEffect >	m_pStreamEffect;
	float			m_flBobScale;
	float			m_flAccumulatedTime;
	Vector			m_vecOldBlobFirePos;

	float			m_flLastThinkTime;

	// Networked
	int				m_nCurrentColor;
	bool			m_bFiringPaint;
	bool			m_bFiringErase;

	CNetworkArray( bool, m_bHasPaint, PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER );
	CNetworkArray( EHANDLE, m_hPaintStream, PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER );

	// Owner and effect before the last network update
	C_BaseCombatCharacter				*m_pLastOwner;
	CUtlReference< CNewParticleEffect >	m_pLastEffect;

	int				m_nBlobRandomSeed;

	// Reconstruction note: the paint ammo state is absent from the 2010 DWARF
	// layout (which declared GetPaintAmmo()/HasPaintAmmo() instead); the retail
	// weapon_paintgun_shared.cpp reads and writes these. Not networked.
	int				m_nPaintAmmo;
	CNetworkArray( int, m_PaintAmmoPerType, PAINT_POWER_TYPE_COUNT_PLUS_NO_POWER );
};

#endif // C_WEAPON_PAINTGUN_H
