// DWARF declaration skeleton for game/server/portal2/weapon_paintgun.h
// Source: Steam2 depot 841_1 server.dylib.dSYM
// Generated reconstruction aid; not original source and not buildable.
// Layouts and offsets are for the 2010 macOS i386 build.
// game/server/portal2/weapon_paintgun.h:20 sizeof=0x678 (i386)
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
	virtual void UpdateOnRemove();  // line 40
	void SetCurrentPaint( PaintPowerType );  // line 42
	PaintPowerType GetCurrentPaint();  // line 43
	int GetPaintCount();  // line 44
	bool ShouldDisplayHUDHint();  // line 46
	void ActivatePaint( PaintPowerType );  // line 48
	void ResetPaint();  // line 49
	void DeactivatePaint( PaintPowerType );  // line 50
	void CleansePaint();  // line 52
	bool HasPaintPower( PaintPowerType );  // line 54
	bool HasAnyPaintPower();  // line 55
	static acttable_t m_acttable[];  // line 64
	virtual acttable_t *ActivityList();  // line 64
	virtual int ActivityListCount();  // line 64
private:
	bool HasCurrentColor();  // line 67
	void SprayPaint( float, int );  // line 68
	void StartShootingSound();  // line 69
	void StopShootingSound();  // line 70
	Vector m_vecOldBlobFirePos; // +0x638  // line 72
	CWeaponPaintGun::NetworkVar_m_bHasPaint m_bHasPaint; // +0x644  // line 74
	CWeaponPaintGun::NetworkVar_m_hPaintStream m_hPaintStream; // +0x64c  // line 75
	float m_flAccumulatedTime; // +0x660  // line 77
	CNetworkVar( int, m_nCurrentColor ); // +0x664  // line 78
	CNetworkVar( bool, m_bFiringPaint ); // +0x668  // line 80
	CNetworkVar( bool, m_bFiringErase ); // +0x669  // line 81
	CSoundPatch *m_pLiquidLoop; // +0x66c  // line 83
	CSoundPatch *m_pLiquidStart; // +0x670  // line 84
	int m_nBlobRandomSeed; // +0x674  // line 86
};

// game/server/portal2/weapon_paintgun.h:75 (declaration)
void NetworkVar_m_hPaintStream();
