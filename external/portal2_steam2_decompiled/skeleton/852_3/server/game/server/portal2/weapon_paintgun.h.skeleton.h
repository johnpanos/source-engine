// DWARF declaration skeleton for game/server/portal2/weapon_paintgun.h
// Source: Steam2 depot 852_3 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/weapon_paintgun.h:20 sizeof=0x664 (i386)
struct CWeaponPaintGun : public CBasePortalCombatWeapon
{
public:
	CWeaponPaintGun();  // line 25
	virtual ~CWeaponPaintGun();  // line 26
	virtual void Spawn();  // line 28
	virtual void Precache();  // line 29
	virtual void PrimaryAttack();  // line 30
	virtual void SecondaryAttack();  // line 31
	virtual bool Deploy();  // line 32
	virtual void Drop( const Vector & );  // line 33
	virtual bool Holster( CBaseCombatWeapon * );  // line 34
	virtual bool SendWeaponAnim( int );  // line 35
	virtual void SetSubType( int );  // line 36
	virtual void WeaponIdle();  // line 37
	void PaintGunThink();  // line 38
	virtual void ItemPostFrame();  // line 39
	void SetCurrentPaint( PaintPowerType );  // line 41
	PaintPowerType GetCurrentPaint();  // line 42
	int GetPaintCount();  // line 43
	bool ShouldDisplayHUDHint();  // line 45
	void ActivatePaint( PaintPowerType );  // line 47
	void ResetPaint();  // line 48
	void DeactivatePaint( PaintPowerType );  // line 49
	void CleansePaint();  // line 51
	bool HasPaintPower( PaintPowerType );  // line 53
	bool HasAnyPaintPower();  // line 54
	static acttable_t m_acttable[];  // line 63
	virtual acttable_t *ActivityList();  // line 63
	virtual int ActivityListCount();  // line 63
private:
	bool HasCurrentColor();  // line 66
	void SprayPaint( float, int );  // line 67
	void StartShootingSound();  // line 68
	void StopShootingSound();  // line 69
	Vector m_vecOldBlobFirePos; // +0x638  // line 71
	CWeaponPaintGun::NetworkVar_m_bHasPaint m_bHasPaint; // +0x644  // line 73
	float m_flAccumulatedTime; // +0x64c  // line 74
	CNetworkVar( int, m_nCurrentColor ); // +0x650  // line 75
	CNetworkVar( bool, m_bFiringPaint ); // +0x654  // line 77
	CNetworkVar( bool, m_bFiringErase ); // +0x655  // line 78
	CSoundPatch *m_pLiquidLoop; // +0x658  // line 80
	CSoundPatch *m_pLiquidStart; // +0x65c  // line 81
	int m_nBlobRandomSeed; // +0x660  // line 83
};
